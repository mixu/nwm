var Workspace = require('./workspace.js');

function Monitor(opts) {
  if(typeof opts == 'undefined') {
    opts = {};
  }
  this.width = opts.width || 0;
  this.height = opts.height || 0 ;
  this.x = opts.x || 0;
  this.y = opts.y || 0;
  this.defaultWorkspace = new Workspace();
}

module.exports = Monitor;
