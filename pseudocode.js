
var Monitor = function() {


};


var Window = function() {
  this.monitor = new Monitor();
};

Window.prototype.raise = function() {};
Window.prototype.setX = function() {};
Window.prototype.setY = function() { };
Window.prototype.setWidth = function() { };
Window.prototype.setHeight = function() { };
Window.prototype.setSibling = function() { };
Window.prototype.setStackMode = function() { };


var NVM = function() {
  this.screens = [
    new Monitor();
  ];

};

/**
 * Set up resources and grab events
 */
NVM.prototype.setup = function() {
  // initialize resources

  // scan for screens
  screens.forEach(function(screen){
    // subscribe to screen events e.g. SubstructureRedirectMask

    // get all windows
    // subscribe to mouse press events in all windows

  })

};

/**
 * Scan and layout current windows
 */
NVM.prototype.scan = function() {
  // XQueryTree for windows
  // call the manage callback for each window
  this.emit('manage', window);
};

NVM.prototype.loop = function() {
  // get next event using XNextEvent
  var event_types = [
    'ButtonPress',
    'ClientMessage', // not sure what to do with this
    'ConfigureRequest',
    'ConfigureNotify', // not sure what to do with this
    'DestroyNotify', // not sure what to do with this
    'EnterNotify',  // not sure what to do with this
    'Expose', // not sure what to do with this
    'FocusIn', // not sure what to do with this
    'KeyPress',
    'MappingNotify', // refresh keyboard mapping?
    'MapRequest', // grab the window events (internally)
    'PropertyNotify', // not sure what to do with this
    'UnmapNotify' // not sure what to do with this
  ];
  // call the handler
  this.emit(event_type);
};


var nvm = new NVM();

/**
 * A single window should be positioned
 */
nvm.on('manage', function(window) {

});

/**
 * A mouse button has been clicked
 */
nvm.on('ButtonPress', function(event) {
  // if a window is clicked
  if(event.window) {
    // raise the window
    event.window.raise();
  }
};

/**
 * A window is requesting to be configured to particular dimensions
 */
nvm.on('ConfigureRequest', function(event){
  // do what the window requested:
  if(event.window) {
    if(event.x) { // x coordinate
      event.window.setX(event.x);
    }
    if(event.y) { // y coordinate
      event.window.setY(event.y);
    }
    if(event.width) {
      event.window.setWidth(event.width);
    }
    if(event.height) {
      event.window.setHeight(event.height);
    }
    if(event.above) {
      event.window.setSibling(event.above);
    }
    if(event.stack_mode) {
      event.window.setStackMode(event.stack_mode);
    }
  }
};

/**
 * A key has been pressed
 */
nvm.on('KeyPress', function(key) {
  // do something, e.g. launch a command
};



nvm.setup();
nvm.scan();
while(true) {
  nvm.loop();
}

