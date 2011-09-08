var NWM = require('./nwm.js');
// npm install webrepl
var webrepl = require('webrepl');

var nwm = new NWM();
nwm.start(function() {
  var re = webrepl.start(8080);
  re.context.nwm = nwm;
});
