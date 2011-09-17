/**
 * These are just test stubs for now: testing a window manager is pretty complicated
 * as X11 events have to be generated etc.
 *
 * There would be quite a bit of instrumentation involved in getting these to run,
 * which might not be worth the effort. At least, first I'd like to work on improving logging
 * and only then see if these are needed.
 */

// Collection tests

exports['getting next item cycles through items'] = function(test) {

};

exports['get without lazyinit returns null for nonexistent ids'] = function(test) {

};

exports['get with lazyinit returns a default object for nonexistent ids'] = function(test) {

};


// Single monitor tests

exports['when started, the focused window is root, monitor is 0 and workspace is 1'] = function(test) {

};

exports['can load a single monitor'] = function(test) {

};

exports['adding a new window makes it the focused_window for the monitor, and the main_window for the workspace'] = function(test) {

};

exports['switching to workspace 2 produces an empty screen'] = function(test){

};

exports['moving a window to workspace 2 adds it and rearranges the windows'] = function(test){

};

exports['closing a window removes it from the set of windows and rearranges the windows'] = function(test) {

};

exports['moving focus between three windows works'] = function(test) {

};

// Multiple monitor tests

exports['can add two monitors'] = function(test) {

};

exports['newly added monitors are added to the current monitor'] = function(test) {

};

// MOUSE / FOCUS

exports['moving the mouse within the monitor changes window focus but not monitor'] = function(test) {

};

exports['moving the mouse from monitor 0 to monitor 1'] = function(test) {
  // when monitor 1 is empty
    // changes current monitor to 1
    // sets workspace.current to 1

  // when monitor 1 has a window
    // same as above
};

exports['changing workspace while focused on monitor 1'] = function(test) {
  // changes current workspace on monitor 1, but not on monitor 0

};

exports['adding new window while focused on monitor 1'] = function(test) {
  // adds the new window to monitor 1

};

// MOVING WINDOWS BETWEEN MONITORS

exports['moving a window from monitor 0 to monitor 1'] = function(test) {
  // adds the new window to monitor 1
  // removes the monitor from monitor 0
  // sets the window workspace to the workspace on monitor 1, not that of monitor 0

};

