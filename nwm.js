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
  // List of windows
  this.windows = {};
  // Screen dimensions
  this.screen = null;
  // Known layous
  this.layouts = {};
  // List of workspaces
  this.workspaces = [];
  // Current workspace id
  this.current_workspace = 1;
  // Keyboard shortcut lookup
  this.shortcuts = [];
  // Currently focused window
  this.focused_window = null;
}

// Window management API
// ---------------------

// Move a window
NWM.prototype.move = function(id, x, y) {
  if(this.windows[id]) {
    this.windows[id].x = x;
    this.windows[id].y = y;
    this.wm.moveWindow(id, x, y);
  }
};

// Resize a window
NWM.prototype.resize = function(id, width, height) {
  if(this.windows[id]) {
    this.windows[id].width = width;
    this.windows[id].height = height;
    this.wm.resizeWindow(id, width, height);
  }
};

// Hide a window
NWM.prototype.hide = function(id) {
  var screen = this.screen;
  if(this.windows[id] && this.windows[id].visible) {
    this.windows[id].visible = false;
    this.wm.moveWindow(id, this.windows[id].x + 2*screen.width, this.windows[id].y);    
    this.rearrange();
  }
};

// Show a window
NWM.prototype.show = function(id) {
  var screen = this.screen;
  if(this.windows[id] && !this.windows[id].visible) {
    this.windows[id].visible = true;
    this.wm.moveWindow(id, this.windows[id].x, this.windows[id].y);    
    this.rearrange();
  }
};

// Events
// ------
NWM.prototype.event = {};

// Window events
// -------------
NWM.prototype.event.window = {};

// A new window is added
NWM.prototype.event.window.add = function(window) {
  if(window.id) {
    window.visible = true;
    window.workspace = this.current_workspace;
    // do not add floating windows
    if(window.isfloating) {
      console.log('Ignoring floating window: ', window);
      return;
    }

    this.windows[window.id] = window;      
    // windows might be placed outside the screen if the wm was terminated
    console.log(window);
    if(window.x > this.screen.width || window.y > this.screen.height) {
      console.log('Moving window '+window.id+' on to screen');
      this.move(window.id, 1, 1);
    }
    // set the new window as the main window for this workspace
    // so new windows get the primary working area
    this.setMainWindow(window.id);
    console.log('onAdd', this.windows[window.id]);
  }
};

// When a window is removed
NWM.prototype.event.window.remove = function(id) {
  console.log('onRemove', id);
  if(this.windows[id]) {
    delete this.windows[id];
  }
  this.rearrange();
};

// When a window is updated
NWM.prototype.event.window.update = function(window) {
  console.log('Window has been updated', window);
  if(window.id && this.windows[window.id]) {      
    var updated_window = this.windows[window.id];
    Object.keys(window).forEach( function(key) {
      updated_window[key] = window[key];
    });
  }
};

// When a window requests full screen mode
NWM.prototype.event.window.fullscreen = function(id, status) {
  console.log('Window requested full screen', id, status);
  if(this.windows[id]) {
    if(status) {
      this.wm.resizeWindow(id, this.screen.width, this.screen.height);
    } else {
      this.rearrange();
    }
  }
});

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
  var window = this.windows[event.id];
  if(window) {
    this.wm.moveWindow(event.id, window.x+change_x, window.y+change_y);      
  }
};

NWM.prototype.event.mouse.enter = function(event){
  console.log('focusing to window', event.id);
  this.focused_window = event.id;
  this.wm.focusWindow(event.id);
});

// Workspace operations
// --------------------

// Switch to another workspace
NWM.prototype.go = function(workspace) {
  var self = this;
  var keys = Object.keys(this.windows);
  keys.forEach(function(id) {
    if(self.windows[id].workspace != workspace) {
      self.hide(id);
    } else {
      self.show(id);
    }
  });
  if(workspace != this.current_workspace) {
    this.current_workspace = workspace;
    this.rearrange();
  }
};

