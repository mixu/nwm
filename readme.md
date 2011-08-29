# nwm - node window manager

nwm is a dynamic window manager for X written at NodeKO 2011. 

It uses libev to interface with X11, and allows you to lay out windows in Node.

I wrote the code in this repo during Node Knockout 2011 ([old repo](https://github.com/mixu/nodeko)). Unfortunately I'm not in the competition since I forgot to confirm my initial team registration (click accept). Not a big deal, my reward is shipping.

**This is the repo you should watch/fork for future updates. Updates coming, need rest now!**

The underlying X11 bindings are written as a Node native extension in c. I apologize for the ugliness of the code - if you look at the first commit, I started on Friday with [this tutorial](https://www.cloudkick.com/blog/2010/aug/23/writing-nodejs-native-extensions/), a vague knowledge of X11 and a lot of enthusiasm.

My plan is to dogfood this in the near future to bring it to a more useful state. Right now, I recommend you run it in a secondary X server using Xephyr.

# Features

nwm supports all kinds of layouts - you can write your own, or use the default (tiling) layout. It's like dwm, but layouts are enforced in JS rather than in C.

The default nwm.js starts a REPL, so you can issue commands to it interactively.

# Installing

    node-waf configure build
    # start Xephyr
    Xephyr -screen 1024x768 -br :1 &
    # export gedit to the X server on display 1
    DISPLAY=:1 gedit
    DISPLAY=:1 gnome-terminal
    # now start nwm.js on display 1
    DISPLAY=:1 node nwm.js


# Using

The default nwm.js starts a REPL, so you can issue commands to it interactively:


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

