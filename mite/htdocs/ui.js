//_____________________________________________________________________________
// Copyright 2008 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com
//
// @fileOverview
// - quick & dirty code to throw up a generic ui
// - also demonstrates creating Dojo widjets programmatically
// - Dojo dependencies in this module ALLOWED

dojo.require("dijit.layout.BorderContainer");
dojo.require("dijit.layout.TabContainer");
dojo.require("dijit.layout.ContentPane");
dojo.require("dijit.form.Button");
dojo.require("dijit.form.CheckBox");
dojo.require("dijit.form.SimpleTextarea");
dojo.require("dijit.Tree");
dojo.require("dijit.Menu");
dojo.require("dojox.grid.DataGrid");
dojo.require("mite.TreeModel");
dojo.require("mite.MiteStore");
dojo.require("dojo.dnd.Source");
dojo.require("dojo.dnd.move");

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
	mite_send_data(input.value, accept,
		function (xhr) {
			if (xhr.readyState == 4) {
				output.value = xhr.responseText;
			}
		}
	);
	return;
}

//_____________________________________________________________________________
function ui_raw_grid() {
	try {
		if (dijit.byId("rawgrid")) {
			return;
		}
		function get(inRowIndex){
			return [this.index, inRowIndex].join(', ');
		}
		
		function get2(inRowIndex) {
			return "Yabba";
		}

		//		define the grid structure:
		var structure = [
			{cells:
				[
					[
						{ name: "Alpha", width: 6 },
						{ name: "Beta" },
						{ name: "Gamma", get: get2 }
					]
				],
			 width: "800px"
			}
		];
		var tabcon = dijit.byId("tabcon");
		var grid = new dojox.grid._Grid({
			id: 'rawgrid',
			get: get,
			structure: structure,
			title: "Raw Grid"
		});
		tabcon.addChild(grid);
		grid.startup();
	} catch (e) {
		alert(e);
	}
}

//_____________________________________________________________________________
function ui_grid(tree, item, node) {
	try {
		if (item.nodeName != "TABLE") {
			var output = dojo.byId('output');
			output.value = "CLICK " + tree.model.getIdentity(item);
			return;
		}
		var store = new mite.MiteStore({
				dataId: item.id
			});
		var c = mite_column_names(item);
		var layout = [];
		for (var i = 0; i < c.length; i++) {
			var column = {width: '200px'};
			column.field = c[i];
			layout.push(column);
		}
		var grid = dijit.byId('grid');
		if (grid) {
			grid.setStore(store);
			grid.attr('structure', layout);
			return;
		}
		grid = new dojox.grid.DataGrid({
				store: store,
				structure: layout,
	            clientSort: true,
			    rowSelector: '20px',
				id: 'grid',
				title: item.title
			});
		var tabcon = dijit.byId('tabcon');
		tabcon.addChild(grid);
		
		// dojo.body().appendChild(grid.domNode);
		grid.startup();
	} catch (e) {
		alert(e);
	}
}

//_____________________________________________________________________________
function ui_map_pane() {
	try {
		if (dijit.byId("mappane")) {
			return;
		}
		var tabcon = dijit.byId("tabcon");
		var bigdiv = document.createElement("div");
		bigdiv.style.width = 500;
		bigdiv.style.height = 500;
		var cp = new dijit.layout.ContentPane({
			id: "mappane",
			class: "box",
			title: "Map Pane"
		});
		cp.containerNode.appendChild(bigdiv);
		var button = new dijit.form.Button({
			id: "mapbutton",
			iconClass: "dijitEditorIcon dijitEditorIconCopy",
			onClick: function (e) {
				alert("BANZAI!");
			},
			style: "position:absolute;top:50px;left:50px",
			label: "INF"
		});
		var buttondiv = dojo.create("div", {}, bigdiv);
		buttondiv.style.width = 100;
		buttondiv.style.height = 100;
		buttondiv.style.border = "1px solid #000";
		buttondiv.style.padding = 10;
		buttondiv.style.backgroundColor = "green";
		buttondiv.appendChild(button.domNode);
		tabcon.addChild(cp);
		cp.startup();
		var moveable = new dojo.dnd.move.parentConstrainedMoveable(buttondiv);
		dojo.connect(moveable, "onMove", buttondiv, function(mover, leftTop) {
			var p = dojo.byId("mxy");
			if (!p) {
				p = dojo.create("p", {id: "mxy"}, buttondiv);
			}
			p.innerHTML = "(" + leftTop.l + ", " + leftTop.t + ")";					
		});
	} catch (e) {
		alert(e);
	}
}

