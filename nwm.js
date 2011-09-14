//     nwm.js
//     (c) 2011 Mikito Takada
//     nwm is freely distributable under the MIT license.
//     Portions of nwm are inspired or borrowed from dwm.

// Modules
// -------

// Native extension
var X11wm = require('./build/default/nwm.node').NodeWM;

// Generic evented collection
// --------------------------
var Collection = function(parent, name, current) {
  this.items = {};
  this.parent = parent;
  this.name = name;
  this.current = current;
  this.lazyInit = false;
};

//Add an item to the collection and notify callback listeners
Collection.prototype.add = function(item) {
  if(!this.items[item.id]) {
    this.items[item.id] = item;
    this.parent.emit('add '+this.name, item);    
  }
};

Collection.prototype.remove = function(callback) {
  var self = this;
  console.log('Collection remove', this.name);  
  Object.keys(this.items).forEach(function(key, index) {
    var result = callback.call(self, self.items[key], key);
    if (!result) {
      // must do this before emitting any events!!
      delete self.items[key];
      self.parent.emit('remove '+self.name, key);
    }
  });
};

Collection.prototype.update = function(id, values) {
  if(this.items[id]) {      
    var updated_item = this.items[id];
    Object.keys(values).forEach(function(key) {
      updated_item[key] = values[key];
    });
    this.parent.emit('update '+this.name, this.items[id]);
  } 
};

Collection.prototype.exists = function(id) {
  return !!this.items[id];
};


Collection.prototype.get = function(id) {
  if(!this.items[id]) {
    if(this.lazyInit) {
      this.items[id] = this.lazyInit(id);            
    } else {
      return null;
    }
  }
  return this.items[id];
};

// Windows
// -------
var Window = function(nwm, window) {
  this.nwm = nwm;
  this.id = window.id;
  this.x = window.x;
  this.y = window.y;
  this.monitor = window.monitor;
  this.width = window.width;
  this.height = window.height;
  this.title = window.title;
  this.instance = window.instance;
  this.class = window.class;
  this.isfloating = window.isfloating;
  this.visible = true;
  this.workspace =  window.workspace;
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
  if(this.visible) {
    this.visible = false;
    console.log('hide', this.id);
    this.nwm.wm.moveWindow(this.id, this.x + 2 * this.nwm.monitors.get(this.monitor).width, this.y);
  }
};

