#include <vector>
#include <algorithm>
#include <errno.h>
#include <locale.h>
#include <stdarg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xinerama.h>

#include "nwm.h"
#include "x11_misc.cc"


// INTERNAL API

// the nwm struct can be static, since it's not like you can
// run multiple instances of a window manager anyway

static void nwm_add_window(Window win, XWindowAttributes *wa);
// OLD: static void HandleAdd(NodeWM* hw, Window win, XWindowAttributes *wa);
static void nwm_update_window(Window win);
static void nwm_remove_window(Window win);

static void nwm_scan_monitors();
static void nwm_add_monitor();
static void nwm_remove_monitor();

// these should go into a function dispach table indexed by the Xevent type
static void event_buttonpress(XEvent *e);
static void event_clientmessage(XEvent *e);
static void event_configurerequest(XEvent *e);
static void event_configurenotify(XEvent *e);
static void event_destroynotify(XEvent *e);
static void event_enternotify(XEvent *e);
static void event_focusin(XEvent *e);
static void event_keypress(XEvent *e);
static void event_maprequest(XEvent *e);
static void event_propertynotify(XEvent *e);
static void event_unmapnotify(XEvent *e);


// then we need another set piece, which binds Node to the nwm library

// just like X11, we should have a single event type which is
// a union of the different kinds of events.

static const char broken[] = "broken";

struct NodeWinMan {
  // X11
  Display *dpy;
  GC gc;
  Window root;
  Window selected;
  // We only track the number of monitors: that allows us to tell if a monitor has been removed or added.
  unsigned int total_monitors;
  // We only track the window ids of windows, nothing else
  std::vector <Window> seen_windows;
  // screen dimensions
  int screen_width, screen_height;
  // grabbed keys
  Key* keys;
  unsigned int numlockmask;
};

static void (*handler[LASTEvent]) (XEvent *) = {
  [ButtonPress] = buttonpress,
  [ClientMessage] = clientmessage,
  [ConfigureRequest] = configurerequest,
  [ConfigureNotify] = configurenotify,
  [DestroyNotify] = destroynotify,
  [EnterNotify] = enternotify,
  [Expose] = expose,
  [FocusIn] = focusin,
  [KeyPress] = keypress,
  [MappingNotify] = mappingnotify,
  [MapRequest] = maprequest,
  [PropertyNotify] = propertynotify,
  [UnmapNotify] = unmapnotify
};


static NodeWinMan nwm;

int nwm_init() {
  nwm.total_monitors = 0;
  nwm.keys = NULL;
  nwm.numlockmask = 0;

  // open the display
  if ( ( hw->dpy = XOpenDisplay(NULL) ) == NULL ) {
    fprintf( stdout, "cannot connect to X server %s\n", XDisplayName(NULL));
    exit( -1 );
  }
  // set error handler
  XSetErrorHandler(xerror);
  XSync(hw->dpy, False);

  // take the default screen
  int screen = DefaultScreen(hw->dpy);
  // get the root window and screen geometry
  hw->root = RootWindow(hw->dpy, screen);
  hw->screen_width = DisplayWidth(hw->dpy, screen);
  hw->screen_height = DisplayHeight(hw->dpy, screen);
  // update monitor geometry (and create hw->monitor)
  updateGeometry(hw);

  XSetWindowAttributes wa;
  // subscribe to root window events e.g. SubstructureRedirectMask
  wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask|ButtonPressMask
                  |EnterWindowMask|LeaveWindowMask|StructureNotifyMask
                  |PropertyChangeMask;
  XSelectInput(hw->dpy, hw->root, wa.event_mask);
  GrabKeys(hw, hw->dpy, hw->root);

  // Scan and layout current windows
  unsigned int i, num;
  Window d1, d2, *wins = NULL;
  XWindowAttributes watt;
  // XQueryTree() function returns the root ID, the parent window ID, a pointer to
  // the list of children windows (NULL when there are no children), and
  // the number of children in the list for the specified window.
  if(XQueryTree(hw->dpy, hw->root, &d1, &d2, &wins, &num)) {
    for(i = 0; i < num; i++) {
      // if we can't read the window attributes,
      // or the window is a popup (transient or override_redirect), skip it
      if(!XGetWindowAttributes(hw->dpy, wins[i], &watt)
      || watt.override_redirect || XGetTransientForHint(hw->dpy, wins[i], &d1)) {
        continue;
      }
      // visible or minimized window ("Iconic state")
      if(watt.map_state == IsViewable )//|| getstate(wins[i]) == IconicState)
        NodeWM::HandleAdd(hw, wins[i], &watt);
    }
    for(i = 0; i < num; i++) { /* now the transients */
      if(!XGetWindowAttributes(hw->dpy, wins[i], &watt))
        continue;
      if(XGetTransientForHint(hw->dpy, wins[i], &d1)
      && (watt.map_state == IsViewable )) //|| getstate(wins[i]) == IconicState))
        NodeWM::HandleAdd(hw, wins[i], &watt);
    }
    if(wins) {
      // To free a non-NULL children list when it is no longer needed, use XFree()
      XFree(wins);
    }
  }

  // emit a rearrange
  hw->Emit(onRearrange, 0, 0);

  // initialize and start
  XSync(hw->dpy, False);

  // return the connection number so the node binding can use it with libev.
  return XConnectionNumber(hw->dpy)
}

