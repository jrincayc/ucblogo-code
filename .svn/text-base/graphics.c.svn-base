/*
 *      graphics.c          logo graphics module          mak
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

#ifdef WIN32
#include <windows.h>
#endif /* WIN32 */
 
#define WANT_EVAL_REGS 1
#include "logo.h"
/*   #include "globals.h"   has been moved further down */
#include <math.h>

#ifdef HAVE_WX
#include "wxGraphics.h"
#elif defined(mac)
#include "macterm.h"
#elif defined(WIN32)
#include "win32trm.h"
#elif defined(__RZTC__)
#include <fg.h>
#include "ztcterm.h"
#elif defined(x_window)
#include "xgraphics.h"
#elif defined(ibm)
#include "ibmterm.h"
#else
#include "nographics.h"
#endif /* end this whole big huge tree */

#include "globals.h"

#ifdef HAVE_WX
int drawToPrinter=0;
int turtlePosition_x=0;
int turtlePosition_y=0;
extern void wx_adjust_label_height();
#endif

#if defined(__RZTC__) && !defined(WIN32) /* sowings */
#define total_turtle_bottom_max (-(MaxY/2))
#else
#define total_turtle_bottom_max turtle_bottom_max
#endif

/* types of graphics moves that can be recorded */
#define FINISHED       0    /* must be zero */
#define LINEXY         1
#define MOVEXY         2
#define LABEL          3
#define SETPENVIS      4
#define SETPENMODE     5
#define SETPENCOLOR    6
#define SETPENSIZE     7
#define SETPENPATTERN  8
#define FILLERUP       9
#define ARC	      10
#define SETPENRGB     11
#define NEXTBUFFER    12
#define STARTFILL     13
#define ENDFILL	      14
#define COLORFILL     15

/* NOTE: See the files (macterm.c and macterm.h) or (ibmterm.c and ibmterm.h)
   for examples of the functions and macros that this file assumes exist. */

#define One (sizeof(int))
#define Two (2*One)
#define Three (3*One)
#define Four (4*One)
#define Big (sizeof(FLONUM))

#define PENMODE_PAINT	0
#define PENMODE_ERASE	1
#define PENMODE_REVERSE	2

int internal_penmode = PENMODE_PAINT;
int drawing_turtle = 0;

enum s_md screen_mode = SCREEN_TEXT;

mode_type current_mode = wrapmode;
FLONUM turtle_x = 0.0, turtle_y = 0.0, turtle_heading = 0.0;
FLONUM x_scale = 1.0, y_scale = 1.0;
BOOLEAN turtle_shown = FALSE, user_turtle_shown = TRUE;
int graphics_setup = 0;
FLONUM wanna_x = 0.0, wanna_y = 0.0;
BOOLEAN out_of_bounds = FALSE;
void setpos_bynumber(FLONUM, FLONUM);
void internal_hideturtle(void);

int max_palette_slot = 0;

char record_buffer[GR_SIZE];
char *record = 0;
FIXNUM record_index = 0;
int last_recorded = -1;
pen_info orig_pen;

BOOLEAN refresh_p = TRUE;
BOOLEAN doing_filled = FALSE;

/************************************************************/

double pfmod(double x, double y) {
    double temp = fmod(x,y);

    if (temp < 0) return temp+y;
    return temp;
}

FLONUM cut_error(FLONUM n) {
    n *= 1000000;
    n = (n > 0 ? floor(n) : ceil(n));
    n /= 1000000;
    if (n == -0.0) n = 0.0;
    return(n);
}

FIXNUM g_round(FLONUM n) {
    n += (n < 0.0 ? -0.5 : 0.5);
    if ((n < 0.0 ? -n : n) > MAXLOGOINT)
	err_logo(TURTLE_OUT_OF_BOUNDS, NIL);
    if (n < 0.0)
	return((FIXNUM)ceil(n));
    return((FIXNUM)floor(n));
}

/************************************************************/
void draw_turtle_helper(void);
void check_x_high(void);
void check_x_low(void);
void forward(FLONUM);

void draw_turtle(void) {
    unsigned int r=0, g=0, b=0;
    int old_color=pen_color;
    int save_vis;

#ifdef HAVE_WX
    if (drawToPrinter) return;
#endif

    if (!turtle_shown) {
		if (!graphics_setup) {
			graphics_setup++;
                        turtle_shown = TRUE;
		}
		return;
	}

    drawing_turtle = 1;
    turtle_shown = 0;
/*
    get_palette(pen_color, &r, &g, &b);
    if (r==0 && g==0 && b==0) {
	set_pen_color(7);
    }
*/
    save_vis=pen_vis;
    pen_vis = -1;
    forward(-1);
    draw_turtle_helper();
    /* all that follows is for "turtle wrap" effect */
    if ((turtle_y > turtle_top_max - turtle_height) &&
	    (current_mode == wrapmode)) {
	turtle_y -= (screen_height + 1);
	draw_turtle_helper();
	check_x_high();
	check_x_low();
	turtle_y += (screen_height + 1);
    }
    if ((turtle_y < turtle_bottom_max + turtle_height) &&
	    (current_mode == wrapmode)) {
	turtle_y += (screen_height + 1);
	draw_turtle_helper();
	check_x_high();
	check_x_low();
	turtle_y -= (screen_height + 1);
    }
    check_x_high();
    check_x_low();
    turtle_shown = 0;
    set_pen_color(old_color);
    forward(1);
    set_pen_vis(save_vis);
    turtle_shown = 1;
    drawing_turtle = 0;
}

void check_x_high(void) {
    if ((turtle_x > turtle_right_max - turtle_height) &&
	    (current_mode == wrapmode)) {
	turtle_x -= (screen_width + 1);
	draw_turtle_helper();
	turtle_x += (screen_width + 1);
    }
}

void check_x_low(void) {
    if ((turtle_x < turtle_left_max + turtle_height) &&
	    (current_mode == wrapmode)) {
	turtle_x += (screen_width + 1);
	draw_turtle_helper();
	turtle_x -= (screen_width + 1);
    }
}
void draw_turtle_helper(void) {
    pen_info saved_pen;
    FLONUM real_heading;
    int left_x, left_y, right_x, right_y, top_x, top_y;
#if 1	/* Evan Marshall Manning <manning@alumni.caltech.edu> */
    double cos_real_heading, sin_real_heading;
    FLONUM delta_x, delta_y;
#endif
   
    prepare_to_draw;
    prepare_to_draw_turtle;
    save_pen(&saved_pen);
    plain_xor_pen();
    pen_vis = 0;
    set_pen_width(1);
    set_pen_height(1);

    real_heading = -turtle_heading + 90.0;
 
    cos_real_heading = cos((FLONUM)(real_heading*degrad));
    sin_real_heading = sin((FLONUM)(real_heading*degrad));
 
    delta_x = x_scale*(FLONUM)(sin_real_heading*turtle_half_bottom);
    delta_y = y_scale*(FLONUM)(cos_real_heading*turtle_half_bottom);
 
    left_x = g_round(turtle_x - delta_x);
    left_y = g_round(turtle_y + delta_y);
 
    right_x = g_round(turtle_x + delta_x);
    right_y = g_round(turtle_y - delta_y);
 
    top_x = g_round(turtle_x + x_scale*(FLONUM)(cos_real_heading*turtle_side));
    top_y = g_round(turtle_y + y_scale*(FLONUM)(sin_real_heading*turtle_side));
 
    /* move to right, draw to left, draw to top, draw to right */
    move_to(screen_x_center + right_x, screen_y_center - right_y);
    line_to(screen_x_center + left_x, screen_y_center - left_y);
    line_to(screen_x_center + top_x, screen_y_center - top_y);
    line_to(screen_x_center + right_x, screen_y_center - right_y);
 
    restore_pen(&saved_pen);
    done_drawing_turtle;
    done_drawing;
}

/************************************************************/
void forward_helper(FLONUM);
BOOLEAN safe_to_save(void);
void save_line(void), save_move(void), save_vis(void), save_mode(void);
void save_color(void), save_size(void), save_pattern(void);
void save_string(char *, int);
void save_arc(FLONUM, FLONUM, FLONUM, FLONUM, FLONUM, FLONUM, FLONUM, FLONUM);

void right(FLONUM a) {
    prepare_to_draw;
    draw_turtle();
    turtle_heading += a;
    turtle_heading = pfmod(turtle_heading,360.0);
    draw_turtle();
    done_drawing;
}

NODE *numeric_arg(NODE *args) {
    NODE *arg = car(args), *val;

    val = cnv_node_to_numnode(arg);
    while (val == UNBOUND && NOT_THROWING) {
	setcar(args, err_logo(BAD_DATA, arg));
	arg = car(args);
	val = cnv_node_to_numnode(arg);
    }
    setcar(args,val);
    return(val);
}

NODE *lright(NODE *arg) {
    NODE *val;
    FLONUM a;
    
    val = numeric_arg(arg);
    if (NOT_THROWING) {
	if (nodetype(val) == INT)
	    a = (FLONUM)getint(val);
	else
	    a = getfloat(val);
	right(a);
    }
    return(UNBOUND);
}

