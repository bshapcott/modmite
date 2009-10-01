//_____________________________________________________________________________
// Copyright 2008 Brad Shapcott
// brad at shapcott dot com
// http://shapcott.com
//
// - Dojo dependencies in this module NOT ALLOWED
// @todo - factor out dependencies on ch.TreeModel

//_____________________________________________________________________________
delta = function(args) {
	this.deltas = [];
	if (args) {
		for (var i in args) {
			this[i] = args[i];
		}
	}
	return;
}

//_____________________________________________________________________________
// - instead of spamming change events, compress multiple events on the
//   same item or children into a single event
// - this is significantly faster than generating events on the fly for
//   bulk updates
// - N.B. it can be useful to set a breakpoint entering this function to
//        examine 'deltas' to determine which items & children
//        appear to have changed
delta.prototype.delta = function() {
	while (it = this.deltas.shift()) {
		delete it.$delta_me;
		console.debug(it);
	}
	return;
}

//_____________________________________________________________________________
// - call when it's state has been modified
delta.prototype.delta_me = function(it) {
	// - otherwise it's already marked and in the delta list
	if (!it.$delta_me) {
		this.deltas.push(it);
		it.$delta_me = true;
	}
	return;
}

//_____________________________________________________________________________
// @todo - force a delta if this list grows too large, especially if finished
//         modifying a subtree
ch = new delta({pedigree: {}});

//_____________________________________________________________________________
ch.cascade = function(f) {
	for (var i in f) {
		if (typeof f[i] === 'function') {
			for (var n in f[i].$$up) {
				if (f[i].$$up[n] === f) {
					f[i].$$up.slice(n, 1);
					break;
				}
				if (f[i].$$up.length === 0) {
					ch.cascade(f[i]);
				}
			}
		}
	}
}

//_____________________________________________________________________________
// - create a closure on value
// @param	val		@dox
ch.close = function(val) {
	if (typeof val === 'function') {
		// - an architectural violation -- a clearinghouse is a generic
		//   data sharing facility that does not have any application
		//   semantics
		throw new Error("clearinghouse doesn't store functions");
	}

	var doppel; // - the closed copy of value
	// - the closure that will hold the value
	function f(arg) {
		if (arg === f) {
			// - short circuit recursion
			// - return nothing for the idiomatic use case of
			//   f being used as a constructor
			return;
		}
		if (typeof arg === 'function') {
			if (arg.$$pedigree === ch.pedigree) {
				throw new Error("closures cannot be used as values");
			}
			// - privileged access, should only be exploited by power
			//   users
			// - direct modification of doppel will not cause an
			//   event, because arg === doppel
			arg = arg(doppel, f);
		}
		if (arg === doppel) {
			return doppel;
		}
		// - if no arg, return a copy of doppel
		if (arg === undefined) {
			if (typeof doppel === 'object') {
				return new f(f);
			}
			return doppel;
		}
		// - to suppress event generation when arg actually results in
		//   zero changes
		var atleastonechange = false;
		var unhook = [];
		if (typeof arg === 'object' && arg !== null) {
			if (doppel instanceof Array !== arg instanceof Array) {
				// - switch from simple Object to Array or vice versa
				for (var i in doppel) {
					if (typeof doppel[i] === 'function') {
						unhook.push(doppel[i]);
					}
					delete f[i];
				}
				// - forces doppel to be recreated from scratch
				//   in the conditional immediately following
				doppel = null;
			}
			if (typeof doppel !== 'object' || doppel === null) {
				doppel = arg instanceof Array ? [] : {};
				atleastonechange = true;
			}
			for (var i in doppel) {
				// - remove any members in doppel but not in arg
				if (!i in arg) {
					if (typeof doppel[i] === 'function') {
						unhook.push(doppel[i]);
					}
					delete d[i];
					delete f[i];
					atleastonechange = true;
				}
			}
			for (var i in arg) {
				if (doppel[i] !== arg[i]) {
					if (typeof arg[i] === 'function') {
						// - prevent the common use case of handing
						//   in arg object with it's own functions
						// - only the clearinghouse's own closures are
						//   copied in as members of this closure value
						if (arg[i].$$pedigree === ch.pedigree) {
							doppel[i] = arg[i];
							doppel[i].$$up.push(f);
							f[i] = doppel[i];
							atleastonechange = true;
						}
					} else {
						doppel[i] = ch.close(arg[i]);
						doppel[i].$$up = [f];
						f[i] = doppel[i];
						atleastonechange = true;
					}
				}
			}
			f.prototype = doppel;
			f.prototype.commit = function() {
				f(this);
			}
		} else if (typeof arg !== 'function') {
			if (doppel != arg) {
				for (var i in doppel) {
					if (typeof doppel[i] === 'function') {
						unhook.push(doppel[i]);
					}
					delete f[i];
				}
				doppel = arg;
				atleastonechange = true;
			}
		}
		for (var i in unhook) {
			for (var n in unhook[i].$$up) {
				if (unhook[i].$$up[n] === f) {
					unhook[i].$$up[n].splice(n, 1);
					if (unhook[i].$$up.length === 0) {
						// @todo -- cascade
						ch.cascade(unhook[i]);
					}
					// - will only remove ONE element for each appearance
					//   in unhook, because multiple members could reference
					//   the same closure
					break;
				}
			}
		}
		if (atleastonechange) {
			ch.delta_me(f);
		}
	} // f(arg)
	f(val);
	// - brand the closure, so it can be distinguished from
	//   other functions on feral objects
	f.$$pedigree = ch.pedigree;
	f.$$sub = [];
	f.sub = function(s) {
		// - dedup
		for (var i in this.$$sub) {
			if (this.$$sub[i] === s) {
				return;
			}
		}
		this.$$sub.push(s);
	}
	f.unsub = function(s) {
		for (var i in this.$$sub) {
			if (this.$$sub[i] === s) {
				this.$$sub.slice(i, 1);
			}
		}
	}
	return f;
}

//_____________________________________________________________________________
ch.root = ch.close({
	$f: "http://localhost/ch",
	$type: 'container',
	$a: [1, 2, 3],
	$m: {alpha: 'alpha', beta: 'beta', gamma: 'gamma'}
});

// EOF