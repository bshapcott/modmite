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
dojo.require("dijit.MenuItem");
dojo.require("dijit.MenuBar");
dojo.require("dijit.MenuBarItem");
dojo.require("dijit.PopupMenuBarItem");
dojo.require("dojox.grid.DataGrid");
dojo.require("mite.TreeModel");
dojo.require("mite.MiteStore");
dojo.require("mite.CardWall");
dojo.require("mite.Data");
dojo.require("dojo.dnd.Source");
dojo.require("dojo.dnd.move");

//_____________________________________________________________________________
// - send the contents of the 'input' widget as an XmlHttpRequest
function ui_send_output() {
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
function ui_ad_hoc() {
	try {
		if (dijit.byId("adhocquery")) {
			return;
		}
		var tabcon = dijit.byId("tabcon");
		var adhoc = new dijit.layout.ContentPane({
			id: "adhocquery",
			title: "Ad Hoc Query",
			closable: true});
		var icp = new dijit.layout.ContentPane({
			class: "box",
			id: "icp"});
		adhoc.containerNode.appendChild(icp.domNode);
		var buttonbox = new dijit.layout.ContentPane({
			class: "box",
			id: "buttonbox"});
		adhoc.containerNode.appendChild(buttonbox.domNode);
		var ocp = new dijit.layout.ContentPane({
			class: "box",
			id: "ocp"});
		adhoc.containerNode.appendChild(ocp.domNode);
		tabcon.addChild(adhoc);

		//_________________________________________________________________________
		// BUTTONS
		var button = new dijit.form.Button({
			iconClass: "dijitEditorIcon dijitEditorIconCopy",
			onClick: function (e) {ui_send_output();},
			label: "SEND"});
		buttonbox.containerNode.appendChild(button.domNode);
		button = new dijit.form.RadioButton({
			name: "otype",
			value: "json",
			checked: true,
			id: "ojson"});
		buttonbox.containerNode.appendChild(button.domNode);
		buttonbox.containerNode.appendChild(dojo.doc.createTextNode("JSON"));
		button = new dijit.form.RadioButton({
			name: "otype",
			value: "xml",
			checked: false,
			id: "oxml"});
		buttonbox.containerNode.appendChild(button.domNode);
		buttonbox.containerNode.appendChild(dojo.doc.createTextNode("XML"));
		button = new dijit.form.Button({
			iconClass: "dijitEditorIcon dijitEditorIconCopy",
			onClick: function (e) {
				var output = dojo.byId("output");
				mite_eval_o(output.value);
			},
			label: "EVAL"});
		buttonbox.containerNode.appendChild(button.domNode);

		var input = new dijit.form.SimpleTextarea({
			id: "input",
			name: "input",
			cols: 120});
		icp.containerNode.appendChild(input.domNode);
		var output = new dijit.form.SimpleTextarea({
			id: "output",
			name: "output",
			cols: 120,
			rows: 25});
		ocp.containerNode.appendChild(output.domNode);
		icp.startup();
		ocp.startup();
		buttonbox.startup();
		adhoc.startup();
		tabcon.selectChild(adhoc);
	} catch (x) {
		console.warn(x);
		console.debug(x.stack);
		alert(x);
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
		var store = item.store ? item.store :
			new mite.MiteStore({dataId: item.id});
		var c = mite_column_names(item);
		var layout = [];
		for (var i = 0; i < c.length; i++) {
			var column = {width: '200px'};
			column.field = c[i];
			layout.push(column);
		}
		var grid = dijit.byId('grid');
		if (grid) {
			if (store === grid.store) {return;}
			grid.setStore(store);
			grid.attr('structure', layout);
			return;
		}
		grid = new dojox.grid.DataGrid({
			store: store,
			structure: layout,
            clientSort: true,
		    rowSelector: "20px",
			id: "grid",
			closable: true,
			title: item.title});
		var tabcon = dijit.byId('tabcon');
		tabcon.addChild(grid);
		grid.startup();
		tabcon.selectChild(grid);
	} catch (x) {
		console.warn(x);
		console.debug(x.stack);
		alert(x);
	}
}

//_____________________________________________________________________________
function ui_card_wall(item) {
	try {
		if (item.nodeName != "TABLE") {return;}
		var store = item.store ? item.store :
			new mite.MiteStore({dataId: item.id});
		var cardwall = dijit.byId("cardwall");
		if (cardwall) {
			if (cardwall.store === store) {return;}
			cardwall.setStore(store);
		} else {
			var cardwall = new mite.CardWall({
				id: "cardwall",
				class: "box",
				title: "Card Wall",
				closable: true,
				store: store });
			var data = new mite.Data({id: 9999, title: 9999});
			cardwall.containerNode.appendChild(data.domNode);
			var tabcon = dijit.byId("tabcon");
			tabcon.addChild(cardwall);
			data.startup();
			cardwall.startup();
			tabcon.selectChild(cardwall);
		}
	} catch (x) {
		console.warn(x);
		console.debug(x.stack);
		alert(x);
	}
}

//_____________________________________________________________________________
function ui_banner(parent) {
	var banner = new dijit.layout.ContentPane({
		region: "top",
		style: "height:60px;background-color:Beige",
		id: "banner"});
	parent.addChild(banner);
	var mb = new dijit.MenuBar();
	var sm = new dijit.Menu({});
	sm.addChild(new dijit.MenuItem({
		label: "Polling",
		onClick: function (e) {
			var table = document.getElementById("999.game.game_check.1");
			table.mite.addEventListener({
			  onSet: function (el, attr, oval, nval) {
				console.debug(attr + " " + oval + " <- " + nval);
			  }
			});
			mite_poll(function (){return "/mite/game/game_check?scid="
				+ mite_get_cell("999.game.game_scope.2", 0, 0);});}}));
	sm.addChild(new dijit.MenuItem({
		label: "Show Hide Tables",
		onClick: function (e) {
			var div = dojo.byId("mite");
			div.style.display = div.style.display == 'none' ? '' : 'none';}}));
	sm.addChild(new dijit.MenuItem({
		label: "Ad Hoc Query",
		onClick: function(evt) {ui_ad_hoc();}}));
	mb.addChild(new dijit.PopupMenuBarItem({
		label: "Utility",
		popup: sm}));
	sm = new dijit.Menu({});
	sm.addChild(new dijit.MenuItem({
		label: "Edit item #1"}));
	sm.addChild(new dijit.MenuItem({
		label: "Edit item #2"}));
	mb.addChild(new dijit.PopupMenuBarItem({
		label: "Edit",
		popup: sm}));
	banner.domNode.appendChild(mb.domNode);
	banner.startup();
	return banner;
}

//_____________________________________________________________________________
function ui_status(parent) {
	var status = new dijit.layout.ContentPane({
		region: "bottom",
		style: "height:50px;background-color:Beige",
		id: "statcon"});
	parent.addChild(status);
	status.startup();
	return status;
}

//_____________________________________________________________________________
function ui_append_status(msg) {
	var statcon = dijit.byId("statcon");
	var div = dojo.create("div", {}, statcon);
	div.innerHTML = msg;
}

//_____________________________________________________________________________
function ui_navigation(parent) {
	var tree = new dijit.Tree({
		persist: false,
		model: new mite.TreeModel({data: document.getElementById("999")}),
		id: "tree",
		onClick: function(item, node) {
			ui_grid(tree, item, node);
		},
		onDblClick: function(item, node) {
			var output = dojo.byId('output');
			output.value = "DBL " + this.model.getIdentity(item);
			return;
		}});
	tree.startup();
	var sidebar = new dijit.layout.ContentPane({
		region: "left",
		splitter: "true",
		style: "width: 250px; background-color:Beige",
		minSize: 250});
	sidebar.domNode.appendChild(tree.domNode);
	parent.addChild(sidebar);
	sidebar.startup();
}

//_____________________________________________________________________________
function ui_nav_menu() {
	// - tree node must be part of the DOM here!
	var menu = new dijit.Menu({
		targetNodeIds: ["tree"]});
	menu.addChild(new dijit.MenuItem({
		label: "Card Wall",
		iconClass:"dijitEditorIcon dijitEditorIconCopy",
		onClick: function(evt) {ui_card_wall(this.getParent().$context);}}));
	var submenu = new dijit.Menu();
	submenu.addChild(new dijit.MenuItem({
		label: "Sub 1",
		iconClass:"dijitEditorIcon dijitEditorIconCopy",
		onClick: function(evt) {}}));
	submenu.addChild(new dijit.MenuItem({
		label: "Sub 2",
		iconClass:"dijitEditorIcon dijitEditorIconCopy",
		onClick: function(evt) {}}));
	menu.addChild(new dijit.PopupMenuItem({
		popup: submenu,
		label: "My Popup"}));
			
	dojo.connect(menu, "_openMyself", menu,
		function(e) {
			var tn = dijit.getEnclosingWidget(e.target);
			menu.$context = tn.item;
			return;
		}
	);
	menu.startup();
}

//_____________________________________________________________________________
function ui_make_the_ui() {

	//_________________________________________________________________________
	// LAYOUT + PANES
	var topcontainer = new dijit.layout.BorderContainer({
		design: "headline",
		style: "height:100%; width:100%",
		id: "topcontainer"});
	var tabcon = new dijit.layout.TabContainer({
		region: "center",
		id: "tabcon",
		tabPosition: "bottom"});
	topcontainer.addChild(tabcon);
	tabcon.startup();
	ui_navigation(topcontainer);
	ui_status(topcontainer);
	ui_banner(topcontainer);
	dojo.body().removeChild(dojo.byId("loading"));
	dojo.body().insertBefore(topcontainer.domNode, dojo.body().firstChild);

	topcontainer.startup();

	ui_nav_menu();	
};

// EOF