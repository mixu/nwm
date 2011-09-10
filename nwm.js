var repl = require('repl');
var X11wm = require('./build/default/nwm.node').NodeWM;
var child_process = require('child_process');
var XK = require('./lib/keysymdef.js');
var Xh = require('./lib/x.js');

var Workspace = function(id, layout) {
  this.layout = layout;
  this.main_window = null;
  this.main_window_scale = 50;
};

Workspace.prototype.getMainWindow = function(nwm) {
  var visible = nwm.visible();
  if(visible.indexOf(''+this.main_window) > -1) {
    console.log('getMainWindow using old value', this.main_window);
    return this.main_window;
  } 
  // no visible window, or the main window has gone away. Pick one from the visible windows
  // Take the largest id (e.g. most recent window)
  console.log('getMainWindow old window gone, get new one', visible, this.main_window, visible.indexOf(this.main_window));
  this.main_window = Math.max.apply(Math, visible);
  return this.main_window;
};

Workspace.prototype.setMainWindow = function(id) {
  this.main_window = id;
};

Workspace.prototype.getMainWindowScale = function() {
  return this.main_window_scale;
};

Workspace.prototype.setMainWindowScale = function(scale) {
  if(scale <= 0) {
    scale = 1;
  }
  if(scale >= 100) {
    scale = 99;
  }
  this.main_window_scale = scale;
};

var NWM = function() {
  // external binding
  this.wm = null;
  // list of windows 
  this.windows = {};
  // screen dimensions
  this.screen = null;
  // list of layouts
  this.layouts = {};
  // list of workspaces
  this.workspaces = [];
  // current workspace id 
  this.current_workspace = 1;

  // Remove?
  this.focused_window = null;
  this.drag_window = null;
}

/**
 * Register a new layout
 */
NWM.prototype.addLayout = function(name, callback){
  this.layouts[name] = callback;
};

/**
 * Given the current layout, get the next layout (e.g. for switching layouts via keyboard shortcut)
 */
NWM.prototype.nextLayout = function(name) {
  var keys = Object.keys(this.layouts);
  var pos = keys.indexOf(name);
  // wrap around the array
  return (keys[pos+1] ? keys[pos+1] : keys[0] );
};

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