NODE *lleft(NODE *arg) {
    NODE *val;
    FLONUM a;
    
    val = numeric_arg(arg);
    if (NOT_THROWING) {
	if (nodetype(val) == INT)
	    a = (FLONUM)getint(val);
	else
	    a = getfloat(val);
	right(-a);
    }
    return(UNBOUND);
}

FLONUM wrap_right(FLONUM, FLONUM, FLONUM, FLONUM, FLONUM);
FLONUM wrap_left(FLONUM, FLONUM, FLONUM, FLONUM, FLONUM);
FLONUM wrap_up(FLONUM, FLONUM, FLONUM, FLONUM, FLONUM);
FLONUM wrap_down(FLONUM, FLONUM, FLONUM, FLONUM, FLONUM);

void forward(FLONUM d) {
   // #ifndef WIN32
    internal_hideturtle();
   // #endif
    prepare_to_draw;
    draw_turtle();
    forward_helper(d);
    draw_turtle();
    done_drawing;
    wanna_x = turtle_x;
    wanna_y = turtle_y;
    out_of_bounds = FALSE;
}

void forward_helper(FLONUM d) {
    FLONUM real_heading, dx, dy, x1, y1, x2, y2, newd = 0.0;
    FIXNUM rx2, ry2;

wraploop:
    if (newd != 0.0) d = newd;
    real_heading = -turtle_heading + 90.0;
    x1 = screen_x_coord;
    y1 = screen_y_coord;
    dx = (FLONUM)(cos((FLONUM)(real_heading*degrad))*d*x_scale);
    dy = (FLONUM)(sin((FLONUM)(real_heading*degrad))*d*y_scale);
    if ((dx < 0 && dx > -0.000001) || (dx > 0 && dx < 0.000001)) dx = 0;
    if ((dy < 0 && dy > -0.000001) || (dy > 0 && dy < 0.000001)) dy = 0;
    x2 = x1 + dx;
    y2 = y1 - dy;
    move_to(g_round(x1), g_round(y1));
    save_move();
    
    if (!drawing_turtle && check_throwing) return;

    if (internal_penmode == PENMODE_REVERSE && pen_vis == 0 && d > 0.0) {
	line_to(g_round(x1), g_round(y1));	/* flip the corner */
	save_line();
    }

    rx2 = g_round(x2);
    ry2 = g_round(y2);
    
    if (!drawing_turtle && check_throwing) return;

    if (current_mode == windowmode ||
	(rx2 >= screen_left && rx2 <= screen_right &&
	 ry2 >= screen_top && ry2 <= screen_bottom)) {
	turtle_x = turtle_x + dx;
	turtle_y = turtle_y + dy;
	line_to(rx2, ry2);
	save_line();
    } else {
	if (rx2 > screen_right && g_round(x1) == screen_right) {
	    move_to(screen_left, g_round(y1));
	    turtle_x = turtle_left_max;
	    goto wraploop;
	}
	if (ry2 < screen_top && g_round(y1) == screen_top) {
	    move_to(g_round(x1), screen_bottom);
	    turtle_y = turtle_bottom_max;
	    goto wraploop;
	}
	if (rx2 < screen_left && g_round(x1) == screen_left) {
	    move_to(screen_right, g_round(y1));
	    turtle_x = turtle_right_max;
	    goto wraploop;
	}
	if (ry2 > screen_bottom && g_round(y1) == screen_bottom) {
	    move_to(g_round(x1), screen_top);
	    turtle_y = turtle_top_max;
	    goto wraploop;
	}
	if ((newd = wrap_right(d, x1, y1, x2, y2)) != 0.0) goto wraploop;
	if ((newd = wrap_left(d, x1, y1, x2, y2)) != 0.0) goto wraploop;
	if ((newd = wrap_up(d, x1, y1, x2, y2)) != 0.0) goto wraploop;
	if ((newd = wrap_down(d, x1, y1, x2, y2)) != 0.0) goto wraploop;
    }

    if (internal_penmode == PENMODE_REVERSE && pen_vis == 0 && d < 0.0) {
	line_to(g_round(screen_x_coord), g_round(screen_y_coord));
	save_line();
    }
}

FLONUM wrap_right(FLONUM d, FLONUM x1, FLONUM y1, FLONUM x2, FLONUM y2) {
    FLONUM yi, newd;
    FIXNUM ryi;
    
    if (x2 > screen_right + 0.5) {
	yi = ((y2 - y1)/(x2 - x1)) * (screen_right + 1 - x1) + y1;
	ryi = g_round(yi);
	if (ryi >= screen_top && ryi <= screen_bottom) {
	    line_to(screen_right, ryi);
	    save_line();
	    turtle_x = turtle_left_max;
	    turtle_y = screen_y_center - yi;
	    if (current_mode == wrapmode) {
		newd = d * ((x2 - screen_right - 1)/(x2 - x1));
		if (newd*d > 0) return(newd);
	    } else {
		turtle_x = turtle_right_max;
		err_logo(TURTLE_OUT_OF_BOUNDS, NIL);
	    }
	}
    }
    return(0.0);
}

FLONUM wrap_left(FLONUM d, FLONUM x1, FLONUM y1, FLONUM x2, FLONUM y2) {
    FLONUM yi, newd;
    FIXNUM ryi;
    
    if (x2 < screen_left - 0.5) {
	yi = ((y1 - y2)/(x2 - x1)) * (x1 + 1 - screen_left) + y1;
	ryi = g_round(yi);
	if (ryi >= screen_top && ryi <= screen_bottom) {
	    line_to(screen_left, ryi);
	    save_line();
	    turtle_x = turtle_right_max;
	    turtle_y = screen_y_center - yi;
	    if (current_mode == wrapmode) {
		newd = d * ((x2 + 1 - screen_left)/(x2 - x1));
		if (newd*d > 0) return(newd);
	    } else {
		turtle_x = turtle_left_max;
		err_logo(TURTLE_OUT_OF_BOUNDS, NIL);	    
	    }
	}
    }
    return(0.0);
}

FLONUM wrap_up(FLONUM d, FLONUM x1, FLONUM y1, FLONUM x2, FLONUM y2) {
    FLONUM xi, newd;
    FIXNUM rxi;

    if (y2 < screen_top - 0.5) {
	xi = ((x2 - x1)/(y1 - y2)) * (y1 + 1 - screen_top) + x1;
	rxi = g_round(xi);
	if (rxi >= screen_left && rxi <= screen_right) {
	    line_to(rxi, screen_top);
	    save_line();
	    turtle_x = xi - screen_x_center;
	    turtle_y = turtle_bottom_max;
	    if (current_mode == wrapmode) {
		newd = d * ((y2 + 1 - screen_top)/(y2 - y1));
		if (newd*d > 0) return(newd);
	    } else {
		turtle_y = turtle_top_max;
		err_logo(TURTLE_OUT_OF_BOUNDS, NIL);
	    }
	} else if (rxi >= (screen_left-1) && rxi <= (screen_right+1)) {
	    rxi = (rxi > screen_right ? screen_right : screen_left);
	    line_to(rxi, screen_top);
	    save_line();
	    turtle_x = xi - screen_x_center;
	    turtle_y = turtle_bottom_max;
	    if (current_mode == wrapmode) {
		newd = d * ((y2 + 1 - screen_top)/(y2 - y1));
		if (newd*d > 0) return(newd);
	    } else {
		turtle_y = turtle_top_max;
		err_logo(TURTLE_OUT_OF_BOUNDS, NIL);
	    }
	}
    }
    return(0.0);
}

FLONUM wrap_down(FLONUM d, FLONUM x1, FLONUM y1, FLONUM x2, FLONUM y2) {
    FLONUM xi, newd;
    FIXNUM rxi;
    
    if (y2 > screen_bottom + 0.5) {
	xi = ((x2 - x1)/(y2 - y1)) * (screen_bottom + 1 - y1) + x1;
	rxi = g_round(xi);
	if (rxi >= screen_left && rxi <= screen_right) {
	    line_to(rxi, screen_bottom);
	    save_line();
	    turtle_x = xi - screen_x_center;
	    turtle_y = turtle_top_max;
	    if (current_mode == wrapmode) {
		newd = d * ((y2 - screen_bottom - 1)/(y2 - y1));
		if (newd*d > 0) return(newd);
	    } else {
		turtle_y = turtle_bottom_max;
		err_logo(TURTLE_OUT_OF_BOUNDS, NIL);
	    }
	} else if (rxi >= (screen_left-1) && rxi <= (screen_right+1)) {
	    rxi = (rxi > screen_right ? screen_right : screen_left);
	    line_to(rxi, screen_bottom);
	    save_line();
	    turtle_x = xi - screen_x_center;
	    turtle_y = turtle_top_max;
	    if (current_mode == wrapmode) {
		newd = d * ((y2 - screen_bottom - 1)/(y2 - y1));
		if (newd*d > 0) return(newd);
	    } else {
		turtle_y = turtle_bottom_max;
		err_logo(TURTLE_OUT_OF_BOUNDS, NIL);
	    }
	}
    }
    return(0.0);
}

