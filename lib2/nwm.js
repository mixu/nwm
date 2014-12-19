var monitors = [];
    mainMonitor = null;

function Nwm() {}

Nwm.addMonitor = function(monitor) {
  monitors[monitor.id] = new Monitor(monitor);
  if (!mainMonitor) {
    mainMonitor = monitors[monitor.id];
  }
};

Nwm.updateMonitor = function(monitor) {
  // this.monitors[monitor.id] ...
};

Nwm.removeMonitor = function(monitor) {
  // notify all workspaces
  // move all windows to a remaining monitor
  // remove the monitor
  delete this.monitors[monitor.id];
};

Nwm.addWindow = function(window) {
  // create the window
  var win = new Window(window);
  // All new windows go on the first monitor, always. They can be moved by user actions.
  mainMonitor.addWindow(win);
};

Nwm.updateWindow = function(window) {
  // windows are updated directly, and any listeners (or collections) are notified
  windows[window.id].set(window);
};

Nwm.removeWindow = function(window) {
  // windows are removed directly, and any listeners (or collections) are notified
  windows[window.id].remove();
};

Nwm.fullscreen = function(id, status) {
  windows[window.id].set('fullscreen', status);
};

// We need better floater handling

// We don't need a rearrange event -- that can be detected from the window changes.

Nwm.keypress = function(event) {
  console.log('keyPress', event, String.fromCharCode(event.keysym));
  // find the matching callback and emit it
  this.shortcuts.forEach(function(shortcut) {
    if (event.keysym == shortcut.key && event.modifier == shortcut.modifier) {
      shortcut.callback(event);
    }
  });
};

module.exports = Nwm;
