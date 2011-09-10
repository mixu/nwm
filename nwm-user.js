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
// Change the base modifier to your liking e.g. Xh.Mod4Mask if you just want to use the meta key without Ctrl
var baseModifier = Xh.Mod4Mask|Xh.ControlMask; 

// Workspace management keys
[XK.XK_1, XK.XK_2, XK.XK_3, XK.XK_4, XK.XK_5, XK.XK_6, XK.XK_7, XK.XK_8, XK.XK_9].forEach(function(key) {
  // number keys are used to move between screens
  nwm.addKey({ key: key, modifier: baseModifier }, function(event) { 
    nwm.go(String.fromCharCode(event.keysym)); 
  });  
  // moving windows between workspaces
  nwm.addKey({ key: key, modifier: baseModifier|Xh.ShiftMask }, function(event) { 
    nwm.focused_window && nwm.windowTo(nwm.focused_window, String.fromCharCode(event.keysym));
  });
});

// enter key is used to launch xterm
nwm.addKey({ key: XK.XK_Return, modifier: baseModifier }, function(event) {
  // check for whether we are running in a different display
  var term = require('child_process').spawn('xterm', ['-lc'], (process.env.DISPLAY ? { env: { 'DISPLAY': process.env.DISPLAY } } : undefined ));
  term.on('exit', function (code) {
    console.log('child process exited with code ', code);
  });  
});

// c key is used to close a window
nwm.addKey({ key: XK.XK_c, modifier: baseModifier }, function(event) {
  nwm.focused_window && nwm.wm.killWindow(nwm.focused_window);
});

// space switches between layout modes
nwm.addKey({ key: XK.XK_space, modifier: baseModifier }, function(event) {
  var workspace = nwm.getWorkspace(nwm.current_workspace);
  workspace.layout = nwm.nextLayout(workspace.layout);
  // monocle hides windows in the current workspace, so unhide them
  nwm.go(nwm.current_workspace);
  nwm.rearrange();  
});

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
