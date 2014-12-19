/**
 * Monocle (a.k.a. fullscreen)
 *
 *  +---------------------+ +---------------------+
 *  |                     | |                     |
 *  |                     | |                     |
 *  |                     | |                     |
 *  |                     | |                     |
 *  |                     | |                     |
 *  |                     | |                     |
 *  |                     | |                     |
 *  +---------------------+ +---------------------+
 *        2 windows               3 windows
 *
 *  +---------------------+ +---------------------+
 *  |                     | |                     |
 *  |                     | |                     |
 *  |                     | |                     |
 *  |                     | |                     |
 *  |                     | |                     |
 *  |                     | |                     |
 *  |                     | |                     |
 *  +---------------------+ +---------------------+
 *        4 windows               5 windows
 */

function monocle(workspace) {
  var windows = workspace.visible();
  var screen = workspace.monitor;
  // make sure that the main window is visible, always!
  var mainId = workspace.mainWindow;
  if (!workspace.nwm.windows.exists(mainId)) {
    return;
  }
  var mainWin = workspace.nwm.windows.get(mainId);
  mainWin.move(screen.x, screen.y);
  mainWin.resize(screen.width, screen.height);
  mainWin.show(); // this may be needed if the previous main window was removed..
  // hide the rest
  var window_ids = Object.keys(windows);
  if (window_ids.length < 1) {
    return;
  }
  // remove from visible
  window_ids = window_ids.filter(function(id) { return (id != mainId); });
  window_ids.forEach(function(id, index) {
    windows[id].hide();
  });
}

if (typeof module != 'undefined') {
  module.exports = monocle;
}
