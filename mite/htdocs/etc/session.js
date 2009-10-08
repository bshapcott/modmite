//_____________________________________________________________________________
// Copyright 2008 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com
//
// @fileOverview
// - session is the top level widget and corresponds to page
// - that is, there it has no Dojo widget, and represents a blank page
//   on which widgets may be placed

if (!mite) {throw new Error("mite.js must be loaded BEFORE session.js");}

//_____________________________________________________________________________
session = {};

//_____________________________________________________________________________
session.datify = function session_datify(data, node) {
	node.appendChild(data);
}

//_____________________________________________________________________________
session.evaluate = function session_evaluate(expr) {
	return document.evaluate(expr, document, null,
		XPathResult.ORDERED_NODE_SNAPSHOT_TYPE, null);
}

//_____________________________________________________________________________
// - get the session information
session.get = function session_get() {
	// \todo actual session identifier
	var me = this;
	mite.send_data('/mite/game/qtask?story=1', 'application/xml',
		function (xhr) {
			if (xhr.readyState == 4) {
				me.responseText = xhr.responseText;
				me.datify(xhr.responseXML.childNodes[0], document.body);
			}
		}
	);
	return;
}

//_________________________________________________________________________ EOF