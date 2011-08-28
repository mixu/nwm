var repl = require('repl');
var X11wm = require('./build/default/helloworld.node').HelloWorld;

/*
var X11wm = function() {};
// events
X11wm.prototype.onAdd = function(window){};
X11wm.prototype.onButtonPress = function(event){};
X11wm.prototype.onConfigureRequest = function(event) {};
X11wm.prototype.onKeyPress = function(event){};
X11wm.prototype.onDrag = function(event){};
// API
X11wm.prototype.resizeWindow = function(id, width, height) {};
X11wm.prototype.focusWindow = function(id) {};
X11wm.prototype.moveWindow = function(id, x, y) {};
X11wm.prototype.raiseWindow = function(id) {};
*/

var NWM = function() {
  this.windows = [];
  this.screen = null;
  this.drag_window = null;
  this.wm = null;
}

NWM.prototype.start = function() {
  this.wm = new X11wm();
  var self = this;
  /**
   * A single window should be positioned
   */
  this.wm.onAdd(function(window) {
    console.log('onAdd', window);
    self.windows.push(window);  

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
  this.wm.onButtonPress(function(event) {
    console.log('Button pressed', event);

    return event;
  });

  /**
   * A window is requesting to be configured to particular dimensions
   */
  this.wm.onConfigureRequest(function(event){
    return event;
  });

  /**
   * A key has been pressed
   */
  this.wm.onKeyPress(function(key) {
    // do something, e.g. launch a command
    return key;
  });  
  this.screen = this.wm.setup();
  this.wm.scan();
  this.wm.loop();

  repl.start().context.nvm = self;
};

NWM.prototype.tile = function() {
  // the way DWM does it is to reserve half the screen for the first screen, 
  // then split the other half among the rest of the screens
  if(this.windows.length == 1) {
    this.wm.moveWindow(this.windows[0].id, 0, 0);
    this.wm.resizeWindow(this.windows[0].id, this.screen.width, this.screen.height);
  } else {
    var halfWidth = Math.floor(this.screen.width / 2);
    var sliceHeight = Math.floor(this.screen.height / (this.windows.length-1) );
    this.wm.moveWindow(this.windows[0].id, 0, 0);
    this.wm.resizeWindow(this.windows[0].id, halfWidth, this.screen.height);
    for(var i = 1; i < this.windows.length; i++) {
      this.wm.moveWindow(this.windows[i].id, halfWidth, (i-1)*sliceHeight);
      this.wm.resizeWindow(this.windows[i].id, halfWidth, sliceHeight);
    }
  }
};

NWM.prototype.move = function() {
  var self = this;
  this.windows.forEach(function(window) {
    self.wm.moveWindow(window.id, Math.floor(Math.random()*200), Math.floor(Math.random()*200), 100+Math.floor(Math.random()*100), 100+Math.floor(Math.random()*11));    
  })  
};




var nwm = new NWM();
nwm.start();