// modules
var NWM = require('./nwm.js');
var XK = require('./lib/keysymdef.js').keysyms;
var Xh = require('./lib/x.js').masks;

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

// Workspace management keys (OK)
[XK.XK_1, XK.XK_2, XK.XK_3, XK.XK_4, XK.XK_5, XK.XK_6, XK.XK_7, XK.XK_8, XK.XK_9].forEach(function(key) {
  // number keys are used to move between screens
  nwm.addKey({ key: key, modifier: baseModifier }, function(event) { 
    var monitor = nwm.monitors.get(nwm.monitors.current);
    monitor.go(String.fromCharCode(event.keysym)); 
  });  
  // moving windows between workspaces
  nwm.addKey({ key: key, modifier: baseModifier|Xh.ShiftMask }, function(event) { 
    var monitor = nwm.monitors.get(nwm.monitors.current);
    nwm.focused_window && monitor.windowTo(nwm.focused_window, String.fromCharCode(event.keysym));
  });
});

// enter key is used to launch xterm (OK)
nwm.addKey({ key: XK.XK_Return, modifier: baseModifier }, function(event) {
  // check for whether we are running in a different display
  var term = require('child_process').spawn('xterm', ['-lc'], { env: process.env });
  term.on('exit', function (code) {
    console.log('child process exited with code ', code);
  });  
});

// c key is used to close a window (OK)
nwm.addKey({ key: XK.XK_c, modifier: baseModifier }, function(event) {
  nwm.focused_window && nwm.wm.killWindow(nwm.focused_window);
});

// space switches between layout modes (OK)
nwm.addKey({ key: XK.XK_space, modifier: baseModifier }, function(event) {
  var monitor = nwm.monitors.get(nwm.monitors.current);
  var workspace = monitor.workspaces.get(monitor.workspaces.current);
  workspace.layout = nwm.nextLayout(workspace.layout);
  // monocle hides windows in the current workspace, so unhide them
  monitor.go(monitor.workspaces.current);
  workspace.rearrange();
});

// h increases the main window size (OK)
[XK.XK_h, XK.XK_F10].forEach(function(key) {
  nwm.addKey({ key: key, modifier: baseModifier }, function(event) {
    var monitor = nwm.monitors.get(nwm.monitors.current);
    var workspace = monitor.workspaces.get(monitor.workspaces.current);
    workspace.setMainWindowScale(workspace.getMainWindowScale() - 5);
    console.log('Set main window scale', workspace.getMainWindowScale());
    workspace.rearrange();    
  });
});

// l decreases the main window size (OK)
[XK.XK_l, XK.XK_F11].forEach(function(key) {
  nwm.addKey({ key: key, modifier: baseModifier }, function(event) {
    var monitor = nwm.monitors.get(nwm.monitors.current);
    var workspace = monitor.workspaces.get(monitor.workspaces.current);
    workspace.setMainWindowScale(workspace.getMainWindowScale() + 5);
    console.log('Set main window scale', workspace.getMainWindowScale());
    workspace.rearrange();    
  });  
});

// tab makes the current window the main window
nwm.addKey({ key: XK.XK_Tab, modifier: baseModifier }, function(event) {
  var monitor = nwm.monitors.get(nwm.monitors.current);
  var workspace = monitor.workspaces.get(monitor.workspaces.current);
  console.log('Set main window', nwm.focused_window);
  workspace.setMainWindow(nwm.focused_window);
  workspace.rearrange();  
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
});
