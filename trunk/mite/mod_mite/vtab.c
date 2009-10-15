//____________________________________________________________________________
// mite - mod microlite - vtab.c
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info
//
/// @file
///
/// - invoke JavaScript via sqlite3 virtual tables

#include "sqlite3.h"

#include <string.h>

static const sqlite3_module module;

//_____________________________________________________________________________
static int xConnect(sqlite3 *db, void *pAux, int argc, const char * const *argv,
                   sqlite3_vtab **ppVTab, char **c)
{
   int rc;
   /// @todo - no local bufs
  char buf[1024];
  *buf = '\0';
  strcat(buf, "CREATE TABLE ");
  strcat(buf, argv[1]);
  strcat(buf, ".");
  strcat(buf, argv[2]);
  strcat(buf, " (n NULL, i INTEGER, r REAL, t TEXT, x BLOB)");
  rc = sqlite3_declare_vtab(db, buf);
  *ppVTab = (sqlite3_vtab *)sqlite3_malloc(sizeof(sqlite3_vtab));
  (**ppVTab).pModule = &module;
  (**ppVTab).zErrMsg = 0;
  // - vtab instance data would be here
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
    info->aConstraintUsage[i].argvIndex = i + 1;
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

typedef struct
{
  sqlite3_vtab_cursor base;
  int row;
  double floor;
  double step;
} cursor;

//_____________________________________________________________________________
static int xOpen(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCursor)
{
  cursor *c = (cursor *)sqlite3_malloc(sizeof(cursor));
  c->row = 0;
  c->floor = .0f;
  c->step = 1.0f;
  *ppCursor = &c->base;
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xClose(cursor *c) {
  sqlite3_free(c);
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xFilter(cursor *c, int idxNum, const char *idxStr,
   int argc, sqlite3_value **argv)
{
  c->row = 0;
  if (argc > 0) {
    c->floor = sqlite3_value_double(argv[0]);
  }
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xNext(cursor *c)
{
  c->row++;
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xEof(cursor *c) {
  return c->row > 100 ? 1 : 0;
}

//_____________________________________________________________________________
static int xColumn(cursor *c, sqlite3_context *ctxt, int i) {
  /// @todo - stack buf
  char buf[16];
  char *txt;
  double v = c->floor + c->step * c->row;
  switch (i) {
    case 0:
      sqlite3_result_null(ctxt);
      break;
    case 1:
      sqlite3_result_int(ctxt, v);
      break;
    case 2:
      sqlite3_result_double(ctxt, v);
      break;
    case 3:
      sprintf(buf, "%f", v);
      txt = sqlite3_malloc(strlen(buf)+1);
      strcpy(txt, buf);
      sqlite3_result_text(ctxt, txt, -1, sqlite3_free);
      break;
    case 4:
      sqlite3_result_blob(ctxt, &v, sizeof(v), NULL);
      break;
    default:
      sqlite3_result_error(ctxt, "too many columns", SQLITE_ERROR);
      return SQLITE_ERROR;
  }
  return SQLITE_OK;
}

//_____________________________________________________________________________
static int xRowid(cursor *c, sqlite3_int64 *pRowid) {
  *pRowid = c->row;
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
  NULL, // update
  NULL, // begin
  NULL, // sync
  NULL, // commit
  NULL, // rollback
  NULL, // findFunction
  xRename
};

static void destructor(void *arg) {
  return;
}

void vtab(sqlite3 *db) {
  char *e;
  int rc = sqlite3_create_module_v2(db, "vtab", &module, NULL, destructor);
  rc = sqlite3_exec(db, "DROP TABLE literal", NULL, NULL, &e);
  rc = sqlite3_exec(db, "CREATE VIRTUAL TABLE literal USING vtab", NULL, NULL, &e);
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
