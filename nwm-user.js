// modules
var NWM = require('./nwm.js');
var XK = require('./lib/keysymdef.js');
var Xh = require('./lib/x.js');
// do: npm install webrepl
var webrepl = require('webrepl');

// instantiate nwm and configure it
var nwm = new NWM();

// LAYOUTS: add layouts from external hash/object
var layouts = require('./nwm-layouts.js');
Object.keys(layouts).forEach(function(name){
  var callback = layouts[name];
  nwm.addLayout(name, layouts[name]);
});

// KEYBOARD SHORTCUTS
var baseModifier = Xh.Mod4Mask|Xh.ControlMask; // to make it easier to reassign the "base" modifier combination
// Workspace management - since we do the same thing for keys 0..9, use an array
[XK.XK_1, XK.XK_2, XK.XK_3, XK.XK_4, XK.XK_5, XK.XK_6, XK.XK_7, XK.XK_8, XK.XK_9].forEach(function(key) {
  // workspace switching
  // number keys are used to move between screens
  nwm.addKey({ key: key, modifier: baseModifier }, function(event) { 
    nwm.go(String.fromCharCode(event.keysym)); 
  });  
  // moving windows between workspaces
  nwm.addKey({ key: key, modifier: baseModifier|Xh.ShiftMask }, function(event) { 
    nwm.focused_window && nwm.windowTo(nwm.focused_window, String.fromCharCode(event.keysym));
  });
});

// starting xterm
// enter key is used to launch xterm
nwm.addKey({ key: XK.XK_Return, modifier: baseModifier }, function(event) {
  // check for whether we are running in a different display
  var term = require('child_process').spawn('xterm', ['-lc'], (process.env.DISPLAY ? { env: { 'DISPLAY': process.env.DISPLAY } } : undefined ));
  term.on('exit', function (code) {
    console.log('child process exited with code ', code);
  });  
});

// closing a window
// c key is used to terminate the process
nwm.addKey({ key: XK.XK_c, modifier: baseModifier }, function(event) {
  nwm.focused_window && nwm.wm.killWindow(nwm.focused_window);
});

// alternating between layout modes
// space switches between layouts
nwm.addKey({ key: XK.XK_space, modifier: baseModifier }, function(event) {
  var workspace = nwm.getWorkspace(nwm.current_workspace);
  workspace.layout = nwm.nextLayout(workspace.layout);
  // monocle hides windows in the current workspace, so unhide them
  nwm.go(nwm.current_workspace);
  nwm.rearrange();  
});

// increase and decrease master area size
// h increases the main window size
[XK.XK_h, XK.XK_F10].forEach(function(key) {
  nwm.addKey({ key: key, modifier: baseModifier }, function(event) {
    var workspace = nwm.getWorkspace(nwm.current_workspace);
    workspace.setMainWindowScale(workspace.getMainWindowScale() - 5);
    console.log('Set main window scale', workspace.getMainWindowScale());
    nwm.rearrange();    
  });
});

// l decreases the main window size
[XK.XK_l, XK.XK_F11].forEach(function(key) {
  nwm.addKey({ key: key, modifier: baseModifier }, function(event) {
    var workspace = nwm.getWorkspace(nwm.current_workspace);
    workspace.setMainWindowScale(workspace.getMainWindowScale() + 5);
    console.log('Set main window scale', workspace.getMainWindowScale());
    nwm.rearrange();    
  });  
});

// make the currently focused window the main window
// tab makes the current window the main window
nwm.addKey({ key: XK.XK_Tab, modifier: baseModifier }, function(event) {
  console.log('Set main window', nwm.focused_window);
  nwm.setMainWindow(nwm.focused_window);
  nwm.rearrange();  
});

// TODO: moving focus 
nwm.addKey({ key: XK.XK_j, modifier: baseModifier }, function() {});
// TODO: graceful shutdown
nwm.addKey({ key: XK.XK_q, modifier: baseModifier|Xh.ShiftMask }, function() {});

// CUSTOM FUNCTIONS

nwm.random = function() {
  var self = this;
  var screen = this.screen;
  var keys = Object.keys(this.windows);
  keys.forEach(function(id, index) {
    self.move(id, Math.floor(Math.random()*(screen.width-300)), Math.floor(Math.random()*(screen.height-300)));    
  });
};

nwm.globalSmall = function() {
  var self = this;
  var keys = Object.keys(this.windows);
  keys.forEach(function(id, index) {
    self.resize(id, 200, 200);    
  });  
};

var tweens = [];

nwm.tween = function(id) {
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

nwm.stop = function() {
  for(var i = 0; i < tweens.length; i++) {
    clearInterval(tweens[i].interval); 
  }
  tweens = [];
};



// START
nwm.start(function() {
  // Expose via stdout
  var repl_stdout = require('repl').start();
  repl_stdout.context.nwm = nwm;
  repl_stdout.context.Xh = Xh;  
  // Expose via webrepl
  console.log('Starting webrepl');
  var repl_web = webrepl.start(6000);
  repl_web.context.nwm = nwm;
});
