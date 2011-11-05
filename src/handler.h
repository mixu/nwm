#ifndef _HANDLER_H_
#define _HANDLER_H_
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

#endif
