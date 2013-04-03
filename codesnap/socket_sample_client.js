var net = require('net');
var timers = require('timers');

var connection = net.connect({port:3000, host:'localhost'}, function(){
  console.log('connected');
  connection.write('world\r\n');
});
connection.setEncoding('utf-8');
connection.on('data', function(data){
  console.log(data);
});
connection.on('end', function() {
  console.log('end');
});
timers.setTimeout(function(){
  connection.end();
}, 1000);
