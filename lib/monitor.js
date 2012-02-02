
var Collection = require('./collection.js');
var Workspace = require('./workspace.js');

// Monitor
var Monitor = function(nwm, monitor) {
  this.nwm = nwm;
  this.id = monitor.id;
  // Screen dimensions
  this.width = monitor.width;
  this.height = monitor.height;
  this.x = monitor.x;
  this.y = monitor.y;
  // List of window ids
  this.window_ids = [];
  // List of workspaces
  this.workspaces = new Collection(nwm, 'workspace', 1);
  var self = this;
  this.workspaces.lazyInit = function(id) {
    console.log('Lazy init workspace', id);
    // Use the current workspace, or if that does not exist, the default values
    var layout_names = Object.keys(self.nwm.layouts);
    if(self.workspaces.exists(self.workspaces.current)) {
      return new Workspace(self.nwm, id, current.layout || layout_names[0], self);
    } else {
      return new Workspace(self.nwm, id, layout_names[0], self);
    }
  };
};

// Window access
Monitor.prototype.filter = function(filtercb) {
  var self = this;
  var results = {};
  this.window_ids.forEach(function(id){
    var window = self.nwm.windows.get(id);
    // If a filter callback is not set, then take all items
    if(!filtercb) {
      results[window.id] = window;
    } else if (filtercb(window)) {
      results[window.id] = window;
    }
  });
  return results;
};

// Switch to another workspace
Monitor.prototype.go = function(workspace_id) {
  var windows = this.filter();
  var window_ids = Object.keys(windows);
  window_ids.forEach(function(window_id) {
    var window = windows[window_id];
    if(window.workspace != workspace_id) {
      window.hide();
    } else {
      window.show();
    }
  });
  var monitor = this;
  if(workspace_id != monitor.workspaces.current) {
    monitor.workspaces.current = workspace_id;
  }
  // always rearrange
  monitor.workspaces.get(monitor.workspaces.current).rearrange();
};

// Move a window to a different workspace
Monitor.prototype.windowTo = function(id, workspace_id) {
  if(this.nwm.windows.exists(id)) {
    var window = this.nwm.windows.get(id);
    var old_workspace = window.workspace;
    window.workspace = workspace_id;
    if(workspace_id == this.workspaces.current) {
      window.show();
    }
    if(old_workspace == this.workspaces.current && old_workspace != workspace_id) {
      window.hide();
    }
    if(workspace_id == this.workspaces.current || old_workspace == this.workspaces.current) {
      this.workspaces.get(this.workspaces.current).rearrange();
    }

    // TODO MOVED HERE FROM ABOVE

    if(from_id == self.id) {
      // pop from window_ids
      var index = inExactIndexOf(self.window_ids, window_id);
      if(index != -1) {
        self.window_ids.splice(index, 1);
      }
      // if the window was focused, then pick another window to focus
      if(self.focused_window == window_id) {
        console.log('Take largest', self.window_ids);
        self.focused_window = Math.max.apply(Math, self.window_ids);
      }
      self.workspaces.get(self.workspaces.current).rearrange();
    }
    if(to_id == self.id) {
      // push to window_ids
      self.window_ids.push(window_id);
      self.workspaces.get(self.workspaces.current).rearrange();
    }
  }
};

Monitor.prototype.inside = function(x, y) {
  return (x >= this.x && x < this.x+this.width
          && y >= this.y && y < this.y+this.height);
};

Monitor.prototype.currentWorkspace = function() {
  return this.workspaces.get(this.workspaces.current);
};

Monitor.prototype.currentWindow = function() {
  if(this.focused_window && this.nwm.windows.exists(this.focused_window)) {
    return this.nwm.windows.get(this.focused_window);
  }
  return false;
};

module.exports = Monitor;
