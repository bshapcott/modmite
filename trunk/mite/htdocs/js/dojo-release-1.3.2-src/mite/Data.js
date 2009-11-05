//_____________________________________________________________________________
// Copyright 2009 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com

dojo.provide("mite.Data");

dojo.require("dijit._Widget");

//_____________________________________________________________________________
dojo.declare(
	"mite.Data", dijit._Widget,
{
	query: null,
	
	poll: null,
	
	//__________________________________________________________________________
	constructor: function(args) {
	},

	//_________________________________________________________________________	
	destroy: function() {
	},

	//_________________________________________________________________________
	buildRendering: function() {
		// @todo - float
		this.domNode = mite_create_element("div", null, mite_container);
		dojo.style(this.domNode, 
			{width: 10, height: 10, border: "1px solid #000",
			 padding: 10, backgroundColor: "red"});
		this.domNode.mite.addEventListener(this);
	},

	//_________________________________________________________________________
	startup: function() {
		this.inherited(arguments);
		mite_send_eval("/mite/game/meta?qid=" + this.id);
	},
	
	//_________________________________________________________________________
	// - propagated from the mite layer
	// - rolled up child changes (new, delete on children)
	// @see dijit.tree.model
	onChildrenChange: function(parent, children) {},
	
	//_________________________________________________________________________
	// - propagated from the mite layer
	// - fine grained item attribute changes
	// @see dojo.data.api.Notification
	onSet: function(element, attr, oval, nval) {},
	
	//_________________________________________________________________________
	// - propagated from the mite layer
	// - rolled up item attribute changes
	// @see dijit.tree.model
	onChange: function(element) {},
	
	//_________________________________________________________________________
	// - propagated from the mite layer
	// - fine grained child creation notification
	// @see dijit.tree.model
	// @see dojo.data.api.Notification
	// @see dijit.tree.model
	onNew: function(element) {},

	//_________________________________________________________________________
	// - propagated from the mite layer
	// - fine grained child delete notification
	// @see dojo.data.api.Notification
	onDelete: function(element) {}			
});

//_________________________________________________________________________ EOF