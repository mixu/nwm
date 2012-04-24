# nwm - node window manager

nwm is a dynamic window manager for X written at NodeKO 2011. It uses libev to interface with X11, and allows you to lay out windows in Node. I started writing nwm at Node Knockout 2011 ([old repo](https://github.com/mixu/nodeko)). The underlying X11 bindings are written as a Node native extension in C++/C.

**This is the repo you should watch/fork for future updates.**

nwm is my primary window manager in Arch and Ubuntu.

**News**

Update (April 2012): Fixed a number of minor issues that I noticed over long term use, mostly related to mouse focus in popup/menu windows. (My last lingering issue turned out to be an issue with the Ubuntu Dust theme setting GtkMenu::*-padding to 0, which had bad interaction with Chromium; e.g. not related to nwm at all.)

Update (Jan 2012): I've recorded a video tutorial / walkthrough on Youtube (HD): http://www.youtube.com/watch?v=sihgPfBj6yE

Update (Dec 2011): Refactoring the codebase to pure C with a small C++ shim; Node 0.6.x compatibility; window colors (only on native X11 due to X server limitation [1](https://www.virtualbox.org/ticket/6479) and [2](https://bugs.launchpad.net/ubuntu/+source/xserver-xorg-video-vmware/+bug/312080)).

The next step is to simplify the JS binding further. I'm not quite satisfied with way monitors and workspaces are managed on the JS side, I think it can be  made simpler. Pull requests are welcome!

# Major features

- Layouts, key bindings, window positions, workspaces - all the major stuff is in Javascript, not C++ (or Haskell, or Lisp :D)
- 4 built-in layouts (**see screenshots below**)
- Support for multi-monitor systems (via Xinerama)
- Support for workspaces (0 - 9 by default), each workspace can have its own layout
- Support for a "main window", allowing for dynamic resizing. Each workspace has its own main window scale setting.
- C++ API abstracts over X11 (w/libev) so you don't need to learn X11 just to customize your layout
- REPL, so you can issue commands to nwm interactively, or expose and control it over TCP/HTTP/whatever

# Installing

You can use a 0.6.x branch, or a 0.4.x of Node. I'm generally using either 0.6.6 or 0.4.12. You should also install xterm via your package manager, since that's the default app launched from nwm.

NOTE: if you are switching from one Node version to another, I recommend deleting the ./build directory first (rm -rf ./build) before recompiling to avoid issues with node-waf.

From github:

    node-waf clean || true && node-waf configure build

You may need the libev-dev packages (e.g. "/usr/bin/ld: cannot find -lev"):

On Fedora: 

    sudo yum install libev-devel

On Ubuntu (10.4) and Debian (6 stable):

    sudo apt-get install libx11-dev libxinerama-dev

On Arch:

    sudo pacman -S xterm

On OSX: install XQuartz, then read the OSX specific instructions.

# Installing as a primary window manager under Linux

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

Select "nwm" from the Sessions menu when logging in.

Debugging: Have a look at ~/nwm.err.log. Mostly, it's a matter of getting all the paths right.

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


# Installing on OSX

Set nwm as the X11 window manager:

- Edit ~/.xinitrc to: "~/nwm/nwm.sh" (assuming that's where nwm is).
- Check/fix the paths in nwm.sh and start XQartz from Applications / Utilities.
- Check the settings under X11 / Preferences (these are just my recommendations)
  - Input:
    - [X] Emulate three button mouse
    - [ ] Follow system keyboard layout
    - [ ] Enable key equivalents under X11
    - [X] Option keys send Alt_L and Alt_R
  - Windows:
    - [ ] Click through inactive windows
    - [X] Focus follows mouse
    - [X] Focus on new windows

Fix the key bindings:

Unlike on other OS's, Apple has a ton of stuff bound to the Command (infinite loop thingy) key and removing those bindings is basically impossible (e.g. Command + Shift + q is a system key combination that cannot be altered). More discussion: http://www.emacswiki.org/emacs/MetaKeyProblems

To work around this, map something else to Mod4, the default modifier key used by nwm. You can either edit the baseModifier variable in nwm-user-sample.js, or:

Run xmodmap, which will show what physical keys are bound to which modifier keys. nwm uses Mod4 by default.

Edit ~/.xmodmap:

    clear Mod1
    clear Mod4
    keycode 66 = Alt_L
    keycode 69 = Alt_R
    add Mod1 = Alt_R
    add Mod4 = Alt_L

Here, I first cleared Mod1 (which by default had both Alt keys mapped to it, then changed Mod4 to left Alt and Mod1 to right Alt). You can run xev to interactively find out what keycodes are associated with what keys.

Finally, run

    xmodmap ~/.xmodmap

to change the keybindings (or close XQuartz and restart it). Alt + Shift + Enter now starts a new xterm instead of Meta + Shift + Enter - see the full keybindings further below.

Some extras (these just make the terminal a bit better):

~/.bashrc (color ls output, better prompt, import .profile):

    alias ls="ls -G"
    export PS1="[\w]$ "
    source ~/.profile
    cd ~

~/.Xresources (colors):

    XTerm*foreground: gray
    XTerm*background: black

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
