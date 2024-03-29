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

DROP TABLE IF EXISTS board;
CREATE TABLE board (
	id INTEGER PRIMARY KEY NOT NULL,
	usr INTEGER REFERENCES user(id) DEFAULT (0),
	grp INTEGER REFERENCES grp(id) DEFAULT (0),
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

CREATE TRIGGER board_updated AFTER INSERT ON board
BEGIN
	UPDATE board SET updated = datetime('now') WHERE id = new.id;
END;

DROP TABLE IF EXISTS piece_type;
CREATE TABLE piece_type (
	id INTEGER PRIMARY KEY NOT NULL,
	name STRING,
	notation STRING,
	icon STRING,
	color INTEGER,
	x INTEGER NOT NULL,
	y INTEGER NOT NULL
);

INSERT INTO piece_type (id, notation, name, color, x, y) VALUES (0, 'KP', 'pawn', 0, 0, 0);
INSERT INTO piece_type (id, notation, name, color, x, y) VALUES (1, 'KR', 'rook', 0, 0, 0);
INSERT INTO piece_type (id, notation, name, color, x, y) VALUES (2, 'KB', 'bishop', 0, 0, 0);
INSERT INTO piece_type (id, notation, name, color, x, y) VALUES (3, 'KN', 'knight', 0, 0, 0);
INSERT INTO piece_type (id, notation, name, color, x, y) VALUES (4, 'Q', 'queen', 0, 0, 0);
INSERT INTO piece_type (id, notation, name, color, x, y) VALUES (5, 'K', 'king', 0, 0, 0);

DROP TABLE IF EXISTS piece;
CREATE TABLE piece (
	id INTEGER PRIMARY KEY NOT NULL,
	usr INTEGER REFERENCES user(id) DEFAULT (0),
	grp INTEGER REFERENCES grp(id) DEFAULT (0),
	board INTEGER REFERENCES board(id) DEFAULT (0),
	player INTEGER REFERENCES user(id) DEFAULT (0),
	type INTEGER REFERENCES piece_type(id) DEFAULT (0),
	removed INTEGER DEFAULT (0),
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

CREATE TRIGGER piece_updated AFTER INSERT ON piece
BEGIN
	UPDATE piece SET updated = datetime('now') WHERE id = new.id;
END;

DROP TABLE IF EXISTS move;
CREATE TABLE move (
	id INTEGER PRIMARY KEY NOT NULL,
	usr INTEGER REFERENCES user(id) DEFAULT (0),
	grp INTEGER REFERENCES grp(id) DEFAULT (0),
	piece INTEGER REFERENCES piece(id),
	x INTEGER NOT NULL,
	y INTEGER NOT NULL,
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

DROP TABLE IF EXISTS scope;
CREATE TABLE scope (
	id INTEGER PRIMARY KEY NOT NULL,
	session INTEGER REFERENCES session(id) NOT NULL,
	query INTEGER,
	board REFERENCES board(id),
	sequence INTEGER DEFAULT (0),
	time DATE INTEGER DEFAULT(0) );

CREATE TRIGGER move_updated AFTER INSERT ON move
BEGIN
	UPDATE move SET updated = datetime('now') WHERE id = new.id;
END;

DROP TABLE IF EXISTS sql;
CREATE TABLE sql (
	id INTEGER PRIMARY KEY NOT NULL,
	name 'text/plain',
	stmt 'application/x-sql',
	created DATE DEFAULT (CURRENT_TIMESTAMP),
	updated DATE DEFAULT (CURRENT_TIMESTAMP) );

-- - metadata for stored SQL
INSERT INTO sql (name, stmt)
	VALUES ('meta',
	"SELECT i AS qid FROM literal WHERE i=$qid;
	 SELECT * FROM metasql;
	 SELECT * FROM metatable WHERE tbl IN ('metasql', 'metabinding', 'metatable', 'metaforeign', 'literal', 'session', 'scope', 'piece', 'board', 'piece_type', 'move');
	 SELECT * FROM metabinding;
	 SELECT * FROM metaforeign;");

--
INSERT INTO sql (name, stmt)
	VALUES ('session_start',
	"INSERT INTO session (id) VALUES (null);
	 SELECT session.id AS sid, i AS qid FROM session, literal
		WHERE session.id = last_insert_rowid()
		AND i = $qid;");

-- set up a new game
INSERT INTO sql (name, stmt)
	VALUES ('game_start',
	"SELECT session.id AS sid, i AS qid FROM session, literal WHERE session.id = $sid AND i=$qid;
	 INSERT INTO board (id) VALUES (null);
	 INSERT INTO piece (board, type)
		VALUES (last_insert_rowid(), 0);
	 INSERT INTO piece (board, type)
		VALUES ((SELECT piece.board FROM piece WHERE piece.id = last_insert_rowid()), 1);
	 INSERT INTO piece (board, type)
		VALUES ((SELECT piece.board FROM piece WHERE piece.id = last_insert_rowid()), 2);
	 INSERT INTO piece (board, type)
		VALUES ((SELECT piece.board FROM piece WHERE piece.id = last_insert_rowid()), 3);
	 INSERT INTO piece (board, type)
		VALUES ((SELECT piece.board FROM piece WHERE piece.id = last_insert_rowid()), 4);
	 SELECT board.id AS board FROM piece, board
		WHERE piece.id = last_insert_rowid()
		AND board.id = piece.board;");

-- get current game state
INSERT INTO sql (name, stmt)
	VALUES ('game_state',
	"SELECT session.id AS sid, i AS qid FROM session, literal WHERE session.id = $sid AND i=$qid;
	 SELECT * FROM board WHERE id = $board;
	 SELECT * FROM piece WHERE board = $board;
	 SELECT * FROM piece_type;");

-- - propose a move
INSERT INTO sql (name, stmt)
	VALUES ('game_move',
	"SELECT session.id AS sid, query AS qid FROM session, scope
		WHERE scope.id = $scid AND session.id = session;
	 INSERT INTO move (piece, x, y) VALUES ($piece, $x, $y);");

--
INSERT INTO sql (name, stmt)
	VALUES ('game_scope',
	"SELECT session.id AS sid, i AS qid FROM session, literal WHERE session.id = $sid AND i=$qid;
	 INSERT OR REPLACE INTO scope (id, session, query, board)
		VALUES ((SELECT id FROM scope WHERE session = $sid AND query = $qid AND board = $board),
			$sid, $qid, $board);
	 SELECT id AS scid FROM scope WHERE id = last_insert_rowid();");

INSERT INTO sql (name, stmt)
	VALUES ('game_check',
	"SELECT session.id AS sid, query AS qid FROM session, scope
		WHERE scope.id = $scid AND session.id = session;
	 SELECT scope.id AS scid,
		(SELECT count(*) FROM move, scope, piece
			WHERE scope.id = $scid
			AND move.updated > (SELECT time FROM scope WHERE id = $scid)
			AND move.piece = piece.id
			AND piece.board = scope.board)
			AS count
		FROM scope
		WHERE scope.id = $scid;");

-- 
INSERT INTO sql (name, stmt)
	VALUES ('game_delta',
	"SELECT session.id AS sid, query AS qid FROM scope, session
		WHERE scope.id = $scid AND session.id = session;
	 SELECT piece.id, move.x, move.y FROM move, piece
		WHERE move.updated > (SELECT time FROM scope WHERE id = $scid)
		AND piece.id = move.piece
		AND piece.board = (SELECT board FROM scope WHERE id = $scid);
	 UPDATE scope SET time = (SELECT max(move.updated) FROM move, piece
		WHERE piece.id = move.piece AND piece.board = (SELECT board FROM scope WHERE id = $scid))
		WHERE id = $scid;");

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
