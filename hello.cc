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

static const char *event_names[] = {
"",
"",
"KeyPress",
"KeyRelease",
"ButtonPress",
"ButtonRelease",
"MotionNotify",
"EnterNotify",
"LeaveNotify",
"FocusIn",
"FocusOut",
"KeymapNotify",
"Expose",
"GraphicsExpose",
"NoExpose",
"VisibilityNotify",
"CreateNotify",
"DestroyNotify",
"UnmapNotify",
"MapNotify",
"MapRequest",
"ReparentNotify",
"ConfigureNotify",
"ConfigureRequest",
"GravityNotify",
"ResizeRequest",
"CirculateNotify",
"CirculateRequest",
"PropertyNotify",
"SelectionClear",
"SelectionRequest",
"SelectionNotify",
"ColormapNotify",
"ClientMessage",
"MappingNotify" };

#define WIDTH 800
#define HEIGHT 600

using namespace node;
using namespace v8;

class WindowObject: ObjectWrap {
  private:
  public:
  
  static Persistent<FunctionTemplate> s_ct;
  static void Init(Handle<Object> target) {
    HandleScope scope;
    // create a local FunctionTemplate
    Local<FunctionTemplate> t = FunctionTemplate::New(New);

    // initialize our template
    s_ct = Persistent<FunctionTemplate>::New(t);
    // set the field count
    s_ct->InstanceTemplate()->SetInternalFieldCount(0);
    // set the symbol for this function
    s_ct->SetClassName(String::NewSymbol("WindowObject"));
    
    // FUNCTIONS
    NODE_SET_PROTOTYPE_METHOD(s_ct, "test", Test);

    // FINALLY: export the current function template
    target->Set(String::NewSymbol("HelloWorld"),
                s_ct->GetFunction());

  }

  // C++ constructor
  WindowObject() { }

  ~WindowObject() { }

  // New method for v8
  static Handle<Value> New(const Arguments& args) {
    HandleScope scope;
    WindowObject* obj = new WindowObject();
    // use ObjectWrap.Wrap to store hw in this
    obj->Wrap(args.This());
    // return this
    return args.This();
  }

  static Handle<Value> Test(const Arguments& args) {
    HandleScope scope;
    // create and return a new string
    Local<String> result = String::New("Hello from Test object");
    return scope.Close(result);
  }
};

typedef struct Client Client;
struct Client {
  unsigned int id;
  Monitor *mon;
  Window win;
};

class HelloWorld: ObjectWrap
{
private:
  int m_count;
  Display *dpy;
  Window wnd;
  GC gc;
  int screen, screen_width, screen_height;
  Window root;
  // callbacks
  Persistent<Function> cbManage, cbButtonPress, cbConfigureRequest, cbKeyPress;


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

    // set the static Hello function as prototype.hello
    NODE_SET_PROTOTYPE_METHOD(s_ct, "hello", Hello);

    // callbacks
    NODE_SET_PROTOTYPE_METHOD(s_ct, "onManage", OnManage);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "onButtonPress", OnButtonPress);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "onConfigureRequest", OnConfigureRequest);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "onKeyPress", OnKeyPress);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "allCallbacks", AllCallbacks);

    // pseudocode stuff, dont call, it will break shit
    NODE_SET_PROTOTYPE_METHOD(s_ct, "setup", Setup);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "scan", Scan);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "loop", Loop);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "getWindow", GetWindow);

    // FINALLY: export the current function template
    target->Set(String::NewSymbol("HelloWorld"),
                s_ct->GetFunction());
  }

  // C++ constructor
  HelloWorld() :
    m_count(0)
  {
  }

  ~HelloWorld()
  {
  }

  // New method for v8
  static Handle<Value> New(const Arguments& args)
  {
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

  /**
   * Prepare the window object and call the Node.js callback.
   */
  static void EmitManage(HelloWorld* hw, Window win, XWindowAttributes *wa) {
    TryCatch try_catch;
    // onManage receives a window object
    Local<Value> argv[1];
    argv[0] = HelloWorld::makeWindow(wa->x, wa->y, wa->height, wa->width, wa->border_width);

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

  
    // move and map the window
    XMoveResizeWindow(hw->dpy, win, ce.x, ce.y, ce.width, ce.height);    
    XMapWindow(hw->dpy, win);

    // subscribe to window events
    XSelectInput(hw->dpy, win, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);

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

  static Local<Object> makeWindow(int x, int y, int height, int width, int border_width) {
    // window object to return
    Local<Object> result = Object::New();

    // read and set the window geometry
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
    //Handle<Value> result = 
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


  static Handle<Value> AllCallbacks(const Arguments& args) {
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    Local<Value> argv[1];
    argv[0] = String::New("Hello World");

    TryCatch try_catch;
    // onManage receives a window object
    Local<Value> windowObj[1];
    windowObj[0] = HelloWorld::makeWindow(1, 2, 3, 4, 5);

    Handle<Value> result = hw->cbManage->Call(Context::GetCurrent()->Global(), 1, windowObj);
    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
    
    // cast to object
    Handle<Object> obj = Handle<Object>::Cast(result); 
    // cast to int
    v8::Handle<v8::Value> h = obj->Get(String::NewSymbol("x"));
    int x = h->Int32Value();

    windowObj[0] = HelloWorld::makeWindow(x, 2, 3, 4, 5);

    hw->cbButtonPress->Call(Context::GetCurrent()->Global(), 1, windowObj);
    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
    hw->cbConfigureRequest->Call(Context::GetCurrent()->Global(), 1, argv);
    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
    hw->cbKeyPress->Call(Context::GetCurrent()->Global(), 1, argv);
    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }

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

    TryCatch try_catch;
    Local<Value> argv[1];
    argv[0] = String::New("Hello World");
    hw->cbButtonPress->Call(Context::GetCurrent()->Global(), 1, argv);
    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
    

    XEvent event;
    // main event loop 
    XSync(hw->dpy, False);
    while(!XNextEvent(hw->dpy, &event)) {
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
            break;
        case MappingNotify:
            break;
        case MapRequest:
          {
            // read the window attrs, then add it to the managed windows...
            XWindowAttributes wa;
            XMapRequestEvent *ev = &event.xmaprequest;
            if(!XGetWindowAttributes(hw->dpy, ev->window, &wa))
              return;
            if(wa.override_redirect)
              return;
            // dwm actually does this only once per window (e.g. for unknown windows only...)
            // that's because otherwise you'll cause a hang when you map a mapped window again...
            // HelloWorld::EmitManage(hw, ev->window, &wa); 
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

  static Handle<Value> X11_XDrawLine(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    int x1 = args[0]->IntegerValue();
    int y1 = args[1]->IntegerValue();
    int x2 = args[2]->IntegerValue();
    int y2 = args[3]->IntegerValue();
  
    XDrawLine(hw->dpy, hw->wnd, hw->gc, x1, y1, x2, y2);

    Local<String> result = String::New("XSelectInput");
    return scope.Close(result);
  }

  static Handle<Value> Hello(const Arguments& args)
  {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());
    // use hello world
    hw->m_count++;

    // create and return a new string
    Local<String> result = String::New("Hello World");
    return scope.Close(result);
  }

  static Handle<Value> Sample(const Arguments& args) {
    HandleScope scope;
    Local<String> result = String::New("No this required");
    return scope.Close(result);
  }

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
