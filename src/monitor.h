class Monitor {
  public:
    inline void setWidth(int w) { this->width = w; }
    inline void setHeight(int h) { this->height = h; }
    Client *clients;
  private:
    int id;
    int x, y, width, height;
  //  Client *sel;
  //  Client *stack;
    Monitor *next;
};
