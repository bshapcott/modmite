//____________________________________________________________________________
// mite - mod microlite - json.h
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info

#ifndef json_h
#define json_h

#include "db.h"

/// - callback functions to generate JSON from database results
/// - frequently JSON output must be contained in an array context,
///   because JSON and JavaScript allow keys to appear multiple
///   times within a pair of braces, with each later occurence
///   simply overwriting the data of the prior occurence (so that
///   only the last occurence remains when JSON parsing completes)
/// - for event style processing (comparable to XML SAX), this isn't
///   a problem, but is for static representation (comparable to XML
///   DOM, because unlike the DOM, only one element of the same name
///   can exist at the same level at a time)
/// - so all the extra array processing semantics in the yajl callbacks
///   and in the JSON output are necessary
/// - nesting of the JSON output follows the nesting of invocations to
///   the output callbacks
output_callbacks json_ocb;

/// - parse an HTTP input stream as JSON
/// - the parser is progressive, and may be invoked multiple times
///   with part of the input
/// - yajl is (currently) used for parsing
/// - callback functions from yajl then drive parsing semantics
/// - parsing semantics are controlled by level (depth of JSON
///   structures) which parallel db_X functions which are driven
///   by parsing semantics
/// - the JSON in turn is mapped to database operations, and the results
///   of the database operations used to create the HTTP response
/// @param xn  - transaction containing data for a single HTTP request
/// @param buf - input buffer containing part of the HTTP input
/// @param len - number of characters in 'buf'
void json_parse(Transaction *xn, const char *buf, apr_size_t len);

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
