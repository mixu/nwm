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
    // set the static Hello function as prototype.hello
    NODE_SET_PROTOTYPE_METHOD(s_ct, "XOpenDisplay", X11_XOpenDisplay);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "XCreateSimpleWindow", X11_XCreateSimpleWindow);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "XSelectInput", X11_XSelectInput);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "XMapWindow", X11_XMapWindow);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "XSetForeground", X11_XSetForeground);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "XDrawLine", X11_XDrawLine);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "XFlush", X11_XFlush);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "XMapWindow", X11_XMapWindow);
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

  static Handle<Value> X11_XOpenDisplay(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    hw->dpy = XOpenDisplay(NIL);

    Local<String> result = String::New("XOpenDisplay");
    return scope.Close(result);
  }

  static Handle<Value> X11_XCreateSimpleWindow(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    int blackColor = BlackPixel(hw->dpy, DefaultScreen(hw->dpy));
    hw->wnd = XCreateSimpleWindow(hw->dpy,
                          DefaultRootWindow(hw->dpy), 0, 0, 
                 WIDTH, HEIGHT, 0, 
                 blackColor, blackColor);

    Local<String> result = String::New("XCreateSimpleWindow");
    return scope.Close(result);
  }

  static Handle<Value> X11_XSelectInput(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    // We want to get MapNotify events
    XSelectInput(hw->dpy, hw->wnd, StructureNotifyMask);

    Local<String> result = String::New("XSelectInput");
    return scope.Close(result);
  }

  static Handle<Value> X11_XMapWindow(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    XMapWindow(hw->dpy, hw->wnd);


    Local<String> result = String::New("XSelectInput");
    return scope.Close(result);
  }

  static Handle<Value> X11_XSetForeground(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    hw->gc = XCreateGC(hw->dpy, hw->wnd, 0, NIL);
    int whiteColor = WhitePixel(hw->dpy, DefaultScreen(hw->dpy));
    XSetForeground(hw->dpy, hw->gc, whiteColor);
        // Wait for the MapNotify event
        for(;;) {
         XEvent e;
         XNextEvent(hw->dpy, &e);
         if (e.type == MapNotify)
          break;
        }

    Local<String> result = String::New("XSelectInput");
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

     
  static Handle<Value> X11_XFlush(const Arguments& args) {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());

    XFlush(hw->dpy);

    Local<String> result = String::New("XFlush");
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
      v8::FunctionTemplate::New(HelloWorld::Sample)->GetFunction());

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
