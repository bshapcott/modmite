//____________________________________________________________________________
// mite - mod microlite - sql.c
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info

#include "db.h"
#include "sql.h"
#include "meta.h"
#include "sqlite3.h"
#include "apr_strings.h"
#include "apr_tables.h"
#include <assert.h>

//____________________________________________________________________________
#if CH_DEV
void sql_parse(Transaction *xn, char const *buf, apr_size_t len) {
   sqlite3_stmt *s = NULL;
   char const *sql;
   if (xn->path->nelts < 2) {
      (*xn->ocb->error)(xn, "database must be specified in path");
      return;
   }
   strncpy(&xn->scratch[1], ((char **)xn->path->elts)[1], 252);
   (*xn->ocb->root_start)(xn, &xn->scratch[1]);
   db_start(xn);
   // \todo - maybe can just ref script from Transaction before calling
   //         into db_sql_start
   sql = apr_pstrmemdup(xn->request->pool, buf, len);
   while (sql && *sql) {
      if (sqlite3_prepare_v2(xn->db, sql, -1, &s, &sql) == SQLITE_OK) {
         // - empty SQL returns OK, but no statement is prepared
         if (NULL != s) {
            *(sqlite3_stmt **)apr_array_push(xn->statements) = s;
            xn->synthetic = 1;
         }
      } else {
         db_error(xn, sqlite3_errmsg(xn->db));
         sql_cleanup(xn);
         break;
      }
   }
   if (!apr_is_empty_array(xn->statements)) {
      xn->name = "ad_hoc";
      db_sql_start(xn);
      db_bind_start(xn);
      db_bind_end(xn);
      db_sql_end(xn);
   }
   db_end(xn);
   (*xn->ocb->root_end)(xn);
   return;
}
#endif

//____________________________________________________________________________
void sql_get(Transaction *xn) {
   sqlite3_stmt *stmt = NULL;
   int rc = sqlite3_prepare_v2(xn->db, "SELECT stmt FROM query WHERE name = ?",
                               -1, &stmt, NULL);
   sqlite3_bind_text(stmt, 1, xn->name, -1, SQLITE_TRANSIENT);
   while (sqlite3_step(stmt) == SQLITE_ROW) {
      char const *sql =
         (const char *)sqlite3_column_text(stmt, 0);
      while (sql && *sql) {
         sqlite3_stmt *s = NULL;
         if (sqlite3_prepare_v2(xn->db, sql, -1, &s, &sql) == SQLITE_OK) {
            // - empty SQL returns OK, but no statement is prepared
            if (NULL != s) {
               *(sqlite3_stmt **)apr_array_push(xn->statements) = s;
               xn->synthetic = 0;
            }
         } else {
            (*xn->ocb->error)(xn, "bad SQL");
            sql_cleanup(xn);
            break;
         }
      }
   }
   return;
}

