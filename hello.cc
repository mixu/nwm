/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying 
 * LICENSE file.
 */

#include <v8.h>
#include <node.h>

#include <X11/Xlib.h>
#include <assert.h>   // I include this to test return values the lazy way
#include <unistd.h>   // So we got the profile for 10 seconds
#define NIL (0)       // A name for the void pointer

#define WIDTH 800
#define HEIGHT 600

using namespace node;
using namespace v8;

class HelloWorld: ObjectWrap
{
private:
  int m_count;
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
    // set the static Hello function as prototype.hello
    NODE_SET_PROTOTYPE_METHOD(s_ct, "hello", Hello);
    // export the current function template
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

  static Handle<Value> Hello(const Arguments& args)
  {
    HandleScope scope;
    // extract helloworld from args.this
    HelloWorld* hw = ObjectWrap::Unwrap<HelloWorld>(args.This());
    // use hello world
    hw->m_count++;


     Display *dpy;
     Window wnd;

     dpy = XOpenDisplay(NIL);

     int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
     int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

     wnd = XCreateSimpleWindow(dpy,
                          DefaultRootWindow(dpy), 0, 0, 
                 WIDTH, HEIGHT, 0, 
                 blackColor, blackColor);
     // We want to get MapNotify events

     XSelectInput(dpy, wnd, StructureNotifyMask);
     XMapWindow(dpy, wnd);
     
     GC gc = XCreateGC(dpy, wnd, 0, NIL);
     XSetForeground(dpy, gc, whiteColor);

     
        // Wait for the MapNotify event

        for(;;) {
         XEvent e;
         XNextEvent(dpy, &e);
         if (e.type == MapNotify)
          break;
        }
        XDrawLine(dpy, wnd, gc, 10, 60, 180, 20);
     
     
     XFlush(dpy);
        sleep(10);


    // create and return a new string
    Local<String> result = String::New("Hello World");
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
