# Todo (C++ bindings)

Todo:

- Ignore popup windows (C)
- Expose stacking order and stacking operations (C)
- Display full info of PropertyNotify, ClientMessage, ConfigureRequest and ConfigureNotify as nwm should not honor some requests e.g. guake Ctrl+shift+t.
- Test reloading key bindings on the fly (C)
- Customizable mouse key bindings (C)
- Graceful exit support (C)

# Todo (Nwm.js)

- Multi-monitor bug fixes:
  - Maximizing a window on a secondary monitor causes it to maximize in the primary monitor instead (JS)
  - Removing a monitor causes windows on that monitor to be inaccessible rather than being moved to the remaining monitor (JS)
  - Transient windows which are too large to fit the current monitor should be resized (JS)
  - Transient windows should be repositioned to the current screen when they open (JS)
- TCP interface (JS)
- Test with conky and dzen, figure out how to make integration w/those easy (JS)
- Website and tutorial e.g. http://xmonad.org/tour.html
- Saving state on exit (JS)
- Floating window mode (JS)


# Done (C++ bindings)

- Monitor dimension change notifications (DONE)
- Expose window titles to JS (DONE)
- Support switch to full screen requests (DONE)
- Expose window classes to JS (DONE)
- Ignore numlock (DONE)
- Ignore transient windows (DONE)
- Multi-monitor support (DONE)

# Done (nwm.js)

- More layouts: (DONE)
    - Wide (upper half for main, lower half for others)
    - Monocle (full screen)
    - Grid (close to equal size)
- Shortcut for switching between layouts (DONE)
- Per-workspace layouts (DONE)
- Keyboard shortcut for making the currently focused window the main window (DONE)
- Resize main window area with shortcut (DONE)
- Newly mapped window should become the main window (DONE)
- Support for loading configuration files (DONE)
    - Ability to customize keyboard shortcuts from conf file
    - Add new key bindings (e.g. to launch apps or change layouting) from conf file
    - git clone + npm installation of personalized config
    - Code hot loading from file
- Setting main focus should move that window to the first window in grid layout (DONE)
- Multi-monitor bug fixes:
    - GetMainWindow() should take into account whether the window is on the same monitor as the workspace (JS)
    - Rearrange should apply to all windows (e.g. when a new window is mapped it is put at 0,0 but is associated with the focused monitor in C) (DONE)
    - Window termination relies on nwm.focused_window which is deprecated (JS)
    - Need a key to reassign a window to a different monitor (JS)
    - The window_ids set of Monitors is not updated when a window's monitor_id is changed (JS)
    - Focusing on a monitor requires that there is at least one window on the screen, as monitors.current is updated based on window info (JS)
    - After swapping to a new workspace on screen 0, creating new terms causes them to go to workspace that was last active rather than the current workspace -- e.g. if last oper was to swap screen 1 to ws 3, ws 3 will be used; if previous ws was screen 0 ws 1, then ws 1 will be used (JS)
    - When a pre-existing window is shown again, it gets visible but is not assigned to the current workspace (JS)
- Transients still need to get focus on mouseEnter for transient dialogs with text entry to work (DONE)



