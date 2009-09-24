//____________________________________________________________________________
// mite - mod microlite - request.c
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info
//
/// @file
///
/// - access request header elements as a virtual table
/// \todo - when db is pooled, need to know current user to get transaction
///         info (make ptr in db pool entry, set when owner gets resource,
///         unset when resource released)
/// \todo - is env access required?


#include <httpd.h>
#include <sqlite3.h>
#include <string.h>
#include "db.h"

static const sqlite3_module module;

//_____________________________________________________________________________
struct cursor
{
  sqlite3_vtab_cursor base;
  int r;
};

//_____________________________________________________________________________
struct vtab
{
  sqlite3_vtab base;
  Transaction *xn;
};

//_____________________________________________________________________________
static int xConnect(sqlite3 *db, void *pAux, int argc, const char * const *argv,
                   sqlite3_vtab **ppVTab, char **c)
{
  int rc = sqlite3_declare_vtab(db, "CREATE TABLE request (key STRING, value STRING)");
  struct vtab *v = (struct vtab *)sqlite3_malloc(sizeof(struct vtab));
  *ppVTab = (sqlite3_vtab *)v;
  (**ppVTab).pModule = &module;
  (**ppVTab).zErrMsg = 0;
  v->xn = (Transaction *)pAux;
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xCreate(sqlite3 *db, void *pAux, int argc, const char * const * argv,
                  sqlite3_vtab **ppVTab, char **c)
{
  // - module instance data here
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
  // - vtab instance only
  sqlite3_free(pVTab);
  return SQLITE_OK;
}

//_____________________________________________________________________________
// - destroy the actual virtual table (DROP TABLE)
static int xDestroy(sqlite3_vtab *pVTab) {
  // - table implementation
  sqlite3_free(pVTab);
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor)
{
  struct cursor *c = (struct cursor *)sqlite3_malloc(sizeof(struct cursor));
  c->r = 0;
  *ppCursor = &c->base;
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xClose(struct cursor *c) {
  sqlite3_free(c);
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xFilter(struct cursor *c, int idxNum, const char *idxStr,
   int argc, sqlite3_value **argv)
{
  c->r = 0;
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xNext(struct cursor *c)
{
  c->r++;
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xEof(struct cursor *c) {
  struct vtab *v = (struct vtab *)c->base.pVtab;
  apr_array_header_t const *a = apr_table_elts(v->xn->request->headers_in);
  return c->r >= a->nelts;
}

//_____________________________________________________________________________
static int xColumn(struct cursor *c, sqlite3_context *ctxt, int i) {
  struct vtab *v = (struct vtab *)c->base.pVtab;
  apr_array_header_t const *ah = apr_table_elts(v->xn->request->headers_in);
  apr_table_entry_t *te = (apr_table_entry_t *)ah->elts;
  switch (i) {
    case 0:
      sqlite3_result_text(ctxt, te[c->r].key, -1, NULL);
      break;
    case 1:
      sqlite3_result_text(ctxt, te[c->r].val, -1, NULL);
      break;
    default:
      return SQLITE_ERROR;
  }
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xRowid(struct cursor *c, sqlite3_int64 *pRowid) {
  *pRowid = c->r;
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xUpdate(sqlite3_vtab *pVTab, int i, sqlite3_value **val,
   sqlite3_int64 *v)
{
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xBegin(sqlite3_vtab *pVTab) {
  return 0;
}

//_____________________________________________________________________________
static int xSync(sqlite3_vtab *pVTab) {
  return 0;
}

//_____________________________________________________________________________
static int xCommit(sqlite3_vtab *pVTab) {
	return 0;
}

//_____________________________________________________________________________
static int xRollback(sqlite3_vtab *pVTab) {
  return 0;
}

//_____________________________________________________________________________
static int xFindFunction(sqlite3_vtab *pVtab, int nArg, const char *zName,
   void (**pxFunc)(sqlite3_context*,int,sqlite3_value**),
   void **ppArg)
{
  return 0;
}

//_____________________________________________________________________________
static int xRename(sqlite3_vtab *pVtab, const char *zNew) {
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
  xRename
};

static void destructor(void *arg) {
  return;
}

void request(Transaction *xn) {
  char *e;
  int rc = sqlite3_create_module_v2(xn->db, "vrq", &module, xn, destructor);
  rc = sqlite3_exec(xn->db, "DROP TABLE request", NULL, NULL, &e);
  rc = sqlite3_exec(xn->db, "CREATE VIRTUAL TABLE request USING vrq", NULL, NULL, &e);
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
