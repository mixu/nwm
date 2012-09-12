function Nwm() {

}

Nwm.addMonitor = function(monitor) {
  this.monitors.push(new Monitor(monitor));
};

Nwm.updateMonitor = function(monitor) {

};

Nwm.removeMonitor = function(monitor) {

};

Nwm.addWindow = function(window) {
  // All new windows go on the first monitor, always. They can be moved by user actions.
};

module.exports = Nwm;
