
var NWM = require('./build/default/helloworld.node').HelloWorld;


var nwm = new NWM();

/**
 * A single window should be positioned
 */
nwm.onManage(function(window) {
  console.log('onManage', window);
  window.height = 300;
  window.width = 500;

  // this is called once for each window
  // so the window should be added somewhere 
  // and the layout should be recalculated based on all the currently known windows
  // and only then should the window be returned..

  return window;
});

/**
 * A mouse button has been clicked
 */
nwm.onButtonPress(function(event) {
  console.log('Button pressed', event);
  return event;
});

/**
 * A window is requesting to be configured to particular dimensions
 */
nwm.onConfigureRequest(function(event){
  return event;
});

/**
 * A key has been pressed
 */
nwm.onKeyPress(function(key) {
  // do something, e.g. launch a command
  return key;
});

var screen = nwm.setup();
console.log('Screen dimensions', screen);
nwm.scan();
console.log('Scan done');
nwm.loop();
console.log('Loop started');

