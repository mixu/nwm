/*
  Rethinking:

  We have several interfaces:

  - One that handles OnCallback(func) and Emit(name, args)
  - Number of events that get emitted:
    - This includes creating the args object before calling emit
    - Each of these has its own signature
  - Then there are functions that get called from Node that deal with windows
  - Then we have Start(), EventLoop() and the X11 event handlers for those

  In terms of simplifying the interface, we could probably get rid of the
  NWM object completely and just export a bunch of functions instead.

 */

#include <X11/keysym.h>
#include <X11/Xlib.h>


// EXTERNAL API

// this should initialize everything except keys
extern int nwm_init();

typedef struct Key Key;
struct Key {
  unsigned int mod;
  KeySym keysym;
  Key* next;
};

// initialize keys
extern void nwm_empty_keys();
extern void nwm_add_key(Key** keys, KeySym keysym, unsigned int mod);
extern void nwm_grab_keys();

// known callbacks
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
typedef enum callback_map callback_map;


// initialize the function that gets called when events are emitted
extern void nwm_emit_function();

// this should be called each time we want to process events/
// that might be because of libev or manually
extern void nwm_loop();


extern void nwm_move_window(Window win, int x, int y);
extern void nwm_resize_window(Window win, int width, int height);
extern void nwm_focus_window(Window win);
extern void nwm_kill_window(Window win);
extern void nwm_configure_window(Window win, int x, int y, int width, int height, \
    int border_width, int above, int detail, int value_mask);
extern void nwm_notify_window(Window win, int x, int y, int width, int height, \
    int border_width, int above, int detail, int value_mask);

enum nwm_event_t {
  nwm_Keypress,
  nwm_Mousedrag,
  nwm_Monitor,
  nwm_ButtonPress,
  nwm_EnterNotify,
  nwm_ConfigureRequest,
  nwm_Window,
  nwm_Windowtitle
};
typedef enum nwm_event_t nwm_event_t;

typedef struct {
  int type;
  // keypress
  int x;
  int y;
  unsigned int keycode;
  KeySym keysym;
  unsigned int modifier;
} nwm_keypress;

typedef struct {
  int type;
  // mousegdrag
  Window id;
  int x;
  int y;
  int move_x;
  int move_y;

} nwm_mousedrag;

typedef struct {
  int type;
  // monitor
  int id;
  int x;
  int y;
  int width;
  int height;
} nwm_monitor;

typedef struct {
  int type;
  // window
  int id;
  int x;
  int y;
  int width;
  int height;
  Bool isfloating;
} nwm_window;

typedef struct {
  int type;
  // window
  int id;
  int fullscreen;
} nwm_window_fullscreen;

typedef struct {
  int type;
  // window
  int id;
  char* title;
  char* instance;
  char* klass;
} nwm_window_title;

typedef struct {
  int type;
  // window
  int id;
  int x;
  int y;
} nwm_enternotify;

typedef union _nwm_event {
  int type;
  nwm_keypress keypress;
  nwm_mousedrag mousedrag;
  nwm_monitor monitor;
  nwm_window window;
  nwm_window_title window_title;
  nwm_enternotify enternotify;
  XEvent xev;
} nwm_event;


typedef struct {
  // X11
  Display *dpy;
  GC gc;
  Window root;
  Window selected;
  // We only track the number of monitors: that allows us to tell if a monitor has been removed or added.
  unsigned int total_monitors;
  // We only track the window ids of windows, nothing else
  Window seen_windows[1024];
  // screen dimensions
  int screen_width, screen_height;
  // grabbed keys
  Key* keys;
  unsigned int numlockmask;
} NodeWinMan;

static NodeWinMan nwm;
