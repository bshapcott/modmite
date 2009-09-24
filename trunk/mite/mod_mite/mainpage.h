//____________________________________________________________________________
// mite - mod microlite - mainpage.h
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info
//
// - this file contains no source code, only documentation for doxygen
//
/// @mainpage Mod MIcroliTE (mite)
///
/// This is a <B>work in progress</B> provided as a code sample.  Written
/// permission by the author is required to use or copy this code.  This
/// code is part of a working but incomplete system (although not all of
/// the work products required to build this code are included herein).
///
/// This is an overview of the mite (MIcroliTE) Apache module, which
/// accepts commands as JSON, XML, URL or raw SQL (the latter in
/// development mode only), interprets the commands as stored procedures
/// to execute against a SQLITE3 database, and returns the results as
/// JSON or XML.
///
/// This module makes the SQLITE database directly available to client
/// applications.  The intent is that almost all processing be offloaded
/// to the client using JavaScript, and is intended to create extremely
/// 'thin' servers to support rich client applications by eliminating
/// an intermediate scripting layer between the HTTP server and the
/// database.
///
/// In the future support may be provided for using JavaScript in triggers
/// and stored procedures, inverting the typical server stack by placing
/// scripting 'behind' the database.  In this way a scripting facility
/// needs only be involved for features which benefit from or require
/// server side processing, while otherwise retaining direct access to
/// SQL DML without scripting.
///
/// Many web-based applications make use of only basic database functionality
/// and require little of the database's relational power.  Using SQLITE
/// provides a facility somewhere between flat files fully lacking relational
/// power, and enterprise class relational databases (which are overkill for
/// many types of web applications).
///
/// A second goal is to allow applications to be contained within a single
/// SQLITE database, including any stored procedures and scripts.  In this way
/// complete applications can be deployed to servers and migrate between them,
/// and perhaps to RIA-enabled clients.
///
/// @section http_sec Interaction with the Apache HTTP server
///
/// http.h http.cpp
///
/// Functions to interact with the Apache HTTP server.  Registers a handler
/// for the clearinghouse (ch) module, and routes input to the appropriate
/// content handler (json, xml, url or sql) per MIME content type (if given).
///
/// The basic form of input is to identify a table or view by name, or a
/// set of SQL stored in the database using the name as a key, and use the
/// remaining input as bind parameters to execute the stored procedures.
/// That is, input is essentially one or more simple commands with parameters
/// that are converted to SQL commands using a basic mapping.
///
/// Input is mapped directly to stored procedures through 'db_X' functions.
/// There is no intermediate representation.
///
/// Result rows (if any) are output as JSON or XML.
///
/// Populates the Transaction structure, which contains all the context
/// associated with a single HTTP request.
///
/// The Apache Module Book provides a wealth of detail on designing
/// Apache modules.
///
/// In the future the module may also work with Microsoft IIS (ISAPI) and/or
/// lighttpd, which are the second and third most popular HTTP servers after
/// Apache.
///
/// @section db_sec   Interaction with the SQLITE3 database
///
/// db.h db.cpp
///
/// SQLITE3 database operations.  SQL is either fetched from the database
/// itself as a sort of lightweight stored procedure (SQLITE3 does not
/// actually support full stored procedures), or synthesized when the input
/// refers directly to a table or view.
///
/// The input parser calls the 'db_X' to populate the stored procedures,
/// which are executed on the fly, and the results used to generate output
/// through an output_callbacks struct in the Transaction.
///
/// In a production module the database connections and stored procedures
/// must be pooled and shared between HTTP requests threads for performance.
/// This is not yet implemented.
///
/// Direct access to tables and views is considered a security risk, and
/// will not be available in production code (it will not simply be
/// deactivated, but will be removed by preprocessor conditionals).
/// Table and view access is provided as a convenience for development
/// builds only.
///
/// As raw SQL processing is also absent from production builds, the
/// only database access permitted is through the psuedo-stored procedures
/// through JSON, XML or URL commands.  In addition, an access control
/// system for stored procedures is planned.
///
/// @section json_sec Read and write JSON
///
/// json.h json.cpp
///
/// Functions to handle JSON input and output.
///
/// The input is handled by yajl.  This may be replaced if not production quality
/// (note that any parser used by this module must be progressive, as the input
/// buffer is processed in parts).  The input functions are based on yajl
/// callbacks (i.e. yajl is to JSON as SAX is to XML).  The parsing semantics
/// directly map to stored procedure execution via 'db_X' functions.
///
/// The module also contains the output functions used to populate an
/// output_callbacks struct.
///
/// There is no JavaScript on the server, and JSON is simply parsed as
/// text and never mapped to JavaScript.
///
/// @section xml_sec  Read and write XML
///
/// xml.h xml.cpp
///
/// The input is handled by xpat, the XML parser included with Apache.  The input
/// functions are based on xpat callbacks, similar to SAX.  The parsing semantics
/// directly map to stored procedure execution via 'db_X' functions.
///
/// The module also contains the output functions used to populate an
/// output_callbacks struct.
///
/// XML is never represented as such internally, and no DOM or DOM-like
/// representations are used.
///
/// @section url_sec  Read URLs
///
/// url.h url.cpp
///
/// Simple commands may be represented as a URL.  Only the portion of the URL
/// following that which identifies the handler is used.  The path portion
/// identifies the table, view or stored procedure, and the query parameters
/// are used as the bind parameters for the SQL stored procedures.
///
/// The final part of the path may also be interpreted as a primary key
/// in some circumstances.
///
/// @section sql_sec  Read SQL
///
/// sql.h sql.cpp
///
/// Processes the input as raw SQL.
///
/// This functionality is considered a security risk and intended only for
/// development use.  It will be removed from production builds completely
/// by conditional compilation.
///
/// @section meta_sec Write database metadata
///
/// meta.h meta.cpp
///
/// Technologies such as RPC and CORBA require clients and servers to have
/// local copies of metadata (in the form of IDL), which must be synchronized
/// by an external mechanism.  Typically the IDL metadata (barring IFR/DSI/DII,
/// for which full support is weak) generates code which is compiled directly
/// into the client or server software.
///
/// XML 'solves' this problem by providing metadata with each and every part
/// of the actual data, resulting in near maximum verbosity.
///
/// This technology bases the metadata on the database schema (the
/// homomorphism between external interface and database schema, and the
/// consistency and traceability it enables, is considered a virture).
/// There seems to be little motivation to otherwise invent yet another
/// schema language.  The metadata can be exchanged between agents arbitrarily
/// -- for instance, as part of an initial handshake, or as a 'header' section
/// in each message, or not at all (by agents which treat the data
/// generically).  Agents may cache the metadata for use with subsequent messages.
///
/// This removes the rigidity of RPC or CORBA, without the verbosity of XML.
///
/// Metadata is formatted as if it were the results of a SQL query, whether or
/// not that is how the metadata is obtained internally, since that simplifies
/// the number of formats the client must handle.
//
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
//________________________________________________________________________ EOF
