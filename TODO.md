# Todo (C++ bindings)

Todo:

- Expose stacking order and stacking operations (C)
- Display full info of PropertyNotify, ClientMessage, ConfigureRequest and ConfigureNotify as nwm should not honor some requests e.g. guake Ctrl+shift+t.
- Test reloading key bindings on the fly (C)
- Customizable mouse key bindings (C)

# Todo (Nwm.js)

Rethinking the JS collections:

Instead of having NWM own a canonical set of windows and monitors, have windows in an array on the nwm-user-sample side.

Do not track the current monitor in the window. Instead:
- Each monitor contains workspaces. Each workspace is always on a particular monitor.
- windows are added and removed from workspaces when they are moved.
- Windows should not have a monitor property
- Windows should not have a workspace property

Every item (monitor, window, layoutspace) should be accessed with a numeric index in an array.
Lazy initialization should not exist. Errors should be thrown for non-numeric accesses.

Events defined in terms of nwm.on(), eg:
- Monitor: nwm.on('add window'), nwm.on('change window monitor'),
 nwm.on('remove window'), nwm.on('before remove monitor') should be removed and transferred to the actual event callback.

Layouts and workspaces should be merged.

The layouts should be able to store their own data, such as lists of window ids.

The layouts should not be updated when windows are removed/added, but only when they are redrawn.

When the layouts are redrawn, they should be updated.

Updates should be exposed like this:

1) layoutspace sends its known windows
2) it gets a set of added windows (new ids) and a set of removed ids
3) it then updates whatever internal structures are used accordingly
4) then the render calculations take place

This allows direct updates as well, and means that the sanity check can be done once.

The monitor is responsible for knowing which windows have been assigned to which workspaces.
The layoutspace is responsible for maintaining it's own "main window", "window order" and so forth; as well as for implementing add() and remove() for sanity checking all it's internal structures.

The monitor keeps an object indexed by layout name, and keeps track of which layout is active. That particular layoutspace object then receives the notification.

Windows never change monitor, they just move from one layoutspace association to another.

Hiding() and showing() a window should only affect the current layoutspace. So if you stay on the same workspace but switch layout, then the window should be shown again.

 There are many cases where we just want to sanity check the window list:
 - when retrieving the main window for a workspace


- Multi-monitor bug fixes:
  - Transient windows which are too large to fit the current monitor should be resized (JS)
  - Transient windows should be repositioned to the current screen when they open (JS)
- Test with conky and dzen, figure out how to make integration w/those easy (JS)
- Website and tutorial e.g. http://xmonad.org/tour.html
- Saving state on exit (JS)
- Floating window mode for Flash fullscreen etc. (JS)

