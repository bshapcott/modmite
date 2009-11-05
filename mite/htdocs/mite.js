//_____________________________________________________________________________
// Copyright 2008 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com
//
// - mite server connector
//   - manages connection to back end data on behalf of the browser application
//   - all server data is converted to HTML microformat, and the client works
//     with the HTML, not directly with the data from the server
// - vanilla JavaScript ONLY in this module!
//   - no JavaScript libraries allowed (Dojo, Prototype, jQuery, etc.)
//   - want this module library-agnostic, so that it can be USED by any of the
//     JavaScript libraries
//   - unfortunately that means no CSS Selectors or XPath, because IE doesn't
//     have a native implementation :-P, so lots of DOM traversal and manipulation
//     instead

//_____________________________________________________________________________
// - x-browser cruft
// @todo: Msxml2.XMLHTTP, Microsoft.XMLHTTP, Msxml2.XMLHTTP.4.0
function mite_get_xhr() {
	var xhr = null;
	if (window.XMLHttpRequest) {
		xhr = new XMLHttpRequest;
	} else if (typeof ActiveXObject != "undefined") {
		xhr = new ActiveXObject("Microsoft.XMLHTTP");
	}
	return xhr;
}

//_____________________________________________________________________________
// - send data via XmlHttpRequest
// @param	data		- data to be sent
// @param	accept		- requested mime type of response data
//                        (per IANA.org, or X-token)
// @param	cb			- callback function to handle any response
function mite_send_data(data, accept, cb) {
	var xhr = mite_get_xhr();
	if (!xhr) {
		return;
	}
	xhr.onreadystatechange = function () {cb(xhr);}
	var method = "POST";
	var content = data;
	var url = "/mite";
	// - auto-detect the MIME type of the input using extremely basic
	//   heuristics, ignoring leading and trailing whitespace
	var mime_type;
	if (/^\s*\{/.test(data)) {
		// - json is anything text starting with '{'
		mime_type = "application/json";
	} else if (/^\s*\</.test(data)) {
		// - xml is any text starting with '<'
		mime_type = "application/xml";
	} else if (/\S+\s+\S+/.test(data)) {
		// - SQL is at least two non-whitespace sequences (i.e. tokens)
		mime_type = "application/x-sql";
	} else if (/\s*\S+\s*$/.test(data)) {
		// - a URL is single unbroken sequence of non-whitespace characters
		mime_type = "";
	} else {
		throw new Error("The MIME type of the input text cannot be detected.");
	}
	switch (mime_type) {
		case "application/x-sql":
			url = "/mite/game";
			break;
		case "":
			method = "GET";
			url = data;
			content = "";
			break;
		default:
			break; // - lint fodder
	}
	xhr.open(method, url, true);	
	xhr.setRequestHeader("Content-Type", mime_type);
	xhr.setRequestHeader("Accept", accept);
	xhr.send(data);
}

//______________________________________________________________________________
function mite() {}

//______________________________________________________________________________
mite.prototype.getChildrenByName = function mite_getChildrenByName(name) {
	var c = [];
	for (var i = 0; i < this.element.childNodes.length; i++) {
		if (this.element.childNodes[i].nodeType == document.ELEMENT_NODE &&
			this.element.childNodes[i].nodeName === name)
		{
			c.push(this.element.childNodes[i]);
		}
	}
	return c;
}

//______________________________________________________________________________
mite.prototype.getChildren = function mite_getChildren() {
	return [];
}

//______________________________________________________________________________
mite.prototype.mayHaveChildren = function mite_mayHaveChildren() {
	return this.getChildren().length !== 0;
}

//_______________________________________________________________________________
// - more like a Java-style listener, attempts to send all events on the API,
//   as long as the listener has a corresponding method, and doesn't support
//   function-style callbacks (use an anonymous object instead)
// - this is somewhat internal, and meant to bind in to the event system of
//   whatever JavaScript library is being employed (e.g. Dojo, upon which the
//   APIs are based, but not dependent)
mite.prototype.addEventListener = function mite_addEventListener(listener) {
	if (!this.listener) {
		this.listener = [];
	}
	this.listener.push(listener);
}

//______________________________________________________________________________
mite.prototype.removeEventListener = function mite_removeEventListener(listener) {
	if (listeners && listeners.length) {
		for (var i = 0; i < listeners.length; i++) {
			if (listeners[i] === listener) {
				listeners.splice(i+1, 1);
			}
		}
	}
}

//______________________________________________________________________________
mite.prototype.fireEvent = function mite_fireEvent(event, args) {
	if (this.listener && this.listener.length > 0) {
		for (var i = 0; i < this.listener.length; i++) {
			if (this.listener[i][event]
				&&
				typeof this.listener[i][event] === "function")
			{
				this.listener[i][event].apply(this.listener[i], args);
			}
		}
	}
	// - bubble up
	if (this.element.parentNode.mite) {
		this.element.parentNode.mite[event].apply(this.element.parentNode.mite, args);
	}
}

//______________________________________________________________________________
// - rolled up child changes (new, delete on children), fired after ALL changes
//   have been applied
// - based, but not dependent on, Dojo APIs
// @see dijit.tree.model
mite.prototype.onChildrenChange = function mite_onChildrenChange(parent) {
	this.fireEvent("onChildrenChange", arguments);
}

//______________________________________________________________________________
// - fine grained item attribute changes, fired as EACH INDIVIDUAL attribute
//   is changed
// - based, but not dependent on, Dojo APIs
// @see dojo.data.api.Notification
mite.prototype.onSet = function mite_onSet(element, attr, oval, nval) {
	this.fireEvent("onSet", arguments);
}

//______________________________________________________________________________
// - rolled up item attribute changes, fired after ALL attribute changes
//   have been applied
// - based, but not dependent on, Dojo APIs
// @see dijit.tree.model
mite.prototype.onChange = function mite_onChange(element) {
	this.fireEvent("onChange", arguments);
}

//______________________________________________________________________________
// - fine grained child creation notification, fired as EACH child is created
// - based, but not dependent on, Dojo APIs
// @see dijit.tree.model
// @see dojo.data.api.Notification
mite.prototype.onNew = function mite_onNew(element) {
	this.fireEvent("onNew", arguments);
}

//______________________________________________________________________________
// - fine grained child delete notification, fired as EACH child is deleted
// - based, but not dependent on, Dojo APIs
// @see dojo.data.api.Notification
mite.prototype.onDelete = function mite_onDelete(element) {
	this.fireEvent("onDelete", arguments);
}

//______________________________________________________________________________
function mite_container(div) {
	this.element = div;
}

//______________________________________________________________________________
mite_container.prototype = new mite();

//______________________________________________________________________________
mite_container.prototype.getChildren = function mite_container_getChildren() {
	return this.getChildrenByName("DIV");
}

//______________________________________________________________________________
function mite_meta(el) {
	this.element = el;
}

//______________________________________________________________________________
mite_meta.prototype = new mite();

//______________________________________________________________________________
function mite_sql(div) {
	this.element = div;
}

//______________________________________________________________________________
mite_sql.prototype = new mite();

//______________________________________________________________________________
mite_sql.prototype.getChildren = function mite_sql_getChildren() {
	return this.getChildrenByName("TABLE");
}

//______________________________________________________________________________
function mite_stmt(table) {
	this.element = table;
}

//______________________________________________________________________________
mite_stmt.prototype = new mite();

//______________________________________________________________________________
mite_stmt.prototype.getChildren = function mite_stmt_getChildren() {
	var tbody = this.getChildrenByName("TBODY");
	return tbody.length > 0 ? tbody[0].mite.getChildren() : [];
}

//______________________________________________________________________________
function mite_rows(tbody) {
	this.element = tbody;
}

//______________________________________________________________________________
mite_rows.prototype = new mite();

//______________________________________________________________________________
function mite_row(tr) {
	this.element = tr;
}

//______________________________________________________________________________
mite_row.prototype = new mite();

//______________________________________________________________________________
function mite_cell(td) {
	this.element = td;
}

//______________________________________________________________________________
mite_cell.prototype = new mite();

//______________________________________________________________________________
// - create the DIV used as the root of the HTML microformat hierarchy
function mite_init() {
	var div = mite_find_or_create("mite", "div", document.body, mite_container);
	div.style.display = "none";
	mite_find_or_create(999, "div", div, mite_container);
}

//______________________________________________________________________________
// - create a 'tag' element as a child of 'parent', and construct a new JS
//   object from 'ctor'
function mite_create_element(tag, parent, ctor) {
	var e = document.createElement(tag);
	e.mite = new ctor(e);
	if (parent) {
		parent.appendChild(e);
	}
	return e;
}

//______________________________________________________________________________
// - find the element with the given 'id', or create such an element
// - parent is NOT used to narrow the scope of the search, but only to provide
//   an anchor for the element creation
function mite_find_or_create(id, type, parent, ctor) {
	var e = document.getElementById(id);
	if (!e) {
		e = mite_create_element(type, parent, ctor);
		e.setAttribute("id", id);
		var t = /[^.]+$/.exec(e.id);
		if (t) {
			e.title = t[0];
		}
		// @todo - parentInfo per data.api.Notification
		e.mite.onNew(e);
		if (parent && parent.mite) {
			// @todo - roll up deltas on parent
			parent.mite.onChildrenChange(parent, parent.mite.getChildren());
		}
	}
	return e;
}

//_____________________________________________________________________________
// - find rows in the table with matching column values
// @return - an array of matching TR elements
function mite_tr_lookup(table, nvp) {
	var r = [];
	var tr = table.getElementsByTagName("tbody")[0].getElementsByTagName("tr");
	for (var i = 0; i < tr.length; i++) {
		var td = tr[i].getElementsByTagName("td");
		var f = true;
		for (var k in nvp) {
			if (nvp[k] != td[k].innerHTML) {
				f = false;
				break;
			}
		}
		if (f) {
			r.push(tr[i]);
		}
	}
	return r;
}

//_____________________________________________________________________________
// - convert table rows (TR elements) to JS objects
// @param	tr		- array of TR DOM nodes
// @return			- an array of corresponding JS objects representing the TR
//                    elements (one object per row with a field for each
//                    column)
// @todo - maybe cache on DOM.mite?
function mite_tr2obj(tr) {
	var r = [];
	for (var i = 0; i < tr.length; i++) {
		var s = [];
		var td = tr[i].getElementsByTagName("td");
		for (var j = 0; j < td.length; j++) {
			s.push(td[j].innerHTML);
		}
		r.push(s);
	}
	return r;
}

//_____________________________________________________________________________
// - convert ALL table rows to JS objects
function mite_table2obj(table) {
	var tr = table.getElementsByTagName("tbody")[0].getElementsByTagName("tr");
	return mite_tr2obj(tr);
}

//_____________________________________________________________________________
// - find rows in the table with matching column values
// - calls mite_tr_lookup, then converts the TR rows to JS arrays whose
//   elements are the TD values
// @return - a JavaScript array representing the matching TR elements
function mite_lookup(table, nvp) {
	var tr = mite_tr_lookup(table, nvp);
	return mite_tr2obj(tr);
}

//_____________________________________________________________________________
// - return the column indices of the table's primary keys
// @todo - use metadata tables, don't rely on tfoot
function mite_pk_indices(table) {
	var tfoot = table.getElementsByTagName("tfoot");
	if (tfoot.length === 0) {
		// - no footer, so no metadata for primary key
		return undefined;
	}
	if (tfoot[0].getElementsByTagName("tr").length === 0) {
		// - no rows in footer, so no metadata for primary key
		return undefined;
	}
	// @todo - don't hardcode row # (from meta<X> column)
	var tdf = tfoot[0].getElementsByTagName("tr")[6].getElementsByTagName("td");
	var ix = [];
	for (var i = 0; i < tdf.length; i++) {
		if (tdf[i].innerHTML == "1") {
			ix.push(i);
		}
	}
	return ix;
}

//_____________________________________________________________________________
// - find a row in the table with the same matching primary key(s)
// @todo - use metadata tables, not thead
function mite_pk_lookup(table, row) {
	var thead = table.getElementsByTagName("thead");
	if (thead.length === 0) {
		return [];
	}
	var ix = mite_pk_indices(table);
	if (!ix || ix.length === 0) {
		return [];
	}
	// @todo - don't hardcode row # (from meta<X> column)
	var nvp = {};
	var tdh = thead[0].getElementsByTagName("tr")[6].getElementsByTagName("td");
	for (var i = 0; i < ix.length; i++) {
		nvp[ix[i]] = row[tdh[ix[i]].innerHTML];
	}
	return mite_tr_lookup(table, nvp);
}

//_____________________________________________________________________________
function mite_column_name(table, index) {
	var thead = table.getElementsByTagName("thead");
	if (thead.length === 0) {
		return index;
	}
	return thead[0].getElementsByTagName("tr")[6].getElementsByTagName("td")[index].innerHTML;
}

//_____________________________________________________________________________
// - convert server data to HTML microformat
function mite_microformat(qid, table, db, sql, stmt, rows) {
	var caption = table.getElementsByTagName("caption");
	if (caption.length === 0) {
		caption = document.createElement("caption");
		caption.innerHTML = db + "." + sql + "." + stmt;
		table.insertBefore(caption, table.firstChild);
	}
	var tbody = table.getElementsByTagName("tbody");
	if (tbody.length === 0) {
		tbody = [mite_create_element("tbody", table, mite_rows)];
	}
	var rc = false;
	for (var row in rows) {
		var r = rows[row];
		var p = mite_pk_lookup(table, r);
		var tr;
		// @todo - check at most one member of 'p'
		if (p.length > 0) {
			tr = p[0];
			var cc = false;
			var td = tr.getElementsByTagName("td");
			var i = 0;
			for (var c in r) {
				var delta = td[i].innerHTML != r[c];
				if (delta) {
					var old = td[i].innerHTML;
					td[i].innerHTML = r[c];
					cc = true;
					tr.mite.onSet(tr, mite_column_name(table, i), old, r[c]);
				}
				i++;
			}
			if (cc) {
				tr.mite.onChange(tr);
			}
		} else {
			rc = true;
			tr = mite_create_element("tr", tbody[0], mite_row);
			for (var c in r) {
				var td = mite_create_element("td", tr, mite_cell);
				td.setAttribute("class", "mite");
				td.innerHTML = r[c];
			}
			tr.mite.onNew(tr);
		}
	} // row
	if (rc) {
		tbody[0].mite.onChildrenChange(tbody[0], tbody[0].mite.getChildren());
	}
}

//_____________________________________________________________________________
// - decorate the HTML microformat with metadata
// - note that metadata is itself HTML microformat, and is self-describing
//   since it is derived from server data
function mite_metasql(qid, table, db, sql, stmt) {
	var thead = table.getElementsByTagName("thead");
	if (thead.length > 0) {
		return;
	}
	thead = mite_create_element("thead", table, mite_meta);
	// @todo - fix hardcoded id
	var metasql = document.getElementById(qid + "." + db + ".meta.1");
	if (!metasql) {
		return;
	}
	// @todo - fix hardcoded id
	var metatable = document.getElementById(qid + "." + db + ".meta.2");
	var mtl = metatable.getElementsByTagName("tbody")[0].getElementsByTagName("tr")[0].getElementsByTagName("td").length;
	var tfoot;
	if (metatable) {
		tfoot = mite_create_element("tfoot", table, mite_meta);
	}
	var thr = [];
	var tfr = [];
	var s = mite_lookup(metasql, {0: sql, 1: stmt});
	for (var i in s) {
		for (var j in s[i]) {
			if (!thr[j]) {
				thr[j] = mite_create_element("tr", thead, mite_meta);
			}
			var td = mite_create_element("td", thr[j], mite_meta);
			td.setAttribute("class", "meta");
			td.innerHTML = s[i][j];
		}
		var t = mite_lookup(metatable, {0: s[i][4], 2: s[i][5]});
		var len = t.length > 0 ? t[0].length : mtl;
		for (var j = 0; j < len; j++) {
			if (!tfr[j]) {
				tfr[j] = mite_create_element("tr", tfoot, mite_meta);
			}
			var td = mite_create_element("td", tfr[j], mite_meta);
			td.setAttribute("class", "meta");
			td.innerHTML = t.length > 0 ? t[0][j] : "";
		}
	}
}

//_____________________________________________________________________________
// - returns the result column names (NOT necessarily the table column names,
//   if AS was used in the SQL)
// @todo - use metadata tables, not thead
function mite_column_names(table) {
	var c;
	var thead = table.getElementsByTagName("thead");
	var td;
	if (thead.length > 0) {
		var tr = thead[0].getElementsByTagName("tr");
		if (tr.length >= 7) {
			td = tr[6].getElementsByTagName("td");
			c = [];
			for (var i = 0; i < td.length; i++) {
				c.push(td[i].innerHTML);
			}
		}
	}
	return c;
}

//_____________________________________________________________________________
// - set the id and title attribute for the table's rows (TR) and cells (TD)
function mite_id(table) {
	var cn = mite_column_names(table);
	var ix = mite_pk_indices(table);
	if (!ix) {return;}
	var tbody = table.getElementsByTagName("tbody");
	if (tbody.length === 0) {return;}
	var tr = tbody[0].getElementsByTagName("tr");
	for (var row = 0; row < tr.length; row++) {
		if (tr[row].id) {continue;}
		var td = tr[row].getElementsByTagName("td");
		if (ix.length > 0) {
			tr[row].title = "";
			var s = "";
			for (var i = 0; i < ix.length; i++) {
				tr[row].title = tr[row].title + s + td[ix[i]].innerHTML;
				s = ".";
			}
			tr[row].id = table.id + "." + tr[row].title;
		} else {
			tr[row].id = table.id + "." + row;
			tr[row].title = row;
		}
		for (var col = 0; col < td.length; col++) {
			td[col].id = tr[row].id + "." + col;
			td[col].title = cn ? cn[col] : col;
		}
	}
}

//_____________________________________________________________________________
function mite_get_cell(id, r, c) {
	var table = document.getElementById(id);
	var tbody = table.getElementsByTagName("tbody");
	if (tbody.length === 0) return;
	var tr = tbody[0].getElementsByTagName("tr");
	if (tr.length === 0) return;
	var td = tr[r].getElementsByTagName("td");
	if (td.length === 0) return;
	return td[c].innerHTML;	
}

//_____________________________________________________________________________
function mite_get_session(qid) {
	return mite_get_cell(qid + ".game.session_start.1", 0, 0);
}

//_____________________________________________________________________________
// @todo - notifications for data changes
// - DIV can be ground zero for connecting to other subsystems
function mite_eval_o(data) {
	try {
		var it = eval("(" + data + ")");
		// - convert JSON to microformat
		for (var pass = 0; pass < 2; pass++) {
			for (var dpad in it) {for (var db in it[dpad]) {
				for (var pad in it[dpad][db]) {for (var sql in it[dpad][db][pad]) {
					var qid;
					// - look for query identifier
					for (var i = 0; i < 3; i++) {
						try {
							qid = it[dpad][db][pad][sql][i][0]["qid"];
						} catch (e) {
						}
						if (qid) {break;}
					}
					if (!qid) {
						qid = 0;
					}
					var div_top = mite_find_or_create("mite", "div", document.body, mite_container);
					var div_query = mite_find_or_create(qid, "div", div_top, mite_container);
					var div_sql = mite_find_or_create(qid + "." + db + "." + sql, "div", div_query, mite_sql);
					for (var stmt in it[dpad][db][pad][sql]) {
						var table = mite_find_or_create(qid + "." + db + "." + sql + "." + stmt,
							"table", div_sql, mite_stmt);
						table.setAttribute("class", "mite");
						switch (pass) {
						case 0:
							mite_microformat(qid, table, db, sql, stmt, it[dpad][db][pad][sql][stmt]);
							break;
						case 1:
							mite_metasql(qid, table, db, sql, stmt);
							mite_id(table);
							break;
						}
					} // stmt
				}} // pad + sql
			}} // pad + db
		} // pass
	} catch (e) {
		console.warn(e);
		console.debug(e.stack);
	}
}

//__________________________________________________________________________
function mite_send_eval(data, fn) {
  mite_send_data(data, "application/json", function (xhr) {
    if (xhr.readyState == 4) {
	  mite_eval_o(xhr.responseText);
	  if (fn) {
		fn();
	  }
    }
  });
}

//__________________________________________________________________________
// - quick & dirty scratch test
function mite_scratch_test(funcs) {
  if (funcs.length === 0) {
	throw new Error("empty funcs");
  }
  var data = (funcs.shift())();
  mite_send_data(data, "application/json", function (xhr) {
    if (xhr.readyState == 4) {
	  mite_eval_o(xhr.responseText);
      if (funcs.length > 0) mite_scratch_test(funcs);
    }
  });
}

//__________________________________________________________________________
function mite_poll(query) {
	return setInterval(function (q) {mite_scratch_test([q]);}, 4000, query);
}

//_________________________________________________________________________ EOF