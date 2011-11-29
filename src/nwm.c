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
static void nwm_update_window(Window win);
static void nwm_remove_window(Window win, Bool destroyed);

static void nwm_scan_monitors();
void nwm_add_monitor();
void nwm_remove_monitor();

static void nwm_emit(callback_map event, nwm_event *ev);

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

static void (*handler[LASTEvent]) (XEvent *) = {
  [ButtonPress] = event_buttonpress,
  [ClientMessage] = event_clientmessage,
  [ConfigureRequest] = event_configurerequest,
  [ConfigureNotify] = event_configurenotify,
  [DestroyNotify] = event_destroynotify,
  [EnterNotify] = event_enternotify,
//  [Expose] = event_expose,
  [FocusIn] = event_focusin,
  [KeyPress] = event_keypress,
//  [MappingNotify] = event_mappingnotify,
  [MapRequest] = event_maprequest,
  [PropertyNotify] = event_propertynotify,
  [UnmapNotify] = event_unmapnotify
};

void nwm_emit(callback_map event, nwm_event *ev) {
  // NOP until I figure out how to call Node from here...
}


int nwm_init() {
  nwm.total_monitors = 0;
  nwm.keys = NULL;
  nwm.numlockmask = 0;

  // open the display
  if ( ( nwm.dpy = XOpenDisplay(NULL) ) == NULL ) {
    fprintf( stdout, "cannot connect to X server %s\n", XDisplayName(NULL));
    exit( -1 );
  }
  // set error handler
  XSetErrorHandler(xerror);
  XSync(nwm.dpy, False);

  // take the default screen
  int screen = DefaultScreen(nwm.dpy);
  // get the root window and screen geometry
  nwm.root = RootWindow(nwm.dpy, screen);
  nwm.screen_width = DisplayWidth(nwm.dpy, screen);
  nwm.screen_height = DisplayHeight(nwm.dpy, screen);
  // update monitor geometry (and create nwm.monitor)
  nwm_scan_monitors();

  XSetWindowAttributes wa;
  // subscribe to root window events e.g. SubstructureRedirectMask
  wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask|ButtonPressMask
                  |EnterWindowMask|LeaveWindowMask|StructureNotifyMask
                  |PropertyChangeMask;
  XSelectInput(nwm.dpy, nwm.root, wa.event_mask);
  nwm_grab_keys(nwm.dpy, nwm.root);

  // Scan and layout current windows
  unsigned int i, num;
  Window d1, d2, *wins = NULL;
  XWindowAttributes watt;
  // XQueryTree() function returns the root ID, the parent window ID, a pointer to
  // the list of children windows (NULL when there are no children), and
  // the number of children in the list for the specified window.
  if(XQueryTree(nwm.dpy, nwm.root, &d1, &d2, &wins, &num)) {
    for(i = 0; i < num; i++) {
      // if we can't read the window attributes,
      // or the window is a popup (transient or override_redirect), skip it
      if(!XGetWindowAttributes(nwm.dpy, wins[i], &watt)
      || watt.override_redirect || XGetTransientForHint(nwm.dpy, wins[i], &d1)) {
        continue;
      }
      // visible or minimized window ("Iconic state")
      if(watt.map_state == IsViewable )//|| getstate(wins[i]) == IconicState)
        nwm_add_window(wins[i], &watt);
    }
    for(i = 0; i < num; i++) { /* now the transients */
      if(!XGetWindowAttributes(nwm.dpy, wins[i], &watt))
        continue;
      if(XGetTransientForHint(nwm.dpy, wins[i], &d1)
      && (watt.map_state == IsViewable )) //|| getstate(wins[i]) == IconicState))
        nwm_add_window(wins[i], &watt);
    }
    if(wins) {
      // To free a non-NULL children list when it is no longer needed, use XFree()
      XFree(wins);
    }
  }

  // emit a rearrange
  nwm_emit(onRearrange, NULL);

  // initialize and start
  XSync(nwm.dpy, False);

  // return the connection number so the node binding can use it with libev.
  return XConnectionNumber(nwm.dpy);
}

