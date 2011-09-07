class Client {
public:
  Client(Window win, Monitor* monitor, int id, int x, int y, int width, int height) {
    this->win = win;
    this->mon = monitor;
    this->id = id;
    this->x = x;
    this->y = y;
    this->width = width;
    this->height = height;
  }
  static Client* getByWindow(Monitor* monit, Window win);
  static Client* getById(Monitor* monit, int id);
  inline int getId() { return this->id; }
  inline Window getWin() { return this->win; }
  void attach();
  void detach(); 
  void resize(Display* dpy, int width, int height) {
      fprintf( stderr, "ResizeWindow: id=%d width=%d height=%d \n", id, width, height);    
      XResizeWindow(dpy, this->win, width, height);    
      XFlush(dpy);    
  }
  void move(Display* dpy, int x, int y) {
      fprintf( stderr, "MoveWindow: id=%d x=%d y=%d \n", id, x, y);    
      XMoveWindow(dpy, this->win, x, y);    
      XFlush(dpy);
  }
  private:
    int id;
    int x, y, width, height;
    Client *next;
    Client *snext;
    Monitor *mon;
    Window win;
};

