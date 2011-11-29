#include <v8.h>
#include <node.h>
#include <ev.h>
#include <string.h>
#include "nwm.h"

using namespace node;
using namespace v8;

class NodeWM: ObjectWrap {
public:
  ev_io watcher;
  // callback storage
  Persistent<Function>* callbacks[onLast];

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

    // Callbacks
    NODE_SET_PROTOTYPE_METHOD(s_ct, "on", OnCallback);
    // API
    NODE_SET_PROTOTYPE_METHOD(s_ct, "moveWindow", MoveWindow);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "resizeWindow", ResizeWindow);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "focusWindow", FocusWindow);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "killWindow", KillWindow);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "configureWindow", ConfigureWindow);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "notifyWindow", NotifyWindow);
    // Setting up
    NODE_SET_PROTOTYPE_METHOD(s_ct, "start", Start);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "keys", SetGrabKeys);

    // FINALLY: export the current function template
    target->Set(String::NewSymbol("NodeWM"), s_ct->GetFunction());
  }

  // C++ constructor
  NodeWM() { }
  ~NodeWM() { }

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
        v8::String::New("addMonitor"),
        v8::String::New("updateMonitor"),
        v8::String::New("removeMonitor"),
        v8::String::New("addWindow"),
        v8::String::New("updateWindow"),
        v8::String::New("removeWindow"),
        v8::String::New("rearrange"),
        v8::String::New("mouseDown"),
        v8::String::New("mouseDrag"),
        v8::String::New("configureRequest"),
        v8::String::New("keyPress"),
        v8::String::New("enterNotify"),
        v8::String::New("fullscreen")
      };

    v8::Local<v8::String> value = Local<v8::String>::Cast(args[0]);
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

  void Emit(callback_map event, nwm_emit *ev) {

    // instead of Handle<Value> argument, we will pass a single struct that
    // represents the various event types that nwm generates

    TryCatch try_catch;
    if(this->callbacks[event] != NULL) {
      Handle<Function> *callback = cb_unwrap(this->callbacks[event]);
      (*callback)->Call(Context::GetCurrent()->Global(), 1, ToNode(ev));
      if (try_catch.HasCaught()) {
        FatalException(try_catch);
      }
    }
  }

  #define INT_FIELD(name, value) \
      o->Set(String::NewSymbol(#name), Integer::New(value))

  static Handle<Value>* ToNode(nwm_emit *ev) {
    Local<Object> o = Object::New();
    switch(ev->type) {
      case nwm_Window:
    result->Set(String::NewSymbol("id"), Integer::New(win));
    result->Set(String::NewSymbol("x"), Integer::New(x));
    result->Set(String::NewSymbol("y"), Integer::New(y));
    result->Set(String::NewSymbol("height"), Integer::New(height));
    result->Set(String::NewSymbol("width"), Integer::New(width));
    result->Set(String::NewSymbol("isfloating"), Integer::New(isfloating));

      case nwm_Windowtitle:
  result->Set(String::NewSymbol("id"), Integer::New(win));
  result->Set(String::NewSymbol("title"), String::New(name));
  result->Set(String::NewSymbol("instance"), String::New(instance));
  result->Set(String::NewSymbol("class"), String::New(klass));

      case nwm_Monitor:
        INT_FIELD(id, ev->monitor.id);
        INT_FIELD(x, ev->monitor.x);
        INT_FIELD(y, ev->monitor.y);
        INT_FIELD(width, ev->monitor.width);
        INT_FIELD(height, ev->monitor.height);
        break;
      case nwm_Keypress:
        INT_FIELD(x, ev->keypress.x);
        INT_FIELD(y, ev->keypress.y);
        INT_FIELD(keysym, ev->keypress.keysym);
        INT_FIELD(keycode, ev->keypress.keycode);
        INT_FIELD(modifier, ev->keypress.modifier);
        break;
      case nwm_Mousedrag:
        INT_FIELD(id, ev->mousedrag.id);
        INT_FIELD(x, ev->mousedrag.x);
        INT_FIELD(y, ev->mousedrag.y);
        INT_FIELD(move_y, ev->mousedrag.move_y);
        INT_FIELD(move_x, ev->mousedrag.move_x);
        break;
      case nwm_ButtonPress:
        INT_FIELD(id, ev->xev.xbutton.window);
        INT_FIELD(x, ev->xev.xbutton.x);
        INT_FIELD(y, ev->xev.xbutton.y);
        INT_FIELD(button, ev->xev.xbutton.button);
        INT_FIELD(state, ev->xev.xbutton.state);
        break;
      case nwm_EnterNotify:
        o->Set(String::NewSymbol("id"), Integer::New(ev->xev.xcrossing.window));
        o->Set(String::NewSymbol("x"), Integer::New(ev->xev.xcrossing.x));
        o->Set(String::NewSymbol("y"), Integer::New(ev->xev.xcrossing.y));
        o->Set(String::NewSymbol("x_root"), Integer::New(ev->xev.xcrossing.x_root));
        o->Set(String::NewSymbol("y_root"), Integer::New(ev->xev.xcrossing.y_root));
        break;
      case nwm_ConfigureRequest:
        o->Set(String::NewSymbol("id"), Integer::New(ev->xev.xconfigurerequest.window));
        o->Set(String::NewSymbol("x"), Integer::New(ev->xev.xconfigurerequest.x));
        o->Set(String::NewSymbol("y"), Integer::New(ev->xev.xconfigurerequest.y));
        o->Set(String::NewSymbol("width"), Integer::New(ev->xev.xconfigurerequest.width));
        o->Set(String::NewSymbol("height"), Integer::New(ev->xev.xconfigurerequest.height));
        o->Set(String::NewSymbol("above"), Integer::New(ev->xev.xconfigurerequest.above));
        o->Set(String::NewSymbol("detail"), Integer::New(ev->xev.xconfigurerequest.detail));
        o->Set(String::NewSymbol("value_mask"), Integer::New(ev->xev.xconfigurerequest.value_mask));
        break;
    }
    return o;
  }

  static Handle<Value> SetGrabKeys(const Arguments& args) {
    HandleScope scope;
    unsigned int i;
    v8::Handle<v8::Value> keysym, modifier;
    // extract from args.this
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());
    v8::Local<v8::Array> arr = Local<v8::Array>::Cast(args[0]);

    nwm_empty_keys();
    // set keys
    for(i = 0; i < arr->Length(); i++) {
      v8::Local<v8::Object> obj = Local<v8::Object>::Cast(arr->Get(i));
      keysym = obj->Get(String::NewSymbol("key"));
      modifier = obj->Get(String::NewSymbol("modifier"));
      nwm_add_key(&hw->keys, keysym->IntegerValue(), modifier->IntegerValue());
    }
    return Undefined();
  }

  static Handle<Value> Start(const Arguments& args) {
    HandleScope scope;
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    // DO NOT REMOVE THIS. For some reason, OSX will segfault without it. Not sure why.
    fprintf( stdout, "EIO INIT\n");
    int fd = nwm_init();
    ev_io_init(&hw->watcher, EIO_RealLoop, fd, EV_READ);
    hw->watcher.data = hw;
    ev_io_start(EV_DEFAULT_ &hw->watcher);

    return Undefined();
  }

  static void EIO_RealLoop(EV_P_ struct ev_io* watcher, int revents) {
    fprintf( stdout, "EIO LOOP\n");
    nwm_loop();
  }

  static Handle<Value> ResizeWindow(const Arguments& args) {
    HandleScope scope;
    nwm_resize_window(args[0]->Uint32Value(), args[1]->IntegerValue(), args[2]->IntegerValue());
    return Undefined();
  }

  static Handle<Value> MoveWindow(const Arguments& args) {
    HandleScope scope;
    nwm_move_window(args[0]->Uint32Value(), args[1]->IntegerValue(), args[2]->IntegerValue());
    return Undefined();
  }

  static Handle<Value> FocusWindow(const Arguments& args) {
    HandleScope scope;
    nwm_focus_window(args[0]->Uint32Value());
    return Undefined();
  }

  static Handle<Value> KillWindow(const Arguments& args) {
    HandleScope scope;
    nwm_kill_window(args[0]->Uint32Value());
    return Undefined();
  }

  static Handle<Value> ConfigureWindow(const Arguments& args) {
    HandleScope scope;
    nwm_configure_window(args[0]->Uint32Value(), args[1]->IntegerValue(),
      args[2]->IntegerValue(), args[3]->IntegerValue(), args[4]->IntegerValue(),
      args[5]->IntegerValue(), args[6]->IntegerValue(), args[7]->IntegerValue(),
      args[8]->IntegerValue());
    return Undefined();
  }


  static Handle<Value> NotifyWindow(const Arguments& args) {
    HandleScope scope;
    nwm_notify_window(args[0]->Uint32Value(), args[1]->IntegerValue(),
      args[2]->IntegerValue(), args[3]->IntegerValue(), args[4]->IntegerValue(),
      args[5]->IntegerValue(), args[6]->IntegerValue(), args[7]->IntegerValue(),
      args[8]->IntegerValue());
    return Undefined();
  }

};

Persistent<FunctionTemplate> NodeWM::s_ct;

extern "C" {
  // target for export
  static void init (Handle<Object> target) {
    NodeWM::Init(target);
  }

  // macro to export
  NODE_MODULE(nwm, init);
}
