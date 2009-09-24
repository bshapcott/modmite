//_____________________________________________________________________________
// Copyright 2008 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com

dojo.provide("ch.TreeModel");

//_____________________________________________________________________________
// - based on dijit.tree.TreeStoreModel, but doesn't require dijit/Tree.js
//   to be loaded
// - adapts a clearinghouse to a Dojo dijit.Tree widget
dojo.declare(
	"ch.TreeModel",
	null,
{
	//____________________________________________________________________________
	constructor: function(args) {
		for (var i in args) {
			this[i] = args[i];
		}
		if (!this.clearinghouse) {
			throw new Error("ch.TreeModel requires a clearinghouse!");
		}
		this.clearinghouse.listen(this);
		return;
	},
	//____________________________________________________________________________
	destroy: function(){
	},
	//____________________________________________________________________________
	getRoot: function(onItem){
		onItem(this.clearinghouse.$root);
	},
	//____________________________________________________________________________
	mayHaveChildren: function(item) {
		// todo: infer from metadata
		return item.$container;
	},
	//____________________________________________________________________________
	getChildren: function(parentItem, onComplete){
		var c;
		if (parentItem.$container) {
			for (var i in parentItem) {
				if (parentItem[i] === null) {
					throw Error("null child");
				}
				if (		(typeof parentItem[i] === "object"
						||
							typeof parentItem[i] === "function")
					&&
							(i !== "prototype"
						||
							typeof parentItem !== "function")
					&&
						i !== "$$p"
					&&
						i !== "$$meta"
					&&
						parentItem[i].$n !== undefined)
				{
						(c || (c = [])).push(parentItem[i]);
				}
			}
		}
		onComplete(c || []);
	},
	//____________________________________________________________________________
	getIdentity: function(item){
		return this.clearinghouse.oid(item);
	},
	//____________________________________________________________________________
	getLabel: function(item){
		return item.$n;
	},
	//____________________________________________________________________________
	newItem: function(args, parent){
		return;
	},
	//____________________________________________________________________________
	pasteItem: function(childItem, oldParentItem, newParentItem, bCopy) {
		// todo: implement
		return;
	},
	//____________________________________________________________________________
	onChange: function(item) {
		return;
	},
	//____________________________________________________________________________
	onChildrenChange: function(parent, newChildrenList) {
		return;
	},
	//____________________________________________________________________________
	onDelete: function(item) {
		return;
	}
});

// EOF