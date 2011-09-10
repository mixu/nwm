var NWM = require('./nwm.js');
// npm install webrepl
var webrepl = require('webrepl');

var nwm = new NWM();
// add layouts from external hash/object
var layouts = require('./nwm-layouts.js');
Object.keys(layouts).forEach(function(name){
  var callback = layouts[name];
  nwm.addLayout(name, layouts[name]);
});
nwm.start(function() {
  console.log('Starting webrepl');
  var re = webrepl.start(6000);
  re.context.nwm = nwm;
});
