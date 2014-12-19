function Window(opts) {
  if (typeof opts == 'undefined') {
    opts = {};
  }
  this.id = opts.id;
  this._width = opts.width || 0;
  this._height = opts.height || 0;
  this._x = opts.x || 0;
  this._y = opts.y || 0;
  this.pending = { };
}

var propertyNames = ['width', 'height', 'x', 'y'];

// define ES5 getters and setters
propertyNames.forEach(function(prop) {
  var name = '_' + prop;
  Object.defineProperty(Window.prototype, prop, {
    get: function() {
      return (typeof this.pending[name] != 'undefined' ?
              this.pending[name] : this[name]);
    },
    set: function(v) {
      this.pending[name] = v;
    },
    enumerable: true
  });
});

// applies pending changes as operations
Window.prototype.sync = function() {
  var self = this,
      changedProperties = propertyNames.filter(function(prop) {
        return (this.pending['_' + prop] && this.pending['_' + prop] != this['_' + prop]);
      });
  // persist dimension change
  if (changedProperties.indexOf('width') + changedProperties.indexOf('height') > -1) {
    wm.resizeWindow(this.id, this.pending._width, this.pending._height);
  }
  if (changedProperties.indexOf('x') + changedProperties.indexOf('y') > -1) {
    wm.moveWindow(this.id, this.pending._x, this.pending._y);
  }
};

module.exports = Window;