FLONUM get_number(NODE *arg) {
    NODE *val = numeric_arg(arg);

    if (NOT_THROWING) {
	if (nodetype(val) == INT)
	    return (FLONUM)getint(val);
	else
	    return getfloat(val);
    } else return 0.0;
}


NODE *lforward(NODE *arg) {
    FLONUM d = get_number(arg);

    if (NOT_THROWING) {
	forward(d);
    }
    return(UNBOUND);
}

NODE *lback(NODE *arg) {
    FLONUM d = get_number(arg);

    if (NOT_THROWING) {
	forward(-d);
    }
    return(UNBOUND);
}

NODE *lshowturtle(NODE *args) {
    if(!graphics_setup) graphics_setup++;
    prepare_to_draw;
    if (!turtle_shown) {
	turtle_shown = TRUE;
	draw_turtle();
    }
    done_drawing;
    user_turtle_shown = TRUE;
    return(UNBOUND);
}

void internal_hideturtle() {
    if(!graphics_setup) graphics_setup++;
    prepare_to_draw;
    if (turtle_shown) {
	draw_turtle();
	turtle_shown = FALSE;
    }
    done_drawing;
}

NODE *lhideturtle(NODE *args) {
    internal_hideturtle();
    user_turtle_shown = FALSE;
    return(UNBOUND);
}

void fix_turtle_shownness() {
    if (graphics_setup
#if !defined(x_window) || defined(HAVE_WX)
		    && screen_mode != SCREEN_TEXT
#endif
	) {
      if(user_turtle_shown)
	(void)lshowturtle(NIL);
    }
}

NODE *lshownp(NODE *args) {
    return(user_turtle_shown ? TrueName() : FalseName());
}

NODE *lsetheading(NODE *arg) {
    NODE *val;
    
    val = numeric_arg(arg);
    if (NOT_THROWING) {
	prepare_to_draw;
	draw_turtle();
	if (nodetype(val) == INT)
	    turtle_heading = (FLONUM)getint(val);
	else
	    turtle_heading = getfloat(val);
	turtle_heading = pfmod(turtle_heading,360.0);
	draw_turtle();
	done_drawing;
    }
    return(UNBOUND);
}

NODE *lheading(NODE *args) {
    return(make_floatnode(turtle_heading));
}

NODE *vec_arg_helper(NODE *args, BOOLEAN floatok, BOOLEAN three) {
    NODE *arg = car(args), *val1, *val2, *val3 = NIL;

    while (NOT_THROWING) {
	if (arg != NIL &&
	is_list(arg) &&
	cdr(arg) != NIL &&
	(three ? (cddr(arg) != NIL && cdr(cddr(arg)) == NIL) : cddr(arg) == NIL)) {
	    val1 = cnv_node_to_numnode(car(arg));
	    val2 = cnv_node_to_numnode(cadr(arg));
		if (three) val3 = cnv_node_to_numnode(car(cddr(arg)));
	    if (val1 != UNBOUND && val2 != UNBOUND &&
		(floatok || (nodetype(val1) == INT && getint(val1) >= 0 &&
			     nodetype(val2) == INT && getint(val2) >= 0 &&
				 (!three || (nodetype(val3) == INT && getint(val3) >= 0))))) {
		setcar(arg, val1);
		setcar(cdr(arg), val2);
		if (three) setcar (cddr(arg), val3);
		return(arg);
	    }
	}
	setcar(args, err_logo(BAD_DATA, arg));
	arg = car(args);
    }
    return(UNBOUND);
}

NODE *vector_arg(NODE *args) {
    return vec_arg_helper(args,TRUE,FALSE);
}

NODE *pos_int_vector_arg(NODE *args) {
    return vec_arg_helper(args,FALSE,FALSE);
}

NODE *rgb_arg(NODE *args) {
	return vec_arg_helper(args,TRUE,TRUE);
}

FLONUM towards_helper(FLONUM x, FLONUM y, FLONUM from_x, FLONUM from_y) {
    FLONUM m, a, tx, ty;

    tx = from_x/x_scale;
    ty = from_y/y_scale;

    if (x != tx || y != ty) {
	if (x == tx)
	    a = (y < ty) ? -90 : 90;
	else {
	    m = (y - ty)/(x - tx);
	    a = atan(m)/degrad;
	    if (x < tx) a = fmod(a + 180.0,360.0);
	}
	a = -(a - 90.0);
	return (a < 0 ? 360.0+a : a);
    }
    return 0.0;
}

NODE *ltowards(NODE *args) {
    NODE *xnode, *ynode = UNBOUND, *arg;
    FLONUM x, y;
    
    arg = vector_arg(args);
    if (NOT_THROWING) {
	xnode = car(arg);
	ynode = cadr(arg);
	
	x = ((nodetype(xnode) == FLOATT) ? getfloat(xnode) :
			  (FLONUM)getint(xnode));
	y = ((nodetype(ynode) == FLOATT) ? getfloat(ynode) :
			  (FLONUM)getint(ynode));
	return make_floatnode(towards_helper(x, y, turtle_x, turtle_y));
    }
    return(UNBOUND);
}

NODE *lpos(NODE *args) {
    return(cons(make_floatnode(cut_error(turtle_x/x_scale)),
	cons(make_floatnode(cut_error(turtle_y/y_scale)), NIL)));
}

NODE *lscrunch(NODE *args) {
    return(cons(make_floatnode(x_scale), cons(make_floatnode(y_scale), NIL)));
}

NODE *lhome(NODE *args) {
    prepare_to_draw;
    out_of_bounds = FALSE;
    setpos_bynumber((FLONUM)0.0, (FLONUM)0.0);
    draw_turtle();
    turtle_heading = 0.0;
    draw_turtle();
    done_drawing;
    return(UNBOUND);
}

void cs_helper(int centerp) {    
#if defined(x_window) && !HAVE_WX
    clearing_screen++;
#endif
    prepare_to_draw;
    clear_screen;
#if defined(x_window) && !HAVE_WX
    clearing_screen==0;
#endif
    if (centerp) {
	wanna_x = wanna_y = turtle_x = turtle_y = turtle_heading = 0.0;
	out_of_bounds = FALSE;
	move_to(screen_x_coord, screen_y_coord);
    }
    if (!graphics_setup) {
	graphics_setup++;
        turtle_shown = TRUE;
    }
    draw_turtle();
    save_pen(&orig_pen);
    p_info_x(orig_pen) = g_round(screen_x_coord);
    p_info_y(orig_pen) = g_round(screen_y_coord);
    (void)safe_to_save();
    record = record_buffer;
    record_index = One;
    last_recorded = -1;
    record[record_index] = FINISHED;
    if (turtle_x != 0.0 || turtle_y != 0.0) save_move();
    if (pen_vis != 0) save_vis();
    if (internal_penmode != PENMODE_PAINT) save_mode();
    if (pen_color != 7) save_color();
    if (pen_width != 1 || pen_height != 1) save_size();
    done_drawing;
}

NODE *lclearscreen(NODE *args) {
    cs_helper(TRUE);
    return(UNBOUND);
}

NODE *lclean(NODE *args) {
    cs_helper(FALSE);
    return(UNBOUND);
}

void setpos_commonpart(FLONUM target_x, FLONUM target_y) {
    FLONUM scaled_x, scaled_y, tx, ty, save_heading;
    BOOLEAN wrapping = FALSE;
    
    if (NOT_THROWING) {
	scaled_x = target_x * x_scale;
	scaled_y = target_y * y_scale;
	wrapping = scaled_x > turtle_right_max || scaled_x < turtle_left_max ||
		   scaled_y > turtle_top_max || scaled_y < turtle_bottom_max;
	if (current_mode == fencemode && wrapping)
	    err_logo(TURTLE_OUT_OF_BOUNDS, NIL);
	else if (current_mode == wrapmode && (wrapping || out_of_bounds)) {
	    save_heading = turtle_heading;
	    turtle_heading = towards_helper(target_x, target_y,
					    wanna_x, wanna_y);
	    tx = wanna_x/x_scale;
	    ty = wanna_y/y_scale;
#define sq(z) ((z)*(z))
	    forward_helper(sqrt(sq(target_x - tx) + sq(target_y - ty)));
	    turtle_heading = save_heading;
	    wanna_x = scaled_x;
	    wanna_y = scaled_y;
	    out_of_bounds = wrapping;
	}
	else {
	    out_of_bounds = FALSE;
	    wanna_x = turtle_x = scaled_x;
	    wanna_y = turtle_y = scaled_y;
	    line_to(g_round(screen_x_coord),
		    g_round(screen_y_coord));
	    save_line();
	}
	draw_turtle();
	done_drawing;
    }
}

void setpos_bynumber(FLONUM target_x, FLONUM target_y) {
    if (NOT_THROWING) {
	prepare_to_draw;
	draw_turtle();
	move_to(g_round(screen_x_coord), g_round(screen_y_coord));
	setpos_commonpart(target_x, target_y);
	done_drawing;
    }
}

