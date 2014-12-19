# Installing from github

To install nwm from github:

    git clone git://github.com/mixu/nwm.git
    rm -rf ./build
    npm install --production

# Installing on Node 0.6.x

If you are using Node 0.6.x, you need to checkout src/nwm_node.cc at revision db3545413d:

    git clone git://github.com/mixu/nwm.git
    git checkout db3545413d src/nwm/nwm_node.cc
    rm -rf ./build
    npm install --production

This is because - sadly - the uv_poll_* functionality does not exist in the version of libuv bundled with node 0.6.x. But thanks to NOT writing the vast majority of the native binding in C++, this workaround will work for quite a while until you upgrade.

# Installing on OSX

Note: There is no official support for nwm on OSX - nwm can run there but you're on your own for debugging this, as a lack of a sane package manager makes it hard to debug issues with missing dependencies.

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
