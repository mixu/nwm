// Generic evented collection
// --------------------------
var Collection = function(parent, name, current) {
  this.items = {};
  this.parent = parent;
  this.name = name;
  this.current = current;
  this.lazyInit = false;
};

//Add an item to the collection and notify callback listeners
Collection.prototype.add = function(item) {
  if(!this.items[item.id]) {
    this.items[item.id] = item;
    this.parent.emit('add '+this.name, item);
  }
};

Collection.prototype.remove = function(callback) {
  var self = this;
  console.log('Collection remove', this.name);
  Object.keys(this.items).forEach(function(key, index) {
    var result = callback.call(self, self.items[key], key);
    if (!result) {
      self.parent.emit('before remove '+self.name, key);
      // must do this before emitting any events!!
      delete self.items[key];
      self.parent.emit('remove '+self.name, key);
    }
  });
};

Collection.prototype.update = function(id, values) {
  var self = this;
  if(this.items[id]) {
    Object.keys(values).forEach(function(key) {
      self.items[id][key] = values[key];
    });
    console.log('Updated collection item', id);
    this.parent.emit('update '+this.name, this.items[id]);
  }
};

Collection.prototype.exists = function(id) {
  return !!this.items[id];
};


Collection.prototype.get = function(id) {
  if(!this.items[id]) {
    if(this.lazyInit) {
      this.items[id] = this.lazyInit(id);
    } else {
      return null;
    }
  }
  return this.items[id];
};

Collection.prototype.next = function(id) {
  var keys = Object.keys(this.items);
  var pos = keys.indexOf(id);
  pos = (pos == -1 ? keys.indexOf(''+id) : pos);
  // Wrap around the array
  return (keys[pos+1] ? keys[pos+1] : keys[0] );
};

Collection.prototype.prev = function(id) {
  var keys = Object.keys(this.items);
  var pos = keys.indexOf(id);
  pos = (pos == -1 ? keys.indexOf(''+id) : pos);
  // Wrap around the array
  return (keys[pos-1] ? keys[pos-1] : keys[keys.length-1] );
};

module.exports = Collection;

