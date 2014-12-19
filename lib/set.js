function Set() {
  this.items = {};
  this.length = 0;
}

require('util').inherits(Set, require('events').EventEmitter);

Set.prototype.add = function(key) {
  // only increment if the key is undefined
  if (typeof this.items[key] === 'undefined') {
    this.length++;
    this.emit('add', key);
  }
  this.items[key] = true;
};

Set.prototype.has = function(key) {
  return !!this.items[key];
};

Set.prototype.remove = function(key) {
  // only decrement if the key was previously set
  if (typeof this.items[key] !== 'undefined') {
    this.length--;
    this.emit('remove', key);
  }
  delete this.items[key];
};

['filter', 'forEach', 'every', 'map', 'some'].forEach(function(name) {
  Set.prototype[name] = function() {
    return Array.prototype[name].apply(Object.keys(this.items), arguments);
  };
});

module.exports = Set;
