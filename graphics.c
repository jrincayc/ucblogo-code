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

#include "logo.h"
/*   #include "globals.h"   has been moved further down */
#include <math.h>

#ifdef mac
#include "macterm.h"
#elif defined(WIN32)
#include "win32trm.h"
#elif defined(__ZTC__)
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

#if defined(__ZTC__) && !defined(WIN32) /* sowings */
#define total_turtle_bottom_max (-(MaxY/2))
#else
#define total_turtle_bottom_max turtle_bottom_max
#endif

/* types of graphics moves that can be recorded */
#define LINEXY         1
#define MOVEXY         2
#define LABEL          3
#define SETPENVIS      4
#define SETPENMODE     5
#define SETPENCOLOR    6
#define SETPENSIZE     7
#define SETPENPATTERN  8
#define FILLERUP	   9

/* NOTE: See the files (macterm.c and macterm.h) or (ibmterm.c and ibmterm.h)
   for examples of the functions and macros that this file assumes exist. */

#define One (sizeof(int))
#define Two (2*One)
#define Three (3*One)

#define PENMODE_PAINT	0
#define PENMODE_ERASE	1
#define PENMODE_REVERSE	2

int internal_penmode = PENMODE_PAINT;

mode_type current_mode = wrapmode;
FLONUM turtle_x = 0.0, turtle_y = 0.0, turtle_heading = 0.0;
FLONUM x_scale = 1.0, y_scale = 1.0;
BOOLEAN turtle_shown = TRUE;
FLONUM wanna_x = 0.0, wanna_y = 0.0;
BOOLEAN out_of_bounds = FALSE;
void setpos_bynumber(FLONUM, FLONUM);

char record[GR_SIZE];
int record_index = 0;
pen_info orig_pen;

BOOLEAN record_next_move = FALSE, refresh_p = TRUE;

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
    if (n < 0.0)
	return((FIXNUM)ceil(n));
    return((FIXNUM)floor(n));
}

/************************************************************/
void draw_turtle_helper(void);
void check_x_high(void);
void check_x_low(void);

