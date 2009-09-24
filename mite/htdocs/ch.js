//_____________________________________________________________________________
// Copyright 2008 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com
//
// - Dojo dependencies in this module NOT ALLOWED
// @todo - factor out dependencies on ch.TreeModel

//_____________________________________________________________________________
delta = function delta(args) {
	this.deltas = [];
	// @todo - should be a list
	this.listener = null;
	if (args) {
		for (var i in args) {
			this[i] = args[i];
		}
	}
	return;
}

//_____________________________________________________________________________
delta.prototype.listen = function(l) {
	this.listener = l;
	return;
}

//_____________________________________________________________________________
// - instead of spamming change events, compress multiple events on the
//   same item or children into a single event
// - each item appears in the list only once, and is marked for producing
//   a self (onChange) &| child (onChidrenChange) change event
// - this is significantly faster than generating events on the fly for
//   bulk updates
// - N.B. it can be useful to set a breakpoint entering this function to
//        examine 'deltas' to determine which items & children
//        appear to have changed
delta.prototype.delta = function delta_delta() {
	if (!this.listener) {
		throw new Error("No listener for deltas.");
	}
	while (it = this.deltas.pop()) {
		if (it.$delta_me) {
			delete it.$delta_me;
			this.listener.onChange(it);
		}
		if (it.$delta_child) {
			var listener = this.listener; // - for the closure
			this.listener.getChildren(it, function(children) {
				delete it.$delta_child;			
				listener.onChildrenChange(it, children);
				return;
			});
		}
	}
	if (0 > this.deltas.length) {
		console.error("delta ended prematurely on " + it);
	}
	return;
}

//_____________________________________________________________________________
// - call when it's state has been modified
delta.prototype.delta_me = function delta_delta_me(it) {
	// - otherwise it's already marked and in the delta list
	if (!it.$delta_me && !it.$delta_child) {
		this.deltas.push(it);
	}
	it.$delta_me = true;
	return;
}

//_____________________________________________________________________________
// - call when it's *set* of children have changed (that is, getChildren will
//   have a different result)
delta.prototype.delta_child = function delta_delta_child(it) {
	// - otherwise it's already marked and in the delta list
	if (!it.$delta_me && !it.$delta_child) {
		this.deltas.push(it);
	}
	it.$delta_child = true;
	return;
}

// EOF