//____________________________________________________________________________
// todo: error handling
#if CH_DEV
void sql_synth(Transaction *xn) {
   int i, pk, rc;
   char *scratch;
   char first;
   char bind = 0;
   char key = 0, // - at least one primary key column specified with value
      nonkey = 0, // - at least one non-key column specified
      keyless = 0; // - at least one primary key column specified without value
   sqlite3_stmt **sp;
   apr_table_entry_t *te;
   xn->synthetic = 1;
   te = (apr_table_entry_t *)(apr_table_elts(xn->parameters)->elts);
   for (i = 0; i < apr_table_elts(xn->parameters)->nelts; i++) {
      if (te[i].key) {
         int rc = sqlite3_table_column_metadata(xn->db, NULL, xn->name,
                                                te[i].key, NULL, NULL, NULL, &pk, NULL);
         if (pk) {
            key |= SQLITE_OK == rc;
         } else {
            nonkey |= SQLITE_OK == rc;
         }
         // - key column specified, but no value
         keyless |=
            SQLITE_OK == rc && pk && !(te[i].val && *te[i].val);
         if (keyless || (key && nonkey)) {
            break;
         }
      }
   }
   scratch = &xn->scratch[1];
   sp = (sqlite3_stmt **)apr_array_push(xn->statements);
   if (keyless) {
      sprintf(scratch, "REPLACE INTO %s ", xn->name);
      for (i = 0; i < apr_table_elts(xn->parameters)->nelts; i++) {
         if (te[i].key) {
            rc = sqlite3_table_column_metadata(xn->db, NULL, xn->name,
                                               te[i].key, NULL, NULL, NULL, &pk, NULL);
            if (SQLITE_OK == rc && !(pk && !(te[i].val && *te[i].val))) {
               strcat(scratch, bind ? ", " : "(");
               bind = 1;
               strcat(scratch, te[i].key);
            }
         }
      }
      if (bind) {
         strcat(scratch, ") VALUES (");
         bind = 0;
         for (i = 0; i < apr_table_elts(xn->parameters)->nelts; i++) {
            if (te[i].key) {
               rc = sqlite3_table_column_metadata(xn->db, NULL, xn->name,
                                                  te[i].key, NULL, NULL, NULL, &pk, NULL);
               if (SQLITE_OK == rc && !(pk && !(te[i].val && *te[i].val))) {
                  if (bind) {
                     strcat(scratch, ", ");
                  }
                  bind = 1;
                  strcat(scratch, "$");
                  strcat(scratch, te[i].key);
               }
            }
         }
         strcat(scratch, ")");
      } else {
         strcat(scratch, "DEFAULT VALUES");
      }
   } else {
      if (key && nonkey) {
         sprintf(scratch, "UPDATE %s SET ", xn->name);
         first = 1;
         for (i = 0; i < apr_table_elts(xn->parameters)->nelts; i++) {
            if (te[i].key) {
               rc = sqlite3_table_column_metadata(xn->db, NULL, xn->name,
                                                  te[i].key, NULL, NULL, NULL, &pk, NULL);
               if (SQLITE_OK == rc && !pk) {
                  if (!first) {
                     strcat(scratch, ", ");
                     first = 0;
                  }
                  strcat(scratch, te[i].key);
                  strcat(scratch, " = $");
                  strcat(scratch, te[i].key);
               }
            }
         }
      } else {
         if (scratch == xn->name) {
            // todo: could just shift name up and copy in literal
            xn->name = apr_pstrdup(xn->request->pool, scratch);
         }
         sprintf(scratch, "SELECT * FROM %s", xn->name);
      }
      if (key || nonkey) {
         char first = 1;
         int i;
         for (i = 0; i < apr_table_elts(xn->parameters)->nelts; i++) {
            if (te[i].key) {
               rc = sqlite3_table_column_metadata(xn->db, NULL, xn->name,
                                                  te[i].key, NULL, NULL, NULL, &pk, NULL);
               if (SQLITE_OK == rc && !(!pk && key && nonkey)) {
                  strcat(scratch, first ? " WHERE " : " AND ");
                  strcat(scratch, te[i].key);
                  strcat(scratch, " = $");
                  strcat(scratch, te[i].key);
                  first = 0;
               }
            }
         }
      }
   }
   if (SQLITE_OK != sqlite3_prepare_v2(xn->db, scratch, -1, sp, NULL)) {
      apr_array_pop(xn->statements);
      db_error(xn, sqlite3_errmsg(xn->db));
   } else if (key || nonkey) {
      // - perform late binding using the parameters used to
      //   synthesize the SQL
      int i;
      for (i = 0; i < apr_table_elts(xn->parameters)->nelts; i++) {
         db_col_key(xn, te[i].key, strlen(te[i].key));
         db_col_val(xn, te[i].val, te[i].val ? strlen(te[i].val) : 0);
      }
   }
   return;
}
#endif

//____________________________________________________________________________
void sql_cleanup(Transaction *xn) {
   int i;
   for (i = 0; i < xn->statements->nelts; i++) {
      sqlite3_stmt *statement = ((sqlite3_stmt **)xn->statements->elts)[i];
      sqlite3_finalize(statement);
   }
   apr_array_clear(xn->statements);
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
