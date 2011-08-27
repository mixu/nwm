#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include"myX.h"


/* compile with cc myXdemo.c myX.c -lX11 -lm */

myXwin *window;

main()
{
  int i;
  srand48(2);

  window=myXopen("MyWindow",0.3,0.3,0.0,0.0);
  myXsetcolor(window,Blue);
  myXpoint(window,0.5,0.5);
  for(i=0;i<10;++i)
    myXdrawto(window,drand48(),drand48());
  myXsetcolor(window,Black);
  myXprint(window,0.5,0.5,"Hello world");
  while(1);
}