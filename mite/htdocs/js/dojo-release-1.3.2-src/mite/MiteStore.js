dojo.provide("mite.MiteStore");
dojo.require("dojox.data.HtmlStore");

dojo.declare("mite.MiteStore", dojox.data.HtmlStore, {

	constructor: function(args) {
		if (args.dataId) {
			var s = dojo.byId(args.dataId);
			if (s && s.mite) {
				// - mite produces its own events, which must be converted
				//   to dojo.data.api.Notification events
				s.mite.addEventListener(this);
			}
		}
	},
	
	_getHeadings: function(){
		this._headings = [];
		if (this._rootNode.tHead) {
			// - HtmlStore expects attribute names in the first table header row
			// - in mite, attribute names appears in metadata in the last table
			//   header row
			// @todo - don't hardcode row index
			dojo.forEach(this._rootNode.tHead.rows[6].cells, dojo.hitch(this, function(th) {
				this._headings.push(dojox.xml.parser.textContent(th));
			}));
		} else {
			this._headings = ["name"];
		}
	},

	// NOTIFICATION API

	getFeatures: function() {
		return {
			'dojo.data.api.Read': true,
			'dojo.data.api.Identity': true,
			'dojo.data.api.Notification': true
		};
	},

	onSet: function(item, attribute, oldValue, newValue) {},
	onNew: function(newItem, parentInfo) {},
	onDelete: function(deletedItem) {}

});
