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
extern void nwm_add_key(KeySym keysym, unsigned int mod);

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
extern void nwm_set_emit_function(void (*callback)(callback_map event, void *ev));

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

typedef struct {
  // keypress
  int x;
  int y;
  unsigned int keycode;
  KeySym keysym;
  unsigned int modifier;
} nwm_keypress;

typedef struct {
  // mousegdrag
  Window id;
  int x;
  int y;
  int move_x;
  int move_y;

} nwm_mousedrag;

typedef struct {
  // monitor
  int id;
  int x;
  int y;
  int width;
  int height;
} nwm_monitor;

typedef struct {
  // window
  int id;
  int x;
  int y;
  int width;
  int height;
  Bool isfloating;
} nwm_window;

typedef struct {
  // window
  int id;
  int fullscreen;
} nwm_window_fullscreen;

typedef struct {
  // window
  int id;
  char* title;
  char* instance;
  char* klass;
} nwm_window_title;

typedef struct {
  // window
  int id;
  int x;
  int y;
} nwm_enternotify;

