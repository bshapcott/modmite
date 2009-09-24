//____________________________________________________________________________
// mite - mod microlite - todo.h
//
// Copyright 2009 Brad Shapcott -- see end of file for license & contact info
//
// INDEXED
/// - move project to google code
/// - web service
///   - SOAP
///   - WSDL
/// - server appliance, Amazon AMI
/// - front end
///   - flexible spreadsheet
/// - MIME column type
///   - single valued return
/// - db caching (APR resource mgmt)
///   - prepared statement caching (APR resource mgmt)
///   - precaching prepared statements
///   - discovery
///   - statement pooling
///     - reset and clear sqlite bindings if not ad hoc
/// - parameter clearing must be determined by parser
/// - positional parameters
///   - autogen map by analyzing prepared statements
///     - order of appearance
///   - for tables, use natural column order
///   - task: [1, "foo", "bar"]
/// - array style return
///   - no metadata mixed in data
/// - dev tools
///   - firebug
///   - browser plugin
/// - mastering (google file system and big table)
///   - client connects to master
/// - internal config & stats
/// - in memory database
/// - SNMPv2
/// - vtab for other native functionality
/// - paths
///   - <path>/<dbname>/<keys>
/// - javascript vtab
///   - reentrant
///   - pool v. gc
///   - expose sqlite via javascript
///   - manage interpreter instances efficiently
/// - javascript repair
///   - get delta.js
///   - rewind to previously working copy & refactor again
/// - db pass thru vtab
/// - metadata as vtab
///   - query metadata for db query auto
/// - request headers & parameters vtab
/// - form post
///   - form2db form.cpp
/// - binding descriptions
///   - SMD?
///   - XML Schema?
///   - task: {foo:{pk: true, type: int}}
/// - mixed media
///   - <game><task>[{}, {}, []]</task></game>
///   - bidirectional?
///   - ["<task id='1'>", {}]
///   - why? string length limits in browsers
/// - viral updates
///   - code
///   - data
/// - syndication
///   - cache
///   - client can query source for updates
/// - last row id chaining
///   - vtab
/// - rcs tags 
/// - nesting
///   - using pragma foreign_key_list
///   - e.g. task: {story: {...}} 
///   - reverse story: {id: 3, task: [{...}, {...}]}
///     - task must have foreign key to story
/// - multiple binds
///   - task: [{}, {}, []]
///   - instead of [{task: {}}, {task: {}}]
/// - spark
///   - COMET notifications-only server
///   - browser must query server for actual data
///   - short & long cycle
///   - piggybacking
///   - URL only, no data push (only pull)
/// - dev mode
///   - raw SQL
///   - statement synthesis
///   - production simulation mode
/// - *readable* switch
///   - by MIME type
///   - text/plain (pretty print JSON v. app/json)
///   - text/xml (pretty print XML, include comments in DEV, v. app/xml)
/// 
/// UNKNOWN
/// - optimization
///   - check if parsed text persists, to eliminate copying
/// 
/// EVAL
/// - empty
/// 
/// BLOG
/// - database bypass web->script->database
/// - often scripting is just a front end for a database
/// - decoration of results with presentation logic
///   - blending of model and view
///   - no separation, no encapsulation
/// - screen-scraping robots
///   - mainframe era technologies on the web
/// - code where used
/// - enterprise & pub-sub database caching
///   - usually start with pubsub or direct enterprise database
///   - retrofit caching to lightweight database
///     - start with lightweight database instead
/// - consistency (js client === js server)
/// - offlining
///   - google gears
/// - clearinghouse architecture
///   - wiki
///   - forums
///   - blogs
/// - tennis analogy
///   - need to regress to improve technique
///   - but it's worse -- like a saw, the tools get duller over time, what used
///     to work stops working (as well, anyway)
///   - gets increasingly harder to maintain same level of productivity
/// - WORLD -> ( HTML -> DATA )
///   - ( WORLD -> HTML ) -> DATA
///   - dumpster diving for data
///   - you want the ingredients but I'm giving you soup
///   - you need to work backwards to the ingredients
///   - semantic baggage
///   - actually makes data harder to deal with
///   - semantics are the middle game, data the opening position
///     and presentation the end game -- don't mix!
/// - we think of fixing 'a bug' or adding 'a feature'
///   - imagine a machine for building houses -- each design defect
///     gets replicated to every house, each improvement the same
///   - s/w design (source) product (executable)
///     - s/w is NOT the product
///   - engineers are always designing
///   - do not conflate design with product, even if the conversion
///     occurs on the fly in an interpreter
///   - we fix one design defect and many product defects
///   - we even replace defective houses (malleable medium)
///   - tools and medium are the same (software builds software)
/// - those who do not understand relational databases are doomed to
///   re-implement them, badly (NVP)
/// - cascading object pools
///   - class A {static pool P;}
///   - class B {static pool P;}
///   - A::do() {B *b = new (A::P) B();}
///   - modify new to place pool info in header
///   - must pass parent object (else starts new pool)
/// - owner-managed antipattern
/// - type evaluation antipattern
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
