//_____________________________________________________________________________
// Copyright 2008 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com

dojo.provide("ch.TreeModel");

//_____________________________________________________________________________
// - based on dijit.tree.TreeStoreModel, but doesn't require dijit/Tree.js
//   to be loaded
dojo.declare(
	"ch.TreeModel",
	null,
{
	//____________________________________________________________________________
	constructor: function(args) {
		for (var i in args) {
			this[i] = args[i];
		}
		return;
	},
	//____________________________________________________________________________
	destroy: function(){
	},
	//____________________________________________________________________________
	getRoot: function(onItem){
	},
	//____________________________________________________________________________
	mayHaveChildren: function(item) {
		// todo: infer from metadata
	},
	//____________________________________________________________________________
	getChildren: function(parentItem, onComplete){
	},
	//____________________________________________________________________________
	getIdentity: function(item){
	},
	//____________________________________________________________________________
	getLabel: function(item){
	},
	//____________________________________________________________________________
	newItem: function(args, parent){
	},
	//____________________________________________________________________________
	pasteItem: function(childItem, oldParentItem, newParentItem, bCopy) {
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