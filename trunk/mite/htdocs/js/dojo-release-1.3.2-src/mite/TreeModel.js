//_____________________________________________________________________________
// Copyright 2008 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com

dojo.provide("mite.TreeModel");

//_____________________________________________________________________________
// - based on dijit.tree.TreeStoreModel, but doesn't require dijit/Tree.js
//   to be loaded
dojo.declare(
	"mite.TreeModel",
	null,
{
	//____________________________________________________________________________
	constructor: function(args) {
		for (var i in args) {
			this[i] = args[i];
		}
		var r = dojo.byId("mite");
		// - mite produces its own events, which must be converted
		//   to dijit.tree.model events
		r.mite.addEventListener(this);
		return;
	},
	
	//____________________________________________________________________________
	destroy: function(){
	},
	
	//____________________________________________________________________________
	getRoot: function(onItem){
		onItem(dojo.byId("mite"));
	},
	
	//____________________________________________________________________________
	mayHaveChildren: function(item) {
		return item.mite.mayHaveChildren();
	},
	
	//____________________________________________________________________________
	getChildren: function(parentItem, onComplete){
		onComplete(parentItem.mite.getChildren());
	},
	
	//____________________________________________________________________________
	getIdentity: function(item){
		return item.id;
	},
	
	//____________________________________________________________________________
	getLabel: function(item){
		var t = item.getAttribute('title'); 
		return t ? t : item.id;
	},
	
	//____________________________________________________________________________
	newItem: function(args, parent){
		return;
	},
	
	//____________________________________________________________________________
	pasteItem: function(childItem, oldParentItem, newParentItem, bCopy) {
		return;
	},
	
	//____________________________________________________________________________
	// - event targets
	onChange: function(item) {},
	onChildrenChange: function(parent, newChildrenList) {},
	onDelete: function(item) {}
});

// EOF