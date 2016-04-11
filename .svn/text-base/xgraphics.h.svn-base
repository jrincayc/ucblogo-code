/* X window system graphics header file. */

#include <memory.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

extern int have_x;
extern int back_ground;

extern void real_window_init();
void logofill(void);

/* Some X-related defines. */
#define BORDER	1
#define FONT	"fixed"
#define NUMCOLORS 512
#define NUMINITCOLORS 16
#define EVENT_MASK  (StructureNotifyMask | PointerMotionMask \
		     | ButtonPressMask | ButtonReleaseMask)
#define DEFAULT_HEIGHT           500
#define DEFAULT_WIDTH            500

#define GR_SIZE         60000

#define checkX { \
    if (have_x < 0) real_window_init(); \
    if (!have_x) { \
	err_logo(BAD_GRAPH_INIT,NIL); \
	return; \
    } \
}

#define prepare_to_draw          {checkX; placate_x();}
#define done_drawing             XFlush(dpy)
extern void placate_x();

#define prepare_to_draw_turtle nop()
#define done_drawing_turtle nop()

#define screen_left              0
#define screen_right             (screen_width-1)
#define screen_top               0
#define screen_bottom            (screen_height-1)

/* #define screen_height (1 + screen_bottom - screen_top) */
/* #define screen_width (1 + screen_right - screen_left) */

#define screen_x_center (screen_left + (screen_width)/2)
#define screen_y_center (screen_top + (screen_height)/2)

#define turtle_left_max ((screen_left) - (screen_x_center))
#define turtle_right_max ((screen_right) - (screen_x_center))
#define turtle_top_max ((screen_y_center) - (screen_top))
#define turtle_bottom_max ((screen_y_center) - (screen_bottom))

#define screen_x_coord ((screen_x_center) + turtle_x)
#define screen_y_coord ((screen_y_center) - turtle_y)

#define turtle_height 18
#define turtle_half_bottom 6.0
#define turtle_side 19.0

#define clear_screen		 XClearWindow(dpy, win)

#define erase_screen()		 XClearWindow(dpy, win)

#define line_to(a,b)             if(xgr_pen.vis==0)\
                                 XDrawLine(dpy,win,xgr_pen.pm,\
                                 xgr_pen.xpos,xgr_pen.ypos,\
                                 (a),(b));\
                                 xgr_pen.xpos=(a);\
                                 xgr_pen.ypos=(b)

#define move_to(a,b)             xgr_pen.xpos=(a);\
                                 xgr_pen.ypos=(b)

#define draw_string(s)           XDrawString(dpy,win,xgr_pen.pm,\
                                 xgr_pen.xpos,xgr_pen.ypos,\
                                 (s),strlen((s)));

#define set_pen_vis(v)           xgr_pen.vis=(v)

#define set_pen_mode(m)          xgr_pen.pm=(m)

#define set_pen_color(c)         draw_turtle();\
                                 xgr_pen.color=c%NUMCOLORS;\
                                 XSetForeground(dpy,draw_gc,color[2+xgr_pen.color].pixel);\
                                 XSetForeground(dpy,reverse_gc,color[2+xgr_pen.color].pixel);\
                                 draw_turtle();

#define set_back_ground(c)       back_ground=c%NUMCOLORS;\
                                 XSetBackground(dpy,draw_gc,color[2+back_ground].pixel);\
                                 XSetBackground(dpy,reverse_gc,color[2+back_ground].pixel);\
                                 XSetBackground(dpy,erase_gc,color[2+back_ground].pixel);\
                                 XSetForeground(dpy,erase_gc,color[2+back_ground].pixel);\
				 XSetWindowBackground(dpy,win,color[2+back_ground].pixel);\
				 redraw_graphics();

#define set_pen_width(w)         XSetLineAttributes(dpy, draw_gc, w, LineSolid, \
						    CapProjecting, JoinMiter);\
				 XSetLineAttributes(dpy, erase_gc, w, LineSolid, \
						    CapProjecting, JoinMiter);\
				 XSetLineAttributes(dpy, reverse_gc, w, LineSolid, \
						    CapProjecting, JoinMiter);\
                                 xgr_pen.pw = w;

#define set_pen_height(h)        XSetLineAttributes(dpy, draw_gc, h, LineSolid, \
						    CapProjecting, JoinMiter);\
				 XSetLineAttributes(dpy, erase_gc, h, LineSolid, \
						    CapProjecting, JoinMiter);\
				 XSetLineAttributes(dpy, reverse_gc, h, LineSolid, \
						    CapProjecting, JoinMiter);\
                                 xgr_pen.ph = h;

#define set_pen_x(x)             nop()
#define set_pen_y(y)             nop()

/* pen_info is a stucture type with fields for the various
   pen characteristics including the location, size, color,
   mode (e.g. XOR or COPY), pattern, visibility (0 = visible) */

typedef struct {
  int color;
  int xpos;
  int ypos;
  int vis;
  int pw;
  int ph;
  GC  pm;
} pen_info;

extern pen_info xgr_pen;

#define p_info_x(p)              (p.xpos)
#define p_info_y(p)              (p.ypos)

/* All these should take an argument, like the two just above.
   Then we could support multiple turtles.
 */

#define pen_width                xgr_pen.pw
#define pen_height               xgr_pen.ph
#define pen_color                xgr_pen.color
#define pen_mode                 xgr_pen.pm
#define pen_vis                  xgr_pen.vis
#define pen_x                    (xgr_pen.xpos)
#define pen_y                    (xgr_pen.ypos)
#define get_node_pen_pattern     (cons(make_intnode(-1), NIL))

#define pen_reverse              pen_mode=reverse_gc
#define pen_erase                pen_mode=erase_gc
#define pen_down                 pen_mode=draw_gc

#define button                   get_button()
#define mouse_x                  get_mouse_x()
#define mouse_y                  get_mouse_y()

/* There seems little point in implementing these unless we put
   everything in one window.  (Possibly use a slave xterm?)
 */
#define full_screen              nop()
#define split_screen             nop()
#define text_screen              nop()

#define plain_xor_pen()          pen_reverse

#define label(s)                 XDrawImageString(dpy,win,xgr_pen.pm,\
                                 xgr_pen.xpos,xgr_pen.ypos,\
                                 (s), strlen(s))

#define tone(p,d)                nop()
#define get_pen_pattern(p)       nop()
#define set_pen_pattern(p)       nop()
#define set_list_pen_pattern(p)  nop()

extern void set_palette(int, unsigned int, unsigned int, unsigned int);
extern void get_palette(int, unsigned int*, unsigned int*, unsigned int*);

/* The sparc has fmod.  So I use it. */
/* #define fmod(x,y)                x */


extern void nop();

/* Global X variables. */
extern int screen_height, screen_width;

extern Display    *dpy;		/* X server connection */
extern Window      win;		/* Window ID */
extern GC          draw_gc,     /* GC to draw with */
                   erase_gc,    /* GC to draw with */
                   reverse_gc;  /* GC to draw with */

extern XColor color[];
extern XColor dummy;

extern int get_mouse_x(), get_mouse_y();


/* Avoid name conflicts.  Note: if xgraphics.c uses True and
   False, bad things are likely to happen.
 */
#undef True
#undef False


