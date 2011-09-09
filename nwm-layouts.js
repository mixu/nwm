
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
layouts.tile = function(nwm) {
  // the way DWM does it is to reserve half the screen for the first screen,
  // then split the other half among the rest of the screens
  var windows = nwm.visible();
  var screen = nwm.screen;
  if(windows.length < 1) {
    return;
  }
  var firstId = windows.shift();
  if(windows.length == 0) {
    nwm.move(firstId, 0, 0);
    nwm.resize(firstId, screen.width, screen.height);
  } else {
    var halfWidth = Math.floor(screen.width / 2);
    var sliceHeight = Math.floor(screen.height / (windows.length) );
    nwm.move(firstId, 0, 0);
    nwm.resize(firstId, halfWidth, screen.height);
    windows.forEach(function(id, index) {
      nwm.move(id, halfWidth, index*sliceHeight);
      nwm.resize(id, halfWidth, sliceHeight);
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
 */

layouts.monocle = function(nwm){
  var windows = nwm.visible();
  var screen = nwm.screen;
  if(windows.length < 1) {
    return;
  }
  var firstId = windows.shift();
  nwm.move(firstId, 0, 0);
  nwm.resize(firstId, screen.width, screen.height);
  windows.forEach(function(id, index) {
    nwm.hide(id);
  });
}

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
layouts.wide = function(nwm) {
  // the way DWM does it is to reserve half the screen for the first screen,
  // then split the other half among the rest of the screens
  var windows = nwm.visible();
  var screen = nwm.screen;
  if(windows.length < 1) {
    return;
  }
  var firstId = windows.shift();
  if(windows.length == 0) {
    nwm.move(firstId, 0, 0);
    nwm.resize(firstId, screen.width, screen.height);
  } else {
    var halfHeight = Math.floor(screen.height / 2);
    var sliceWidth = Math.floor(screen.width / (windows.length) );
    nwm.move(firstId, 0, 0);
    nwm.resize(firstId, screen.width, halfHeight);
    windows.forEach(function(id, index) {
      nwm.move(id, index*sliceWidth, halfHeight);
      nwm.resize(id, sliceWidth, halfHeight);
    });
  }
};

module.exports = layouts;
