
all: nwm.c

nwm.c:
#	gcc -g -std=c99 -pedantic -I/usr/include -DXINERAMA -c src/nwm.c -o nwm.o
	gcc -g -std=c99 -pedantic -Wall -I. -I/usr/include -I/usr/X11R6/include -DVERSION="5.9" -DXINERAMA -c src/nwm.c -o nwm.o
