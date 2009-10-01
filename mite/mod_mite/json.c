//____________________________________________________________________________
// mite - mod microlite - json.cpp
//
// Copyright 2008-2009 Brad Shapcott
//
/// @file
///
/// - convert between JSON and SQL

#include "json.h"
#include "sql.h"
#include "sqlite3.h"
#include "apr_reslist.h"
#include "apr_hash.h"
#include "apr_strings.h"
#include "yajl/yajl_parse.h"
#include <assert.h>

static yajl_callbacks callbacks;

enum {
   /// - document root, the outer braces
   root,
   /// - database level
   db,
   /// - SQL, identified by stored SQL, table or view name
   sql,
   /// - array is open (handling depends on next token)
   array,
   /// - processing array elements (i.e. array not empty)
   elements,
   /// - directly binding an object, e.g. foo: {...}
   binding,
   /// - binding objects within an array context, e.g. foo: [{...},{...}]
   objects,
   /// - using array members as positional bind, e.g. foo: [1, 2, 3]
   single,
   /// - positional bind within array context, e.g. foo: [[1,2,3],[4,5,6]]
   multi
};

static char *json_comment_get(Transaction *xn);
static void json_comment_reset(Transaction *xn);

//____________________________________________________________________________
void json_parse(Transaction *xn, const char *buf, apr_size_t len) {
   yajl_parser_config cfg = {1, 1};
   yajl_handle hand = yajl_alloc(&callbacks, &cfg, NULL, (void *)xn);
   yajl_status stat =
      yajl_parse(hand, (unsigned char const *)buf, len);
   // todo: check status and handle errors
   if (yajl_status_ok != stat) {
      // todo: error handling
      unsigned char *err = yajl_get_error(hand, 1, (unsigned char const*)buf, len);
      (*xn->ocb->error)(xn, (char *)err);
      yajl_free_error(hand, err);
   }
   yajl_free(hand);
   return;
}

//____________________________________________________________________________
/// - if currently processing array, put state back to processing single
///   values
/// @param u - the user data passed to callback function
///          - casts to Transaction
/// @return  - the Transaction supplied as user data
/// @todo - check in proper state?
static Transaction *json_check_state(void *u) {
   Transaction *xn = (Transaction *)u;
   if (xn->level == array) {
      db_bind_start(xn);
      xn->level = single;
   }
   return xn;
}

//____________________________________________________________________________
/// - clean up after processing binding
/// @param xn  - transaction containing data for a single HTTP request
static void json_cleanup(Transaction *xn) {
   apr_table_clear(xn->parameters);
   if (xn->synthetic) {
      sql_cleanup(xn);
   }
   return;
}

//____________________________________________________________________________
/// - process a JSON 'null' literal
/// - yajl callback
/// @param u - the user data passed to callback function
///          - casts to Transaction
/// @return  - 1, to indicate to yajl that parsing should continue
static int json_null(void *u) {
   db_col_val(json_check_state(u), NULL, 0);
   return 1;
}

//____________________________________________________________________________
/// - process a JSON boolean literal
/// - yajl callback
/// @param u - the user data passed to callback function
///          - casts to Transaction
/// @param b - the value of the boolean literal
/// @return  - 1, to indicate to yajl that parsing should continue
static int json_boolean(void *u, int b) {
   db_col_val(json_check_state(u), b ? "1" : "0", 1);
   return 1;
}

//____________________________________________________________________________
/// - process a JSON string literal
/// - yajl callback
/// @param u - the user data passed to callback function
///          - casts to Transaction
/// @param s - the value of the string literal
/// @param n - the length (in bytes) of the string literal (not number of
///            characters)
/// @return  - 1, to indicate to yajl that parsing should continue
static int json_string(void *u, unsigned char const *s, unsigned int n) {
   Transaction *xn = json_check_state(u);
   if (xn->level == sql) {
      // - "task: 1" is shorthand for "task: {id: 1}"
      // todo: check for singular integer primary key instead of assuming
      //       "id"
      xn->name = &xn->scratch[1];
      db_sql_start(xn);
      db_bind_start(xn);
      strcpy(&xn->scratch[1], "id");
      db_col_val(xn, (char const *)s, n);
      db_bind_end(xn);
      json_cleanup(xn);
      db_sql_end(xn);
   } else {
      db_col_val(xn, (char const *)s, n);
   }
   return 1;
}

