/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying 
 * LICENSE file.
 */

#include <v8.h>
#include <node.h>
#include "/usr/local/node/include/node/ev/ev.h"

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

#include <assert.h>   // I include this to test return values the lazy way
#include <unistd.h>   // So we got the profile for 10 seconds
#define NIL (0)       // A name for the void pointer
#define MAXWIN 512
#include "event_names.h"


using namespace node;
using namespace v8;

typedef struct Monitor Monitor;
typedef struct Client Client;
struct Client {
  int id;
  int x, y, width, height;
  Client *next;
  Client *snext;
  Monitor *mon;
  Window win;
};

struct Monitor {
  int id;
  int x, y, width, height;
  Client *clients;
//  Client *sel;
//  Client *stack;
  Monitor *next;
  Window barwin;
};

// make these classes of their own

enum callback_map { 
  onAdd, 
  onRemove,
  onRearrange,
  onButtonPress,
  onConfigureRequest,
  onKeyPress,
  onEnterNotify,
  onLast
};


class HelloWorld: ObjectWrap
{
private:
  Display *dpy;
  Window wnd;
  GC gc;
  int screen, screen_width, screen_height;
  Window root;
  Window selected;
  // callbacks
  Persistent<Function>* callbacks[onLast];

  int next_index;

  Monitor* monit;
public:

