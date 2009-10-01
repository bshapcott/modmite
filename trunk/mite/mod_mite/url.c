//____________________________________________________________________________
// mite - mod microlite - url.c
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info

#include "http.h"
#include "sql.h"
#include "url.h"
#include "meta.h"
#include "apr_strings.h"

//____________________________________________________________________________
void url_parse(Transaction *xn) {
   int t;
   url_parse_path(xn);
   if (xn->request->method_number == M_POST) {
      url_parse_query(xn);
      return;
   }
   if (3 > xn->path->nelts) {
      // @todo - vtab for db config
      return;
   }
   (*xn->ocb->root_start)(xn, ((char **)xn->path->elts)[0]);
   strncpy(&xn->scratch[1], ((char **)xn->path->elts)[1], 252);
   db_start(xn);
   if (!xn->db) {
      db_end(xn);
      return;
   }
   url_parse_query(xn);
   for (t = 2; t < xn->path->nelts; t++) {
      xn->name = ((char **)xn->path->elts)[t];
      db_sql_start(xn);
      db_bind_start(xn);
      if (!apr_is_empty_array(xn->statements)) {
         apr_array_header_t const *parms = apr_table_elts(xn->parameters);
         apr_table_entry_t *te = (apr_table_entry_t *)parms->elts;
         int i;
         for (i = 0; i < parms->nelts; i++) {
            db_col_key(xn, te[i].key, strlen(te[i].key));
            db_col_val(xn, te[i].val, strlen(te[i].val));
         }
      }
      db_bind_end(xn);
      db_sql_end(xn);
   }
   db_end(xn);
   (*xn->ocb->root_end)(xn);
   return;
}

//____________________________________________________________________________
/// - parse the path into components
void url_parse_path(Transaction *xn) {
   char *remaining, *token;
   for (token =  apr_strtok((char *)
         (apr_pstrdup(xn->request->pool, xn->request->parsed_uri.path)),
         "/", &remaining); token; token = apr_strtok(NULL, "/", &remaining))
   {
      if (isdigit(*token)) {
         apr_table_setn(xn->parameters, "id", token);
      } else {
         *(char **)apr_array_push(xn->path) = token;
      }
   }
   return;
}

//____________________________________________________________________________
/// - parse the query string into key/value pairs
void url_parse_query(Transaction *xn) {
   if (xn->request->parsed_uri.query) {
      char *remaining, *token;
      for (token = apr_strtok((char *)
            (apr_pstrdup(xn->request->pool, xn->request->parsed_uri.query)),
            "&", &remaining); token; token = apr_strtok(NULL, "&", &remaining))
      {
         char *k, *v;
         k = apr_strtok(token, "=", &v);
         apr_table_addn(xn->parameters, k, v);
      }
   }
   return;
}
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
