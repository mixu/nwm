static const char *event_names[] = { "", "", "KeyPress", "KeyRelease",
  "ButtonPress", "ButtonRelease", "MotionNotify", "EnterNotify",
  "LeaveNotify", "FocusIn", "FocusOut", "KeymapNotify",
  "Expose", "GraphicsExpose", "NoExpose", "VisibilityNotify",
  "CreateNotify", "DestroyNotify", "UnmapNotify",
  "MapNotify", "MapRequest", "ReparentNotify", "ConfigureNotify",
  "ConfigureRequest", "GravityNotify", "ResizeRequest",
  "CirculateNotify", "CirculateRequest", "PropertyNotify",
  "SelectionClear", "SelectionRequest", "SelectionNotify",
  "ColormapNotify", "ClientMessage", "MappingNotify"
};

static Bool isuniquegeom(XineramaScreenInfo *unique, size_t len, XineramaScreenInfo *info) {
  unsigned int i;

  for(i = 0; i < len; i++)
    if(unique[i].x_org == info->x_org && unique[i].y_org == info->y_org
    && unique[i].width == info->width && unique[i].height == info->height)
      return False;
  return True;
}

int xerror(Display *dpy, XErrorEvent *ee) {
  if(ee->error_code == BadWindow
  || (ee->request_code == X_SetInputFocus && ee->error_code == BadMatch)
  || (ee->request_code == X_PolyText8 && ee->error_code == BadDrawable)
  || (ee->request_code == X_PolyFillRectangle && ee->error_code == BadDrawable)
  || (ee->request_code == X_PolySegment && ee->error_code == BadDrawable)
  || (ee->request_code == X_ConfigureWindow && ee->error_code == BadMatch)
  || (ee->request_code == X_GrabButton && ee->error_code == BadAccess)
  || (ee->request_code == X_GrabKey && ee->error_code == BadAccess)
  || (ee->request_code == X_CopyArea && ee->error_code == BadDrawable))
    return 0;
  fprintf(stdout, "nwm: fatal error: request code=%d, error code=%d\n",
      ee->request_code, ee->error_code);
//    return xerrorxlib(dpy, ee); /* may call exit */
  exit(-1);
  return 0;
}

int xerrordummy(Display *dpy, XErrorEvent *ee) {
  return 0;
}

Bool gettextprop(Display* dpy, Window w, Atom atom, char *text, unsigned int size) {
  char **list = NULL;
  int n;
  XTextProperty name;

  if(!text || size == 0)
    return False;
  text[0] = '\0';
  XGetTextProperty(dpy, w, &name, atom);
  if(!name.nitems)
    return False;
  if(name.encoding == XA_STRING)
    strncpy(text, (char *)name.value, size - 1);
  else {
    if(XmbTextPropertyToTextList(dpy, &name, &list, &n) >= Success && n > 0 && *list) {
      strncpy(text, *list, size - 1);
      XFreeStringList(list);
    }
  }
  text[size - 1] = '\0';
  XFree(name.value);
  return True;
}

int updatenumlockmask(Display* dpy) {
  unsigned int i;
  int j;
  XModifierKeymap *modmap;

  int numlockmask = 0;
//  this->numlockmask = 0;
  modmap = XGetModifierMapping(dpy);
  for(i = 0; i < 8; i++)
    for(j = 0; j < modmap->max_keypermod; j++)
      if(modmap->modifiermap[i * modmap->max_keypermod + j] == XKeysymToKeycode(dpy, XK_Num_Lock))
//        this->numlockmask = (1 << i);
          numlockmask = (1 << i);
  XFreeModifiermap(modmap);
  return numlockmask;
}

static Bool isprotodel(Display* dpy, Window win) {
  int i, n;
  Atom *protocols;
  Bool ret = False;

  if(XGetWMProtocols(dpy, win, &protocols, &n)) {
    for(i = 0; !ret && i < n; i++)
      if(protocols[i] == XInternAtom(dpy, "WM_DELETE_WINDOW", False))
        ret = True;
    XFree(protocols);
  }
  return ret;
}

static Bool SendEvent(Display* dpy, Window wnd, Atom proto) {
  int n;
  Atom *protocols;
  Bool exists = False;
  XEvent ev;

  if(XGetWMProtocols(dpy, wnd, &protocols, &n)) {
    while(!exists && n--)
      exists = protocols[n] == proto;
    XFree(protocols);
  }
  if(exists) {
    ev.type = ClientMessage;
    ev.xclient.window = wnd;
    Atom atom = XInternAtom(dpy, "WM_PROTOCOLS", False);
    ev.xclient.message_type = atom;
    ev.xclient.format = 32;
    ev.xclient.data.l[0] = proto;
    ev.xclient.data.l[1] = CurrentTime;
    XSendEvent(dpy, wnd, False, NoEventMask, &ev);
  }
  return exists;
}

Bool getrootptr(Display* dpy, Window root, int *x, int *y) {
  int di;
  unsigned int dui;
  Window dummy;

  return XQueryPointer(dpy, root, &dummy, &dummy, x, y, &di, &di, &dui);
}

void grabButtons(Window wnd, Bool focused) {
  return; // TODO TODO
  // updatenumlockmask();
//    {
//      unsigned int i;
//      unsigned int modifiers[] = { 0, LockMask, this->numlockmask, this->numlockmask|LockMask };
//      XUngrabButton(this->dpy, AnyButton, AnyModifier, wnd);
// If focused, then we only grab the modifier keys. Otherwise, we grab all buttons..
//      if(focused) {
//        fprintf( stdout, "GRABBUTTONS - focused: true\n");
//          for(i = 0; i < 4; i++) {
//            XGrabButton(dpy, Button1,
//                              Mod4Mask|ControlMask|modifiers[i],
//                              wnd, False, (ButtonPressMask|ButtonReleaseMask),
//                              GrabModeAsync, GrabModeSync, None, None);
//          }
//      } else {
//        fprintf( stdout, "GRABBUTTONS - focused: false\n");
//        XGrabButton(this->dpy, AnyButton, AnyModifier, wnd, False,
//                    (ButtonPressMask|ButtonReleaseMask), GrabModeAsync, GrabModeSync, None, None);
//      }
//    }
}