// Show a window
Window.prototype.show = function() {
  if(!this.visible) {
    this.visible = true;
    console.log('show', this.id);
    this.nwm.wm.moveWindow(this.id, this.x, this.y);    
  }
};

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
  // Listen to events
  var self = this;
  nwm.on('add window', function(window) {
    console.log('Monitor add window', window.id);
    if(window.monitor == self.id) {
      self.window_ids.push(window.id);
      // Set the new window as the main window for this workspace so new windows get the primary working area
      self.workspaces.get(self.workspaces.current).setMainWindow(window.id);      
    }
  });
  nwm.on('remove window', function(id) {
    console.log('Monitor remove window', id);
    var index = -1;
    self.window_ids.some(function(tid, inx) {
      if(tid == id) {
        index = inx;
        return true;
      }
      return false;
    });
    if(index != -1) {      
      self.window_ids.splice(index, 1);
      self.workspaces.get(self.workspaces.current).rearrange();
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
  if(workspace_id != this.workspaces.current) {
    this.workspaces.current = workspace_id;
    this.workspaces.get(this.workspaces.current).rearrange();
  }
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

// Get the currently visible windows (used in implementing layouts)
Workspace.prototype.visible = function() {
  var self = this;
  return this.monitor.filter(function(window){
    return (window.visible  // is visible
      && window.workspace == self.id // on current workspace
    );    
  });
};

// Rearrange the windows on the current workspace
Workspace.prototype.rearrange = function() {
  console.log('rearrange', this.layout);
  var callback = this.nwm.layouts[this.layout];
  callback(this);
};

// Get the main window
Workspace.prototype.getMainWindow = function() {
  // check if the current window:
  // 1) exists
  // 2) is on this workspace
  // 3) is visible
  if(this.nwm.windows.exists(this.main_window)){
    var window = this.nwm.windows.get(this.main_window);
    if(window.workspace == this.id && window.visible) {
      return this.main_window;
    }
  }
  // otherwise, take all the visible windows on this workspace
  // and pick the largest id
  var window_ids = Object.keys(this.visible());
  console.log('Take largest', window_ids);
  this.main_window = Math.max.apply(Math, window_ids);
  return this.main_window;
};

// Set the main window
Workspace.prototype.setMainWindow = function(id) {
  this.main_window = id;
};

// Get the main window scale
Workspace.prototype.getMainWindowScale = function() {
  return this.main_window_scale;
};

// Set the main window scale
Workspace.prototype.setMainWindowScale = function(scale) {
  this.main_window_scale = Math.min(Math.max(scale, 1), 99);
};

// Node Window Manager
// -------------------
var NWM = function() {
  // A reference to the nwm C++ X11 binding
  this.wm = null;
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
NWM.prototype.event = {};

// Monitor events
// --------------
NWM.prototype.event.monitor = {};

// A new monitor is added
NWM.prototype.event.monitor.add = function(monitor) {
  console.log('add monitor', monitor);
  this.monitors.add(new Monitor(this, monitor));
};

// A monitor is updated
NWM.prototype.event.monitor.update = function(monitor) {
  console.log('update monitor', monitor);  
  this.monitors.update(monitor.id, monitor);
};

// A monitor is removed
NWM.prototype.event.monitor.remove = function(id) {
  console.log('remove monitor', id);
  this.monitors.remove(function(monitor){ return (monitor.id != id); });
};

// Window events
// -------------
NWM.prototype.event.window = {};

// A new window is added
NWM.prototype.event.window.add = function(window) {
  if(window.id) {
    var monitor = this.monitors.get(window.monitor);
    window.workspace = monitor.workspaces.current;
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
    this.windows.add(window);
  }
};

// When a window is removed
NWM.prototype.event.window.remove = function(id) {
  console.log('onRemove', id);
  this.windows.remove(function(window) { return (window.id != id); });
};

// When a window is updated
NWM.prototype.event.window.update = function(window) {
  console.log('Window has been updated', window);
  this.windows.update(window.id, window);
};

// When a window requests full screen mode
NWM.prototype.event.window.fullscreen = function(id, status) {
  console.log('Window requested full screen', id, status);
  if(this.windows.exists(id)) {
    var monitor = this.monitors.get(this.monitors.current);
    var workspace = monitor.workspaces.get(monitor.workspaces.current);
    if(status) {
      this.wm.moveWindow(id, 0, 0);
      this.wm.resizeWindow(id, monitor.width, monitor.height);
    } else {
      workspace.rearrange();
    }
  }
};

// TODO - NOT IMPLEMENTED: A window is requesting to be configured to particular dimensions
NWM.prototype.event.window.reconfigure = function(event){
  return event;
};

// Mouse events
// ------------
NWM.prototype.event.mouse = {};

// A mouse button has been clicked
NWM.prototype.event.mouse.down = function(event) {
  console.log('Mouse button pressed', event);
  this.wm.focusWindow(event.id);
};

// A mouse drag is in progress
NWM.prototype.event.mouse.drag = function(event) {
  console.log('Mouse drag event', event);
  // move when drag is triggered
  var change_x = event.move_x - event.x;
  var change_y = event.move_y - event.y;
  var window = this.windows.exists(event.id) && this.windows.get(event.id);
  if(window) {
    this.wm.moveWindow(event.id, window.x+change_x, window.y+change_y);      
  }
};

NWM.prototype.event.mouse.enter = function(event){
  console.log('focusing to window', event.id);
  this.focused_window = event.id;
  this.wm.focusWindow(event.id);
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
  this.wm = new X11wm();
  var self = this;

  // Monitor events
  this.wm.on('addMonitor', function(monitor) { self.event.monitor.add.call(self, monitor); });
  this.wm.on('removeMonitor', function(id) { self.event.monitor.remove.call(self, id); });
  this.wm.on('updateMonitor', function(monitor) { self.event.monitor.update.call(self, monitor); });

  // Window events
  this.wm.on('addWindow', function(window) { self.event.window.add.call(self, window); });
  this.wm.on('removeWindow', function(id) { self.event.window.remove.call(self, id); });
  this.wm.on('updateWindow', function(window) { self.event.window.update.call(self, window); });
  this.wm.on('fullscreen', function(id, status) { self.event.window.fullscreen.call(self, id, status); });
  this.wm.on('configureRequest', function(event) { self.event.window.reconfigure.call(self, event); });

  // Screen events
  this.wm.on('rearrange', function() { 
    var monitor = self.monitors.get(self.monitors.current);
    console.log(self.monitors.current, monitor);
    console.log(monitor.workspaces);
    monitor.workspaces.get(monitor.workspaces.current).rearrange();
  });

  // Mouse events
  this.wm.on('mouseDown', function(event) { self.event.mouse.down.call(self, event); });
  this.wm.on('mouseDrag', function(event) { self.event.mouse.drag.call(self, event); });
  this.wm.on('enterNotify', function(event) { self.event.mouse.enter.call(self, event); }); 

  // A key has been pressed
  this.wm.on('keyPress', function(event) {
    console.log('keyPress', event, String.fromCharCode(event.keysym));
    // find the matching callback and emit it
    self.shortcuts.forEach(function(shortcut) {
      if(event.keysym == shortcut.key && event.modifier == shortcut.modifier ) {
        shortcut.callback(event);
      };
    });
  });

  var grab_keys = [];
  console.log(this.shortcuts);
  this.shortcuts.forEach(function(shortcut) {
    grab_keys.push( { key: shortcut.key, modifier: shortcut.modifier });
  });
  this.wm.keys(grab_keys);
  this.wm.setup();
  this.wm.scan();
  this.wm.loop();
  if(callback) {
    callback();
  }
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
