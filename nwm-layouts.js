
var layouts = {};

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
layouts.tile = function(workspace) {
  // the way DWM does it is to reserve half the screen for the first screen,
  // then split the other half among the rest of the screens
  var windows = workspace.visible();
  console.log('TILE', windows);
  var screen = workspace.nwm.monitors.screen;
  console.log('TILE screen', screen);  
  if(windows.length < 1) {
    return;
  }
  var mainId = workspace.getMainWindow();
  console.log('mainID', mainId);
  if(windows.length == 1) {
    windows[0].move(0, 0);
    windows[0].resize(screen.width, screen.height);
  } else {
    // when main scale = 50, the divisor is 2
    //nwm.getMainWindowScale()
    var mainScaleFactor = (100 / 50 );
    var halfWidth = Math.floor(screen.width / mainScaleFactor);
    windows[mainId].move(0, 0);
    windows[mainId].resize(halfWidth, screen.height);
    // remove from visible
    var ids = Object.keys(windows);
    ids = ids.filter(function(id) { return (id != mainId); });
    console.log(ids);
    ids = ids.map(function(id) { return parseInt(id, 10); });
    console.log(ids);
    console.log('tile', 'main window', mainId, 'others', windows );
    var remainWidth = screen.width - halfWidth;
    var sliceHeight = Math.floor(screen.height / (ids.length) );
    ids.forEach(function(id, index) {
      console.log(halfWidth, index, sliceHeight, index*sliceHeight);
      windows[id].move(halfWidth, index*sliceHeight);
      windows[id].resize(remainWidth, sliceHeight);
    });
  }
};

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

layouts.monocle = function(nwm){
  var windows = nwm.visible();
  var screen = nwm.screen;
  if(windows.length < 1) {
    return;
  }
  var mainId = nwm.getMainWindow();
  nwm.move(mainId, 0, 0);
  nwm.resize(mainId, screen.width, screen.height);
  // remove from visible
  windows = windows.filter(function(id) { return (id != mainId); });
  windows.forEach(function(id, index) {
    nwm.hide(id);
  });
}
 */

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
layouts.wide = function(nwm) {
  // the way DWM does it is to reserve half the screen for the first screen,
  // then split the other half among the rest of the screens
  var windows = nwm.visible();
  var screen = nwm.screen;
  if(windows.length < 1) {
    return;
  }
  var mainId = nwm.getMainWindow();
  if(windows.length == 1) {
    nwm.move(mainId, 0, 0);
    nwm.resize(mainId, screen.width, screen.height);
  } else {
    // when main scale = 50, the divisor is 2
    var mainScaleFactor = (100 / nwm.getMainWindowScale() );
    var halfHeight = Math.floor(screen.height / mainScaleFactor);
    nwm.move(mainId, 0, 0);
    nwm.resize(mainId, screen.width, halfHeight);
    // remove from visible
    windows = windows.filter(function(id) { return (id != mainId); });
    var remainHeight = screen.height - halfHeight;
    var sliceWidth = Math.floor(screen.width / (windows.length) );
    windows.forEach(function(id, index) {
      nwm.move(id, index*sliceWidth, halfHeight);
      nwm.resize(id, sliceWidth, remainHeight);
    });
  }
};
 */


/**
 * Grid (a.k.a fair)

    +----------+----------+ +----------+----------+
    |          |          | |          |          |
    |          |          | |          |          |
    |          |          | |          |          |
    |          |          | +----------+----------+
    |          |          | |                     |
    |          |          | |                     |
    |          |          | |                     |
    +---------------------+ +---------------------+
          2 windows               3 windows

    +----------+----------+ +------+-------+------+
    |          |          | |      |       |      |
    |          |          | |      |       |      |
    |          |          | |      |       |      |
    +----------+----------+ +------+---+---+------+
    |          |          | |          |          |
    |          |          | |          |          |
    |          |          | |          |          |
    +---------------------+ +---------------------+
          4 windows               5 windows
 

layouts.grid = function(nwm) {
  var windows = nwm.visible();
  var screen = nwm.screen;
  if(windows.length < 1) {
    return;
  }
  var rows, cols;
  for(cols = 0; cols <= windows.length/2; cols++) {
    if(cols * cols >= windows.length) {
      break;
    }
  }
  rows = ((cols && (cols -1) * cols >= windows.length) ? cols - 1 : cols);
  console.log('rows, cols', rows, cols);
  // cells
  var cellHeight = screen.height / (rows ? rows : 1);
  var cellWidth = screen.width / (cols ? cols : 1);
  console.log('Cell dimensions', cellWidth, cellHeight);
  // order the windows so that the main window is the first window in the grid
  // and the others are in numeric order (with wraparound)
  var mainId = nwm.getMainWindow();
  var mainPos = windows.indexOf(mainId);
  var ordered = windows.slice(mainId).concat(windows.slice(0, mainPos));
  console.log('Ordered grid windows', ordered);
  ordered.forEach(function(id, index) {

    if(rows > 1 && index == (rows*cols) - cols 
       && (windows.length - index) <= ( windows.length) 
      ) {
      cellWidth = screen.width / (windows.length - index);
    }

    var newX = Math.floor(index % cols) * cellWidth;
    var newY = Math.floor(index / cols) * cellHeight;
    nwm.move(id, Math.floor(newX), Math.floor(newY));

    // adjust height/width of last row/col's windows
    var adjustHeight = ( (index >= cols * (rows -1) ) ?  screen.height - cellHeight * rows : 0 );
    var adjustWidth = 0;
    if(rows > 1 && index == windows.length-1 && (windows.length - index) < (windows.length % cols) ) {
      adjustWidth = screen.width - cellWidth * (windows.length % cols );     
    } else {
      adjustWidth = ( ((index + 1) % cols == 0 ) ? screen.width - cellWidth * cols : 0 );
    }
 
    nwm.resize(id, Math.floor(cellWidth+adjustWidth), Math.floor(cellHeight+adjustHeight) );
  });
};
*/
module.exports = layouts;
