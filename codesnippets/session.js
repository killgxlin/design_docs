var protobuf = require('protobuf_for_node');
var events = require('events');

function Session() {
  this.socket = null;
}

Session.prototype = new events.EventEmitter;
