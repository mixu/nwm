/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying
 * LICENSE file.
 */

#include <v8.h>
#include <node.h>
#include <ev.h>

#include <vector>
#include <algorithm>
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

typedef struct Client Client;
typedef struct NodeWM NodeWM;

static const char broken[] = "broken";
#include "client.h"
int xerror(Display *dpy, XErrorEvent *ee) {
  if(ee->error_code == BadWindow
  || (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
  || (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
  || (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
  || (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
  || (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
  || (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
  || (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
  || (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
    return 0;
  fprintf(stdout, "nwm: fatal error: request code=%d, error code=%d\n",
      ee->request_code, ee->error_code);
//    return xerrorxlib(dpy, ee); /* may call exit */
  exit(-1);
  return 0;
}

int xerrordummy(Display *dpy, XErrorEvent *ee) {
  return 0;
}

Bool gettextprop(Display* dpy, Window w, Atom atom, char *text, unsigned int size) {
  char **list = NULL;
  int n;
  XTextProperty name;

  if(!text || size == 0)
    return False;
  text[0] = '\0';
  XGetTextProperty(dpy, w, &name, atom);
  if(!name.nitems)
    return False;
  if(name.encoding == XA_STRING)
    strncpy(text, (char *)name.value, size - 1);
  else {
    if(XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
      strncpy(text, *list, size - 1);
      XFreeStringList(list);
    }
  }
  text[size - 1] = '\0';
  XFree(name.value);
  return True;
}


enum callback_map {
  onAddMonitor,
  onUpdateMonitor,
  onRemoveMonitor,
  onAddWindow,
  onUpdateWindow,
  onRemoveWindow,
  onRearrange,
  onMouseDown,
  onMouseDrag,
  onConfigureRequest,
  onKeyPress,
  onEnterNotify,
  onFullscreen,
  onLast
};


class NodeWM: ObjectWrap
{
private:
  // X11
  Display *dpy;
  GC gc;
  Window root;
  Window selected;
  // We only track the number of monitors: that allows us to tell if a monitor has been removed or added.
  unsigned int total_monitors;
  std::vector <Client> clients;
  std::vector <Window> seen_windows;
  // screen dimensions
  int screen, screen_width, screen_height;
  // callback storage
  Persistent<Function>* callbacks[onLast];
  // grabbed keys
  Key* keys;
  unsigned int numlockmask;

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
    NODE_SET_PROTOTYPE_METHOD(s_ct, "configureWindow", ConfigureWindow);

    // Setting up
    NODE_SET_PROTOTYPE_METHOD(s_ct, "start", Start);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "keys", SetGrabKeys);

    // FINALLY: export the current function template
    target->Set(String::NewSymbol("NodeWM"), s_ct->GetFunction());
  }

  // C++ constructor
  NodeWM() :
    total_monitors(0),
    keys(NULL),
    numlockmask(0)
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

  static Bool isuniquegeom(XineramaScreenInfo *unique, size_t len, XineramaScreenInfo *info) {
    unsigned int i;

    for(i = 0; i < len; i++)
      if(unique[i].x_org == info->x_org && unique[i].y_org == info->y_org
      && unique[i].width == info->width && unique[i].height == info->height)
        return False;
    return True;
  }

  Client* getClientByWindow(Window win) {
    unsigned int j;
    for(j = 0; j < this->clients.size(); j++) {
      if(this->clients[j].getWin() == win)
        return &this->clients[j];
    }
    return NULL;
  }

  static void updateGeometry(NodeWM* hw) {
    if(XineramaIsActive(hw->dpy)) {
      fprintf( stdout, "Xinerama active\n");
      int i, n, nn;
      unsigned int j;
      XineramaScreenInfo *info = XineramaQueryScreens(hw->dpy, &nn);
      XineramaScreenInfo *unique = NULL;

      n = hw->total_monitors;
      fprintf( stdout, "Monitors known %d, monitors found %d\n", n, nn);
      /* only consider unique geometries as separate screens */
      if(!(unique = (XineramaScreenInfo *)malloc(sizeof(XineramaScreenInfo) * nn))) {
        fprintf( stdout, "fatal: could not malloc() %lu bytes\n", sizeof(XineramaScreenInfo) * nn);
        exit( -1 );
      }
      for(i = 0, j = 0; i < nn; i++)
        if(isuniquegeom(unique, j, &info[i]))
          memcpy(&unique[j++], &info[i], sizeof(XineramaScreenInfo));
      XFree(info);
      nn = j;
      if(n <= nn) {
        // reserve space
        for(i = 0; i < (nn - n); i++) { // new monitors available
          fprintf(stdout, "Monitor %d \n", i);
          hw->total_monitors++;
        }
        // update monitor dimensions
        //  We just emit the monitors and don't track the dimensions in the binding at all.
        for(i = 0; i < nn; i++) {
          // emit ADD MONITOR or UPDATE MONITOR here
          Local<Value> argv[1];
          Local<Object> result = Object::New();
          result->Set(String::NewSymbol("id"), Integer::New(i));
          result->Set(String::NewSymbol("x"), Integer::New(unique[i].x_org));
          result->Set(String::NewSymbol("y"), Integer::New(unique[i].y_org));
          result->Set(String::NewSymbol("width"), Integer::New(unique[i].width));
          result->Set(String::NewSymbol("height"), Integer::New(unique[i].height));
          argv[0] = result;
          fprintf( stdout, "Emit monitor %d\n", i);
          if(i >= n) {
            hw->Emit(onAddMonitor, 1, argv);
          } else {
            hw->Emit(onUpdateMonitor, 1, argv);
          }
        }
      } else { // fewer monitors available nn < n
        fprintf( stdout, "Less monitors available %d %d\n", n, nn);
        for(i = nn; i < n; i++) {
          // emit REMOVE MONITOR (i)
          Local<Value> argv[1];
          argv[0] = Integer::New(i);
          hw->Emit(onRemoveMonitor, 1, argv);
          // remove monitor
          hw->total_monitors--;
        }
      }
      free(unique);
    } else {
      if(hw->total_monitors == 0) {
        hw->total_monitors++;
        // emit ADD MONITOR
        Local<Value> argv[1];
        Local<Object> result = Object::New();
        result->Set(String::NewSymbol("id"), Integer::New(0));
        result->Set(String::NewSymbol("x"), Integer::New(0));
        result->Set(String::NewSymbol("y"), Integer::New(0));
        result->Set(String::NewSymbol("width"), Integer::New(hw->screen_width));
        result->Set(String::NewSymbol("height"), Integer::New(hw->screen_height));
        argv[0] = result;
        hw->Emit(onAddMonitor, 1, argv);
      }
    }
    // update the selected monitor on Node.js side
    int x, y;
    Local<Value> argv[1];
    if(hw->getrootptr(&x, &y)) {
      fprintf(stdout, "EmitEnterNotify wid = %li \n", hw->root);
      Local<Object> result = Object::New();
      result->Set(String::NewSymbol("id"), Integer::New(hw->root));
      result->Set(String::NewSymbol("x"), Integer::New(x));
      result->Set(String::NewSymbol("y"), Integer::New(y));
      argv[0] = result;
      hw->Emit(onEnterNotify, 1, argv);
    }

    fprintf( stdout, "Done with updategeom\n");
    return;
  }

  /**
   * Prepare the window object and call the Node.js callback.
   */
  static void HandleAdd(NodeWM* hw, Window win, XWindowAttributes *wa) {
    Window trans = None;
    TryCatch try_catch;
    // onManage receives a window object
    Local<Value> argv[1];
    Bool isfloating = false;
    // check whether the window is transient
    XGetTransientForHint(hw->dpy, win, &trans);
    isfloating = (trans != None);
    Client* c = new Client(win, wa->x, wa->y, wa->height, wa->width, isfloating);
    // add to global window list
    hw->clients.push_back(*c);
    argv[0] = c->toNode();

    // call the callback in Node.js, passing the window object...
    hw->Emit(onAddWindow, 1, argv);
    hw->updateWindowStr(win); // update title and class, emit onUpdateWindow

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

    fprintf( stdout, "manage: x=%d y=%d width=%d height=%d \n", ce.x, ce.y, ce.width, ce.height);

    XSendEvent(hw->dpy, win, False, StructureNotifyMask, (XEvent *)&ce);

    // subscribe to window events
    XSelectInput(hw->dpy, win, EnterWindowMask|FocusChangeMask|PropertyChangeMask|StructureNotifyMask);
    hw->GrabButtons(win, False);

    if(isfloating) {
      XRaiseWindow(hw->dpy, c->getWin());
    }

    // move and (finally) map the window
    XMoveResizeWindow(hw->dpy, win, ce.x, ce.y, ce.width, ce.height);
    XMapWindow(hw->dpy, win);

    delete c;
 }

  static Handle<Value> ResizeWindow(const Arguments& args) {
    HandleScope scope;
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    Window id = args[0]->Uint32Value();
    int width = args[1]->IntegerValue();
    int height = args[2]->IntegerValue();

    fprintf( stdout, "ResizeWindow: id=%li width=%d height=%d \n", id, width, height);
    XResizeWindow(hw->dpy, id, width, height);
    XFlush(hw->dpy);

    return Undefined();
  }

  static Handle<Value> MoveWindow(const Arguments& args) {
    HandleScope scope;
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    Window id = args[0]->Uint32Value();
    int x = args[1]->IntegerValue();
    int y = args[2]->IntegerValue();

    fprintf( stdout, "MoveWindow: id=%li x=%d y=%d \n", id, x, y);
    XMoveWindow(hw->dpy, id, x, y);
    XFlush(hw->dpy);
    return Undefined();
  }

  static Handle<Value> FocusWindow(const Arguments& args) {
    HandleScope scope;
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    Window id = args[0]->Uint32Value();
    RealFocus(hw, id);
    return Undefined();
  }

  static Bool isprotodel(Display* dpy, Window win) {
    int i, n;
    Atom *protocols;
    Bool ret = False;

    if(XGetWMProtocols(dpy, win, &protocols, &n)) {
      for(i = 0; !ret && i < n; i++)
        if(protocols[i] == XInternAtom(dpy, "WM_DELETE_WINDOW", False))
          ret = True;
      XFree(protocols);
    }
    return ret;
  }

  static Handle<Value> KillWindow(const Arguments& args) {
    HandleScope scope;
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    Window id = args[0]->Uint32Value();

    XEvent ev;
    // check whether the client supports "graceful" termination
    if(isprotodel(hw->dpy, id)) {
      ev.type = ClientMessage;
      ev.xclient.window = id;
      ev.xclient.message_type = XInternAtom(hw->dpy, "WM_PROTOCOLS", False);
      ev.xclient.format = 32;
      ev.xclient.data.l[0] = XInternAtom(hw->dpy, "WM_DELETE_WINDOW", False);
      ev.xclient.data.l[1] = CurrentTime;
      XSendEvent(hw->dpy, id, False, NoEventMask, &ev);
    }
    else {
      XGrabServer(hw->dpy);
      XSetErrorHandler(xerrordummy);
      XSetCloseDownMode(hw->dpy, DestroyAll);
      XKillClient(hw->dpy, id);
      XSync(hw->dpy, False);
      XSetErrorHandler(xerror);
      XUngrabServer(hw->dpy);
    }
    return Undefined();
  }

  static Handle<Value> ConfigureWindow(const Arguments& args) {
    HandleScope scope;
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    hw->RealConfigureWindow(args[0]->Uint32Value(), args[1]->IntegerValue(),
      args[2]->IntegerValue(), args[3]->IntegerValue(), args[4]->IntegerValue(),
      args[5]->IntegerValue(), args[6]->IntegerValue(), args[7]->IntegerValue(),
      args[8]->IntegerValue());
    return Undefined();
  }

  void RealConfigureWindow(Window win, int x, int y, int width, int height,
    int border_width, int above, int detail, int value_mask) {
      XWindowChanges wc;
      wc.x = x;
      wc.y = y;
      wc.width = width;
      wc.height = height;
      wc.border_width = border_width;
      wc.sibling = above;
      wc.stack_mode = detail;
      XConfigureWindow(this->dpy, win, value_mask, &wc);
  }

  static void RealFocus(NodeWM* hw, Window win) {
    fprintf( stdout, "FocusWindow: id=%li\n", win);
    hw->GrabButtons(win, True);
    XSetInputFocus(hw->dpy, win, RevertToPointerRoot, CurrentTime);
    Atom atom = XInternAtom(hw->dpy, "WM_TAKE_FOCUS", False);
    SendEvent(hw, win, atom);
    XFlush(hw->dpy);
    hw->selected = win;
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
  void GrabButtons(Window wnd, Bool focused) {
    this->updatenumlockmask();
//    {
//      unsigned int i;
//      unsigned int modifiers[] = { 0, LockMask, this->numlockmask, this->numlockmask|LockMask };
//      XUngrabButton(this->dpy, AnyButton, AnyModifier, wnd);
//      if(focused) {
//        fprintf( stdout, "GRABBUTTONS - focused: true\n");
//          for(i = 0; i < 4; i++) {
//            XGrabButton(dpy, Button1,
//                              Mod4Mask|ControlMask|modifiers[i],
//                              wnd, False, (ButtonPressMask|ButtonReleaseMask),
//                              GrabModeAsync, GrabModeSync, None, None);
//          }
//      } else {
//        fprintf( stdout, "GRABBUTTONS - focused: false\n");
//        XGrabButton(this->dpy, AnyButton, AnyModifier, wnd, False,
//                    (ButtonPressMask|ButtonReleaseMask), GrabModeAsync, GrabModeSync, None, None);
//      }
//    }
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
      fprintf( stdout, "fatal: could not malloc() %lu bytes\n", sizeof(Key));
      exit( -1 );
    }
    curr->keysym = keysym;
    curr->mod = mod;
    curr->next = *keys;
    *keys = curr;
  }

  void updatenumlockmask(void) {
    unsigned int i;
    int j;
    XModifierKeymap *modmap;

    this->numlockmask = 0;
    modmap = XGetModifierMapping(dpy);
    for(i = 0; i < 8; i++)
      for(j = 0; j < modmap->max_keypermod; j++)
        if(modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dpy, XK_Num_Lock))
          this->numlockmask = (1 << i);
    XFreeModifiermap(modmap);
  }

  static void GrabKeys(NodeWM* hw, Display* dpy, Window root) {
    hw->updatenumlockmask();
    {
      unsigned int i;
      unsigned int modifiers[] = { 0, LockMask, hw->numlockmask, hw->numlockmask|LockMask };
      XUngrabKey(hw->dpy, AnyKey, AnyModifier, hw->root);
      for(Key* curr = hw->keys; curr != NULL; curr = curr->next) {
        fprintf( stdout, "grab key -- key: %li modifier %d \n", curr->keysym, curr->mod);
        // also grab the combinations of screen lock and num lock (as those should not matter)
        for(i = 0; i < 4; i++) {
          XGrabKey(hw->dpy, XKeysymToKeycode(hw->dpy, curr->keysym), curr->mod | modifiers[i], hw->root, True, GrabModeAsync, GrabModeAsync);
        }
      }
    }
  }

  static void HandleButtonPress(NodeWM* hw, XEvent *e) {
    XButtonPressedEvent *ev = &e->xbutton;
    Local<Value> argv[1];

    fprintf(stdout, "Handle(mouse)ButtonPress\n");

    // fetch root_x,root_y

    argv[0] = NodeWM::makeButtonPress(ev->window, ev->x, ev->y, ev->button, ev->state);
    // call the callback in Node.js, passing the window object...
    hw->Emit(onMouseDown, 1, argv);
    fprintf(stdout, "Call cbButtonPress\n");
    // now emit the drag events
    GrabMouseRelease(hw, ev->window);
  }

  Bool getrootptr(int *x, int *y) {
    int di;
    unsigned int dui;
    Window dummy;

    return XQueryPointer(this->dpy, this->root, &dummy, &dummy, x, y, &di, &di, &dui);
  }

  static void GrabMouseRelease(NodeWM* hw, Window id) {
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

#include "return_values.cc"

  void addWindow(Window win, int x, int y, int width, int height, Bool isfloating) {
    fprintf( stderr, "Create client %li (x %d, y %d, w %d, h %d, float %d)\n", win, x, y, width, height, isfloating);
    // window object to return
    Local<Object> result = Object::New();

    // read and set the window geometry
    result->Set(String::NewSymbol("id"), Integer::New(win));
    result->Set(String::NewSymbol("x"), Integer::New(x));
    result->Set(String::NewSymbol("y"), Integer::New(y));
    result->Set(String::NewSymbol("height"), Integer::New(height));
    result->Set(String::NewSymbol("width"), Integer::New(width));
    result->Set(String::NewSymbol("isfloating"), Integer::New(isfloating));

  }

  void updateWindowStr(Window win) {
    char name[256];
    char klass[256];
    char instance[256];
    // update title
    Atom NetWMName = XInternAtom(dpy, "_NET_WM_NAME", False);
    if(!gettextprop(this->dpy, win, NetWMName, name, sizeof name))
      gettextprop(this->dpy, win, XA_WM_NAME, name, sizeof name);
    if(name[0] == '\0') /* hack to mark broken clients */
      strcpy(name, broken);
    // update class
    XClassHint ch = { 0 };
    if(XGetClassHint(this->dpy, win, &ch)) {
      if(ch.res_class) {
        strncpy(klass, ch.res_class, 256-1 );
      } else {
        strncpy(klass, broken, 256-1 );
      }
      klass[256-1] = 0;
      if(ch.res_name) {
        strncpy(instance, ch.res_name, 256-1 );
      } else {
        strncpy(instance, broken, 256-1 );
      }
      instance[256-1] = 0;
      if(ch.res_class)
        XFree(ch.res_class);
      if(ch.res_name)
        XFree(ch.res_name);
    }

    Local<Object> result = Object::New();
    result->Set(String::NewSymbol("id"), Integer::New(win));
    result->Set(String::NewSymbol("title"), String::New(name));
    result->Set(String::NewSymbol("instance"), String::New(instance));
    result->Set(String::NewSymbol("class"), String::New(klass));

    // emit onUpdateWindow
    Local<Value> argv[1];
    argv[0] = result;
    this->Emit(onUpdateWindow, 1, argv);
  }

  static void HandleKeyPress(NodeWM* hw, XEvent *e) {
    KeySym keysym;
    XKeyEvent *ev;

    ev = &e->xkey;
    keysym = XKeycodeToKeysym(hw->dpy, (KeyCode)ev->keycode, 0);
    Local<Value> argv[1];
    // we always unset numlock and LockMask since those should not matter
    argv[0] = NodeWM::makeKeyPress(ev->x, ev->y, ev->keycode, keysym, (ev->state & ~(hw->numlockmask|LockMask)));
    // call the callback in Node.js, passing the window object...
    hw->Emit(onKeyPress, 1, argv);
  }

  static void HandleEnterNotify(NodeWM* hw, XEvent *e) {
    XCrossingEvent *ev = &e->xcrossing;
    // onManage receives a window object
    fprintf(stdout, "HandleEnterNotify wid = %li \n", ev->window);
    Local<Value> argv[1];
    Local<Object> result = Object::New();
    result->Set(String::NewSymbol("id"), Integer::New(ev->window));
    result->Set(String::NewSymbol("x"), Integer::New(ev->x));
    result->Set(String::NewSymbol("y"), Integer::New(ev->y));
    result->Set(String::NewSymbol("x_root"), Integer::New(ev->x_root));
    result->Set(String::NewSymbol("y_root"), Integer::New(ev->y_root));
    argv[0] = result;
    // call the callback in Node.js, passing the window object...
    // Note that this also called for the root window (focus monitor)
    hw->Emit(onEnterNotify, 1, argv);
  }

  static void HandleFocusIn(NodeWM* hw, XEvent *e) {
    XFocusChangeEvent *ev = &e->xfocus;
    fprintf(stdout, "HandleFocusIn for window id %li\n", ev->window);
    if(hw->selected && ev->window != hw->selected){
      // Preventing focus stealing
      // http://mail.gnome.org/archives/wm-spec-list/2003-May/msg00013.html
      // We will always revert the focus to whatever was last set by Node (e.g. enterNotify).
      // This prevents naughty applications from stealing the focus permanently.
      fprintf(stdout, "Reverting focus change by window id %li to %li \n", ev->window, hw->selected);
      RealFocus(hw, hw->selected);
    }
  }

  static void HandleUnmapNotify(NodeWM* hw, XEvent *e) {
    Client *c;
    XUnmapEvent *ev = &e->xunmap;
    if((c = hw->getClientByWindow(ev->window)))
      NodeWM::HandleRemove(hw, c, False);
  }

  static void HandleDestroyNotify(NodeWM* hw, XEvent *e) {
    Client *c;
    XDestroyWindowEvent *ev = &e->xdestroywindow;

    if((c = hw->getClientByWindow(ev->window)))
      NodeWM::HandleRemove(hw, c, True);
  }

  static void HandleConfigureNotify(NodeWM* hw, XEvent *e) {
    XConfigureEvent *ev = &e->xconfigure;

    if(ev->window == hw->root) {
      hw->screen_width = ev->width;
      hw->screen_height = ev->height;
      // update monitor structures
      updateGeometry(hw);
      hw->Emit(onRearrange, 0, 0);
    }
  }

  static void HandlePropertyNotify(NodeWM* hw, XEvent *e) {
    XPropertyEvent *ev = &e->xproperty;
    // could be used for tracking hints, transient status and window name
    if((ev->window == hw->root) && (ev->atom == XA_WM_NAME)) {
      // the root window name has changed
    } else if(ev->state == PropertyDelete) {
      return; // ignore property deletes
    } else {
      Client* c = hw->getClientByWindow(ev->window);
      if(c) {
        Atom NetWMName = XInternAtom(hw->dpy, "_NET_WM_NAME", False);
        if(ev->atom == XA_WM_NAME || ev->atom == NetWMName) {
          hw->updateWindowStr(ev->window); // update title and class
        }
      }
    }
  }

  static void HandleClientMessage(NodeWM* hw, XEvent *e) {
    XClientMessageEvent *cme = &e->xclient;
    Client *c = hw->getClientByWindow(cme->window);
    Atom NetWMState = XInternAtom(hw->dpy, "_NET_WM_STATE", False);
    Atom NetWMFullscreen = XInternAtom(hw->dpy, "_NET_WM_STATE_FULLSCREEN", False);
    Local<Value> argv[2];

    if((c)
    && (cme->message_type == NetWMState && cme->data.l[1] == NetWMFullscreen)) {
      argv[0] = Integer::New(c->getWin());
      if(cme->data.l[0]) {
        XChangeProperty(hw->dpy, cme->window, NetWMState, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)&NetWMFullscreen, 1);
        argv[1] = Integer::New(1);
        XRaiseWindow(hw->dpy, c->getWin());
      }
      else {
        XChangeProperty(hw->dpy, cme->window, NetWMState, XA_ATOM, 32,
                        PropModeReplace, (unsigned char*)0, 0);
        argv[1] = Integer::New(0);
      }
      hw->Emit(onFullscreen, 2, argv);
    }
  }

  static void HandleRemove(NodeWM* hw, Client *c, Bool destroyed) {
    fprintf( stdout, "HandleRemove\n");
    Window win = c->getWin();
    // emit a remove
    Local<Value> argv[1];
    argv[0] = Integer::New(win);
    fprintf( stdout, "HandleRemove - emit onRemovewindow, %li\n", win);
    hw->Emit(onRemoveWindow, 1, argv);
    if(!destroyed) {
      fprintf( stdout, "X11 cleanup\n");
      XGrabServer(hw->dpy);
      XUngrabButton(hw->dpy, AnyButton, AnyModifier, win);
      XSync(hw->dpy, False);
      XUngrabServer(hw->dpy);
    }
    // remove from monitor
    fprintf( stdout, "Iterate window list to remove window from global list\n");
    std::vector<Client>& vec = hw->clients; // use shorter name
    std::vector<Client>::iterator iter = vec.begin();
    iter = vec.begin();
    while (iter != vec.end()) {
      Window vid = iter->getWin();
      if (vid == win) {
        fprintf( stdout, "Perform erase as item %li is equal to needle %li.\n", vid, win);
        iter = vec.erase(iter);
      } else {
        fprintf( stdout, "Not perform erase as item %li is not equal to needle %li.\n", vid, win);
        ++iter;
      }
    }
    fprintf( stdout, "Focusing to root window\n");
    RealFocus(hw, hw->root);
    fprintf( stdout, "Emitting rearrange\n");
    hw->Emit(onRearrange, 0, 0);
  }

  static Handle<Value> Start(const Arguments& args) {
    HandleScope scope;
    // extract from args.this
    NodeWM* hw = ObjectWrap::Unwrap<NodeWM>(args.This());

    // initialize resources
    // atoms

    // open the display
    if ( ( hw->dpy = XOpenDisplay(NIL) ) == NULL ) {
      fprintf( stdout, "cannot connect to X server %s\n", XDisplayName(NULL));
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

    // Scan and layout current windows

    unsigned int i, num;
    Window d1, d2, *wins = NULL;
    XWindowAttributes watt;
    // XQueryTree() function returns the root ID, the parent window ID, a pointer to
    // the list of children windows (NULL when there are no children), and
    // the number of children in the list for the specified window.
    if(XQueryTree(hw->dpy, hw->root, &d1, &d2, &wins, &num)) {
      for(i = 0; i < num; i++) {
        // if we can't read the window attributes,
        // or the window is a popup (transient or override_redirect), skip it
        if(!XGetWindowAttributes(hw->dpy, wins[i], &watt)
        || watt.override_redirect || XGetTransientForHint(hw->dpy, wins[i], &d1)) {
          continue;
        }
        // visible or minimized window ("Iconic state")
        if(watt.map_state == IsViewable )//|| getstate(wins[i]) == IconicState)
          NodeWM::HandleAdd(hw, wins[i], &watt);
      }
      for(i = 0; i < num; i++) { /* now the transients */
        if(!XGetWindowAttributes(hw->dpy, wins[i], &watt))
          continue;
        if(XGetTransientForHint(hw->dpy, wins[i], &d1)
        && (watt.map_state == IsViewable )) //|| getstate(wins[i]) == IconicState))
          NodeWM::HandleAdd(hw, wins[i], &watt);
      }
      if(wins) {
        // To free a non-NULL children list when it is no longer needed, use XFree()
        XFree(wins);
      }
    }

    // emit a rearrange
    hw->Emit(onRearrange, 0, 0);

    // initialize and start
    XSync(hw->dpy, False);
    // DO NOT REMOVE THIS. For some reason, OSX will segfault without it. Not sure why.
    fprintf( stdout, "EIO INIT\n");
    int fd = XConnectionNumber(hw->dpy);
    ev_io_init(&hw->watcher, EIO_RealLoop, fd, EV_READ);
    hw->watcher.data = hw;
    ev_io_start(EV_DEFAULT_ &hw->watcher);

    return Undefined();
  }

  static void EIO_RealLoop(EV_P_ struct ev_io* watcher, int revents) {
    fprintf( stdout, "EIO LOOP\n");
    NodeWM* hw = static_cast<NodeWM*>(watcher->data);

    XEvent event;
    // main event loop
    while(XPending(hw->dpy)) {
      XNextEvent(hw->dpy, &event);
      fprintf(stdout, "got event %s (%d).\n", event_names[event.type], event.type);
      // handle event internally --> calls Node if necessary
      switch (event.type) {
        case ButtonPress:
          {
            NodeWM::HandleButtonPress(hw, &event);
          }
          break;
        case ClientMessage:
          NodeWM::HandleClientMessage(hw, &event);
          break;
        case ConfigureRequest:
          {
            XConfigureRequestEvent *ev = &event.xconfigurerequest;
            // TODO:
            // dwm checks for whether the window is known,
            // only unknown windows are allowed to configure themselves.

            // We should trigger the Node callback, passing:
            // window id, x, y, width, height, border_width and detail

            Local<Value> argv[1];
            Local<Object> result = Object::New();
            result->Set(String::NewSymbol("id"), Integer::New(ev->window));
            result->Set(String::NewSymbol("x"), Integer::New(ev->x));
            result->Set(String::NewSymbol("y"), Integer::New(ev->y));
            result->Set(String::NewSymbol("width"), Integer::New(ev->width));
            result->Set(String::NewSymbol("height"), Integer::New(ev->height));
            result->Set(String::NewSymbol("above"), Integer::New(ev->above));
            result->Set(String::NewSymbol("detail"), Integer::New(ev->detail));
            result->Set(String::NewSymbol("value_mask"), Integer::New(ev->value_mask));
            argv[0] = result;
            hw->Emit(onConfigureRequest, 1, argv);

            // Node should call AllowReconfigure()
            // or ConfigureNotify() + Move/Resize etc.
          }
          break;
        case ConfigureNotify:
            NodeWM::HandleConfigureNotify(hw, &event);
            break;
        case DestroyNotify:
            NodeWM::HandleDestroyNotify(hw, &event);
            break;
        case EnterNotify:
            NodeWM::HandleEnterNotify(hw, &event);
            break;
//        case Expose:
//            break;
        case FocusIn:
            NodeWM::HandleFocusIn(hw, &event);
            break;
        case KeyPress:
          {
            NodeWM::HandleKeyPress(hw, &event);
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
              fprintf(stdout, "XGetWindowAttributes failed\n");
              return;
            }
            if(wa.override_redirect)
              return;
            fprintf(stdout, "MapRequest\n");
            Client* c = hw->getClientByWindow(ev->window);
            if(c == NULL) {
              // only map new windows
              NodeWM::HandleAdd(hw, ev->window, &wa);
              // emit a rearrange
              hw->Emit(onRearrange, 0, 0);
            } else {
              fprintf(stdout, "Window is known\n");
            }
          }
            break;
        case PropertyNotify:
            NodeWM::HandlePropertyNotify(hw, &event);
            break;
        case UnmapNotify:
            NodeWM::HandleUnmapNotify(hw, &event);
            break;
        default:
          fprintf(stdout, "Did nothing with %s (%d)\n", event_names[event.type], event.type);
          break;
      }
    }
    return;
  }

  ev_io watcher;
};

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
