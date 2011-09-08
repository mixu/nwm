var NWM = require('./nwm.js');
// npm install webrepl
var webrepl = require('webrepl');

var nwm = new NWM();
nwm.start(function() {
  console.log('Starting webrepl');
  var re = webrepl.start(6000);
  re.context.nwm = nwm;
});
