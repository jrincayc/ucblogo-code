/* X window graphics for logo.  Nov. 1991. */

/* Note:  logo uses True and False as variables to hold references
   to the nodes containing the values true and false.  X uses them
   as macros serving the functions often served by the more standard
   TRUE and FALSE.  To avoid a conflict, don't use True and False in
   this file.
 */

#ifndef X_DISPLAY_MISSING

#include <stdio.h>

#include "logo.h"
#include "xgraphics.h"
#include "globals.h"

char *LogoPlatformName="X11";

int clearing_screen = 0;

#ifdef SIGWINCH

#ifdef SIG_TAKES_ARG
#define sig_arg 0
RETSIGTYPE x_win_resize(int sig)
#else
#define sig_arg 
RETSIGTYPE x_win_resize()
#endif
{
    signal(SIGWINCH, x_win_resize);
    placate_x();
    SIGRET
}
#endif	/* SIGWINCH */

int have_x = -1;

XWMHints xwmh = {
  (InputHint|StateHint),	/* flags */
  FALSE,			/* input */
  NormalState,                  /* initial_state */
  0,				/* icon pixmap */
  0,				/* icon window */
  0, 0,                         /* icon location */
  0,				/* icon mask */
  0,				/* Window group */
};

int screen_height = DEFAULT_HEIGHT;
int screen_width  = DEFAULT_WIDTH;
int x_mouse_x, x_mouse_y, x_buttondown=0;

/* We'll use 16 named colors for now (see xgraphics.h).
   The ordering here corresponds to the zero-based argument
   to setpencolor that gives that color -- pink is 12,
   turquoise is 10 etc.
 */

char *colorname[NUMINITCOLORS] = {
  "black", "blue"   , "green" , "cyan" ,
  "red"  , "magenta", "yellow", "white",
  "brown",
  "tan",
  "dark green",  /* Should be 'forest' */
  "aquamarine",  /* Should be 'aqua'   */
  "salmon",
  "purple",
  "orange",
  "grey"
};

XColor color[NUMCOLORS+2];
XColor dummy;

void nop() {
}

pen_info xgr_pen;
int back_ground;

Display    *dpy;		/* X server connection   */
Window      win;		/* Window ID             */
GC          draw_gc,            /* GC to draw with       */
            up_gc,              /* Do nothing gc.        */
            erase_gc,           /* Erase gc.             */
            reverse_gc;         /* GC to reverse things. */

int         screen;

int save_argc;
char ** save_argv;

void x_window_init(int argc, char **argv)
{
    save_argc = argc;
    save_argv = argv;
}