// Move a window to a different workspace
NWM.prototype.windowTo = function(id, workspace) {
  if(this.windows[id]) {
    var old_workspace = this.windows[id].workspace;
    this.windows[id].workspace = workspace;
    if(workspace == this.current_workspace) {
      this.show(id);
    }
    if(old_workspace == this.current_workspace && old_workspace != workspace) {
      this.hide(id);
    }
    if(workspace == this.current_workspace || old_workspace == this.current_workspace) {
      this.rearrange();
    }
  }
};

// Get a workspace
NWM.prototype.getWorkspace = function(id) {
  if(!this.workspaces[id]) {
    // use the current workspace, or if that does not exist, the default values
    if(this.workspaces[this.current_workspace]) {
      this.workspaces[id] = new Workspace(id, current.layout);
    } else {
      var layout_names = Object.keys(this.layouts);
      this.workspaces[id] = new Workspace(id, layout_names[0]);
    }
  }
  return this.workspaces[id];
};

NWM.prototype.getMainWindow = function() {
  var workspace = this.getWorkspace(this.current_workspace);
  console.log('getMainWindow', this.current_workspace);
  return workspace.getMainWindow(this);
};

NWM.prototype.setMainWindow = function(id) {
  var workspace = this.getWorkspace(this.current_workspace);
  return workspace.setMainWindow(id);  
};

NWM.prototype.getMainWindowScale = function() {
  var workspace = this.getWorkspace(this.current_workspace);
  return workspace.getMainWindowScale();
};

// Layout operations
// -----------------

// Register a new layout
NWM.prototype.addLayout = function(name, callback){
  this.layouts[name] = callback;
};

// Rearrange the windows on the current workspace
NWM.prototype.rearrange = function() {
  var workspace = this.getWorkspace(this.current_workspace);
  console.log('rearrange', workspace.layout);
  var callback = this.layouts[workspace.layout];
  callback(this);
};

// Given the current layout, get the next layout (e.g. for switching layouts via keyboard shortcut)
NWM.prototype.nextLayout = function(name) {
  var keys = Object.keys(this.layouts);
  var pos = keys.indexOf(name);
  // Wrap around the array
  return (keys[pos+1] ? keys[pos+1] : keys[0] );
};

// Get the currently visible windows (used in implementing layouts)
NWM.prototype.visible = function() {
  var self = this;
  var keys = Object.keys(this.windows);
  keys = keys.filter(function(id) {
    return (
      self.windows[id].visible  // is visible
      && self.windows[id].workspace == self.current_workspace // on current workspace
    );
  });
  console.log('get visible', 'workspace = ', self.current_workspace, keys);
  return keys;
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

  // Window events
  this.wm.on('add', function(window) { self.event.window.add.call(self, window); });
  this.wm.on('remove', function(id) { self.event.window.remove.call(self, id); });
  this.wm.on('update', function(window) { self.event.window.update.call(self, window); });
  this.wm.on('fullscreen', function(id, status) { self.event.window.fullscreen.call(self, id, status); });
  this.wm.on('configureRequest', function(event) { self.event.window.reconfigure.call(self, event); });

  // Screen events
  this.wm.on('rearrange', function() { self.rearrange(); }); 
  this.wm.on('resize', function(screen) { self.screen = screen; });

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
  this.screen = this.wm.setup();  
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

// Monitor
//  has Workspaces
//  has Windows

// Workspace
//  has Windows (subset of what the Monitor has)


// Workspaces
// ----------
var Workspace = function(id, layout) {
  // Each workspace has a layout
  this.layout = layout;
  // Each workspace has a main_window
  this.main_window = null;
  // The main window can be scaled (interpretation differs)
  this.main_window_scale = 50;
};

// Get the main window
Workspace.prototype.getMainWindow = function(nwm) {
  var visible = nwm.visible();
  if(visible.indexOf(''+this.main_window) > -1) {
    console.log('getMainWindow using old value', this.main_window);
    return this.main_window;
  }
  // no visible window, or the main window has gone away. Pick one from the visible windows..
  console.log('getMainWindow old window gone, get new one', visible, this.main_window, visible.indexOf(this.main_window));
  // Take the largest id (e.g. most recent window)
  this.main_window = Math.max.apply(Math, visible);
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
