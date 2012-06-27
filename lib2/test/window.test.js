var assert = require('assert'),

    Window = require('../window.js');

exports['given a window'] = {

  before: function(done) {
    this.w = new Window();
    done();
  },

  'can get and set width height x y': function(done){
    done();
  },

  'can get and set title instance klass': function(done) {
    done();
  },


  'can get and set visible': function(done) {

  },

  'can apply pending changes': function(done) {

  }

};

// if this module is the script being run, then run the tests:
if (module == require.main) {
  var mocha = require('child_process').spawn('mocha', [ '--colors', '--ui', 'exports', '--reporter', 'spec', __filename ]);
  mocha.stdout.pipe(process.stdout);
  mocha.stderr.pipe(process.stderr);
}
