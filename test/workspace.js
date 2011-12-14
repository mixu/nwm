var Workspace = function(nwm, id, layout, monitor) {
  this.nwm = nwm;
  //  has an id
  this.id = id;
  // Each workspace has a layout
  this.layout = layout;
  // Each workspace has a main_window
  this.mainWindow = 0;
  // The main window can be scaled (interpretation differs)
  this.mainWindowScale = 50;
  this.monitor = monitor;
  this.monitor = { x: 0, y: 0, width: 600, height: 400 };
};

Workspace.prototype.rearrange = function() {
  if(!this.nwm.windows[this.mainWindow]) {
    this.mainWindow = Math.min.apply(Math, Object.keys(this.nwm.windows));
  }

  this.nwm.layouts[this.layout](this);
};

Workspace.prototype.visible = function() {
  var result = [];
  Object.keys(this.nwm.windows).forEach(function(id) {
    result[id] = this.nwm.windows[id];
  });
  return result;
};

Workspace.prototype.setMainWindowScale = function(value) {
  this.mainWindowScale = value;
};

Workspace.prototype.getMainWindowScale = function() {
  return this.mainWindowScale;
};
