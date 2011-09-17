/**
 * This is a hot loadable layout for nwm. See module.exports 
 * at end of the file for details.
 *
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

function grid(workspace) {
  var windows = workspace.visible();
  var screen = workspace.monitor;
  var window_ids = Object.keys(windows);
  if(window_ids.length < 1) {
    return;
  }
  var rows, cols;
  for(cols = 0; cols <= window_ids.length/2; cols++) {
    if(cols * cols >= window_ids.length) {
      break;
    }
  }
  rows = ((cols && (cols -1) * cols >= window_ids.length) ? cols - 1 : cols);
  console.log('rows, cols', rows, cols);
  // cells
  var cellHeight = screen.height / (rows ? rows : 1);
  var cellWidth = screen.width / (cols ? cols : 1);
  console.log('Cell dimensions', cellWidth, cellHeight);
  // order the windows so that the main window is the first window in the grid
  // and the others are in numeric order (with wraparound)
  var mainId = workspace.mainWindow;
  var mainPos = window_ids.indexOf(''+mainId);
  mainPos = (mainPos == -1 ? window_ids.indexOf(mainId) : mainPos);
  var ordered = window_ids.slice(mainPos).concat(window_ids.slice(0, mainPos));
  ordered.forEach(function(id, index) {
    if(rows > 1 && index == (rows*cols) - cols 
       && (window_ids.length - index) <= ( window_ids.length) 
      ) {
      cellWidth = screen.width / (window_ids.length - index);
    }

    var newX = Math.floor(index % cols) * cellWidth;
    var newY = Math.floor(index / cols) * cellHeight;
    windows[id].move(Math.floor(newX), Math.floor(newY));

    // adjust height/width of last row/col's windows
    var adjustHeight = ( (index >= cols * (rows -1) ) ?  screen.height - cellHeight * rows : 0 );
    var adjustWidth = 0;
    if(rows > 1 && index == window_ids.length-1 && (window_ids.length - index) < (window_ids.length % cols) ) {
      adjustWidth = screen.width - cellWidth * (window_ids.length % cols );     
    } else {
      adjustWidth = ( ((index + 1) % cols == 0 ) ? screen.width - cellWidth * cols : 0 );
    }
 
    windows[id].resize(Math.floor(cellWidth+adjustWidth), Math.floor(cellHeight+adjustHeight) );
  });
}

// Hot loading works like this:
// You export a callback function, which gets called every time 
// a hot load needs to occur. 
// The function gets the running instance of nwm, and does it's thing
// e.g. adds a new layout etc.
module.exports = function(nwm) {
  nwm.addLayout('grid', grid);
};
