class Monitor {
  public:
    inline void setId(int id) { this->id = id; }
    inline void setWidth(int w) { this->width = w; }
    inline void setHeight(int h) { this->height = h; }
    inline void setX(int x) { this->x = x; }
    inline void setY(int y) { this->y = y; }
    inline int getWidth() { return this->width; }
    inline int getHeight() { return this->height; }
    inline int getX() { return this->x; }
    inline int getY() { return this->y; }
    std::vector <Client> clients;
  private:
    int id;
    int x, y, width, height;
};
