//____________________________________________________________________________
// mite - mod microlite - url.h
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info

#ifndef url_h
#define url_h

#include "db.h"

/// - simple database operations can be inferred directly from a URL
/// - URLs are input only, no output format is implemented
/// @param xn  - the transaction information associated with the HTTP request
void url_parse(Transaction *xn);
/// - parse out the path components
/// - the first part of the path is used by Apache to indicate this
///   module
/// - the path component immediately following identifies the database by
///   basename
/// - the next path component identifies the table or view name
/// - the next path component, if present, must be numeric and is converted
///   into the value of a bind parameter with the name 'id'
/// @param xn  - the transaction information associated with the HTTP request
/// @todo - support subdirectories for database file
/// @todo - ensure virtual server support
/// @todo - possibly support compound keys
/// @todo - support simple key with name other than 'id' (required full
///         implementation of metadata database support)
void url_parse_path(Transaction *xn);
/// - parse the query component of the URL for bind parameters
/// - for tables and views, the type of SQL operation depends on the
///   parameters; see the body of sql_synth for more detail
/// @param xn  - the transaction information associated with the HTTP request
/// @todo - check that precedence of cookie versus query session id makes sense
void url_parse_query(Transaction *xn);

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
