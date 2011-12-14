var NWM = function() {
  // Known layouts
  this.layouts = {
    tile: tile,
    wide: wide,
    grid: grid
  };
  // monitors
  this.monitors = [ ];
  this.windows = { };
  // counter
  this.counter = 1;
};

NWM.prototype.addMonitor = function (monitor) {
  this.monitors.push(new Monitor(this, monitor));
};

NWM.prototype.addWindow = function () {
  var id = this.counter++;
  // create new div
  $('<div id="w'+id+'" class="win">#'+id+'</div>').appendTo('#monitor');
  // bind to enternotify
  var self = this;
  $('#w'+id).bind('mouseenter', function() {
    self.enterNotify({ id: id });
  });
  // add to management
  this.windows[id] = new Window(id);
};

NWM.prototype.removeWindow = function(id) {
  delete this.windows[id];
  $('#w'+id).remove();
  if(Object.keys(nwm.windows).length > 0) {
    nwm.focusWindow(nwm.windows[Math.min.apply(Math, Object.keys(nwm.windows))].id);
  }
};

NWM.prototype.focusWindow = function(id) {
  this.monitors[0].focused_window = id;
  $('.focus').removeClass('focus');
  $('#w'+id).addClass('focus');
};

NWM.prototype.enterNotify = function(event) {
  this.focusWindow(event.id);
};


NWM.prototype.rearrange = function() {
  this.monitors.forEach(function(monitor) {
    monitor.workspaces[0].rearrange();
  });
};

NWM.prototype.nextLayout = function(name) {
  var keys = Object.keys(this.layouts);
  var pos = keys.indexOf(name);
  // Wrap around the array
  return (keys[pos+1] ? keys[pos+1] : keys[0] );
};
