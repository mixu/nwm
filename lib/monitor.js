
var Collection = require('./collection.js');
var Workspace = require('./workspace.js');

// Some concepts copied from https://bitbucket.org/mathematicalcoffee/workspace-grid-gnome-shell-extension/src/0739528fb210f05813d2b2e8f19001084c04db23/workspace-grid%40mathematical.coffee.gmail.com/extension.js?at=default


/* These double as keybinding names and ways for moveWorkspace to know which
 * direction I want to switch to
 */
const UP = 'up';
const DOWN = 'down';
const LEFT = 'left';
const RIGHT = 'right';

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

/*
 * BEGIN: Helper Functions
 */

/* Converts an index (from 0 to Monitor.screen.n_workspaces) into [row, column]
 * being the row and column of workspace `index` according to the user's layout.
 *
 * Row and column start from 0.
 */
Monitor.prototype.indexToRowCol = function indexToRowCol(index) {
  // row-major. 0-based.
  return [Math.floor(index / this.nwm.config.workspaces.cols),
    index % this.nwm.config.workspaces.cols];
}

/* Converts a row and column (0-based) into the index of that workspace.
 *
 * If the resulting index is greater than MAX_WORKSPACES (the maximum number
 * of workspaces allowable by Mutter), it will return -1.
 */
Monitor.prototype.rowColToIndex = function rowColToIndex(row, col) {
  // row-major. 0-based.
  var idx = row * this.nwm.config.workspaces.cols + col;
  if (idx >= this.nwm.config.max_workspaces) {
    idx = -1;
  }
  return idx;
}

// Takes a few human-readable directions and converts them into something this object can understand.
Monitor.prototype.translateWorkspaceId = function translateWorkspaceId(workspace_id) {
  switch (workspace_id.toLowerCase()) {
    case 'back':
      workspace_id = this.previous_workspace;
      break;
    case 'next':
      workspace_id = this.workspaces.current +1;
      break;
    case 'prev': case 'previous':
      workspace_id = this.workspaces.current -1;
      break;
    case UP:
      workspace_id = this.getWorkspaceToMy(UP);
      break;
    case DOWN:
      workspace_id = this.getWorkspaceToMy(DOWN);
      break;
    case LEFT:
      workspace_id = this.getWorkspaceToMy(LEFT);
      break;
    case RIGHT:
      workspace_id = this.getWorkspaceToMy(RIGHT);
      break;
  }

  return workspace_id;
}

/*
 * END: Helper Functions
 */


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

// Use: next_workspace = this.getWorkspaceToMy('left');
Monitor.prototype.getWorkspaceToMy = function getWorkspaceToMy(direction) {
  var from, row, col, to, _;
  from = this.workspaces.current;
  _ = this.indexToRowCol(from);
  row = _[0], col = _[1];

  with(this.nwm.config.workspaces) {
    switch(direction) {
      case LEFT:
        if (col === 0) {
//          if (this.nwm.config.workspaces.hasOwnProperty('wraparound') && this.nwm.config.workspaces.wraparound) {
          if (wraparound) {
            col = cols - 1;
            if (!wraparound_same) row--;
          }
        } else {
          col--;
        }
        break;
      case RIGHT:
        if (col === cols - 1) {
//          if (this.nwm.config.workspaces.hasOwnProperty('wraparound') && this.nwm.config.workspaces.wraparound) {
          if (wraparound) {
            col = 0;
            if (!wraparound_same) row++;
          }
        } else {
          col++;
        }
        break;
      case UP:
        if (row === 0) {
//          if (this.nwm.config.workspaces.hasOwnProperty('wraparound') && this.nwm.config.workspaces.wraparound) {
          if (wraparound) {
            row = rows - 1;
            if (!wraparound_same) col--;
          }
        } else {
          row--;
        }
        break;
      case DOWN:
        if (row === rows - 1) {
//          if (this.nwm.config.workspaces.hasOwnProperty('wraparound') && this.nwm.config.workspaces.wraparound) {
          if (wraparound) {
            row = 0;
            if (!wraparound_same) col++;
          }
        } else {
          row++;
        }
        break;
    }
    if (col < 0 || row < 0) {
      to = this.rowColToIndex(Math.max(0, row), Math.max(0, col) );
    } else if (
        col > cols - 1
        || row > rows - 1
    ) {
      to = 0;
    } else {
      to = this.rowColToIndex(row, col);
    }
  
    // log('moving from workspace %d to %d'.format(from, to));
    if (to !== from) {
      return to;
    }
  }
};

// Switch to another workspace
Monitor.prototype.go = function(workspace_id) {
  var windows = this.filter();
  var window_ids = Object.keys(windows);
  workspace_id = this.translateWorkspaceId(workspace_id);

  window_ids.forEach(function(window_id) {
    var window = windows[window_id];
    if(window.workspace != workspace_id) {
      window.hide();
    } else {
      window.show();
    }
  });

  if(workspace_id != this.workspaces.current) {
    this.previous_workspace = this.workspaces.current;
    this.workspaces.current = workspace_id;
  }
  // always rearrange
  this.workspaces.get(this.workspaces.current).rearrange();
  return this;
};

// Move a window to a different workspace. This is irrespective of movement.
// Strictly moves a window. Navigation is a separate function.
Monitor.prototype.moveWindowTo = function(id, workspace_id) {
  workspace_id = this.translateWorkspaceId(workspace_id);

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
