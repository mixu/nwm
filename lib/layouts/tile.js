
function Tile(nwm) {
  this.nwm = nwm;
  this.windows = [];
  this.main = null;

  this.scale = 50;
};

// when the scale variable is changed
Tile.prototype.setScale = function(scale) {
  this.scale = scale;
};

// this function is called to get the list of known windows before update
Tile.prototype.query = function() {
  return this.windows;
};

// this function is called with the changes to window list since the last render
Tile.prototype.update = function(changes) {
  // remove
  var mainWindowRemoved = false;
  this.windows = this.windows.filter(function(id) {
    if(id == this.main) {
      // main window was removed
      mainWindowRemoved = true;
    }
    return (changes.removed.indexOf(id) > -1);
  });
  // add
  this.windows.concat(changes.added);
  // if the main window was removed
  if(mainWindowRemoved) {
    // pick the largest id
    console.log('Take largest', this.windows);
    this.main = Math.max.apply(Math, this.windows);
  }
};

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


Tile.prototype.rearrange = function(screen) {
  // nothing to do
  if(this.windows.length == 0) {
    return;
  }
  // get all window objects as a hash
  var windows = nwm.windows.get(this.windows);
  if(this.windows.length == 1) {
    // this is easy
    windows[this.main].move(screen.x, screen.y);
    windows[this.main].resize(screen.width, screen.height);
    return;
  }

  // when main scale = 50, the divisor is 2
  var mainScaleFactor = (100 / this.scale );
  var halfWidth = Math.floor(screen.width / mainScaleFactor);
  windows[this.main].move(screen.x, screen.y);
  windows[this.main].resize(halfWidth, screen.height);
  // remove the main window from the set
  delete windows[this.main];
  // calculate size
  var remainWidth = screen.width - halfWidth;
  var sliceHeight = Math.floor(screen.height / (ids.length) );
  // set sizes
  Object.keys(windows).forEach(function(id, index) {
    var window = windows[id];
    window.move(screen.x + halfWidth, screen.y + index*sliceHeight);
    window.resize(remainWidth, sliceHeight);
  });
};
