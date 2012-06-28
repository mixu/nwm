var assert = require('assert'),

    Window = require('../window.js');

exports['given a window'] = {

  before: function(done) {
    this.w = new Window();
    done();
  },

  'can get and set width height x y': function(done){
    var w = this.w;
    w.x = 100;
    assert.equal(w.x, 100);
    w.y = 200;
    assert.equal(w.y, 200);
    w.width = 300;
    assert.equal(w.width, 300);
    w.height = 400;
    assert.equal(w.height, 400);
    done();
  },

  'can get and set title instance klass': function(done) {
    done();
  },

  'can get and set visible': function(done) {
    done();
  },

  'pending changes are only applied on sync() call': function(done) {
    done();
  }

};

// if this module is the script being run, then run the tests:
if (module == require.main) {
  var mocha = require('child_process').spawn('mocha', [ '--colors', '--ui', 'exports', '--reporter', 'spec', __filename ]);
  mocha.stdout.pipe(process.stdout);
  mocha.stderr.pipe(process.stderr);
}
