//_____________________________________________________________________________
// Copyright 2008 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com
//
// @todo - rename this to mite.js
// - mite server connector (mite)
// - vanilla JavaScript ONLY in this module
// - no JS libraries allowed (Dojo, Prototype, jQuery, etc.)
//   - unfortunately that means no CSS Selectors or XPath, because IE doesn't
//     have a native implementation

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
function mite_toJson() {
	var obj = {};
	for (var i in this) {
		if (i !== "json" && i !== "__json__") {
			obj[i] = /^\$\$/.test(i) ? mite_oid(this[i]) : this[i];
		}
	}
	// - serialize this instead of actual object
	return obj;
}

//_____________________________________________________________________________'
function mite_find_or_create(doc, id, type, parent) {
	var e = doc.getElementById(id);
	if (!e) {
		e = document.createElement(type);
		e.setAttribute("id", id);
		e.setAttribute("class", type);
		s.style.display = 'none';
		parent.appendChild(e);
	}
	return e;
}

//_____________________________________________________________________________
function mite_lookup(table, nvp) {
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
function mite_pk_lookup(table, row) {
	var tfoot = table.getElementsByTagName("tfoot");
	if (tfoot.length == 0) {
		return [];
	}
	tfoot = tfoot[0];
	var thead = table.getElementsByTagName("thead");
	if (thead.length == 0) {
		return [];
	}
	thead = thead[0];
	// @todo - don't hardcode row # (from meta<X> column)
	var nvp;
	var tdf = tfoot.getElementsByTagName("tr")[6].getElementsByTagName("td");
	var tdh = thead.getElementsByTagName("tr")[6].getElementsByTagName("td");
	for (var i = 0; i < tdf.length; i++) {
		if (tdf[i].innerHTML == "1") {
			if (!nvp) {
				nvp = {};
			}
			nvp[i] = row[tdh[i].innerHTML];
		}
	}
	return nvp ? mite_tr_lookup(table, nvp) : [];
}

//_____________________________________________________________________________
function mite_microformat(qid, table, db, sql, stmt, rows) {
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
		var p = mite_pk_lookup(table, r);
		var tr;
		// @todo - check at most one member of 'p'
		if (p.length > 0) {
			tr = p[0];
			var td = tr.getElementsByTagName("td");
			var i = 0;
			for (var c in r) {
				td[i++].innerHTML = r[c];
			}			
		} else {
			tr = document.createElement("tr");
			tbody.appendChild(tr);
			for (var c in r) {
				var td = document.createElement("td");
				td.innerHTML = r[c];
				tr.appendChild(td);
			}
		}
	} // row
	return;
}

//_____________________________________________________________________________
function mite_metasql(qid, table, db, sql, stmt) {
	var thead = table.getElementsByTagName("thead");
	if (thead.length > 0) {
		return;
	}
	thead = document.createElement("thead");
	table.appendChild(thead);
	var metasql = document.getElementById(db + ".meta." + qid + "." + 1);
	if (!metasql) {
		return;
	}
	var metatable = document.getElementById(db + ".meta." + qid + "." + 2);
	var mtl = metatable.getElementsByTagName("tbody")[0].getElementsByTagName("tr")[0].getElementsByTagName("td").length;
	var tfoot;
	if (metatable) {
		tfoot = document.createElement("tfoot");
		table.appendChild(tfoot);
	}
	var thr = [];
	var tfr = [];
	var s = mite_lookup(metasql, {0: sql, 1: stmt});
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
		var t = mite_lookup(metatable, {0: s[i][4], 2: s[i][5]});
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
function mite_get_session() {
	return mite_get_cell("game.session_start.999.1", 0, 0);
}

//_____________________________________________________________________________
// @todo - notifications for data changes
// - DIV can be ground zero for connecting to other subsystems
function mite_eval_o(data) {
	try {
		var it = eval('(' + data + ')');
	} catch (e) {
		console.warn("mite_eval_o");
		console.warn(e);
		return;
	}
	// - convert JSON to microformat
	for (var pass = 0; pass < 2; pass++) {
		for (var dpad in it) {for (var db in it[dpad]) {
			for (var pad in it[dpad][db]) {for (var sql in it[dpad][db][pad]) {
				var qid;
				// - look for query identifier
				for (var i = 0; i < 3; i++) {
					try {
						qid = it[dpad][db][pad][sql][i][0]['qid'];
					} catch (e) {
					}
					if (qid) {
						break;
					}
				}
				if (!qid) {
					qid = "666";
				}
				var div = mite_find_or_create(document, qid, "div", document.body);
				for (var stmt in it[dpad][db][pad][sql]) {
					var table = mite_find_or_create(document, db + "." + sql + "." + qid + "." + stmt,
						"table", div);
					switch (pass) {
					case 0:
						mite_microformat(qid, table, db, sql, stmt, it[dpad][db][pad][sql][stmt]);
						break;
					case 1:
						mite_metasql(qid, table, db, sql, stmt);
						break;
					}
				} // stmt
			}} // pad + sql
		}} // pad + db
	} // pass
	return;
}

//__________________________________________________________________________
// - quick & dirty scratch test
function mite_scratch_test(funcs) {
  if (funcs.length == 0) {
	console.debug("empty funcs");
  }
  var data = (funcs.shift())();
  // var input = dijit.byId("input");
  // input.attr("value", data);
  mite_send_data(data, 'application/json', function (xhr) {
    if (xhr.readyState == 4) {
      // var output = dijit.byId("output");
      // output.attr("value", xhr.responseText);
      mite_eval_o(xhr.responseText);
      if (funcs.length > 0) mite_scratch_test(funcs);
    }
  });
  return;
}

//__________________________________________________________________________
function mite_poll_test(query) {
	mite_scratch_test([query]);
	return;
}

//__________________________________________________________________________
function mite_poll(query) {
	setInterval(mite_poll_test, 1000, query);
	return;
}

//_________________________________________________________________________ EOF