//_____________________________________________________________________________
// Copyright 2008 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com

//_____________________________________________________________________________
// - x-browser cruft
function getXHR() {
	var xhr = null;
	if (window.XMLHttpRequest) {
		xhr = new XMLHttpRequest;
	} else if (typeof ActiveXObject != "undefined") {
		xhr = new ActiveXObject("Microsoft.XMLHTTP");
	}
	return xhr;
}

//_____________________________________________________________________________
function init() {
	var xhr = getXHR();
	if (xhr) {
		xhr.onreadystatechange = function () {onReadyStateChange(xhr);}
		xhr.open("GET", "slite/game/session", true);
		xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
		xhr.send(null);
	}
    return;
}

//_____________________________________________________________________________
function post_any(name, doc) {
	var xhr = getXHR();
	if (xhr) {
		xhr.onreadystatechange = function () {onReadyStateChange(xhr);}
		xhr.open("POST", "slite/game/" + name, true);
		xhr.setRequestHeader("Content-Type", "application/xml");
		xhr.send(doc);
	}
    return;
}

//_____________________________________________________________________________
function post_data(name, data) {
	var new_doc = document.implementation.createDocument(null, null, null);
	var n = new_doc.createElement("slite");
	new_doc.appendChild(n);
	var c = new_doc.createElement("game");
	n.appendChild(c);
	toXML.call(data, new_doc, c, name);
	post_any(name, new_doc);
	return;
}

//_____________________________________________________________________________
function onReadyStateChange(xhr) {
	if (xhr.readyState == 4) {
		var n = new Object;
		toJS.call(n, xhr.responseXML);
		notify.call(n, notify.listeners);
	}
	return;
}

//_____________________________________________________________________________
// - refine with channels or fine-grained?
function notify(listeners) {
	for (var child in this) {
		var id = child.id;
		// tktk: iterate listeners before children?
		for (var i in listeners) {
			var listener = listeners[i];
			var func;
			var target;
			switch(typeof listener){
				case "object":
					func = listener[child];
					target = listener;
					break;
				case "function":
					func = listener;
					break;
			}
			if (func) {
				var a = this[child] instanceof Array ? this[child] : [this[child]];
				for (var i in a) {
					var next = func.call(target, child, a[i]);
					if (next) {
						notify.call(a[i], next instanceof Array ? next : [next]);
					}
				}
			}			
		}
	}
	return;
}

//_____________________________________________________________________________
notify.listeners = new Array;

//_____________________________________________________________________________
// - generic conversion of XML-derived data to HTML
function toHTML(name, data) {
	var child;
	var div = document.getElementById(name + "-view-" + data.id);
	if (!div) {
		var body = document.getElementsByTagName("body")[0];
		div = document.createElement("div");
		div.className = name + "-view";
		div.id = name + "-view-" + data.id;
		div[name] = data;
		for (var n in data) {
			if (typeof data[n] != "function") {
				child = document.createElement("div");
				child.className = name + "-" + n;
				var label = document.createElement("div");
				label.className = name + "-" + n + "-label";
				label.appendChild(document.createTextNode(n));
				child.appendChild(label);
				var value = document.createElement("textarea");
				if ("id" == n || "timestamp" == n) {
					value.readOnly = true;
				} else {
					value.onchange = function() {this[name][this.member] = this.value;}
				}
				value.id = name + "-" + n + "-" + data.id;
				value.className = "class", name + "-" + n + "-value";
				value.value = data[n];
				value[name] = data;
				value.member = n;
				child.appendChild(value);
				div.appendChild(child);
			}
		}
		child = document.createElement("button");
		child.id = name + "-button-" + data.id;
		child.type = "button";
		child.className = name + "-button-submit";
		child.name = "POST";
		child.onclick = function() {post_data(name, data);}
		child.appendChild(document.createTextNode(name + " " + data.id));
		div.appendChild(child);
		body.appendChild(div);
	} else {
		for (var n in data) {
			var value = document.getElementById(name + "-" + n + "-" + data.id);
			if (value.value != data[n]) {
				value.value = data[n];
				value.style.color = "red";
			} else {
				value.style.color = "";
			}
			value[name] = data;
		}
		var child = document.getElementById(name + "-button-" + data.id);
		child.onclick = function() {post_data(name, data);}
	}
	return;
}

//_____________________________________________________________________________
function View() {
	notify.listeners.push(this);
	return;
}

//_____________________________________________________________________________
View.prototype.result = function(name, result) {
	return this;
}

//_____________________________________________________________________________
View.prototype.task = toHTML;

//_____________________________________________________________________________
View.prototype.session = toHTML;

//_____________________________________________________________________________
var the_view = new View;

//_____________________________________________________________________________
// - convert an arbitrary JavaScript object to XML
function toXML(doc, node, name) {
	var n = doc.createElement(name);
	if (this instanceof String) {
		// - interpret as element rather than attribute
		n.appendChild(document.createTextNode(this));
		node.appendChild(n);
	} else if (typeof this === "object") {
		for (var c in this) {
			if (this[c] != null) {
				switch (typeof this[c]) {
				case "object":
					if (this[c] instanceof Array) {
						for (var n in this[c]) {
							toXML.call(this[c][n], doc, n, c);
						}
					} else {
						toXML.call(this[c], doc, n, c);
					}
					break;
				case "number":
				case "string":
				case "boolean":
					// - map simplex members to attributes
					n.setAttribute(c, this[c])
				}
			}
		}
	}
	node.appendChild(n);
	return;
}

//_____________________________________________________________________________
// - convert XML to a JavaScript object
// - the JavaScript object is simply a data container
function toJS(node) {
	if (node.attributes != null) {
		for (var i = 0; i < node.attributes.length; i++) {
			var a = node.attributes[i];
			this[a.nodeName] = a.nodeValue;
		}
	}
	for (var i = 0; i < node.childNodes.length; i++) {
		var n = node.childNodes[i];
		var c = null;
		for (var j = 0; j < n.childNodes.length; j++) {
			if (n.childNodes[j].nodeType == 3) {
									if (c != null) {
					c = String.concat(c, n.childNodes[j].nodeValue);			
				} else {
					c = new String(n.childNodes[j].nodeValue);
				}
			}
		}
		if (c == null) {
			c = new Object();
		}
		toJS.call(c, n);
		if (this[n.tagName] == undefined) {
			this[n.tagName] = c;
		} else if (this[n.tagName] instanceof Array) {
			this[n.tagName].push(c);
		} else {
			this[n.tagName] = new Array(this[n.tagName], c);
		}
	}
	return;
}

//_____________________________________________________________________________
window.onload = init;

// EOF