// Workspace
// ---------
var Workspace = function(nwm, id, layout, monitor) {
  this.nwm = nwm;
  //  has an id
  this.id = id;
  // Each workspace has a layout
  this.layout = layout;
  // Each workspace has a main_window
  this.main_window = null;
  // The main window can be scaled (interpretation differs)
  this.main_window_scale = 50;
  this.monitor = monitor;
};

Workspace.prototype = {
  // Get the main window
  get mainWindow(){
    // check if the current window:
    // 1) exists
    // 2) is on this workspace
    // 3) is visible
    // 4) is on the current monitor
    if(this.nwm.windows.exists(this.main_window)){
      var window = this.nwm.windows.get(this.main_window);
      if(window.workspace == this.id && window.visible && window.monitor == this.monitor.id) {
        return this.main_window;
      }
    }
    // otherwise, take all the visible windows on this workspace
    // and pick the largest id
    var window_ids = Object.keys(this.visible());
    console.log('Take largest', window_ids);
    this.main_window = Math.max.apply(Math, window_ids);
    return this.main_window;
  },
  // Set the main window
  set mainWindow(id){
    this.main_window = id;
  }
};

// Get the currently visible windows (used in implementing layouts)
Workspace.prototype.visible = function() {
  var self = this;
  return this.monitor.filter(function(window){
    return (window.visible  // is visible
      && window.workspace == self.id // on current workspace
      && window.monitor == self.monitor.id // on current monitor
    );
  });
};

// Rearrange the windows on the current workspace
Workspace.prototype.rearrange = function() {
  console.log('rearrange', this.layout);
  var callback = this.nwm.layouts[this.layout];
  callback(this);
};

// Get the main window scale
Workspace.prototype.getMainWindowScale = function() {
  return this.main_window_scale;
};

// Set the main window scale
Workspace.prototype.setMainWindowScale = function(scale) {
  this.main_window_scale = Math.min(Math.max(scale, 1), 99);
};

module.exports = Workspace;
