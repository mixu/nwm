/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying 
 * LICENSE file.
 */

#include <v8.h>
#include <node.h>
#include <libev/ev.h>

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

#define NIL (0)       // A name for the void pointer
#include "event_names.h"


using namespace node;
using namespace v8;

typedef struct Key Key;
struct Key {
  unsigned int mod;
  KeySym keysym;
  Key* next;
};

typedef struct Monitor Monitor;
typedef struct Client Client;
typedef struct NodeWM NodeWM;

#include "client.h"
#include "monitor.h"
#include "handler.h"


// make these classes of their own

enum callback_map { 
  onAdd,  
  onRemove,
  onRearrange,
  onMouseDown,
  onMouseDrag,
  onConfigureRequest,
  onKeyPress,
  onEnterNotify,
  onResize,
  onUpdate,
  onFullscreen,
  onLast
};


class NodeWM: ObjectWrap
{
private:
  // window id
  int next_index;
  // X11
  Display *dpy;
  GC gc;
  Window wnd;
  Window root;
  Window selected;
  Monitor* monit;
  // screen dimensions
  int screen, screen_width, screen_height;
  // callback storage
  Persistent<Function>* callbacks[onLast];
  // grabbed keys
  Key* keys;
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
    NODE_SET_PROTOTYPE_METHOD(s_ct, "killWindow", KillWindow);

