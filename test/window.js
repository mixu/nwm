var Window = function(id) {
  this.id = id;
  this.monitor = 1;
};

Window.prototype.move = function(x, y) {
  $('#w'+this.id).css('left', x).css('top', y);
};

Window.prototype.resize = function(width, height) {
  $('#w'+this.id).width(width).height(height);

};

Window.prototype.hide = function() {

};

Window.prototype.show = function() {

};
