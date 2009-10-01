
cx = {};

cx.maker = function() {
	var table = [];
	function t(f) {
		if (typeof f === 'function' && f === funcs[0]) {
			f(table);
		}
	}
	t.create = function(arg) {
		table.push(arg);
	}
	t.dump = function() {
		for (var i in table) {
			console.debug(table[i]);
		}
	}
	return t;
}

it = cx.maker();

