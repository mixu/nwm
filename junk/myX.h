#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/X.h>

/*-------------------------------*/
typedef struct myXwin {
/*----------------------*/
Display   *display;
Window    win;
GC    gc;
XFontStruct   *fontstruct;
Pixmap    pix;
unsigned long white,black;
Colormap  cmap;
unsigned long   colors[10];
char    name[500];
/*----------------------*/
unsigned int  width,height;
int   drawxpt,drawypt;
int   drawprvx,drawprvy;
int   drawflag;
int   writemode;
} myXwin;
/*-------------------------------*/
extern myXwin *myXopen();
/*--------------------------------*/
#define MouseLeft   1
#define MouseMiddle 2
#define MouseRight  3
#define MouseMove 0
#define MouseNOP  -1

#define White 0
#define Black 1
#define Red 2
#define Green 3
#define Blue  4
#define Magenta 5
#define Cyan  6

/*-------------------------------*/
myXwin *myXopen(char *name,double dwide,double dhigh,double wx,double wy);

int myXmouse(myXwin *W,double *x, double *y);
void myXclose(myXwin *W);
void myXupdate(myXwin *W);
void myXerase(myXwin *W);
void myXpenup(myXwin *W);
void myXclear(myXwin *W);
void myXprint(myXwin *W,double x,double y, char *text);
void myXline(myXwin *W,double x1,double y1,double x2,double y2);
void myXdrawto(myXwin *W,double x,double y);
void myXpoint(myXwin *W,double x,double y);
void myXsetline(myXwin *W,int line);
void myXsetcolor(myXwin *W,int color);
void myXsetmode(myXwin *W,int mode);

void myXdump(myXwin *W,char* filename);
/*-------------------------------*/