void setpos_helper(NODE *xnode, NODE *ynode) {
    FLONUM target_x, target_y;
    
    if (NOT_THROWING) {
	internal_hideturtle();
	prepare_to_draw;
	draw_turtle();
	move_to(g_round(screen_x_coord), g_round(screen_y_coord));
	target_x = ((xnode == NIL) ?
		turtle_x/x_scale :
		((nodetype(xnode) == FLOATT) ? getfloat(xnode) :
		 (FLONUM)getint(xnode)));
	target_y = ((ynode == NIL) ?
		turtle_y/y_scale :
		((nodetype(ynode) == FLOATT) ? getfloat(ynode) :
		 (FLONUM)getint(ynode)));
	setpos_commonpart(target_x, target_y);
	done_drawing;
    }
}

NODE *lsetpos(NODE *args) {
    NODE *arg = vector_arg(args);

    if (NOT_THROWING) {
	setpos_helper(car(arg), cadr(arg));
    }
    return(UNBOUND);
}

NODE *lsetxy(NODE *args) {
    NODE *xnode, *ynode;
    
    xnode = numeric_arg(args);
    ynode = numeric_arg(cdr(args));
    if (NOT_THROWING) {
	setpos_helper(xnode, ynode);
    }
    return(UNBOUND);
}

NODE *lsetx(NODE *args) {
    NODE *xnode;
    
    xnode = numeric_arg(args);
    if (NOT_THROWING) {
	setpos_helper(xnode, NIL);
    }
    return(UNBOUND);
}

NODE *lsety(NODE *args) {
    NODE *ynode;
    
    ynode = numeric_arg(args);
    if (NOT_THROWING) {
	setpos_helper(NIL, ynode);
    }
    return(UNBOUND);
}

NODE *lwrap(NODE *args) {
    prepare_to_draw;
    draw_turtle();
    current_mode = wrapmode;
    while (turtle_x > turtle_right_max) {
	turtle_x -= screen_width;
    }
    while (turtle_x < turtle_left_max) {
	turtle_x += screen_width;
    }
    while (turtle_y > turtle_top_max) {
	turtle_y -= screen_height;
    }
    while (turtle_y < turtle_bottom_max) {
	turtle_y += screen_height;
    }
    move_to(screen_x_coord, screen_y_coord);
    draw_turtle();
    done_drawing;
    return(UNBOUND);
}

NODE *lfence(NODE *args) {
    (void)lwrap(args);	    /* get turtle inside the fence */
    prepare_to_draw;
    draw_turtle();
    current_mode = fencemode;
    draw_turtle();
    done_drawing;
    return(UNBOUND);
}

NODE *lwindow(NODE *args) {
    prepare_to_draw;
    draw_turtle();
    current_mode = windowmode;
    draw_turtle();
    done_drawing;
    return(UNBOUND);
}


NODE *lturtlemode(NODE *args) {
    switch (current_mode) {
	case wrapmode: return(theName(Name_wrap));
	case fencemode: return(theName(Name_fence));
	case windowmode: return(theName(Name_window));
    }
    return(UNBOUND);	/* Can't get here, but makes compiler happy */
}

NODE *lfill(NODE *args) {    
    prepare_to_draw;
    draw_turtle();
    logofill();
    draw_turtle();
    if (safe_to_save()) {
	save_move();
	last_recorded = record[record_index] = FILLERUP;
	record_index += One;
	record[record_index] = FINISHED;
    }
    done_drawing;
    return(UNBOUND);

}

NODE *llabel(NODE *arg) {
    char textbuf[300];
    short theLength;
    char *old_stringptr = print_stringptr;
    int old_stringlen = print_stringlen;

    print_stringptr = textbuf;
    print_stringlen = 300;
    ndprintf((FILE *)NULL,"%p",car(arg));
    *print_stringptr = '\0';
    print_stringptr = old_stringptr;
    print_stringlen = old_stringlen;
	
    if (NOT_THROWING) {
	prepare_to_draw;
	draw_turtle();
	theLength = strlen(textbuf);
#ifdef mac
	c_to_pascal_string(textbuf, theLength);
#endif
	label(textbuf);
	save_string(textbuf,theLength);
	draw_turtle();
	done_drawing;
    }
    return(UNBOUND);
}

NODE *ltextscreen(NODE *args) {
    text_screen;
    screen_mode = SCREEN_TEXT;
    return(UNBOUND);
}

NODE *lsplitscreen(NODE *args) {
    if (!graphics_setup) {
	graphics_setup++;
//        turtle_shown = TRUE;
    }
    split_screen;
    screen_mode = SCREEN_SPLIT;
    return(UNBOUND);
}

NODE *lfullscreen(NODE *args) {
    if (!graphics_setup) {
	graphics_setup++;
//        turtle_shown = TRUE;
    }
    full_screen;
    screen_mode = SCREEN_FULL;
    return(UNBOUND);
}

NODE *lscreenmode(NODE *args) {
    switch (screen_mode) {
	case SCREEN_TEXT: return(theName(Name_textscreen));
	case SCREEN_SPLIT: return(theName(Name_splitscreen));
	case SCREEN_FULL: return(theName(Name_fullscreen));
    }
    return(UNBOUND);	/* Can't get here, but makes compiler happy */
}

NODE *lpendownp(NODE *args) {
    return(pen_vis == 0 ? TrueName() : FalseName());
}

NODE *lpencolor(NODE *args) {
    if (pen_color == PEN_COLOR_OFFSET)
	return lpalette(cons(make_intnode(PEN_COLOR_OFFSET),NIL));
    return(make_intnode((FIXNUM)pen_color));
}

NODE *lbackground(NODE *args) {
    if (back_ground == BACKGROUND_COLOR_OFFSET)
	return lpalette(cons(make_intnode(BACKGROUND_COLOR_OFFSET),NIL));
    return(make_intnode((FIXNUM)back_ground));
}

NODE *lpensize(NODE *args) {
    return(cons(make_intnode((FIXNUM)pen_width),
	cons(make_intnode((FIXNUM)pen_height), NIL)));
}

NODE *lpenpattern(NODE *args) {
    return(get_node_pen_pattern);
}

NODE *lpendown(NODE *args) {
    pen_vis = 0;
    save_vis();
    return(UNBOUND);
}

NODE *lpenup(NODE *args) {
    if (pen_vis == 0)
	pen_vis--;
    save_vis();
    return(UNBOUND);
}

NODE *lpenpaint(NODE *args) {
    internal_penmode = PENMODE_PAINT;
    pen_down;
    save_mode();
    return(lpendown(NIL));
}

NODE *lpenerase(NODE *args) {
    internal_penmode = PENMODE_ERASE;
    pen_erase;
    save_mode();
    return(lpendown(NIL));
}

NODE *lpenreverse(NODE *args) {
    internal_penmode = PENMODE_REVERSE;
    pen_reverse;
    save_mode();
    return(lpendown(NIL));
}

NODE *lpenmode(NODE *args) {
    switch(internal_penmode) {
	case PENMODE_PAINT: return theName(Name_paint);
	case PENMODE_ERASE: return theName(Name_erase);
	case PENMODE_REVERSE: return theName(Name_reverse);
    }
    return(UNBOUND);	/* Can't get here, but makes compiler happy */
}

NODE *lsetpencolor(NODE *arg) {
    NODE *val;

    if (NOT_THROWING) {
	prepare_to_draw;
	if (is_list(car(arg))) {
	    val = make_intnode(PEN_COLOR_OFFSET);
	    lsetpalette(cons(val,arg));
	} else
	    val = pos_int_arg(arg);
	set_pen_color(getint(val));
	save_color();
	done_drawing;
    }
    return(UNBOUND);
}

NODE *lsetbackground(NODE *arg) {
    NODE *val;

    if (!graphics_setup) {
	graphics_setup++;
	turtle_shown=TRUE;
    }

    if (NOT_THROWING) {
	prepare_to_draw;
	if (is_list(car(arg))) {
	    val = make_intnode(BACKGROUND_COLOR_OFFSET);
	    lsetpalette(cons(val,arg));
	} else
	    val = pos_int_arg(arg);
	set_back_ground(getint(val));
	done_drawing;
    }
    return(UNBOUND);
}

NODE *lsetpalette(NODE *args) {
	NODE *slot = integer_arg(args);
	NODE *arg = rgb_arg(cdr(args));
	int slotnum = (int)getint(slot);

	if (slotnum < -SPECIAL_COLORS) {
	    err_logo(BAD_DATA_UNREC, slot);
	} else if (NOT_THROWING && ((slotnum > 7) || (slotnum < 0))) {
//		prepare_to_draw;
		set_palette(slotnum,
		    (unsigned int)(get_number(arg)*65535/100.0),
		    (unsigned int)(get_number(cdr(arg))*65535/100.0),
		    (unsigned int)(get_number(cddr(arg))*65535/100.0));
		if (pen_color == slotnum) {
		    set_pen_color(slotnum);
		}
//		done_drawing;
		if (slotnum > max_palette_slot) max_palette_slot = slotnum;
	}
	return(UNBOUND);
}

