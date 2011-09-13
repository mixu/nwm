//     nwm.js
//     (c) 2011 Mikito Takada
//     nwm is freely distributable under the MIT license.
//     Portions of nwm are inspired or borrowed from dwm.

// Modules
// -------

// Native extension
var X11wm = require('./build/default/nwm.node').NodeWM;
// X11 keysym definitions
var XK = require('./lib/keysymdef.js');
// X.h defitions for key masks
var Xh = require('./lib/x.js');

// Node Window Manager
// -------------------

// A window manager consists of
var NWM = function() {
  // A reference to the nwm C++ X11 binding
  this.wm = null;
  // Known layous
  this.layouts = {};
  // Keyboard shortcut lookup
  this.shortcuts = [];
  // monitors
  this.monitors = new MonitorList(this);
}

// Events
// ------
NWM.prototype.event = {};

// Monitor events
// --------------
NWM.prototype.event.monitor = {};

// A new monitor is added
NWM.prototype.event.monitor.add = function(monitor) {
  console.log('add monitor', monitor);
  var mon = this.monitors.get(monitor.id);
  mon.width = monitor.width;
  mon.height = monitor.height;
};

// A monitor is updated
NWM.prototype.event.monitor.update = function(monitor) {
  console.log('update monitor', monitor);  
  var mon = this.monitors.get(monitor.id);
  mon.width = monitor.width;
  mon.height = monitor.height;
};

// A monitor is removed
NWM.prototype.event.monitor.remove = function(id) {
  console.log('remove monitor', id);
};

// Window events
// -------------
NWM.prototype.event.window = {};

// A new window is added
NWM.prototype.event.window.add = function(window) {
  if(window.id) {
    window.visible = true;
    window.workspace = this.monitors.current().workspaces.current().id;
    // do not add floating windows
    if(window.isfloating) {
      console.log('Ignoring floating window: ', window);
      return;
    }

    this.monitors.current().windows[window.id] = new Window(nwm, window);
    // windows might be placed outside the screen if the wm was terminated
    console.log(window);
    if(window.x > this.monitors.current().width || window.y > this.monitors.current().height) {
      console.log('Moving window '+window.id+' on to screen');
      window.move(1, 1);
    }
    // set the new window as the main window for this workspace
    // so new windows get the primary working area
    this.monitors.current().workspaces.current().setMainWindow(window.id);
    console.log('onAdd', this.monitors.current().windows);
  }
};

// When a window is removed
NWM.prototype.event.window.remove = function(id) {
  console.log('onRemove', id);
  if(this.monitors.windows[id]) {
    delete this.monitors.windows[id];
  }
  this.rearrange();
};

// When a window is updated
NWM.prototype.event.window.update = function(window) {
  console.log('Window has been updated', window);
  if(window.id && this.monitors.windows[window.id]) {      
    var updated_window = this.monitors.windows[window.id];
    Object.keys(window).forEach( function(key) {
      updated_window[key] = window[key];
    });
  }
};