void draw_turtle(void) {
    if (!turtle_shown) return;
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
    
    prepare_to_draw;
    prepare_to_draw_turtle;
    save_pen(&saved_pen);
    plain_xor_pen();
    pen_vis = 0;
    
    real_heading = -turtle_heading + 90.0;
	
    left_x = g_round(turtle_x + x_scale*(FLONUM)(cos((FLONUM)((real_heading + 90.0)*degrad))*turtle_half_bottom));
    left_y = g_round(turtle_y + y_scale*(FLONUM)(sin((FLONUM)((real_heading + 90.0)*degrad))*turtle_half_bottom));

    right_x = g_round(turtle_x + x_scale*(FLONUM)(cos((FLONUM)((real_heading - 90.0)*degrad))*turtle_half_bottom));
    right_y = g_round(turtle_y + y_scale*(FLONUM)(sin((FLONUM)((real_heading - 90.0)*degrad))*turtle_half_bottom));

    top_x = g_round(turtle_x + x_scale*(FLONUM)(cos((FLONUM)(real_heading*degrad))*turtle_side));
    top_y = g_round(turtle_y + y_scale*(FLONUM)(sin((FLONUM)(real_heading*degrad))*turtle_side));

    move_to(screen_x_center + left_x, screen_y_center - left_y);
    line_to(screen_x_center + top_x, screen_y_center - top_y);
    move_to(screen_x_center + right_x, screen_y_center - right_y);
    line_to(screen_x_center + top_x, screen_y_center - top_y);
    move_to(screen_x_center + left_x, screen_y_center - left_y);
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

void right(FLONUM a) {
    draw_turtle();
    turtle_heading += a;
    turtle_heading = pfmod(turtle_heading,360.0);
    draw_turtle();
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

int wrap_right(FLONUM, FLONUM, FLONUM, FLONUM, FLONUM);
int wrap_left(FLONUM, FLONUM, FLONUM, FLONUM, FLONUM);
int wrap_up(FLONUM, FLONUM, FLONUM, FLONUM, FLONUM);
int wrap_down(FLONUM, FLONUM, FLONUM, FLONUM, FLONUM);

void forward(FLONUM d) {
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
    FLONUM real_heading, dx, dy, x1, y1, x2, y2;
    
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
    if (record_next_move) {
	save_move();
	record_next_move = FALSE;
    }
    
    if (check_throwing) return;

    if (internal_penmode == PENMODE_REVERSE && pen_vis == 0 && d > 0.0) {
	line_to(g_round(x1), g_round(y1));	/* flip the corner */
	save_line();
    }

    if (current_mode == windowmode ||
	(x2 >= screen_left && x2 <= screen_right &&
	 y2 >= screen_top && y2 <= screen_bottom)) {
	turtle_x = turtle_x + dx;
	turtle_y = turtle_y + dy;
	line_to(g_round(x2), g_round(y2));
	save_line();
    }
    else
	if (!wrap_right(d, x1, y1, x2, y2))
	    if (!wrap_left(d, x1, y1, x2, y2))
		if (!wrap_up(d, x1, y1, x2, y2))
		    wrap_down(d, x1, y1, x2, y2);

    if (internal_penmode == PENMODE_REVERSE && pen_vis == 0 && d < 0.0) {
	line_to(g_round(screen_x_coord), g_round(screen_y_coord));
	save_line();
    }
}

int wrap_right(FLONUM d, FLONUM x1, FLONUM y1, FLONUM x2, FLONUM y2) {
    FLONUM yi, newd;
    
    if (x2 > screen_right) {
	yi = ((y2 - y1)/(x2 - x1)) * (screen_right + 1 - x1) + y1;
	if (yi >= screen_top && yi <= screen_bottom) {
	    line_to(screen_right, g_round(yi));
	    save_line();
	    record_next_move = TRUE;
	    turtle_x = turtle_left_max;
	    turtle_y = screen_y_center - yi;
	    if (current_mode == wrapmode) {
		newd = d * ((x2 - screen_right - 1)/(x2 - x1));
		if (newd*d > 0) forward_helper(newd);
		return(1);
	    }
	    turtle_x = turtle_right_max;
	    err_logo(TURTLE_OUT_OF_BOUNDS, NIL);
	}
    }
    return(0);
}

int wrap_left(FLONUM d, FLONUM x1, FLONUM y1, FLONUM x2, FLONUM y2) {
    FLONUM yi, newd;
    
    if (x2 < screen_left) {
	yi = ((y1 - y2)/(x2 - x1)) * (x1 + 1 - screen_left) + y1;
	if (yi >= screen_top && yi <= screen_bottom) {
	    line_to(screen_left, g_round(yi));
	    save_line();
	    record_next_move = TRUE;
	    turtle_x = turtle_right_max;
	    turtle_y = screen_y_center - yi;
	    if (current_mode == wrapmode) {
		newd = d * ((x2 + 1 - screen_left)/(x2 - x1));
		if (newd*d > 0) forward_helper(newd);
		return(1);
	    }
	    turtle_x = turtle_left_max;
	    err_logo(TURTLE_OUT_OF_BOUNDS, NIL);	    
	}
    }
    return(0);
}

int wrap_up(FLONUM d, FLONUM x1, FLONUM y1, FLONUM x2, FLONUM y2) {
    FLONUM xi, newd;
    
    if (y2 < screen_top) {
	xi = ((x2 - x1)/(y1 - y2)) * (y1 + 1 - screen_top) + x1;
	if (xi >= screen_left && xi <= screen_right) {
	    line_to(g_round(xi), screen_top);
	    save_line();
	    record_next_move = TRUE;
	    turtle_x = xi - screen_x_center;
	    turtle_y = turtle_bottom_max;
	    if (current_mode == wrapmode) {
		newd = d * ((y2 + 1 - screen_top)/(y2 - y1));
		if (newd*d > 0) forward_helper(newd);
		return(1);
	    }
	    turtle_y = turtle_top_max;
	    err_logo(TURTLE_OUT_OF_BOUNDS, NIL);
	}
    }
    return(0);
}

int wrap_down(FLONUM d, FLONUM x1, FLONUM y1, FLONUM x2, FLONUM y2) {
    FLONUM xi, newd;
    
    if (y2 > screen_bottom) {
	xi = ((x2 - x1)/(y2 - y1)) * (screen_bottom + 1 - y1) + x1;
	if (xi >= screen_left && xi <= screen_right) {
	    line_to(g_round(xi), screen_bottom);
	    save_line();
	    record_next_move = TRUE;
	    turtle_x = xi - screen_x_center;
	    turtle_y = turtle_top_max;
	    if (current_mode == wrapmode) {
		newd = d * ((y2 - screen_bottom - 1)/(y2 - y1));
		if (newd*d > 0) forward_helper(newd);
		return(1);
	    }
	    turtle_y = turtle_bottom_max;
	    err_logo(TURTLE_OUT_OF_BOUNDS, NIL);
	}
    }
    return(0);
}

NODE *lforward(NODE *arg) {
    NODE *val;
    FLONUM d;
    
    val = numeric_arg(arg);
    if (NOT_THROWING) {
	if (nodetype(val) == INT)
	    d = (FLONUM)getint(val);
	else
	    d = getfloat(val);
	forward(d);
    }
    return(UNBOUND);
}

NODE *lback(NODE *arg) {
    NODE *val;
    FLONUM d;
    
    val = numeric_arg(arg);
    if (NOT_THROWING) {
	if (nodetype(val) == INT)
	    d = (FLONUM)getint(val);
	else
	    d = getfloat(val);
	forward(-d);
    }
    return(UNBOUND);
}

NODE *lshowturtle(NODE *args) {
    prepare_to_draw;
    if (!turtle_shown) {
	turtle_shown = TRUE;
	draw_turtle();
    }
    done_drawing;
    return(UNBOUND);
}

NODE *lhideturtle(NODE *args) {
    prepare_to_draw;
    if (turtle_shown) {
	draw_turtle();
	turtle_shown = FALSE;
    }
    done_drawing;
    return(UNBOUND);
}

NODE *lshownp(NODE *args) {
    return(turtle_shown ? True : False);
}

NODE *lsetheading(NODE *arg) {
    NODE *val;
    
    val = numeric_arg(arg);
    if (NOT_THROWING) {
	draw_turtle();
	if (nodetype(val) == INT)
	    turtle_heading = (FLONUM)getint(val);
	else
	    turtle_heading = getfloat(val);
	turtle_heading = pfmod(turtle_heading,360.0);
	draw_turtle();
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
	return vec_arg_helper(args,FALSE,TRUE);
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
    out_of_bounds = FALSE;
    setpos_bynumber((FLONUM)0.0, (FLONUM)0.0);
    draw_turtle();
    turtle_heading = 0.0;
    draw_turtle();
    return(UNBOUND);
}

void cs_helper(int centerp) {    
    prepare_to_draw;
    clear_screen;
    if (centerp) {
	wanna_x = wanna_y = turtle_x = turtle_y = turtle_heading = 0.0;
	out_of_bounds = FALSE;
	move_to(screen_x_coord, screen_y_coord);
    }
    draw_turtle();
    save_pen(&orig_pen);
    p_info_x(orig_pen) = g_round(screen_x_coord);
    p_info_y(orig_pen) = g_round(screen_y_coord);
    record_index = 0;
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
	done_drawing;
	draw_turtle();
    }
}

void setpos_bynumber(FLONUM target_x, FLONUM target_y) {
    if (NOT_THROWING) {
	prepare_to_draw;
	draw_turtle();
	move_to(g_round(screen_x_coord), g_round(screen_y_coord));
	setpos_commonpart(target_x, target_y);
    }
}

void setpos_helper(NODE *xnode, NODE *ynode) {
    FLONUM target_x, target_y;
    
    if (NOT_THROWING) {
	prepare_to_draw;
	draw_turtle();
	move_to(g_round(screen_x_coord), g_round(screen_y_coord));
	target_x = ((xnode == NIL) ?
		turtle_x :
		((nodetype(xnode) == FLOATT) ? getfloat(xnode) :
		 (FLONUM)getint(xnode)));
	target_y = ((ynode == NIL) ?
		turtle_y :
		((nodetype(ynode) == FLOATT) ? getfloat(ynode) :
		 (FLONUM)getint(ynode)));
	setpos_commonpart(target_x, target_y);
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
    return(UNBOUND);
}

NODE *lfence(NODE *args) {
    (void)lwrap(args);	    /* get turtle inside the fence */
    draw_turtle();
    current_mode = fencemode;
    draw_turtle();
    return(UNBOUND);
}

NODE *lwindow(NODE *args) {
    draw_turtle();
    current_mode = windowmode;
    draw_turtle();
    return(UNBOUND);
}

NODE *lfill(NODE *args) {
    draw_turtle();
    logofill();
    draw_turtle();
    if (safe_to_save()) {
	record[record_index] = FILLERUP;
	record_index += One;
    }
    return(UNBOUND);
}

NODE *llabel(NODE *arg) {
    char textbuf[300];
    short theLength;

    print_stringptr = textbuf;
    print_stringlen = 300;
    ndprintf((FILE *)NULL,"%p",car(arg));
    *print_stringptr = '\0';
	
    if (NOT_THROWING) {
	prepare_to_draw;
	draw_turtle();
	theLength = strlen(textbuf);
#ifdef mac
	c_to_pascal_string(textbuf, theLength);
#endif
	label(textbuf);
	save_string(textbuf,theLength);
	record_next_move = TRUE;
	draw_turtle();
	done_drawing;
    }
    return(UNBOUND);
}

NODE *ltextscreen(NODE *args) {
    text_screen;
    return(UNBOUND);
}

NODE *lsplitscreen(NODE *args) {
    split_screen;
    return(UNBOUND);
}

NODE *lfullscreen(NODE *args) {
    full_screen;
    return(UNBOUND);
}

NODE *lpendownp(NODE *args) {
    return(pen_vis == 0 ? True : False);
}

NODE *lpenmode(NODE *args) {
    return(get_node_pen_mode);
}

NODE *lpencolor(NODE *args) {
    return(make_intnode((FIXNUM)pen_color));
}

NODE *lbackground(NODE *args) {
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

NODE *lsetpencolor(NODE *arg) {
    NODE *val = pos_int_arg(arg);

    if (NOT_THROWING) {
	prepare_to_draw;
	set_pen_color(getint(val));
	save_color();
	done_drawing;
    }
    return(UNBOUND);
}

NODE *lsetbackground(NODE *arg) {
    NODE *val = pos_int_arg(arg);

    if (NOT_THROWING) {
	prepare_to_draw;
	set_back_ground(getint(val));
	done_drawing;
    }
    return(UNBOUND);
}

NODE *lsetpalette(NODE *args) {
	NODE *slot = pos_int_arg(args);
	NODE *arg = rgb_arg(cdr(args));

	if (NOT_THROWING && ((int)getint(slot) > 7)) {
		prepare_to_draw;
		set_palette((int)getint(slot),
			    (unsigned int)getint(car(arg)),
			    (unsigned int)getint(cadr(arg)),
			    (unsigned int)getint(car(cddr(arg))));
		done_drawing;
	}
	return(UNBOUND);
}

NODE *lpalette(NODE *args) {
	NODE *arg = pos_int_arg(args);
	unsigned int r=0, g=0, b=0;

	if (NOT_THROWING) {
		get_palette((int)getint(arg), &r, &g, &b);
		return cons(make_intnode((FIXNUM)r),
					cons(make_intnode((FIXNUM)g),
						 cons(make_intnode((FIXNUM)b), NIL)));
	}
	return UNBOUND;
}

NODE *lsetpensize(NODE *args) {
    NODE *arg = pos_int_vector_arg(args);

    if (NOT_THROWING) {
	prepare_to_draw;
	set_pen_width((int)getint(car(arg)));
	set_pen_height((int)getint(cadr(arg)));
	save_size();
	done_drawing;
    }
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
#ifdef __ZTC__
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
    }
    return(UNBOUND);
}

NODE *lmousepos(NODE *args) {
#ifdef WIN32 /* sowings */
    return NIL;
#else
    return(cons(make_intnode(mouse_x), cons(make_intnode(mouse_y), NIL)));
#endif
}

NODE *lbuttonp(NODE *args) {
    if (button)
	return(True);
    return(False);
}

NODE *ltone(NODE *args) {
#ifndef WIN32
    NODE *p, *d;
    FIXNUM pitch, duration;
    
    p = numeric_arg(args);
    d = numeric_arg(cdr(args));
    
    if (NOT_THROWING) {
	pitch = (nodetype(p) == FLOATT) ? (FIXNUM)getfloat(p) : getint(p);
	duration = (nodetype(d) == FLOATT) ? (FIXNUM)getfloat(d) : getint(d);
	if (pitch > 0) tone(pitch, duration);
    }
#else
    MessageBeep(0xffffffff);
#endif
    
    return(UNBOUND);
}

NODE *larc(NODE *arg) {
    NODE *val1;
    NODE *val2;

    FLONUM angle;
    FLONUM radius;
    FLONUM ang;
    FLONUM tx;
    FLONUM ty;
    FLONUM x;
    FLONUM y;
    FLONUM count;
    FLONUM delta;
    FLONUM i;

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

    count = abs(angle*radius/200.0);
    if (count == 0.0) count = 1.0;
    delta = angle/count;

    /* draw each line segment of arc (will do wrap) */

    for (i=0.0;i<=count;i=i+1.0)
       {

       /* calc x y */

       x = sin(ang*3.141592654/180.0)*radius;
       y = cos(ang*3.141592654/180.0)*radius;

       /* jump to begin of first line segment without drawing */

       if (i==0.0)
          {
          pen_state = pen_vis;
          pen_vis = -1;
	  save_vis();
          setpos_bynumber(tx+x, ty+y);
          pen_vis = pen_state;
	  save_vis();
          }

       /* else do segment */

       else
          {
          setpos_bynumber(tx+x, ty+y);
          }

       ang = ang + delta;
       }

    /* assure we draw something and end in the exact right place */

    x = sin((turtle_heading+angle)*3.141592654/180.0)*radius;
    y = cos((turtle_heading+angle)*3.141592654/180.0)*radius;

    setpos_bynumber(tx+x, ty+y);

    /* restore state */

    turtle_shown = turtle_state;

    turtle_x = tx;
    turtle_y = ty;

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

/************************************************************/
/* The rest of this file implements the recording of moves in
   the graphics window and the playing back of those moves.  It's
   needed on machines like the Macintosh where the contents of the
   graphics window can get erased and need to be redrawn.  On
   machines where no graphics redrawing is necessary, set the size
   of the recording buffer to 1 in logo.h. */

BOOLEAN safe_to_save(void) {
    return(refresh_p && record_index < (GR_SIZE - 300));
}

void save_lm_helper (void) {
    *(int *)(record + record_index + One) = pen_x;
    *(int *)(record + record_index + Two) = pen_y;
    record_index += Three;
}

void save_line(void) {
    if (safe_to_save()) {
	record[record_index] = LINEXY;
	save_lm_helper();
    }
}

void save_move(void) {
    if (safe_to_save()) {
	record[record_index] = MOVEXY;
	save_lm_helper();
    }
}

void save_vis(void) {
    if (safe_to_save()) {
	record[record_index] = SETPENVIS;
	record[record_index + 1] = (char)pen_vis;
	record_index += One;
    }
}

void save_mode(void) {
    if (safe_to_save()) {
	record[record_index] = SETPENMODE;
#ifdef x_window
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
    if (safe_to_save()) {
	record[record_index] = SETPENCOLOR;
	*(int *)(record + record_index + One) = pen_color;
	record_index += Two;
    }
}

void save_size(void) {
    if (safe_to_save()) {
	record[record_index] = SETPENSIZE;
	*(int *)(record + record_index + One) = pen_width;
	*(int *)(record + record_index + Two) = pen_height;
	record_index += Three;
    }
}

void save_pattern(void) {
    if (safe_to_save()) {
	record[record_index] = SETPENPATTERN;
	get_pen_pattern(&record[record_index + One]);
	record_index += One+8;
    }
}

void save_string(char *s, int len) {
    int count;

    if (safe_to_save()) {
	record[record_index] = LABEL;
	record[record_index + One] = (unsigned char)len;
	for (count = 0; count <= len; count++)
	    record[record_index + One+1 + count] = s[count];
	record[record_index + One+2 + len] = '\0';
	record_index += (One+2 + len + (One-1)) & ~(One-1);
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
    int r_index = 0;
    pen_info saved_pen;
    BOOLEAN saved_shown;
#if defined(__ZTC__) && !defined(WIN32)
    BOOLEAN save_splitscreen = in_splitscreen;
#endif
    
    if (!refresh_p) {
    /*	clear_screen;
	draw_turtle();	*/
	return;
    }

    saved_shown = turtle_shown;
    turtle_shown = FALSE;
    save_pen(&saved_pen);
    restore_pen(&orig_pen);

#if defined(__ZTC__) && !defined(WIN32)
    full_screen;
#endif

    erase_screen();

#ifdef __TURBOC__
    moveto(p_info_x(orig_pen),p_info_y(orig_pen));
#endif

    while (r_index < record_index)
	switch (record[r_index]) {
	    case (LINEXY) :
		line_to(*(int *)(record + r_index + One),
			*(int *)(record + r_index + Two));
		r_index += Three;
		break;
	    case (MOVEXY) :
		move_to(*(int *)(record + r_index + One),
			*(int *)(record + r_index + Two));
		r_index += Three;
		break;
	    case (LABEL) :
		draw_string(record + r_index + One+1);
		r_index += (One+2 + record[r_index + One] + (One-1)) & ~(One-1);
		break;
	    case (SETPENVIS) :
		set_pen_vis(record[r_index + 1]);
		r_index += One;
		break;
	    case (SETPENMODE) :
#ifdef x_window
		set_pen_mode(*(GC *)(record + r_index + One));
#else
		set_pen_mode(*(int *)(record + r_index + One));
#endif
		internal_penmode = *(int *)(record + record_index + Two);
		r_index += Three;
		break;
	    case (SETPENCOLOR) :
		set_pen_color(*(int *)(record + r_index + One));
		r_index += Two;
		break;
	    case (SETPENSIZE) :
		set_pen_width(*(int *)(record + r_index + One));
		set_pen_height(*(int *)(record + r_index + Two));
		r_index += Three;
		break;
	    case (SETPENPATTERN) :
		set_pen_pattern(&record[r_index + One]);
		r_index += One+8;
		break;
	    case (FILLERUP) :
		logofill();
		r_index += One;
		break;
	}

    restore_pen(&saved_pen);
    turtle_shown = saved_shown;

#if defined(__ZTC__) && !defined(WIN32)
    if (save_splitscreen) {split_screen;}
#endif

    draw_turtle();
}

/* This is called when the graphics coordinate system has been shifted.
   It adds a constant amount to each x and y coordinate in the record. */
void resize_record(int dh, int dv) {
    int r_index = 0;
    
    p_info_x(orig_pen) += dh;
    p_info_y(orig_pen) += dv;
    
    while (r_index < record_index)
	switch (record[r_index]) {
	    case (LINEXY) :
	    case (MOVEXY) :
		*(int *)(record + r_index + One) += dh;
		*(int *)(record + r_index + Two) += dv;
		r_index += Three;
		break;
	    case (LABEL) :
		r_index += (One+2 + record[r_index + One] + (One-1)) & ~(One-1);
		break;
	    case (SETPENVIS) :
	    case (FILLERUP) :
		r_index += One;
		break;
	    case (SETPENCOLOR) :
		r_index += Two;
		break;
	    case (SETPENSIZE) :
	    case (SETPENMODE) :
		r_index += Three;
		break;
	    case (SETPENPATTERN) :
		r_index += One+8;
		break;
	}
}
