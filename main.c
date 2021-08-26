/* See LICENSE file for license details. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#include <X11/Xutil.h>
#include <sl/arg.h>

static char *argv0;

static Display *dpy;
static Atom seltype = XA_PRIMARY ;

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-c|-s]\n", argv0);
	exit(1);
}

static unsigned char *
getsel(unsigned long offset, unsigned long *len, unsigned long *remain) {
	Atom xa_clip_string, utf8_string, typeret, clipboard;
	Window w;
	XEvent ev;
	int format;
	unsigned char *data;
	unsigned char *result = 0;

	utf8_string = XInternAtom(dpy, "UTF8_STRING", False);
	xa_clip_string = XInternAtom(dpy, "XSEL_STRING", False);

	w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 10, 10, 200, 200,
			1, CopyFromParent, CopyFromParent);

	XConvertSelection(dpy, seltype, utf8_string, xa_clip_string,
			w, CurrentTime);

	XFlush(dpy);
	XNextEvent(dpy, &ev);
	if(ev.type == SelectionNotify && ev.xselection.property != None) {
		XGetWindowProperty(dpy, w, ev.xselection.property, offset, 4096L, False,
				AnyPropertyType, &typeret, &format, len, remain, &data);
		if(*len) {
			result = malloc(sizeof(unsigned char) * *len);
			memcpy(result, data, *len);
		}
		XDeleteProperty(dpy, w, ev.xselection.property);
	}
	XDestroyWindow(dpy, w);

	return result;
}

int
main(int argc, char **argv) {
	unsigned char *data;
	unsigned long i, offset, len, remain;

	dpy = XOpenDisplay(0);
	if(!dpy)
		return 1 ;

	argv0 = argv[0] ;
	ARGBEGIN {
	case 'c' :
		seltype = XA_CLIPBOARD(dpy) ;
	break;
	case 's' :
		seltype = XA_SECONDARY ;
	break;
	default:
		usage();
	} ARGEND ;

	len = offset = remain = 0;
	do {
		data = getsel(offset, &len, &remain);
		for(i = 0; i < len; i++)
			putchar(data[i]);
		offset += len;
		free(data);
	} while(remain) ;

	XCloseDisplay(dpy);
	return 0;
}