//____________________________________________________________________________
/// - process a JSON numeric literal
/// - processed as a string literal, because sqlite just processes everything
///   as a string
/// - yajl callback
/// @param u - the user data passed to callback function
///          - casts to Transaction
/// @param s - the numeric literal value as a string
/// @param n - the number of byters in the string literal (not characters)
/// @return  - 1, to indicate to yajl that parsing should continue
static int json_number(void *u, char const *s, unsigned int n) {
   return json_string(u, (unsigned char const *)s, n);
}

//____________________________________________________________________________
/// - key is LHS of colon, i.e. the 'name' of the next value parsed
///   on the RHS
/// - yajl callback
/// @param u - the user data passed to callback function
///          - casts to Transaction
/// @param k - the value of key
/// @param n - the number of bytes in key string (not characters)
/// @return  - 1, to indicate to yajl that parsing should continue
static int json_map_key(void *u, unsigned char const *k, unsigned int n) {
   Transaction *xn = (Transaction *)u;
   strncpy(&xn->scratch[1], (char const *)k, n);
   xn->scratch[n+1] = 0;
   return 1;
}

//____________________________________________________________________________
/// - start mapping a JSON structure, which starts with an opening brace
/// - yajl callback
///   '{'
/// @param u - the user data passed to callback function
///          - casts to Transaction
/// @return  - 1, to indicate to yajl that parsing should continue
static int json_map_start(void *u) {
   Transaction *xn = (Transaction *)u;
   switch (xn->level) {
   case root:
      (*xn->ocb->root_start)(xn,
                             ((char **)xn->path->elts)[0]);
      xn->level = db;
      break;
   case db:
      db_start(xn);
      xn->level = sql;
      break;
   case sql:
      xn->name = &xn->scratch[1];
      db_sql_start(xn);
      db_bind_start(xn);
      xn->level = binding;
      break;
   case array:
   case elements:
      db_bind_start(xn);
      xn->level = objects;
      break;
   default:
      // todo: nested
      (*xn->ocb->error)(xn, "unexpected '{'");
   }
   return 1;
}

//____________________________________________________________________________
/// - completes processing of a JSON structure, which ends with a closing
///   brace '}'
/// - yajl callback
/// @param u - the user data passed to callback function
///          - casts to Transaction
/// @return  - 1, to indicate to yajl that parsing should continue
static int json_map_end(void *u) {
   Transaction *xn = (Transaction *)u;
   switch (xn->level) {
   case db:
      db_end(xn);
      xn->level = root;
      (*xn->ocb->root_end)(xn);
      break;
   case sql:
      xn->level = db;
      break;
   case binding:
      db_bind_end(xn);
      json_cleanup(xn);
      db_sql_end(xn);
      xn->level = sql;
      break;
   case objects:
      db_bind_end(xn);
      json_cleanup(xn);
      xn->level = elements;
      break;
   default:
      (*xn->ocb->error)(xn, "unexpected '}'");
   }
   return 1;
}

//____________________________________________________________________________
/// - start processing a JSON array, which starts with a square bracket '['
/// - yajl callback
/// @param u - the user data passed to callback function
///          - casts to Transaction
/// @return  - 1, to indicate to yajl that parsing should continue
/// @todo - positional binds
static int json_array_start(void *u) {
   Transaction *xn = (Transaction *)u;
   switch (xn->level) {
   case sql:
      xn->name = &xn->scratch[1];
      db_sql_start(xn);
      xn->level = array;
      break;
   case array:
   case elements:
      xn->scratch[1] = 0;
      xn->column = 1;
      db_bind_start(xn);
      xn->level = multi;
      break;
   default:
      // todo: nested
      (*xn->ocb->error)(xn, "unexpected '['");
   }
   return 1;
}

//____________________________________________________________________________
/// - complete processing of a JSON array, which ends with a square bracket
///   ']'
/// - yajl callback
/// @param u - the user data passed to callback function
///          - casts to Transaction
/// @return  - 1, to indicate to yajl that parsing should continue
static int json_array_end(void *u) {
   Transaction *xn = (Transaction *)u;
   switch (xn->level) {
   case array:
      // - only happens with empty array
      db_bind_start(xn);
      db_bind_end(xn);
      json_cleanup(xn);
      // no break; statement is intentional! 
   case elements:
      db_sql_end(xn);
      xn->level = sql;
      break;
   case single:
      db_bind_end(xn);
      json_cleanup(xn);
      db_sql_end(xn);
      xn->level = sql;
      break;
   case multi:
      db_bind_end(xn);
      json_cleanup(xn);
      xn->level = elements;
      break;
   default:
      (*xn->ocb->error)(xn, "unexpected ']'");
   }
   return 1;
}

