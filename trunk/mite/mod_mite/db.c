//____________________________________________________________________________
// mite - mod microlite - db.c
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info

#include "http.h"
#include "db.h"
#include "meta.h"
#include "request.h"
#include "sql.h"
#include "vjs.h"
#include "vtab.h"
#include "sqlite3.h"
#include "apr_strings.h"
#include <assert.h>

struct output_callbacks;
extern output_callbacks json_ocb;
extern output_callbacks xml_ocb;

//____________________________________________________________________________
Transaction *db_transaction_new(request_rec *r) {
  const char *accept;
  Transaction *xn = (Transaction *)apr_palloc(r->pool, sizeof(Transaction));
  xn->request = r;
  xn->level = 0;
  xn->statements =
    apr_array_make(xn->request->pool, 12, sizeof(sqlite3_stmt *));
  xn->scratch = xn->parm = (char *)apr_palloc(r->pool, 256);
  // todo: *xn->scratch++ = '$';
  *xn->scratch = '$';
  xn->parameters = apr_table_make(xn->request->pool, 12);
  xn->path = apr_array_make(r->pool, 3, sizeof(char *));
  xn->ocb = &json_ocb;
  // todo: replace quick & dirty handling of Accept header
  accept =  apr_table_get(r->headers_in, "Accept");
  if (strstr(accept, "xml")) {
    xn->ocb = &xml_ocb;
  }
  xn->synthetic = 0;
  xn->sql_count = xn->stmt_count = 0;
  xn->data = NULL;
  xn->comment = NULL;
  return xn;
}

//____________________________________________________________________________
/// @todo - connection pooling
void db_start(Transaction *xn) {
  int rc;
  /// @todo - bounds check
  if (xn->scratch[1]) {
    strcat(xn->scratch, ".db");
  };
  /// @todo - fstat first and fail if does not exist!
  /// @todo - db pool
  xn->sql_count = xn->stmt_count = 0;
  rc = sqlite3_open(&xn->scratch[1], &xn->db);
  if (xn->scratch[1]) {
    xn->scratch[strlen(xn->scratch)-3] = 0;
  }
  (*xn->ocb->db_start)(xn, &xn->scratch[1]);
  if (rc != SQLITE_OK) {
    db_error(xn, "database open failed");
  } else {
    vtab(xn->db);
    vjs(xn->db);
    meta(xn->db, xn->request->pool);
    request(xn);
  }
  return;
}

//____________________________________________________________________________
void db_end(Transaction *xn) {
  if (xn->db) {
    sqlite3_close(xn->db);
  }
  (*xn->ocb->db_end)(xn);
  return;
}

//____________________________________________________________________________
void db_sql_start(Transaction *xn) {
  assert((xn->statements->nelts == 0 && xn->name && *xn->name) ||
         xn->statements->nelts);
  if (xn->statements->nelts == 0) {
    sql_get(xn);
  }
  if (!(xn->name && *xn->name)) {
    xn->name = "anonymous";
  } else if (xn->statements->nelts == 0 && xn->name == &xn->scratch[1]) {
    // - duplicate so name isn't overwritten during parameter
    //   binds, because it will be required for sql_synth
    xn->name = apr_pstrdup(xn->request->pool, &xn->scratch[1]);
  }
  ++xn->sql_count;
  xn->stmt_count = 0;
  (*xn->ocb->sql_start)(xn);
  return;
}

//____________________________________________________________________________
void db_sql_end(Transaction *xn) {
  sql_cleanup(xn);
  (*xn->ocb->sql_end)(xn);
  return;
}

//____________________________________________________________________________
void db_bind_start(Transaction *xn) {
  int i;
  for (i = 0; i < xn->statements->nelts; i++) {
    sqlite3_stmt *statement = ((sqlite3_stmt **)xn->statements->elts)[i];
    sqlite3_reset(statement);
    sqlite3_clear_bindings(statement);
  }
  return;
}