void nwm_empty_keys() {
  Key* curr = nwm.keys;
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

void nwm_grab_keys(Display* dpy, Window root) {
  nwm.numlockmask = updatenumlockmask(dpy);
  {
    unsigned int i;
    unsigned int modifiers[] = { 0, LockMask, nwm.numlockmask, nwm.numlockmask|LockMask };
    XUngrabKey(nwm.dpy, AnyKey, AnyModifier, nwm.root);
    for(Key* curr = nwm.keys; curr != NULL; curr = curr->next) {
      fprintf( stdout, "grab key -- key: %li modifier %d \n", curr->keysym, curr->mod);
      // also grab the combinations of screen lock and num lock (as those should not matter)
      for(i = 0; i < 4; i++) {
        XGrabKey(nwm.dpy, XKeysymToKeycode(nwm.dpy, curr->keysym), curr->mod | modifiers[i], nwm.root, True, GrabModeAsync, GrabModeAsync);
      }
    }
  }
}


void nwm_emit_function() {

}


void nwm_loop() {
  XEvent event;

  // main event loop
  while(XPending(nwm.dpy)) {
    XNextEvent(nwm.dpy, &event);
    fprintf(stdout, "got event %s (%d).\n", event_names[event.type], event.type);

    if(handler[event.type]) {
      handler[event.type](&event); /* call handler */
    } else {
      fprintf(stdout, "Did nothing with %s (%d)\n", event_names[event.type], event.type);
    }
  }
}

void nwm_move_window(Window win, int x, int y) {
  fprintf( stdout, "MoveWindow: id=%li x=%d y=%d \n", win, x, y);
  XMoveWindow(nwm.dpy, win, x, y);
  XFlush(nwm.dpy);
}

void nwm_resize_window(Window win, int width, int height) {
  fprintf( stdout, "ResizeWindow: id=%li width=%d height=%d \n", win, width, height);
  XResizeWindow(nwm.dpy, win, width, height);
  XFlush(nwm.dpy);
}

void nwm_focus_window(Window win){
  fprintf( stdout, "FocusWindow: id=%li\n", win);
  grabButtons(win, True);
  XSetInputFocus(nwm.dpy, win, RevertToPointerRoot, CurrentTime);
  Atom atom = XInternAtom(nwm.dpy, "WM_TAKE_FOCUS", False);
  SendEvent(nwm.dpy, win, atom);
  XFlush(nwm.dpy);
  nwm.selected = win;
}
void nwm_kill_window(Window win) {
  XEvent ev;
  // check whether the client supports "graceful" termination
  if(isprotodel(nwm.dpy, win)) {
    ev.type = ClientMessage;
    ev.xclient.window = win;
    ev.xclient.message_type = XInternAtom(nwm.dpy, "WM_PROTOCOLS", False);
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = XInternAtom(nwm.dpy, "WM_DELETE_WINDOW", False);
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(nwm.dpy, win, False, NoEventMask, &ev);
  } else {
    XGrabServer(nwm.dpy);
    XSetErrorHandler(xerrordummy);
    XSetCloseDownMode(nwm.dpy, DestroyAll);
    XKillClient(nwm.dpy, win);
    XSync(nwm.dpy, False);
    XSetErrorHandler(xerror);
    XUngrabServer(nwm.dpy);
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
  XConfigureWindow(nwm.dpy, win, value_mask, &wc);
}

void nwm_notify_window(Window win, int x, int y, int width, int height,
    int border_width, int above, int detail, int value_mask) {
  XConfigureEvent ce;

  ce.type = ConfigureNotify;
  ce.display = nwm.dpy;
  ce.event = win;
  ce.window = win;
  ce.x = x;
  ce.y = y;
  ce.width = width;
  ce.height = height;
  ce.border_width = border_width;
  ce.above = None;
  ce.override_redirect = False;
  XSendEvent(nwm.dpy, win, False, StructureNotifyMask, (XEvent *)&ce);
}

void nwm_add_window(Window win, XWindowAttributes *wa) {
  Window trans = None;
  Bool isfloating = False;
  XConfigureEvent ce;
  nwm_event event_data;

  // check whether the window is transient
  XGetTransientForHint(nwm.dpy, win, &trans);
  isfloating = (trans != None);

  fprintf( stderr, "Create client %li (x %d, y %d, w %d, h %d, float %d)\n", win, wa->x, wa->y, wa->width, wa->height, isfloating);
  // emit onAddWindow in Node.js
  event_data.type = nwm_Window;
  event_data.window.id = win;
  event_data.window.x = wa->x;
  event_data.window.y = wa->y;
  event_data.window.height = wa->height;
  event_data.window.width = wa->width;
  event_data.window.isfloating = isfloating;
  nwm_emit(onAddWindow, event_data);

  // push the window id so we know what windows we've seen
// TODO TODO TODO    this->seen_windows.push_back(win);

  nwm_update_window(win); // update title and class, emit onUpdateWindow

  // configure the window
  ce.type = ConfigureNotify;
  ce.display = nwm.dpy;
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

  XSendEvent(nwm.dpy, win, False, StructureNotifyMask, (XEvent *)&ce);
  // subscribe to window events
  XSelectInput(nwm.dpy, win, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
  grabButtons(win, False);

  if(isfloating) {
    XRaiseWindow(nwm.dpy, win);
  }

  // move and (finally) map the window
  XMoveResizeWindow(nwm.dpy, win, ce.x, ce.y, ce.width, ce.height);
  XMapWindow(nwm.dpy, win);
}

void nwm_update_window(Window win) {
  char name[256];
  char klass[256];
  char instance[256];
  // update title
  Atom NetWMName = XInternAtom(nwm.dpy, "_NET_WM_NAME", False);
  if(!gettextprop(nwm.dpy, win, NetWMName, name, sizeof name))
    gettextprop(nwm.dpy, win, XA_WM_NAME, name, sizeof name);
  if(name[0] == '\0') /* hack to mark broken clients */
    strcpy(name, broken);
  // update class
  XClassHint ch = { 0 };
  if(XGetClassHint(nwm.dpy, win, &ch)) {
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

  nwm_window_title event_data;

  event_data.type = nwm_Windowtitle;
  event_data.id = win;
  event_data.title = name;
  event_data.instance = instance;
  event_data.klass = klass;

  // emit onUpdateWindow
  nwm_emit(onUpdateWindow, event_data);
}

void nwm_remove_window(Window win, Bool destroyed) {
  fprintf( stdout, "HandleRemove - emit onRemovewindow, %li\n", win);
  nwm_event event_data;
  event_data.type = nwm_Window;
  event_data.id = win;

  // emit a remove
  nwm_emit(onRemoveWindow, event_data);
  if(!destroyed) {
    XGrabServer(nwm.dpy);
    XUngrabButton(nwm.dpy, AnyButton, AnyModifier, win);
    XSync(nwm.dpy, False);
    XUngrabServer(nwm.dpy);
  }
  // remove from seen list of windows
  // TODO TODO TODO std::vector<Window>& vec = nwm.seen_windows; // use shorter name
  // TODO TODO TODO std::vector<Window>::iterator newEnd = std::remove(vec.begin(), vec.end(), win);
  // TODO TODO TODO vec.erase(newEnd, vec.end());

  fprintf( stdout, "Focusing to root window\n");
  nwm_focus_window(nwm.root);
  fprintf( stdout, "Emitting rearrange\n");
  nwm_emit(onRearrange, NULL);
}


static void nwm_scan_monitors() {
  Local<Value> argv[1];

  if(XineramaIsActive(nwm.dpy)) {
    fprintf( stdout, "Xinerama active\n");
    int i, n, nn;
    unsigned int j;
    XineramaScreenInfo *info = XineramaQueryScreens(nwm.dpy, &nn);
    XineramaScreenInfo *unique = NULL;

    n = nwm.total_monitors;
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
        nwm.total_monitors++;
      }
      // update monitor dimensions
      //  We just emit the monitors and don't track the dimensions in the binding at all.
      for(i = 0; i < nn; i++) {
        // emit ADD MONITOR or UPDATE MONITOR here
        argv[0] = NodeWM::makeMonitor(i, unique[i].x_org, unique[i].y_org, unique[i].width, unique[i].height);
        fprintf( stdout, "Emit monitor %d\n", i);
        if(i >= n) {
          nwm.Emit(onAddMonitor, 1, argv);
        } else {
          nwm.Emit(onUpdateMonitor, 1, argv);
        }
      }
    } else { // fewer monitors available nn < n
      fprintf( stdout, "Less monitors available %d %d\n", n, nn);
      for(i = nn; i < n; i++) {
        // emit REMOVE MONITOR (i)
        argv[0] = Integer::New(i);
        nwm.Emit(onRemoveMonitor, 1, argv);
        // remove monitor
        nwm.total_monitors--;
      }
    }
    free(unique);
  } else {
    if(nwm.total_monitors == 0) {
      nwm.total_monitors++;
      // emit ADD MONITOR
      argv[0] = NodeWM::makeMonitor(0, 0, 0, nwm.screen_width, nwm.screen_height);
      nwm.Emit(onAddMonitor, 1, argv);
    }
  }
  // update the selected monitor on Node.js side
  int x, y;
  if(nwm.getrootptr(&x, &y)) {
    fprintf(stdout, "EmitEnterNotify wid = %li \n", nwm.root);
    Local<Object> result = Object::New();
    result->Set(String::NewSymbol("id"), Integer::New(nwm.root));
    result->Set(String::NewSymbol("x"), Integer::New(x));
    result->Set(String::NewSymbol("y"), Integer::New(y));
    argv[0] = result;
    nwm.Emit(onEnterNotify, 1, argv);
  }

  fprintf( stdout, "Done with updategeom\n");
}


  static void GrabMouseRelease(Window id) {
    // disabled for now
    return;
/*
    XEvent ev;
    int x, y;
    Local<Value> argv[1];

    if(XGrabPointer(nwm.dpy, nwm.root, False,
      ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync,
      GrabModeAsync, None, XCreateFontCursor(nwm.dpy, XC_fleur), CurrentTime) != GrabSuccess) {
      return;
    }
    if(!nwm.getrootptr(&x, &y)) {
      return;
    }
    do{
      XMaskEvent(nwm.dpy, ButtonPressMask|ButtonReleaseMask|PointerMotionMask|ExposureMask|SubstructureRedirectMask, &ev);
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
            nwm.Emit(onMouseDrag, 1, argv);
          }
          break;
      }
    } while(ev.type != ButtonRelease);

    XUngrabPointer(nwm.dpy, CurrentTime); */
  }

static void event_buttonpress(XEvent *e) {
  fprintf(stdout, "Handle(mouse)ButtonPress\n");
  // fetch root_x,root_y
  Local<Value> argv[1];
  argv[0] = NodeWM::eventToNode(e);
  // call the callback in Node.js, passing the window object...
  nwm.Emit(onMouseDown, 1, argv);
  fprintf(stdout, "Call cbButtonPress\n");
  // now emit the drag events
  GrabMouseRelease(hw, e->xbutton.window);
}

static void event_clientmessage(XEvent *e) {
  XClientMessageEvent *cme = &e->xclient;
  Atom NetWMState = XInternAtom(nwm.dpy, "_NET_WM_STATE", False);
  Atom NetWMFullscreen = XInternAtom(nwm.dpy, "_NET_WM_STATE_FULLSCREEN", False);
  Local<Value> argv[2];

  if(cme->message_type == NetWMState && cme->data.l[1] == NetWMFullscreen) {
    argv[0] = Integer::New(cme->window);
    if(cme->data.l[0]) {
      XChangeProperty(nwm.dpy, cme->window, NetWMState, XA_ATOM, 32,
                      PropModeReplace, (unsigned char*)&NetWMFullscreen, 1);
      argv[1] = Integer::New(1);
      XRaiseWindow(nwm.dpy, cme->window);
    }
    else {
      XChangeProperty(nwm.dpy, cme->window, NetWMState, XA_ATOM, 32,
                      PropModeReplace, (unsigned char*)0, 0);
      argv[1] = Integer::New(0);
    }
    nwm.Emit(onFullscreen, 2, argv);
  }
}

static void event_configurerequest(XEvent *e) {
  // dwm checks for whether the window is known,
  // only unknown windows are allowed to configure themselves.
  Local<Value> argv[1];
  argv[0] = NodeWM::eventToNode(&event);
  // Node should call AllowReconfigure()or ConfigureNotify() + Move/Resize etc.
  nwm.Emit(onConfigureRequest, 1, argv);
}

static void event_configurenotify(XEvent *e) {
  XConfigureEvent *ev = &e->xconfigure;

  if(ev->window == nwm.root) {
    nwm.screen_width = ev->width;
    nwm.screen_height = ev->height;
    // update monitor structures
    nwm_scan_monitors(hw);
    nwm.Emit(onRearrange, 0, 0);
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
  nwm.Emit(onEnterNotify, 1, argv);
}

static void event_focusin(XEvent *e) {
  XFocusChangeEvent *ev = &e->xfocus;
  fprintf(stdout, "HandleFocusIn for window id %li\n", ev->window);
  if(nwm.selected && ev->window != nwm.selected){
    bool found = (std::find(nwm.seen_windows.begin(), nwm.seen_windows.end(), ev->window) != nwm.seen_windows.end());
    // Preventing focus stealing
    // http://mail.gnome.org/archives/wm-spec-list/2003-May/msg00013.html
    // We will always revert the focus to whatever was last set by Node (e.g. enterNotify).
    // This prevents naughty applications from stealing the focus permanently.
    if(found) {
      // only revert if the change was to a top-level window that we manage
      // For instance, FF menus would otherwise get reverted..
      fprintf(stdout, "Reverting focus change by window id %li to %li \n", ev->window, nwm.selected);
      RealFocus(hw, nwm.selected);
    }
  }
}

static void event_keypress(XEvent *e) {
  KeySym keysym;
  XKeyEvent *ev;

  ev = &e->xkey;
  keysym = XKeycodeToKeysym(nwm.dpy, (KeyCode)ev->keycode, 0);
  Local<Value> argv[1];
  // we always unset numlock and LockMask since those should not matter
  argv[0] = NodeWM::makeKeyPress(ev->x, ev->y, ev->keycode, keysym, (ev->state & ~(nwm.numlockmask|LockMask)));
  // call the callback in Node.js, passing the window object...
  nwm.Emit(onKeyPress, 1, argv);
}

static void event_maprequest(XEvent *e) {
  // read the window attrs, then add it to the managed windows...
  XWindowAttributes wa;
  XMapRequestEvent *ev = &event.xmaprequest;
  if(!XGetWindowAttributes(nwm.dpy, ev->window, &wa)) {
    fprintf(stdout, "XGetWindowAttributes failed\n");
    return;
  }
  if(wa.override_redirect)
    return;
  fprintf(stdout, "MapRequest\n");
  bool found = (std::find(nwm.seen_windows.begin(), nwm.seen_windows.end(), ev->window) != nwm.seen_windows.end());
  if(!found) {
    // only map new windows
    NodeWM::HandleAdd(hw, ev->window, &wa);
    // emit a rearrange
    nwm.Emit(onRearrange, 0, 0);
  } else {
    fprintf(stdout, "Window is known\n");
  }
}

static void event_propertynotify(XEvent *e) {
  XPropertyEvent *ev = &e->xproperty;
  // could be used for tracking hints, transient status and window name
  if((ev->window == nwm.root) && (ev->atom == XA_WM_NAME)) {
    // the root window name has changed
  } else if(ev->state == PropertyDelete) {
    return; // ignore property deletes
  } else {
    Atom NetWMName = XInternAtom(nwm.dpy, "_NET_WM_NAME", False);
    if(ev->atom == XA_WM_NAME || ev->atom == NetWMName) {
      nwm.updateWindowStr(ev->window); // update title and class
    }
  }
}

static void event_unmapnotify(XEvent *e) {
  nwm_remove_window(hw, event.xunmap.window, False);
}
