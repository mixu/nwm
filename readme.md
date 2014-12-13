# nwm

A dynamic window manager for X11 written with Node.js

nwm is what I use for window management in Arch, Debian and Ubuntu.

### Why?

- Supported: Ubuntu, Arch, Fedora, Debian
- Dynamically tiling window manager with adjustable main window size
- Multiple monitor support (Xinerama)
- Workspaces/virtual desktops (0 - 9 by default)
- Layouts: vertical tiling, horizontal tiling, grid, fullscreen
- Each workspace can have its own layout
- Everything is defined/laid out in Javascript; a native X11 binding written in in C++/C emits events to Node.js

# Installing

Prerequisites: a 0.10.x/0.8.x/0.6.x branch version of Node and xterm (if not installed). Install the following dev packages:

- On Ubuntu (10.4) and Debian (6 stable): `sudo apt-get install libx11-dev libxinerama-dev`
- On Arch (after installing X11): `sudo pacman -S xterm libxinerama`; also, you need to [set python to be python2](http://stackoverflow.com/questions/15400985/how-to-completely-replace-python-3-with-python-2-in-arch-linux) for [gyp](http://en.wikipedia.org/wiki/GYP_%28software%29), the build tool that gets invoked by [node-gyp](https://github.com/TooTallNate/node-gyp).
- On Fedora: (need to update this)
- On OSX: nwm does unofficially run under X11 in OSX - see osx.md in the repo for instructions

From github:

    git clone git://github.com/mixu/nwm.git
    rm -rf ./build
    npm install --production

If you are using Node 0.6.x, you need to checkout src/nwm_node.cc at revision db3545413d:

    git clone git://github.com/mixu/nwm.git
    git checkout db3545413d src/nwm/nwm_node.cc
    rm -rf ./build
    npm install --production

This is because - sadly - the uv_poll_* functionality does not exist in the version of libuv bundled with node 0.6.x. But thanks to NOT writing the vast majority of the native binding in C++, this workaround will work for quite a while until you upgrade.

See further below for instructions on how to set up nwm as a desktop session under GDM/Gnome.

# Tutorial

Youtube: http://www.youtube.com/watch?v=sihgPfBj6yE (sorry for the crappy audio!)

## Launching programs

When you start nwm, you will be presented with an empty screen.

To start a terminal (xterm), press: ```Win + Shift + Enter```. nwm takes care of dynamically rearranging windows. Launch a second terminal by pressing ```Win + Shift + Enter``` again.

Other programs are launched from the console. For example: ```google-chrome &``` launches Google Chrome, if you have it installed.

## Switching between layouts

nwm comes bundled with a number of different layouts. Press: ```Win + Space``` to toggle between different layouts.

## Moving focus

Move the mouse on top of the window you want to focus. You can also use ```Win + j``` / ```Win + k``` if you don't like the mouse.

## Using workspaces

The way I use nwm is by dedicating workspaces to different tasks (e.g. different programming projects, managing files, terminal windows). Each monitor has 9 workspaces, numbered from 1 to 9. To change the workspace, press:

```Win + [number key between from 1 to 9]```

To move the focused window to a different workspace, press:

```Win + Shift + [number key between from 1 to 9]```

If you have more than one monitor attached, then you will have 9 workspaces on each monitor. You can change the layout on each workspace individually.

## Resizing the window

Sometimes, you want to make one of the windows larger or smaller than the others, like a terminal window next to a web browser window.

In nwm, the window on the right hand side is considered to be the main window.

Press: ```Win + H``` or ```Win + F10``` to shrink the main window.

Press: ```Win + L``` or ```Win + F11``` to grow the main window.

You can change the main window size setting for each workspace separately.

## Reordering windows

To set the focused window as the main window, use Win + Tab.

## Closing windows

To close the currently focused window, press: ```Win + shift + c```

## Using multiple monitors

This is basically just like using workspaces. You can move the currently focused window to a different screen by pressing: ```Win + Shift + ,``` and ```Win + Shift + .```

That's it.

# Keyboard shortcuts

    # Launching programs
    Meta + Shift + Enter -- Start xterm

    # Switching between layouts
    Meta + Space -- Switch between tile, monocle, wide and grid layouts

    # Focus
    Meta + j -- Focus next window
    Meta + k -- Focus previous window

    # Main window
    Meta + h -- Decrease master area size
    Meta + F10
    Meta + l -- Increase master area size
    Meta + F11
    Meta + Enter -- Sets currently focused window as main window

    # Closing windows
    Meta + Shift + c -- Close focused window

    # Workspaces
    Meta + [1..n] -- Switch to workspace n
    Meta + Shift + [1..n] -- Move window to workspace n
    Meta + BackSpace -- Switch back and forth between the last two workspaces

    # Multi-monitor keys
    Meta + Shift + , -- Send focused window to previous screen
    Meta + Shift + . -- Send focused window to next screen

# Adding nwm under your login manager as an alternative under Linux

If you are using Gnome (GDM as login manager)

1: Create nwm.sh somewhere (and chmod +x it):

    #!/bin/sh
    /usr/local/bin/node /path/to/nwm-user-sample.js 2> ~/nwm.err.log 1> ~/nwm.log

Note: run "which node" to find out the path to Node in the script above.

2: add the following as nwm.desktop to /usr/share/xsessions:

    [Desktop Entry]
    Encoding=UTF-8
    Name=nwm
    Comment=This session starts nwm
    Exec=/PATH/TO/nwm.sh
    Type=Application

Select "nwm" from the Sessions menu when logging in. If you run into issues, have a look at ~/nwm.err.log. Mostly, it's a matter of getting all the paths (to Node, to the nwm files) right.

## Changing keyboard shortcuts

To customize the keyboard shortcuts, change nwm-user-sample.js. Let's look at the keyboard shortcut for xterm in nwm-user-sample.js:

    {
      key: 'Return', // enter key launches xterm
      modifier: [ 'shift' ],
      callback: function(event) {
        child_process.spawn('xterm', ['-lc'], { env: process.env });
      }
    },

There are three parts to a basic shortcut:

- The key ('Return'). You can find the names of the keys in ./lib/keysyms.js.
- The modifier key ([ 'shift' ]). You can use shift or ctrl.
- The callback.

## Writing new layouts and reassigning keyboard shortcuts


For more extensive customization, see https://github.com/mixu/nwm-user which has a package.json file and hence makes it possible to git clone + npm install your window manager.

## Vertical Stack Tiling (e.g. DWM's tiling)

![screenshot](https://github.com/mixu/nwm/raw/master/docs/screenshots/tile.png)

## Bottom Stack Tiling (a.k.a. wide)

![screenshot](https://github.com/mixu/nwm/raw/master/docs/screenshots/wide.png)

## Grid (a.k.a fair)

![screenshot](https://github.com/mixu/nwm/raw/master/docs/screenshots/grid.png)

# Running under a secondary X11 server (Xephyr)

If you want to test or develop nwm, the easiest way is to use Xephyr:

    # start Xephyr
    Xephyr -screen 1024x768 -nodri -br :1 &
    # export gedit to the X server on display 1
    DISPLAY=:1 gedit
    DISPLAY=:1 gnome-terminal
    # now start nwm.js on display 1
    DISPLAY=:1 node nwm-user-sample.js

Under Xephyr, the base key combination is Ctrl+Meta (e.g. Ctrl+Win). When running natively, the base key is Meta (Win). This is so that I can test nwm inside itself, yet have decent shortcuts:

# Tips for running under a VM

Some tips for running nwm in a VM:

- If you use VMware Workstation, you have to start vmware-user manually for multi-monitor support via Xinerama after starting nwm.
- If you use VirtualBox, you have to use xrandr manually for multi-monitor support (e.g. xrandr --output VBOX0 --auto --left-of VBOX1).

VirtualBox sometimes gets your virtual screen sizes wrong. If this happens, you need to rerun xrandr, otherwise Xinerama reports the starting index of your second display incorrectly. You can see this by running xrandr:

    VBOX0 connected 1440x900+0+0 0mm x 0mm
    VBOX1 connected 2560x1440+2560+0 0mm x 0mm
                              !!!!

The display VBOX1 is marked as starting at x=2560 even though VBOX0 ends at 1440. This was because VirtualBox resized the VBOX0 screen incorrectly when you ran xrandr. This is a VirtualBox bug, not a nwm one.