// When a window requests full screen mode
NWM.prototype.event.window.fullscreen = function(id, status) {
  console.log('Window requested full screen', id, status);
  if(this.monitors.current().windows[id]) {
    if(status) {
      this.wm.resizeWindow(id, this.monitors.current().width, this.monitors.current().height);
    } else {
      this.rearrange();
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
  var window = this.monitors.windows[event.id];
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
  this.wm.on('rearrange', function() { self.monitors.current().workspaces.current().rearrange(); }); 
  this.wm.on('resize', function(screen) { 
    self.monitors.current().width = screen.width;
    self.monitors.current().height = screen.height;
  });

  // Mouse events
  this.wm.on('mouseDown', function(event) { self.event.mouse.down.call(self, event); });
  this.wm.on('mouseDrag', function(event) { self.event.mouse.drag.call(self, event); });
  this.wm.on('enterNotify', function(event) { self.event.mouse.enter.call(self, event); }); 

  /**
   * A key has been pressed
   */
  this.wm.on('keyPress', function(event) {
    // do something, e.g. launch a command
    var chr = String.fromCharCode(event.keysym);
    // decode keysym name
    var keysym_name = '';
    Object.keys(XK).every(function(name) {
      if(XK[name] == event.keysym) {
        keysym_name = name;
        return false; // stop iteration
      }
      return true;
    });
    // decode modifier names
    var modifiers = [];
    Object.keys(Xh).forEach(function(name){
      if(event.modifier & Xh[name]) {
        modifiers.push(name);
      }
    });

    console.log('keyPress', event, chr, keysym_name, modifiers);
    // find the matching callback and emit it
    self.shortcuts.forEach(function(shortcut) {
      if(event.keysym == shortcut.key && event.modifier == shortcut.modifier ) {
        shortcut.callback(event);
      };
    });
  });

  var grab_keys = [];
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
  console.log('hot load', filename);
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
  var content = require('fs').readFileSync(fullname, 'utf8');
  // remove shebang
  content = stripBOM(content).replace(/^\#\!.*/, '');
  var sandbox = {};
  // emulate require()
  for (var k in global) {
    sandbox[k] = global[k];
  }
  sandbox.require = require;
  sandbox.__filename = filename;
  sandbox.__dirname = require('path').dirname(filename);
  sandbox.exports = {};
  sandbox.module = sandbox;
  sandbox.global = sandbox;

  try {
    require('vm').runInNewContext(content, sandbox);
  } catch(err) {
    console.log('Error: running hot loaded file ', fullname, 'failed. Probably a syntax error.');
    return;
  }
  // now call the callback
  if(sandbox.exports && isFunction(sandbox.exports)) {
    sandbox.exports(this);
  } else {
    console.log('Error: hot loading file ', fullname, 'failed. Probably a syntax error.');
  }
};


// Windows
// -------
var Window = function(nwm, window) {
  this.nwm = nwm;
  // Window properties
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
  this.visible = window.visible;
  this.workspace = window.workspace;
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
    this.nwm.wm.moveWindow(this.id, this.x + 2 * nwm.monitors.get(window.monitor).width, this.y);
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
var Monitor = function(nwm, id) {
  this.nwm = nwm;
  this.id = id;
  // Screen dimensions
  this.width = 0;
  this.height = 0;
  // List of windows
  this.windows = {};
  // List of workspaces
  this.workspaces = new WorkspaceList(nwm);
  // Currently focused window
  this.focused_window = null;
};

// Window access
Monitor.prototype.filter = function(filtercb) {
  var self = this;
  var results = {};
  var ids = Object.keys(this.windows);
  ids.forEach(function(id){
    var window = self.windows[id];
    // if a filter callback is not set, then take all items
    if(!filtercb) {
      results[window.id] = window;
    } else if (filtercb(window)) {
      results[window.id] = window;
    }
  });
  return results;
};

// Switch to another workspace
Monitor.prototype.go = function(workspace) {
  var self = this;
  var windows = this.filter();
  windows.forEach(function(window) {
    if(window.workspace != workspace) {
      window.hide();
    } else {
      window.show();
    }
  });
  if(workspace != this.current_workspace) {
    this.current_workspace = workspace;
    this.rearrange();
  }
};

// Move a window to a different workspace
Monitor.prototype.windowTo = function(id, workspace) {
  if(this.windows[id]) {
    var window = this.windows[id];
    var old_workspace = window.workspace;
    window.workspace = workspace;
    if(workspace == this.current_workspace) {
      window.show();
    }
    if(old_workspace == this.current_workspace && old_workspace != workspace) {
      window.hide();
    }
    if(workspace == this.current_workspace || old_workspace == this.current_workspace) {
      this.rearrange();
    }
  }
};

Monitor.prototype.getCurrentWorkspace = function() {
  return this.workspaces.current();
};

Monitor.prototype.getMainWindow = function() {
  return this.workspaces.current().getMainWindow();
};

var MonitorList = function(nwm) {
  this.nwm = nwm;
  this.monitors = {};
  this.current_monitor = 0;
};

// Get a workspace
MonitorList.prototype.get = function(id) {
  if(!this.monitors[id]) {
    this.monitors[id] = new Monitor(this.nwm, id);
  }
  return this.monitors[id];  
};

MonitorList.prototype.current = function() {
  return this.get(this.current_monitor);
}

// Workspace
// ---------
var Workspace = function(nwm, id, layout, monitor) {
  this.nwm = nwm;
  //  has an id
  this.id = id;
  //  has Windows (subset of Monitor)
  this.windows = {}
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
  return nwm.monitors.current().filter(function(window){
    return (
      window.visible  // is visible
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
Workspace.prototype.getMainWindow = function(nwm) {
  var visible = this.visible();
  var window_ids = Object.keys(visible);
  if(window_ids.indexOf(''+this.main_window) > -1) {
    console.log('getMainWindow using old value', this.main_window);
    return this.main_window;
  }
  // no visible window, or the main window has gone away. Pick one from the visible windows..
  console.log('getMainWindow old window gone, get new one', window_ids, this.main_window, window_ids.indexOf(this.main_window));
  // Take the largest id (e.g. most recent window)
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
  if(scale <= 0) {
    scale = 1;
  }
  if(scale >= 100) {
    scale = 99;
  }
  this.main_window_scale = scale;
};

// Workspace operations
// --------------------

var WorkspaceList = function(nwm) {
  this.nwm = nwm;
  // Current workspace id
  this.current_workspace = 1;
  this.workspaces = [];
};

// Get a workspace
WorkspaceList.prototype.get = function(id) {
  if(!this.workspaces[id]) {
    // use the current workspace, or if that does not exist, the default values
    var layout_names = Object.keys(this.nwm.layouts);
    if(this.workspaces[this.current_workspace]) {
      this.workspaces[id] = new Workspace(this.nwm, id, current.layout || layout_names[0], this.nwm.monitors.current().id);
    } else {
      this.workspaces[id] = new Workspace(this.nwm, id, layout_names[0], this.nwm.monitors.current().id);
    }
  }
  return this.workspaces[id];  
};

WorkspaceList.prototype.current = function() {
  return this.get(this.current_workspace);
}

// if this module is the script being run, then run the window manager
if (module == require.main) {
  var nwm = new NWM();
  // add layouts from external hash/object
  var layouts = require('./nwm-layouts.js');
  Object.keys(layouts).forEach(function(name){
    var callback = layouts[name];
    nwm.addLayout(name, layouts[name]);
  });
  nwm.start(function() {
    var re = require('repl').start();
    re.context.nwm = nwm;
    re.context.Xh = Xh;
  });
}

module.exports = NWM;
