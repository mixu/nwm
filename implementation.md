
# NWM implementation notes

## nwm.js

nwm is implemented over the native binding.

It binds event handlers to all the main events:

- addMonitor: a new monitor is added
- updateMonitor: an existing monitor is changed (e.g. resolution changes)
- removeMonitor: a monitor is removed (e.g. disconnect external monitor)
- addWindow: a new window is added
- updateWindow: the properties of the window change, like the window title
- removeWindow: the window is closed
- fullscreen: a window requests full screen mode
- configureRequest: a window wants to change it's size, stacking order or border width. Generally we only want to allow events that don't screw up the layout.
- mouseDown / mouseDrag: WIP mouse events
- enterNotify: mouse enters a new window
- rearrange: the native binding suggests a rearrange, generally because a monitor or window was removed
- keyPress: a key combination that we previously registered in the native binding was pressed

and makes changes to the associated items:

- monitors
- layoutspaces
- windows

# Monitors

The monitor is:

- responsible for knowing which windows have been assigned to which workspaces.
- responsible for moving windows to new workspaces
- responsible for triggering rearranges on the workspace

Monitors have:

- a width and height
- x and y coordinates (within the root X window)
- a list of workspaces and a list of windows for each workspace

# Layoutspaces / workspaces

Layoutspaces are a combination of workspace and layout.

It makes more sense to combine the two - every workspace has a layout, and more advanced layouts need to store stuff.

