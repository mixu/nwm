var assert = require('assert'),

    Tile = require('../../layouts/tile.js'),
    Model = require('unbundle_model').Model;

exports['given Tile layout'] = {

  beforeEach: function() {
    this.tile = new Tile({});
  },

  'mainWindow is initially empty': function() {
    assert.equal(this.tile.mainWindow, null);
  },

  'mainWindow is set when a new window is added and unset when removed': function() {
    var newWin = new Model({ id: 1});
    this.tile.windows.add(newWin);
    assert.equal(this.tile.mainWindow, newWin);
    this.tile.windows.remove(newWin);
    assert.equal(this.tile.mainWindow, null);
  },

  'mainWindow is unset when it is removed': function() {
    var wins = [ new Model({ id: 1}), new Model({ id: 2})];
    this.tile.windows.add(wins);
    assert.equal(this.tile.mainWindow, wins[0]);
    this.tile.windows.remove(wins[0]);
    assert.equal(this.tile.mainWindow, wins[1]);
  }

};

// if this module is the script being run, then run the tests:
if (module == require.main) {
  var mocha = require('child_process').spawn('mocha', [ '--colors', '--ui', 'exports', '--reporter', 'spec', __filename ]);
  mocha.stdout.pipe(process.stdout);
  mocha.stderr.pipe(process.stderr);
}
