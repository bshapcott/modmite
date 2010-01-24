//____________________________________________________________________________
// mite - mod microlite - xml.c
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info
//
/// @file
///
/// - convert between XML and SQL
/// @todo - pretty print iff text/xml
/// @todo - supress comments for application/xml

#include "xml.h"
#include "db.h"
#include "sqlite3.h"
#include "expat.h"
#include "apr_reslist.h"
#include "apr_hash.h"
#include "apr_strings.h"

static void xml_start_element(void *u, const XML_Char *name,
						 const XML_Char **atts);
static void xml_end_element(void *u, const XML_Char *name);
static void xml_character_data(void *u, const XML_Char *s, int len);

//____________________________________________________________________________
void xml_parse(Transaction *xn, const char *buf, apr_size_t len) {
   XML_Parser parser = XML_ParserCreate(NULL);
   xn->data = parser;
   XML_SetElementHandler(parser, xml_start_element, xml_end_element);
   XML_SetCharacterDataHandler(parser, xml_character_data);
   XML_SetUserData(parser, xn);
   if (0 == XML_Parse(parser, buf, len, 1)) {
      // todo: proper error output
      ap_rprintf(xn->request, "<error line=\"%d\" column=\"%d\" index=\"%d\" code=\"%d\">%s</error>",
                 XML_GetCurrentLineNumber(parser),
                 XML_GetCurrentColumnNumber(parser),
                 XML_GetCurrentByteIndex(parser),
                 XML_GetErrorCode(parser),
                 XML_ErrorString(XML_GetErrorCode(parser)));
   }
   XML_ParserFree(parser);
   return;
}

//____________________________________________________________________________
/// - xpat callback for XML element start
/// - interpretation of element depends on parse level (nesting)
/// @param u    - user data for callback function, casts to Transaction
/// @param name - XML element name
/// @param atts - element's tag attributes
static void xml_start_element(void *u, const XML_Char *name,
                              const XML_Char **atts)
{
   Transaction *xn = (Transaction *)u;
   // todo: fixup bogus fixed alloc size
   char *parm = (char *)apr_palloc(xn->request->pool, 32);
   parm[0] = '$';
   // todo: support nesting
   switch (xn->level) {
   case 0:
      (*xn->ocb->root_start)(xn,
                             ((char **)xn->path->elts)[0]);
      break;
   case 1:
      // todo: bounds check
      strncpy(&xn->scratch[1], name, 252);
      db_start(xn);
      break;
   case 2:
      xn->name = name;
      db_sql_start(xn);
      db_bind_start(xn);
      {
         int ac = XML_GetSpecifiedAttributeCount((XML_Parser)xn->data)/2;
         int i;
         for (i = 0; i < ac; i++) {
            db_col_key(xn, atts[i*2], strlen(atts[i*2]));
            db_col_val(xn, atts[i*2+1], strlen(atts[i*2]));
         }
      }
      break;
   default:
      // - row level
      db_col_key(xn, name, strlen(name));
   }
   xn->level++;
   return;
}

//____________________________________________________________________________
/// - xpat callback for XML element end
/// @param u    - user data for callback function, casts to Transaction
/// @param name - XML element name
static void xml_end_element(void *u, const XML_Char *name) {
   Transaction *xn = (Transaction *)u;
   xn->level--;
   switch (xn->level) {
   case 0:
      (*xn->ocb->root_end)(xn);
      break;
   case 1:
      db_end(xn);
      break;
   case 2:
      db_bind_end(xn);
      db_sql_end(xn);
      apr_table_clear(xn->parameters);
      break;
   default:
      ;
   }
   return;
}

//____________________________________________________________________________
/// - xpat callback for character data (text)
/// - processed as a column value (the containing tag element identifies the
///   column)
/// @todo - check if xpat breaks up character data (like Xerces), and if so
///         handle multiple calls for the same character data block correctly
/// @param u   - user data for callback function, casts to Transaction
/// @param s   - the character data value
/// @param len - number of bytes in 's' (not characters)
static void xml_character_data(void *u, const XML_Char *s, int len) {
   // todo: support multi-part text
   db_col_val((Transaction *)u, s, len);
   return;
}

//____________________________________________________________________________
/// - start output of XML
/// - outputs a 'result' tag as the XML's root element
/// @param xn - the transaction information associated with the HTTP request
/// @param loc - location (Apache directory)
static void xml_root_start(Transaction *xn, char const *loc) {
   ap_set_content_type(xn->request, "text/xml;charset=ascii");
   ap_rprintf(xn->request, "<mite built=\"" __DATE__ " " __TIME__ "\" loc=\"%s\">",
              loc ? loc : "NONE");
   return;
}

//____________________________________________________________________________
/// - completes output of XML
/// - closes the root element
/// @param xn - the transaction information associated with the HTTP request
static void xml_root_end(Transaction *xn) {
   ap_rputs("</mite>", xn->request);
   return;
}

