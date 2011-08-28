/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying 
 * LICENSE file.
 */

#include <v8.h>
#include <node.h>
#include <ev.h>

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

class HelloWorld: ObjectWrap
{
private:
  Display *dpy;
  Window wnd;
  GC gc;
  int screen, screen_width, screen_height;
  Window root;
  // callbacks
  Persistent<Function> cbManage, cbButtonPress, cbConfigureRequest, cbKeyPress;
  Window windows[MAXWIN];
  int next_index;

public:

  static Persistent<FunctionTemplate> s_ct;
  static void Init(Handle<Object> target)
  {
    HandleScope scope;
    // create a local FunctionTemplate
    Local<FunctionTemplate> t = FunctionTemplate::New(New);

    // initialize our template
    s_ct = Persistent<FunctionTemplate>::New(t);
    // set the field count
    s_ct->InstanceTemplate()->SetInternalFieldCount(1);
    // set the symbol for this function
    s_ct->SetClassName(String::NewSymbol("HelloWorld"));

    // FUNCTIONS

    // callbacks
    NODE_SET_PROTOTYPE_METHOD(s_ct, "onAdd", OnManage);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "onButtonPress", OnButtonPress);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "onConfigureRequest", OnConfigureRequest);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "onKeyPress", OnKeyPress);
    // API
    NODE_SET_PROTOTYPE_METHOD(s_ct, "moveWindow", MoveWindow);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "resizeWindow", ResizeWindow);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "focusWindow", FocusWindow);

    // Setting up
    NODE_SET_PROTOTYPE_METHOD(s_ct, "setup", Setup);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "scan", Scan);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "loop", Loop);

    // FINALLY: export the current function template
    target->Set(String::NewSymbol("HelloWorld"),
                s_ct->GetFunction());
  }

  // C++ constructor
  HelloWorld() :
    next_index(0)
  {
  }

  ~HelloWorld()
  {
  }

  // New method for v8
  static Handle<Value> New(const Arguments& args) {
    HandleScope scope;
    HelloWorld* hw = new HelloWorld();
    // use ObjectWrap.Wrap to store hw in this
    hw->Wrap(args.This());
    // return this
    return args.This();
  }

  static Handle<Value> OnManage(const Arguments& args) {
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    // store function
    Local<Function> cb = Local<Function>::Cast(args[0]);
    hw->cbManage = Persistent<Function>::New(cb);

    return Undefined();
  }

  static Handle<Value> OnButtonPress(const Arguments& args) {
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    // store function
    Local<Function> cb = Local<Function>::Cast(args[0]);
    hw->cbButtonPress = Persistent<Function>::New(cb);

    return Undefined();
  }

  static Handle<Value> OnConfigureRequest(const Arguments& args) {
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    // store function
    Local<Function> cb = Local<Function>::Cast(args[0]);
    hw->cbConfigureRequest = Persistent<Function>::New(cb);

    return Undefined();
  }

  static Handle<Value> OnKeyPress(const Arguments& args) {
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    // store function
    Local<Function> cb = Local<Function>::Cast(args[0]);
    hw->cbKeyPress = Persistent<Function>::New(cb);

    return Undefined();
  }

  static int getIntegerValue(Handle<Object> obj, Local<String> symbol) {
    v8::Handle<v8::Value> h = obj->Get(symbol);    
    int val = h->IntegerValue();
    (void) fprintf( stderr, "getIntegerValue: %d \n", val);
    return val;
  }

  static int getWindowId(HelloWorld* hw, Window win) {
    int i;
    for(i = 0; i < hw->next_index; i++) {
      if(hw->windows[i] == win) {
        return i;
      }
    }
    return -1;
  }


  /**
   * Prepare the window object and call the Node.js callback.
   */
  static void EmitManage(HelloWorld* hw, Window win, XWindowAttributes *wa) {
    TryCatch try_catch;
    // onManage receives a window object
    Local<Value> argv[1];
    // temporarily store window to hw->wnd
    hw->windows[hw->next_index] = win;  
    argv[0] = HelloWorld::makeWindow(hw->next_index, wa->x, wa->y, wa->height, wa->width, wa->border_width);
    hw->next_index++;

    // call the callback in Node.js, passing the window object...
    Handle<Value> result = hw->cbManage->Call(Context::GetCurrent()->Global(), 1, argv);
    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
      
    // get the return value
    // cast to object
    Handle<Object> obj = Handle<Object>::Cast(result); 
    // now apply the changes
    // configure the window
    XConfigureEvent ce;

    ce.type = ConfigureNotify;
    ce.display = hw->dpy;
    ce.event = win;
    ce.window = win;
    ce.x = getIntegerValue(obj, String::NewSymbol("x"));
    ce.y = getIntegerValue(obj, String::NewSymbol("y"));
    ce.width = getIntegerValue(obj, String::NewSymbol("width"));
    ce.height = getIntegerValue(obj, String::NewSymbol("height"));
    ce.border_width = getIntegerValue(obj, String::NewSymbol("border_width"));
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



/*
    XWindowChanges wc;
    // set window border width
    XConfigureWindow(hw->dpy, win, CWBorderWidth, &wc);
    // set border color 
    // XSetWindowBorder(dpy, win, dc.norm[ColBorder]);

    // get the window size hints (e.g. what size it wants to be)

    // (later) get the XGetWMHints to control whether the window should ever be focused...

*/

  }

  static Handle<Value> ResizeWindow(const Arguments& args) {
    HandleScope scope;
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    int id = args[0]->IntegerValue();
    int width = args[1]->IntegerValue();
    int height = args[2]->IntegerValue();

    Window win = hw->windows[id];
    fprintf( stderr, "ResizeWindow: id=%d width=%d height=%d \n", id, width, height);    
    XResizeWindow(hw->dpy, win, width, height);    
    XFlush(hw->dpy);
    return Undefined();
  } 

  static Handle<Value> MoveWindow(const Arguments& args) {
    HandleScope scope;
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    int id = args[0]->IntegerValue();
    int x = args[1]->IntegerValue();
    int y = args[2]->IntegerValue();

    Window win = hw->windows[id];
    fprintf( stderr, "MoveWindow: id=%d x=%d y=%d \n", id, x, y);    
    XMoveWindow(hw->dpy, win, x, y);    
    XFlush(hw->dpy);
    return Undefined();
  }

  static Handle<Value> FocusWindow(const Arguments& args) {
    HandleScope scope;
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    int id = args[0]->IntegerValue();

    Window win = hw->windows[id];
    fprintf( stderr, "FocusWindow: id=%d\n", id);    
    XSetInputFocus(hw->dpy, win, RevertToPointerRoot, CurrentTime);    
    XFlush(hw->dpy);
    return Undefined();
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
    TryCatch try_catch;
    // onManage receives a window object
    Local<Value> argv[1];

    fprintf(stderr, "EmitButtonPress\n");

    // fetch window: ev->window --> to window id
    // fetch root_x,root_y
    argv[0] = HelloWorld::makeButtonPress(ev->x, ev->y, ev->button);
    fprintf(stderr, "makeButtonPress\n");

    // call the callback in Node.js, passing the window object...
    hw->cbButtonPress->Call(Context::GetCurrent()->Global(), 1, argv);
    fprintf(stderr, "Call cbButtonPress\n");
    if (try_catch.HasCaught()) {
      fprintf(stderr, "try catch cbButtonPress\n");
      FatalException(try_catch);
    }
  }

  static Local<Object> makeButtonPress(int x, int y, unsigned int button) {
    // window object to return
    Local<Object> result = Object::New();

    // read and set the window geometry
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
    KeySym keysym;
    XKeyEvent *ev;

    ev = &e->xkey;
//    keysym = XKeycodeToKeysym(hw->dpy, (KeyCode)ev->keycode, 0);

    fprintf(stderr, "EmitKeyPress\n");

    TryCatch try_catch;
    Local<Value> argv[1];
    argv[0] = HelloWorld::makeKeyPress(ev->x, ev->y, ev->keycode, ev->state);

    // call the callback in Node.js, passing the window object...
    hw->cbKeyPress->Call(Context::GetCurrent()->Global(), 1, argv);
    if (try_catch.HasCaught()) {
      fprintf(stderr, "try catch EmitKeyPress\n");
      FatalException(try_catch);
    }
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

  static Handle<Value> Setup(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    // initialize resources

    // open the display
    if ( ( hw->dpy = XOpenDisplay(NIL) ) == NULL ) {
      (void) fprintf( stderr, "cannot connect to X server %s\n", XDisplayName(NULL));
      exit( -1 );
    }

    // take the default screen
    hw->screen = DefaultScreen(hw->dpy);
    // get the root window
    hw->root = RootWindow(hw->dpy, hw->screen);

    // get screen geometry
    hw->screen_width = DisplayWidth(hw->dpy, hw->screen);
    hw->screen_height = DisplayHeight(hw->dpy, hw->screen);

    XSetWindowAttributes wa;
    // subscribe to root window events e.g. SubstructureRedirectMask
    wa.event_mask = SubstructureRedirectMask|SubstructureNotifyMask|ButtonPressMask
                    |EnterWindowMask|LeaveWindowMask|StructureNotifyMask
                    |PropertyChangeMask;
    XSelectInput(hw->dpy, hw->root, wa.event_mask);

    // this is where you would grab the key inputs

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
          HelloWorld::EmitManage(hw, wins[i], &wa);
      }
      for(i = 0; i < num; i++) { /* now the transients */
        if(!XGetWindowAttributes(hw->dpy, wins[i], &wa))
          continue;
        if(XGetTransientForHint(hw->dpy, wins[i], &d1)
        && (wa.map_state == IsViewable )) //|| getstate(wins[i]) == IconicState))
          HelloWorld::EmitManage(hw, wins[i], &wa);
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
    (void) fprintf( stderr, "In Loop\n");

    HelloWorld* hw = static_cast<HelloWorld*>(watcher->data);    
    
    XEvent event;
    // main event loop 
    XSync(hw->dpy, False);
    while(XPending(hw->dpy)) {
      XNextEvent(hw->dpy, &event);
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
            break;
        case EnterNotify:
            break;
        case Expose:
            break;
        case FocusIn:
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
            if(HelloWorld::getWindowId(hw, ev->window) == -1) {
              fprintf(stderr, "Emit manage!\n");
              // dwm actually does this only once per window (e.g. for unknown windows only...)
              // that's because otherwise you'll cause a hang when you map a mapped window again...
              HelloWorld::EmitManage(hw, ev->window, &wa);
            }
          }
            break;
        case PropertyNotify:
            break;
        case UnmapNotify:
            break;
        default:
            fprintf(stderr, "got unexpected %s (%d) event.\n", event_names[event.type], event.type);
            break;
      }
      fprintf(stderr, "got event %s (%d).\n", event_names[event.type], event.type);
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

Persistent<FunctionTemplate> HelloWorld::s_ct;

extern "C" {
  // target for export
  static void init (Handle<Object> target)
  {
    HelloWorld::Init(target);
  }

  // macro to export helloworld
  NODE_MODULE(helloworld, init);
}


// could also store symbols like this:
//    Persistent<String> port_symbol = NODE_PSYMBOL("port");