  static Persistent<FunctionTemplate> s_ct;
  static void Init(Handle<Object> target) {
    HandleScope scope;
    // create a local FunctionTemplate
    Local<FunctionTemplate> t = FunctionTemplate::New(New);

    // initialize our template
    s_ct = Persistent<FunctionTemplate>::New(t);
    // set the field count
    s_ct->InstanceTemplate()->SetInternalFieldCount(1);
    // set the symbol for this function
    s_ct->SetClassName(String::NewSymbol("NodeWM"));

    // FUNCTIONS

    // callbacks
    NODE_SET_PROTOTYPE_METHOD(s_ct, "on", OnCallback);

    // API
    NODE_SET_PROTOTYPE_METHOD(s_ct, "moveWindow", MoveWindow);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "resizeWindow", ResizeWindow);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "focusWindow", FocusWindow);

    // Setting up
    NODE_SET_PROTOTYPE_METHOD(s_ct, "setup", Setup);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "scan", Scan);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "loop", Loop);

    // FINALLY: export the current function template
    target->Set(String::NewSymbol("NodeWM"),
                s_ct->GetFunction());
  }

  // C++ constructor
  HelloWorld() :
    next_index(1),
    monit(NULL)
  {
  }

  ~HelloWorld()
  {
  }

  // New method for v8
  static Handle<Value> New(const Arguments& args) {
    HandleScope scope;
    HelloWorld* hw = new HelloWorld();
    for(int i = 0; i < onLast; i++) {
      hw->callbacks[i] = NULL;
    }
    // use ObjectWrap.Wrap to store hw in this
    hw->Wrap(args.This());
    // return this
    return args.This();
  }

  // EVENTS

  static Handle<Value> OnCallback(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    v8::Local<v8::String> map[onLast+1] = {
        v8::String::New("add"),
        v8::String::New("remove"), 
        v8::String::New("rearrange"),
        v8::String::New("buttonPress"),
        v8::String::New("configureRequest"),
        v8::String::New("keyPress"),
        v8::String::New("enterNotify"),
      };

    v8::Local<v8::String> value  = Local<v8::String>::Cast(args[0]);
    int selected = -1;
    for(int i = 0; i < onLast; i++) {
      if( strcmp(*v8::String::AsciiValue(value),  *v8::String::AsciiValue(map[i])) == 0 ) {
        selected = i;
        break;        
      }
    }
    if(selected == -1) {
      return Undefined();
    }
    // store function
    hw->callbacks[selected] = cb_persist(args[1]);

    Local<Object> result = Object::New();
    result->Set(String::NewSymbol("test"), Integer::New(selected));
    return scope.Close(result);
  }

  void Emit(callback_map event, int argc, Handle<Value> argv[]) {
    TryCatch try_catch;
    if(this->callbacks[event] != NULL) {
      Handle<Function> *callback = cb_unwrap(this->callbacks[event]);
      (*callback)->Call(Context::GetCurrent()->Global(), argc, argv);                
      if (try_catch.HasCaught()) {
        FatalException(try_catch);
      }
    }
  }

  // GET from Node.js

  static int getIntegerValue(Handle<Object> obj, Local<String> symbol) {
    v8::Handle<v8::Value> h = obj->Get(symbol);    
    int val = h->IntegerValue();
    fprintf( stderr, "getIntegerValue: %d \n", val);
    return val;
  }

  // Client management

  static void attach(Client *c) {
    c->next = c->mon->clients;
    c->mon->clients = c;
  }

  static void detach(Client *c) {
    Client **tc;
    for(tc = &c->mon->clients; *tc && *tc != c; tc = &(*tc)->next);
    *tc = c->next;    
  }

  static Client* createClient(Window win, Monitor* monitor, int id, int x, int y, int width, int height) {
    Client *c;
    if(!(c = (Client *)calloc(1, sizeof(Client)))) {
      fprintf( stderr, "fatal: could not malloc() %lu bytes\n", sizeof(Client));
      exit( -1 );      
    }
    c->win = win;
    c->mon = monitor;
    c->id = id;
    c->x = x;
    c->y = y;
    c->width = width;
    c->height = height;
    return c;
  }

  static Monitor* createMonitor(void) {
    Monitor *m;
    if(!(m = (Monitor *)calloc(1, sizeof(Monitor)))) {
      fprintf( stderr, "fatal: could not malloc() %lu bytes\n", sizeof(Monitor));
      exit( -1 );            
    }
   fprintf( stderr, "Create monitor\n");
    return m;    
  }

  static Client* getByWindow(HelloWorld* hw, Window win) {
    Client *c;
//    Monitor *m;
    for(c = hw->monit->clients; c; c = c->next)
      if(c->win == win)
        return c;
    return NULL;
  }

  static Client* getById(HelloWorld* hw, int id) {
    Client *c;
    for(c = hw->monit->clients; c; c = c->next)
      if(c->id == id)
        return c;
    return NULL;  
  }

  static void updateGeometry(HelloWorld* hw) {
    if(!hw->monit)
      hw->monit = createMonitor();
    if(hw->monit->width != hw->screen_width || hw->monit->height != hw->screen_width){
      hw->monit->width = hw->screen_width;
      hw->monit->height = hw->screen_height;
    }
    return;
  }

  /**
   * Prepare the window object and call the Node.js callback.
   */
  static void EmitAdd(HelloWorld* hw, Window win, XWindowAttributes *wa) {
    TryCatch try_catch;
    // onManage receives a window object
    Local<Value> argv[1];
    // temporarily store window to hw->wnd
    Client* c = createClient(win, hw->monit, hw->next_index, wa->x, wa->y, wa->height, wa->width);
    attach(c);
    argv[0] = HelloWorld::makeWindow(hw->next_index, wa->x, wa->y, wa->height, wa->width, wa->border_width);
    hw->next_index++;

    // call the callback in Node.js, passing the window object...
    hw->Emit(onAdd, 1, argv);
      
    // configure the window
    XConfigureEvent ce;

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

    (void) fprintf( stderr, "manage: x=%d y=%d width=%d height=%d \n", ce.x, ce.y, ce.width, ce.height);

    XSendEvent(hw->dpy, win, False, StructureNotifyMask, (XEvent *)&ce);

  
    // subscribe to window events
    XSelectInput(hw->dpy, win, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
    GrabButtons(hw->dpy, win, False);

    // move and (finally) map the window
    XMoveResizeWindow(hw->dpy, win, ce.x, ce.y, ce.width, ce.height);    
    XMapWindow(hw->dpy, win);

    // emit a rearrange
    EmitRearrange(hw);
  }

  static void EmitRearrange(HelloWorld* hw) {
//    TryCatch try_catch;
      hw->Emit(onRearrange, 0, 0);
//    hw->cbRearrange->Call(Context::GetCurrent()->Global(), 0, 0);
//    if (try_catch.HasCaught()) {
//      FatalException(try_catch);
//    }    
  }


  static Handle<Value> ResizeWindow(const Arguments& args) {
    HandleScope scope;
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    int id = args[0]->IntegerValue();
    int width = args[1]->IntegerValue();
    int height = args[2]->IntegerValue();

    Client* c = getById(hw, id);
    if(c && c->win) {
      fprintf( stderr, "ResizeWindow: id=%d width=%d height=%d \n", id, width, height);    
      XResizeWindow(hw->dpy, c->win, width, height);    
      XFlush(hw->dpy);
    }
    return Undefined();
  } 

  static Handle<Value> MoveWindow(const Arguments& args) {
    HandleScope scope;
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    int id = args[0]->IntegerValue();
    int x = args[1]->IntegerValue();
    int y = args[2]->IntegerValue();

    Client* c = getById(hw, id);
    if(c && c->win) {
      fprintf( stderr, "MoveWindow: id=%d x=%d y=%d \n", id, x, y);    
      XMoveWindow(hw->dpy, c->win, x, y);    
      XFlush(hw->dpy);
    }
    return Undefined();
  }

  static Handle<Value> FocusWindow(const Arguments& args) {
    HandleScope scope;
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    int id = args[0]->IntegerValue();
    RealFocus(hw, id);
    return Undefined();
  }

  static void RealFocus(HelloWorld* hw, int id) {
    Window win;
    Client* c = getById(hw, id);
    if(c && c->win) {
      win = c->win;
    } else {
      win = hw->root;
    }
    fprintf( stderr, "FocusWindow: id=%d\n", id);    
    // do not focus on the same window... it'll cause a flurry of events...
    if(hw->selected && hw->selected != win) {
      GrabButtons(hw->dpy, win, True);
      XSetInputFocus(hw->dpy, win, RevertToPointerRoot, CurrentTime);    
      Atom atom = XInternAtom(hw->dpy, "WM_TAKE_FOCUS", False);
      SendEvent(hw, win, atom);
      XFlush(hw->dpy);      
      hw->selected = win;
    }
  }

  static Bool SendEvent(HelloWorld* hw, Window wnd, Atom proto) {
    int n;
    Atom *protocols;
    Bool exists = False;
    XEvent ev;

    if(XGetWMProtocols(hw->dpy, wnd, &protocols, &n)) {
      while(!exists && n--)
        exists = protocols[n] == proto;
      XFree(protocols);
    }
    if(exists) {
      ev.type = ClientMessage;
      ev.xclient.window = wnd;
      Atom atom = XInternAtom(hw->dpy, "WM_PROTOCOLS", False);
      ev.xclient.message_type = atom;
      ev.xclient.format = 32;
      ev.xclient.data.l[0] = proto;
      ev.xclient.data.l[1] = CurrentTime;
      XSendEvent(hw->dpy, wnd, False, NoEventMask, &ev);
    }
    return exists;    
  }

  /**
   * If focused, then we only grab the modifier keys.
   * Otherwise, we grab all buttons..
   */
  static void GrabButtons(Display* dpy, Window wnd, Bool focused) {
    XUngrabButton(dpy, AnyButton, AnyModifier, wnd);
    if(focused) {
      XGrabButton(dpy, Button3,
                        Mod4Mask,
                        wnd, False, (ButtonPressMask|ButtonReleaseMask),
                        GrabModeAsync, GrabModeSync, None, None);
    } else {
      XGrabButton(dpy, AnyButton, AnyModifier, wnd, False,
                  (ButtonPressMask|ButtonReleaseMask), GrabModeAsync, GrabModeSync, None, None);
    }
  }

  static void GrabKeys(Display* dpy, Window root) {
    XUngrabKey(dpy, AnyKey, AnyModifier, root);
    // windows Z
    XGrabKey(dpy, XK_z, Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);
    // windows X
    XGrabKey(dpy, XK_x, Mod4Mask, root, True, GrabModeAsync, GrabModeAsync);

  }


  static Local<Object> makeWindow(int id, int x, int y, int height, int width, int border_width) {
    // window object to return
    Local<Object> result = Object::New();

    // read and set the window geometry
    result->Set(String::NewSymbol("id"), Integer::New(id));
    result->Set(String::NewSymbol("x"), Integer::New(x));
    result->Set(String::NewSymbol("y"), Integer::New(y));
    result->Set(String::NewSymbol("height"), Integer::New(height));
    result->Set(String::NewSymbol("width"), Integer::New(width));
    result->Set(String::NewSymbol("border_width"), Integer::New(border_width));
    // read and set the monitor (not important)
//    result->Set(String::NewSymbol("monitor"), Integer::New(hw->selected_monitor));
    return result;
  }

  static void EmitButtonPress(HelloWorld* hw, XEvent *e) {
    XButtonPressedEvent *ev = &e->xbutton;
    // onManage receives a window object
    Local<Value> argv[1];

    fprintf(stderr, "EmitButtonPress\n");

    // fetch window: ev->window --> to window id
    // fetch root_x,root_y
    Client* c = getByWindow(hw, ev->window);
    if(c) {
      int id = c->id;
      argv[0] = HelloWorld::makeButtonPress(id, ev->x, ev->y, ev->button);
      fprintf(stderr, "makeButtonPress\n");

      // call the callback in Node.js, passing the window object...
      hw->Emit(onButtonPress, 1, argv);
      fprintf(stderr, "Call cbButtonPress\n");
    }
  }

  static Local<Object> makeButtonPress(int id, int x, int y, unsigned int button) {
    // window object to return
    Local<Object> result = Object::New();

    // read and set the window geometry
    result->Set(String::NewSymbol("id"), Integer::New(id));
    result->Set(String::NewSymbol("x"), Integer::New(x));
    result->Set(String::NewSymbol("y"), Integer::New(y));
    // resolve the button
    switch(button) {
      case Button1:
        result->Set(String::NewSymbol("button"), Integer::New(1));
        break;
      case Button2:
        result->Set(String::NewSymbol("button"), Integer::New(2));
        break;
      case Button3:
        result->Set(String::NewSymbol("button"), Integer::New(3));
        break;
      case Button4:
        result->Set(String::NewSymbol("button"), Integer::New(4));
        break;
      case Button5:
      default:
        result->Set(String::NewSymbol("button"), Integer::New(5));
        break;
    }
    return result;
  }

  static void EmitKeyPress(HelloWorld* hw, XEvent *e) {
//    KeySym keysym;
    XKeyEvent *ev;

    ev = &e->xkey;
//    keysym = XKeycodeToKeysym(hw->dpy, (KeyCode)ev->keycode, 0);

    fprintf(stderr, "EmitKeyPress\n");
    Local<Value> argv[1];
    argv[0] = HelloWorld::makeKeyPress(ev->x, ev->y, ev->keycode, ev->state);
    // call the callback in Node.js, passing the window object...
    hw->Emit(onKeyPress, 1, argv);
  }

  static Local<Object> makeKeyPress(int x, int y, unsigned int keycode, unsigned int mod) {
    // window object to return
    Local<Object> result = Object::New();

    // read and set the window geometry
    result->Set(String::NewSymbol("x"), Integer::New(x));
    result->Set(String::NewSymbol("y"), Integer::New(y));
    result->Set(String::NewSymbol("keycode"), Integer::New(keycode));
    result->Set(String::NewSymbol("mod"), Integer::New(mod));
    return result;
  }

  static void EmitEnterNotify(HelloWorld* hw, XEvent *e) {
    XCrossingEvent *ev = &e->xcrossing;
    // onManage receives a window object
    Local<Value> argv[1];

    fprintf(stderr, "EmitEnterNotify\n");

    Client* c = getByWindow(hw, ev->window);
    if(c) {
      int id = c->id;
      argv[0] = HelloWorld::makeEvent(id);
      // call the callback in Node.js, passing the window object...
      hw->Emit(onEnterNotify, 1, argv);
    }
  }

  static void EmitFocusIn(HelloWorld* hw, XEvent *e) {
    XFocusChangeEvent *ev = &e->xfocus;
    Client* c = getByWindow(hw, ev->window);
    if(c) {
      int id = c->id;
      RealFocus(hw, id);
    }
  }

  static void EmitUnmapNotify(HelloWorld* hw, XEvent *e) {
    Client *c;
    XUnmapEvent *ev = &e->xunmap;
    if((c = getByWindow(hw, ev->window)))
      EmitRemove(hw, c, False);
  }

  static void EmitDestroyNotify(HelloWorld* hw, XEvent *e) {
    Client *c;
    XDestroyWindowEvent *ev = &e->xdestroywindow;

    if((c = getByWindow(hw, ev->window)))
      EmitRemove(hw, c, True);    
  }

  static void EmitRemove(HelloWorld* hw, Client *c, Bool destroyed) {
//    Monitor *m = c->mon;
//    XWindowChanges wc;
    fprintf( stderr, "EmitRemove\n");
    int id = c->id;
    // emit a remove
    Local<Value> argv[1];
    argv[0] = Integer::New(id);
    hw->Emit(onRemove, 1, argv);
    detach(c);
    if(!destroyed) {
      XGrabServer(hw->dpy);
      XUngrabButton(hw->dpy, AnyButton, AnyModifier, c->win);
      XSync(hw->dpy, False);
      XUngrabServer(hw->dpy);
    }
    free(c);
    RealFocus(hw, -1);
    EmitRearrange(hw);
  }

  static Local<Object> makeEvent(int id) {
    // window object to return
    Local<Object> result = Object::New();
    result->Set(String::NewSymbol("id"), Integer::New(id));
    return result;
  }


  static Handle<Value> Setup(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    // initialize resources
    // atoms

    // open the display
    if ( ( hw->dpy = XOpenDisplay(NIL) ) == NULL ) {
      (void) fprintf( stderr, "cannot connect to X server %s\n", XDisplayName(NULL));
      exit( -1 );
    }
    // set error handler
    XSetErrorHandler(xerror);
    XSync(hw->dpy, False);

    // take the default screen
    hw->screen = DefaultScreen(hw->dpy);
    // get the root window
    hw->root = RootWindow(hw->dpy, hw->screen);

    // get screen geometry
    hw->screen_width = DisplayWidth(hw->dpy, hw->screen);
    hw->screen_height = DisplayHeight(hw->dpy, hw->screen);
    // update monitor geometry (and create hw->monitor)
    updateGeometry(hw);

    XSetWindowAttributes wa;
    // subscribe to root window events e.g. SubstructureRedirectMask
    wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask|ButtonPressMask
                    |EnterWindowMask|LeaveWindowMask|StructureNotifyMask
                    |PropertyChangeMask;
    XSelectInput(hw->dpy, hw->root, wa.event_mask);

    GrabKeys(hw->dpy, hw->root);

    Local<Object> result = Object::New();
    result->Set(String::NewSymbol("width"), Integer::New(hw->screen_width));
    result->Set(String::NewSymbol("height"), Integer::New(hw->screen_height));
    return scope.Close(result);
  }



  /**
   * Scan and layout current windows
   */
  static Handle<Value> Scan(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    unsigned int i, num;
    Window d1, d2, *wins = NULL;
    XWindowAttributes wa;
    // XQueryTree() function returns the root ID, the parent window ID, a pointer to 
    // the list of children windows (NULL when there are no children), and 
    // the number of children in the list for the specified window. 
    if(XQueryTree(hw->dpy, hw->root, &d1, &d2, &wins, &num)) {
      for(i = 0; i < num; i++) {
        // if we can't read the window attributes, 
        // or the window is a popup (transient or override_redirect), skip it
        if(!XGetWindowAttributes(hw->dpy, wins[i], &wa)
        || wa.override_redirect || XGetTransientForHint(hw->dpy, wins[i], &d1)) {
          continue;          
        }
        // visible or minimized window ("Iconic state")
        if(wa.map_state == IsViewable )//|| getstate(wins[i]) == IconicState)
          HelloWorld::EmitAdd(hw, wins[i], &wa);
      }
      for(i = 0; i < num; i++) { /* now the transients */
        if(!XGetWindowAttributes(hw->dpy, wins[i], &wa))
          continue;
        if(XGetTransientForHint(hw->dpy, wins[i], &d1)
        && (wa.map_state == IsViewable )) //|| getstate(wins[i]) == IconicState))
          HelloWorld::EmitAdd(hw, wins[i], &wa);
      }
      if(wins) {
        // To free a non-NULL children list when it is no longer needed, use XFree()
        XFree(wins);
      }
    }

    Local<String> result = String::New("Scan done");
    return scope.Close(result);
  }

  static Handle<Value> Loop(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    // use ev_io

    // initiliaze and start 
    ev_io_init(&hw->watcher, EIO_RealLoop, XConnectionNumber(hw->dpy), EV_READ);
    hw->watcher.data = hw;
    ev_io_start(EV_DEFAULT_ &hw->watcher);

//    hw->Ref();

    return Undefined();
  }

  static void EIO_RealLoop(EV_P_ struct ev_io* watcher, int revents) {

    HelloWorld* hw = static_cast<HelloWorld*>(watcher->data);    
    
    XEvent event;
    // main event loop 
    XSync(hw->dpy, False);
    while(XPending(hw->dpy)) {
      XNextEvent(hw->dpy, &event);
      fprintf(stderr, "got event %s (%d).\n", event_names[event.type], event.type);      
      // handle event internally --> calls Node if necessary 
      switch (event.type) {
        case ButtonPress:
          {
            fprintf(stderr, "EmitButtonPress\n");
            HelloWorld::EmitButtonPress(hw, &event);
          }
          break;
        case ConfigureRequest:
            break;
        case ConfigureNotify:
            break;
        case DestroyNotify:
            HelloWorld::EmitDestroyNotify(hw, &event);        
            break;
        case EnterNotify:
            HelloWorld::EmitEnterNotify(hw, &event);
            break;
        case Expose:
            break;
        case FocusIn:
         //   HelloWorld::EmitFocusIn(hw, &event);
            break;
        case KeyPress:
          {
            fprintf(stderr, "EmitKeyPress\n");
            HelloWorld::EmitKeyPress(hw, &event);
          }
            break;
        case MappingNotify:
            break;
        case MapRequest:
          {
            // read the window attrs, then add it to the managed windows...
            XWindowAttributes wa;
            XMapRequestEvent *ev = &event.xmaprequest;
            if(!XGetWindowAttributes(hw->dpy, ev->window, &wa)) {
              fprintf(stderr, "XGetWindowAttributes failed\n");              
              return;
            }
            if(wa.override_redirect)
              return;
            fprintf(stderr, "MapRequest\n");
            Client* c = HelloWorld::getByWindow(hw, ev->window);
            if(c == NULL) {
              fprintf(stderr, "Emit manage!\n");
              // dwm actually does this only once per window (e.g. for unknown windows only...)
              // that's because otherwise you'll cause a hang when you map a mapped window again...
              HelloWorld::EmitAdd(hw, ev->window, &wa);
            } else {
              fprintf(stderr, "Window is known\n");              
            }
          }
            break;
        case PropertyNotify:
            break;
        case UnmapNotify:
            HelloWorld::EmitUnmapNotify(hw, &event);
            break;
        default:
            break;
      }
    }
    return;
  }

  static int xerror(Display *dpy, XErrorEvent *ee) {
    if(ee->error_code == BadWindow
    || (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
    || (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
    || (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
    || (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
    || (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
    || (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
    || (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
    || (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
      return 0;
    fprintf(stderr, "dwm: fatal error: request code=%d, error code=%d\n",
        ee->request_code, ee->error_code);
//    return xerrorxlib(dpy, ee); /* may call exit */    
    exit(-1);
    return 0;
  }

/*
  static Handle<Value> GetWindow(const Arguments& args) {
    HandleScope scope;
    
    // how to return a new object
//    Local<Object> result = Object::New();
//    result->Set(String::NewSymbol("test"), Integer::New(123));
//    return scope.Close(result);

    // how to return a new function
    Local<Object> result = Object::New();
    result->Set(String::NewSymbol("test"), 
      v8::FunctionTemplate::New(WindowObject::Test)->GetFunction());

    return scope.Close(result);
  }
*/
  ev_io watcher;
};

Persistent<FunctionTemplate> HelloWorld::s_ct;

extern "C" {
  // target for export
  static void init (Handle<Object> target)
  {
    HelloWorld::Init(target);
  }

  // macro to export helloworld
  NODE_MODULE(nwm, init);
}


// could also store symbols like this:
//    Persistent<String> port_symbol = NODE_PSYMBOL("port");