void nwm_empty_keys() {
  Key* curr = hw->keys;
  Key* next;
  while(curr != NULL) {
    next = curr->next;
    free(curr);
    curr = next;
  }
}

void nwm_add_key(Key** keys, KeySym keysym, unsigned int mod) {
  Key* curr;
  if(!(curr = (Key*)calloc(1, sizeof(Key)))) {
    fprintf( stdout, "fatal: could not malloc() %lu bytes\n", sizeof(Key));
    exit( -1 );
  }
  curr->keysym = keysym;
  curr->mod = mod;
  curr->next = *keys;
  *keys = curr;
}

void nwm_grab_keys(NodeWM* hw, Display* dpy, Window root) {
  hw->updatenumlockmask();
  {
    unsigned int i;
    unsigned int modifiers[] = { 0, LockMask, hw->numlockmask, hw->numlockmask|LockMask };
    XUngrabKey(hw->dpy, AnyKey, AnyModifier, hw->root);
    for(Key* curr = hw->keys; curr != NULL; curr = curr->next) {
      fprintf( stdout, "grab key -- key: %li modifier %d \n", curr->keysym, curr->mod);
      // also grab the combinations of screen lock and num lock (as those should not matter)
      for(i = 0; i < 4; i++) {
        XGrabKey(hw->dpy, XKeysymToKeycode(hw->dpy, curr->keysym), curr->mod | modifiers[i], hw->root, True, GrabModeAsync, GrabModeAsync);
      }
    }
  }
}


void nwm_emit_function() {

}


void nwm_loop() {
  XEvent event;

  // main event loop
  while(XPending(hw->dpy)) {
    XNextEvent(hw->dpy, &event);
    fprintf(stdout, "got event %s (%d).\n", event_names[event.type], event.type);

    if(handler[ev.type]) {
      handler[ev.type](&ev); /* call handler */
    } else {
      fprintf(stdout, "Did nothing with %s (%d)\n", event_names[event.type], event.type);
    }
  }
}

void nwm_move_window(Window win, int x, int y) {
  fprintf( stdout, "MoveWindow: id=%li x=%d y=%d \n", win, x, y);
  XMoveWindow(hw->dpy, win, x, y);
  XFlush(hw->dpy);
}

void nwm_resize_window(Window win, int width, int height) {
  fprintf( stdout, "ResizeWindow: id=%li width=%d height=%d \n", win, width, height);
  XResizeWindow(hw->dpy, win, width, height);
  XFlush(hw->dpy);
}

void nwm_focus_window(Window win){
  fprintf( stdout, "FocusWindow: id=%li\n", win);
  hw->GrabButtons(win, True);
  XSetInputFocus(hw->dpy, win, RevertToPointerRoot, CurrentTime);
  Atom atom = XInternAtom(hw->dpy, "WM_TAKE_FOCUS", False);
  SendEvent(hw, win, atom);
  XFlush(hw->dpy);
  hw->selected = win;
}
void nwm_kill_window(Window win) {
  XEvent ev;
  // check whether the client supports "graceful" termination
  if(isprotodel(hw->dpy, win)) {
    ev.type = ClientMessage;
    ev.xclient.window = win;
    ev.xclient.message_type = XInternAtom(hw->dpy, "WM_PROTOCOLS", False);
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = XInternAtom(hw->dpy, "WM_DELETE_WINDOW", False);
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(hw->dpy, win, False, NoEventMask, &ev);
  } else {
    XGrabServer(hw->dpy);
    XSetErrorHandler(xerrordummy);
    XSetCloseDownMode(hw->dpy, DestroyAll);
    XKillClient(hw->dpy, win);
    XSync(hw->dpy, False);
    XSetErrorHandler(xerror);
    XUngrabServer(hw->dpy);
  }
}

