var HelloWorld = require('./build/default/helloworld.node').HelloWorld;

var obj = new HelloWorld();

/*
console.log(obj.XOpenDisplay());
console.log(obj.XCreateSimpleWindow());
console.log(obj.XSelectInput());
console.log(obj.XMapWindow());
console.log(obj.XSetForeground());
console.log(obj.XDrawLine(10, 60, 180, 20));
console.log(obj.XDrawLine(20, 120, 90, 10));
console.log(obj.XFlush());
console.log(obj.XMapWindow());
console.log(obj.hello());
*/

//obj.setManage(function(text) { console.log("Return value: "+text); });

obj.onManage(function(text) { console.log('onManage: ', text); });
obj.onButtonPress(function(text) { console.log('onButtonPress: ', text); });
obj.onConfigureRequest(function(text) { console.log('onConfigureRequest: ', text); });
obj.onKeyPress(function(text) { console.log('onKeyPress: ', text); });
obj.allCallbacks();


// setTimeout(function() {}, 10000);