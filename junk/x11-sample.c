#include <X11/Xlib.h>
#include <assert.h>   // I include this to test return values the lazy way
#include <unistd.h>   // So we got the profile for 10 seconds
#define NIL (0)       // A name for the void pointer

#define WIDTH 800
#define HEIGHT 600

int main()
{
   Display *dpy;
   Window wnd;

   dpy = XOpenDisplay(NIL);

   int blackColor = BlackPixel(dpy, DefaultScreen(dpy));
   int whiteColor = WhitePixel(dpy, DefaultScreen(dpy));

   wnd = XCreateSimpleWindow(dpy,
                        DefaultRootWindow(dpy), 0, 0, 
               WIDTH, HEIGHT, 0, 
               blackColor, blackColor);
   // We want to get MapNotify events

   XSelectInput(dpy, wnd, StructureNotifyMask);
   XMapWindow(dpy, wnd);
   
   GC gc = XCreateGC(dpy, wnd, 0, NIL);
   XSetForeground(dpy, gc, whiteColor);

   
      // Wait for the MapNotify event

      for(;;) {
       XEvent e;
       XNextEvent(dpy, &e);
       if (e.type == MapNotify)
        break;
      }
      XDrawLine(dpy, wnd, gc, 10, 60, 180, 20);
   
   
   XFlush(dpy);
      sleep(10);
   return 0;
}