void nwm_configure_window(Window win, int x, int y, int width, int height,
  int border_width, int above, int detail, int value_mask) {
  XWindowChanges wc;
  wc.x = x;
  wc.y = y;
  wc.width = width;
  wc.height = height;
  wc.border_width = border_width;
  wc.sibling = above;
  wc.stack_mode = detail;
  XConfigureWindow(this->dpy, win, value_mask, &wc);
}

void nwm_notify_window(Window win, int x, int y, int width, int height,
    int border_width, int above, int detail, int value_mask) {
  XConfigureEvent ce;

  ce.type = ConfigureNotify;
  ce.display = this->dpy;
  ce.event = win;
  ce.window = win;
  ce.x = x;
  ce.y = y;
  ce.width = width;
  ce.height = height;
  ce.border_width = border_width;
  ce.above = None;
  ce.override_redirect = False;
  XSendEvent(this->dpy, win, False, StructureNotifyMask, (XEvent *)&ce);
}

static void nwm_add_window(Window win, XWindowAttributes *wa);
  static void HandleAdd(NodeWM* hw, Window win, XWindowAttributes *wa) {
    Window trans = None;
    TryCatch try_catch;
    Bool isfloating = false;
    XConfigureEvent ce;

    // check whether the window is transient
    XGetTransientForHint(hw->dpy, win, &trans);
    isfloating = (trans != None);

    // emit onAddWindow in Node.js
    hw->addWindow(win, wa->x, wa->y, wa->height, wa->width, isfloating);
    hw->updateWindowStr(win); // update title and class, emit onUpdateWindow

    // configure the window
    ce.type = ConfigureNotify;
    ce.display = hw->dpy;
    ce.event = win;
    ce.window = win;
    ce.x = wa->x;
    ce.y = wa->y;
    ce.width = wa->width;
    ce.height = wa->height;
    ce.border_width = wa->border_width;
    ce.above = None;
    ce.override_redirect = False;

    fprintf( stdout, "manage: x=%d y=%d width=%d height=%d \n", ce.x, ce.y, ce.width, ce.height);

    XSendEvent(hw->dpy, win, False, StructureNotifyMask, (XEvent *)&ce);
    // subscribe to window events
    XSelectInput(hw->dpy, win, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
    hw->GrabButtons(win, False);

    if(isfloating) {
      XRaiseWindow(hw->dpy, win);
    }

    // move and (finally) map the window
    XMoveResizeWindow(hw->dpy, win, ce.x, ce.y, ce.width, ce.height);
    XMapWindow(hw->dpy, win);
 }
  void addWindow(Window win, int x, int y, int width, int height, Bool isfloating) {
    fprintf( stderr, "Create client %li (x %d, y %d, w %d, h %d, float %d)\n", win, x, y, width, height, isfloating);

    // push the window id so we know what windows we've seen
    this->seen_windows.push_back(win);

    // window object to return
    Local<Object> result = Object::New();
    // read and set the window geometry
    result->Set(String::NewSymbol("id"), Integer::New(win));
    result->Set(String::NewSymbol("x"), Integer::New(x));
    result->Set(String::NewSymbol("y"), Integer::New(y));
    result->Set(String::NewSymbol("height"), Integer::New(height));
    result->Set(String::NewSymbol("width"), Integer::New(width));
    result->Set(String::NewSymbol("isfloating"), Integer::New(isfloating));

    Local<Value> argv[1];
    argv[0] = result;
    this->Emit(onAddWindow, 1, argv);
  }
}