NODE *make_rgbnode(unsigned int val) {
    FLONUM result=val * 100.0 / 65535.0;

    if (result == g_round(result)) 
	return make_intnode((FIXNUM)result);
    else
	return make_floatnode(result);
}

NODE *lpalette(NODE *args) {
    NODE *arg = integer_arg(args);
    unsigned int r=0, g=0, b=0;

    if (getint(arg) < -SPECIAL_COLORS) err_logo(BAD_DATA_UNREC, arg);
    if (NOT_THROWING) {
	get_palette((int)getint(arg), &r, &g, &b);
	return cons(make_rgbnode(r), 
			cons(make_rgbnode(g),
				cons(make_rgbnode(b), NIL)));
    }
    return UNBOUND;
}

void save_palette(FILE *fp) {
    unsigned int colors[3];
    int i;
    fwrite(&max_palette_slot, sizeof(int), 1, fp);
    for (i=8; i <= max_palette_slot; i++) {
	get_palette(i, &colors[0], &colors[1], &colors[2]);
	fwrite(colors, sizeof(int), 3, fp);
    }
}

void restore_palette(FILE *fp) {
    unsigned int colors[3];
    int i, nslots;
    fread(&nslots, sizeof(int), 1, fp);
    if (nslots > max_palette_slot) max_palette_slot = nslots;
    for (i=8; i <= nslots; i++) {
	fread(colors, sizeof(int), 3, fp);
	set_palette(i, colors[0], colors[1], colors[2]);
    }
}

NODE *lsetpensize(NODE *args) {
    NODE *arg;

    prepare_to_draw;
    if (is_list(car(args))) {
	arg = pos_int_vector_arg(args);
	if (NOT_THROWING) {
	    set_pen_width((int)getint(car(arg)));
	    set_pen_height((int)getint(cadr(arg)));
	}
    } else {    /* 5.5 accept single number for [n n] */
	arg = pos_int_arg(args);
	if (NOT_THROWING) {
	    set_pen_width((int)getint(arg));
	    set_pen_height((int)getint(arg));
	}
    }
    if (NOT_THROWING)
	save_size();
    done_drawing;
    return(UNBOUND);
}

NODE *lsetpenpattern(NODE *args) {    
    NODE *arg;

    arg = car(args);
    while ((!is_list(arg)) && NOT_THROWING)
	arg = err_logo(BAD_DATA, arg);
	
    if (NOT_THROWING) {
	prepare_to_draw;
	set_list_pen_pattern(arg);
	save_pattern();
	done_drawing;
    }

    return(UNBOUND);
}

NODE *lsetscrunch(NODE *args) {
    NODE *xnode, *ynode;

    xnode = numeric_arg(args);
    ynode = numeric_arg(cdr(args));

    if (NOT_THROWING) {
	prepare_to_draw;
	draw_turtle();
	x_scale = (nodetype(xnode) == FLOATT) ? getfloat(xnode) :
			       (FLONUM)getint(xnode);
	y_scale = (nodetype(ynode) == FLOATT) ? getfloat(ynode) :
			       (FLONUM)getint(ynode);
	draw_turtle();
	done_drawing;
#ifdef __RZTC__
	{
	    FILE *fp = fopen("scrunch.dat","r");
	    if (fp != NULL) {
		fclose(fp);
		fp = fopen("scrunch.dat","w");
		if (fp != NULL) {
		    fwrite(&x_scale, sizeof(FLONUM), 1, fp);
		    fwrite(&y_scale, sizeof(FLONUM), 1, fp);
		    fclose(fp);
		}
	    }
	}
#endif
#ifdef HAVE_WX
	//update the label height!

	wx_adjust_label_height();
#endif
    }
    return(UNBOUND);
}

NODE *lmousepos(NODE *args) {
    return(cons(make_floatnode(mouse_x/x_scale),
		cons(make_floatnode(mouse_y/y_scale), NIL)));
}

#ifdef HAVE_WX

NODE *lclickpos(NODE *args) {
    return(cons(make_floatnode(click_x/x_scale),
		cons(make_floatnode(click_y/y_scale), NIL)));
}

#endif

NODE *lbuttonp(NODE *args) {

    if (button)
	return(TrueName());
    return(FalseName());
}

NODE *lbutton(NODE *args) {
#ifdef HAVE_WX
    return(make_intnode(lastbutton));
#else
    return(make_intnode(button));
#endif
}

NODE *ltone(NODE *args) {
    NODE *p, *d;
    FIXNUM pitch, duration;
    
    p = numeric_arg(args);
    d = numeric_arg(cdr(args));
    
    if (NOT_THROWING) {
	pitch = (nodetype(p) == FLOATT) ? (FIXNUM)getfloat(p) : getint(p);
	duration = (nodetype(d) == FLOATT) ? (FIXNUM)getfloat(d) : getint(d);
	if (pitch > 0) tone(pitch, duration);
}
    return(UNBOUND);
}

void do_arc(FLONUM count, FLONUM ang, FLONUM radius, FLONUM delta,
	    FLONUM tx, FLONUM ty, FLONUM angle, FLONUM thead, BOOLEAN save) {
    FLONUM x;
    FLONUM y;
    FLONUM i;
    BOOLEAN save_refresh = refresh_p;
    FLONUM save_x = turtle_x;
    FLONUM save_y = turtle_y;
    int pen_state;

    refresh_p = 0;
    if (save) {
	x = sin(ang*3.141592654/180.0)*radius;
	y = cos(ang*3.141592654/180.0)*radius;
	turtle_x = tx+x*x_scale;
	turtle_y = ty+y*y_scale;
    }

    /* draw each line segment of arc (will do wrap) */

    for (i=1.0;i<=count;i=i+1.0) {

       /* calc x y */

       x = sin(ang*3.141592654/180.0)*radius;
       y = cos(ang*3.141592654/180.0)*radius;
       setpos_bynumber(tx/x_scale+x, ty/y_scale+y);
       ang = ang + delta;
    }

    /* assure we draw something and end in the exact right place */

    x = sin((thead+angle)*3.141592654/180.0)*radius;
    y = cos((thead+angle)*3.141592654/180.0)*radius;

    setpos_bynumber(tx/x_scale+x, ty/y_scale+y);

    if (save) {
	pen_state = pen_vis;
	pen_vis = -1;
	setpos_bynumber(tx/x_scale, ty/y_scale);
	pen_vis = pen_state;
	turtle_x = save_x;
	turtle_y = save_y;
    }
    refresh_p = save_refresh;
}

NODE *larc(NODE *arg) {
    NODE *val1;
    NODE *val2;

    FLONUM angle;
    FLONUM radius;
    FLONUM ang;
    FLONUM tx;
    FLONUM ty;
    FLONUM count;
    FLONUM delta;
    FLONUM x;
    FLONUM y;

    int turtle_state;
    int pen_state;
    
    /* get args */

    val1 = numeric_arg(arg);
    val2 = numeric_arg(cdr(arg));

    if (NOT_THROWING) {

	if (nodetype(val1) == INT)
	    angle = (FLONUM)getint(val1);
	else
	    angle = getfloat(val1);

	if (nodetype(val2) == INT)
	    radius = (FLONUM)getint(val2);
	else
	    radius = getfloat(val2);

	internal_hideturtle();
	prepare_to_draw;
	draw_turtle();

	/* save and force turtle state */

	turtle_state = turtle_shown;
	turtle_shown = 0;

	/* grab things before they change and use for restore */

	ang = turtle_heading;
	tx = turtle_x;
	ty = turtle_y;

	/* calculate resolution parameters */

	if (angle > 360.0) angle = 360.0+fmod(angle, 360.0);
	if (angle < -360.0) angle = -(360.0+fmod(-angle, 360.0));
	count = fabs(angle*radius/200.0);	/* 4.5 */
	if (count == 0.0) count = 1.0;
	delta = angle/count;

	/* jump to begin of first line segment without drawing */

	x = sin(ang*3.141592654/180.0)*radius;
	y = cos(ang*3.141592654/180.0)*radius;

	pen_state = pen_vis;
	pen_vis = -1;
	save_vis();
	setpos_bynumber(tx/x_scale+x, ty/y_scale+y);
	pen_vis = pen_state;
	save_vis();
	ang = ang + delta;

	save_arc(count, ang, radius, delta, tx, ty, angle, turtle_heading);

	do_arc(count, ang, radius, delta, tx, ty, angle, turtle_heading, 0);

	/* restore state */

	pen_state = pen_vis;
	pen_vis = -1;
	save_vis();
	setpos_bynumber(tx/x_scale, ty/y_scale);
	pen_vis = pen_state;
	save_vis();

	turtle_shown = turtle_state;
	draw_turtle();
	wanna_x = turtle_x;
	wanna_y = turtle_y;
	out_of_bounds = FALSE;
	pen_state = pen_vis;
	pen_vis = -1;
	save_vis();
	forward_helper((FLONUM)0.0);    /* Lets fill work -- dunno why */
	pen_vis = pen_state;
	save_vis();

	done_drawing;
    }
    return(UNBOUND);
}

#ifdef HAVE_WX

