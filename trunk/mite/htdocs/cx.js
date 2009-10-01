//_____________________________________________________________________________
// Copyright 2008 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com
//
// - mite server connector (cx)
// - vanilla JavaScript ONLY in this module
// - no JS libraries allowed (Dojo, YUI, etc.)

//_____________________________________________________________________________
cx = {};

//_____________________________________________________________________________
// - x-browser cruft
// @todo: Msxml2.XMLHTTP, Microsoft.XMLHTTP, Msxml2.XMLHTTP.4.0
cx.get_xhr = function cx_get_xhr() {
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
cx.send_data = function cx_send_data(data, accept, cb) {
	var xhr = cx.get_xhr();
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
		mime_type = 'application/json';
	} else if (/^\s*\</.test(data)) {
		// - xml is any text starting with '<'
		mime_type = 'application/xml';
	} else if (/\S+\s+\S+/.test(data)) {
		// - SQL is at least two non-whitespace sequences (i.e. tokens)
		mime_type = 'application/x-sql';
	} else if (/\s*\S+\s*$/.test(data)) {
		// - a URL is single unbroken sequence of non-whitespace characters
		mime_type = '';
	} else {
		throw new Error("The MIME type of the input text cannot be detected.");
	}
	switch (mime_type) {
		case 'application/x-sql':
			url = "/mite/game";
			break;
		case '':
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
    return;
}

//_____________________________________________________________________________
// - custom serializer breaks cyclic serialization in dojo.toJson
// todo: prototype func
cx.toJson = function cx_toJson() {
	var obj = {};
	for (var i in this) {
		if (i !== "json" && i !== "__json__") {
			obj[i] = /^\$\$/.test(i) ? cx.oid(this[i]) : this[i];
		}
	}
	// - serialize this instead of actual object
	return obj;
}

//_____________________________________________________________________________'
cx.find_or_create = function cx_find_or_create(doc, id, type, parent) {
	var e = doc.getElementById(id);
	if (!e) {
		e = document.createElement(type);
		e.setAttribute("id", id);
		e.setAttribute("class", type);
		parent.appendChild(e);
	}
	return e;
}

//_____________________________________________________________________________
cx.lookup = function cx_lookup(table, kvp) {
	var r = [];
	var tr = table.getElementsByTagName("tbody")[0].getElementsByTagName("tr");
	for (var i = 0; i < tr.length; i++) {
		var td = tr[i].getElementsByTagName("td");
		var f = true;
		for (var k in kvp) {
			if (kvp[k] != td[k].innerHTML) {
				f = false;
				break;
			}
		}
		if (f) {
			var s = [];
			for (var j = 0; j < td.length; j++) {
				s.push(td[j].innerHTML);
			}
			r.push(s);
		}
	}
	return r;
}

//_____________________________________________________________________________
cx.microformat = function cx_microformat(qid, table, db, sql, stmt, rows) {
	var caption = table.getElementsByTagName("caption");
	if (caption.length == 0) {
		caption = document.createElement("caption");
		caption.innerHTML = db + " " + sql + " " + stmt;
		table.insertBefore(caption, table.firstChild);
	}
	var tbody = table.getElementsByTagName("tbody");
	if (tbody.length == 0) {
		tbody = document.createElement("tbody");
		table.appendChild(tbody);
	} else {
		tbody = tbody[0];
	}
	for (var row in rows) {
		var r = rows[row];
		var tr = document.createElement("tr");
		tbody.appendChild(tr);
		for (var c in r) {
			var td = document.createElement("td");
			td.innerHTML = r[c];
			tr.appendChild(td);
		}
	} // row
	return;
}

//_____________________________________________________________________________
cx.metasql = function cx_metasql(qid, table, db, sql, stmt) {
	var thead = table.getElementsByTagName("thead");
	if (thead.length > 0) {
		return;
	}
	var meta = /^meta/.test(sql) ? sql : "meta" + sql;
	thead = document.createElement("thead");
	table.appendChild(thead);
	var metasql = document.getElementById(db + "." + meta + "." + qid + "." + 1);
	if (!metasql) {
		return;
	}
	var metatable = document.getElementById(db + "." + meta + "." + qid + "." + 2);
	var mtl = metatable.getElementsByTagName("tbody")[0].getElementsByTagName("tr")[0].getElementsByTagName("td").length;
	var tfoot;
	if (metatable) {
		tfoot = document.createElement("tfoot");
		table.appendChild(tfoot);
	}
	var thr = [];
	var tfr = [];
	var s = this.lookup(metasql, {0: sql, 1: stmt});
	for (var i in s) {
		for (var j in s[i]) {
			if (!thr[j]) {
				thr[j] = document.createElement("tr");
				thead.appendChild(thr[j]);
			}
			var td = document.createElement("td");
			td.setAttribute("class", "meta");
			td.innerHTML = s[i][j];
			thr[j].appendChild(td);
		}
		var t = this.lookup(metatable, {0: s[i][4], 2: s[i][5]});
		var len = t.length > 0 ? t[0].length : mtl;
		for (var j = 0; j < len; j++) {
			if (!tfr[j]) {
				tfr[j] = document.createElement("tr");
				tfoot.appendChild(tfr[j]);
			}
			var td = document.createElement("td");
			td.setAttribute("class", "meta");
			td.innerHTML = t.length > 0 ? t[0][j] : "";
			tfr[j].appendChild(td);
		}
	}
	return;
}

//_____________________________________________________________________________
// @todo - notifications for data changes
// - DIV can be ground zero for connecting to other subsystems
cx.eval_o = function cx_eval_o(data, root) {
	try {
		var it = eval('(' + data + ')');
	} catch (e) {
		console.warn("cx_eval_o");
		console.warn(e);
		return;
	}
	// - convert the input data into microformat
	for (var pass = 0; pass < 2; pass++) {
		for (var dpad in it) {for (var db in it[dpad]) {
			for (var pad in it[dpad][db]) {for (var sql in it[dpad][db][pad]) {
				var qid = it[dpad][db][pad][sql][0][0]['qid'];
				var div = this.find_or_create(document, qid, "div", document.body);
				for (var stmt in it[dpad][db][pad][sql]) {
					var table = this.find_or_create(document, db + "." + sql + "." + qid + "." + stmt,
						"table", div);
					switch (pass) {
					case 0:
						this.microformat(qid, table, db, sql, stmt, it[dpad][db][pad][sql][stmt]);
						break;
					case 1:
						this.metasql(qid, table, db, sql, stmt);
						break;
					}
				} // stmt
			}} // pad + sql
		}} // pad + db
	} // pass
	return;
}

//_________________________________________________________________________ EOF