static void nwm_update_window(Window win) {
  char name[256];
  char klass[256];
  char instance[256];
  // update title
  Atom NetWMName = XInternAtom(dpy, "_NET_WM_NAME", False);
  if(!gettextprop(this->dpy, win, NetWMName, name, sizeof name))
    gettextprop(this->dpy, win, XA_WM_NAME, name, sizeof name);
  if(name[0] == '\0') /* hack to mark broken clients */
    strcpy(name, broken);
  // update class
  XClassHint ch = { 0 };
  if(XGetClassHint(this->dpy, win, &ch)) {
    if(ch.res_class) {
      strncpy(klass, ch.res_class, 256-1 );
    } else {
      strncpy(klass, broken, 256-1 );
    }
    klass[256-1] = 0;
    if(ch.res_name) {
      strncpy(instance, ch.res_name, 256-1 );
    } else {
      strncpy(instance, broken, 256-1 );
    }
    instance[256-1] = 0;
    if(ch.res_class)
      XFree(ch.res_class);
    if(ch.res_name)
      XFree(ch.res_name);
  }

  Local<Object> result = Object::New();
  result->Set(String::NewSymbol("id"), Integer::New(win));
  result->Set(String::NewSymbol("title"), String::New(name));
  result->Set(String::NewSymbol("instance"), String::New(instance));
  result->Set(String::NewSymbol("class"), String::New(klass));

  // emit onUpdateWindow
  Local<Value> argv[1];
  argv[0] = result;
  this->Emit(onUpdateWindow, 1, argv);
}

static void nwm_remove_window(Window win) {
  static void HandleRemove(NodeWM* hw, Window win, Bool destroyed) {
    Local<Value> argv[1];
    argv[0] = Integer::New(win);
    fprintf( stdout, "HandleRemove - emit onRemovewindow, %li\n", win);
    // emit a remove
    hw->Emit(onRemoveWindow, 1, argv);
    if(!destroyed) {
      XGrabServer(hw->dpy);
      XUngrabButton(hw->dpy, AnyButton, AnyModifier, win);
      XSync(hw->dpy, False);
      XUngrabServer(hw->dpy);
    }
    // remove from seen list of windows
    std::vector<Window>& vec = hw->seen_windows; // use shorter name
    std::vector<Window>::iterator newEnd = std::remove(vec.begin(), vec.end(), win);
    vec.erase(newEnd, vec.end());

    fprintf( stdout, "Focusing to root window\n");
    RealFocus(hw, hw->root);
    fprintf( stdout, "Emitting rearrange\n");
    hw->Emit(onRearrange, 0, 0);
  }
}


