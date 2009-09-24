--_____________________________________________________________________________
-- mite - mod microlite
-- Copyright 2009 Brad Shapcott -- see end of file for license & contact info

DROP TABLE IF EXISTS usr;
CREATE TABLE usr (
	id INTEGER PRIMARY KEY NOT NULL,
	name 'text/plain',
	password BLOB,
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

INSERT INTO usr (id, name) VALUES (0, 'root');
INSERT INTO usr (id, name) VALUES (1, 'guest');

DROP TABLE IF EXISTS grp;
CREATE TABLE grp (
	id INTEGER PRIMARY KEY NOT NULL,
	name 'text/plain',
	usr INTEGER REFERENCES usr(id),
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

INSERT INTO grp (id, name) VALUES (0, 'wheel');
INSERT INTO grp (id, name) VALUES (1, 'guest');

DROP TABLE IF EXISTS member;
CREATE TABLE member (
	usr INTEGER REFERENCES usr(id) DEFAULT(0),
	grp INTEGER REFERENCES grp(id) DEFAULT(0),
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

INSERT INTO member (grp, usr) VALUES (0, 0);
INSERT INTO member (grp, usr) VALUES (1, 1);

DROP TABLE IF EXISTS session;
CREATE TABLE session (
	id INTEGER PRIMARY KEY NOT NULL,
	permissions INTEGER DEFAULT (0),
	usr INTEGER REFERENCES usr(id) DEFAULT (0),
	grp INTEGER REFERENCES grp(id) DEFAULT (0),
	ttl DATE,
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

INSERT INTO session (usr) VALUES (0);

DROP TABLE IF EXISTS widget_type;
CREATE TABLE widget_type (
	id INTEGER PRIMARY KEY NOT NULL,
	usr INTEGER REFERENCES usr(id) DEFAULT (0),
	grp INTEGER REFERENCES grp(id) DEFAULT (0),
	url 'application/x-url',
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

INSERT INTO widget_type (id, url) VALUES (0, 'login');
INSERT INTO widget_type (id, url) VALUES (1, 'browser'); 

DROP TABLE IF EXISTS widget;
CREATE TABLE widget (
	id INTEGER PRIMARY KEY NOT NULL,
	usr INTEGER REFERENCES usr(id) DEFAULT (0),
	grp INTEGER REFERENCES grp(id) DEFAULT (0),
	parent INTEGER REFERENCES widget(id),
	session INTEGER REFERENCES session(id),
	widget_type INTEGER REFERENCES widget_type(id),
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

INSERT INTO widget (usr, parent, session, widget_type) VALUES (1, 0, 0, 0);
INSERT INTO widget (usr, parent, session, widget_type) VALUES (0, 0, 1, 1);

DROP TABLE IF EXISTS epic;
CREATE TABLE epic (
	id INTEGER PRIMARY KEY NOT NULL,
	usr INTEGER REFERENCES user(id) DEFAULT (0),
	grp INTEGER REFERENCES grp(id) DEFAULT (0),
	title 'text/plain',
	content 'text/plain',
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

DROP TABLE IF EXISTS story;
CREATE TABLE story (
	id INTEGER PRIMARY KEY NOT NULL,
	usr INTEGER REFERENCES user(id) DEFAULT (0),
	grp INTEGER REFERENCES grp(id) DEFAULT (0),
	epic INTEGER REFERENCES epic(id),
	title 'text/plain',
	content 'text/plain',
	url 'application/x-url',
	estimate FLOAT,
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

INSERT INTO story (epic, title, content, url, estimate)
       VALUES (0, "Display mapping values", "Display mapping values in UI.",
       "http://foobar.com/display_map", 8.0);
INSERT INTO story (epic, title, content, url, estimate)
       VALUES (0, "Hook up hierarchical data", "Resolve all references and links in returned data.",
       "http://foobar.com/hier_data", 12.5);

DROP TABLE IF EXISTS task;
CREATE TABLE task (
	id INTEGER PRIMARY KEY NOT NULL,
	usr INTEGER REFERENCES user(id) DEFAULT (0),
	grp INTEGER REFERENCES grp(id) DEFAULT (0),
	story INTEGER REFERENCES story(id),
	title 'text/plain',
	content 'text/plain',
	url 'application/x-url',
	estimate FLOAT,
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

INSERT INTO task (story, title, content, url, estimate)
       VALUES (1, "Finish Designing UI",
       "Finally finish designing the UI with proper resizing.",
       "http://foobar.com/finish_ui", 8);
INSERT INTO task (story, title, content, url, estimate)
       VALUES (1, "Autogen SQL Script",
       "Generate the database schema and DDL from object descriptions.",
       "http://foobar.com/autogen", 8);
INSERT INTO task (story, title, content, url, estimate)
       VALUES (1, "Fix Timestamp Trigger",
       "Timestamp trigger affects all rows.",
       "http://foobar.com/timestamp", 8);
INSERT INTO task (story, title, content, url, estimate)
       VALUES (2, "Resolve links",
       "Resolve links in returned data.",
       "http://foobar.com/resolve_linx", 8);
INSERT INTO task (story, title, content, url, estimate)
       VALUES (2, "Resolve references",
       "Resolve references in returned data.",
       "http://foobar.com/resolve_refs", 8);

CREATE TRIGGER task_timestamp AFTER INSERT ON task
BEGIN
	UPDATE task SET timestamp = datetime('now') WHERE id = new.id;
END;

DROP VIEW IF EXISTS story_task;
CREATE VIEW story_task AS SELECT task.id AS child, task.story
	FROM task, story WHERE story.id = task.story;

DROP TABLE IF EXISTS theme;
CREATE TABLE theme (
	id INTEGER PRIMARY KEY NOT NULL,
	title 'text/plain',
	content 'text/plain',
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

DROP TABLE IF EXISTS tag;
CREATE TABLE tag (
	id INTEGER PRIMARY KEY NOT NULL,
	title 'text/plain',
	content 'text/plain',
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

DROP TABLE IF EXISTS query;
CREATE TABLE query (
	id INTEGER PRIMARY KEY NOT NULL,
	name 'text/plain',
	stmt 'application/x-sql',
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );
	
INSERT INTO query (name, stmt)
	VALUES ('qtask',
	"SELECT id, title, content, 'task' AS type FROM task WHERE story=$story;
	 SELECT id, title, content, 'story' AS type FROM story WHERE id=$story;
	 SELECT story.id AS story, task.id AS task FROM story, task
		WHERE story.id = task.story AND story.id = $story");

--_____________________________________________________________________ CONTACT
-- mailto:brad at modmite dot com
-- http://modmite.com
--_____________________________________________________________________ LICENSE
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--	http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.
--_________________________________________________________________________ EOF