//____________________________________________________________________________
/// - structure containing callback functions for yajl
static yajl_callbacks callbacks =
   {json_null, json_boolean, NULL, NULL, json_number, json_string,
    json_map_start, json_map_key, json_map_end,
    json_array_start, json_array_end};

//____________________________________________________________________________
/// - start output of JSON
/// - simply outputs an opening brace
/// @param xn  - the transaction information associated with the HTTP request
/// @param loc - location (Apache directory)
static void json_root_start(Transaction *xn, char const *loc) {
   ap_rputs("[{", xn->request);
   return;
}

//____________________________________________________________________________
/// - complete output of JSON
/// - simply outputs a closing brace
/// @param xn - the transaction information associated with the HTTP request
static void json_root_end(Transaction *xn) {
   ap_rputs("}]", xn->request);
   return;
}

//____________________________________________________________________________
/// - start output of database metadata
/// - outputs a synthetic key, since none is directly associated with the
///   database, and an opening brace
/// @param xn - the transaction information associated with the HTTP request
static void json_meta_start(Transaction *xn) {
   ap_rputs("\"meta\":{", xn->request);
}

//____________________________________________________________________________
/// - completes output of database metadata
/// - simply outputs a closing brace
/// @param xn - the transaction information associated with the HTTP request
static void json_meta_end(Transaction *xn) {
   ap_rputs("}", xn->request);
   return;
}

//____________________________________________________________________________
/// - start processing SQL for a particular database
/// - key is the name of the database, which is the database file's basename
///   (the filename with path and extension removed)
/// - nested output (that within the database's context) is contained in a
///   JSON array
/// @param xn  - the transaction information associated with the HTTP request
/// @param dbn - database name
static void json_db_start(Transaction *xn, char const *dbn) {
   // todo: compact output for JS
   ap_set_content_type(xn->request, "application/json;charset=ascii");
   ap_rprintf(xn->request, "\"%s\": [", dbn);
   return;
}

//____________________________________________________________________________
/// - completes processing for a particular database
/// - close the array contained output nexted in this database's context
/// @param xn - the transaction information associated with the HTTP request
static void json_db_end(Transaction *xn) {
   ap_rprintf(xn->request, "]%s\n", json_comment_get(xn));
   json_comment_reset(xn);
   return;
}

//____________________________________________________________________________
/// - starts processing SQL
/// - SQL is identified by name, and may be a table or view name (against
///   which actual SQL statements are generated or synthesized) or a key
///   to a table holding SQL statements
/// @param xn - the transaction information associated with the HTTP request
static void json_sql_start(Transaction *xn) {
   assert(xn->sql_count > 0);
   ap_rprintf(xn->request, "%s%s%s{\"%s\": [",
              xn->sql_count == 1 ? "" : ",",
              xn->sql_count == 1 ? "" : json_comment_get(xn),
              xn->sql_count == 1 ? "" : "\n",
              xn->name);
   if (xn->sql_count != 1) json_comment_reset(xn);
   return;
}

//____________________________________________________________________________
/// - completes SQL processing
/// - outputs square bracket ']' to close the array
/// @param xn - the transaction information associated with the HTTP request
static void json_sql_end(Transaction *xn) {
   ap_rputs("]}", xn->request);
   return;
}

//____________________________________________________________________________
/// - start processing a single SQL statement (INSERT, UPDATE, etc.)
/// - contains the row results from processing the statement in a JSON array
/// @param xn - the transaction information associated with the HTTP request
static void json_stmt_start(Transaction *xn) {
   assert(xn->stmt_count > 0);
   ap_rprintf(xn->request, "%s%s\n  [", xn->stmt_count == 1 ? "" : ",",
              json_comment_get(xn));
   json_comment_reset(xn);
   return;
}

