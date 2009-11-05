//_____________________________________________________________________________
// Copyright 2009 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com

dojo.provide("mite.CardWall");

dojo.require("dijit.layout.ContentPane");
dojo.require("dojo.dnd.move");
dojo.require("mite.Data");

//_____________________________________________________________________________
dojo.declare(
	"mite._Card", dijit._Widget,
{
	moveable: null,

	//__________________________________________________________________________
	constructor: function(args) {
	},

	//_________________________________________________________________________	
	destroy: function() {
	},

	//____________________________________________________________________________
	postMixInProperties: function(){
		this._fieldMap = {};
	},

	//_________________________________________________________________________
	buildRendering: function() {
		this.domNode = dojo.create("div", {});
		this.domNode.style.width = 100;
		this.domNode.style.height = 100;
		this.domNode.style.border = "1px solid #000";
		this.domNode.style.padding = 10;
		this.domNode.style.backgroundColor = "wheat";
		this.moveable = new dojo.dnd.Moveable(this.domNode);
	},

	//_________________________________________________________________________	
	setAttr: function(attr, val) {
		var div = this._fieldMap[attr];
		if (!div) {
			div = dojo.create("div", {title: attr}, this.domNode);
			this._fieldMap[attr] = div;
		}
		div.innerHTML = "<b>" + attr + "</b>:" + val + "<br/>";
		if (attr === "x") {
			this.domNode.style.position = "absolute";
			this.domNode.style.left = val;
		} else if (attr === "y") {
			this.domNode.style.top = val;
			this.domNode.style.position = "absolute";
		}
	}
});

//_____________________________________________________________________________
dojo.declare(
	"mite.CardWall",
	dijit.layout.ContentPane,
{
	//_________________________________________________________________________
	store: null,

	//_________________________________________________________________________
	constructor: function(args) {
		this.store = args.store;
	},
	
	//_________________________________________________________________________
	buildRendering: function(){
		this.inherited(arguments);
		var bigdiv = dojo.create("div");
		bigdiv.style.width = 500;
		bigdiv.style.height = 500;
		this.containerNode.appendChild(bigdiv);
	},

	//_________________________________________________________________________
	postCreate: function() {
		this.setStore(this.store);
	},
	
	//_________________________________________________________________________
	postMixInProperties: function(){
		this._itemMap = {};
	},

	//_________________________________________________________________________
	destroy: function(){
	},
	
	//_________________________________________________________________________
	setStore: function(store) {
		if (this.newcon) {
			this.disconnect(this.newcon);
		}
		if (this.setcon) {
			this.disconnect(this.setcon);
		}
		this.destroyDescendants();
		this.store = store;
		var req = this.store.fetch({onItem: this.make_card, scope: this});
		this.newcon = this.connect(this.store, "onNew", "make_card");
		this.setcon = this.connect(this.store, "onSet", "card_attr");
	},
	
	//_________________________________________________________________________
	make_card: function(item, request) {
		var id = this.store.getIdentity(item);
		var card = new mite._Card();
		this._itemMap[id] = card;
		var attr = this.store.getAttributes(item);
		for (var i = 0; i < attr.length; i++) {
			var val = this.store.getValue(item, attr[i]);
			card.setAttr(attr[i], val);
		}
		this.containerNode.appendChild(card.domNode);
	},
	
	//_________________________________________________________________________
	card_attr: function(item, attr, oval, nval) {
		var id = this.store.getIdentity(item);
		var card = this._itemMap[id];
		card.setAttr(attr, nval);
	}


});

//_________________________________________________________________________ EOF