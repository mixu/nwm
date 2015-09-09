#include <nan.h>
#include <string.h>

extern "C" {
  #include "nwm.h"
}

using namespace node;
using namespace v8;

static void EIO_Loop(uv_poll_t* handle, int status, int events);

// callback storage
Nan::Callback * callbacks[onLast];

// EVENTS
static NAN_METHOD(OnCallback) {
  Nan::HandleScope scope;

  Nan::MaybeLocal<v8::String> map[onLast+1] = {
      Nan::New<String>("addMonitor"),
      Nan::New<String>("updateMonitor"),
      Nan::New<String>("removeMonitor"),
      Nan::New<String>("addWindow"),
      Nan::New<String>("updateWindow"),
      Nan::New<String>("removeWindow"),
      Nan::New<String>("rearrange"),
      Nan::New<String>("mouseDown"),
      Nan::New<String>("mouseDrag"),
      Nan::New<String>("configureRequest"),
      Nan::New<String>("keyPress"),
      Nan::New<String>("enterNotify"),
      Nan::New<String>("fullscreen")
    };

  v8::String::Utf8Value param1(info[0]->ToString());
  int selected = -1;
  for(int i = 0; i < onLast; i++) {
    if( strcmp(*param1,  *v8::String::Utf8Value (map[i].ToLocalChecked()->ToString())) == 0 ) {
      selected = i;
      break;
    }
  }
  if (selected != -1) {
    // store function
    callbacks[selected] = new Nan::Callback(info[1].As<Function>());
  }

  info.GetReturnValue().SetUndefined();
}

#define INT_FIELD(name, value) \
    Nan::Set(o, Nan::New(name).ToLocalChecked(), Nan::New(value));