int insidefill = 0;

struct mypoint {
    int x,y;
};

NODE *lfilled(NODE *args) {
    NODE *val, *arg;
    char *start, *ptr;
    int start_idx, idx, count, color;
    unsigned int r,g,b;
    struct mypoint *points, *point;
    FLONUM x1,y1,lastx,lasty;
    int old_refresh = refresh_p;

    prepare_to_draw;
    if (is_list(car(args))) {
	val = make_intnode(FILLED_COLOR_OFFSET);
	lsetpalette(cons(val,args));
    } else
	val = pos_int_arg(args);
    done_drawing;
    color = getint(val);

    old_refresh = refresh_p;
    refresh_p = 1;  /* have to save polygon to fill it */

    if (!safe_to_save() || insidefill) {
	err_logo(BAD_GRAPH_INIT, NIL);
	refresh_p = old_refresh;
	return UNBOUND;
    }

    insidefill = 1;
    start = record;
    start_idx = record_index;
    x1 = screen_x_coord;
    y1 = screen_y_coord;

    arg = runnable_arg(cdr(args));
    if (NOT_THROWING) {
	last_recorded = record[record_index] = STARTFILL;
	record_index += Three;	/* Will be filled in at endfill */
	record[record_index] = FINISHED;
	if (color == FILLED_COLOR_OFFSET) {
	    get_palette(FILLED_COLOR_OFFSET, &r, &g, &b);
	    last_recorded = record[record_index] = COLORFILL;
	    *(unsigned int *)(record + record_index + One) = r;
	    *(unsigned int *)(record + record_index + Two) = g;
	    *(unsigned int *)(record + record_index + Three) = b;
	    record_index += Four;
	}

	doing_filled = TRUE;
	(void)evaluator(arg, begin_line);
	doing_filled = FALSE;
    }

    insidefill = 0;

    if (!safe_to_save()) {
	err_logo(BAD_GRAPH_INIT, NIL);
	refresh_p = old_refresh;
	return UNBOUND;
    }

    if (NOT_THROWING) {
	last_recorded = record[record_index] = ENDFILL;
	*(char **)(record + record_index + One) = start;
	*(int *)(record + record_index + Two) = start_idx;
	record_index += Three;
	record[record_index] = FINISHED;
	    count=0;
	for (ptr = start, idx = start_idx; ptr[idx] != FINISHED; ) {
	switch (ptr[idx]) {
		case (LINEXY) :
		case (MOVEXY) :
		    count++;
		case (SETPENMODE) :
		case (SETPENSIZE) :
		case (STARTFILL) :
		case (ENDFILL) :
		    idx += Three;
		    break;
		case (LABEL) :
		    idx += (One+2 + ptr[idx + One] + (One-1)) & ~(One-1);
		    break;
		case (SETPENVIS) :
		case (FILLERUP) :
		    idx += One;
		    break;
		case (SETPENCOLOR) :
		    idx += Two;
		    break;
		case (SETPENRGB) :
		case (COLORFILL) :
		    idx += Four;
		    break;
		case (SETPENPATTERN) :
		    idx += One+8;
		    break;
		case (ARC) :
		    idx += 9*Big;
		    break;
		case (NEXTBUFFER):
		    ptr = *(char **)(ptr);
		    idx = One;
		    break;
	    }
	}
	point = points = malloc((count+1)*sizeof(struct mypoint));
	point->x = g_round(x1);
	point->y = g_round(y1);
	point++;
	ptr = start;
	idx = start_idx;
	*(int *)(start + start_idx + One) = count+1;
	*(int *)(start + start_idx + Two) = color;

    while (ptr[idx] != FINISHED) {
	switch (ptr[idx]) {
	    case (LINEXY) :
	    case (MOVEXY) :
		lastx = *(int *)(ptr + idx + One);
		lasty = *(int *)(ptr + idx + Two);
		point->x = screen_x_center+lastx;
		point->y = screen_y_center-lasty;
		point++;
	    case (SETPENMODE) :
	    case (SETPENSIZE) :
	    case (STARTFILL) :
	    case (ENDFILL) :
		idx += Three;
		break;
	    case (LABEL) :
		idx += (One+2 + ptr[idx + One] + (One-1)) & ~(One-1);
		break;
	    case (SETPENVIS) :
	    case (FILLERUP) :
		idx += One;
		break;
	    case (SETPENCOLOR) :
		idx += Two;
		break;
	    case (SETPENRGB) :
	    case (COLORFILL) :
		idx += Four;
		break;
	    case (SETPENPATTERN) :
		idx += One+8;
		break;
	    case (ARC) :
		idx += 9*Big;
		break;
	    case (NEXTBUFFER):
		ptr = *(char **)(ptr);
		idx = One;
		break;
	    }
	}
	doFilled(color, count+1, points);
	free(points);
    }

    refresh_p = old_refresh;
    if (!refresh_p) {
	record = start;
	record_index = start_idx;
	record[record_index] = FINISHED;
    }
    return UNBOUND;
}

NODE *lprintpict(NODE *args) {
    if (args != NIL)
	wxlPrintPreviewPict();
    else
	wxlPrintPict();
    return UNBOUND;
}

NODE *lprinttext(NODE *args) {
    if (args != NIL)
	wxlPrintPreviewText();
    else
	wxlPrintText();
    return UNBOUND;
}
#endif

/************************************************************/
/* The rest of this file implements the recording of moves in
   the graphics window and the playing back of those moves.  It's
   needed on machines like the Macintosh where the contents of the
   graphics window can get erased and need to be redrawn.  On
   machines where no graphics redrawing is necessary, set the size
   of the recording buffer to 1 in logo.h. */

BOOLEAN safe_to_save(void) {
    char *newbuf;

    if (!refresh_p || drawing_turtle) return FALSE;
    if (record == 0) {	/* first time */
	record = record_buffer;
	*(char **)(record) = 0;
	record_index = One;
	return TRUE;
    }
    if (record_index < (GR_SIZE - 300)) return TRUE;	/* room here */
    if (*(char **)(record) != 0) {    /* already allocated next one */
	*(record + record_index) = NEXTBUFFER;
	record = *(char **)(record);
	record_index = One;
	return TRUE;
    }
    newbuf = malloc(GR_SIZE);	/* get a new buffer */
    if (newbuf == NULL) return FALSE;	/* failed */
    
    *(record + record_index) = NEXTBUFFER;

    *(char **)(record) = newbuf;
    record = newbuf;
    *(char **)(record) = 0; 
    record_index = One;
    return TRUE;
}

void save_lm_helper (void) {
    *(int *)(record + record_index + One) = g_round(turtle_x);
    *(int *)(record + record_index + Two) = g_round(turtle_y);
    record_index += Three;
}

void save_line(void) {
    if (safe_to_save()) {
	last_recorded = record[record_index] = LINEXY;
	save_lm_helper();
	record[record_index] = FINISHED;
    }
}

void save_move(void) {
    if (safe_to_save()) {
	if (record_index >= Three && last_recorded == MOVEXY && !doing_filled)
	    record_index -= Three;
	last_recorded = record[record_index] = MOVEXY;
	save_lm_helper();
	record[record_index] = FINISHED;
    }
}

void save_vis(void) {
    if (safe_to_save()) {
	last_recorded = record[record_index] = SETPENVIS;
	record[record_index + 1] = pen_vis;
	record_index += One;
	record[record_index] = FINISHED;
    }
}

void save_mode(void) {
    if (safe_to_save()) {
	last_recorded = record[record_index] = SETPENMODE;
#if defined(x_window) && !HAVE_WX
	*(GC *)(record + record_index + One) = pen_mode;
#else
	*(int *)(record + record_index + One) = pen_mode;
#endif
	*(int *)(record + record_index + Two) = internal_penmode;
	record_index += Three;
	save_color();
    }
}

void save_color(void) {
    unsigned int r,g,b;

    if (safe_to_save()) {
	if (pen_color == PEN_COLOR_OFFSET) {
	    get_palette(pen_color, &r, &g, &b);
	    last_recorded = record[record_index] = SETPENRGB;
	    *(unsigned int *)(record + record_index + One) = r;
	    *(unsigned int *)(record + record_index + Two) = g;
	    *(unsigned int *)(record + record_index + Three) = b;
	    record_index += Four;
	} else {
	    last_recorded = record[record_index] = SETPENCOLOR;
	    *(int *)(record + record_index + One) = pen_color;
	    record_index += Two;
	}
	record[record_index] = FINISHED;
    }
}

void save_size(void) {
    if (safe_to_save()) {
	last_recorded = record[record_index] = SETPENSIZE;
	*(int *)(record + record_index + One) = pen_width;
	*(int *)(record + record_index + Two) = pen_height;
	record_index += Three;
	record[record_index] = FINISHED;
    }
}

void save_pattern(void) {
    if (safe_to_save()) {
	last_recorded = record[record_index] = SETPENPATTERN;
	get_pen_pattern(&record[record_index + One]);
	record_index += One+8;
	record[record_index] = FINISHED;
    }
}

