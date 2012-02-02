var Tile = require('./new_tile.js');

function Wide(nwm) {
  this.nwm = nwm;
  this.windows = [];
  this.main = null;

  this.scale = 50;
};

// when the scale variable is changed
Wide.prototype.setScale = Tile.prototype.setScale;

// this function is called to get the list of known windows before update
Wide.prototype.query = Tile.prototype.query;

// this function is called with the changes to window list since the last render
Wide.prototype.update = Tile.prototype.update;

/**
 * Bottom Stack Tiling (a.k.a. wide)
 *
 *  +----------+----------+ +----------+----------+
 *  |                     | |                     |
 *  |                     | |                     |
 *  |                     | |                     |
 *  +---------------------+ +---------------------+
 *  |                     | |          |          |
 *  |                     | |          |          |
 *  |                     | |          |          |
 *  +---------------------+ +---------------------+
 *        2 windows               3 windows
 *
 *  +---------------------+ +---------------------+
 *  |                     | |                     |
 *  |                     | |                     |
 *  |                     | |                     |
 *  +------+-------+------+ +----+-----+-----+----+
 *  |      |       |      | |    |     |     |    |
 *  |      |       |      | |    |     |     |    |
 *  |      |       |      | |    |     |     |    |
 *  +------+-------+------+ +----+-----+-----+----+
 *        4 windows               5 windows
 */

Wide.prototype.rearrange = function(screen) {
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
  var halfHeight = Math.floor(screen.height / mainScaleFactor);
  windows[this.main].move(screen.x, screen.y);
  windows[this.main].resize(screen.width, halfHeight);
  // remove the main window from the set
  delete windows[this.main];
  // calculate size
  var remainHeight = screen.height - halfHeight;
  var sliceWidth = Math.floor(screen.width / (window_ids.length) );
  // set sizes
  Object.keys(windows).forEach(function(id, index) {
    var window = windows[id];
    window.move(screen.x + index*sliceWidth, screen.y + halfHeight);
    window.resize(sliceWidth, remainHeight);
  });
};
