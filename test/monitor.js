var Monitor = function(nwm, monitor) {
  this.nwm = nwm;
  this.id = 1;
  // Screen dimensions
  this.width = monitor.width;
  this.height = monitor.height;
  this.x = monitor.x;
  this.y = monitor.y;
  // List of window ids
  this.window_ids = [];
  // List of workspaces
  this.workspaces = [ new Workspace(nwm, 1, 'tile', this) ];
  // Currently focused window
  this.focused_window = 0;
};
