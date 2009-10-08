//_____________________________________________________________________________
// Copyright 2008 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com
//
// @fileOverview
// - quick & dirty code to throw up a generic ui
// - also demonstrates creating Dojo widjets programmatically
// - Dojo dependencies in this module ALLOWED

if (!mite) {throw new Error("mite.js must be loaded BEFORE ui.js");}

//_____________________________________________________________________________
ui = {};

//_____________________________________________________________________________
// - send the contents of the 'input' widget as an XmlHttpRequest
ui.send_output = function() {
	var input = dojo.byId('input');
	var output = dojo.byId('output');
	var accept;
	if (dojo.byId('ojson').checked) {
		accept = 'application/json';
	} else if (dojo.byId('oxml').checked) {
		accept = 'application/xml';
	}
	mite.send_data(input.value, accept,
		function (xhr) {
			if (xhr.readyState == 4) {
				output.value = xhr.responseText;
			}
		}
	);
	return;
}

//_____________________________________________________________________________
// @todo: refactor
ui.make_the_ui = function(m) {

	//_________________________________________________________________________
	// TREE
	/*
	var tree = new dijit.Tree({
		persist: false,
		// model: new ch.TreeModel({clearinghouse: m}),
		id: "tree",
		onClick: function(item, node) {
			var output = document.getElementById('output');
			output.value = this.model.getIdentity(item) + "\n\n"
				+ dojo.toJson(item, true);
			return;
		}
	});
	*/
	// - code floating about the net, that doesn't seem to work
	//dojo.connect(tree.domNode, "ondblclick", function(evt) {
	//	var selectedNode = dijit.getEnclosingWidget(evt.target);
	//	selectedItem = selectedNode.item;
	//	alert("double click");
	//	return;
	//});

	//_________________________________________________________________________
	// MENUS
	var menu = new dijit.Menu({ style: "display: none;"});
	var menuItem1 = new dijit.MenuItem(
	{
		label: "Context Dump",
		iconClass:"dijitEditorIcon dijitEditorIconCopy",
		onClick: function(evt) {
			var output = dojo.byId('output');
			output.value = dojo.toJson(this.getParent().$context, true);
		}
	});
	menu.addChild(menuItem1);

	var menuItem2 = new dijit.MenuItem({
		label: "Change Event",
		iconClass:"dijitEditorIcon dijitEditorIconCopy",
		onClick: function(evt) {
		}
	});
	menu.addChild(menuItem2);

	menu.addChild(new dijit.MenuItem({
		label: "Child Event",
		iconClass:"dijitEditorIcon dijitEditorIconCopy",
		onClick: function(evt) {
		}
	}));

	var submenu = new dijit.Menu({ style: "display: none;"});
	var subMenuItem1 = new dijit.MenuItem({
		label: "Sub 1",
		iconClass:"dijitEditorIcon dijitEditorIconCopy",
		onClick: function(evt) {}
	});
	var subMenuItem2 = new dijit.MenuItem({
		label: "Sub 2",
		iconClass:"dijitEditorIcon dijitEditorIconCopy",
		onClick: function(evt) {}
	});
	submenu.addChild(subMenuItem1);
	submenu.addChild(subMenuItem2);
	var popup = new dijit.PopupMenuItem({
		popup: submenu,
		label: "My Popup",
		iconClass:"dijitEditorIcon dijitEditorIconCopy"
	});
	menu.addChild(popup);
	// menu.bindDomNode(tree.domNode);
	dojo.connect(menu, "_openMyself", menu,
		function(e)
		{
			var tn = dijit.getEnclosingWidget(e.target);
			menu.$context = tn.item;
			return;
		}
	);
	
	//_________________________________________________________________________
	// LAYOUT + PANES
	var topcontainer = new dijit.layout.BorderContainer({
		design: "sidebar",
		style: "height:100%; width:100%",
		id: "topcontainer"
	});
	var sidebar = new dijit.layout.ContentPane(
	{
		region: "left",
		splitter: "true",
		style: "width: 250px",
		minSize: 250
	});
	// sidebar.domNode.appendChild(tree.domNode);
	var center = new dijit.layout.ContentPane({
		region: "center"
	});
	var icp = new dijit.layout.ContentPane({
		class: "box",
		id: "icp"
	});
	center.containerNode.appendChild(icp.domNode);
	var buttonbox = new dijit.layout.ContentPane({
		class: "box",
		id: "buttonbox"
	});
	center.containerNode.appendChild(buttonbox.domNode);
	var ocp = new dijit.layout.ContentPane({
		class: "box",
		id: "ocp"
	});
	center.containerNode.appendChild(ocp.domNode);
	topcontainer.addChild(center);
	topcontainer.addChild(sidebar);
	
	//_________________________________________________________________________
	// BUTTONS
	var button = new dijit.form.Button({
		iconClass: "dijitEditorIcon dijitEditorIconCopy",
		onClick: function (e) {ui.send_output();},
		label: "SEND"
	});
	buttonbox.containerNode.appendChild(button.domNode);
	button = new dijit.form.RadioButton({
		name: "otype",
		value: "json",
		checked: true,
		id: "ojson"
	});
	buttonbox.containerNode.appendChild(button.domNode);
	buttonbox.containerNode.appendChild(dojo.doc.createTextNode("JSON"));
	button = new dijit.form.RadioButton({
		name: "otype",
		value: "xml",
		checked: false,
		id: "oxml"
	});
	buttonbox.containerNode.appendChild(button.domNode);
	buttonbox.containerNode.appendChild(dojo.doc.createTextNode("XML"));
	button = new dijit.form.Button({
		iconClass: "dijitEditorIcon dijitEditorIconCopy",
		onClick: function (e) {
			output = dojo.byId("output");
			mite.eval_o(output.value);
		},
		label: "EVAL"
	});
	buttonbox.containerNode.appendChild(button.domNode);
	var input = new dijit.form.SimpleTextarea({
		id: "input",
		name: "input",
		cols: 80
	});
	icp.containerNode.appendChild(input.domNode);
	var output = new dijit.form.SimpleTextarea({
		id: "output",
		name: "output",
		cols: 80,
		rows: 25
	});
	ocp.containerNode.appendChild(output.domNode);
	dojo.body().removeChild(dojo.byId("loading"));
	dojo.body().appendChild(topcontainer.domNode);

	//_________________________________________________________________________
	// STARTUP
	// tree.startup();
	sidebar.startup();
	icp.startup();
	ocp.startup();
	buttonbox.startup();
	center.startup();
	topcontainer.startup();
};

// EOF