void save_string(char *s, int len) {
    int count;

    if (safe_to_save()) {
	last_recorded = record[record_index] = LABEL;
	record[record_index + One] = (unsigned char)len;
	for (count = 0; count <= len; count++)
	    record[record_index + One+1 + count] = s[count];
	record[record_index + One+2 + len] = '\0';
	record_index += (One+2 + len + (One-1)) & ~(One-1);
	record[record_index] = FINISHED;
    }
}

void save_arc(FLONUM count, FLONUM ang, FLONUM radius, FLONUM delta,
	      FLONUM tx, FLONUM ty, FLONUM angle, FLONUM thead) {
    if (safe_to_save()) {
	last_recorded = record[record_index] = ARC;
	*(FLONUM *)(record + record_index + Big) = count;
	*(FLONUM *)(record + record_index + 2 * Big) = ang;
	*(FLONUM *)(record + record_index + 3 * Big) = radius;
	*(FLONUM *)(record + record_index + 4 * Big) = delta;
	*(FLONUM *)(record + record_index + 5 * Big) = tx;
	*(FLONUM *)(record + record_index + 6 * Big) = ty;
	*(FLONUM *)(record + record_index + 7 * Big) = angle;
	*(FLONUM *)(record + record_index + 8 * Big) = thead;
	record_index += 9*Big;
	record[record_index] = FINISHED;
    }
}

NODE *lrefresh(NODE *args) {
    refresh_p = TRUE;
    return(UNBOUND);
}

NODE *lnorefresh(NODE *args) {
    refresh_p = FALSE;
    return(UNBOUND);
}

void redraw_graphics(void) {
    FLONUM save_tx, save_ty, save_th;
    FIXNUM r_index = One;
    char *bufp = record_buffer;
    int lastx, lasty;
    pen_info saved_pen;
    BOOLEAN saved_shown;
#if defined(__RZTC__) && !defined(WIN32)
    BOOLEAN save_splitscreen = in_splitscreen;
#endif
#ifdef HAVE_WX
    char *start, *ptr;
    int start_idx, idx, count, color;
    unsigned int r,g,b;
    struct mypoint *points = 0, *point = 0;
#endif

    if (!refresh_p ) {
    /*	clear_screen;
	draw_turtle();	*/
	return;
    }

    prepare_to_draw;
    if(!graphics_setup){
        done_drawing;
        return;
    }
    save_tx = turtle_x;
    save_ty = turtle_y;
    save_th = turtle_heading;
    saved_shown = turtle_shown;
    turtle_shown = FALSE;
    save_pen(&saved_pen);
    restore_pen(&orig_pen);
	
#if defined(__RZTC__) && !defined(WIN32)
    full_screen;
#endif

    erase_screen();
    wanna_x = wanna_y = turtle_x = turtle_y = turtle_heading = 0.0;
    out_of_bounds = FALSE;
    move_to(screen_x_coord, screen_y_coord);
    internal_penmode = PENMODE_PAINT;
    pen_down;
    set_pen_color((FIXNUM)7);
    set_pen_width(1);
    set_pen_height(1);

#ifdef __TURBOC__
    moveto(p_info_x(orig_pen),p_info_y(orig_pen));
#endif

    lastx = lasty = 0;

    while (bufp[r_index] != FINISHED) {
    	turtle_x = (FLONUM)(lastx);
	turtle_y = (FLONUM)(lasty);
	switch (bufp[r_index]) {
	    case (LINEXY) :
		lastx = *(int *)(bufp + r_index + One);
		lasty = *(int *)(bufp + r_index + Two);
		line_to(screen_x_center+lastx, screen_y_center-lasty);
		r_index += Three;
#ifdef HAVE_WX
	    if (point != NULL) {
		point->x = screen_x_center+lastx;
		point->y = screen_y_center-lasty;
		point++;
	    }
#endif
		break;
	    case (MOVEXY) :
		lastx = *(int *)(bufp + r_index + One);
		lasty = *(int *)(bufp + r_index + Two);
		move_to(screen_x_center+lastx, screen_y_center-lasty);
		r_index += Three;
#ifdef HAVE_WX
	    if (point != NULL) {
		point->x = screen_x_center+lastx;
		point->y = screen_y_center-lasty;
		point++;
	    }
#endif
		break;
	    case (LABEL) :
 		draw_string((unsigned char *)(bufp + r_index + One+1));
		move_to(screen_x_center+lastx, screen_y_center-lasty);
		r_index += (One+2 + bufp[r_index + One] + (One-1)) & ~(One-1);
		break;
	    case (SETPENVIS) :
		set_pen_vis(bufp[r_index + 1]);
		r_index += One;
		break;
	    case (SETPENMODE) :
#if defined(x_window) && !HAVE_WX
		set_pen_mode(*(GC *)(bufp + r_index + One));
#else
		set_pen_mode(*(int *)(bufp + r_index + One));
#endif
		internal_penmode = *(int *)(bufp + r_index + Two);
		r_index += Three;
		break;
	    case (SETPENCOLOR) :
		set_pen_color((FIXNUM)(*(int *)(bufp + r_index + One)));
		r_index += Two;
		break;
	    case (SETPENRGB) :
		set_palette(PEN_COLOR_OFFSET,
				(*(int *)(bufp + r_index + One)),
				(*(int *)(bufp + r_index + Two)),
				(*(int *)(bufp + r_index + Three)));
		set_pen_color((FIXNUM)(PEN_COLOR_OFFSET));
		r_index += Four;
		break;
	    case (COLORFILL) :
#ifdef HAVE_WX
		set_palette(FILLED_COLOR_OFFSET,
				(*(int *)(bufp + r_index + One)),
				(*(int *)(bufp + r_index + Two)),
				(*(int *)(bufp + r_index + Three)));
#endif
		r_index += Four;
		break;
	    case (STARTFILL) :
#ifdef HAVE_WX
		point = points = malloc((*(int *)(bufp + r_index + One))
					    * sizeof(struct mypoint));
		if (point != NULL) {
		    point->x = screen_x_center+lastx;
		    point->y = screen_y_center-lasty;
		    point++;
		}
#endif
		r_index += Three;
		break;
	    case (ENDFILL) :
#ifdef HAVE_WX
		start = *(char **)(bufp + r_index + One);
		start_idx = *(int *)(bufp + r_index + Two);
		doFilled((*(int *)(start + start_idx + Two)),
			 (*(int *)(start + start_idx + One)),
			 points);
		free(points);
		point = points = 0;
#endif
		r_index += Three;
		break;
	    case (SETPENSIZE) :
		set_pen_width(*(int *)(bufp + r_index + One));
		set_pen_height(*(int *)(bufp + r_index + Two));
		r_index += Three;
		break;
	    case (SETPENPATTERN) :
		set_pen_pattern(&bufp[r_index + One]);
		r_index += One+8;
		break;
	    case (FILLERUP) :
		logofill();
		r_index += One;
		break;
	    case (ARC) :
		do_arc(*(FLONUM *)(bufp + r_index + Big),
		       *(FLONUM *)(bufp + r_index + 2 * Big),
		       *(FLONUM *)(bufp + r_index + 3 * Big),
		       *(FLONUM *)(bufp + r_index + 4 * Big),
		       *(FLONUM *)(bufp + r_index + 5 * Big),
		       *(FLONUM *)(bufp + r_index + 6 * Big),
		       *(FLONUM *)(bufp + r_index + 7 * Big),
		       *(FLONUM *)(bufp + r_index + 8 * Big),
		       1);
		r_index += 9*Big;
		break;
	    case (NEXTBUFFER):
		bufp = *(char **)(bufp);
		r_index = One;
		break;
	}
    }

    restore_pen(&saved_pen);
    turtle_shown = saved_shown;

#if defined(__RZTC__) && !defined(WIN32)
    if (save_splitscreen) {split_screen;}
#endif

    turtle_x = save_tx;
    turtle_y = save_ty;
    turtle_heading = save_th;
    draw_turtle();
    done_drawing;
}

NODE *lsavepict(NODE *args) {
    FILE *fp;
    static int bg;
    FIXNUM want, cnt;
    FIXNUM zero = 0;
    char *p;
    char *buf = record_buffer;
	
#if defined(WIN32)||defined(ibm)
	extern NODE *lopen(NODE *, char *);
	lopen(args,"wb");
#else
    lopenwrite(args);
#endif
    if (NOT_THROWING) {
	fp = (FILE *)file_list->n_obj;
	save_palette(fp);
	fwrite(&record_index, sizeof(FIXNUM), 1, fp);
	while (buf != record) {
	    want = GR_SIZE;
	    p = buf;
	    while (want > 0) {
		    cnt = fwrite(p, 1, want, fp);
		    if (ferror(fp) || cnt <= 0) {
			    err_logo(FILE_ERROR,
				    make_static_strnode("File too big"));
			    lclose(args);
			    return UNBOUND;
		    }
		    want -= cnt;
		    p += cnt;
	    }
	    buf = *(char **)(buf);
	}
	(void)fwrite(&zero, One, 1, fp);
	want = record_index;
	p = record + One;
	while (want > 0) {
		cnt = fwrite(p, 1, want, fp);
		if (ferror(fp) || cnt <= 0) {
			err_logo(FILE_ERROR,
				make_static_strnode("File too big"));
			lclose(args);
			return UNBOUND;
		}
		want -= cnt;
		p += cnt;
	}
	bg = (int)back_ground;
	fwrite(&bg, sizeof(int), 1, fp);
	lclose(args);
    }
    return UNBOUND;
}

