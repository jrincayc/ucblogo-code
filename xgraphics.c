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
int x_mouse_x, x_mouse_y;

/* We'll use 16 named colors for now (see xgraphics.h).
   The ordering here corresponds to the zero-based argument
   to setpencolor that gives that color -- pink is 12,
   turquoise is 10 etc.
 */

char *colorname[NUMCOLORS] = {
  "black", "blue"   , "green" , "cyan" ,
  "red"  , "magenta", "yellow", "white",
  "brown",
  "tan",
  "dark green",  /* Should be 'forest' */
  "aquamarine",  /* Should be 'aqua'   */
  "pink",        /* Should be 'salmon' */
  "purple",
  "orange",
  "grey"
};

XColor color[NUMCOLORS];
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

  for(n = 0; n < NUMCOLORS; n++)
    XAllocNamedColor(dpy, DefaultColormap(dpy, screen),
		     colorname[n], &color[n], &dummy);

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
  xsh.height = screen_height;
  xsh.width = screen_width;
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
}

NODE *Get_node_pen_mode(GC mode)
{
  long cur_mode = (long)mode;

  checkX;

  if(cur_mode == (long)draw_gc)
    return(make_static_strnode("paint"));

  if(cur_mode == (long)erase_gc)
    return(make_static_strnode("erase"));

  if(cur_mode == (long)reverse_gc)
    return(make_static_strnode("reverse"));

  return(make_static_strnode("unknown"));
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

    checkX;

  while(XCheckWindowEvent(dpy, win, EVENT_MASK, (XEvent *)&event))

    switch(event.type) {

    case ConfigureNotify:

      xce = (XConfigureEvent *)&event;
      screen_height = xce->height;
      screen_width  = xce->width;

      XClearWindow(dpy, win);
      if(turtle_shown)
	draw_turtle();
      
      break;

    case MotionNotify:

      xme = (XMotionEvent *)&event;
      x_mouse_x = xme->x;
      x_mouse_y = xme->y;
      break;
    }
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
			   color[xgr_pen.color].pixel );
	
	XPutImage( dpy, win, draw_gc, img, 0, 0, 0, 0, screen_width,
			   screen_height );
  XDestroyImage( img );
}

#endif