void real_window_init()
{
  unsigned long fth, pad;	/* Font size parameters */
  unsigned long fg, bg, bd;	/* Pixel values */
  unsigned long bw;		/* Border width */
  XFontStruct *fontstruct;	/* Font descriptor */
  XGCValues   gcv;		/* Struct for creating GC */
  XEvent      event;		/* Event received */
  XSizeHints  xsh;		/* Size hints for window manager */
  XSetWindowAttributes xswa;	/* Temporary Set Window Attribute struct */
  int n;

  /* Open X display. */
  if ((dpy = XOpenDisplay(NULL)) == NULL) {
    have_x = 0;
    return;
  } else have_x = 1;

  screen = DefaultScreen(dpy);

  /* Load default font. */
  if ((fontstruct = XLoadQueryFont(dpy, FONT)) == NULL) {
    have_x = 0;
    return;
  }

  fth = fontstruct->max_bounds.ascent + fontstruct->max_bounds.descent;

  /*
   * Select colors.
   */

  for(n = 0; n < NUMINITCOLORS; n++)
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen),
		     colorname[n], &color[n+2], &dummy);

    for (n = NUMINITCOLORS; n < NUMCOLORS; n++)
	color[n+2] = color[(n+2) % NUMINITCOLORS];

    xgr_pen.color = 7;

  /* Not much alternative to the following,
     because of the use of the xor operation to erase the turtle.
     The background HAS to be the zero pixel or the scheme won't work.
   */
  bg = BlackPixel(dpy, screen);
  fg = bd = WhitePixel(dpy, screen);

  /*
   * Set the border width of the window,  and the gap between the text
   * and the edge of the window, "pad".
   */
  pad = BORDER;
  bw = 1;

  /* Set up size/position hints. */
  xsh.flags = (PPosition | PSize);
  xsh.height = DEFAULT_HEIGHT;
  xsh.width = DEFAULT_WIDTH;
  xsh.x = (DisplayWidth(dpy, DefaultScreen(dpy)) - xsh.width) / 2;
  xsh.y = (DisplayHeight(dpy, DefaultScreen(dpy)) - xsh.height) / 2;

  win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy),
			    xsh.x, xsh.y, xsh.width, xsh.height,
			    bw, bd, bg);

  /* Label window. */
  XSetStandardProperties(dpy, win, "BXLogo", "BXLogo", None,
				save_argv, save_argc, &xsh);
  XSetWMHints(dpy, win, &xwmh);

  /* Set up default color map. */
  xswa.colormap = DefaultColormap(dpy, DefaultScreen(dpy));
  xswa.backing_store = Always;
  XChangeWindowAttributes(dpy, win, CWColormap | CWBackingStore, &xswa);


 /*
  * Create GCs.
  */

  /* Make the foreground and background fields in the GC
     match the fg and bg variables that were passed to the window.
   */

  gcv.font = fontstruct->fid;
  gcv.background = BlackPixel(dpy, screen);
  gcv.foreground = WhitePixel(dpy, screen);
  gcv.plane_mask = AllPlanes;

  /* Normal drawing GC. */
  draw_gc = XCreateGC(dpy, win,
		      (GCPlaneMask | GCFont | GCForeground | GCBackground),
		      &gcv);

  /* Create GC for erasing/drawing turtle. */
  gcv.function = GXxor;
  reverse_gc = XCreateGC(dpy, win, 
			 (GCPlaneMask | GCFont | GCFunction |
			  GCForeground | GCBackground), 
			 &gcv);

  /* Erase gc just draws the bacground color. */
  gcv.foreground = bg;
  gcv.function = GXcopy;
  erase_gc = XCreateGC(dpy, win,
		       (GCPlaneMask | GCFont | GCForeground | GCBackground),
		       &gcv);

  gcv.function = GXnoop;
  up_gc = XCreateGC(dpy, win,
		    (GCPlaneMask | GCFunction | GCForeground | GCBackground),
		    &gcv);

  xgr_pen.pm = draw_gc;
  xgr_pen.vis = 0;

  XSelectInput(dpy, win, EVENT_MASK);

 /*
  * Map the window to make it visible.  See Section 3.5.
  */
  XMapWindow(dpy, win);

 /*
  * Loop,  examining each event.  Exit when we get mapped.
  */
  do {
   /*
    * Get the next event
    */
    XWindowEvent(dpy, win, StructureNotifyMask, &event);

    /*
     * Wait for the map notify event.
     */
    if (event.type == MapNotify) {
      XClearWindow(dpy, win);
      break;
    }
  } while(1);

  lclearscreen(NIL);
  move_to(screen_x_coord, screen_y_coord);
/*   if(turtle_shown)
    draw_turtle();  */
#ifdef SIGWINCH
    signal(SIGWINCH, x_win_resize);
#endif
}

void save_pen(pen_info *p) {
    memcpy(((char *)(p)),((char *)(&xgr_pen)),sizeof(pen_info));
}

void restore_pen(pen_info *p) {
    memcpy(((char *)(&xgr_pen)),((char *)(p)),sizeof(pen_info));
    set_pen_width(p->pw);
    set_pen_height(p->ph);
}

