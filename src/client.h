
#include "handler.h"

class Client {
public:
  Client(Window win, Monitor* monitor, int id, int x, int y, int width, int height) {
    this->win = win;
    this->mon = monitor;
    this->id = id;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
  }
  static Client* getByWindow(Monitor* monit, Window win);
  static Client* getById(Monitor* monit, int id);
  inline int getId() { return this->id; }
  inline Window getWin() { return this->win; }
  void attach();
  void detach(); 
  void resize(Display* dpy, int width, int height) {
    fprintf( stderr, "ResizeWindow: id=%d width=%d height=%d \n", id, width, height);    
    XResizeWindow(dpy, this->win, width, height);    
    XFlush(dpy);    
  }
  void move(Display* dpy, int x, int y) {
    fprintf( stderr, "MoveWindow: id=%d x=%d y=%d \n", id, x, y);    
    XMoveWindow(dpy, this->win, x, y);    
    XFlush(dpy);
  }
  void kill(Display* dpy) {
    XEvent ev;
    // check whether the client supports "graceful" termination
    if(this->isprotodel(dpy)) {
      ev.type = ClientMessage;
      ev.xclient.window = this->win;
      ev.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", False);
      ev.xclient.format = 32;
      ev.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
      ev.xclient.data.l[1] = CurrentTime;
      XSendEvent(dpy, this->win, False, NoEventMask, &ev);
    }
    else {
      XGrabServer(dpy);
      XSetErrorHandler(xerrordummy);
      XSetCloseDownMode(dpy, DestroyAll);
      XKillClient(dpy, this->win);
      XSync(dpy, False);
      XSetErrorHandler(xerror);
      XUngrabServer(dpy);
    }
  }
  private:
    int id;
    int x, y, width, height;
    Client *next;
    Client *snext;
    Monitor *mon;
    Window win;
    Bool isprotodel(Display* dpy) {
      int i, n;
      Atom *protocols;
      Bool ret = False;

      if(XGetWMProtocols(dpy, this->win, &protocols, &n)) {
        for(i = 0; !ret && i < n; i++)
          if(protocols[i] == XInternAtom(dpy, "WM_DELETE_WINDOW", False))
            ret = True;
        XFree(protocols);
      }
      return ret;
    };
};

