var Workspace = require('../workspace');
/**
 * Dwm's tiling a.k.a "Vertical Stack Tiling"
 *
 *  +----------+----------+ +----------+----------+
 *  |          |          | |          |          |
 *  |          |          | |          |          |
 *  |          |          | |          |          |
 *  |          |          | |          +----------+
 *  |          |          | |          |          |
 *  |          |          | |          |          |
 *  |          |          | |          |          |
 *  +---------------------+ +---------------------+
 *        2 windows               3 windows
 *
 *  +----------+----------+ +----------+----------+
 *  |          |          | |          |          |
 *  |          |          | |          +----------+
 *  |          +----------+ |          |          |
 *  |          |          | |          +----------+
 *  |          +----------+ |          |          |
 *  |          |          | |          +----------+
 *  |          |          | |          |          |
 *  +---------------------+ +---------------------+
 *        4 windows               5 windows
 */

function Tile(opts) {
  var self = this;
  Workspace.apply(this, opts);
  this.mainWindow = null;
  this.scale = 50;
  this.windows.on('add', function(win) {
    if (!self.mainWindow) {
      self.mainWindow = win;
    }
    self.tile();
  });

  this.windows.on('remove', function(win) {
    if (win == self.mainWindow) {
      self.mainWindow = self.windows.at(0);
    }
    self.tile();
  });

  this.keypress.on('scaleUp', function() {
    self.scale += 10;
    self.tile();
  });
  this.keypress.on('scaleDown', function() {
    self.scale -= 10;
    self.tile();
  });
}

Tile.prototype = new Workspace();

Tile.prototype.tile = function() {
  var self = this, windows = this.windows;
  // the way DWM does it is to reserve half the screen for the first screen,
  // then split the other half among the rest of the screens
  if (windows.length < 1) return;
  if (windows.length == 1) {
    this.mainWindow.set({
      x: this.x, y: this.y,
      width: this.width, height: this.height
    }).sync();
    return;
  }
  // when main scale = 50, the divisor is 2
  var factor = (100 / this.scale),
      halfWidth = Math.floor(this.width / factor),
      remainWidth = this.width - halfWidth,
      sliceHeight = Math.floor(this.height / (windows.length - 1));

  this.mainWindow.set({
    x: this.x, y: this.y,
    width: halfWidth, height: this.height
  }).sync();

  windows.forEach(function(window, index) {
    if (window == self.mainWindow) return;
    window.set({
      x: self.x + halfWidth, y: self.y + index * sliceHeight,
      width: remainWidth, sliceHeight: sliceHeight
    }).sync();
  });
};

module.exports = Tile;