static void Emit(callback_map event, void *ev) {
  Nan::HandleScope scope;


  v8::Local<v8::Object> o = Nan::New<v8::Object>();
  switch(event) {
    case onAddMonitor:
    case onUpdateMonitor:
    case onRemoveMonitor:
      {
        nwm_monitor* e = (nwm_monitor*) ev;
        INT_FIELD("id", e->id);
        INT_FIELD("x", e->x);
        INT_FIELD("y", e->y);
        INT_FIELD("width", e->width);
        INT_FIELD("height", e->height);
      }
      break;
    case onAddWindow:
    case onFullscreen:
      {
        nwm_window* e = (nwm_window*) ev;
        INT_FIELD("id", e->id);
        INT_FIELD("x", e->x);
        INT_FIELD("y", e->y);
        INT_FIELD("width", e->width);
        INT_FIELD("height", e->height);
        INT_FIELD("isfloating", e->isfloating);
      }
      break;
    case onRemoveWindow:
      {
        nwm_window* e = (nwm_window*) ev;
        INT_FIELD("id", e->id);
      }
      break;
    case onUpdateWindow:
      {
        nwm_window_title* e = (nwm_window_title*) ev;
        INT_FIELD("id", e->id);
        Nan::Set(o, Nan::New("title").ToLocalChecked(), Nan::New(e->title).ToLocalChecked());
        Nan::Set(o, Nan::New("instance").ToLocalChecked(), Nan::New(e->instance).ToLocalChecked());
        Nan::Set(o, Nan::New("class").ToLocalChecked(), Nan::New(e->klass).ToLocalChecked());
      }
      break;
    case onRearrange:
    case onLast:
      // no data
      break;
    case onMouseDown:
      {
        XButtonEvent* e = (XButtonEvent *) ev;
        Nan::Set(o, Nan::New("id").ToLocalChecked(), Nan::New<Uint32>((int32_t)e->window));
        INT_FIELD("x", e->x);
        INT_FIELD("y", e->y);
        INT_FIELD("button", e->button);
        INT_FIELD("state", e->state);
      }
      break;
    case onMouseDrag:
      {
        nwm_mousedrag* e = (nwm_mousedrag*) ev;
        Nan::Set(o, Nan::New("id").ToLocalChecked(), Nan::New<Uint32>((int32_t)e->id));
        INT_FIELD("x", e->x);
        INT_FIELD("y", e->y);
        INT_FIELD("move_y", e->move_y);
        INT_FIELD("move_x", e->move_x);
      }
      break;
    case onConfigureRequest:
      {
        XConfigureRequestEvent* e = (XConfigureRequestEvent*) ev;
        Nan::Set(o, Nan::New("id").ToLocalChecked(), Nan::New<Uint32>((int32_t)e->window));
        Nan::Set(o, Nan::New("x").ToLocalChecked(), Nan::New(e->x));
        Nan::Set(o, Nan::New("y").ToLocalChecked(), Nan::New(e->y));
        Nan::Set(o, Nan::New("width").ToLocalChecked(), Nan::New(e->width));
        Nan::Set(o, Nan::New("height").ToLocalChecked(), Nan::New(e->height));
        Nan::Set(o, Nan::New("above").ToLocalChecked(), Nan::New<Uint32>((int32_t)e->above));
        Nan::Set(o, Nan::New("detail").ToLocalChecked(), Nan::New(e->detail));
        Nan::Set(o, Nan::New("value_mask").ToLocalChecked(), Nan::New<Uint32>((int32_t)e->value_mask));
      }
      break;
    case onKeyPress:
      {
        nwm_keypress* e = (nwm_keypress*) ev;
        INT_FIELD("x", e->x);
        INT_FIELD("y", e->y);
        Nan::Set(o, Nan::New("keysym").ToLocalChecked(), Nan::New<Uint32>((int32_t)e->keysym));
        INT_FIELD("keycode", e->keycode);
        INT_FIELD("modifier", e->modifier);
      }
      break;
    case onEnterNotify:
     // NOTE: the enternotify structure is also used with only x and y when the selected monitor is updated..
     // that might not be needed, however, since it is not really essential to the Node WM..
      {
        XCrossingEvent* e = (XCrossingEvent*) ev;
        Nan::Set(o, Nan::New("id").ToLocalChecked(), Nan::New<Uint32>((int32_t)e->window));
        Nan::Set(o, Nan::New("x").ToLocalChecked(), Nan::New(e->x));
        Nan::Set(o, Nan::New("y").ToLocalChecked(), Nan::New(e->y));
        Nan::Set(o, Nan::New("x_root").ToLocalChecked(), Nan::New(e->x_root));
        Nan::Set(o, Nan::New("y_root").ToLocalChecked(), Nan::New(e->y_root));
      }
      break;
  }
  Local<Value> argv[1];
  argv[0] = o;

  // instead of Handle<Value> argument, we will pass a single struct that
  // represents the various event types that nwm generates

  if(callbacks[event] != NULL) {

    callbacks[event]->Call(Nan::GetCurrentContext()->Global(), 1, argv);
/*
    Handle<Function> *callback = cb_unwrap(callbacks[event]);
    (*callback)->Call(Context::GetCurrent()->Global(), 1, argv);
    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }
  */
  }
}

static NAN_METHOD(SetGrabKeys) {
  Nan::HandleScope scope;
  unsigned int i;
  Nan::MaybeLocal<v8::Value> keysym, modifier;
  int32_t kk;
  uint32_t mm;
  v8::Local<v8::Array> arr = Local<v8::Array>::Cast(info[0]);

  nwm_empty_keys();
  // set keys
  for(i = 0; i < arr->Length(); i++) {
    v8::Local<v8::Object> obj = Local<v8::Object>::Cast(arr->Get(i));
    keysym = Nan::Get(obj, Nan::New("key").ToLocalChecked());
    modifier = Nan::Get(obj, Nan::New("modifier").ToLocalChecked());
    kk = (unsigned int)Nan::To<uint32_t>(keysym.ToLocalChecked()).FromMaybe(0);
    mm = Nan::To<uint32_t>(modifier.ToLocalChecked()).FromMaybe(0);
    nwm_add_key(kk, mm);
  }
  info.GetReturnValue().SetUndefined();
}

