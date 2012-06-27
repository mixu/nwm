var assert = require('assert'),

    Workspace = require('../workspace.js');

exports['given a workspace'] = {

  before: function(done) {
    this.w = new Workspace();
    done();
  },

  'can receive add window and remove window events': function(done) {
    done();
  },

  'can call rearrange()': function(done){
    done();
  }

};

// if this module is the script being run, then run the tests:
if (module == require.main) {
  var mocha = require('child_process').spawn('mocha', [ '--colors', '--ui', 'exports', '--reporter', 'spec', __filename ]);
  mocha.stdout.pipe(process.stdout);
  mocha.stderr.pipe(process.stderr);
}
