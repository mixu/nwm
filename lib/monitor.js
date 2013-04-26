
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
  // Currently focused window
  this.focused_window = null;
  this.previous_workspace = 1;

  function inExactIndexOf(arr, needle) {
    var index = -1;
    arr.some(function(item, inx) {
      if(item == needle) {
        index = inx;
        return true;
      }
      return false;
    });
    return index;
  };

  // Listen to events
  var self = this;
  nwm.on('add window', function(window) {
    console.log('Monitor add window', window.id);
    if(window.monitor == self.id) {
      self.window_ids.push(window.id);
      // Set the new window as the main window for this workspace so new windows get the primary working area
      self.workspaces.get(self.workspaces.current).mainWindow = window.id;
    }
    Object.keys(self.workspaces.items).forEach(function(ws_id) {
      self.workspaces.get(ws_id).rearrange();
    });
  });
  nwm.on('change window monitor', function(window_id, from_id, to_id) {
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
  });
  nwm.on('remove window', function(id) {
    console.log('Monitor remove window', id);
    var index = inExactIndexOf(self.window_ids, id);
    if(index != -1) {
      self.window_ids.splice(index, 1);
    }
    self.workspaces.get(self.workspaces.current).rearrange();
  });
  // move items to a remaining monitor if a monitor is removed
  nwm.on('before remove monitor', function(id) {
    if(id == self.id) {
      // get a remaining monitor
      var remaining_id = nwm.monitors.next(self.id);
      var remaining = nwm.monitors.get(remaining_id);
      // get all the windows on this monitor
      self.window_ids.forEach(function(wid){
        var window = nwm.windows.get(wid);
        window.monitor = remaining_id;
        window.workspace = remaining.workspaces.current;
      });
      nwm.monitors.current = remaining;
      remaining.workspaces.get(remaining.workspaces.current).rearrange();
    }
  });
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
    monitor.previous_workspace = monitor.workspaces.current;
    monitor.workspaces.current = workspace_id;
  }
  // always rearrange
  monitor.workspaces.get(monitor.workspaces.current).rearrange();
};

Monitor.prototype.goBack = function() {
  var cur_workspace = this.workspaces.current;
  this.go(this.previous_workspace);
  this.previous_workspace = cur_workspace;
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
