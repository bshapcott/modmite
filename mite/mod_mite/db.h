//____________________________________________________________________________
// mite - mod microlite - db.h
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info
//
/// @file
///
/// Functions related to database processing, which are driven by a parser (XML,
/// JSON or URL).  As the input HTTP request stream is parsed, the db_X functions
/// are invoked to perform actual database operations.
///
/// In turn, the db_X functions drive the output (the HTTP reply).  The transaction
/// cycle
///
///          INPUT    DATABASE   OUTPUT
///          json_                json_
///  HTTP -> url_   ->  db_   ->  xml_  -> HTTP
/// request  xml_                          reply
///          sql_
///

#ifndef db_h
#define db_h

#include "http.h"
#include "sqlite3.h"
#include "options.h"
#include "http_protocol.h"
#include "apr_tables.h"

typedef struct Transaction Transaction;

//____________________________________________________________________________
/// - each output type (XML, JSON) populates an instance of this structure
///   in Transaction with callbacks that are invoked to output results from
///   the database
/// - output callbacks are nested by part of transaction and result being
///   processed, delimited by X_start and X_end callback functions
///   where the hierarchy of values of X is:
///   - 'root' for the entire transaction
///   - 'database' for a named database
///   - 'sql' for SQL executed against the named database
///   - 'stmt' for each actual SQL statement executed
///   - 'row' for each result row
typedef struct output_callbacks {
  /// - start of the entire transaction (i.e. a single HTTP request)
  void (*root_start)(Transaction *xn, const char *loc);
  /// - end of the entire transaction
  void (*root_end)(Transaction *xn);
  /// - start metadata processing
  void (*meta_start)(Transaction *xn);
  /// - end metadata processing
  void (*meta_end)(Transaction *xn);
  /// - start processing a named database
  void (*db_start)(Transaction *xn, const char *dbn);
  /// - end database processing
  void (*db_end)(Transaction *xn);
  /// - start SQL processing
  void (*sql_start)(Transaction *xn);
  /// - end SQL processing
  void (*sql_end)(Transaction *xn);
  /// - start processing a single SQL statement
  void (*stmt_start)(Transaction *xn);
  /// - end processing a single SQL statement
  void (*stmt_end)(Transaction *xn);
  /// - start output for a single result row
  void (*row_start)(Transaction *xn, int pos);
  /// - end output for a single result row
  void (*row_end)(Transaction *xn, char open);
  /// - process a single column containing a simple value
  void (*column)(Transaction *xn, const char *key, const unsigned char *val,
                 int pos);
  /// - indicates change from simple values (numeric) to more complex
  ///   values (multi-line strings), which some output types will need
  ///   to handle differently (e.g. XML switches from attributes to
  ///   nested elements)
  /// - it may be that the simple and complex values must be distinguished
  ///   for the output engine, or it may be more efficent to represent the
  ///   values if distinguished
  void (*col_change)(Transaction *xn);
  /// - process a single column containing a complex value
  void (*text)(Transaction *xn, const char *key, const unsigned char *val,
               int pos);
  /// - posts a coment with the output engine
  /// - depending on the engine, the comment may not be output immediately,
  ///   but may instead be deferred until a legal position for insertion of
  ///   a comment is encountered and/or output of the comment will not ruin
  ///   the output format
  void (*comment)(Transaction *xn, const char *msg);
  /// - posts an error with the output engine
  void (*error)(Transaction *xn, const char *msg);
} output_callbacks;

//____________________________________________________________________________
/// - a transaction is a single HTTP request
/// - this aggregates data that would otherwise be passed separately on stack
struct Transaction {
  /// - HTTP request record from Apache
  request_rec *request;
  /// - multiple output formats are supported; callbacks are a form of
  ///   polymorphism
  output_callbacks *ocb;
  /// - connection to sqlite database
  /// @todo - should come from a connection pool
  sqlite3 *db;
  /// - path elements extracted from the target URL
  apr_array_header_t *path;
  /// - SQL statements
  apr_array_header_t *statements;
  /// \todo dox
  const char *name;
  /// - transaction parameters, derived from URL query parameters, cookie
  ///   (session only), or last inserted rowid
  apr_table_t *parameters;
  /// - parse level
  int level;
  /// - scratch area, to avoid temp on stack
  char *scratch;
  /// - parameter name
  char *parm;
  /// - database column
  int column;
  /// - ???
  void *data;
  /// - ???
  char *comment;
  /// - SQL statement being executed was generated programatically
  char synthetic;
  /// numbuer of SQL commands in database operation
  int sql_count;
  /// - number of statements to execute in SQL command
  int stmt_count;
};

/// - create a new transaction record
/// @param r - request record from Apache
Transaction *db_transaction_new(request_rec *r);
/// - multiple databases may be affected in a single Transaction
/// - this function open a single database connection identified
///   by name
/// @param xn - the transaction information associated with the HTTP request
void db_start(Transaction *xn);
/// - closes the current database connection
/// @param xn - the transaction information associated with the HTTP request
void db_end(Transaction *xn);
/// - called when a new set of SQL statements is to be invoked
/// @param xn - the transaction information associated with the HTTP request
void db_sql_start(Transaction *xn);
/// - finished executing a set of SQL statements
/// @param xn - the transaction information associated with the HTTP request
void db_sql_end(Transaction *xn);
/// - start binding parameters (extracted from the HTTP request) to SQL
///   statements
/// @param xn - the transaction information associated with the HTTP request
void db_bind_start(Transaction *xn);
/// -parameter binding is complete, execute the SQL statements
/// @param xn - the transaction information associated with the HTTP request
/// @todo - clean up synthesized sql statements
void db_bind_end(Transaction *xn);
/// - convenience routine to invoke db_col_key and db_col_val when
///   key and value are available immediately (which depends on the parser
///   processing the HTTP input stream)
/// @param xn - the transaction information associated with the HTTP request
/// @param k - database column key (column name)
/// @param v - database column value
/// @todo - clean up synthesized sql statements
void db_col_kv(Transaction *xn, const char *k, const char *v);
/// - process the database column key (column name)
/// @param xn  - the transaction information associated with the HTTP request
/// @param key - database column key (column name)
/// @param len - length of key string (for parsers that don't pass NULL
///              terminated strings
void db_col_key(Transaction *xn, const char *key, size_t len);
/// - process the database column value
/// - must follow a call to db_col_key, or the column name must be set in the
///   Transaction scratch buffer
/// @param xn  - the transaction information associated with the HTTP request
/// @param val - database column value
/// @param len - length of val string (for parsers that don't pass NULL
///              terminated strings
void db_col_val(Transaction *xn, const char *val, size_t len);
/// -report an error
/// @param xn  - the transaction information associated with the HTTP request
/// @param msg - the error message
void db_error(Transaction *xn, const char *msg);

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