//____________________________________________________________________________
void db_bind_end(Transaction *xn) {
  int i, j, c, r;
  int col_count; 
  char h;	
  int row_count = 0;
  if (apr_is_empty_array(xn->statements)) {
    sql_synth(xn);
  }
  if (apr_is_empty_array(xn->statements)) {
    db_error(xn, "no SQL");
    return;
  }
  for (i = 0; i < xn->statements->nelts; i++) {
    sqlite3_stmt *statement = ((sqlite3_stmt **)xn->statements->elts)[i];
    row_count = 0;
    ++xn->stmt_count;
    (*xn->ocb->stmt_start)(xn);
#if CH_DEV
    (*xn->ocb->comment)(xn, sqlite3_sql(statement));
#endif
    while (SQLITE_ROW == (r = sqlite3_step(statement))) {
      (*xn->ocb->row_start)(xn, ++row_count);
      c = sqlite3_data_count(statement);
      h = 0;
      col_count = 0;
      for (j = 0; j < c; j++) {
        if (!sqlite3_column_decltype(statement, j)
            ||
            strcmp(sqlite3_column_decltype(statement, j), "TEXT"))
          {
            (*xn->ocb->column)(xn,
                               sqlite3_column_name(statement, j),
                               sqlite3_column_text(statement, j), ++col_count);
          } else {
          h = 1;
        }
      }
      if (h) {
        (*xn->ocb->col_change)(xn);
        for (j = 0; j < c; j++) {
          if (sqlite3_column_decltype(statement, j)
              &&
              !strcmp(sqlite3_column_decltype(statement, j), "TEXT"))
            {
              (*xn->ocb->text)(xn,
                               sqlite3_column_name(statement, j),
                               sqlite3_column_text(statement, j), ++col_count);
            }
        }
      }
      (*xn->ocb->row_end)(xn, !h);
    }
    if (r != SQLITE_OK && r != SQLITE_DONE) {
      // @todo - fail the transaction, don't just emit comment!
      (*xn->ocb->comment)(xn, sqlite3_errmsg(xn->db));
    }
    (*xn->ocb->stmt_end)(xn);
  }
  return;
}

//____________________________________________________________________________
void db_col_kv(Transaction *xn, char const *k, char const *v) {
  db_col_key(xn, k, strlen(k));
  db_col_val(xn, v, strlen(v));
  return;
}

//____________________________________________________________________________
void db_col_key(Transaction *xn, char const *key, size_t len) {
  strncpy(&xn->scratch[1], key, len);
  xn->scratch[len+1] = 0;
  return;
}

//____________________________________________________________________________
void db_col_val(Transaction *xn, char const *val, size_t len) {
#if CH_DEV
  (*xn->ocb->comment)(xn, xn->scratch);
  (*xn->ocb->comment)(xn, "=");
  (*xn->ocb->comment)(xn, apr_pstrmemdup(xn->request->pool, val, len));
#endif
  if (apr_is_empty_array(xn->statements)) {
    // - accumulate paramaters so a sql_synth can be tried later
    /// @todo - is the duplication necessary? maybe text is preserved
    ///         in the parse buffer
    /// @todo - for positional binds, use table info to look up or
    ///         binding positions for stored SQL
    apr_table_addn(xn->parameters,
                   apr_pstrdup(xn->request->pool, &xn->scratch[1]),
                   apr_pstrmemdup(xn->request->pool, val, len));
  } else {
    int i;
    for (i = 0; i < xn->statements->nelts; i++) {
      sqlite3_stmt *statement = ((sqlite3_stmt **)xn->statements->elts)[i];
      int index = xn->scratch[1] ?
        sqlite3_bind_parameter_index(statement, xn->scratch) :
        xn->column++;
      if (index > 0) {
        if (val) {
          sqlite3_bind_text(statement, index, val, len, SQLITE_TRANSIENT);
        } else {
          sqlite3_bind_null(statement, index);
        }
      }
    }
  }
  return;
}

//____________________________________________________________________________
void db_error(Transaction *xn, char const *msg) {
  (*xn->ocb->error)(xn, msg);
  return;
}

//____________________________________________________________________________
// JUNKYARD
#if 0

// tktk: pool during config
apr_table_t *tables;
for (int i = 0; i < 2; i++) {
  sqlite3_stmt *s = NULL;
  int rc = sqlite3_prepare_v2(db,
                              i ? "SELECT name FROM sqlite_master WHERE type = 'table'"
                              : "SELECT distinct name FROM query", -1, &s, NULL);
  tables = apr_table_make(r->pool, 12);
  while (sqlite3_step(s) == SQLITE_ROW) {
    const char *t = apr_pstrdup(r->pool, (const char *)sqlite3_column_text(s, 0));
    apr_table_addn(tables, t, t);
  }
  sqlite3_finalize(s);
 }

apr_table_get(tables, sqlite3_column_name(*sp, i));
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
