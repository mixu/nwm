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



using namespace node;
using namespace v8;

class Test: ObjectWrap {
  private:
    Display *dpy;
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
    s_ct->SetClassName(String::NewSymbol("HelloWorld"));

    // FUNCTIONS
    NODE_SET_PROTOTYPE_METHOD(s_ct, "loop", Loop);

    // FINALLY: export the current function template
    target->Set(String::NewSymbol("HelloWorld"),
                s_ct->GetFunction());
  }

  // C++ constructor
  Test(){ }

  ~Test() { }

  // New method for v8
  static Handle<Value> New(const Arguments& args) {
    HandleScope scope;
    Test* hw = new Test();
    // use ObjectWrap.Wrap to store hw in this
    hw->Wrap(args.This());
    // return this
    return args.This();
  }

  static Handle<Value> Loop(const Arguments& args) {
    HandleScope scope;
    // extract Test from args.this
    Test* hw = ObjectWrap::Unwrap<Test>(args.This());
    (void) fprintf( stderr, "Starting loop\n");

    if ( ( hw->dpy = XOpenDisplay(NIL) ) == NULL ) {
      (void) fprintf( stderr, "cannot connect to X server %s\n", XDisplayName(NULL));
      exit( -1 );
    }

    // use ev_io

    // initiliaze and start 
    ev_io_init(&hw->watcher, EIO_RealLoop, XConnectionNumber(hw->dpy), EV_READ);
    hw->watcher.data = hw;
    ev_io_start(EV_DEFAULT_ &hw->watcher);

    return Undefined();
  }

  static void EIO_RealLoop(EV_P_ struct ev_io* watcher, int revents) {
    (void) fprintf( stderr, "In Loop\n");
//    Test* hw = static_cast<Test*>(watcher->data);    

    return;
  }

  ev_io watcher;
};

Persistent<FunctionTemplate> Test::s_ct;

extern "C" {
  // target for export
  static void init (Handle<Object> target) {
    Test::Init(target);
  }

  // macro to export Test
  NODE_MODULE(test, init);
}

