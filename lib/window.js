// Windows
// -------
var Window = function(nwm, window) {
  this.nwm = nwm;
  this.id = window.id;
  this.x = window.x;
  this.y = window.y;
  this._monitor = window.monitor;
  this.width = window.width;
  this.height = window.height;
  this.title = window.title;
  this.instance = window.instance;
  this.class = window.class;
  this.isfloating = window.isfloating;
  this.visible = true;
  this.workspace = window.workspace;
};

Window.prototype = {
  get monitor() {
    return this._monitor;
  },
  set monitor(id) {
    // when the monitor_id changes, the monitors should be notified of the change
    if (this._monitor != id) {
      var from = this._monitor;
      this._monitor = id;
      console.log('change window monitor', this.id, from, this._monitor);
      this.nwm.emit('change window monitor', this.id, from, this._monitor);
    }
  }
};

// Move a window
Window.prototype.move = function(x, y) {
  this.x = x;
  this.y = y;
  console.log('move', this.id, x, y);
  this.nwm.wm.moveWindow(this.id, x, y);
};

// Resize a window
Window.prototype.resize = function(width, height) {
  this.width = width;
  this.height = height;
  console.log('resize', this.id, width, height);
  this.nwm.wm.resizeWindow(this.id, width, height);
};

// Hide a window
Window.prototype.hide = function() {
  if (this.visible) {
    this.visible = false;
//    console.log('hide', this.id);
    // window should be moved to twice the total monitor width
    var total = 0;
    var monitors = Object.keys(this.nwm.monitors.items);
    var self = this;
    monitors.forEach(function(id) {
      total += self.nwm.monitors.get(id).width;
    });
    this.nwm.wm.moveWindow(this.id, this.x + 2 * total, this.y);
  }
};

// Show a window
Window.prototype.show = function() {
  if (!this.visible) {
    this.visible = true;
//    console.log('show', this.id);
    this.nwm.wm.moveWindow(this.id, this.x, this.y);
  }
};

module.exports = Window;
