#include <v8.h>
#include <node.h>
#include <ev.h>
#include <string.h>

extern "C" {
  #include "nwm.h"
}

using namespace node;
using namespace v8;


Persistent<Function>* callbacks[onLast];

class NodeWM: ObjectWrap {
public:
  ev_io watcher;
  // callback storage

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
      callbacks[i] = NULL;
    }
    // use ObjectWrap.Wrap to store hw in this
    hw->Wrap(args.This());
    // return this
    return args.This();
  }

  // EVENTS
  static Handle<Value> OnCallback(const Arguments& args) {
    HandleScope scope;

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
    callbacks[selected] = cb_persist(args[1]);

    return Undefined();
  }

  static void Emit(callback_map event, void *ev) {

    // instead of Handle<Value> argument, we will pass a single struct that
    // represents the various event types that nwm generates

    TryCatch try_catch;
    if(callbacks[event] != NULL) {
      Handle<Function> *callback = cb_unwrap(callbacks[event]);
      (*callback)->Call(Context::GetCurrent()->Global(), 1, ToNode(event, ev));
      if (try_catch.HasCaught()) {
        FatalException(try_catch);
      }
    }
  }

  #define INT_FIELD(name, value) \
      o->Set(String::NewSymbol(#name), Integer::New(value))

  static Handle<Value>* ToNode(callback_map type, void *ev) {
    Local<Object> o = Object::New();
    switch(type) {
      case onAddMonitor:
      case onUpdateMonitor:
      case onRemoveMonitor:
        {
          nwm_monitor* e = (nwm_monitor*) ev;
          INT_FIELD(id, e->id);
          INT_FIELD(x, e->x);
          INT_FIELD(y, e->y);
          INT_FIELD(width, e->width);
          INT_FIELD(height, e->height);
        }
        break;
      case onAddWindow:
      case onFullscreen:
        {
          nwm_window* e = (nwm_window*) ev;
          INT_FIELD(id, e->id);
          INT_FIELD(x, e->x);
          INT_FIELD(y, e->y);
          INT_FIELD(width, e->width);
          INT_FIELD(height, e->height);
          INT_FIELD(isfloating, e->isfloating);
        }
        break;
      case onRemoveWindow:
        {
          nwm_window* e = (nwm_window*) ev;
          INT_FIELD(id, e->id);
        }
        break;
      case onUpdateWindow:
        {
          nwm_window_title* e = (nwm_window_title*) ev;
          INT_FIELD(id, e->id);
          o->Set(String::NewSymbol("title"), String::New(e->title));
          o->Set(String::NewSymbol("instance"), String::New(e->instance));
          o->Set(String::NewSymbol("class"), String::New(e->klass));
        }
        break;
      case onRearrange:
      case onLast:
        // no data
        break;
      case onMouseDown:
        {
          XButtonEvent* e = (XButtonEvent *) ev;
          INT_FIELD(id, e->window);
          INT_FIELD(x, e->x);
          INT_FIELD(y, e->y);
          INT_FIELD(button, e->button);
          INT_FIELD(state, e->state);
        }
        break;
      case onMouseDrag:
        {
          nwm_mousedrag* e = (nwm_mousedrag*) ev;
          INT_FIELD(id, e->id);
          INT_FIELD(x, e->x);
          INT_FIELD(y, e->y);
          INT_FIELD(move_y, e->move_y);
          INT_FIELD(move_x, e->move_x);
        }
        break;
      case onConfigureRequest:
        {
          XConfigureRequestEvent* e = (XConfigureRequestEvent*) ev;
          o->Set(String::NewSymbol("id"), Integer::New(e->window));
          o->Set(String::NewSymbol("x"), Integer::New(e->x));
          o->Set(String::NewSymbol("y"), Integer::New(e->y));
          o->Set(String::NewSymbol("width"), Integer::New(e->width));
          o->Set(String::NewSymbol("height"), Integer::New(e->height));
          o->Set(String::NewSymbol("above"), Integer::New(e->above));
          o->Set(String::NewSymbol("detail"), Integer::New(e->detail));
          o->Set(String::NewSymbol("value_mask"), Integer::New(e->value_mask));
        }
        break;
      case onKeyPress:
        {
          nwm_keypress* e = (nwm_keypress*) ev;
          INT_FIELD(x, e->x);
          INT_FIELD(y, e->y);
          INT_FIELD(keysym, e->keysym);
          INT_FIELD(keycode, e->keycode);
          INT_FIELD(modifier, e->modifier);
        }
        break;
      case onEnterNotify:
       // NOTE: the enternotify structure is also used with only x and y when the selected monitor is updated..
       // that might not be needed, however, since it is not really essential to the Node WM..
        {
          XCrossingEvent* e = (XCrossingEvent*) ev;
          o->Set(String::NewSymbol("id"), Integer::New(e->window));
          o->Set(String::NewSymbol("x"), Integer::New(e->x));
          o->Set(String::NewSymbol("y"), Integer::New(e->y));
          o->Set(String::NewSymbol("x_root"), Integer::New(e->x_root));
          o->Set(String::NewSymbol("y_root"), Integer::New(e->y_root));
        }
        break;
    }
    Local<Value> argv[1];
    argv[0] = o;
    return argv;
  }

  static Handle<Value> SetGrabKeys(const Arguments& args) {
    HandleScope scope;
    unsigned int i;
    v8::Handle<v8::Value> keysym, modifier;
    // extract from args.this
//    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());
    v8::Local<v8::Array> arr = Local<v8::Array>::Cast(args[0]);

    nwm_empty_keys();
    // set keys
    for(i = 0; i < arr->Length(); i++) {
      v8::Local<v8::Object> obj = Local<v8::Object>::Cast(arr->Get(i));
      keysym = obj->Get(String::NewSymbol("key"));
      modifier = obj->Get(String::NewSymbol("modifier"));
      nwm_add_key(keysym->IntegerValue(), modifier->IntegerValue());
    }
    return Undefined();
  }

  static Handle<Value> Start(const Arguments& args) {
    HandleScope scope;
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    nwm_set_emit_function(Emit);

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
    nwm_loop();
    return Undefined();
  }

  static Handle<Value> MoveWindow(const Arguments& args) {
    HandleScope scope;
    nwm_move_window(args[0]->Uint32Value(), args[1]->IntegerValue(), args[2]->IntegerValue());
    nwm_loop();
    return Undefined();
  }

  static Handle<Value> FocusWindow(const Arguments& args) {
    HandleScope scope;
    nwm_focus_window(args[0]->Uint32Value());
    nwm_loop();
    return Undefined();
  }

  static Handle<Value> KillWindow(const Arguments& args) {
    HandleScope scope;
    nwm_kill_window(args[0]->Uint32Value());
    nwm_loop();
    return Undefined();
  }

  static Handle<Value> ConfigureWindow(const Arguments& args) {
    HandleScope scope;
    nwm_configure_window(args[0]->Uint32Value(), args[1]->IntegerValue(),
      args[2]->IntegerValue(), args[3]->IntegerValue(), args[4]->IntegerValue(),
      args[5]->IntegerValue(), args[6]->IntegerValue(), args[7]->IntegerValue(),
      args[8]->IntegerValue());
    nwm_loop();
    return Undefined();
  }


  static Handle<Value> NotifyWindow(const Arguments& args) {
    HandleScope scope;
    nwm_notify_window(args[0]->Uint32Value(), args[1]->IntegerValue(),
      args[2]->IntegerValue(), args[3]->IntegerValue(), args[4]->IntegerValue(),
      args[5]->IntegerValue(), args[6]->IntegerValue(), args[7]->IntegerValue(),
      args[8]->IntegerValue());
    nwm_loop();
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
