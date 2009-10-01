//____________________________________________________________________________
// mite - mod microlite - meta.c
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info
//
/// @file
///
/// - get database metadata via virtual tables
///
/// - get all the database metadata available at a particular location
/// - this is a compromise between RPC/CORBA style metadata (IDL) which
///   is not transmitted (usually -- although IFR/DSI/DII exist, they are
///   poorly supported by most vendors and cumbersome to use in practise)
///   and XML, which transmits the same metadata multiple times in the
///   same message
/// - instead, message metadata is made available on demand by the location
///   providing the data, and is derived from database metadata
/// - the metadata can be retrieved once by a client, either as a separate
///   operation before requesting data, or inline with the first message
///   requiring the data
/// - presumably the client will cache metadata locally for subsequent
///   requests
///
/// @todo - apr_pool instead of sqlite memory allocation
#include "meta.h"
#include "apr_strings.h"
#include "apr_tables.h"

#include <string.h>
#include <stdio.h>

//_____________________________________________________________________________
static const sqlite3_module module;

//_____________________________________________________________________________
typedef struct
{
  sqlite3_vtab_cursor base;
  apr_pool_t *pool;
  /// @todo - optimize repeated column data
  /// @todo - metadata vtabs should be cached
  apr_array_header_t *table;
  int r;
} cursor;

//_____________________________________________________________________________
typedef struct
{
   sqlite3_vtab base;
   sqlite3 *db;
   int (*filter)(cursor *, int, char const *, int, sqlite3_value **);
} vtab;

//_____________________________________________________________________________
static int xFilter_bind(cursor *c, int idxNum, char const *idxStr,
   int argc, sqlite3_value **argv);
static int xFilter_foreign(cursor *c, int idxNum, char const *idxStr,
   int argc, sqlite3_value **argv);
static int xFilter_sql(cursor *c, int idxNum, char const *idxStr,
   int argc, sqlite3_value **argv);
static int xFilter_table(cursor *c, int idxNum, char const *idxStr,
   int argc, sqlite3_value **argv);
static int xFilter_view(cursor *c, int idxNum, char const *idxStr,
   int argc, sqlite3_value **argv);

