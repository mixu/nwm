var Workspace = require('./workspace.js');

/*
 * A monitor is has rectangular dimensions and manages a set of workspaces.
 * The monitor receives information about windows assigned to it.
 * It needs to:
 *  - assign each received window to the currently active workspace
 *  - if the monitor is removed, it has to know which windows it contains
 */

function Monitor(opts) {
  if (typeof opts == 'undefined') {
    opts = {};
  }
  this.width = opts.width || 0;
  this.height = opts.height || 0;
  this.x = opts.x || 0;
  this.y = opts.y || 0;
  this.currentWorkspace = new Workspace();

  // listen to keyboard events about workspace switching

}

Monitor.prototype.addWindow = function(window) {
  // windows can be located outside the current monitor or be too large
  if (win.x > this.x || win.y > this.y) {
    win.move(this.x, this.y);
  }
  if (win.height > this.height || win.width > this.width) {
    win.resize(this.width, this.height);
  }
  // set the focused window ?
  this.focusedWindow = window;
  // handling floating windows ??

  // listen to changes to fullscreen
  window.on('change:fullscreen', function(model, value) {
    if (value) {
      // TODO change the current workspace to use the monocle layout
      // resize
      window.set({ x: this.x, y: this.y, width: this.width, height: this.height }).sync();
    } else {
      // TODO restore layout
    }
  });


  // add to current workspace
  this.currentWorkspace.windows.add(window);
};

Monitor.prototype.removeWindow = function(window) {
  // check whether we have a window with this name
  // if so, then remove the window
  // and notify the containing workspace
};

module.exports = Monitor;
