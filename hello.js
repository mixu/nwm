var X11wm = require('./build/default/nwm.node').NodeWM;
var XK = require('./keysymdef.js');
var Xh = require('./x.js');

var obj = new X11wm();

console.log('enterNotify', obj.testing(
  [
      { key: XK.XK_1, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_2, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_3, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_4, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_5, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_6, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_7, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_8, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_9, modifier: Xh.Mod4Mask|Xh.ControlMask },
      { key: XK.XK_0, modifier: Xh.Mod4Mask|Xh.ControlMask },

      { key: XK.XK_1, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_2, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_3, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_4, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_5, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_6, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_7, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_8, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_9, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },
      { key: XK.XK_0, modifier: Xh.Mod4Mask|Xh.ControlMask|Xh.ShiftMask },

      { key: XK.XK_Return, modifier: Xh.Mod4Mask|Xh.ControlMask }
  ]
));
