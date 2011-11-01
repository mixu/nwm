//     nwm.js
//     (c) 2011 Mikito Takada
//     nwm is freely distributable under the MIT license.
//     Portions of nwm are inspired or borrowed from dwm.

// Modules
// -------

// Native extension
var X11wm = require('./build/default/nwm.node').NodeWM;
var Collection = require('./lib/collection.js');
var Monitor = require('./lib/monitor.js');
var Window = require('./lib/window.js');

// Node Window Manager
// -------------------
var NWM = function(binding) {
  // A reference to the nwm C++ X11 binding
  this.wm = (binding ? binding : new X11wm());
  // Known layous
  this.layouts = {};
  // Keyboard shortcut lookup
  this.shortcuts = [];
  // monitors
  this.monitors = new Collection(this, 'monitor', 0);
  // windows -- this is the global storage for windows, any other objects just store ids referring to this hash.
  this.windows = new Collection(this, 'window', 1);
}

require('util').inherits(NWM, require('events').EventEmitter);

// Events
// ------
NWM.prototype.events = {
  // Monitor events
  // --------------
  // A new monitor is added
  addMonitor: function(monitor) {
    this.monitors.add(new Monitor(this, monitor));
    this.monitors.current = monitor.id;
  },

  // A monitor is updated
  updateMonitor: function(monitor) {
    this.monitors.update(monitor.id, monitor);
  },

  // A monitor is removed
  removeMonitor: function(id) {
    console.log('Remove monitor', id);
    this.monitors.remove(function(monitor){ return (monitor.id != id); });
  },

  // Window events
  // -------------
  // A new window is added
  addWindow: function(window) {
    if(window.id) {
      var monitor = this.monitors.get(this.monitors.current);
      window.workspace = monitor.workspaces.current;
      // ignore monitor number from binding as windows should open on the focused monitor
      window.monitor = this.monitors.current;

      var current_monitor = this.monitors.get(this.monitors.current);
      if(current_monitor.focused_window == null) {
        current_monitor.focused_window = window.id;
      }
      // do not add floating windows
      if(window.isfloating) {
        console.log('Ignoring floating window: ', window);
        return;
      }
      var window = new Window(this, window);
      // windows might be placed outside the screen if the wm was terminated
      if(window.x > monitor.width || window.y > monitor.height) {
        window.move(1, 1);
      }
      console.log('Add window', window);
      this.windows.add(window);
    }
  },

  // When a window is removed
  removeWindow: function(id) {
    this.windows.remove(function(window) { return (window.id != id); });
  },

  // When a window is updated
  updateWindow: function(window) {
    if(this.windows.exists(window.id)) {
      // ignore monitor number from binding as it will go out of date if the window is moved
      window.monitor = this.windows.get(window.id).monitor;
      this.windows.update(window.id, window);
    }
  },

  // When a window requests full screen mode
  fullscreen: function(id, status) {
    if(this.windows.exists(id)) {
      var window = this.windows.get(id);
      // use the monitor dimensions associated with the window
      var monitor = this.monitors.get(window.monitor);
      var workspace = monitor.workspaces.get(monitor.workspaces.current);
      if(status) {
        this.wm.moveWindow(id, monitor.x, monitor.y);
        this.wm.resizeWindow(id, monitor.width, monitor.height);
      } else {
        workspace.rearrange();
      }
    }
  },

  // TODO - NOT IMPLEMENTED: A window is requesting to be configured to particular dimensions
  configureRequest: function(event){
    return event;
  },

  // Mouse events
  // ------------
  // A mouse button has been clicked
  mouseDown: function(event) {
    this.wm.focusWindow(event.id);
  },

  // A mouse drag is in progress
  mouseDrag: function(event) {
    // move when drag is triggered
    var change_x = event.move_x - event.x;
    var change_y = event.move_y - event.y;
    var window = this.windows.exists(event.id) && this.windows.get(event.id);
    if(window) {
      this.wm.moveWindow(event.id, window.x+change_x, window.y+change_y);
    }
  },

  // Mouse enters a window
  enterNotify: function(event){
    if(this.windows.exists(event.id)) {
      var window = this.windows.get(event.id);
      console.log('focused monitor is ', this.monitors.current, 'focusing to', window.monitor, window.title);
      // Current monitor changes when a new window in that monitor receives focus
      if(this.monitors.exists(window.monitor)) {
        this.monitors.current = window.monitor;
      }
      this.monitors.get(window.monitor).focused_window = event.id;
      this.wm.focusWindow(event.id);
    } else {
      console.log('WARNING got focus event for nonexistent (transient) window', event);
      // transients need this to happen to get keyboard focus
      this.wm.focusWindow(event.id);
    }
  },

  focusIn: function(event) {
    if(this.windows.exists(event.id)) {
      var window = this.windows.get(event.id);
      console.log('Change focused monitor ', this.monitors.current, '=>', window.monitor, '(but not window)');
      if(this.monitors.exists(window.monitor)) {
        this.monitors.current = window.monitor;
      }
      // Important: Don't change window focus here, as focusIn events get emitted after focusWindow() is called,
      // and the focus is already set correctly when we do it ourself. And calling focusWindow() again here is a bad idea,
      // as it will generate another focusIn event. Generally, you want enterNotify above.
    }
  },

  // This is emitted when we need to shift focus based on pointer location within the
  // ROOT window -- to make it possible to shift focus from one monitor to another
  // without already having a window on that monitor
  focusMonitor: function(x, y) {
    // event.x, event y represent coordinates in the root
    console.log('Focus monitor by root window', x, y);
    // go through the monitors and find a matching monitor
    var monitor_ids = Object.keys(this.monitors.items);
    var self = this;
    monitor_ids.some(function(monid) {
      var monitor = self.monitors.get(monid);
      if(monitor.inside(x, y)) {
        console.log('Change focused monitor: ', monid);
        self.monitors.current = monid;
        return true; // end iteration
      }
      return false; // continue iteration
    });
  },

  // Screen events
  // -------------
  rearrange: function() {
    var self = this;
    // rearrange current workspace on all monitors
    var monitors = Object.keys(this.monitors.items);
    monitors.forEach(function(id) {
      var monitor = self.monitors.get(id);
      monitor.workspaces.get(monitor.workspaces.current).rearrange();
    });
  },

  // Keyboard events
  // ---------------
  // A key has been pressed
  keyPress: function(event) {
    console.log('keyPress', event, String.fromCharCode(event.keysym));
    // find the matching callback and emit it
    this.shortcuts.forEach(function(shortcut) {
      if(event.keysym == shortcut.key && event.modifier == shortcut.modifier ) {
        shortcut.callback(event);
      };
    });
  }
};

