/*
 *      macterm.h          mac-specific graphics macros         mak
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
 
#include <QuickDraw.h>
extern WindowPtr graphics_window, listener_window;  /* mak */

#define GR_SIZE 30000

#define prepare_to_draw GetPort(&savePort); SetPort(graphics_window)
#define done_drawing SetPort(savePort)

#define prepare_to_draw_turtle nop()
#define done_drawing_turtle nop()

#define screen_left 1
#define screen_right (graphics_window->portRect.right - 1)
#define screen_top 1
#define screen_bottom (graphics_window->portRect.bottom - 1)

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

#define clear_screen erase_screen()

#define line_to(x,y) LineTo(x,y)
#define move_to(x,y) MoveTo(x,y)
#define draw_string(s) DrawString(s)
#define set_pen_vis(v) graphics_window->pnVis = v
#define set_pen_mode(m) graphics_window->pnMode = m
#define set_pen_color(c) mac_set_pc(c)
#define set_back_ground(c) mac_set_bg(c);
#define set_pen_width(w) graphics_window->pnSize.h = w
#define set_pen_height(h) graphics_window->pnSize.v = h
#define set_pen_x(x) graphics_window->pnLoc.h = x
#define set_pen_y(y) graphics_window->pnLoc.v = y

/* pen_info is a stucture type with fields for the various
   pen characteristics including the location, size, color,
   mode (e.g. XOR or COPY), pattern, visibility (0 = visible) */

typedef struct { PenState ps;
				 int      vis;
				 long     color; } pen_info;

#define p_info_x(p) p.ps.pnLoc.h
#define p_info_y(p) p.ps.pnLoc.v

#define pen_width graphics_window->pnSize.h
#define pen_height graphics_window->pnSize.v
#define pen_mode graphics_window->pnMode
#define pen_vis graphics_window->pnVis
#define pen_x graphics_window->pnLoc.h
#define pen_y graphics_window->pnLoc.v
#define get_node_pen_pattern Get_node_pen_pattern()

#define pen_reverse graphics_window->pnMode = patXor
#define pen_erase graphics_window->pnMode = patBic
#define pen_down graphics_window->pnMode = patCopy

#define button Button()
#define mouse_x mickey_x()
#define mouse_y mickey_y()

#define full_screen f_screen()
#define split_screen s_screen()
#define text_screen t_screen()

#define max(x,y) ((x > y) ? x : y)

/* colors
#define	blackColor		33
#define	whiteColor		30
#define	redColor		205
#define	greenColor		341
#define	blueColor		409
#define	cyanColor		273
#define	magentaColor	137
#define	yellowColor		69 */

extern GrafPtr savePort;
extern int x_coord, y_coord, x_max, y_max, tty_charmode;
extern char so_arr[], se_arr[];

extern void save_pen(), restore_pen(), plain_xor_pen(), set_pen_pattern();
extern void set_list_pen_pattern(), get_pen_pattern(), erase_screen();
extern void t_screen(), s_screen(), f_screen(), tone(), logofill(), nop();
extern FIXNUM mickey_x(), mickey_y();
extern NODE *Get_node_pen_pattern();
extern void c_to_pascal_string(), pascal_to_c_string();
extern FIXNUM pen_color, back_ground;
extern void mac_set_pc(), mac_set_bg();
extern void get_palette(int slot,
			unsigned int *r, unsigned int *g, unsigned int *b);
extern void set_palette(int slot,
			unsigned int r, unsigned int g, unsigned int b);
