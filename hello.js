var HelloWorld = require('./build/default/helloworld.node').HelloWorld;

var obj = new HelloWorld();

obj.on("enterNotify", function(aaa) { console.log(' hi mom ', aaa); } );

console.log('enterNotify', obj.test1("enterNotify"));
console.log('enterNotify', obj.test1("enterNotify"));
console.log('add', obj.test2("add"));



/*
console.log('rearrange', obj.on("rearrange", function() {} ));
console.log('configureRequest', obj.on("configureRequest", function() {} ));
console.log('enterNotify', obj.on("enterNotify", function() {} ));
*/
// setTimeout(function() {}, 10000);