//_____________________________________________________________________________
function ui_card_wall(item) {
	try {
		if (item.nodeName != "TABLE") {
			return;
		}
		var store = new mite.MiteStore({
				dataId: item.id
			});
		
		if (!dijit.byId("cardwall")) {
			var tabcon = dijit.byId("tabcon");
			var bigdiv = document.createElement("div");
			bigdiv.style.width = 500;
			bigdiv.style.height = 500;
			var cp = new dijit.layout.ContentPane({
				id: "cardwall",
				class: "box",
				title: "Card Wall"
			});
			cp.containerNode.appendChild(bigdiv);
			tabcon.addChild(cp);
			cp.startup();
		}
		function make_card(item, request) {
			var card = dojo.create("div", {}, bigdiv);
			card.style.width = 100;
			card.style.height = 100;
			card.style.border = "1px solid #000";
			card.style.padding = 10;
			card.style.backgroundColor = "wheat";
			var attr = store.getAttributes(item);
			var html = "";
			for (var i = 0; i < attr.length; i++) {
				var val = store.getValue(item, attr[i]);
				html = html + "<b>" + attr[i] + "</b>:" + val + "<br/>";
			}
			card.innerHTML = html;
			var moveable = new dojo.dnd.Moveable(card);
		}
		var req = store.fetch({onItem: make_card});
	} catch (x) {
		alert (x);
	}
}

//_____________________________________________________________________________
// @todo: refactor
ui.make_the_ui = function() {

	//_________________________________________________________________________
	// TREE
	var div = document.getElementById('999');
	var tree = new dijit.Tree({
		persist: false,
		model: new mite.TreeModel({data: div}),
		id: "tree",
		onClick: function(item, node) {
			ui_grid(tree, item, node);
		},
		onDblClick: function(item, node) {
			var output = dojo.byId('output');
			output.value = "DBL " + this.model.getIdentity(item);
			return;
		}
	});

	//_________________________________________________________________________
	// MENUS
	var menu = new dijit.Menu({ style: "display: none;"});
	var menuItem1 = new dijit.MenuItem(
	{
		label: "Make Map Pane",
		iconClass:"dijitEditorIcon dijitEditorIconCopy",
		onClick: function(evt) {ui_map_pane();}
	});
	menu.addChild(menuItem1);

	var menuItem2 = new dijit.MenuItem({
		label: "Make Grid Tab",
		iconClass:"dijitEditorIcon dijitEditorIconCopy",
		onClick: function(evt) {ui_raw_grid();}
	});
	menu.addChild(menuItem2);

	menu.addChild(new dijit.MenuItem({
		label: "Card Wall",
		iconClass:"dijitEditorIcon dijitEditorIconCopy",
		onClick: function(evt) {ui_card_wall(this.getParent().$context);}
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
	menu.bindDomNode(tree.domNode);
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
	sidebar.domNode.appendChild(tree.domNode);
	var tabcon = new dijit.layout.TabContainer({
		region: "center",
		id: "tabcon"
	});
	var center = new dijit.layout.ContentPane({
		region: "center",
		id: "center",
		title: "Ad Hoc Query"
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
	tabcon.addChild(center);
	topcontainer.addChild(tabcon);
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
			var output = dojo.byId("output");
			mite_eval_o(output.value);
		},
		label: "EVAL"
	});
	buttonbox.containerNode.appendChild(button.domNode);
	button = new dijit.form.Button({
		iconClass: "dijitEditorIcon dijitEditorIconCopy",
		onClick: function (e) {
			var div = dojo.byId("mite");
			div.style.display = div.style.display == 'none' ? '' : 'none';
		},
		label: "SHOW/HIDE TABLES"
	});
	buttonbox.containerNode.appendChild(button.domNode);

	button = new dijit.form.Button({
		iconClass: "dijitEditorIcon dijitEditorIconCopy",
		onClick: function (e) {
			mite_poll(function (){return "/mite/game/game_check?scid="
				+ mite_get_cell("999.game.game_scope.2", 0, 0);});
		},
		label: "POLL"
	});
	buttonbox.containerNode.appendChild(button.domNode);

	var input = new dijit.form.SimpleTextarea({
		id: "input",
		name: "input",
		cols: 120
	});
	icp.containerNode.appendChild(input.domNode);
	var output = new dijit.form.SimpleTextarea({
		id: "output",
		name: "output",
		cols: 120,
		rows: 25
	});
	ocp.containerNode.appendChild(output.domNode);
	dojo.body().removeChild(dojo.byId("loading"));
	dojo.body().insertBefore(topcontainer.domNode, dojo.body().firstChild);

	//_________________________________________________________________________
	// STARTUP
	tree.startup();
	sidebar.startup();
	icp.startup();
	ocp.startup();
	buttonbox.startup();
	center.startup();
	tabcon.startup();
	topcontainer.startup();
};

// EOF