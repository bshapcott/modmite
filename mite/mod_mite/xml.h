//____________________________________________________________________________
// mite - mod microlite - xml.h
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info

#ifndef xml_h
#define xml_h

#include "db.h"
#include "httpd.h"
#include "http_protocol.h"
#include "http_config.h"

/// - set of callback functions to generate XML from database results
/// - nesting of the XML output follows the nesting of invocations to
///   the output callbacks
output_callbacks xml_ocb;

/// - parse an HTTP input stream as XML
/// - requires a progressive parser (currently xpat, which is distributed with
///   Apache)
/// - the XML in turn is mapped to database operations, and the results
///   of the database operations used to create the HTTP response
/// - XML permits the same tag element to appear multiple times at the same
///   level
void xml_parse(Transaction *xn, const char *buf, apr_size_t len);

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
