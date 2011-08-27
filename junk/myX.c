/*-----------------------------------------*/
/* myX.c X11 interface routines            */
/* v 1.0           */
/* xphased by Thomas P. Witelski 1995/1996 */
/* witelski@math.duke.edu                  */
/* This software is provided "as is" without express or implied warranty. */
/*-----------------------------------------*/
#include "myX.h"
#include<X11/cursorfont.h>
#include <stdio.h>

/*--------------------------------*/
/*----------------------*/
/*----------------------*/

myXwin *myXopen(char *name,double dwide,double dhigh,double wx,double wy)
{
int i;
char color_vals[7][10]={"white","black","red","green",
                      "blue","magenta","cyan"};
  myXwin *W;
  unsigned int swidth,sheight;
  unsigned int wide,high;
  unsigned int wxx,wyy;
  /*----------------------*/
  char       *fontName;   /* Name of font for string */
  XGCValues   gcv;    /* Struct for creating GC */
  XSizeHints  hints;    /* Size hints for window manager */


  XColor xcolor;
  Cursor cursor;

/*----------------------*/
/*******************************/
if((W=(myXwin *)calloc(1,sizeof(myXwin)))==NULL)
  {fprintf(stderr,"Memory allocation error\n");exit(1);}
if((W->display = XOpenDisplay(NULL))==NULL) 
  {fprintf(stderr, "Cant open display\n"); exit(1);}
if ((fontName = XGetDefault(W->display, "", "font")) == NULL) 
fontName = "fixed"; /* load the X-windows text FONT */
if ((W->fontstruct = XLoadQueryFont(W->display, fontName)) == NULL) 
{ fprintf(stderr, "Cant get font\n"); exit(1); } 
/*******************************/

  swidth=DisplayWidth(W->display,DefaultScreen(W->display));
  sheight=DisplayHeight(W->display,DefaultScreen(W->display));
  W->white=WhitePixel(W->display,DefaultScreen(W->display));
  W->black=BlackPixel(W->display,DefaultScreen(W->display));

  wide=swidth*dwide;
  high=sheight*dhigh;
  wxx=swidth*wx;
  wyy=sheight*wy;

  hints.flags = (USPosition | PSize | PMinSize | PMaxSize );
  hints.height=high;
  hints.width=wide;
  hints.base_height=high;
  hints.base_width=wide;
  hints.min_height=high;
  hints.min_width=wide;
  hints.max_height=high;
  hints.max_width=wide;
  hints.x=wxx;
  hints.y=wyy;


/*----------------------------------------------------------*/

  strcpy(W->name,name);

  W->width=wide;
  W->height=high;
/*----------------------------------------------------------*/
/*----------------------------------------------------------*/
W->win = XCreateSimpleWindow(W->display, RootWindow(W->display, 0),
     wxx,wyy,W->width,W->height,1,W->black,W->white); 
/*--------color graphics stuff---------*/
      W->cmap=DefaultColormap(W->display,DefaultScreen(W->display));
      for(i=0;i<7;++i)
      {
              XParseColor(W->display,W->cmap,color_vals[i],&xcolor);
              XAllocColor(W->display,W->cmap,&xcolor);
              W->colors[i]=xcolor.pixel;
      }
/*--------color graphics stuff---------*/

/*----------------------------------------------------------*/
  XSetNormalHints(W->display,W->win,&hints);
  XStoreName(W->display,W->win,name);

XSynchronize(W->display,True);



/*----------------------------------------------------------*/
if(W->gc)
{
  XFreeGC(W->display,W->gc);
  XFreePixmap(W->display,W->pix);
}
W->pix=XCreatePixmap(W->display,W->win,W->width,
  W->height,DefaultDepth(W->display,DefaultScreen(W->display)));
W->gc = XCreateGC(W->display, W->pix, 0,(XGCValues *)0);
myXsetcolor(W,White);
/*
XSetBackground(W->display,W->gc,W->white);
*/
XFillRectangle(W->display,W->pix,W->gc,0,0,W->width,W->height);
myXsetcolor(W,Black);
/*----------------------------------------------------------*/
cursor=XCreateFontCursor(W->display,XC_crosshair);
XDefineCursor(W->display,W->win,cursor);
/*----------------------------------------------------------*/


/* setup input to the window */
XSelectInput(W->display, W->win, ExposureMask|KeyPressMask|ButtonPressMask
|ButtonReleaseMask|PointerMotionHintMask);
XMapRaised(W->display, W->win); /* make window visible */

XSetWindowBackgroundPixmap(W->display,W->win,W->pix);
/*****************************/
  myXsetcolor(W,Black);
  myXsetmode(W,0);
  myXsetline(W,1);
  myXupdate(W);
  myXclear(W);
  myXclear(W);
/*****************************/
  return(W);
}

