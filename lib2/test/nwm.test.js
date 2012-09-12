var assert = require('assert'),

    Nwm = require('../nwm.js');

exports['given a window manager'] = {

  before: function(done) {
    this.n = new Nwm();
    done();
  },

  'can add keyboard shortcuts': function(done) {
    done();
  },

  'can add layouts': function(done) {
    done();
  },

  'can add, update and remove monitors': function(done) {
    this.n.addMonitor({
      id: 1,
      x: 0, y: 0,
      width: 100, height: 100
    });

    this.n.updateMonitor({
      id: 1,
      x: 0, y: 0,
      width: 1000, height: 1000
    });

    this.n.removeMonitor({ id: 1});

    done();
  },

  'can add, update and remove windows': function(done) {

    this.n.addWindow({
      id: 1,
      x: 0, y: 0,
      width: 100, height: 100,
      isFloating: true
    });
    done();
  },

  'can add and remove a floating window': function(done) {
    done();
  },

  'events about windows that do not exist or are floating are ignored': function(done) {
    done();
  },

  'when a enternotify occurs, can update the selected window and monitor': function(done) {
    done();
  }

  // the rearrange event should be removed


};

// if this module is the script being run, then run the tests:
if (module == require.main) {
  var mocha = require('child_process').spawn('mocha', [ '--colors', '--ui', 'exports', '--reporter', 'spec', __filename ]);
  mocha.stdout.pipe(process.stdout);
  mocha.stderr.pipe(process.stderr);
}
