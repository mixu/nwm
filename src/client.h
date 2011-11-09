
class Client {
public:
  Client(Window win, int x, int y, int width, int height, Bool isfloating) {
    fprintf( stderr, "Create client %li (x %d, y %d, w %d, h %d, float %d)\n", win, x, y, width, height, isfloating);
    this->win = win;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
    this->isfloating = isfloating;
  }
  inline Window getWin() { return this->win; }
  Local<Object> toNode() {
    // window object to return
    Local<Object> result = Object::New();

    // read and set the window geometry
    result->Set(String::NewSymbol("id"), Integer::New(this->win));
    result->Set(String::NewSymbol("x"), Integer::New(this->x));
    result->Set(String::NewSymbol("y"), Integer::New(this->y));
    result->Set(String::NewSymbol("height"), Integer::New(this->height));
    result->Set(String::NewSymbol("width"), Integer::New(this->width));
    result->Set(String::NewSymbol("isfloating"), Integer::New(this->isfloating));
    return result;
  }

  private:
    int x, y, width, height;
    Client *next;
    Client *snext;
    Window win;
    Bool isfloating;
};

