// UCB Logo Web Graphics

#define NUMCOLORS 512
#define GR_SIZE 60000
typedef struct mypoint
{
   int x;
   int y;
} mypoint_t;

// #define prepare_to_draw (void)0
// #define done_drawing (void)0

#define prepare_to_draw web_prepare_to_draw();
#define done_drawing web_done_drawing();

#define screen_left 0
#define screen_right 640
#define screen_top 0
#define screen_bottom 480

#define screen_height (1 + screen_bottom - screen_top)
#define screen_width (1 + screen_right - screen_left)

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

#define clear_screen web_clear_screen()

/* pen_info is a stucture type with fields for the various
   pen characteristics including the location, size, color,
   mode (e.g. XOR or COPY), pattern, visibility (0 = visible) */

//fixme copypasta somebody elses
typedef struct
{
   int color;
   int xpos;
   int ypos;
   int vis;
   int pw;
   int ph;
   int pen_md;
} pen_info;

void save_pen(pen_info *p);
void restore_pen(pen_info *p);

extern pen_info xgr_pen;

#define p_info_x(p) (p.xpos)
#define p_info_y(p) (p.ypos)

#define pen_width xgr_pen.pw
#define pen_height xgr_pen.ph
#define pen_color xgr_pen.color
#define pen_mode xgr_pen.pen_md
#define pen_vis xgr_pen.vis
#define pen_x (xgr_pen.xpos)
#define pen_y (xgr_pen.ypos)
#define get_node_pen_pattern (cons(make_intnode(-1), NIL))

#define back_ground bg

#define pen_reverse web_pen_reverse()
#define pen_erase web_pen_erase()
#define pen_down web_pen_down()

#define button web_get_button()
#define mouse_x web_get_mouse_x()
#define mouse_y web_get_mouse_y()

#define full_screen web_full_screen()
#define split_screen web_split_screen()
#define text_screen web_text_screen()

#define plain_xor_pen() web_pen_reverse()

// #define fmod(x,y) x

#define prepare_to_draw_turtle web_prepare_to_draw_turtle()
#define done_drawing_turtle web_done_drawing(turtle_shown)

extern void set_palette(int, unsigned int, unsigned int, unsigned int);
extern void get_palette(int, unsigned int *, unsigned int *, unsigned int *);
void set_pen_color(int);
void web_prepare_to_draw();
void web_prepare_to_draw_turtle();
void web_done_drawing();
void set_pen_vis(int);
void web_pen_reverse();
void set_pen_width(int);
void set_pen_height(int);
void move_to(FIXNUM, FIXNUM);
void line_to(FIXNUM, FIXNUM);
void web_clear_screen();
void logofill();
void label(char *);
void web_split_screen();
void web_full_screen();
void web_pen_down();
void web_pen_erase();
void set_back_ground(int);
#define set_list_pen_pattern(arg) ((void)0)
int web_get_mouse_x();
int web_get_mouse_y();
int web_get_button();
void tone(int, int);
void doFilled(int color, int count, mypoint_t* points);
#define wxlPrintPreviewPict() ((void)0)
#define wxlPrintPict() ((void)0)
#define wxlPrintPreviewText() ((void)0)
#define wxlPrintText() ((void)0)
#define get_pen_pattern(pen_info) ((void)0)
void erase_screen();
#define draw_string(s) label(s);
void set_pen_mode(int);
#define set_pen_pattern(a) ((void)0)

extern int bg;

#define click_x webGetClickX()
#define click_y webGetClickY()
#define lastbutton webGetLastButton()
