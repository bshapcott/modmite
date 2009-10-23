//____________________________________________________________________________
// mite - mod microlite - http.h
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info

#ifndef http_h
#define http_h

#include "sqlite3.h"
#include "apr_tables.h"
#include "options.h"

//____________________________________________________________________________
/// - per server configuation for this Apache module
typedef struct my_server_config {
	char dummy;
} my_server_config;

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