NODE *lloadpict(NODE *args) {
    FILE *fp;
    static int bg;
    FIXNUM want, cnt, next, rec_idx;
    char *p, *buf=record_buffer, *newbuf;

#if defined(WIN32)||defined(ibm)
	extern NODE *lopen(NODE *, char *);
	lopen(args,"rb");
#else
    lopenread(args);
#endif
    if (NOT_THROWING) {
	prepare_to_draw;
	fp = (FILE *)file_list->n_obj;
	restore_palette(fp);
	fread(&rec_idx, sizeof(FIXNUM), 1, fp);
	if (rec_idx < 0 || rec_idx >= GR_SIZE) {
		err_logo(FILE_ERROR,
				 make_static_strnode("File bad format"));
		lclose(args);
		return UNBOUND;
	}
	(void)fread(&next, One, 1, fp);
	while (next != 0) {
	    want = GR_SIZE - One;
	    p = buf + One;
	    while (want > 0) {
		    cnt = fread(p, 1, want, fp);
		    if (ferror(fp) || cnt <= 0) {
			    record_index = 0;
			    last_recorded = -1;
			    done_drawing;
			    err_logo(FILE_ERROR,
				      make_static_strnode("File bad format"));
			    lclose(args);
			    return UNBOUND;
		    }
		    want -= cnt;
		    p += cnt;
	    }
	    newbuf = *(char **)(buf);
	    if (newbuf == 0) {
		newbuf = malloc(GR_SIZE);	/* get a new buffer */
		if (newbuf == NULL) {
		    done_drawing;
		    err_logo(FILE_ERROR,
				  make_static_strnode("Not enough memory."));
		    lclose(args);
		    return UNBOUND;
		}
		*(char **)(buf) = newbuf;
	    }
	    buf = newbuf;
	    (void)fread(&next, One, 1, fp);
	}
	want = rec_idx;
	p = buf + One;
	while (want > 0) {
		cnt = fread(p, 1, want, fp);
		if (ferror(fp) || cnt <= 0) {
			done_drawing;
			err_logo(FILE_ERROR,
				  make_static_strnode("File bad format"));
			lclose(args);
			return UNBOUND;
		}
		want -= cnt;
		p += cnt;
	}
	record = buf;
	record_index = rec_idx;
	fread(&bg, sizeof(int), 1, fp);
	lclose(args);
	set_back_ground((FIXNUM)bg);
	done_drawing;
    }
    return UNBOUND;
}

void ps_string(FILE *fp, char *p) {
    int ch;

    while ((ch = *p++)) {
	if (ch=='(' || ch==')' || ch=='\\') fprintf(fp, "\\");
	fprintf(fp, "%c", ch);
    }
}

void rgbprint(FILE *fp, int cnum) {
    unsigned int r=0, g=0, b=0;

    get_palette(cnum, &r, &g, &b);
    fprintf(fp, "%6.4f %6.4f %6.4f", ((double)r)/65535,
		((double)g)/65535, ((double)b)/65535);
}

#ifdef mac
extern void fixMacType(NODE *args);
#endif

NODE *lepspict(NODE *args) {
    FILE *fp;
    int r_index = One, act=0, lastx = 0, lasty = 0, vis = 0;
    char *bufp = record_buffer;

#ifdef mac
    fixMacType(args);
    lopenappend(args);
#else
    lopenwrite(args);
#endif
    if (NOT_THROWING) {
	fp = (FILE *)file_list->n_obj;

	fprintf(fp, "%%!PS-Adobe-3.0 EPSF-3.0\n");
	ndprintf(fp, "%%Title: %p\n", car(args));
	fprintf(fp, "%%%%Creator: Logo2PS 1.1; Copyright 1997, Vladimir Batagelj\n");
	fprintf(fp, "%%%%BoundingBox: %d %d %d %d\n",
		screen_left, screen_top, screen_right, screen_bottom);
	fprintf(fp, "%%%%EndComments\n");
	ndprintf(fp, "%%Page: %p\n", car(args));
	fprintf(fp, "1 setlinecap 1 setlinejoin\n");
	fprintf(fp, "/Courier 9 selectfont\n");

	fprintf(fp, "gsave\n");
	rgbprint(fp, back_ground);
	fprintf(fp, " setrgbcolor\n");
	fprintf(fp, "%d %d moveto %d %d lineto %d %d lineto %d %d lineto\n",
		    screen_left, screen_top, screen_right, screen_top,
		    screen_right, screen_bottom, screen_left, screen_bottom);
	fprintf(fp, " closepath fill grestore\n");

	fprintf(fp, "%d %d moveto\n", screen_x_center, screen_y_center);

	while (bufp[r_index] != FINISHED) {
	    switch (bufp[r_index]) {
		case (LINEXY) :
		    if (!vis) {
			lastx = screen_x_center + *(int *)(bufp + r_index + One);
			lasty = screen_y_center + *(int *)(bufp + r_index + Two);
			fprintf(fp, "%d %d lineto\n", lastx, lasty);
			r_index += Three;
			act++;
			break;	/* else fall through */
		    }
		case (MOVEXY) :
		    lastx = screen_x_center + *(int *)(bufp + r_index + One);
		    lasty = screen_y_center + *(int *)(bufp + r_index + Two);
		    fprintf(fp, "%d %d moveto\n", lastx, lasty);
		    r_index += Three;
		    break;
		case (LABEL) :
		    fprintf(fp, "gsave -5 1 rmoveto (");
		    ps_string(fp, bufp + r_index + One+1);
		    fprintf(fp, ") show grestore\n");
		    fprintf(fp, "%d %d moveto\n", lastx, lasty);
		    r_index += (One+2 + bufp[r_index + One] + (One-1)) & ~(One-1);
		    break;
		case (SETPENVIS) :
		    vis = bufp[r_index + 1];
		    r_index += One;
		    break;
		case (SETPENMODE) :
		    r_index += Three;
		    break;
		case (SETPENCOLOR) :
		    if (act) {
			fprintf(fp, "stroke %d %d moveto\n", lastx, lasty);
			act = 0;
		    }
		    rgbprint(fp, (*(int *)(bufp + r_index + One)));
		    fprintf(fp, " setrgbcolor\n");
		    r_index += Two;
		    break;
		case (SETPENRGB) :
		    if (act) {
			fprintf(fp, "stroke %d %d moveto\n", lastx, lasty);
			act = 0;
		    }
		    set_palette(-1, (*(int *)(bufp + r_index + One)),
				    (*(int *)(bufp + r_index + Two)),
				    (*(int *)(bufp + r_index + Three)));
		    rgbprint(fp, -1);
		    fprintf(fp, " setrgbcolor\n");
		    r_index += Four;
		    break;
		case (SETPENSIZE) :
		    if (act) {
			fprintf(fp, "stroke %d %d moveto\n", lastx, lasty);
			act = 0;
		    }
		    fprintf(fp, "%d setlinewidth\n",
			    (*(int *)(bufp + r_index + One)));
		    r_index += Three;
		    break;
		case (SETPENPATTERN) :
		    r_index += One+8;
		    break;
		case (FILLERUP) :
		    r_index += One;
		    break;
		case (ARC) :
		{
		    FLONUM tx = screen_x_center +
						*(FLONUM *)(bufp + r_index + 5*Big);
		    FLONUM ty = screen_y_center +
						*(FLONUM *)(bufp + r_index + 6*Big);
		    FLONUM radius = *(FLONUM *)(bufp + r_index + 3*Big);
		    FLONUM angle =*(FLONUM *)(bufp + r_index + 7*Big);
		    FLONUM thead = *(FLONUM *)(bufp + r_index + 8*Big);
		    
		    /* move to beginning of the arc */
		    fprintf(fp, "%f %f moveto\n",
			    tx + radius * cos(degrad * (90 - thead - angle)),
			    ty + radius * sin(degrad * (90 - thead - angle)));
		    
		    /* x-coord y-coord r ang1 ang2 arc  - */
		    fprintf(fp, "%f %f %f %f %f arc\n",
			    tx, ty, radius, 
			    90 - thead - angle, 
			    90 - thead); 

		    /* bring ps-pen back to where turtle is */
		    fprintf(fp, "%d %d moveto\n", lastx, lasty);
		}

		r_index += 9*Big;
		break;
	    case (NEXTBUFFER):
		bufp = *(char **)(bufp);
		r_index = One;
		break;
	    case (STARTFILL):
	    case (ENDFILL):
		r_index += Three;
		break;
	    case (COLORFILL):
		r_index += Four;
		break;
	    }
	}
	    
	fprintf(fp, "stroke\nshowpage\n%%%%EOF\n");

	lclose(args);
    }
    return UNBOUND;
}
