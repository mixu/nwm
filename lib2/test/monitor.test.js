var assert = require('assert'),

    Monitor = require('../monitor.js');

exports['given a monitor'] = {

  before: function(done) {
    this.m = new Monitor();
    done();
  },

  'can get and set x y width height': function(done) {
    done();
  },

  'has a default workspace': function(done) {
    done();
  },

  'can receive keyboard events': function(done) {
    done();
  },

  'on keyboard event, can go to a new workspace by index': function(done) {
    done();
  },

  'on keyboard event, can cycle the layout in the current workspace': function(done) {
    done();
  },

  'can add a new window to the current workspace': function(done) {
    done();
  },

  'on keyboard event, move the current window to a different workspace': function(done) {
    done();
  },

  'when a window requests fullscreen, activates the built-in monocle mode temporarily': function(done) {

  },

  'when the window that requested fullscreen is removed or requests normal size, returns to the previous layout': function(done) {

  }

};

// if this module is the script being run, then run the tests:
if (module == require.main) {
  var mocha = require('child_process').spawn('mocha', [ '--colors', '--ui', 'exports', '--reporter', 'spec', __filename ]);
  mocha.stdout.pipe(process.stdout);
  mocha.stderr.pipe(process.stderr);
}
