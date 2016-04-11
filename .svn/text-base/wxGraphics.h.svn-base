
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
  int pen_md;
} pen_info;



pen_info* getPen();


#define SCREEN_WIDTH		1
#define SCREEN_HEIGHT		2
#define	BACK_GROUND			3
#define	IN_SPLITSCREEN		4
#define	IN_GRAPHICS_MODE	5
#define NUMCOLORS 256
#define NUMINITCOLORS 16


#define GR_SIZE         60000

#define prepare_to_draw          wxPrepare()
#define done_drawing             nop()

#define prepare_to_draw_turtle nop()
#define done_drawing_turtle  nop()

    
//#define screen_height (1 + screen_bottom - screen_top) 
//#define screen_width (1 + screen_right - screen_left) 
#define screen_width			 wxGetInfo(SCREEN_WIDTH)
#define	screen_height			 wxGetInfo(SCREEN_HEIGHT)
#define back_ground				 wxGetInfo(BACK_GROUND)
#define xgr_pen					 getPen()

#define screen_left              0
#define screen_right             (screen_width)
#define screen_top               0
#define screen_bottom            (screen_height)


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

#define refresh_screen()         wx_refresh();
#define clear_screen		 wx_clear();

#define erase_screen()		 wx_clear();

#define line_to(a,b)             wxDrawLine(xgr_pen->xpos, \
                                 xgr_pen->ypos,a,b,!xgr_pen->vis);\
                                 xgr_pen->xpos=(a);\
                                 xgr_pen->ypos=(b)

#define move_to(a,b)             xgr_pen->xpos=(a); \
                                 xgr_pen->ypos=(b); \
				 turtlePosition_x=screen_x_coord; \
				 turtlePosition_y=screen_y_coord;

#define draw_string(s)           wxLabel(s);

#define set_pen_vis(v)           xgr_pen->vis=(v)

#define set_pen_mode(m)          xgr_pen->pen_md=(m)

#define set_pen_color(c)         draw_turtle();\
                                 xgr_pen->color=c%NUMCOLORS;\
                                 draw_turtle();

#define set_back_ground(c)       wxSetInfo(BACK_GROUND,c%NUMCOLORS);\
                                 redraw_graphics();

#define set_pen_width(w)         wxSetPenWidth(w);

#define set_pen_height(h)        nop();

#define set_pen_x(x)             nop()
#define set_pen_y(y)             nop()



#define p_info_x(p)              (p.xpos)
#define p_info_y(p)              (p.ypos)

/* All these should take an argument, like the two just above.
   Then we could support multiple turtles.
 */

#define pen_width                xgr_pen->pw
#define pen_height               xgr_pen->pw	/* not ph, which isn't set */
#define pen_color                xgr_pen->color
#define pen_mode                 xgr_pen->pen_md
#define pen_vis                  xgr_pen->vis
#define pen_x                    (xgr_pen->xpos)
#define pen_y                    (xgr_pen->ypos)
#define get_node_pen_pattern     (cons(make_intnode(-1), NIL))

#define PEN_REVERSE              0
#define PEN_ERASE                1
#define PEN_DOWN                 2

#define pen_reverse              pen_mode=PEN_REVERSE
#define pen_erase                pen_mode=PEN_ERASE
#define pen_down                 pen_mode=PEN_DOWN

#define button                   wxGetButton()
#define lastbutton		 wxGetLastButton()
#define mouse_x                  wxGetMouseX()
#define mouse_y                  wxGetMouseY()
#define click_x                  wxGetClickX()
#define click_y                  wxGetClickY()

#define full_screen              wxFullScreen()
#define split_screen             wxSplitScreen()
#define text_screen              wxTextScreen()

#define in_splitscreen wxGetInfo(IN_SPLITSCREEN)
#define in_graphics_mode wxGetInfo(IN_GRAPHICS_MODE)

#define plain_xor_pen()          pen_reverse

#define label(s)                 wxLabel(s)

#define tone(p,d)                nop()
#define get_pen_pattern(p)       nop()
#define set_pen_pattern(p)       nop()
#define set_list_pen_pattern(p)  nop()

extern void set_palette(int, unsigned int, unsigned int, unsigned int);
extern void get_palette(int, unsigned int*, unsigned int*, unsigned int*);
extern void save_pen(pen_info*), restore_pen(pen_info*);
extern void logofill();




/* The sparc has fmod.  So I use it. */
/* #define fmod(x,y)                x */


extern void nop();