//_____________________________________________________________________________
static int xConnect(sqlite3 *db, void *pAux, int argc, char const * const *argv,
                   sqlite3_vtab **ppVTab, char **c)
{
   static struct {
      char const *name;
      char const *sql;
      int (*filter)(cursor *, int, char const *, int, sqlite3_value **);
   }
   q[] =
      {{"bind", "CREATE TABLE metabinding (name STRING, bind STRING)",
        xFilter_bind},
       {"foreign", "CREATE TABLE metaforeign (tbl STRING, id INTEGER, seq INTEGER, foreign_tbl STRING,"
        " c_from STRING, c_to STRING, on_update STRING, on_delete STRING, c_match STRING)",
        xFilter_foreign},
       {"sql", "CREATE TABLE metasql (name STRING, sid INTEGER, cid INTEGER, db STRING,"
        " tbl STRING, origin STRING, col STRING)",
        xFilter_sql},
       {"table", "CREATE TABLE metatable (tbl STRING, cid INTEGER, name STRING, type STRING,"
        " not_null INTEGER, dflt_value STRING, pk STRING)",
        xFilter_table},
       {"view", "CREATE TABLE metaview (name STRING, cid INTEGER, db STRING,"
        " tbl STRING, origin STRING, col STRING)",
        xFilter_view}
      };
  int i = 2;
  int rc;
  vtab *v;
  if (3 < argc) {
     for (i = 0; i < sizeof(q)/sizeof(*q); i++) {
        if (0 == apr_strnatcasecmp(q[i].name, argv[3])) {
           break;
        }
     }
  }
  if (i >= sizeof(q)/sizeof(*q)) {
     return SQLITE_ERROR;
  }
  /// @todo - db column
  rc = sqlite3_declare_vtab(db, q[i].sql);
  v = (vtab *)sqlite3_malloc(sizeof(vtab));
  *ppVTab = (sqlite3_vtab *)v;
  v->base.pModule = &module;
  v->db = db;
  v->filter = q[i].filter;
  v->base.zErrMsg = 0;
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xCreate(sqlite3 *db, void *pAux, int argc, const char * const * argv,
                  sqlite3_vtab **ppVTab, char **c)
{
  return xConnect(db, pAux, argc, argv, ppVTab, c);
}

//_____________________________________________________________________________
static int xBestIndex(sqlite3_vtab *pVTab, sqlite3_index_info *info) {
  int i;
  info->idxNum = 0;
  info->idxStr = 0;
  info->needToFreeIdxStr = 0;
  info->estimatedCost = 1.f;
  for (i = 0; i < info->nConstraint; i++) {
    info->aConstraintUsage[i].omit = 0;
  }
  return SQLITE_OK;
}

//_____________________________________________________________________________
// - destroy the *connection* to a virtual table
static int xDisconnect(sqlite3_vtab *pVTab) {
  sqlite3_free(pVTab);
  return SQLITE_OK;
}

//_____________________________________________________________________________
// - destroy the actual virtual table (DROP TABLE)
static int xDestroy(sqlite3_vtab *pVTab) {
  sqlite3_free(pVTab);
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor)
{
  cursor *c = (cursor *)sqlite3_malloc(sizeof(cursor));
  *ppCursor = &c->base;
  apr_pool_create(&c->pool, NULL);
  c->table = apr_array_make(c->pool, 12, sizeof(apr_array_header_t *));
  c->r = 0;
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xClose(cursor *c) {
  apr_pool_destroy(c->pool);
  sqlite3_free(c);
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xFilter(cursor *c, int idxNum, const char *idxStr,
   int argc, sqlite3_value **argv)
{
  return ((vtab *)c->base.pVtab)->filter(c, idxNum, idxStr, argc, argv);
}

//_____________________________________________________________________________
/// - metadata for the SQL statement bindings
/// - this will eventually support positional binds, so that in the same
///   way results can be passed in a message without column names, bindings
///   can be passed without parameter names (i.e. passed positionally)
/// - this differs from positional binds per SQL statement, because multiple
///   statements may be involved in a stored SQL
/// - a binding set covering the entire stored SQL is
///   synthesized from the bindings of individual SQL statements in the
///   procedure
/// - each binding is listed ONCE in the order in which it FIRST appears
static int xFilter_bind(cursor *c, int idxNum, const char *idxStr,
   int argc, sqlite3_value **argv)
{
  sqlite3_stmt *stmt[2];
  sqlite3_prepare_v2(((vtab *)c->base.pVtab)->db,
                     "SELECT name, stmt FROM query",
                     -1, &stmt[0], NULL);
  while (SQLITE_ROW == sqlite3_step(stmt[0])) {
    char const *name = apr_pstrdup(c->pool, sqlite3_column_text(stmt[0], 0));
    char const *sql = (char const *)sqlite3_column_text(stmt[0], 1);
    apr_table_t *dedup = apr_table_make(c->pool, 12);
    while (sql && *sql) {
      if (SQLITE_OK == sqlite3_prepare_v2
          (((vtab *)c->base.pVtab)->db, sql, -1, &stmt[1], &sql))
      {
        int n = sqlite3_bind_parameter_count(stmt[1]);
        int i;
        for (i = 0; i < n; i++) {
          char const *bind = apr_pstrdup(c->pool, sqlite3_bind_parameter_name(stmt[1], i+1));
          if (!apr_table_get(dedup, bind)) {
            apr_array_header_t *row;
            apr_table_setn(dedup, bind, (char *)(i+1));
            row = apr_array_make(c->pool, 2, sizeof(char *));
            APR_ARRAY_PUSH(c->table, apr_array_header_t *) = row;
            APR_ARRAY_IDX(row, 0, char *) = (char *)name;
            APR_ARRAY_IDX(row, 1, char *) = (char *)bind;
          }
        }
      }
    }
  }
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int meta_query(cursor *c, int idxNum, const char *idxStr,
                      int argc, sqlite3_value **argv, char const *query)
{
   /// @todo - replace
   char buf[1024];
   sqlite3_stmt *stmt[2];
   sqlite3_prepare_v2(((vtab *)c->base.pVtab)->db,
      "SELECT name FROM sqlite_master WHERE type = 'table'",
      -1, &stmt[0], NULL);
   while (SQLITE_ROW == sqlite3_step(stmt[0])) {
      int i, n;
      apr_array_header_t *row;
      // - only autoinc is missing from sqlite3_table_column_metadata
      //   vs. PRAGMA table_info
      char const *name = apr_pstrdup(c->pool, (char const *)sqlite3_column_text(stmt[0], 0));
      sprintf(buf, query, name);
      sqlite3_prepare_v2(((vtab *)c->base.pVtab)->db, buf, -1, &stmt[1], NULL);
      while (SQLITE_ROW == sqlite3_step(stmt[1])) {
         n = sqlite3_column_count(stmt[1]);
         row = apr_array_make(c->pool, n, sizeof(char *));
         APR_ARRAY_PUSH(c->table, apr_array_header_t *) = row;
         APR_ARRAY_PUSH(row, char const *) = name;
         for (i = 0; i < n; i++) {
            char const *txt = sqlite3_column_text(stmt[1], i);
            APR_ARRAY_PUSH(row, char const *) = apr_pstrdup(c->pool, txt);
         }
      }
   }
   return SQLITE_OK;
}

//_____________________________________________________________________________
static int xFilter_foreign(cursor *c, int idxNum, const char *idxStr,
   int argc, sqlite3_value **argv)
{
   return meta_query(c, idxNum, idxStr, argc, argv,
      "PRAGMA foreign_key_list(%s)");
}

//_____________________________________________________________________________
static int xFilter_sql(cursor *c, int idxNum, const char *idxStr,
   int argc, sqlite3_value **argv)
{
  sqlite3_stmt *stmt[2];
  sqlite3_prepare_v2(((vtab *)c->base.pVtab)->db,
    "SELECT name, stmt FROM query",
    -1, &stmt[0], NULL);
  while (SQLITE_ROW == sqlite3_step(stmt[0])) {
    apr_array_header_t *row;
    char const *name = apr_pstrdup(c->pool, (char const *)sqlite3_column_text(stmt[0], 0));
    char const *sql = (char const *)sqlite3_column_text(stmt[0], 1);
    int s = 0;
    while (sql && *sql) {
      if (SQLITE_OK == sqlite3_prepare_v2
        (((vtab *)c->base.pVtab)->db, sql, -1, &stmt[1], &sql))
      {
        /// \todo - replace stack buf
        char sbuf[16];
        int i, n = sqlite3_column_count(stmt[1]);
        char *ss = apr_pstrdup(c->pool, itoa(s, sbuf, 10));
        for (i = 0; i < n; i++) {
          row = apr_array_make(c->pool, n, sizeof(char *));
          APR_ARRAY_PUSH(c->table, apr_array_header_t *) = row;
          APR_ARRAY_PUSH(row, char const *) = name;
          APR_ARRAY_PUSH(row, char const *) = ss;
          // \todo - small opt by caching strings in array
          APR_ARRAY_PUSH(row, char const *) =
            apr_pstrdup(c->pool, itoa(i, sbuf, 10));
          APR_ARRAY_PUSH(row, char const *) =
            apr_pstrdup(c->pool, sqlite3_column_database_name(stmt[1], i));
          APR_ARRAY_PUSH(row, char const *) =
            apr_pstrdup(c->pool, sqlite3_column_table_name(stmt[1], i));
          APR_ARRAY_PUSH(row, char const *) =
            apr_pstrdup(c->pool, sqlite3_column_origin_name(stmt[1], i));
          APR_ARRAY_PUSH(row, char const *) =
            apr_pstrdup(c->pool, sqlite3_column_name(stmt[1], i));
        }
      }
      s++;
    }
  }
  return SQLITE_OK;
}

//_____________________________________________________________________________
/// - process metadata for a table
static int xFilter_table(cursor *c, int idxNum, const char *idxStr,
   int argc, sqlite3_value **argv)
{
   return meta_query(c, idxNum, idxStr, argc, argv,
      "PRAGMA table_info(%s)");
}

//_____________________________________________________________________________
/// - process metadata for a database view
static int xFilter_view(cursor *c, int idxNum, const char *idxStr,
   int argc, sqlite3_value **argv)
{
  char buf[1024];
  sqlite3_stmt *stmt[2];
  sqlite3_prepare_v2(((vtab *)c->base.pVtab)->db,
    "SELECT name FROM sqlite_master WHERE type = 'view'",
    -1, &stmt[0], NULL);
  while (SQLITE_ROW == sqlite3_step(stmt[0])) {
    apr_array_header_t *row;
    char const *name = apr_pstrdup(c->pool, (char const *)sqlite3_column_text(stmt[0], 0));
    strcpy(buf, "SELECT *, 1 AS is_view FROM ");
    strcat(buf, name);
    if (SQLITE_OK == sqlite3_prepare_v2
      (((vtab *)c->base.pVtab)->db, buf, -1, &stmt[1], NULL))
    {
      char sbuf[16];
      int i, n = sqlite3_column_count(stmt[1]);
      for (i = 0; i < n; i++) {
        row = apr_array_make(c->pool, n, sizeof(char *));
        APR_ARRAY_PUSH(c->table, apr_array_header_t *) = row;
        APR_ARRAY_PUSH(row, char const *) = name;
        // \todo - small opt by caching strings in array
        APR_ARRAY_PUSH(row, char const *) =
          apr_pstrdup(c->pool, itoa(i, sbuf, 10));
        APR_ARRAY_PUSH(row, char const *) =
          apr_pstrdup(c->pool, sqlite3_column_database_name(stmt[1], i));
        APR_ARRAY_PUSH(row, char const *) =
          apr_pstrdup(c->pool, sqlite3_column_table_name(stmt[1], i));
        APR_ARRAY_PUSH(row, char const *) =
          apr_pstrdup(c->pool, sqlite3_column_origin_name(stmt[1], i));
        APR_ARRAY_PUSH(row, char const *) =
          apr_pstrdup(c->pool, sqlite3_column_name(stmt[1], i));
      }
    }
  }
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xNext(cursor *c) {
  c->r++;
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xEof(cursor *c) {
  return c->r >= c->table->nelts;
}

//_____________________________________________________________________________
static int xColumn(cursor *c, sqlite3_context *ctxt, int i) {
  apr_array_header_t *row = APR_ARRAY_IDX(c->table, c->r, apr_array_header_t *);
  char const *txt = APR_ARRAY_IDX(row, i, char const *);
  sqlite3_result_text(ctxt, txt, -1, NULL);
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xRowid(cursor *c, sqlite3_int64 *pRowid) {
  *pRowid = c->r;
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xRename(vtab *pVtab, char const *zNew) {
  return SQLITE_OK;
}

//_____________________________________________________________________________
static const sqlite3_module module = {
  1,
  xCreate,
  xConnect,
  xBestIndex,
  xDisconnect,
  xDestroy,
  xOpen,
  // - casting the func for arg types here saves casting within the func
  (int (*)(sqlite3_vtab_cursor *))xClose,
  (int (*)(sqlite3_vtab_cursor *, int, char const *, int, sqlite3_value **))xFilter,
  (int (*)(sqlite3_vtab_cursor *))xNext,
  (int (*)(sqlite3_vtab_cursor *))xEof,
  (int (*)(sqlite3_vtab_cursor *, sqlite3_context *, int))xColumn,
  (int (*)(sqlite3_vtab_cursor *, sqlite3_int64 *))xRowid,
  NULL, // xUpdate
  NULL, // xBegin
  NULL, // xSync
  NULL, // xCommit
  NULL, // xRollback
  NULL, // xFindFunction
  (int (*)(sqlite3_vtab *, char const *))xRename
};

//_____________________________________________________________________________
static void dtor(void *arg) {
  return;
}

//_____________________________________________________________________________
void meta(sqlite3 *db, apr_pool_t *p) {
   static char *m[] = {
      "DROP TABLE metasql",     "CREATE VIRTUAL TABLE metasql USING meta(sql)",
      "DROP TABLE metabinding", "CREATE VIRTUAL TABLE metabinding USING meta(bind)",
      "DROP TABLE metatable",   "CREATE VIRTUAL TABLE metatable USING meta(table)",
      "DROP TABLE metaforeign", "CREATE VIRTUAL TABLE metaforeign USING meta(foreign)",
      "DROP TABLE metaview",    "CREATE VIRTUAL TABLE metaview USING meta(view)",
      NULL
   };
   char *e, **s;
   int rc = sqlite3_create_module_v2(db, "meta", &module, p, dtor);
   for (s = m; *s != NULL; s++) {
      rc = sqlite3_exec(db, *s, NULL, NULL, &e);
   }
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
