/*
 *      ztcterm.h          IBM-specific graphics macros         mak
 *
 *	Copyright (C) 1993 by the Regents of the University of California
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *  
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *  
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#define GR_SIZE 15000

#define prepare_to_draw gr_mode()
#define done_drawing nop()

#define prepare_to_draw_turtle nop()
#define done_drawing_turtle nop()

#define screen_left 0
#define screen_right (MaxX)
#define screen_top 0
#define screen_bottom (ibm_screen_bottom)
#define max_screen_bottom (MaxY)

#define screen_height (1 + screen_bottom - screen_top)
#define max_screen_height (1 + max_screen_bottom - screen_top)
#define screen_width (1 + screen_right - screen_left)

#define screen_x_center (screen_left + (screen_width)/2)
#define screen_y_center (screen_top + (max_screen_height)/2)

#define turtle_left_max ((screen_left) - (screen_x_center))
#define turtle_right_max ((screen_right) - (screen_x_center))
#define turtle_top_max ((screen_y_center) - (screen_top))
#define turtle_bottom_max ((screen_y_center) - (screen_bottom))

#define screen_x_coord ((screen_x_center) + turtle_x)
#define screen_y_coord ((screen_y_center) - turtle_y)

#define turtle_height t_height()
#define turtle_half_bottom t_half_bottom()
#define turtle_side t_side()

#define clear_screen erase_screen()

#define line_to(x,y) {if (current_vis==0) lineto((int)x,(int)y); else moveto((int)x,(int)y);}
#define move_to(x,y) moveto((int)x,(int)y)
#define draw_string(s) outtext((char *)s)
#define set_pen_vis(v) current_vis = v
#define set_pen_mode(m) set_ibm_pen_mode(m)
#define set_pen_color(c) {ztc_set_penc(c);}
#define set_back_ground(c) {ztc_set_bg(c);}
#define set_pen_width(w) {ztc_penwidth = w;}
#define set_pen_height(h) {ztc_penwidth = h;}
#define set_pen_x(x) moveto((int)x, ztc_y)
#define set_pen_y(y) moveto(ztc_x, (int)y)

/* pen_info is a stucture type with fields for the various
   pen characteristics including the location, size, color,
   mode (e.g. XOR or COPY), pattern, visibility (0 = visible) */

typedef struct { int    h;
		 int	v;
		 int	vis;
		 int	width;
		 int    color;
		 char	pattern[8];
		 int	mode; } pen_info;

#define p_info_x(p) p.h
#define p_info_y(p) p.v

#define pen_width ztc_penwidth
#define pen_height ztc_penwidth
#define pen_mode get_ibm_pen_mode()
#define pen_vis current_vis
#define pen_x ztc_x
#define pen_y ztc_y
#define get_node_pen_pattern Get_node_pen_pattern()

#define pen_reverse ibm_pen_xor()
#define pen_erase ibm_pen_erase()
#define pen_down ibm_pen_down()

#define button Button()
#define mouse_x mickey_x()
#define mouse_y mickey_y()

#define full_screen f_screen()
#define split_screen s_screen()
#define text_screen t_screen()

/* definitions from term.c and math.c for ibmterm.c */
extern int x_coord, y_coord, x_max, y_max, tty_charmode;
extern char so_arr[], se_arr[];

/* definitions from ibmterm.c for graphics.c */
extern void gr_mode();
extern void ibm_pen_erase(), ibm_pen_down(), ibm_pen_xor();
extern void set_ibm_pen_mode();
extern int get_ibm_pen_mode();
extern void save_pen(), restore_pen(), set_pen_pattern();
extern void plain_xor_pen();
extern void set_list_pen_pattern(), get_pen_pattern(), erase_screen();
extern void label(), logofill();
extern void t_screen(), s_screen(), f_screen(), tone();
extern FIXNUM mickey_x(), mickey_y();
extern NODE *Get_node_pen_pattern();
extern FIXNUM t_height();
extern FIXNUM pen_color, back_ground;
extern FLONUM t_half_bottom(), t_side();
extern int current_vis, ibm_screen_bottom;
extern BOOLEAN in_erase_mode;
extern int ztc_penwidth;
extern fg_coord_t ztc_x, ztc_y;
extern void get_palette(int slot,
			unsigned int *r, unsigned int *g, unsigned int *b);
extern void set_palette(int slot,
			unsigned int r, unsigned int g, unsigned int b);
