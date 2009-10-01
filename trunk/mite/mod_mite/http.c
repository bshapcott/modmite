//____________________________________________________________________________
// mite - mod microlite - http.c
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info

#include "http.h"
#include "xml.h"
#include "json.h"
#include "sql.h"
#include "url.h"
#include "httpd.h"
#include "http_protocol.h"
#include "http_config.h"
#include "sqlite3.h"
#include "apr_strings.h"
#include "apr_time.h"

//____________________________________________________________________________
static void http_hooks(apr_pool_t* pool);
static void *http_create_server_conf(apr_pool_t *pool, server_rec *s);

//____________________________________________________________________________
/// The Apache Modules Book p. 239
module AP_MODULE_DECLARE_DATA mite_module = {
   STANDARD20_MODULE_STUFF,
   NULL, /* my_creat_dir_conf */
   NULL, /* my_merge_dir_conf */
   http_create_server_conf,
   NULL, /* my_merge_server_conf */
   NULL, /* my_cmds */
   http_hooks
};

//____________________________________________________________________________
/// Clean up the database resources.
/// Not yet implemented.
/// @param t - user data supplied to callback
static apr_status_t db_cleanup(void *t) {
   // db_pool_finalize();
   return APR_SUCCESS;
}

//____________________________________________________________________________
/// Create per-server module configuration.  This will be called TWICE -- once
/// when the configuration file is read, and again when the server is created.
/// @param pool - server lifetime memory allocation pool
/// @param s    - server configuration record
static void *http_create_server_conf(apr_pool_t *pool, server_rec *s) {
   my_server_config *config;
   config = (my_server_config *)apr_palloc(pool, sizeof(my_server_config));
   // db_pool_init();
   apr_pool_cleanup_register(pool, NULL, db_cleanup, NULL);
   return config;
}

//____________________________________________________________________________
/// Per-child initialization.  Apache MPM may spawn multiple children to
/// handle requests.
/// @param p - child lifetime memory allocation pool
/// @param s - server configuration record
static void http_initialize_child(apr_pool_t *p, server_rec *s) {
   my_server_config *config = (my_server_config *)
      (ap_get_module_config(s->module_config, &mite_module));
   return;
}

/// - granularity for reading input stream
#define BUFLEN 8192

//____________________________________________________________________________
/// Parse the input stream as JSON, XML or SQL.  Note that a progressive
/// parser is requred.
/// @param xn - transaction information
static void http_parse_content(Transaction *xn) {
   apr_status_t status;
   int end = 0;
   apr_size_t bytes;
   char const *buf;
   apr_bucket *b;
   apr_bucket_brigade *bb;
   int has_input = 0;
   void (*parser)(Transaction *, char const *, apr_size_t) = NULL;
   const char *hdr = apr_table_get(xn->request->headers_in, "Content-Length");
   if (hdr) {
      has_input = 1;
   }
   hdr = apr_table_get(xn->request->headers_in, "Transfer-Encoding");
   if (hdr) {
      if (strcasecmp(hdr, "chunked") == 0) {
         has_input = 1;
      } else {
         (*xn->ocb->error)(xn, "unsupported transfer encoding");
         return;
      }
   }
   // todo: handle form input
   if (hdr = apr_table_get(xn->request->headers_in, "Content-Type")) {
      char *content_type, *charset;
      content_type = apr_strtok(apr_pstrdup(xn->request->pool, hdr),
                                ";", &charset);
      if (!strcmp("application/json", content_type)
          ||
          !strcmp("text/json", content_type))
         {
            parser = json_parse;
         } else if (!strcmp("application/xml", content_type)
                    ||
			!strcmp("text/xml", content_type))
      {
         parser = xml_parse;
#if CH_DEV
      } else if (!strcmp("application/x-sql", content_type)) {
         // - there is no IANA registered MIME type for SQL, so
         //   use an x-token
         // todo: use content size in the headers to create a buf
         //       large enough for content body, because SQL parser
         //       is NOT progressive
         parser = sql_parse;
#endif
      }
   }
   if (has_input && parser) {
      bb = apr_brigade_create(xn->request->pool, xn->request->connection->bucket_alloc);
      do {
         status = ap_get_brigade(xn->request->input_filters, bb,
                                 AP_MODE_READBYTES, APR_BLOCK_READ, BUFLEN);
         if (APR_SUCCESS == status) {
            for (b = APR_BRIGADE_FIRST(bb); b!= APR_BRIGADE_SENTINEL(bb); b = APR_BUCKET_NEXT(b)) {
               if (APR_BUCKET_IS_EOS(b)) {
                  end = 1;
                  break;
               } else if (APR_BUCKET_IS_METADATA(b)) {
                  continue;
               }
               status = apr_bucket_read(b, &buf, &bytes, APR_BLOCK_READ);
               (*parser)(xn, buf, bytes);
            }
         }
         apr_brigade_cleanup(bb);
      } while (!end && status == APR_SUCCESS);
      if (APR_SUCCESS != status) {
         (*xn->ocb->error)(xn, "error reading request body");
      }
   }
   return;
}