NWM.prototype.start = function(callback) {
  this.wm = new X11wm();
  var self = this;  

  /**
   * A new window is added
   */
  this.wm.on('add', function(window) {
    if(window.id) {
      window.visible = true;
      window.workspace = self.current_workspace;
      self.windows[window.id] = window;      
      // windows might be placed outside the screen if the wm was terminated
       console.log(window);
      if(window.x > self.screen.width || window.y > self.screen.height) {
        console.log('Moving window '+window.id+' on to screen');
        self.move(window.id, 1, 1);
      }
      // set the new window as the main window for this workspace
      // so new windows get the primary working area
      self.setMainWindow(window.id);
      console.log('onAdd', self.windows[window.id]);
    }
  });

  this.wm.on('remove', function(id) {
    console.log('onRemove', id);
    if(self.windows[id]) {
      delete self.windows[id];
    }
    self.rearrange();
  });

  this.wm.on('rearrange', function() { self.rearrange(); }); 

  /**
   * A mouse button has been clicked
   */
  this.wm.on('mouseDown', function(event) {
    console.log('Mouse button pressed', event);
    self.wm.focusWindow(event.id);
  });
  this.wm.on('mouseDrag', function(event) {
    console.log('Mouse drag event', event);
    // move when drag is triggered
    var change_x = event.move_x - event.x;
    var change_y = event.move_y - event.y;
    var window = self.windows[event.id];
    if(window) {
      self.wm.moveWindow(event.id, window.x+change_x, window.y+change_y);      
    }
  });

  this.wm.on('enterNotify',function(event){
    console.log('focusing to window', event.id);
    self.focused_window = event.id;
    self.wm.focusWindow(event.id);    
  });

  /**
   * A window is requesting to be configured to particular dimensions
   */
  this.wm.on('configureRequest', function(event){
    return event;
  });

  /**
   * A key has been pressed
   */
  this.wm.on('keyPress', function(key) {
    // do something, e.g. launch a command
    var chr = String.fromCharCode(key.keysym);
    // decode keysym name
    var keysym_name = '';
    Object.keys(XK).every(function(name) {
      if(XK[name] == key.keysym) {
        keysym_name = name;
        return false; // stop iteration
      }
      return true;
    });
    // decode modifier names
    var modifiers = [];
    Object.keys(Xh).forEach(function(name){
      if(key.modifier & Xh[name]) {
        modifiers.push(name);
      }
    });

    console.log('keyPress', key, chr, keysym_name, modifiers);
    // number keys are used to move between screens
    if( key.keysym > XK.XK_0 && key.keysym <= XK.XK_9) {
      // check the modifier
      if( (key.modifier & Xh.Mod4Mask) && (key.modifier & Xh.ControlMask) ) {
        console.log('go to workspace',  chr);      
        self.go(chr); // jump to workspace        
      }
      if( (key.modifier & Xh.Mod4Mask) && (key.modifier & Xh.ControlMask) && (key.modifier & Xh.ShiftMask)) {
        console.log('move focused window to workspace', chr);
        if(self.focused_window) {
          self.windowTo(self.focused_window, chr);
        }
      } 
     
    } 
    // enter key is used to launch xterm
    if(key.keysym == XK.XK_Return) {
      // enter pressed ...
      console.log('Enter key, start xterm');
      console.log(process.env);
      var term;
      if(process.env.DISPLAY == ':1') {
        term = child_process.spawn('xterm', ['-lc'], { env: { 'DISPLAY': ':1' } });        
      } else {
        term = child_process.spawn('xterm', ['-lc']);        
      }
      term.on('exit', function (code) {
        console.log('child process exited with code ' + code);
      });
    }
    // c key is used to terminate the process
    if(key.keysym == XK.XK_c && self.focused_window) {
      console.log('Kill window', self.focused_window);
      self.wm.killWindow(self.focused_window);
    }
    // space switches between layouts
    if(key.keysym == XK.XK_space) {
      var workspace = self.getWorkspace(self.current_workspace);
      console.log('Change layout from ', workspace.layout);
      workspace.layout = self.nextLayout(workspace.layout);
      console.log('to ', workspace.layout);      
      // monocle hides windows in the current workspace, so unhide them
      self.go(self.current_workspace);
      self.rearrange();
    }
    // tab makes the current window the main window
    if(key.keysym == XK.XK_Tab) {
      console.log('Set main window', self.focused_window);
      self.setMainWindow(self.focused_window);
      self.rearrange();
    }
    // h increases the main window size
    if(key.keysym == XK.XK_h || key.keysym == XK.XK_F10) {
      var workspace = self.getWorkspace(self.current_workspace);
      workspace.setMainWindowScale(workspace.getMainWindowScale() - 5);
      console.log('Set main window scale', workspace.getMainWindowScale());
      self.rearrange();
    }
    // l decreases the main window size
    if(key.keysym == XK.XK_l || key.keysym == XK.XK_F11) {
      var workspace = self.getWorkspace(self.current_workspace);
      workspace.setMainWindowScale(workspace.getMainWindowScale() + 5);
      console.log('Set main window scale', workspace.getMainWindowScale());
      self.rearrange();
    }

    return key;
  });

  /**
   * When the screen is resized
   */
  this.wm.on('resize', function(screen) {
    console.log('Screen size changed', screen);
    self.screen = screen;
  });

  /**
   * When a window is updated
   */
  this.wm.on('update', function(window) {
    console.log('Window has been updated', window);
    if(window.id && self.windows[window.id]) {      
      var updated_window = self.windows[window.id];
      Object.keys(window).forEach( function(key) {
        updated_window[key] = window[key];
      });
    }
  });

  this.wm.on('fullscreen', function(id, status) {
    console.log('Window requested full screen', id, status);
    if(self.windows[id]) {
      if(status) {
        self.wm.resizeWindow(id, self.screen.width, self.screen.height);
      } else {
        self.rearrange();
      }
    }
  });

  this.wm.keys([ 
      // workspace switching
      { key: XK.XK_1, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_2, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_3, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_4, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_5, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_6, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_7, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_8, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_9, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_0, modifier: Xh.Mod4Mask|Xh.ControlMask },
      // moving windows between workspaces
      { key: XK.XK_1, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_2, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_3, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_4, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_5, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_6, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_7, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_8, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_9, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_0, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      // starting xterm
      { key: XK.XK_Return, modifier: Xh.Mod4Mask|Xh.ControlMask },
      // closing a window
      { key: XK.XK_c, modifier: Xh.Mod4Mask|Xh.ControlMask },
      // alternating between layout modes
      { key: XK.XK_space, modifier: Xh.Mod4Mask|Xh.ControlMask },
      // increase and decrease master area size
      { key: XK.XK_h, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_l, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_F10, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_F11, modifier: Xh.Mod4Mask|Xh.ControlMask },
      // make the currently focused window the master
      { key: XK.XK_Tab, modifier: Xh.Mod4Mask|Xh.ControlMask },
      // TODO: moving focus 
      { key: XK.XK_j, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_k, modifier: Xh.Mod4Mask|Xh.ControlMask },
      // TODO: graceful shutdown
      { key: XK.XK_q, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask }
    ]);
  this.screen = this.wm.setup();  
  this.wm.scan();
  this.wm.loop();
  if(callback) {
    callback();
  }
};

NWM.prototype.hide = function(id) {
  var screen = this.screen;
  if(this.windows[id] && this.windows[id].visible) {
    this.windows[id].visible = false;
    this.wm.moveWindow(id, this.windows[id].x + 2*screen.width, this.windows[id].y);    
    this.rearrange();
  }
};

NWM.prototype.show = function(id) {
  var screen = this.screen;
  if(this.windows[id] && !this.windows[id].visible) {
    this.windows[id].visible = true;
    this.wm.moveWindow(id, this.windows[id].x, this.windows[id].y);    
    this.rearrange();
  }
};

NWM.prototype.move = function(id, x, y) {
  if(this.windows[id]) {
    this.windows[id].x = x;
    this.windows[id].y = y;
    this.wm.moveWindow(id, x, y);
  }
};

NWM.prototype.resize = function(id, width, height) {
  if(this.windows[id]) {
    this.windows[id].width = width;
    this.windows[id].height = height;
    this.wm.resizeWindow(id, width, height);
  }
};

NWM.prototype.visible = function() {
  var self = this;
  var keys = Object.keys(this.windows);
  keys = keys.filter(function(id) { return (self.windows[id].visible && self.windows[id].workspace == self.current_workspace); });
  console.log('get visible', 'workspace = ', self.current_workspace, keys);
  return keys;
};

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

NWM.prototype.gimme = function(id){
  this.windowTo(id, this.workspace);
}

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

NWM.prototype.rearrange = function() {
  var workspace = this.getWorkspace(this.current_workspace);
  var callback = this.layouts[workspace.layout];
  callback(this);  
};


NWM.prototype.random = function() {
  var self = this;
  var screen = this.screen;
  var keys = Object.keys(this.windows);
  keys.forEach(function(id, index) {
    self.move(id, Math.floor(Math.random()*(screen.width-300)), Math.floor(Math.random()*(screen.height-300)));    
  });
};

NWM.prototype.globalSmall = function() {
  var self = this;
  var keys = Object.keys(this.windows);
  keys.forEach(function(id, index) {
    self.resize(id, 200, 200);    
  });  
};

var tweens = [];

NWM.prototype.tween = function(id) {
  var self = this;
  var radius = 200;
  var circle_x = Math.floor(self.screen.width / 2);
  var circle_y = Math.floor(self.screen.height / 2);
  if(circle_x+radius*2 > self.screen.width) {
    circle_x -= radius*2;
  }
  if(circle_y+radius*2 > self.screen.height) {
    circle_y -= radius*2;
  }
  if(circle_x-radius*2 < 0) {
    circle_x += radius*2;
  }
  if(circle_y-radius*2 < 0) {
    circle_y += radius*2;
  }

  function circularPath(index) {

    var cx = circle_x;
    var cy = circle_y;
    var aStep = 3;   // 3 degrees per step
    var theta = index * aStep;  // +ve angles are cw

    var newX = cx + radius * Math.cos(theta * Math.PI / 180);
    var newY = cy + radius * Math.sin(theta * Math.PI / 180);

    // return an object defining state that can be understood by drawFn
    return  {x: newX, y: newY};
  }
  var step = 0;
  var result = setInterval(function() {
    step++;
    var pos = circularPath(step);
    self.move(id, Math.floor(pos.x), Math.floor(pos.y));
  }, 1000 / 60);
  tweens.push({ id: id, interval: result} );
};

NWM.prototype.stop = function() {
  for(var i = 0; i < tweens.length; i++) {
    clearInterval(tweens[i].interval); 
  }
  tweens = [];
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
    var re = repl.start();
    re.context.nwm = nwm;
    re.context.Xh = Xh;  
  });
}

module.exports = NWM;
