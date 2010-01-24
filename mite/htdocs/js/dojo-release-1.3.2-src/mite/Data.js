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
		this.index = 0;
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
		if (this.query) {
			if (this.query instanceof Array) {
				this.sendNext();
			} else {
				mite_send_eval(this.query, this.onQueryComplete, this);
			}
		}
	},
	
	//_________________________________________________________________________
	sendNext: function() {
		mite_send_eval(this.query[this.index++],
			this.index === this.query.length ? this.onQueryComplete : arguments.callee,
			this);
	},

	//_________________________________________________________________________
	// - all one-time (non-polling) queries have been completed
	onQueryComplete: function() {},
	
	//_________________________________________________________________________
	// - propagated verbatim from the mite layer
	// - for users that would rather listen on the widget than the mite layer
	// - rolled up child changes (new, delete on children)
	// @see dijit.tree.model
	onChildrenChange: function(parent, children) {},
	
	//_________________________________________________________________________
	// - propagated verbatim from the mite layer
	// - for users that would rather listen on the widget than the mite layer
	// - fine grained item attribute changes
	// @see dojo.data.api.Notification
	onSet: function(element, attr, oval, nval) {},
	
	//_________________________________________________________________________
	// - propagated verbatim from the mite layer
	// - for users that would rather listen on the widget than the mite layer
	// - rolled up item attribute changes
	// @see dijit.tree.model
	onChange: function(element) {},
	
	//_________________________________________________________________________
	// - propagated verbatim from the mite layer
	// - for users that would rather listen on the widget than the mite layer
	// - fine grained child creation notification
	// @see dijit.tree.model
	// @see dojo.data.api.Notification
	// @see dijit.tree.model
	onNew: function(element) {},

	//_________________________________________________________________________
	// - propagated verbatim from the mite layer
	// - for users that would rather listen on the widget than the mite layer
	// - fine grained child delete notification
	// @see dojo.data.api.Notification
	onDelete: function(element) {}

});

//_________________________________________________________________________ EOF