static void nwm_scan_monitors() {
  Local<Value> argv[1];

  if(XineramaIsActive(hw->dpy)) {
    fprintf( stdout, "Xinerama active\n");
    int i, n, nn;
    unsigned int j;
    XineramaScreenInfo *info = XineramaQueryScreens(hw->dpy, &nn);
    XineramaScreenInfo *unique = NULL;

    n = hw->total_monitors;
    fprintf( stdout, "Monitors known %d, monitors found %d\n", n, nn);
    /* only consider unique geometries as separate screens */
    if(!(unique = (XineramaScreenInfo *)malloc(sizeof(XineramaScreenInfo) * nn))) {
      fprintf( stdout, "fatal: could not malloc() %lu bytes\n", sizeof(XineramaScreenInfo) * nn);
      exit( -1 );
    }
    for(i = 0, j = 0; i < nn; i++)
      if(isuniquegeom(unique, j, &info[i]))
        memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
    XFree(info);
    nn = j;
    if(n <= nn) {
      // reserve space
      for(i = 0; i < (nn - n); i++) { // new monitors available
        fprintf(stdout, "Monitor %d \n", i);
        hw->total_monitors++;
      }
      // update monitor dimensions
      //  We just emit the monitors and don't track the dimensions in the binding at all.
      for(i = 0; i < nn; i++) {
        // emit ADD MONITOR or UPDATE MONITOR here
        argv[0] = NodeWM::makeMonitor(i, unique[i].x_org, unique[i].y_org, unique[i].width, unique[i].height);
        fprintf( stdout, "Emit monitor %d\n", i);
        if(i >= n) {
          hw->Emit(onAddMonitor, 1, argv);
        } else {
          hw->Emit(onUpdateMonitor, 1, argv);
        }
      }
    } else { // fewer monitors available nn < n
      fprintf( stdout, "Less monitors available %d %d\n", n, nn);
      for(i = nn; i < n; i++) {
        // emit REMOVE MONITOR (i)
        argv[0] = Integer::New(i);
        hw->Emit(onRemoveMonitor, 1, argv);
        // remove monitor
        hw->total_monitors--;
      }
    }
    free(unique);
  } else {
    if(hw->total_monitors == 0) {
      hw->total_monitors++;
      // emit ADD MONITOR
      argv[0] = NodeWM::makeMonitor(0, 0, 0, hw->screen_width, hw->screen_height);
      hw->Emit(onAddMonitor, 1, argv);
    }
  }
  // update the selected monitor on Node.js side
  int x, y;
  if(hw->getrootptr(&x, &y)) {
    fprintf(stdout, "EmitEnterNotify wid = %li \n", hw->root);
    Local<Object> result = Object::New();
    result->Set(String::NewSymbol("id"), Integer::New(hw->root));
    result->Set(String::NewSymbol("x"), Integer::New(x));
    result->Set(String::NewSymbol("y"), Integer::New(y));
    argv[0] = result;
    hw->Emit(onEnterNotify, 1, argv);
  }

  fprintf( stdout, "Done with updategeom\n");
}

  void GrabButtons(Window wnd, Bool focused) {
    this->updatenumlockmask();
//    {
//      unsigned int i;
//      unsigned int modifiers[] = { 0, LockMask, this->numlockmask, this->numlockmask|LockMask };
//      XUngrabButton(this->dpy, AnyButton, AnyModifier, wnd);
// If focused, then we only grab the modifier keys. Otherwise, we grab all buttons..
//      if(focused) {
//        fprintf( stdout, "GRABBUTTONS - focused: true\n");
//          for(i = 0; i < 4; i++) {
//            XGrabButton(dpy, Button1,
//                              Mod4Mask|ControlMask|modifiers[i],
//                              wnd, False, (ButtonPressMask|ButtonReleaseMask),
//                              GrabModeAsync, GrabModeSync, None, None);
//          }
//      } else {
//        fprintf( stdout, "GRABBUTTONS - focused: false\n");
//        XGrabButton(this->dpy, AnyButton, AnyModifier, wnd, False,
//                    (ButtonPressMask|ButtonReleaseMask), GrabModeAsync, GrabModeSync, None, None);
//      }
//    }
  }

  static void GrabMouseRelease(NodeWM* hw, Window id) {
    // disabled for now
    return;

    XEvent ev;
    int x, y;
    Local<Value> argv[1];

    if(XGrabPointer(hw->dpy, hw->root, False,
      ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync,
      GrabModeAsync, None, XCreateFontCursor(hw->dpy, XC_fleur), CurrentTime) != GrabSuccess) {
      return;
    }
    if(!hw->getrootptr(&x, &y)) {
      return;
    }
    do{
      XMaskEvent(hw->dpy, ButtonPressMask|ButtonReleaseMask|PointerMotionMask|ExposureMask|SubstructureRedirectMask, &ev);
      switch(ev.type) {
        case ConfigureRequest:
          // handle normally
          break;
        case Expose:
          // handle normally
          break;
        case MapRequest:
          // handle normally
          break;
        case MotionNotify:
          {
            argv[0] = NodeWM::makeMouseDrag(id, x, y, ev.xmotion.x, ev.xmotion.y); // , ev.state);
            hw->Emit(onMouseDrag, 1, argv);
          }
          break;
      }
    } while(ev.type != ButtonRelease);

    XUngrabPointer(hw->dpy, CurrentTime);
  }

static void event_buttonpress(XEvent *e) {
  fprintf(stdout, "Handle(mouse)ButtonPress\n");
  // fetch root_x,root_y
  Local<Value> argv[1];
  argv[0] = NodeWM::eventToNode(e);
  // call the callback in Node.js, passing the window object...
  hw->Emit(onMouseDown, 1, argv);
  fprintf(stdout, "Call cbButtonPress\n");
  // now emit the drag events
  GrabMouseRelease(hw, e->xbutton.window);
}

static void event_clientmessage(XEvent *e) {
  XClientMessageEvent *cme = &e->xclient;
  Atom NetWMState = XInternAtom(hw->dpy, "_NET_WM_STATE", False);
  Atom NetWMFullscreen = XInternAtom(hw->dpy, "_NET_WM_STATE_FULLSCREEN", False);
  Local<Value> argv[2];

  if(cme->message_type == NetWMState && cme->data.l[1] == NetWMFullscreen) {
    argv[0] = Integer::New(cme->window);
    if(cme->data.l[0]) {
      XChangeProperty(hw->dpy, cme->window, NetWMState, XA_ATOM, 32,
                      PropModeReplace, (unsigned char*)&NetWMFullscreen, 1);
      argv[1] = Integer::New(1);
      XRaiseWindow(hw->dpy, cme->window);
    }
    else {
      XChangeProperty(hw->dpy, cme->window, NetWMState, XA_ATOM, 32,
                      PropModeReplace, (unsigned char*)0, 0);
      argv[1] = Integer::New(0);
    }
    hw->Emit(onFullscreen, 2, argv);
  }
}