//____________________________________________________________________________
/// - completes processing of a single SQL statement
/// - outputs square bracket ']' to close the array
/// @param xn - the transaction information associated with the HTTP request
static void json_stmt_end(Transaction *xn) {
   ap_rputs("]", xn->request);
   return;
}

//____________________________________________________________________________
/// - start processing a single result row
/// - the row result is represented by a JSON object containing column values
/// @param xn  - the transaction information associated with the HTTP request
/// @param pos - row position, first row is '1'
///            - used for formatting (e.g. delimiters)
static void json_row_start(Transaction *xn, int pos) {
   assert(pos > 0);
   ap_rprintf(xn->request, "%s%s%s{",
              pos == 1 ? " " : ",",
              pos == 1 ? "" : json_comment_get(xn),
              pos == 1 ? "" : "\n    ");
   if (pos != 1) json_comment_reset(xn);
   return;
}

//____________________________________________________________________________
/// - completes processing a single result row
/// - outputs closing brace '}' to close the JSON object
/// @param xn   - the transaction information associated with the HTTP request
/// @param open - ignored
static void json_row_end(Transaction *xn, char open) {
   ap_rputs("}", xn->request);
   return;
}

//____________________________________________________________________________
/// - process a column in a result row
/// @param xn  - the transaction information associated with the HTTP request
/// @param key - column name
/// @param val - column string value
/// @param pos - row position, first row is '1'
///            - used for formatting (e.g. delimiters)
static void json_col(Transaction *xn, char const *key, unsigned char const *val,
                     int pos)
{
   assert(pos > 0);
   ap_rprintf(xn->request, "%s%s%s\"%s\":\t%s%s%s",
              pos == 1 ? " " : ",",
              pos == 1 ? "" : json_comment_get(xn),
              pos == 1 ? "" : "\n      ",
              key,
              val ? "\"" : "", val ? val : (unsigned char const *)"null", val ? "\"" : "");
   if (pos != 1) json_comment_reset(xn);
   return;
}

//____________________________________________________________________________
/// - does nothing
/// @param xn - the transaction information associated with the HTTP request
static void json_col_change(Transaction *xn) {
   return;
}

//____________________________________________________________________________
/// - process a comment
/// - output of the comment is deferred until an appropriate point in the
///   JSON
/// - comments are accumulated until output
/// @param xn  - the transaction information associated with the HTTP request
/// @param msg - comment
static void json_comment(Transaction *xn, char const *msg) {
#if CH_DEV
   if (!xn->comment) {
      xn->comment = (char *)apr_palloc(xn->request->pool, 256);
      *xn->comment = 0;
   }
   if (!*xn->comment) {
      strcat(xn->comment, " /* ");
   }
   strcat(xn->comment, msg);
#endif
   return;
}

//____________________________________________________________________________
/// - get all the pending comment text
/// @param xn - the transaction information associated with the HTTP request
/// @return   - all the comment text accumulated from calls to json_comment()
static char *json_comment_get(Transaction *xn) {
   if (xn->comment && *xn->comment) {
      strcat(xn->comment, " */");
   }
   return xn->comment ? xn->comment : "";
}

//____________________________________________________________________________
/// - clears all accumulated comment text, setting pending comments to be
///   empty
/// @param xn - the transaction information associated with the HTTP request
static void json_comment_reset(Transaction *xn) {
   xn->comment && (*xn->comment = 0);
   return;
}

//____________________________________________________________________________
/// - outputs the error as a comment immediately (i.e. not deferred)
/// @param xn  - the transaction information associated with the HTTP request
/// @param msg - the error message
/// @todo - this is for development, need real error handling here for
///         production code
static void json_error(Transaction *xn, char const *msg) {
#if CH_DEV
   ap_rprintf(xn->request, "/* ERROR: %s */", msg);
#endif
   return;
}

//____________________________________________________________________________
output_callbacks json_ocb = {
   json_root_start, json_root_end,
   json_meta_start, json_meta_end,
   json_db_start, json_db_end, json_sql_start, json_sql_end,
   json_stmt_start, json_stmt_end, json_row_start, json_row_end,
   json_col, json_col_change, json_col, json_comment, json_error};

//____________________________________________________________________________
// JUNKYARD
#if 0
#endif

//_____________________________________________________________________ CONTACT
// mailto:brad at modmite dot com
// http://modmite.com
//_____________________________________________________________________ LICENSE
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//_________________________________________________________________________ EOF