void placate_x()
{
  XEvent           event;
  XConfigureEvent *xce;
  XMotionEvent    *xme;
  XButtonEvent    *xbe;
    checkX;

  while(XCheckWindowEvent(dpy, win, EVENT_MASK, (XEvent *)&event))

    switch(event.type) {

    case ConfigureNotify:

      xce = (XConfigureEvent *)&event;
      screen_height = xce->height;
      screen_width  = xce->width;

      XClearWindow(dpy, win);
      /* if (!clearing_screen) */ redraw_graphics();

      break;

    case MotionNotify:

      xme = (XMotionEvent *)&event;
      x_mouse_x = xme->x;
      x_mouse_y = xme->y;
      break;

    case ButtonPress:
	xbe = (XButtonEvent *)&event;
#undef button
	x_buttondown = xbe->button;
	mouse_click;
	break;

    case ButtonRelease:
	x_buttondown = 0;
	break;
    }
}

void check_X11_stop() {
    static int count=300;

    if (--count == 0) {
	count = 300;
	checkX;
	placate_x();
    }
}

int get_button()
{
  checkX;

  placate_x();

  return( x_buttondown );
}

int get_mouse_x()
{
  checkX;

  placate_x();

  return( x_mouse_x - (screen_width / 2));
}


int get_mouse_y()
{
  checkX;

  placate_x();

  return((screen_height / 2) - x_mouse_y);
}

 
void floodfill( XImage *img, int x, int y, unsigned long
					   oldColor, unsigned long newColor ) 
{
	int lastBorder;
	int leftLimit, rightLimit;
	int i;
	if (oldColor == newColor) {
		/* Nothing to be done */
		return;
	}
	/* Seek left */
	leftLimit = (-1);
	for (i = x; (i >= 0); i--) {
		if (XGetPixel(img, i, y) != oldColor) {
			break;
		}
		XPutPixel(img, i, y, newColor);
		leftLimit = i;
	}
	if (leftLimit == (-1)) {
		return;
	}
	/* Seek right */
	rightLimit = x;
	for (i = (x+1); (i < screen_width); i++) {	
		if (XGetPixel(img, i, y) != oldColor) {
			break;
		}
		XPutPixel(img, i, y, newColor);
		rightLimit = i;
	}
	/* Look at lines above and below and start paints */
	/* Above */
	if (y > 0) {
		lastBorder = 1;
		for (i = leftLimit; (i <= rightLimit); i++) {
			int c;
			c = XGetPixel(img, i, y-1);
			if (lastBorder) {
				if (c == oldColor) {	
					floodfill(img, i, y-1, oldColor, newColor);
					lastBorder = 0;
				}
			} else if (c != oldColor) {
				lastBorder = 1;
			}
		}
	}
	/* Below */
	if (y < screen_height - 1) {
		lastBorder = 1;
		for (i = leftLimit; (i <= rightLimit); i++) {
			int c;
			c = XGetPixel(img, i, y+1);
			if (lastBorder) {
				if (c == oldColor) {
					floodfill(img, i, y+1, oldColor, newColor);
					lastBorder = 0;
				}
			} else if (c != oldColor) {
				lastBorder = 1;
			}
		}
	}
}
 
void logofill() {
	XImage *img = XGetImage(dpy, win, 0, 0, screen_width, screen_height, -1,
							ZPixmap );
	floodfill( img, xgr_pen.xpos, xgr_pen.ypos,
			   XGetPixel( img, xgr_pen.xpos, xgr_pen.ypos ),
			   color[(xgr_pen.color)+2].pixel );
	
	XPutImage( dpy, win, draw_gc, img, 0, 0, 0, 0, screen_width,
			   screen_height );
  XDestroyImage( img );
}


void set_palette(int n, unsigned int r, unsigned int g, unsigned int b) {
    n+=2;
    color[n].red = r;
    color[n].green = g;
    color[n].blue = b;
    color[n].flags = DoRed|DoGreen|DoBlue;  /* unnecessary? */
    XAllocColor(dpy, DefaultColormap(dpy, DefaultScreen(dpy)), &color[n]);
}

void get_palette(int n, unsigned int *r, unsigned int *g, unsigned int *b) {
    n+=2;
    *r = color[n].red;
    *g = color[n].green;
    *b = color[n].blue;
}

#endif