// Layout operations
// -----------------

// Register a new layout
NWM.prototype.addLayout = function(name, callback){
  this.layouts[name] = callback;
};

// Given the current layout, get the next layout (e.g. for switching layouts via keyboard shortcut)
NWM.prototype.nextLayout = function(name) {
  var keys = Object.keys(this.layouts);
  var pos = keys.indexOf(name);
  // Wrap around the array
  return (keys[pos+1] ? keys[pos+1] : keys[0] );
};

// Keyboard shortcut operations
// ----------------------------

// Add a new keyboard shortcut
NWM.prototype.addKey = function(keyobj, callback) {
  this.shortcuts.push({ key: keyobj.key, modifier: keyobj.modifier, callback: callback });
};

// Start the window manager
NWM.prototype.start = function(callback) {
  var self = this;
  // Initialize event handlers, bind this in the functions to nwm
  Object.keys(this.events).forEach(function(eventname) {
    self.wm.on(eventname, function() {
      var args = Array.prototype.slice.call(arguments);
      console.log('JS: ', eventname, args);
      self.events[eventname].apply(self, args);
    });
  });

  var grab_keys = [];
  console.log(this.shortcuts);
  this.shortcuts.forEach(function(shortcut) {
    grab_keys.push( { key: shortcut.key, modifier: shortcut.modifier });
  });
  this.wm.keys(grab_keys);
  this.wm.start();
  if(callback) {
    callback();
  }
};

NWM.prototype.currentMonitor = function() {
  return this.monitors.get(this.monitors.current);
};

// Load, and watch a single file for changes.
NWM.prototype.hotLoad = function(filename) {
  var self = this;
  // Load all files in the directory
  self.require(filename);
  // Watch the directory
  console.log('watch', filename)
  require('fs').watchFile(filename, function (curr, prev) {
    if (curr.mtime.toString() !== prev.mtime.toString()) {
      self.require(filename);
    }
  });
};

// Non-caching version of Node's default require()
NWM.prototype.require = function(filename) {
  // From lib/module.js in the Node.js core (v.0.5.3)
  function stripBOM(content) {
    // Remove byte order marker. This catches EF BB BF (the UTF-8 BOM)
    // because the buffer-to-string conversion in `fs.readFileSync()`
    // translates it to FEFF, the UTF-16 BOM.
    if (content.charCodeAt(0) === 0xFEFF) {
      content = content.slice(1);
    }
    return content;
  }
  function isFunction(obj) {
    return !!(obj && obj.constructor && obj.call && obj.apply);
  };
  var fullname = require.resolve(filename);
  console.log('readfile', filename, fullname)
  // remove shebang
  var content = stripBOM(require('fs').readFileSync(fullname, 'utf8')).replace(/^\#\!.*/, '');
  var sandbox = { };
  // emulate require()
  for (var k in global) {
    sandbox[k] = global[k];
  }
  sandbox.require = require;
  sandbox.__filename = fullname;
  sandbox.__dirname = require('path').dirname(filename);
  sandbox.exports = {};
  sandbox.module = sandbox;
  sandbox.global = sandbox;
  try {
    require('vm').runInNewContext(content, sandbox);
    if(sandbox.exports && isFunction(sandbox.exports)) {
      sandbox.exports(this);
    }
  } catch(err) {
    console.log('Error: running hot loaded file ', fullname, 'failed. Probably a syntax error.');
    return;
  }
};

if (module == require.main) {
  console.log('Please run nwm via "node nwm-user-sample.js" (or some other custom config file).');
}

module.exports = NWM;