    // Setting up
    NODE_SET_PROTOTYPE_METHOD(s_ct, "setup", Setup);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "scan", Scan);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "loop", Loop);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "keys", SetGrabKeys);

    // FINALLY: export the current function template
    target->Set(String::NewSymbol("NodeWM"),
                s_ct->GetFunction());
  }

  // C++ constructor
  NodeWM() :
    next_index(1),
    monit(NULL),
    keys(NULL)
  {
  }

  ~NodeWM()
  {
  }

  // New method for v8
  static Handle<Value> New(const Arguments& args) {
    HandleScope scope;
    NodeWM* hw = new NodeWM();
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
    // extract from args.this
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    v8::Local<v8::String> map[onLast+1] = {
        v8::String::New("add"),
        v8::String::New("remove"),
        v8::String::New("rearrange"),
        v8::String::New("mouseDown"),
        v8::String::New("mouseDrag"),
        v8::String::New("configureRequest"),
        v8::String::New("keyPress"),
        v8::String::New("enterNotify"),
        v8::String::New("resize"),
        v8::String::New("update"),
        v8::String::New("fullscreen")
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

    return Undefined();
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

  static Monitor* createMonitor(void) {
    Monitor *m;
    if(!(m = (Monitor *)calloc(1, sizeof(Monitor)))) {
      fprintf( stderr, "fatal: could not malloc() %lu bytes\n", sizeof(Monitor));
      exit( -1 );            
    }
   fprintf( stderr, "Create monitor\n");
    return m;    
  }

  static void updateGeometry(NodeWM* hw) {
    if(!hw->monit) {
      hw->monit = new Monitor();
    }
    hw->monit->setWidth(hw->screen_width);
    hw->monit->setHeight(hw->screen_height);
    return;
  }

  /**
   * Prepare the window object and call the Node.js callback.
   */
  static void EmitAdd(NodeWM* hw, Window win, XWindowAttributes *wa) {
    TryCatch try_catch;
    // onManage receives a window object
    Local<Value> argv[1];
    Client* c = new Client(win, hw->monit, hw->next_index, wa->x, wa->y, wa->height, wa->width);
    c->updatetitle(hw->dpy);
    c->updateclass(hw->dpy);
    c->attach();
    argv[0] = c->toNode(); 
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
    hw->Emit(onRearrange, 0, 0);
  }

  static Handle<Value> ResizeWindow(const Arguments& args) {
    HandleScope scope;
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    int id = args[0]->IntegerValue();
    int width = args[1]->IntegerValue();
    int height = args[2]->IntegerValue();

    Client* c = Client::getById(hw->monit, id);
    if(c) {
      c->resize(hw->dpy, width, height);
    }
    return Undefined();
  } 

  static Handle<Value> MoveWindow(const Arguments& args) {
    HandleScope scope;
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    int id = args[0]->IntegerValue();
    int x = args[1]->IntegerValue();
    int y = args[2]->IntegerValue();

    Client* c = Client::getById(hw->monit, id);
    if(c) {
      c->move(hw->dpy, x, y);
    }
    return Undefined();
  }

  static Handle<Value> FocusWindow(const Arguments& args) {
    HandleScope scope;
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    int id = args[0]->IntegerValue();
    RealFocus(hw, id);
    return Undefined();
  }

  static Handle<Value> KillWindow(const Arguments& args) {
    HandleScope scope;
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    int id = args[0]->IntegerValue();

    Client* c = Client::getById(hw->monit, id);
    if(c) {
      c->kill(hw->dpy);
    }
    return Undefined();
  }

  static void RealFocus(NodeWM* hw, int id) {
    Window win;
    Client* c = Client::getById(hw->monit, id);
    if(c) {
      win = c->getWin();
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

  static Bool SendEvent(NodeWM* hw, Window wnd, Atom proto) {
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
      fprintf( stderr, "GRABBUTTONS - focused: true\n");      
      XGrabButton(dpy, Button1,
                        Mod4Mask|ControlMask,
                        wnd, False, (ButtonPressMask|ButtonReleaseMask),
                        GrabModeAsync, GrabModeSync, None, None);
    } else {
      fprintf( stderr, "GRABBUTTONS - focused: false\n");      
      XGrabButton(dpy, AnyButton, AnyModifier, wnd, False,
                  (ButtonPressMask|ButtonReleaseMask), GrabModeAsync, GrabModeSync, None, None);
    }
  }

  /**
   * Set the keys to grab
   */
  static Handle<Value> SetGrabKeys(const Arguments& args) {
    HandleScope scope;
    unsigned int i;
    v8::Handle<v8::Value> keysym, modifier;
    // extract from args.this
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());
    v8::Local<v8::Array> arr = Local<v8::Array>::Cast(args[0]);

    // empty keys
    Key* curr = hw->keys;
    Key* next;
    while(curr != NULL) {
      next = curr->next;
      free(curr);
      curr = next;
    }
    // set keys
    for(i = 0; i < arr->Length(); i++) {
      v8::Local<v8::Object> obj = Local<v8::Object>::Cast(arr->Get(i));
      keysym = obj->Get(String::NewSymbol("key"));
      modifier = obj->Get(String::NewSymbol("modifier"));
      pushKey(&hw->keys, keysym->IntegerValue(), modifier->IntegerValue());
    }
    return Undefined();
  }

  static void pushKey(Key** keys, KeySym keysym, unsigned int mod) {
    Key* curr;
    if(!(curr = (Key*)calloc(1, sizeof(Key)))) {
      fprintf( stderr, "fatal: could not malloc() %lu bytes\n", sizeof(Key));
      exit( -1 );         
    }
    curr->keysym = keysym;
    curr->mod = mod;
    curr->next = *keys;
    *keys = curr;
//    fprintf( stderr, "key: %d modifier %d \n", curr->keysym, curr->mod);
  }

  static void GrabKeys(NodeWM* hw, Display* dpy, Window root) {
    XUngrabKey(hw->dpy, AnyKey, AnyModifier, hw->root);
    for(Key* curr = hw->keys; curr != NULL; curr = curr->next) {
      XGrabKey(hw->dpy, XKeysymToKeycode(hw->dpy, curr->keysym), curr->mod, hw->root, True, GrabModeAsync, GrabModeAsync);
    }
  }

  static void EmitButtonPress(NodeWM* hw, XEvent *e) {
    XButtonPressedEvent *ev = &e->xbutton;
    Local<Value> argv[1];

    fprintf(stderr, "EmitButtonPress\n");

    // fetch window: ev->window --> to window id
    // fetch root_x,root_y
    Client* c = Client::getByWindow(hw->monit, ev->window);
    if(c) {
      int id = c->getId();
      argv[0] = NodeWM::makeButtonPress(id, ev->x, ev->y, ev->button, ev->state);
      fprintf(stderr, "makeButtonPress\n");
      // call the callback in Node.js, passing the window object...
      hw->Emit(onMouseDown, 1, argv);
      fprintf(stderr, "Call cbButtonPress\n");
      // now emit the drag events
      GrabMouseRelease(hw, id);
    }
  }

  Bool getrootptr(int *x, int *y) {
    int di;
    unsigned int dui;
    Window dummy;

    return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
  }

  static void GrabMouseRelease(NodeWM* hw, int id) {
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
    return;
  }


  static Local<Object> makeMouseDrag(int id, int x, int y, int movex, int movey //, unsigned int state
  ) {
    // window object to return
    Local<Object> result = Object::New();

    // read and set the window geometry
    result->Set(String::NewSymbol("id"), Integer::New(id));
    result->Set(String::NewSymbol("x"), Integer::New(x));
    result->Set(String::NewSymbol("y"), Integer::New(y));
    result->Set(String::NewSymbol("move_x"), Integer::New(movex));
    result->Set(String::NewSymbol("move_y"), Integer::New(movey));
//    result->Set(String::NewSymbol("state"), Integer::New(state));
    return result;
  }


  static Local<Object> makeButtonPress(int id, int x, int y, unsigned int button, unsigned int state) {
    // window object to return
    Local<Object> result = Object::New();

    // read and set the window geometry
    result->Set(String::NewSymbol("id"), Integer::New(id));
    result->Set(String::NewSymbol("x"), Integer::New(x));
    result->Set(String::NewSymbol("y"), Integer::New(y));
    result->Set(String::NewSymbol("button"), Integer::New(button));
    result->Set(String::NewSymbol("state"), Integer::New(state));
    return result;
  }

  static void EmitKeyPress(NodeWM* hw, XEvent *e) {
    KeySym keysym;
    XKeyEvent *ev;

    ev = &e->xkey;
    keysym = XKeycodeToKeysym(hw->dpy, (KeyCode)ev->keycode, 0);
    Local<Value> argv[1];
    argv[0] = NodeWM::makeKeyPress(ev->x, ev->y, ev->keycode, keysym, ev->state);
    // call the callback in Node.js, passing the window object...
    hw->Emit(onKeyPress, 1, argv);
  }

  static Local<Object> makeKeyPress(int x, int y, unsigned int keycode, KeySym keysym, unsigned int mod) {
    // window object to return
    Local<Object> result = Object::New();
    // read and set the window geometry
    result->Set(String::NewSymbol("x"), Integer::New(x));
    result->Set(String::NewSymbol("y"), Integer::New(y));
    result->Set(String::NewSymbol("keysym"), Integer::New(keysym));
    result->Set(String::NewSymbol("keycode"), Integer::New(keycode));
    result->Set(String::NewSymbol("modifier"), Integer::New(mod));
    return result;
  }

  static void EmitEnterNotify(NodeWM* hw, XEvent *e) {
    XCrossingEvent *ev = &e->xcrossing;
    // onManage receives a window object
    Local<Value> argv[1];

    fprintf(stderr, "EmitEnterNotify\n");

    Client* c = Client::getByWindow(hw->monit, ev->window);
    if(c) {
      int id = c->getId();
      argv[0] = NodeWM::makeEvent(id);
      // call the callback in Node.js, passing the window object...
      hw->Emit(onEnterNotify, 1, argv);
    }
  }

  static void EmitFocusIn(NodeWM* hw, XEvent *e) {
    XFocusChangeEvent *ev = &e->xfocus;
    Client* c = Client::getByWindow(hw->monit, ev->window);
    if(c) {
      int id = c->getId();
      RealFocus(hw, id);
    }
  }

  static void EmitUnmapNotify(NodeWM* hw, XEvent *e) {
    Client *c;
    XUnmapEvent *ev = &e->xunmap;
    if((c = Client::getByWindow(hw->monit, ev->window)))
      EmitRemove(hw, c, False);
  }

  static void EmitDestroyNotify(NodeWM* hw, XEvent *e) {
    Client *c;
    XDestroyWindowEvent *ev = &e->xdestroywindow;

    if((c = Client::getByWindow(hw->monit, ev->window)))
      EmitRemove(hw, c, True);    
  }

  static void EmitConfigureNotify(NodeWM* hw, XEvent *e) {
    XConfigureEvent *ev = &e->xconfigure;
    Local<Value> argv[1];

    if(ev->window == hw->root) {
      hw->screen_width = ev->width;
      hw->screen_height = ev->height;
      // update monitor structures
      updateGeometry(hw);
      // make screen object
      Local<Object> result = Object::New();
      result->Set(String::NewSymbol("width"), Integer::New(hw->screen_width));
      result->Set(String::NewSymbol("height"), Integer::New(hw->screen_height));
      argv[0] = result;
      hw->Emit(onResize, 1, argv);
      hw->Emit(onRearrange, 0, 0);
    }
  }

  static void EmitPropertyNotify(NodeWM* hw, XEvent *e) {
    XPropertyEvent *ev = &e->xproperty;
    // could be used for tracking hints, transient status and window name
    if((ev->window == hw->root) && (ev->atom == XA_WM_NAME)) {
      // the root window name has changed      
    } else if(ev->state == PropertyDelete) {
      return; // ignore property deletes
    } else {
      Local<Value> argv[1];
      Client* c = Client::getByWindow(hw->monit, ev->window);
      if(c) {
        Atom NetWMName = XInternAtom(hw->dpy, "_NET_WM_NAME", False);
        if(ev->atom == XA_WM_NAME || ev->atom == NetWMName) {
          c->updatetitle(hw->dpy);
          c->updateclass(hw->dpy);
          argv[0] = c->toNode();
          // call the callback in Node.js, passing the window object...
          hw->Emit(onUpdate, 1, argv);              
        }
      }
    }
  }

  static void EmitClientMessage(NodeWM* hw, XEvent *e) {
    XClientMessageEvent *cme = &e->xclient;
    Client *c = Client::getByWindow(hw->monit, cme->window);
    Atom NetWMState = XInternAtom(hw->dpy, "_NET_WM_STATE", False);
    Atom NetWMFullscreen = XInternAtom(hw->dpy, "_NET_WM_STATE_FULLSCREEN", False);
    Local<Value> argv[2];

    if((c)
    && (cme->message_type == NetWMState && cme->data.l[1] == NetWMFullscreen)) {
      if(cme->data.l[0]) {
        XChangeProperty(hw->dpy, cme->window, NetWMState, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)&NetWMFullscreen, 1);
        argv[0] = Integer::New(c->getId());
        argv[1] = Integer::New(1);
        hw->Emit(onFullscreen, 2, argv);
        // we don't expose XRaiseWindow yet
//        XRaiseWindow(hw->dpy, c->win);
      }
      else {
        XChangeProperty(hw->dpy, cme->window, NetWMState, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)0, 0);
        argv[0] = Integer::New(c->getId());
        argv[1] = Integer::New(0);
        hw->Emit(onFullscreen, 2, argv);
      }
    }    
  }

  static void EmitRemove(NodeWM* hw, Client *c, Bool destroyed) {
    fprintf( stderr, "EmitRemove\n");
    int id = c->getId();
    // emit a remove
    Local<Value> argv[1];
    argv[0] = Integer::New(id);
    hw->Emit(onRemove, 1, argv);
    c->detach();
    if(!destroyed) {
      XGrabServer(hw->dpy);
      XUngrabButton(hw->dpy, AnyButton, AnyModifier, c->getWin());
      XSync(hw->dpy, False);
      XUngrabServer(hw->dpy);
    }
    delete c;
    RealFocus(hw, -1);
    hw->Emit(onRearrange, 0, 0);
  }

  static Local<Object> makeEvent(int id) {
    // window object to return
    Local<Object> result = Object::New();
    result->Set(String::NewSymbol("id"), Integer::New(id));
    return result;
  }


  static Handle<Value> Setup(const Arguments& args) {
    HandleScope scope;
    // extract from args.this
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    // initialize resources
    // atoms

    // open the display
    if ( ( hw->dpy = XOpenDisplay(NIL) ) == NULL ) {
      fprintf( stderr, "cannot connect to X server %s\n", XDisplayName(NULL));
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

    GrabKeys(hw, hw->dpy, hw->root);

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
    // extract from args.this
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

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
          NodeWM::EmitAdd(hw, wins[i], &wa);
      }
      for(i = 0; i < num; i++) { /* now the transients */
        if(!XGetWindowAttributes(hw->dpy, wins[i], &wa))
          continue;
        if(XGetTransientForHint(hw->dpy, wins[i], &d1)
        && (wa.map_state == IsViewable )) //|| getstate(wins[i]) == IconicState))
          NodeWM::EmitAdd(hw, wins[i], &wa);
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
    // extract from args.this
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    // use ev_io

    // initiliaze and start 
    XSync(hw->dpy, False);
    ev_io_init(&hw->watcher, EIO_RealLoop, XConnectionNumber(hw->dpy), EV_READ);
    hw->watcher.data = hw;
    ev_io_start(EV_DEFAULT_ &hw->watcher);

//    hw->Ref();

    return Undefined();
  }

  static void EIO_RealLoop(EV_P_ struct ev_io* watcher, int revents) {

    NodeWM* hw = static_cast<NodeWM*>(watcher->data);    
    
    XEvent event;
    // main event loop 
    while(XPending(hw->dpy)) {
      XNextEvent(hw->dpy, &event);
      fprintf(stderr, "got event %s (%d).\n", event_names[event.type], event.type);      
      // handle event internally --> calls Node if necessary 
      switch (event.type) {
        case ButtonPress:
          {
            NodeWM::EmitButtonPress(hw, &event);
          }
          break;
        case ClientMessage:
          NodeWM::EmitClientMessage(hw, &event);
          break;
        case ConfigureRequest:
          {
            XConfigureRequestEvent *ev = &event.xconfigurerequest;
            XWindowChanges wc;
            wc.x = ev->x;
            wc.y = ev->y;
            wc.width = ev->width;
            wc.height = ev->height;
            wc.border_width = ev->border_width;
            wc.sibling = ev->above;
            wc.stack_mode = ev->detail;
            XConfigureWindow(hw->dpy, ev->window, ev->value_mask, &wc);            
          }
          break;
        case ConfigureNotify:
            NodeWM::EmitConfigureNotify(hw, &event);
            break;
        case DestroyNotify:
            NodeWM::EmitDestroyNotify(hw, &event);        
            break;
        case EnterNotify:
            NodeWM::EmitEnterNotify(hw, &event);
            break;
//        case Expose:
//            break;
//        case FocusIn:
         //   NodeWM::EmitFocusIn(hw, &event);
//            break;
        case KeyPress:
          {
            fprintf(stderr, "EmitKeyPress\n");
            NodeWM::EmitKeyPress(hw, &event);
          }
            break;
//        case MappingNotify:
//            break;
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
            Client* c = Client::getByWindow(hw->monit, ev->window);
            if(c == NULL) {
              fprintf(stderr, "Emit manage!\n");
              // dwm actually does this only once per window (e.g. for unknown windows only...)
              // that's because otherwise you'll cause a hang when you map a mapped window again...
              NodeWM::EmitAdd(hw, ev->window, &wa);
            } else {
              fprintf(stderr, "Window is known\n");              
            }
          }
            break;
        case PropertyNotify:
            NodeWM::EmitPropertyNotify(hw, &event);
            break;
        case UnmapNotify:
            NodeWM::EmitUnmapNotify(hw, &event);
            break;
        default:
          fprintf(stderr, "got event %s (%d).\n", event_names[event.type], event.type);      
          fprintf(stderr, "Did nothing\n");
            break;
      }
    }
    return;
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

#include "client.cc"

Persistent<FunctionTemplate> NodeWM::s_ct;

extern "C" {
  // target for export
  static void init (Handle<Object> target)
  {
    NodeWM::Init(target);
  }

  // macro to export
  NODE_MODULE(nwm, init);
}
