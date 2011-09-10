# nwm - node window manager

nwm is a dynamic window manager for X written at NodeKO 2011. 

It uses libev to interface with X11, and allows you to lay out windows in Node.

I wrote the code in this repo during Node Knockout 2011 ([old repo](https://github.com/mixu/nodeko)). Unfortunately I'm not in the competition since I forgot to confirm my initial team registration (click accept). Not a big deal.

**This is the repo you should watch/fork for future updates.**

The underlying X11 bindings are written as a Node native extension in C++/C. 

My plan is to dogfood this in the near future to bring it to a more useful state. Right now, I recommend you run it in a secondary X server using Xephyr.

# Features

Cpp bindings:

- Uses X11 via libev
- API abstracts over X11 so you don't need to learn X11 just to customize your layout

Nwm.js:

- Layout decisions are done in Javascript, not C++.
- Tiling window layout (like dwm): you can write your own in JS.
- Support for workspaces (0-9 via keyboard, max int via JS).
- REPL, so you can issue commands to it interactively

# Installing and running under a secondary X11 server (Xephyr)

    node-waf configure build
    # start Xephyr
    Xephyr -screen 1024x768 -br :1 &
    # export gedit to the X server on display 1
    DISPLAY=:1 gedit
    DISPLAY=:1 gnome-terminal
    # now start nwm.js on display 1
    DISPLAY=:1 node nwm.js

Some notes:

- Xephyr errors out under VirtualBox. You may need to start Xephyr with -nodri if you use VirtualBox with guest additions.
- ev.h: No such file or directory. You need the developer package for libev and specify its location explicitly (e.g. change #include <ev.h> to #include "path_to_ev.h"). 
    - The developer package is libev-devel on Fedora (yum install libev-devel)
    - The developer package is libev-dev on Ubuntu (apt-get install libev-dev)
- Other missing headers. Might be that you installed a version of node that didn't have the right headers for node-waf to work. Try reinstalling node. I am having trouble with the 0.5.x branches on Ubuntu, but v0.4.11 works (git checkout v0.4.11).
- You may have to Chrome start with --explicitly-allowed-ports=6000 for it to connect to some localhost ports.

# Using as a primary window manager

Find out what your login manager is:

    cat /etc/X11/default-display-manager

If it is GDM:

1: Create log directory and copy files

    sudo mkdir /nwm-log
    sudo chmod 777 /nwm-log
    sudo cp -R ./mnt/nwm/ /usr/local/lib/
    sudo chmod +x /usr/local/lib/nwm/nwm.sh

2: add the following as nwm.desktop to /usr/share/xsessions:

    [Desktop Entry]
    Encoding=UTF-8
    Name=nwm
    Comment=This session starts nwm 
    Exec=/usr/local/lib/nwm.sh
    Type=Application

Select "nwm" from the Sessions menu when logging in.

Ubuntu tips: https://help.ubuntu.com/community/CustomXSession

# Keyboard shortcuts

Keyboard shortcuts are now working. The binding is Ctrl+Win which happens to be free in my dev env.

    # Workspaces 
    Ctrl+Win 1 # Switch to workspace 1
    Ctrl+Shift+Win 1 # Move focused window to workspace 1
    ...
    Ctrl+Win 9 # Switch to workspace 9
    Ctrl+Shift+Win 9 # Move focused window to workspace 9
  
    # Launching xterm
    Ctrl+Win Enter # Start xterm

    # Closing window
    Ctrl+Win c # Close focused window

    # Switching between layouts
    Ctrl+Win Space # Switch between tile, monocle and wide layouts


# Using from the console

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

# Todo (C++ bindings)

Done:

- Monitor dimension change notifications (DONE)
- Expose window titles to JS (DONE)
- Support switch to full screen requests (DONE)
- Expose window classes to JS (DONE)
- Ignore numlock (DONE)

Todo:

- Test reloading key bindings on the fly (C)
- Customizable mouse key bindings (C)
- Multi-monitor support (C)
- Expose stacking order and stacking operations (C)
- Graceful exit support (C)

# Todo (Nwm.js)

Done:

- More layouts: (DONE)
    - Wide (upper half for main, lower half for others)
    - Monocle (full screen)
    - Grid (close to equal size)
- Shortcut for switching between layouts (DONE)
- Per-workspace layouts (DONE)

Todo:

- Resize main window area with shortcut (JS)
- Newly mapped window should become the main window (JS)
- Keyboard shortcut for making the currently focused window the main window (JS)
- Keyboard shortcuts for moving and resizing windows (JS)
- Support for loading configuration files (JS)
    - Ability to customize keyboard shortcuts from conf file
    - Add new key bindings (e.g. to launch apps or change layouting) from conf file
    - Code hot loading from conf file
    - git clone + npm installation of personalized config
- TCP or HTTP configuration interface (JS)
- Website and tutorial e.g. http://xmonad.org/tour.html
- Saving state on exit (JS)
- Media key bindings (JS)
- Dropdown Chrome (JS)
- Floating window mode (JS)

Long term plans (likely to be separate projects):

- Drawing window borders and titlebars
- App launcher
- App switcher
- Expose clone (e.g. show all windows, monocle selected window)
- Even more layouts e.g. http://haskell.org/haskellwiki/Xmonad/Screenshots