//____________________________________________________________________________
/// - start processing SQL for a particular database
/// - key is the name of the database, which is the database file's basename
///   (the filename with path and extension removed)
/// - starts an element with 'db' as the tag name
/// @param xn  - the transaction information associated with the HTTP request
/// @param dbn - 
static void xml_db_start(Transaction *xn, char const *dbn) {
   ap_rprintf(xn->request, "<db name=\"%s\">", dbn);
   return;
}

//____________________________________________________________________________
/// - completes processing for a particular database
/// - closes the 'db' element
/// @param xn - the transaction information associated with the HTTP request
static void xml_db_end(Transaction *xn) {
   ap_rputs("</db>", xn->request);
   return;
}

//____________________________________________________________________________
/// - starts processing SQL
/// - SQL is identified by name, and may be a table or view name (against
///   which actual SQL statements are generated or synthesized) or a key
///   to a table holding SQL statements
/// - starts a 'sql' tag element
/// @param xn - the transaction information associated with the HTTP request
static void xml_sql_start(Transaction *xn) {
   ap_rprintf(xn->request, "<sql name=\"%s\">", xn->name);
   return;
}

//____________________________________________________________________________
/// - completes SQL processing
/// - closes the 'sql' tag element
/// @param xn - the transaction information associated with the HTTP request
static void xml_sql_end(Transaction *xn) {
   ap_rputs("</sql>", xn->request);
   return;
}

//____________________________________________________________________________
/// - start processing a single SQL statement (INSERT, UPDATE, etc.)
/// - start a 'statement' tag element
/// @param xn - the transaction information associated with the HTTP request
static void xml_stmt_start(Transaction *xn) {
   ap_rputs("<statement>", xn->request);
   return;
}

//____________________________________________________________________________
/// - completes processing of a single SQL statement
/// - closes the 'statement' tag element
/// @param xn - the transaction information associated with the HTTP request
static void xml_stmt_end(Transaction *xn) {
   ap_rprintf(xn->request, "</statement>");
   return;
}

//____________________________________________________________________________
/// - starts processing a result row
/// - opens the 'row' tag, leaving it open so XML attributes may be added
/// @param xn - the transaction information associated with the HTTP request
/// @param pos - row position, first row is '1'
///            - used for formatting (e.g. delimiters)
static void xml_row_start(Transaction *xn, int pos) {
   ap_rprintf(xn->request, "<row");
   return;
}

//____________________________________________________________________________
/// - completes processing a result row
/// - outputs the closing 'row' element tag if the 'open' parameter is false,
//    otherwise closes the opening 'row' tag
/// @param xn   - the transaction information associated with the HTTP request
/// @param open - indicates whether the 'row' tag has been left open
///             - only happens if there are no nested elements
static void xml_row_end(Transaction *xn, char open) {
   if (open) {
      ap_rputs("/>", xn->request);
   } else {
      ap_rputs("</row>", xn->request);
   }
   return;
}

//____________________________________________________________________________
/// - outputs a database column with a simple type as an XML attribute
/// @param xn - the transaction information associated with the HTTP request
/// @param key - column name
/// @param val - column string value
/// @param pos - row position, first row is '1'
///            - used for formatting (e.g. delimiters)
/// @todo - entity encoding
static void xml_column(Transaction *xn, char const *key, unsigned char const *val,
                       int pos)
{
   ap_rprintf(xn->request, " %s=\"%s\"", key, val); 
   return;
}

//____________________________________________________________________________
/// - changes between processing columns with a simple type as attributes, and
///   columns with a complex type as child elements
/// @param xn - the transaction information associated with the HTTP request
static void xml_col_change(Transaction *xn) {
   ap_rputs(">", xn->request);
   return;
}

//____________________________________________________________________________
/// - outputs a column with a complex type as a child element
/// @param xn - the transaction information associated with the HTTP request
/// @param key - column name
/// @param val - column string value
/// @param pos - row position, first row is '1'
///            - used for formatting (e.g. delimiters)
/// @todo - entity encoding
static void xml_text(Transaction *xn, char const *key, unsigned char const *val,
                     int pos)
{
   ap_rprintf(xn->request, "<%s>%s</%s>", key, val, key); 
   return;
}

//____________________________________________________________________________
/// - outputs a comment as an XML comment block
/// - output is immediate
/// @param xn - the transaction information associated with the HTTP request
/// @param n  - comment
static void xml_comment(Transaction *xn, char const *n) {
#if CH_DEV
   ap_rprintf(xn->request, "<!-- %s -->", n);
#endif
   return;
}

//____________________________________________________________________________
/// - outputs the error as a comment immediately (i.e. not deferred)
/// @param xn - the transaction information associated with the HTTP request
/// @param n  - the error message
/// @todo - this is for development, need real error handling here for
///         production code
static void xml_error(Transaction *xn, char const *n) {
   ap_rprintf(xn->request, "<!-- ERROR: %s -->", n);
   return;
}

//____________________________________________________________________________
output_callbacks xml_ocb = {
   xml_root_start, xml_root_end,
   xml_db_start, xml_db_end, xml_sql_start, xml_sql_end,
   xml_stmt_start, xml_stmt_end, xml_row_start, xml_row_end,
   xml_column, xml_col_change, xml_text, xml_comment, xml_error};

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
