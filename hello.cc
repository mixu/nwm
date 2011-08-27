/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying 
 * LICENSE file.
 */

#include <v8.h>
#include <node.h>

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

    // test storing a callback to a function
    NODE_SET_PROTOTYPE_METHOD(s_ct, "setManage", SetManage);

    // callbacks
    NODE_SET_PROTOTYPE_METHOD(s_ct, "onManage", OnManage);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "onButtonPress", OnButtonPress);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "onConfigureRequest", OnConfigureRequest);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "onKeyPress", OnKeyPress);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "allCallbacks", AllCallbacks);

    // pseudocode stuff, dont call, it will break shit
    NODE_SET_PROTOTYPE_METHOD(s_ct, "Setup", Setup);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "Scan", Scan);
//    NODE_SET_PROTOTYPE_METHOD(s_ct, "OnManage", OnManage);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "Loop", Loop);
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

  struct hello_baton_t {
    HelloWorld *hw;
    Persistent<Function> cb;
  };

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

  static Handle<Value> AllCallbacks(const Arguments& args) {
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    Local<Value> argv[1];
    argv[0] = String::New("Hello World");

    TryCatch try_catch;
    hw->cbManage->Call(Context::GetCurrent()->Global(), 1, argv);
    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
    hw->cbButtonPress->Call(Context::GetCurrent()->Global(), 1, argv);
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

    return Undefined();
  }




  static Handle<Value> SetManage(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    Local<Function> cb = Local<Function>::Cast(args[0]);
    hello_baton_t *baton = new hello_baton_t();
    baton->hw = hw;
    baton->cb = Persistent<Function>::New(cb);

    hw->Ref();
    eio_custom(EIO_Hello, EIO_PRI_DEFAULT, EIO_AfterHello, baton);
    ev_ref(EV_DEFAULT_UC);

    return Undefined();
  }

  static int EIO_Hello(eio_req *req)
  {
    hello_baton_t *baton = static_cast<hello_baton_t *>(req->data);
    sleep(10);
    return 0;
  }

  static int EIO_AfterHello(eio_req *req)
  {
    HandleScope scope;
    hello_baton_t *baton = static_cast<hello_baton_t *>(req->data);
    ev_unref(EV_DEFAULT_UC);
    baton->hw->Unref();

    Local<Value> argv[1];
    argv[0] = String::New("Hello World");

    TryCatch try_catch;

    baton->cb->Call(Context::GetCurrent()->Global(), 1, argv);

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }

    baton->cb.Dispose();

    delete baton;
    return 0;
  }


  static Handle<Value> Setup(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    // initialize resources

    // open the display
    if ( ( hw->dpy = XOpenDisplay(NIL) ) == NULL ) {
      (void) fprintf( stderr, "winman: cannot connect to X server %s\n", XDisplayName(NULL));
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

    Local<String> result = String::New("Setup done");
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
          HelloWorld::OnManage(wins[i], &wa);
      }
      for(i = 0; i < num; i++) { /* now the transients */
        if(!XGetWindowAttributes(hw->dpy, wins[i], &wa))
          continue;
        if(XGetTransientForHint(hw->dpy, wins[i], &d1)
        && (wa.map_state == IsViewable )) //|| getstate(wins[i]) == IconicState))
          HelloWorld::OnManage(wins[i], &wa);
      }
      if(wins) {
        // To free a non-NULL children list when it is no longer needed, use XFree()
        XFree(wins);
      }
    }

    Local<String> result = String::New("Scan done");
    return scope.Close(result);
  }

  /**
   * Prepare the window object and call the Node.js callback.
   */
  static void OnManage(Window win, XWindowAttributes *wa) {

    // window object to return
    Local<Object> result = Object::New();

    // read and set the window geometry
    result->Set(String::NewSymbol("x"), Integer::New(wa->x));
    result->Set(String::NewSymbol("y"), Integer::New(wa->y));
    result->Set(String::NewSymbol("height"), Integer::New(wa->height));
    result->Set(String::NewSymbol("width"), Integer::New(wa->width));
    result->Set(String::NewSymbol("border_width"), Integer::New(wa->border_width));
    // read and set the monitor (not important)
//    result->Set(String::NewSymbol("monitor"), Integer::New(hw->selected_monitor));

    // call the callback in Node.js, passing the window object...
/*
    // now apply the changes
    XWindowChanges wc;

    // set the location
//    c->x = RETURNED_OBJECT.x;
//    c->y = RETURNED_OBJECT.y;

    // set window border width
//    wc.border_width = RETURNED_OBJECT.border_width;
    XConfigureWindow(hw->dpy, win, CWBorderWidth, &wc);
    // set border color 
    // XSetWindowBorder(dpy, win, dc.norm[ColBorder]);

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
    XSendEvent(hw->dpy, win, False, StructureNotifyMask, (XEvent *)&ce);

//    c->w = RETURNED_OBJECT.width;
//    c->h = RETURNED_OBJECT.height;

    // get the window size hints (e.g. what size it wants to be)

    // (later) get the XGetWMHints to control whether the window should ever be focused...

    // subscribe to window events
    XSelectInput(hw->dpy, win, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);

    // move and map the window
    XMoveResizeWindow(hw->dpy, win, wa->x + 2 * hw->screen_width, wa->y, wa->width, wa->height);    
    XMapWindow(hw->dpy, win);
*/
  }  


  static Handle<Value> Loop(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    XEvent event;
    /* main event loop */
    XSync(hw->dpy, False);
    while(!XNextEvent(hw->dpy, &event)) {
      // handle event internally --> calls Node if necessary 
        switch (event.type) {
        case ButtonPress:
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
            break;
        case PropertyNotify:
            break;
        case UnmapNotify:
            break;
        default:
            fprintf(stderr, "got unexpected %d event.\n", event.type);
            break;
      }
      fprintf(stderr, "got event %d.\n", event.type);
    }
    Local<String> result = String::New("Loop");
    return scope.Close(result);
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
