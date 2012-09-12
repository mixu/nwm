var Workspace = require('./workspace.js');

/*
 * A monitor is has rectangular dimensions and manages a set of workspaces.
 * The monitor receives information about windows assigned to it.
 * It needs to:
 *  - assign each received window to the currently active workspace
 *  - if the monitor is removed, it has to know which windows it contains
 */

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
