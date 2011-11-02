  static Local<Object> makeMouseDrag(Window id, int x, int y, int movex, int movey //, unsigned int state
  ) {
    // window object to return
    Local<Object> result = Object::New();

    // read and set the window geometry
    result->Set(String::NewSymbol("id"), Integer::New(id));
    result->Set(String::NewSymbol("x"), Integer::New(x));
    result->Set(String::NewSymbol("y"), Integer::New(y));
    result->Set(String::NewSymbol("move_x"), Integer::New(movex));
    result->Set(String::NewSymbol("move_y"), Integer::New(movey));
//    result->Set(String::NewSymbol("state"), Integer::New(state));
    return result;
  }


  static Local<Object> makeButtonPress(Window id, int x, int y, unsigned int button, unsigned int state) {
    // window object to return
    Local<Object> result = Object::New();

    // read and set the window geometry
    result->Set(String::NewSymbol("id"), Integer::New(id));
    result->Set(String::NewSymbol("x"), Integer::New(x));
    result->Set(String::NewSymbol("y"), Integer::New(y));
    result->Set(String::NewSymbol("button"), Integer::New(button));
    result->Set(String::NewSymbol("state"), Integer::New(state));
    return result;
  }

  static void EmitKeyPress(NodeWM* hw, XEvent *e) {
    KeySym keysym;
    XKeyEvent *ev;

    ev = &e->xkey;
    keysym = XKeycodeToKeysym(hw->dpy, (KeyCode)ev->keycode, 0);
    Local<Value> argv[1];
    // we always unset numlock and LockMask since those should not matter
    argv[0] = NodeWM::makeKeyPress(ev->x, ev->y, ev->keycode, keysym, (ev->state & ~(hw->numlockmask|LockMask)));
    // call the callback in Node.js, passing the window object...
    hw->Emit(onKeyPress, 1, argv);
  }

  static Local<Object> makeKeyPress(int x, int y, unsigned int keycode, KeySym keysym, unsigned int mod) {
    // window object to return
    Local<Object> result = Object::New();
    // read and set the window geometry
    result->Set(String::NewSymbol("x"), Integer::New(x));
    result->Set(String::NewSymbol("y"), Integer::New(y));
    result->Set(String::NewSymbol("keysym"), Integer::New(keysym));
    result->Set(String::NewSymbol("keycode"), Integer::New(keycode));
    result->Set(String::NewSymbol("modifier"), Integer::New(mod));
    return result;
  }

  static Local<Object> makeEvent(Window id) {
    // window object to return
    Local<Object> result = Object::New();
    result->Set(String::NewSymbol("id"), Integer::New(id));
    return result;
  }

