/* X window system graphics header file. */

#include <memory.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

extern int have_x;

#define checkX {if (have_x < 0) real_window_init(); \
		if (!have_x) {err_logo(BAD_GRAPH_INIT,NIL);return;}}

/* Some X-related defines. */
#define BORDER	1
#define FONT	"fixed"
#define NUMCOLORS 16
#define EVENT_MASK  (StructureNotifyMask | PointerMotionMask)
#define DEFAULT_HEIGHT           500
#define DEFAULT_WIDTH            500

#define GR_SIZE         1

#define prepare_to_draw          {checkX; placate_x();}
#define done_drawing             XFlush(dpy)

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

#define clear_screen		 checkX;\
				 XClearWindow(dpy, win);\
                                 XFlush(dpy)

#define line_to(a,b)             checkX;\
				 if(orig_pen.vis==0)\
                                 XDrawLine(dpy,win,orig_pen.pm,\
                                 orig_pen.xpos,orig_pen.ypos,\
                                 (a),(b));\
                                 orig_pen.xpos=(a);\
                                 orig_pen.ypos=(b)

#define move_to(a,b)             checkX;\
                                 orig_pen.xpos=(a);\
                                 orig_pen.ypos=(b)

#define draw_string(s)           checkX;\
                                 XDrawString(dpy,win,orig_pen.pm,\
                                 orig_pen.xpos,orig_pen.ypos,\
                                 (s),strlen((s)));

#define set_pen_vis(v)           orig_pen.vis=(v)

#define set_pen_mode(m)          orig_pen.pm=(m)

#define set_pen_color(c)         checkX;\
                                 if(turtle_shown)\
                                   draw_turtle();\
                                 orig_pen.color=c%NUMCOLORS;\
                                 XSetForeground(dpy,draw_gc,color[orig_pen.color].pixel);\
                                 XSetForeground(dpy,reverse_gc,color[orig_pen.color].pixel);\
                                 if(turtle_shown)\
                                   draw_turtle();

#define set_back_ground(c)	 nop()
#define set_pen_width(w)         nop()
#define set_pen_height(h)        nop()
#define set_pen_x(x)             nop()
#define set_pen_y(y)             nop()
#define get_palette(x,y,z,w)	 nop()
#define set_palette(x,y,z,w)	 nop()

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

#define p_info_x(p)              (p.xpos)
#define p_info_y(p)              (p.ypos)

/* All these should take an argument, like the two just above.
   Then we could support multiple turtles.
 */

#define pen_width                orig_pen.pw
#define pen_height               orig_pen.ph
#define pen_color                orig_pen.color
#define pen_mode                 orig_pen.pm
#define pen_vis                  orig_pen.vis
#define pen_x                    px
#define pen_y                    py
#define get_node_pen_pattern     (cons(make_intnode(-1)), NIL)
#define get_node_pen_mode        Get_node_pen_mode(orig_pen.pm)

#define back_ground		 0

#define pen_reverse              pen_mode=reverse_gc
#define pen_erase                pen_mode=erase_gc
#define pen_down                 pen_mode=draw_gc

/* Hmn, buttons are a problem, aren't they? */
#define button                   FALSE
#define mouse_x                  get_mouse_x()
#define mouse_y                  get_mouse_y()

/* There seems little point in implementing these unless we put
   everything in one window.  (Possibly use a slave xterm?)
 */
#define full_screen              nop()
#define split_screen             nop()
#define text_screen              nop()

#define save_pen(p)              memcpy(((char *)(p)),((char *)(&orig_pen)),sizeof(pen_info))
#define restore_pen(p)           memcpy(((char *)(&orig_pen)),((char *)(p)),sizeof(pen_info))

#define plain_xor_pen()          pen_reverse

#define label(s, len)            checkX;\
                                 XDrawString(dpy,win,orig_pen.pm,\
                                 orig_pen.xpos,orig_pen.ypos,\
                                 (s), (len))

#define tone(p,d)                nop()
#define get_pen_pattern(p)       nop()
#define set_pen_pattern(p)       nop()
#define set_list_pen_pattern(p)  nop()

/* The sparc has fmod.  So I use it. */
/* #define fmod(x,y)                x */


extern int px, py;
extern double degrad;
extern void nop();

/* Global X variables. */
extern int screen_height, screen_width;

extern Display    *dpy;		/* X server connection */
extern Window      win;		/* Window ID */
extern GC          draw_gc,     /* GC to draw with */
                   erase_gc,    /* GC to draw with */
                   reverse_gc;  /* GC to draw with */

extern XColor color[16];
extern XColor dummy;

extern NODE * Get_node_pen_mode();

extern int get_mouse_x(), get_mouse_y();


/* Avoid name conflicts.  Note: if xgraphics.c uses True and
   False, bad things are likely to happen.
 */
#undef True
#undef False


