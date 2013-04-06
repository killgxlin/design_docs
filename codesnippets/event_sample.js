var events = require('events');

function Session() {
}

Session.prototype = new events.EventEmitter;

var session = new Session;

session.on('haha', function(arg){
  console.log(arg);
});

session.emit('haha', 3);
