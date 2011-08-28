

#include <ev.h>

// poll with this.
watcher = malloc(sizeof(struct ev_io));
ev_io_init(watcher, callback, display->fd, EV_READ);
// maybe set fd nonblocking
// libev docs
// http://pod.tst.eu/http://cvs.schmorp.de/libev/ev.pod#code_ev_io_code_is_this_file_descrip
watcher->data = display;


// in the call back 
void callback(EV_P_ struct ev_io* watcher, int revents) {
  Display* display = watcher->data;

  XEvent ev;

  XNextEvent(display, &event)

  switch() {
   // do stuff
  }
}

