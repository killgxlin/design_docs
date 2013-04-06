
var buf = new Buffer(4);
buf[0] = 0x1;
buf[1] = 0x2;
buf[2] = 0x3;
buf[3] = 0x4;
console.log(buf.readUInt32BE(0));
