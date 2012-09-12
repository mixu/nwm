var Collection = require('unbundle_model').Collection,
    EventEmitter = require('events').EventEmitter;
/*
 * A workspace is a rectangular area with a collection of windows that represent the windows inside it.
 * The workspace can do whatever it wants to do with the windows it contains when it is active.
 */
function Workspace() {
  this.windows = new Collection();
  this.keypress = new EventEmitter();
}

module.exports = Workspace;
