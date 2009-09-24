//____________________________________________________________________________
// mite - mod microlite - sql.h
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info

#ifndef sql_h
#define sql_h

#include "options.h"
#include "db.h"

#if CH_DEV
/// - prepare statements from SQL directly
/// - not available in a production environment, as execution of arbitrary
///   SQL is enabled by this function
/// - prepared statements are recorded in Transaction
/// - SQL is input only, no output format is supported
/// @param xn  - the transaction information associated with the HTTP request
/// @param buf - the SQL to be executed
/// @param len - the byte length of 'buf'
void sql_parse(Transaction *xn, char const *buf, apr_size_t len);
#endif
/// - fetch SQL from the 'query' table by name and prepare statements
/// - prepared statements are recorded in Transaction
/// @todo - cache and pool prepared statements
/// @todo - also cache parameter indices (from database metadata)
/// @todo - bind parameters by position based on cached table info
void sql_get(Transaction *xn);
#if CH_DEV
/// - synthesize SQL statements when a request is made on a table or view
/// - not available in a production environment, and any arbitrary table or
///   view may be queried for arbitrary column data
/// - all access to data in a production environment should be via stored
///   procedures
/// - prepared statements are recorded in Transaction
/// @param xn - the transaction information associated with the HTTP request
void sql_synth(Transaction *xn);
#endif
/// - clean up the prepared statements in Transaction, which are currently
///   generated on each request
/// @param xn - the transaction information associated with the HTTP request
/// @todo - prepared statements should be cached and pooled across requests
///       - when pooling is implemented, this function will clean up parameter
///         bindings and return the prepared statements to the pool
void sql_cleanup(Transaction *xn);

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
