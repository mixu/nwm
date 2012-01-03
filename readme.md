# nwm - node window manager

nwm is a dynamic window manager for X written at NodeKO 2011. It uses libev to interface with X11, and allows you to lay out windows in Node. I started writing nwm at Node Knockout 2011 ([old repo](https://github.com/mixu/nodeko)), though it was not in the competition itself.

**This is the repo you should watch/fork for future updates.**

The underlying X11 bindings are written as a Node native extension in C++/C.

**News**

I am using nwm as my primary window manager daily, and I run it on Ubuntu, Fedora and Arch Linux (mostly Arch).

My next two goals for nwm are:

1) Simplifying the C++ binding. I have already been able to eliminate ~ 400 lines of code from the naive implementation by removing things that don't need to be tracked in the binding. For instance, monitors are only represented with a single integer in the C++ binding and all other information is passed and stored in the JS window manager. Next, I want to separate it into two parts: a ~700 line C library and a ~300 line C++ binding to that library. This will improve

2) Simplifying the JS binding.

3) OSX support. This will take a while, since I am doing the rewrite first. The problem is, I don't quite know enough about debugging libev to figure out what's going wrong on OSX.




I'm now actively using this as my primary window manager, as multi-monitor support is finally OK. You can give it a try as your primary as well, and report back bugs -- or nwm in a secondary X server using Xephyr. I'm looking for co-maintainers (e.g. interested in improving the nwm core, including the C++ stuff) and contributors (writing layouts and other JS code). Send pull requests :).

# Major features

- Layouts, key bindings, window positions, workspaces - all the major stuff is in Javascript, not C++ (or Haskell, or Lisp :D)
- 4 built-in layouts (**see screenshots below**)
- Support for multi-monitor systems (via Xinerama)
- Support for workspaces (0 - 9 by default), each workspace can have it's own layout
- Support for a "main window", allowing for dynamic resizing. Each workspace has it's own main window scale setting.
- C++ API abstracts over X11 (w/libev) so you don't need to learn X11 just to customize your layout
- REPL, so you can issue commands to nwm interactively, or expose and control it over TCP/HTTP/whatever

# Installing

You need to use a 0.4.x branch of Node for now, since the 0.5.x branch does not have libev bundled. For instance v0.4.12 works (git checkout v0.4.12 after cloning Joyent's node.js repo). You also want xterm, since that's the default app launched from nwm.

From github:

    node-waf clean || true && node-waf configure build

You may need the libev-dev packages (e.g. "/usr/bin/ld: cannot find -lev"):

- On Fedora: sudo yum install libev-devel
- On Ubuntu: apt-get install libev-dev libev3
- On Arch: pacman -S libev
- On OSX:
    - Install XQuartz
    - brew install libev
    - sudo brew link libev
    - Edit ~/.xinitrc to: "/usr/bin/login -fp $USER /usr/X11/bin/xterm". This will start X11 without a window manager.
    - Run node nwm-user-sample.js manually. You get the REPL, but no update events are triggered.
    - NOTE: Does not work yet on OSX. I'm trying to figure out what's going wrong. So far, it seems to be related to libev.

# Installing as a primary window manager

Find out what your login manager is:

    cat /etc/X11/default-display-manager

If it is GDM:

1: Create nwm.sh (and chmod +x it):

    #!/bin/sh
    /usr/local/bin/node /path/to/nwm-user-sample.js 2> ~/nwm.err.log 1> ~/nwm.log

2: add the following as nwm.desktop to /usr/share/xsessions:

    [Desktop Entry]
    Encoding=UTF-8
    Name=nwm
    Comment=This session starts nwm
    Exec=/PATH/TO/nwm.sh
    Type=Application

Select "nwm" from the Sessions menu when logging in.

Some tips for running nwm in a VM:

- If you use VMware Workstation, you have to start vmware-user manually for multi-monitor support via Xinerama after starting nwm.
- If you use VirtualBox, you have to use xrandr manually for multi-monitor support (e.g. xrandr --output VBOX0 --auto --left-of VBOX1).

VirtualBox sometimes gets your virtual screen sizes wrong. If this happens, you need to rerun xrandr, otherwise Xinerama reports the starting index of your second display incorrectly. You can see this by running xrandr:

    VBOX0 connected 1440x900+0+0 0mm x 0mm
    VBOX1 connected 2560x1440+2560+0 0mm x 0mm
                              !!!!

The display VBOX1 is marked as starting at x=2560 even though VBOX0 ends at 1440. This was because VirtualBox resized the VBOX0 screen incorrectly when you ran xrandr. This is a VirtualBox bug, not a nwm one.

# Running under a secondary X11 server (Xephyr)

    # start Xephyr
    Xephyr -screen 1024x768 -nodri -br :1 &
    # export gedit to the X server on display 1
    DISPLAY=:1 gedit
    DISPLAY=:1 gnome-terminal
    # now start nwm.js on display 1
    DISPLAY=:1 node nwm-user-sample.js

Some notes:

- Xephyr errors out under VirtualBox. You may need to start Xephyr with -nodri if you use VirtualBox with guest additions.
- You may have to Chrome start with --explicitly-allowed-ports=6000 for it to connect to some localhost ports.

# Keyboard shortcuts and layouts

The keyboard shortcuts use a different base key combination depending on whether you are running nwm on Xephyr / running nwm as the primary WM.

Under Xephyr, the base key combination is Ctrl+Meta (e.g. Ctrl+Win). When running natively, the base key is Meta (Win). This is so that I can test nwm inside itself, yet have decent shortcuts:

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

    # Multi-monitor keys
    Meta + Shift + , -- Send focused window to previous screen
    Meta + Shift + . -- Send focused window to next screen

## Writing new layouts and reassigning keyboard shortcuts

Have a look at the files in ./lib/layouts to see how a new layout can be implemented.

To customize the keyboard shortcuts, change nwm-user-sample.js.

For more extensive customization, see https://github.com/mixu/nwm-user which has a package.json file and hence makes it possible to git clone + npm install your window manager.

## Vertical Stack Tiling (e.g. DWM's tiling)

![screenshot](https://github.com/mixu/nwm/raw/master/docs/screenshots/tile.png)

## Monocle (a.k.a. fullscreen)

![screenshot](https://github.com/mixu/nwm/raw/master/docs/screenshots/fullscreen.png)

## Bottom Stack Tiling (a.k.a. wide)

![screenshot](https://github.com/mixu/nwm/raw/master/docs/screenshots/wide.png)

## Grid (a.k.a fair)

![screenshot](https://github.com/mixu/nwm/raw/master/docs/screenshots/grid.png)

# Using from the console

For now, the internal API is a bit messy, but you can use tab completion with the REPL to figure it out.
