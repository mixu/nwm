# nwm

A dynamic window manager for X11 written with Node.js

nwm is what I use for window management in ChromeOS, Arch, Debian and Ubuntu.

### Why?

- **New in 1.3.x**: added support for Node `4.0.x` and `0.12.x`!
- Supported: Ubuntu, ChromeOS, Arch, Fedora, Debian
- Dynamically tiling window manager with adjustable main window size
- Multiple monitor support (Xinerama)
- Workspaces/virtual desktops (0 - 9 by default)
- Layouts: vertical tiling, horizontal tiling, grid, fullscreen
- Each workspace can have its own layout
- Everything is defined/laid out in Javascript; a native X11 binding written in in C++/C emits events to Node.js

*New in 1.1.0*: simplified the installation process by adding a `nwm` command, and simplified customization by introducing a new `~/.nwm-user` loading mechanism.

# Installation

Starting with `v1.3.0`, nwm works with the following Node versions: `4.0.x`, `0.12.x`, `0.10.x`, `0.8.x`. An old commit works with `0.6.x` as well, see [appendix.md](./appendix.md). For `4.0.x`, you may need a newer GCC version, see [this section for instructions](#compiling-under-node-40x).

Other prerequisites: `xterm` and `python` (for node-gyp). Also install the following dev packages:

- On Ubuntu (14.04/12.04/10.04) and Debian (6 stable): `sudo apt-get install libx11-dev libxinerama-dev`
- On ChromeOS, [set up developer mode](https://www.google.com/webhp?#q=chromeos+developer+mode) and then follow the [chromeos.md](/chromeos.md) guide for running my custom [crouton](https://github.com/dnschneid/crouton) deploy target
- On Arch (after installing X11): `sudo pacman -S xterm libxinerama`; also, you need to [set python to be python2](http://stackoverflow.com/questions/15400985/how-to-completely-replace-python-3-with-python-2-in-arch-linux) for [gyp](http://en.wikipedia.org/wiki/GYP_%28software%29), the build tool that gets invoked by [node-gyp](https://github.com/TooTallNate/node-gyp).
- On Fedora: (need to update this, please file a readme PR!)

Next, install nwm via npm with the `-g` flag:

    npm install -g nwm

This installs the `nwm` command globally, which can be then used to easily launch the window manager. If you want to install using git, see [appendix.md](./appendix.md) for more instructions.

Next, add an entry for nwm using `/usr/share/xsessions` (assuming you are using Gnome / GDM) as a login manager:

    nwm --init > /usr/share/xsessions/nwm.desktop

Select "nwm" from the Sessions menu when logging in.

## Customizing nwm

Starting with `v1.1.0`, when nwm is launched via the `nwm` command line tool, it will first look for a file or folder called
`~/.nwm-user` (e.g. `~/.nwm-user.js` or `~/.nwm-user/index.js`). This file allows you to customize your nwm keyboard shortcuts and overall behavior.

If this file is not found, then the default `nwm-user-sample.js` is used to launch the window manager.

Note that the new mechanism is different from the old one. The custom file should export a single function, which takes one parameter: the hash in nwm's `index.js`. This is done so that your custom configuration doesn't need to know where `nwm` is installed.

To get started, copy `nwm-user-sample.js` and customize it. It contains all the necessary boilerplate. You could also keep your custom config in a git repo, and clone it using something like `git clone https://github.com/mixu/nwm-user.git ~/.nwm-user && cd ~/.nwm-user && npm install`.

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

```
Win + [number key between from 1 to 9]
```

To move the focused window to a different workspace, press:

```
Win + Shift + [number key between from 1 to 9]
```

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

# Compiling under Node 4.0.x

To recompile nwm after switching Node version, make sure you install by cloning the repo, then run `rm -rf build && npm install` or reinstall via npm by uninstalling `npm uninstall -g nwm` and then reinstalling `npm install -g nwm`.

In order to compile nwm for Node `4.0.x`, you'll need a GCC version `~4.8`. Older distros - like Ubuntu 12.04 - ship with a GCC `~4.6` which will cause a wall of errors when compiling Node native modules. Here's a recipe for Ubuntu 12.04 (which also allows you to reset back using `update-alternatives --config gcc`:

```sh
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt-get update
sudo apt-get install gcc-4.8 g++-4.8
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 60 --slave /usr/bin/g++ g++ /usr/bin/g++-4.8
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.6 40 --slave /usr/bin/g++ g++ /usr/bin/g++-4.6
sudo update-alternatives --config gcc
```

# Running under a secondary X11 server (Xephyr)

If you want to test or develop nwm, the easiest way is to use Xephyr:

    # start Xephyr
    Xephyr -screen 1024x768 -nodri -br :1 &
    # export gedit to the X server on display 1
    DISPLAY=:1 gedit
    DISPLAY=:1 gnome-terminal
    # now start nwm.js on display 1
    DISPLAY=:1 node bin/nwm --xephyr

Under Xephyr, the base key combination is Ctrl+Meta (e.g. Ctrl+Win). When running natively, the base key is Meta (Win). This is so that I can test nwm inside itself, yet have decent shortcuts. Note the new `--xephyr` option which controls whether to use Ctrl+Meta or just Meta as the base key combination.

# Tips for running under a VM

Some tips for running nwm in a VM:

- If you use VMware Workstation, you have to start vmware-user manually for multi-monitor support via Xinerama after starting nwm.
- If you use VirtualBox, you have to use xrandr manually for multi-monitor support (e.g. xrandr --output VBOX0 --auto --left-of VBOX1).

VirtualBox sometimes gets your virtual screen sizes wrong. If this happens, you need to rerun xrandr, otherwise Xinerama reports the starting index of your second display incorrectly. You can see this by running xrandr:

    VBOX0 connected 1440x900+0+0 0mm x 0mm
    VBOX1 connected 2560x1440+2560+0 0mm x 0mm
                              !!!!

The display VBOX1 is marked as starting at x=2560 even though VBOX0 ends at 1440. This was because VirtualBox resized the VBOX0 screen incorrectly when you ran xrandr. This is a VirtualBox bug, not a nwm one.
