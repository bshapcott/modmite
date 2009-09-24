//____________________________________________________________________________
// mite - mod microlite - meta.h
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info

#ifndef meta_h
#define meta_h

/// @todo - replace with forward decls
#include "apr_pools.h"
#include "sqlite3.h"

void meta(sqlite3 *db, apr_pool_t *p);

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
