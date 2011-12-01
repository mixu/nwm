var tile = require('./tile.js')
  , monocle = require('./monocle.js')
  , wide = require('./wide.js')
  ;

exports.defineLayouts = function(nwm) {
  nwm.addLayout('tile', tile);
  nwm.addLayout('monocle', monocle);
  nwm.addLayout('wide', wide);
};