static void event_configurerequest(XEvent *e) {
  // dwm checks for whether the window is known,
  // only unknown windows are allowed to configure themselves.
  Local<Value> argv[1];
  argv[0] = NodeWM::eventToNode(&event);
  // Node should call AllowReconfigure()or ConfigureNotify() + Move/Resize etc.
  hw->Emit(onConfigureRequest, 1, argv);
}

static void event_configurenotify(XEvent *e) {
  XConfigureEvent *ev = &e->xconfigure;

  if(ev->window == hw->root) {
    hw->screen_width = ev->width;
    hw->screen_height = ev->height;
    // update monitor structures
    updateGeometry(hw);
    hw->Emit(onRearrange, 0, 0);
  }
}

static void event_destroynotify(XEvent *e) {
  nwm_remove_window(hw, event.xdestroywindow.window, True);
}

static void event_enternotify(XEvent *e) {
  Local<Value> argv[1];
  fprintf(stdout, "HandleEnterNotify wid = %li \n", e->xcrossing.window);
  argv[0] = NodeWM::eventToNode(e);
  // call the callback in Node.js, passing the window object...
  // Note that this also called for the root window (focus monitor)
  hw->Emit(onEnterNotify, 1, argv);
}

static void event_focusin(XEvent *e) {
  XFocusChangeEvent *ev = &e->xfocus;
  fprintf(stdout, "HandleFocusIn for window id %li\n", ev->window);
  if(hw->selected && ev->window != hw->selected){
    bool found = (std::find(hw->seen_windows.begin(), hw->seen_windows.end(), ev->window) != hw->seen_windows.end());
    // Preventing focus stealing
    // http://mail.gnome.org/archives/wm-spec-list/2003-May/msg00013.html
    // We will always revert the focus to whatever was last set by Node (e.g. enterNotify).
    // This prevents naughty applications from stealing the focus permanently.
    if(found) {
      // only revert if the change was to a top-level window that we manage
      // For instance, FF menus would otherwise get reverted..
      fprintf(stdout, "Reverting focus change by window id %li to %li \n", ev->window, hw->selected);
      RealFocus(hw, hw->selected);
    }
  }
}

static void event_keypress(XEvent *e) {
  KeySym keysym;
  XKeyEvent *ev;

  ev = &e->xkey;
  keysym = XKeycodeToKeysym(hw->dpy, (KeyCode)ev->keycode, 0);
  Local<Value> argv[1];
  // we always unset numlock and LockMask since those should not matter
  argv[0] = NodeWM::makeKeyPress(ev->x, ev->y, ev->keycode, keysym, (ev->state & ~(hw->numlockmask|LockMask)));
  // call the callback in Node.js, passing the window object...
  hw->Emit(onKeyPress, 1, argv);
}

static void event_maprequest(XEvent *e) {
  // read the window attrs, then add it to the managed windows...
  XWindowAttributes wa;
  XMapRequestEvent *ev = &event.xmaprequest;
  if(!XGetWindowAttributes(hw->dpy, ev->window, &wa)) {
    fprintf(stdout, "XGetWindowAttributes failed\n");
    return;
  }
  if(wa.override_redirect)
    return;
  fprintf(stdout, "MapRequest\n");
  bool found = (std::find(hw->seen_windows.begin(), hw->seen_windows.end(), ev->window) != hw->seen_windows.end());
  if(!found) {
    // only map new windows
    NodeWM::HandleAdd(hw, ev->window, &wa);
    // emit a rearrange
    hw->Emit(onRearrange, 0, 0);
  } else {
    fprintf(stdout, "Window is known\n");
  }
}

static void event_propertynotify(XEvent *e) {
  XPropertyEvent *ev = &e->xproperty;
  // could be used for tracking hints, transient status and window name
  if((ev->window == hw->root) && (ev->atom == XA_WM_NAME)) {
    // the root window name has changed
  } else if(ev->state == PropertyDelete) {
    return; // ignore property deletes
  } else {
    Atom NetWMName = XInternAtom(hw->dpy, "_NET_WM_NAME", False);
    if(ev->atom == XA_WM_NAME || ev->atom == NetWMName) {
      hw->updateWindowStr(ev->window); // update title and class
    }
  }
}

static void event_unmapnotify(XEvent *e) {
  nwm_remove_window(hw, event.xunmap.window, False);
}
