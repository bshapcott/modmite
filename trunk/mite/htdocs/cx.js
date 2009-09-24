//_____________________________________________________________________________
// Copyright 2008 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com
//
// - server connector (cx)
// - Dojo dependencies in this module NOT ALLOWED
//
// - identifier conventions in this module
//   - identifiers synthesized from the database (e.g. table, view and column
//     names) are converted as is
//   - identifiers defined in this module are prefixed with a '$' (although
//     it is possible to use '$' as a prefix in sqlite identifiers, it is
//     uncommon because it conflicts with the syntax for binding parameters and
//     must therefore be quoted)
//   - identifiers prefixed with '$$' denote fields which may form a cycle
//     in a data structure, which can be used to inform traversal algorithms
//     (such as serializers) -- kludgy, but cheaper than cycle detection
//   - commonly used identifiers are:
//     - '$s' - "s"ource function that created the object
//     - '$n' - object's "n"ame
//     - '$t' - object's "t"ype by constructor name
//     - '$$p' - object's "p"arent

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
// - object identifier
// todo: member func?
cx.oid = function cx_oid(obj) {
	if (obj.$n === undefined) {
		// - fake an id
		// @todo - ensure this is truly unique in context
		obj.$n = Math.random().toString(36).substring(2);
		console.error("~oid: " + obj);
	}
	return (obj.$$p ? cx.oid(obj.$$p) + '/' : "") + obj.$n;
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

//_____________________________________________________________________________
// - create an object representing a row of the result set
// - if the object already exists, mix in the result set row
// @param	db		database object
// @param	t		SQL query name
// @param	s		statement index in 'q'
// @param	r		row in result set representing the object
cx.map_obj = function cx_map_obj(root, db, t, s, r) {
}

//_____________________________________________________________________________
// todo: add db padding
cx.eval_o = function cx_eval_o(data, root) {
	try {
		var it = eval('(' + data + ')');
	} catch (e) {
		console.warn(e + data);
		return;
	}
	for (var dpad in it) {for (var db in it[dpad]) {
		console.debug("[" + dpad + "][" + db + "]");
		var div = document.createElement("div");
		div.setAttribute("class", db);
		document.body.appendChild(div);
		for (var pad in it[dpad][db]) {for (var sql in it[dpad][db][pad]) {
			console.debug("[" + dpad + "][" + db + "][" + pad + "][" + sql + "]");
			for (var stmt in it[dpad][db][pad][sql]) {
				console.debug("[" + dpad + "][" + db + "][" + pad + "][" + sql + "][" + stmt + "]");
				var table = document.createElement("table");
				div.appendChild(table);
				for (var row in it[dpad][db][pad][sql][stmt]) {
					console.debug("[" + dpad + "][" + db + "][" + pad + "][" + sql + "][" + stmt + "][" + row + "]");
					var r = it[dpad][db][pad][sql][stmt][row];
					var tr = document.createElement("tr");
					table.appendChild(tr);
					for (var c in r) {
						var span = document.createElement("span");
						span.innerHTML = r[c];
						var td = document.createElement("td");
						td.appendChild(span);
						tr.appendChild(td);
					}
				} // row
			} // stmt
		}} // pad + sql
	}} // pad + db
	this.delta();
	return;
}

//_____________________________________________________________________________
cx.query = function cx_query(q) {
	var f = Math.random().toString(36).substring(2);
	// @debug - non-portable
	q.$s = arguments.callee.name;
	root = cx.prod(cx.$root, f, function() {
		return {
			$container: true,
			$n: f
		};
	});
	cx.prod(root, '$query', function() {return q;});
	// @todo - implement non-Dojo toJson
	cx.send_data(dojo.toJson(q), 'application/json', function(xhr) {
		if (xhr.readyState == 4) {
			cx.eval_o(xhr.responseText, root);
		}
	});
	return;
}

// EOF