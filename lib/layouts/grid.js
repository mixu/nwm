var Tile = require('./new_tile.js');

function Grid(nwm) {
  this.nwm = nwm;
  this.windows = [];
  this.main = null;

  this.scale = 50;
};

// when the scale variable is changed
Grid.prototype.setScale = Tile.prototype.setScale;

// this function is called to get the list of known windows before update
Grid.prototype.query = Tile.prototype.query;

// this function is called with the changes to window list since the last render
Grid.prototype.update = Tile.prototype.update;

/**
 * Grid (a.k.a fair)
 *
 *  +----------+----------+ +----------+----------+
 *  |          |          | |          |          |
 *  |          |          | |          |          |
 *  |          |          | |          |          |
 *  |          |          | +----------+----------+
 *  |          |          | |                     |
 *  |          |          | |                     |
 *  |          |          | |                     |
 *  +---------------------+ +---------------------+
 *        2 windows               3 windows
 *
 *  +----------+----------+ +------+-------+------+
 *  |          |          | |      |       |      |
 *  |          |          | |      |       |      |
 *  |          |          | |      |       |      |
 *  +----------+----------+ +------+---+---+------+
 *  |          |          | |          |          |
 *  |          |          | |          |          |
 *  |          |          | |          |          |
 *  +---------------------+ +---------------------+
 *        4 windows               5 windows
 */

Grid.prototype.rearrange = function(screen) {
  // nothing to do
  if(this.windows.length == 0) {
    return;
  }
  var rows, cols;
  var total = this.windows.length;

  for(cols = 0; cols <= total/2; cols++) {
    if(cols * cols >= total) {
      break;
    }
  }
  rows = ((cols && (cols -1) * cols >= total) ? cols - 1 : cols);
  // cells
  var cellHeight = screen.height / (rows ? rows : 1);
  var cellWidth = screen.width / (cols ? cols : 1);

  // order the windows so that the main window is the first window in the grid
  // and the others are in numeric order (with wraparound)

  var mainPos = this.windows.indexOf(this.main);
  var ordered = this.main.slice(mainPos).concat(this.main.slice(0, mainPos));
  // get all window objects as a hash
  var windows = nwm.windows.get(this.windows);
  ordered.forEach(function(id, index) {
    if(rows > 1 && index == (rows*cols) - cols
       && (total - index) <= ( total)
      ) {
      cellWidth = screen.width / (total - index);
    }

    var newX = screen.x + Math.floor(index % cols) * cellWidth;
    var newY = screen.y + Math.floor(index / cols) * cellHeight;
    windows[id].move(Math.floor(newX), Math.floor(newY));

    // adjust height/width of last row/col's windows
    var adjustHeight = ( (index >= cols * (rows -1) ) ?  screen.height - cellHeight * rows : 0 );
    var adjustWidth = 0;
    if(rows > 1 && index == total-1 && (total - index) < (total % cols) ) {
      adjustWidth = screen.width - cellWidth * (total % cols );
    } else {
      adjustWidth = ( ((index + 1) % cols == 0 ) ? screen.width - cellWidth * cols : 0 );
    }

    windows[id].resize(Math.floor(cellWidth+adjustWidth), Math.floor(cellHeight+adjustHeight) );
  });
};
