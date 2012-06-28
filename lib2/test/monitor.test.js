var assert = require('assert'),

    Monitor = require('../monitor.js'),
    Workspace = require('../workspace.js');

exports['given a monitor'] = {

  beforeEach: function(done) {
    this.m = new Monitor();
    done();
  },

  'can get and set x y width height': function(done) {
    var m = this.m;
    m.x = 100;
    assert.equal(m.x, 100);
    m.y = 200;
    assert.equal(m.y, 200);
    m.width = 300;
    assert.equal(m.width, 300);
    m.height = 400;
    assert.equal(m.height, 400);
    done();
  },

  'has a default workspace': function(done) {
    assert.ok(this.m.defaultWorkspace instanceof Workspace);
    done();
  },

  'can receive events': function(done) {
    var m = this.m;
    m.once('intent:a', function(key) {
      assert.equal(key, a);
      done();
    });
    m.emit('intent:a', 'a');
  },

  'on intent event, can go to a new workspace by index': function(done) {
    done();
  },

  'on intent event, can cycle the layout in the current workspace': function(done) {
    done();
  },

  'can add a new window to the current workspace': function(done) {
    done();
  },

  'on intent event, move the current window to a different workspace': function(done) {
    done();
  },

  'when a window requests fullscreen, activates the built-in monocle mode temporarily': function(done) {
    done();
  },

  'when the window that requested fullscreen is removed or requests normal size, returns to the previous layout': function(done) {
    done();
  }

};

// if this module is the script being run, then run the tests:
if (module == require.main) {
  var mocha = require('child_process').spawn('mocha', [ '--colors', '--ui', 'exports', '--reporter', 'spec', __filename ]);
  mocha.stdout.pipe(process.stdout);
  mocha.stderr.pipe(process.stderr);
}
