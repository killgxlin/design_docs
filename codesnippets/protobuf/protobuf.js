var pb = require('protobuf_for_node.node');
var fs = require('fs');

var Schema = pb.Schema;
var schema = new Schema(fs.readFileSync('msg.desc'));
var CSMsg = schema['pb.CSMsg'];
var SCMsg = schema['pb.SCMsg'];
var csLogin = {id:'ID_CSLogin', login:{playerId:1}}
var scTaskInit = {
  id:'ID_SCTaskInit', 
  taskInit:{
    completed:[1,2,3,4],
    current:[
      { taskid:5,
        count:3,
        canCommit:false
      },
      { taskid:6,
        count:2,
        canCommit:true
      },
      { taskid:7,
        count:0,
        canCommit:false
      }
    ]
  }
};
var serialized = SCMsg.serialize(scTaskInit);
console.log(serialized);
console.log(SCMsg.parse(serialized));