static NAN_METHOD(Start) {
  Nan::HandleScope scope;

  nwm_set_emit_function(Emit);


  int fd = nwm_init();

  uv_poll_t* handle = new uv_poll_t;
  uv_poll_init(uv_default_loop(), handle, fd);
  uv_poll_start(handle, UV_READABLE, EIO_Loop);

  info.GetReturnValue().SetUndefined();
}

static void EIO_Loop(uv_poll_t* handle, int status, int events) {
  nwm_loop();
}

static NAN_METHOD(ResizeWindow) {
  Nan::HandleScope scope;
  nwm_resize_window(info[0]->Uint32Value(), info[1]->IntegerValue(), info[2]->IntegerValue());
  info.GetReturnValue().SetUndefined();
}

static NAN_METHOD(MoveWindow) {
  Nan::HandleScope scope;
  nwm_move_window(info[0]->Uint32Value(), info[1]->IntegerValue(), info[2]->IntegerValue());
  info.GetReturnValue().SetUndefined();
}

static NAN_METHOD(FocusWindow) {
  Nan::HandleScope scope;
  nwm_focus_window(info[0]->Uint32Value());
  info.GetReturnValue().SetUndefined();
}

static NAN_METHOD(KillWindow) {
  Nan::HandleScope scope;
  nwm_kill_window(info[0]->Uint32Value());
  info.GetReturnValue().SetUndefined();
}

static NAN_METHOD(ConfigureWindow) {
  Nan::HandleScope scope;
  nwm_configure_window(info[0]->Uint32Value(), info[1]->IntegerValue(),
    info[2]->IntegerValue(), info[3]->IntegerValue(), info[4]->IntegerValue(),
    info[5]->IntegerValue(), info[6]->IntegerValue(), info[7]->IntegerValue(),
    info[8]->IntegerValue());
  info.GetReturnValue().SetUndefined();
}

static NAN_METHOD(NotifyWindow) {
  Nan::HandleScope scope;
  nwm_notify_window(info[0]->Uint32Value(), info[1]->IntegerValue(),
    info[2]->IntegerValue(), info[3]->IntegerValue(), info[4]->IntegerValue(),
    info[5]->IntegerValue(), info[6]->IntegerValue(), info[7]->IntegerValue(),
    info[8]->IntegerValue());
  info.GetReturnValue().SetUndefined();
}

NAN_MODULE_INIT(init) {
  for(int i = 0; i < onLast; i++) {
    callbacks[i] = NULL;
  }
  // Callbacks
  Nan::Set(target, Nan::New<String>("on").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(OnCallback)).ToLocalChecked());
  // API
  Nan::Set(target, Nan::New<String>("moveWindow").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(MoveWindow)).ToLocalChecked());
  Nan::Set(target, Nan::New<String>("resizeWindow").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(ResizeWindow)).ToLocalChecked());
  Nan::Set(target, Nan::New<String>("focusWindow").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(FocusWindow)).ToLocalChecked());
  Nan::Set(target, Nan::New<String>("killWindow").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(KillWindow)).ToLocalChecked());
  Nan::Set(target, Nan::New<String>("configureWindow").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(ConfigureWindow)).ToLocalChecked());
  Nan::Set(target, Nan::New<String>("notifyWindow").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(NotifyWindow)).ToLocalChecked());
  // Setting up
  Nan::Set(target, Nan::New<String>("start").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(Start)).ToLocalChecked());
  Nan::Set(target, Nan::New<String>("keys").ToLocalChecked(),
    Nan::GetFunction(Nan::New<FunctionTemplate>(SetGrabKeys)).ToLocalChecked());
}

NODE_MODULE(nwm, init);
