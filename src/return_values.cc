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
