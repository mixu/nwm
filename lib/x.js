module.exports = {
  /* Key masks. Used as modifiers to GrabButton and GrabKey, results of QueryPointer,
     state in various key-, mouse-, and button-related events. */

  ShiftMask: (1 << 0),
  LockMask: (1 << 1),
  ControlMask: (1 << 2),
  Mod1Mask: (1 << 3),
  Mod2Mask: (1 << 4),
  Mod3Mask: (1 << 5),
  Mod4Mask: (1 << 6),
  Mod5Mask: (1 << 7),

/* button names. Used as arguments to GrabButton and as detail in ButtonPress
   and ButtonRelease events.  Not to be confused with button masks above.
   Note that 0 is already defined above as "AnyButton".  */
  Button1: 1,
  Button2: 2,
  Button3: 3, // right click
  Button4: 4,
  Button5: 5,

  AnyModifier: (1 << 15)  /* used in GrabButton, GrabKey */

};
