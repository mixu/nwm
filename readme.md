*NOTE: YOU WANT to download or fork https://github.com/mixu/nwm as that repo will have the latest code in the future. The "mixu/nodeko" repo will remain untouched for historical reasons.*

# nwm - node window manager

nwm is a dynamic window manager for X based on dwm. It uses libev to interface with X11, and allows you to lay out windows in Node.

I wrote the code in this repo during Node Knockout 2011. Unfortunately I'm not in the competition since I forgot to confirm my initial team registration (click accept). Not a big deal, my reward is shipping.

The underlying X11 bindings are written as a Node native extension in c. I apologize for the ugliness of the code - if you look at the first commit, I started on Friday with [this tutorial](https://www.cloudkick.com/blog/2010/aug/23/writing-nodejs-native-extensions/), a vague knowledge of X11 and a lot of enthusiasm.

# Features

nwm supports all kinds of layouts - you can write your own, or use the default (tiling) layout. It's like dwm, but layouts are enforced in JS rather than in C.

The default nwm.js starts a REPL, so you can issue commands to it interactively.

# Installing

node-waf configure build
node nwm.js


# Using

To list windows:

    nwm.windows

To manage windows:

    nwm.move(window_id, x, y)
    nwm.resize(window_id, width, height)
    nwm.focus(window_id)
    nwm.hide(window_id)
    nwm.show(window_id)

To apply a layout:

    nwm.tile();

nwm also supports workspaces:

    nwm.go(workspace_number);
    nwm.gimme(window_id);
    nwm.windowTo(window_id, workspace_number);


There are also a couple of easter egg type functions:

    nwm.tween(window_id);
    nwm.stop(); // stop tweens
    nwm.random();
    nwm.globalSmall(); // set all to 200x200

# Writing your own layout engine

You should bind to the following events from the native extension:

- onAdd(callback). Callback is called with a window object when nwm detects a new window is added. You should store the window object somewhere in JS so you can calculate whatever layout you want.
- onRemove(callback). Callback is called with a window id when a window is unmapped or destroyed. When received, you should get rid of the window in your layout engine since the window is gone.

- onRearrange(callback). Called without arguments when windows need to be rearranged - e.g. after all the startup scan of windows is done.
- onButtonPress(callback). Called with an event. Event.button is the mouse button and x,y are the coordinates. 
- onKeyPress(callback). Placeholder for key events, which are not supported yet.

See nwm.js for a full example.

