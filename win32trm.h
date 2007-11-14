/* -*-C-*-
 * Win32trm.h
 *
 * Function prototypes &c for Win32 API (Win95, WinNT) compliant
 * procedures.
 *
 * $Id: win32trm.h,v 1.1.1.1 2005/02/01 19:47:31 paley Exp $
 *
 * $Log: win32trm.h,v $
 * Revision 1.1.1.1  2005/02/01 19:47:31  paley
 * initial commit
 *
 *
 * (c) 1996 UC Regents, etc.
 *
 */

#include <windows.h>
#include "logo.h"

#ifndef _WIN32TRM_H

/* set to 1000 for debugging, should normally be ~10k */
#define GR_SIZE 60000

#define prepare_to_draw win32_prepare_draw()
#define done_drawing

#define prepare_to_draw_turtle win32_turtle_prep()
#define done_drawing_turtle win32_turtle_end()

#define screen_left 0
#define screen_right win32_screen_right()
#define screen_top 0
#define screen_bottom win32_screen_bottom()
#define max_screen_bottom win32_screen_bottom()

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

#define turtle_height (FIXNUM) 18
#define turtle_half_bottom (FLONUM) 6.0
#define turtle_side (FLONUM) 19.0

#define clear_screen win32_erase_screen()

#define line_to(x,y) {\
  if (pen_vis==0) \
    lineto((int)x,(int)y); \
  else \
    moveto((int)x,(int)y);\
  }

#define move_to(x,y) moveto((int)x,(int)y)
/* #define draw_string(s) outtext((char *)s) function */

#define set_pen_vis(v) { turtlePen.vis = v; }
/* functionified */
#define set_pen_mode(m)  win32_set_pen_mode(m)
/* functionified */
#define set_pen_color(c) win32_set_pen_color(c)

#define set_back_ground(c) win32_set_bg(c)
/*  Now functions instead of macros */
#define set_pen_width(w) win32_set_pen_width(w)
#define set_pen_height(h) win32_set_pen_width(h)

#define set_pen_x(x) moveto((int)x, pen_y)
#define set_pen_y(y) moveto(pen_x, (int)y)

#define erase_screen() win32_erase_screen()

/* pen_info is a stucture type with fields for the various
   pen characteristics including the location, size, color,
   mode (e.g. XOR or COPY), pattern, visibility (0 = visible) */

typedef struct { int    h;
		 int	v;
		 int	vis;
		 int	width;
		 int    color;
		 char	pattern[8];
		 int	mode;
#ifdef WIN32
                 HPEN   hpen;
#endif  
} pen_info;

#define p_info_x(p) p.h
#define p_info_y(p) p.v

#define pen_width (int) turtlePen.width
#define pen_height (int) turtlePen.width
#define pen_mode turtlePen.mode
#define pen_vis turtlePen.vis

#define pen_x turtlePen.h
#define pen_y turtlePen.v

#define get_node_pen_pattern win32_get_node_pen_pattern()

#define pen_reverse win32_pen_reverse()
#define pen_erase win32_pen_erase()
#define pen_down win32_pen_down()

#define button (w_button)

#define full_screen win32_con_full_screen()
#define split_screen win32_con_split_screen()
#define text_screen win32_con_text_screen()

/* definitions from term.c and math.c for ibmterm.c */
extern int x_coord, y_coord, x_max, y_max, tty_charmode;
extern char so_arr[], se_arr[];
extern FLONUM degrad;

/* Forward declarations for graphics.c */
extern void gr_mode(), draw_turtle();
extern void win32_pen_erase(), win32_pen_down(), win32_pen_xor();
extern void win32_set_pen_mode(int), win32_set_pen_color(), win32_set_bg();
extern int get_win32_pen_mode(), win32_set_pen_width(int);
extern void save_pen(pen_info*), restore_pen(pen_info*), set_pen_pattern();
extern void plain_xor_pen();
extern void set_list_pen_pattern(), get_pen_pattern(), erase_screen();
extern void label(), logofill();
extern void t_screen(), s_screen(), f_screen(), tone();
extern FIXNUM mickey_x(), mickey_y();
extern NODE *win32_get_node_pen_pattern();
extern FIXNUM t_height();
extern FIXNUM pen_color, back_ground;
extern FLONUM t_half_bottom(), t_side();
extern BOOLEAN in_erase_mode;
extern void win32_init_palette(), win32_pen_reverse();
extern void win32_turtle_prep(), win32_turtle_end();
extern void win32_con_text_screen(), win32_con_split_screen();
extern void win32_prepare_draw(void);
extern void win32_con_full_screen(), win32_clear_text();
extern NODE* win32_lsetcursor(NODE *);
extern int win32_screen_right();
extern pen_info turtlePen;
extern void MoveCursor(int, int), win32_receive_char(char);
extern void win32_parse_line(char*);
extern FIXNUM mouse_x, mouse_y;
extern int w_button;

/* prototypes added by sowings */
extern BOOLEAN safe_to_save();
extern void save_lm_helper(), save_line(), save_move();
extern void save_mode(), save_color(), save_pattern();
extern void save_size(), save_vis();

extern void set_palette(int, unsigned int, unsigned int, unsigned int);
extern void get_palette(int, unsigned int*, unsigned int*, unsigned int*);

#define nop() {} 

#define WIN_PEN_ERASE   1
#define WIN_PEN_DOWN    2
#define WIN_PEN_REVERSE 3

#define NUM_LINES 100
#define CHARS_PER_LINE 200

LRESULT CALLBACK MainFunc(HWND, UINT, WPARAM, LPARAM);

#define _WIN32TRM_H
#endif /* !_WIN32TRM_H */
