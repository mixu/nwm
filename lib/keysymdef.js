module.exports.masks = {
  /* Key masks. Used as modifiers to GrabButton and GrabKey, results of QueryPointer,
     state in various key-, mouse-, and button-related events. */

  ShiftMask:   (1<<0),
  LockMask:    (1<<1),
  ControlMask: (1<<2),
  Mod1Mask:    (1<<3),
  Mod2Mask:    (1<<4),
  Mod3Mask:    (1<<5),
  Mod4Mask:    (1<<6),
  Mod5Mask:    (1<<7),

/* button names. Used as arguments to GrabButton and as detail in ButtonPress
   and ButtonRelease events.  Not to be confused with button masks above.
   Note that 0 is already defined above as "AnyButton".  */
  Button1: 1,
  Button2: 2,
  Button3: 3, // right click
  Button4: 4,
  Button5: 5,

  AnyModifier: (1<<15)  /* used in GrabButton, GrabKey */

};

module.exports.keysyms = {
  VoidSymbol: 0xFFFFFF, /* Void symbol */
  BackSpace: 0xFF08, /* Back space, back char */
  Tab: 0xFF09,
  Linefeed: 0xFF0A, /* Linefeed, LF */
  Clear: 0xFF0B,
  Return: 0xFF0D, /* Return, enter */
  Pause: 0xFF13, /* Pause, hold */
  Scroll_Lock: 0xFF14,
  Sys_Req: 0xFF15,
  Escape: 0xFF1B,
  Delete: 0xFFFF, /* Delete, rubout */
  /* Cursor control & motion */
  Home: 0xFF50,
  Left: 0xFF51,
  Up: 0xFF52,
  Right: 0xFF53,
  Down: 0xFF54,
  Prior: 0xFF55,
  Page_Up: 0xFF55,
  Next: 0xFF56,
  Page_Down: 0xFF56,
  End: 0xFF57,
  Begin: 0xFF58,
  /* Misc functions */
  Select: 0xFF60,
  Print: 0xFF61,
  Execute: 0xFF62,
  Insert: 0xFF63,
  Undo: 0xFF65,
  Redo: 0xFF66,
  Menu: 0xFF67,
  Find: 0xFF68,
  Cancel: 0xFF69,
  Help: 0xFF6A,
  Break: 0xFF6B,
  Mode_switch: 0xFF7E,
  script_switch: 0xFF7E,
  Num_Lock: 0xFF7F,
  /*
   * Keypad functions, keypad numbers cleverly chosen to map to ASCII
   * YOU PROBABLY WANT THE NON-KEYPAD values..
   */
  KP_Space: 0xFF80,
  KP_Tab: 0xFF89,
  KP_Enter: 0xFF8D,
  KP_F1: 0xFF91,
  KP_F2: 0xFF92,
  KP_F3: 0xFF93,
  KP_F4: 0xFF94,
  KP_Home: 0xFF95,
  KP_Left: 0xFF96,
  KP_Up: 0xFF97,
  KP_Right: 0xFF98,
  KP_Down: 0xFF99,
  KP_Prior: 0xFF9A,
  KP_Page_Up: 0xFF9A,
  KP_Next: 0xFF9B,
  KP_Page_Down: 0xFF9B,
  KP_End: 0xFF9C,
  KP_Begin: 0xFF9D,
  KP_Insert: 0xFF9E,
  KP_Delete: 0xFF9F,
  KP_Equal: 0xFFBD,
  KP_Multiply: 0xFFAA,
  KP_Add: 0xFFAB,
  KP_Separator: 0xFFAC,
  KP_Subtract: 0xFFAD,
  KP_Decimal: 0xFFAE,
  KP_Divide: 0xFFAF,

  KP_0: 0xFFB0,
  KP_1: 0xFFB1,
  KP_2: 0xFFB2,
  KP_3: 0xFFB3,
  KP_4: 0xFFB4,
  KP_5: 0xFFB5,
  KP_6: 0xFFB6,
  KP_7: 0xFFB7,
  KP_8: 0xFFB8,
  KP_9: 0xFFB9,

  /*
   * Auxiliary functions; note the duplicate definitions for left and right
   * function keys;  Sun keyboards and a few other manufacturers have such
   * function key groups on the left and/or right sides of the keyboard.
   * We've not found a keyboard with more than 35 function keys total.
   */
  F1: 0xFFBE,
  F2: 0xFFBF,
  F3: 0xFFC0,
  F4: 0xFFC1,
  F5: 0xFFC2,
  F6: 0xFFC3,
  F7: 0xFFC4,
  F8: 0xFFC5,
  F9: 0xFFC6,
  F10: 0xFFC7,
  F11: 0xFFC8,
  L1: 0xFFC8,
  F12: 0xFFC9,
  L2: 0xFFC9,
  F13: 0xFFCA,
  L3: 0xFFCA,
  F14: 0xFFCB,
  L4: 0xFFCB,
  F15: 0xFFCC,
  L5: 0xFFCC,
  F16: 0xFFCD,
  L6: 0xFFCD,
  F17: 0xFFCE,
  L7: 0xFFCE,
  F18: 0xFFCF,
  L8: 0xFFCF,
  F19: 0xFFD0,
  L9: 0xFFD0,
  F20: 0xFFD1,
  L10: 0xFFD1,
  F21: 0xFFD2,
  R1: 0xFFD2,
  F22: 0xFFD3,
  R2: 0xFFD3,
  F23: 0xFFD4,
  R3: 0xFFD4,
  F24: 0xFFD5,
  R4: 0xFFD5,
  F25: 0xFFD6,
  R5: 0xFFD6,
  F26: 0xFFD7,
  R6: 0xFFD7,
  F27: 0xFFD8,
  R7: 0xFFD8,
  F28: 0xFFD9,
  R8: 0xFFD9,
  F29: 0xFFDA,
  R9: 0xFFDA,
  F30: 0xFFDB,
  R10: 0xFFDB,
  F31: 0xFFDC,
  R11: 0xFFDC,
  F32: 0xFFDD,
  R12: 0xFFDD,
  F33: 0xFFDE,
  R13: 0xFFDE,
  F34: 0xFFDF,
  R14: 0xFFDF,
  F35: 0xFFE0,
  R15: 0xFFE0,
/* Modifiers */

  Shift_L: 0xFFE1, /* Left shift */
  Shift_R: 0xFFE2, /* Right shift */
  Control_L: 0xFFE3, /* Left control */
  Control_R: 0xFFE4, /* Right control */
  Caps_Lock: 0xFFE5, /* Caps lock */
  Shift_Lock: 0xFFE6, /* Shift lock */

  Meta_L: 0xFFE7, /* Left meta */
  Meta_R: 0xFFE8, /* Right meta */
  Alt_L: 0xFFE9, /* Left alt */
  Alt_R: 0xFFEA, /* Right alt */
  Super_L: 0xFFEB, /* Left super */
  Super_R: 0xFFEC, /* Right super */
  Hyper_L: 0xFFED, /* Left hyper */
  Hyper_R: 0xFFEE, /* Right hyper */

/*
 * Latin 1
 * (ISO/IEC 8859-1 = Unicode U+0020..U+00FF)
 * Byte 3 = 0
 */
    space: 0x020,
    exclam: 0x021,
    quotedbl: 0x022,
    numbersign: 0x023,
    dollar: 0x024,
    percent: 0x025,
    ampersand: 0x026,
    apostrophe: 0x027,
    quoteright: 0x027,
    parenleft: 0x028,
    parenright: 0x029,
    asterisk: 0x02a,
    plus: 0x02b,
    comma: 0x02c,
    minus: 0x02d,
    period: 0x02e,
    slash: 0x02f,
    0: 0x030,
    1: 0x031,
    2: 0x032,
    3: 0x033,
    4: 0x034,
    5: 0x035,
    6: 0x036,
    7: 0x037,
    8: 0x038,
    9: 0x039,
    colon: 0x03a,
    semicolon: 0x03b,
    less: 0x03c,
    equal: 0x03d,
    greater: 0x03e,
    question: 0x03f,
    at: 0x040,
    A: 0x041,
    B: 0x042,
    C: 0x043,
    D: 0x044,
    E: 0x045,
    F: 0x046,
    G: 0x047,
    H: 0x048,
    I: 0x049,
    J: 0x04a,
    K: 0x04b,
    L: 0x04c,
    M: 0x04d,
    N: 0x04e,
    O: 0x04f,
    P: 0x050,
    Q: 0x051,
    R: 0x052,
    S: 0x053,
    T: 0x054,
    U: 0x055,
    V: 0x056,
    W: 0x057,
    X: 0x058,
    Y: 0x059,
    Z: 0x05a,
    bracketleft: 0x05b,
    backslash: 0x05c,
    bracketright: 0x05d,
    asciicircum: 0x05e,
    underscore: 0x05f,
    grave: 0x060,
    quoteleft: 0x060,
    a: 0x061,
    b: 0x062,
    c: 0x063,
    d: 0x064,
    e: 0x065,
    f: 0x066,
    g: 0x067,
    h: 0x068,
    i: 0x069,
    j: 0x06a,
    k: 0x06b,
    l: 0x06c,
    m: 0x06d,
    n: 0x06e,
    o: 0x06f,
    p: 0x070,
    q: 0x071,
    r: 0x072,
    s: 0x073,
    t: 0x074,
    u: 0x075,
    v: 0x076,
    w: 0x077,
    x: 0x078,
    y: 0x079,
    z: 0x07a,
    braceleft: 0x07b,
    bar: 0x07c,
    braceright: 0x07d,
    asciitilde: 0x07e
};