int myXmouse(myXwin *W,double *x, double *y)
/* check mouse buttons and x,y pos, no wait loop */
/* returns 1,2,3= button, -1=motion, 0=nothing, (x,y)=mouse pos */
{
        XEvent event;
  double xx,yy;
  int x1,y1,x2,y2;
  Window child;
  Window root;
  unsigned int button;

XSelectInput(W->display, W->win, ExposureMask | KeyPressMask |
  PointerMotionMask| ButtonPressMask | StructureNotifyMask
  | SubstructureRedirectMask | VisibilityChangeMask );


XCheckWindowEvent(W->display,W->win,ButtonPressMask|ButtonReleaseMask|
PointerMotionMask,&event);

  if(event.type == ButtonPress)
  {
    *x=(double)(event.xbutton.x/(1.0*W->width));
    *y=(double)(1.0-event.xbutton.y/(1.0*W->height));
    return(event.xbutton.button);
  }
  else
  {
    if(XQueryPointer(W->display,W->win,&root,&child,&x1,&y1,
      &x2,&y2,&button)==False) return(MouseNOP);
    xx=(double)(x2/(1.0*W->width));
    yy=(double)(1.0-y2/(1.0*W->height));
    if(xx<0.0 || xx>1.0) return(MouseNOP);
    if(yy<0.0 || yy>1.0) return(MouseNOP);
    if(fabs(*x-xx)>0.0001 || fabs(*y-yy)>0.0001)
    {
      *x=xx;
      *y=yy;
      return(MouseMove);
    }
    return(MouseNOP);
  }

}

void myXclose(myXwin *W)
{
  XUnloadFont(W->display, W->fontstruct->fid);
  XFreeGC(W->display, W->gc);
  XCloseDisplay(W->display);
}
void myXupdate(myXwin *W)
{
  XSetWindowBackgroundPixmap(W->display,W->win,W->pix);
  XFlush(W->display);
  XFlush(W->display);
}
void myXerase(myXwin *W) /* erase only setmode=0 drawings */
{
  XSetWindowBackgroundPixmap(W->display,W->win,W->pix);
  XClearWindow(W->display,W->win);
  XFlush(W->display);
}
void myXpenup(myXwin *W)  /* stop drawing current line */
{
  W->drawflag=0;
}

void myXclear(myXwin *W) /* erase EVERYTHING */
{
  myXsetcolor(W,White);
  XFillRectangle(W->display,W->pix,W->gc,0,0,W->width,W->height);
  myXsetcolor(W,Black);
  myXpenup(W);
  XClearWindow(W->display, W->win);
  XMapRaised(W->display, W->win); /* make window visible */
}

void myXprint(myXwin *W,double x,double y,char *text)
{
  int xx,yy;
  xx=(int)(x*W->width);
  yy=(int)((1.0-y)*W->height);
  XDrawString(W->display,W->win,W->gc,xx,yy,text,strlen(text));
  if(W->writemode)
  XDrawString(W->display,W->pix,W->gc,xx,yy,text,strlen(text));
  myXupdate(W);
}


void myXline(myXwin *W,double x1,double y1,double x2,double y2)
{
  myXpenup(W);
  myXdrawto(W,x1,y1);
  myXdrawto(W,x2,y2);
}

void myXdrawto(myXwin *W,double x,double y)
{
  int xx,yy;
  xx=(int)(x*W->width);
  yy=(int)((1.0-y)*W->height);
  
if(W->drawflag)
{
  XDrawLine(W->display,W->win,W->gc,W->drawprvx,W->drawprvy,xx,yy);
  if(W->writemode)
  XDrawLine(W->display,W->pix,W->gc,W->drawprvx,W->drawprvy,xx,yy);
}
  W->drawflag=1;
  W->drawprvx=xx;
  W->drawprvy=yy;
  myXupdate(W);
}

void myXpoint(myXwin *W,double x,double y)
{
  int xx,yy;
  xx=(int)(x*W->width);
  yy=(int)((1.0-y)*W->height);
  
  XDrawPoint(W->display,W->win,W->gc,xx,yy);
  if(W->writemode)
  XDrawPoint(W->display,W->pix,W->gc,xx,yy);
  W->drawflag=1;
  W->drawprvx=xx;
  W->drawprvy=yy;
  myXupdate(W);
}

void myXsetline(myXwin *W,int line) /* 1=solid, 0=dashed */
{
  XSetLineAttributes(W->display,W->gc,1,
    (line==1 ? LineSolid:LineOnOffDash),CapRound,JoinRound);
}

void myXsetcolor(myXwin *W,int color)
{
  XSetForeground(W->display,W->gc,W->colors[color]);
}

void myXsetmode(myXwin *W,int mode) /* 0=eraseable, 1=permanent */
{
  W->writemode=mode;
}


void myXdump(myXwin *W,char *filename)
{
      char xcmd[200];
      sprintf(xcmd,"xwd -nobdrs -name \"%s\" %s %s",
      W->name,"| xpr -portrait -compact -device ps -output ",filename);
      system(xcmd);
}

/*=====================================*/
/*=====================================*/
/*=====================================*/
/*=====================================*/
/*=====================================*/

