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

obj.onManage(function(window) { 
  console.log('onManage: ', window); 
  if(window.x) {
    window.x += 100;
  }
  return window;
});
obj.onButtonPress(function(text) { console.log('onButtonPress: ', text); });
obj.onConfigureRequest(function(text) { console.log('onConfigureRequest: ', text); });
obj.onKeyPress(function(text) { console.log('onKeyPress: ', text); });
var result = obj.allCallbacks();

console.log('return value from onManage was', result);

// setTimeout(function() {}, 10000);