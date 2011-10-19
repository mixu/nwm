var X11 = function() {


};

require('util').inherits(X11, require('events').EventEmitter);

// Start the window manager
X11.prototype.start = function() {
  console.log('START');
  this.emit('start');
};

// configure hotkeys
X11.prototype.keys = function() {

};

X11.prototype.moveMouse = function(x, y) {

};

module.exports = X11;
