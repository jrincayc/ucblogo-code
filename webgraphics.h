// UCB Logo Web Graphics

#define GR_SIZE 1

// #define prepare_to_draw nop()
// #define done_drawing nop()

#define prepare_to_draw web_prepare_to_draw();
#define done_drawing web_done_drawing();

#define screen_left 1
#define screen_right 100
#define screen_top 1
#define screen_bottom 100

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
typedef struct { int dummy; } pen_info;

#define p_info_x(p) p.dummy
#define p_info_y(p) p.dummy

#define pen_width pw
#define pen_height ph
#define pen_color pc
#define pen_mode pm
#define pen_vis pv
#define pen_x px
#define pen_y py
#define get_node_pen_pattern make_intnode(0)
#define back_ground bg

#define pen_reverse web_pen_reverse()
#define pen_erase web_pen_erase()
#define pen_down web_pen_down()

#define button FALSE
#define mouse_x 0
#define mouse_y 0

#define full_screen web_full_screen()
#define split_screen web_split_screen()
#define text_screen web_text_screen()

// #define fmod(x,y) x

#define prepare_to_draw_turtle web_prepare_to_draw_turtle()
#define done_drawing_turtle web_done_drawing()

extern int pw, ph, pc, pm, pv, px, py, bg;
extern void nop();

void get_palette();
void set_palette();