//____________________________________________________________________________
/// - only the session cookie is parsed out
/// @todo - alternate handling if cookies disabled on browser, session should
///         be taken from the URL, probably as a query parameter
/// @todo - is this needed now that request vtab is implemented?
static void http_parse_cookie(Transaction *xn) {
   const char *cookie = apr_table_get(xn->request->headers_in, "Cookie");
   if (cookie) {
      char *remaining, *token;
      for (token = apr_strtok((char *)apr_pstrdup(xn->request->pool, cookie),
                              " ;", &remaining);
           token; token = apr_strtok(NULL, "; ", &remaining))
      {

      }
   }
   return;
}

//____________________________________________________________________________
/// - set the session cookie
/// @todo obsfucate or encrypt session id in cookie to prevent trivial
///       session hijacking
/// @todo handle browser disabled cookies use case (maybe client side?)
/// @todo throttle expires refresh
/// @todo - should use cookie vtab
static void http_cookie_set(Transaction *xn) {
#if 0
  char *cookie;
   apr_time_t t = apr_time_now() + (apr_time_t)7 * 24 * 60 * 60 * 1000000;
   char *date = (char *)apr_palloc(xn->request->pool, APR_RFC822_DATE_LEN);
   apr_rfc822_date(date, t);
   cookie = apr_pstrcat(xn->request->pool, "session=", xn->session,
                              "; path=/; expires=", date, NULL);
   apr_table_set(xn->request->headers_out, "Set-Cookie", cookie);
#endif
   return;
}

//____________________________________________________________________________
/// - link to the Apache server
/// - handler must be registered against a URL pattern in httpd.conf
/// @param r - HTTP reqest record
static int http_handler(request_rec* r) {
	Transaction *xn;
	if ( !r->handler || strcmp(r->handler, "mite_module") ) {
      return DECLINED;
   }
   if (r->method_number != M_POST && r->method_number != M_GET) {
      return HTTP_METHOD_NOT_ALLOWED;
   }
   xn = db_transaction_new(r);
   http_parse_cookie(xn);
   url_parse(xn);
   http_cookie_set(xn); // - after url_parse because session may be in query
   http_parse_content(xn);
   return OK;
}

//____________________________________________________________________________
/// AP hooks are declared using the AP_DECLARE_HOOK macro, which uses the
/// APR_DECLARE_EXTERNAL_HOOK macro.  It's possible to define the hook
/// directly or using the latter APR macro, but that's not done anywhere in
/// the 2.2.4 code base.  Search for the former macro to find all available
/// hooks.
/// @param pool - the Apache core memory management pool
static void http_hooks(apr_pool_t* pool) {
   ap_hook_child_init(http_initialize_child, NULL, NULL, APR_HOOK_MIDDLE);
#if 0
   ap_hook_post_config(my_post_config, NULL, NULL, APR_HOOK_MIDDLE);
#endif
   ap_hook_handler(http_handler, NULL, NULL, APR_HOOK_MIDDLE);
   return;
}

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
