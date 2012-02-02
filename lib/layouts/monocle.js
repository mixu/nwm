var Tile = require('./new_tile.js');

function Monocle(nwm) {
  this.nwm = nwm;
  this.windows = [];
  this.main = null;

  this.scale = 50;
};

// when the scale variable is changed
Monocle.prototype.setScale = Tile.prototype.setScale;

// this function is called to get the list of known windows before update
Monocle.prototype.query = Tile.prototype.query;

// this function is called with the changes to window list since the last render
Monocle.prototype.update = Tile.prototype.update;

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

Monocle.prototype.rearrange = function(screen) {
  if(!workspace.nwm.windows.exists(this.main)) {
    return;
  }
  var mainWin = this.nwm.windows.get(this.main);
  mainWin.move(screen.x, screen.y);
  mainWin.resize(screen.width, screen.height);
  mainWin.show(); // this may be needed if the previous main window was removed..
  // hide the rest
  this.windows.forEach(function(id) {
    if(id != this.main) {
      this.nwm.windows.get(id).hide();
    }
  }, this);
};
