var HelloWorld = require('./build/default/helloworld.node').HelloWorld;

var obj = new HelloWorld();
console.log(obj.hello());
