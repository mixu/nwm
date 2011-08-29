var repl = require('repl');
var X11wm = require('./build/default/helloworld.node').HelloWorld;

var NWM = function() {
  this.windows = {};
  this.screen = null;
  this.drag_window = null;
  this.wm = null;
  this.workspace = 1;
}

NWM.prototype.start = function() {
  this.wm = new X11wm();
  var self = this;  
  this.workspace = 1;

  /**
   * A single window should be positioned
   */
  this.wm.onAdd(function(window) {
    if(window.id) {
      window.visible = true;
      window.workspace = self.workspace;
      self.windows[window.id] = window;      
      // windows might be placed outside the screen if the wm was terminated
       console.log(window);
      if(window.x > self.screen.width || window.y > self.screen.height) {
        console.log('Moving window '+window.id+' on to screen');
        self.move(window.id, 1, 1);
      }
      console.log('onAdd', self.windows[window.id]);
    }
  });

  this.wm.onRemove(function(id) {
    console.log('onRemove', id);
    if(self.windows[id]) {
      delete self.windows[id];
    }
    self.rearrange();    
  });

  this.wm.onRearrange(function() { self.rearrange(); }); 

  /**
   * A mouse button has been clicked
   */
  this.wm.onButtonPress(function(event) {
    console.log('Button pressed', event);
    self.wm.focusWindow(event.id);
  });

  this.wm.onEnterNotify(function(event){
    self.wm.focusWindow(event.id);    
  });

  /**
   * A window is requesting to be configured to particular dimensions
   */
  this.wm.onConfigureRequest(function(event){
    return event;
  });

  /**
   * A key has been pressed
   */
  this.wm.onKeyPress(function(key) {
    // do something, e.g. launch a command
    return key;
  });  
  this.screen = this.wm.setup();
  this.wm.scan();
  this.wm.loop();

  repl.start().context.nwm = self;
};

NWM.prototype.hide = function(id) {
  var screen = this.screen;
  if(this.windows[id] && this.windows[id].visible) {
    this.windows[id].visible = false;
    this.wm.moveWindow(id, this.windows[id].x + 2*screen.width, this.windows[id].y);    
    this.rearrange();
  }
};

NWM.prototype.show = function(id) {
  var screen = this.screen;
  if(this.windows[id] && !this.windows[id].visible) {
    this.windows[id].visible = true;
    this.wm.moveWindow(id, this.windows[id].x, this.windows[id].y);    
    this.rearrange();
  }
};

NWM.prototype.move = function(id, x, y) {
  if(this.windows[id]) {
    this.windows[id].x = x;
    this.windows[id].y = y;
    this.wm.moveWindow(id, x, y);
  }
};

NWM.prototype.resize = function(id, width, height) {
  if(this.windows[id]) {
    this.windows[id].width = width;
    this.windows[id].height = height;
    this.wm.resizeWindow(id, width, height);
  }
};

NWM.prototype.visible = function() {
  var self = this;
  var keys = Object.keys(this.windows);
  keys = keys.filter(function(id) { return (self.windows[id].visible && self.windows[id].workspace == self.workspace); });
  console.log('get visible', 'workspace = ', self.workspace, keys);
  return keys;
};

NWM.prototype.go = function(workspace) {
  var self = this;
  var keys = Object.keys(this.windows);
  keys.forEach(function(id) {
    if(self.windows[id].workspace != workspace) {
      self.hide(id); 
    } else {
      self.show(id);
    }
  });
  if(workspace != this.workspace) {
    this.workspace = workspace;
    this.rearrange();
  }
};

NWM.prototype.gimme = function(id){
  this.windowTo(id, this.workspace);
}

NWM.prototype.windowTo = function(id, workspace) {
  if(this.windows[id]) {
    var old_workspace = this.windows[id].workspace;
    this.windows[id].workspace = workspace;
    if(workspace == this.workspace) {
      this.show(id);
    }
    if(old_workspace == this.workspace && old_workspace != workspace) {
      this.hide(id); 
    } 
    if(workspace == this.workspace || old_workspace == this.workspace) {
      this.rearrange();
    }
  }    
};

NWM.prototype.rearrange = function() {
  this.tile();
};

NWM.prototype.tile = function() {
  // the way DWM does it is to reserve half the screen for the first screen, 
  // then split the other half among the rest of the screens
  var keys = this.visible();
  var self = this;
  var screen = this.screen;
  if(keys.length < 1) {
    return;
  }
  var firstId = keys.shift();  
  if(keys.length == 0) {
    this.move(firstId, 0, 0);
    this.resize(firstId, screen.width, screen.height);
  } else {
    var halfWidth = Math.floor(screen.width / 2);
    var sliceHeight = Math.floor(screen.height / (keys.length) );
    this.move(firstId, 0, 0);
    this.resize(firstId, halfWidth, screen.height);
    keys.forEach(function(id, index) {
      self.move(id, halfWidth, index*sliceHeight);
      self.resize(id, halfWidth, sliceHeight);
    });
  }
};

NWM.prototype.random = function() {
  var self = this;
  var screen = this.screen;
  var keys = Object.keys(this.windows);
  keys.forEach(function(id, index) {
    self.move(id, Math.floor(Math.random()*(screen.width-300)), Math.floor(Math.random()*(screen.height-300)));    
  });
};

NWM.prototype.globalSmall = function() {
  var self = this;
  var keys = Object.keys(this.windows);
  keys.forEach(function(id, index) {
    self.resize(id, 200, 200);    
  });  
};

var tweens = [];

NWM.prototype.tween = function(id) {
  var self = this;
  var radius = 200;
  var circle_x = Math.floor(self.screen.width / 2);
  var circle_y = Math.floor(self.screen.height / 2);
  if(circle_x+radius*2 > self.screen.width) {
    circle_x -= radius*2;
  }
  if(circle_y+radius*2 > self.screen.height) {
    circle_y -= radius*2;
  }
  if(circle_x-radius*2 < 0) {
    circle_x += radius*2;
  }
  if(circle_y-radius*2 < 0) {
    circle_y += radius*2;
  }

  function circularPath(index) {

    var cx = circle_x;
    var cy = circle_y;
    var aStep = 3;   // 3 degrees per step
    var theta = index * aStep;  // +ve angles are cw

    var newX = cx + radius * Math.cos(theta * Math.PI / 180);
    var newY = cy + radius * Math.sin(theta * Math.PI / 180);

    // return an object defining state that can be understood by drawFn
    return  {x: newX, y: newY};
  }
  var step = 0;
  var result = setInterval(function() {
    step++;
    var pos = circularPath(step);
    self.move(id, Math.floor(pos.x), Math.floor(pos.y));
  }, 1000 / 60);
  tweens.push({ id: id, interval: result} );
};

NWM.prototype.stop = function() {
  for(var i = 0; i < tweens.length; i++) {
    clearInterval(tweens[i]); 
  }
  tweens = [];
};


var nwm = new NWM();
nwm.start();
