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

#include "logo.h"
/*   #include "globals.h"   has been moved further down */
#include <math.h>

#ifdef mac
#include "macterm.h"
#else
#ifdef ibm
#ifdef __ZTC__
#include <fg.h>
#include "ztcterm.h"
#else
#include "ibmterm.h"
#endif
#else
#ifdef x_window
#include "xgraphics.h"
#else
#include "nographics.h"
#endif
#endif
#endif

#include "globals.h"

#ifdef __ZTC__
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

mode_type current_mode = wrapmode;
FLONUM turtle_x = 0.0, turtle_y = 0.0, turtle_heading = 0.0;
BOOLEAN turtle_shown = TRUE;
FLONUM x_scale = 1.0, y_scale = 1.0;
FLONUM wanna_x = 0.0, wanna_y = 0.0;
BOOLEAN out_of_bounds = FALSE;

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

FLONUM cut_error(FLONUM n)
{
    n *= 1000000;
    n = (n > 0 ? floor(n) : ceil(n));
    n /= 1000000;
    if (n == -0.0) n = 0.0;
    return(n);
}

FIXNUM g_round(FLONUM n)
{
    n += (n < 0.0 ? -0.5 : 0.5);
    if (n < 0.0)
	return((FIXNUM)ceil(n));
    return((FIXNUM)floor(n));
}

/************************************************************/

void draw_turtle()
{
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

check_x_high()
{
    if ((turtle_x > turtle_right_max - turtle_height) &&
	    (current_mode == wrapmode)) {
	turtle_x -= (screen_width + 1);
	draw_turtle_helper();
	turtle_x += (screen_width + 1);
    }
}

check_x_low()
{
    if ((turtle_x < turtle_left_max + turtle_height) &&
	    (current_mode == wrapmode)) {
	turtle_x += (screen_width + 1);
	draw_turtle_helper();
	turtle_x -= (screen_width + 1);
    }
}

draw_turtle_helper()
{
    pen_info saved_pen;
    FLONUM real_heading;
    int left_x, left_y, right_x, right_y, top_x, top_y, center_x, center_y;
    
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

void right(FLONUM a)
{
    draw_turtle();
    turtle_heading += a;
    turtle_heading = pfmod(turtle_heading,360.0);
    draw_turtle();
}

NODE *numeric_arg(NODE *args)
{
    NODE *arg = car(args), *val;

    val = cnv_node_to_numnode(arg);
    while (val == UNBOUND && NOT_THROWING) {
	gcref(val);
	setcar(args, err_logo(BAD_DATA, arg));
	arg = car(args);
	val = cnv_node_to_numnode(arg);
    }
    setcar(args,val);
    return(val);
}

NODE *lright(NODE *arg)
{
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

NODE *lleft(NODE *arg)
{
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

void forward(FLONUM d)
{
    prepare_to_draw;
    draw_turtle();
    forward_helper(d);
    draw_turtle();
    done_drawing;
    wanna_x = turtle_x;
    wanna_y = turtle_y;
    out_of_bounds = FALSE;
}

forward_helper(FLONUM d)
{
    FLONUM real_heading, dx, dy, x1, y1, x2, y2, intercept;
    
    real_heading = -turtle_heading + 90.0;
    x1 = screen_x_coord;
    y1 = screen_y_coord;
    dx = (FLONUM)(cos((FLONUM)(real_heading*degrad))*d*x_scale);
    dy = (FLONUM)(sin((FLONUM)(real_heading*degrad))*d*y_scale);
    x2 = x1 + dx;
    y2 = y1 - dy;
    
    move_to(g_round(x1), g_round(y1));
    if (record_next_move) {
	save_move();
	record_next_move = FALSE;
    }
    
    if (check_throwing) return;
    
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
}

int wrap_right(FLONUM d, FLONUM x1, FLONUM y1, FLONUM x2, FLONUM y2)
{
    FLONUM yi, newd;
    
    if (x2 > screen_right) {
	yi = ((y2 - y1)/(x2 - x1)) * (screen_right - x1) + y1;
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

int wrap_left(FLONUM d, FLONUM x1, FLONUM y1, FLONUM x2, FLONUM y2)
{
    FLONUM yi, newd;
    
    if (x2 < screen_left) {
	yi = ((y1 - y2)/(x2 - x1)) * (x1 - screen_left) + y1;
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

int wrap_up(FLONUM d, FLONUM x1, FLONUM y1, FLONUM x2, FLONUM y2)
{
    FLONUM xi, newd;
    
    if (y2 < screen_top) {
	xi = ((x2 - x1)/(y1 - y2)) * (y1 - screen_top) + x1;
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

int wrap_down(FLONUM d, FLONUM x1, FLONUM y1, FLONUM x2, FLONUM y2)
{
    FLONUM xi, newd;
    
    if (y2 > screen_bottom) {
	xi = ((x2 - x1)/(y2 - y1)) * (screen_bottom - y1) + x1;
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

NODE *lforward(NODE *arg)
{
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

NODE *lback(NODE *arg)
{
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

NODE *lshowturtle()
{
    prepare_to_draw;
    if (!turtle_shown) {
	turtle_shown = TRUE;
	draw_turtle();
    }
    done_drawing;
    return(UNBOUND);
}

NODE *lhideturtle()
{
    prepare_to_draw;
    if (turtle_shown) {
	draw_turtle();
	turtle_shown = FALSE;
    }
    done_drawing;
    return(UNBOUND);
}

NODE *lshownp()
{
    return(turtle_shown ? True : False);
}

NODE *lsetheading(NODE *arg)
{
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

NODE *lheading()
{
    return(make_floatnode(turtle_heading));
}

NODE *vec_arg_helper(NODE *args, BOOLEAN floatok, BOOLEAN three)
{
    NODE *arg = car(args), *val1, *val2, *val3;

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
				 (!three || nodetype(val3) == INT && getint(val3) >= 0)))) {
		setcar(arg, val1);
		setcar(cdr(arg), val2);
		if (three) setcar (cddr(arg), val3);
		return(arg);
	    }
	    gcref(val1);
	    gcref(val2);
		if (three) gcref(val3);
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

FLONUM towards_helper(FLONUM x, FLONUM y, FLONUM from_x, FLONUM from_y)
{
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
}

NODE *ltowards(NODE *args)
{
    NODE *xnode, *ynode = UNBOUND, *arg;
    FLONUM x, y;
    
    arg = vector_arg(args);
    if (NOT_THROWING) {
	xnode = car(arg);
	ynode = cadr(arg);
	
	x = ((nodetype(xnode) == FLOAT) ? getfloat(xnode) :
			  (FLONUM)getint(xnode));
	y = ((nodetype(ynode) == FLOAT) ? getfloat(ynode) :
			  (FLONUM)getint(ynode));
	return make_floatnode(towards_helper(x, y, turtle_x, turtle_y));
    }
    return(UNBOUND);
}

NODE *lpos()
{
    return(cons(make_floatnode(cut_error(turtle_x/x_scale)),
	cons(make_floatnode(cut_error(turtle_y/y_scale)), NIL)));
}

NODE *lscrunch()
{
    return(cons(make_floatnode(x_scale), cons(make_floatnode(y_scale), NIL)));
}

NODE *lhome()
{
    void setpos_helper(NODE *, NODE *);

    out_of_bounds = FALSE;
    setpos_helper(make_intnode((FIXNUM)0), make_intnode((FIXNUM)0));
    draw_turtle();
    turtle_heading = 0.0;
    draw_turtle();
    return(UNBOUND);
}

cs_helper(int centerp)
{    
    prepare_to_draw;
    clear_screen;
    if (centerp) {
	wanna_x = wanna_y = turtle_x = turtle_y = turtle_heading = 0.0;
	out_of_bounds = FALSE;
    }
    draw_turtle();
    save_pen(&orig_pen);
    p_info_x(orig_pen) = g_round(screen_x_coord);
    p_info_y(orig_pen) = g_round(screen_y_coord);
    record_index = 0;
    done_drawing;
}

NODE *lclearscreen()
{
    cs_helper(TRUE);
    return(UNBOUND);
}

NODE *lclean()
{
    cs_helper(FALSE);
    return(UNBOUND);
}

void setpos_helper(NODE *xnode, NODE *ynode)
{
    FLONUM target_x, target_y, scaled_x, scaled_y, tx, ty, save_heading;
    BOOLEAN wrapping = FALSE;
    
    if (NOT_THROWING) {
	prepare_to_draw;
	draw_turtle();
	move_to(g_round(screen_x_coord), g_round(screen_y_coord));
	target_x = ((xnode == NIL) ?
		turtle_x :
		((nodetype(xnode) == FLOAT) ? getfloat(xnode) :
		 (FLONUM)getint(xnode)));
	target_y = ((ynode == NIL) ?
		turtle_y :
		((nodetype(ynode) == FLOAT) ? getfloat(ynode) :
		 (FLONUM)getint(ynode)));
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
	    wanna_x = turtle_x = scaled_x;
	    wanna_y = turtle_y = scaled_y;
	    out_of_bounds = FALSE;
	    line_to(g_round(screen_x_coord), g_round(screen_y_coord));
	    save_line();
	}
	done_drawing;
	draw_turtle();
    }
}

NODE *lsetpos(NODE *args)
{
    NODE *arg = vector_arg(args);

    if (NOT_THROWING) {
	setpos_helper(car(arg), cadr(arg));
    }
    return(UNBOUND);
}

NODE *lsetxy(NODE *args)
{
    NODE *xnode, *ynode;
    
    xnode = numeric_arg(args);
    ynode = numeric_arg(cdr(args));
    if (NOT_THROWING) {
	setpos_helper(xnode, ynode);
    }
    return(UNBOUND);
}

NODE *lsetx(NODE *args)
{
    NODE *xnode;
    
    xnode = numeric_arg(args);
    if (NOT_THROWING) {
	setpos_helper(xnode, NIL);
    }
    return(UNBOUND);
}

NODE *lsety(NODE *args)
{
    NODE *ynode;
    
    ynode = numeric_arg(args);
    if (NOT_THROWING) {
	setpos_helper(NIL, ynode);
    }
    return(UNBOUND);
}

NODE *lwrap()
{
    if (turtle_shown) draw_turtle();
    current_mode = wrapmode;
    if (turtle_shown) draw_turtle();
    return(UNBOUND);
}

NODE *lfence()
{
    if (turtle_shown) draw_turtle();
    current_mode = fencemode;
    if (turtle_shown) draw_turtle();
    return(UNBOUND);
}

NODE *lwindow()
{
    if (turtle_shown) draw_turtle();
    current_mode = windowmode;
    if (turtle_shown) draw_turtle();
    return(UNBOUND);
}

NODE *lfill()
{
    if (turtle_shown) draw_turtle();
    logofill();
    if (turtle_shown) draw_turtle();
    if (safe_to_save()) {
	record[record_index] = FILLERUP;
	record_index += 2;
    }
    return(UNBOUND);
}

NODE *llabel(NODE *arg)
{
    char textbuf[300];
    short theLength;

    print_stringptr = textbuf;
    print_stringlen = 300;
    ndprintf((FILE *)NULL,"%p",car(arg));
    *print_stringptr = '\0';
	
    if (NOT_THROWING) {
	draw_turtle();
#ifdef x_window
	label(textbuf, strlen(textbuf));
#else
#ifdef mac
	theLength = strlen(textbuf);
	c_to_pascal_string(textbuf, theLength);
#endif
	label(textbuf);
	save_string(textbuf);
	record_next_move = TRUE;
#endif
	draw_turtle();
    }
    return(UNBOUND);
}

NODE *ltextscreen()
{
    text_screen;
    return(UNBOUND);
}

NODE *lsplitscreen()
{
    split_screen;
    return(UNBOUND);
}

NODE *lfullscreen()
{
    full_screen;
    return(UNBOUND);
}

NODE *lpendownp()
{
    return(pen_vis == 0 ? True : False);
}

NODE *lpenmode()
{
    return(get_node_pen_mode);
}

NODE *lpencolor()
{
    return(make_intnode((FIXNUM)pen_color));
}

NODE *lbackground()
{
    return(make_intnode((FIXNUM)back_ground));
}

NODE *lpensize()
{
    return(cons(make_intnode((FIXNUM)pen_width),
	cons(make_intnode((FIXNUM)pen_height), NIL)));
}

NODE *lpenpattern()
{
    return(get_node_pen_pattern);
}

NODE *lpendown()
{
    pen_vis = 0;
    save_vis();
    return(UNBOUND);
}

NODE *lpenup()
{
    if (pen_vis == 0)
	pen_vis--;
    save_vis();
    return(UNBOUND);
}

NODE *lpenpaint()
{
    pen_down;
    save_mode();
    return(lpendown());
}

NODE *lpenerase()
{
    pen_erase;
    save_mode();
    return(lpendown());
}

NODE *lpenreverse()
{
    pen_reverse;
    save_mode();
    return(lpendown());
}

NODE *lsetpencolor(NODE *arg)
{
    NODE *val = pos_int_arg(arg);

    if (NOT_THROWING) {
	prepare_to_draw;
	set_pen_color(getint(val));
	save_color();
    }
    return(UNBOUND);
}

NODE *lsetbackground(NODE *arg)
{
    NODE *val = pos_int_arg(arg);

    if (NOT_THROWING) {
	prepare_to_draw;
	set_back_ground(getint(val));
    }
    return(UNBOUND);
}

NODE *lsetpalette(NODE *args) {
	NODE *slot = pos_int_arg(args);
	NODE *arg = rgb_arg(cdr(args));

	if (NOT_THROWING && ((int)getint(slot) > 7)) {
		set_palette((int)getint(slot),
					(unsigned int)getint(car(arg)),
					(unsigned int)getint(cadr(arg)),
					(unsigned int)getint(car(cddr(arg))));
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

NODE *lsetpensize(NODE *args)
{
    NODE *arg = pos_int_vector_arg(args);

    if (NOT_THROWING) {
	prepare_to_draw;
	set_pen_width((int)getint(car(arg)));
	set_pen_height((int)getint(cadr(arg)));
	save_size();
    }
    return(UNBOUND);
}

NODE *lsetpenpattern(NODE *args)
{    
    NODE *arg;

    arg = car(args);
    ref(arg);
    while ((arg == NIL || !is_list(arg)) && NOT_THROWING)
	arg = reref(arg, err_logo(BAD_DATA, arg));
	
    if (NOT_THROWING) {
	prepare_to_draw;
	set_list_pen_pattern(arg);
	save_pattern();
    }

    deref(arg);
    return(UNBOUND);
}

NODE *lsetscrunch(NODE *args)
{
    NODE *xnode, *ynode;

    xnode = numeric_arg(args);
    ynode = numeric_arg(cdr(args));

    if (NOT_THROWING) {
	prepare_to_draw;
	if (turtle_shown) {
	    draw_turtle();
    	}
	x_scale = (nodetype(xnode) == FLOAT) ? getfloat(xnode) :
			       (FLONUM)getint(xnode);
	y_scale = (nodetype(ynode) == FLOAT) ? getfloat(ynode) :
			       (FLONUM)getint(ynode);
	if (turtle_shown) {
	    draw_turtle();
    	}
	done_drawing;
#ifdef __ZTC__
	{
	    FILE *fp = fopen("scrunch.dat","w");
	    if (fp != NULL) {
		fwrite(&x_scale, sizeof(FLONUM), 1, fp);
		fwrite(&y_scale, sizeof(FLONUM), 1, fp);
		fclose(fp);
	    }
	}
#endif
    }
    return(UNBOUND);
}

NODE *lmousepos()
{
    return(cons(make_intnode(mouse_x), cons(make_intnode(mouse_y), NIL)));
}

NODE *lbuttonp()
{
    if (button)
	return(True);
    return(False);
}

NODE *ltone(args)
NODE *args;
{
    NODE *p, *d;
    FIXNUM pitch, duration;
    
    p = numeric_arg(args);
    d = numeric_arg(cdr(args));
    
    if (NOT_THROWING) {
	pitch = (nodetype(p) == FLOAT) ? (FIXNUM)getfloat(p) : getint(p);
	duration = (nodetype(d) == FLOAT) ? (FIXNUM)getfloat(d) : getint(d);
	if (pitch > 0) tone(pitch, duration);
    }
    
    return(UNBOUND);
}

NODE *larc(NODE *arg)
{
    NODE *val1;
    NODE *val2;

    FLONUM angle;
    FLONUM radius;
    FLONUM ang;
    FLONUM tx;
    FLONUM ty;
    FLONUM oldx;
    FLONUM oldy;
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
          setpos_helper(make_floatnode(tx+x), make_floatnode(ty+y));
          pen_vis = pen_state;
	  save_vis();
          }

       /* else do segment */

       else
          {
          setpos_helper(make_floatnode(tx+x), make_floatnode(ty+y));
          }

       ang = ang + delta;
       }

    /* assure we draw something and end in the exact right place */

    x = sin((turtle_heading+angle)*3.141592654/180.0)*radius;
    y = cos((turtle_heading+angle)*3.141592654/180.0)*radius;

    setpos_helper(make_floatnode(tx+x), make_floatnode(ty+y));

    /* restore state */

    turtle_shown = turtle_state;

    turtle_x = tx;
    turtle_y = ty;

    draw_turtle();
    done_drawing;
    wanna_x = turtle_x;
    wanna_y = turtle_y;
    out_of_bounds = FALSE;

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

BOOLEAN safe_to_save()
{
    return(refresh_p && record_index < (GR_SIZE - 300));
}

save_lm_helper ()
{
    *(int *)(record + record_index + 2) = pen_x;
    *(int *)(record + record_index + 4) = pen_y;
    record_index += 6;
}

save_line()
{
    if (safe_to_save()) {
	record[record_index] = LINEXY;
	save_lm_helper();
    }
}

save_move()
{
    if (safe_to_save()) {
	record[record_index] = MOVEXY;
	save_lm_helper();
    }
}

save_vis()
{
    if (safe_to_save()) {
	record[record_index] = SETPENVIS;
	record[record_index + 1] = (char)pen_vis;
	record_index += 2;
    }
}

save_mode()
{
    if (safe_to_save()) {
	record[record_index] = SETPENMODE;
#ifdef x_window
	*(GC *)(record + record_index + 2) = pen_mode;
#else
	*(int *)(record + record_index + 2) = pen_mode;
#endif
	record_index += 4;
	save_color();
    }
}

save_color()
{
    if (safe_to_save()) {
	record[record_index] = SETPENCOLOR;
	*(long *)(record + record_index + 2) = pen_color;
	record_index += 6;
    }
}

save_size()
{
    if (safe_to_save()) {
	record[record_index] = SETPENSIZE;
	*(int *)(record + record_index + 2) = pen_width;
	*(int *)(record + record_index + 4) = pen_height;
	record_index += 6;
    }
}

save_pattern()
{
    int count;
    
    if (safe_to_save()) {
	record[record_index] = SETPENPATTERN;
	get_pen_pattern(&record[record_index + 2]);
	record_index += 10;
    }
}

save_string(s)
char s[];
{
    int count;

    if (safe_to_save()) {
	record[record_index] = LABEL;
	record[record_index + 2] = s[0];
	for (count = 0; count < s[0]; count++)
	    record[record_index + 3 + count] = s[1 + count];
	record_index += 3 + s[0] + even_p(s[0]);
    }
}

NODE *lrefresh()
{
    refresh_p = TRUE;
    return(UNBOUND);
}

NODE *lnorefresh()
{
    refresh_p = FALSE;
    return(UNBOUND);
}

void redraw_graphics()
{
    int r_index = 0;
    /* pen_info saved_pen; */
    BOOLEAN saved_shown;
    
    if (!refresh_p) {
	draw_turtle();
	return;
    }

    saved_shown = turtle_shown;
    turtle_shown = FALSE;
    /* save_pen(&saved_pen); */
    restore_pen(&orig_pen);

#ifndef unix
    erase_screen();
#endif
#ifdef __TURBOC__
    moveto(p_info_x(orig_pen),p_info_y(orig_pen));
#endif
    while (r_index < record_index)
	switch (record[r_index]) {
	    case (LINEXY) :
		line_to(*(int *)(record + r_index + 2), *(int *)(record + r_index + 4));
		r_index += 6;
		break;
	    case (MOVEXY) :
		move_to(*(int *)(record + r_index + 2), *(int *)(record + r_index + 4));
		r_index += 6;
		break;
	    case (LABEL) :
		draw_string(record + r_index + 2);
		r_index += 3 + record[r_index + 2] + even_p(record[r_index + 2]);
		break;
	    case (SETPENVIS) :
		set_pen_vis(record[r_index + 1]);
		r_index += 2;
		break;
	    case (SETPENMODE) :
#ifdef x_window
		set_pen_mode(*(GC *)(record + r_index + 2));
#else
		set_pen_mode(*(int *)(record + r_index + 2));
#endif
		r_index += 4;
		break;
	    case (SETPENCOLOR) :
		set_pen_color(*(long *)(record + r_index + 2));
		r_index += 6;
		break;
	    case (SETPENSIZE) :
		set_pen_width(*(int *)(record + r_index + 2));
		set_pen_height(*(int *)(record + r_index + 4));
		r_index += 6;
		break;
	    case (SETPENPATTERN) :
		set_pen_pattern(&record[r_index + 2]);
		r_index += 10;
		break;
	    case (FILLERUP) :
		logofill();
		r_index += 2;
		break;
	}

    /* restore_pen(&saved_pen); */
    turtle_shown = saved_shown;
    draw_turtle();
}

/* This is called when the graphics coordinate system has been shifted.
   It adds a constant amount to each x and y coordinate in the record. */
void resize_record(int dh, int dv)
{
    int r_index = 0;
    
    p_info_x(orig_pen) += dh;
    p_info_y(orig_pen) += dv;
    
    while (r_index < record_index)
	switch (record[r_index]) {
	    case (LINEXY) :
		*(int *)(record + r_index + 2) += dh;
		*(int *)(record + r_index + 4) += dv;
		r_index += 6;
		break;
	    case (MOVEXY) :
		*(int *)(record + r_index + 2) += dh;
		*(int *)(record + r_index + 4) += dv;
		r_index += 6;
		break;
	    case (LABEL) :
		r_index += 3 + record[r_index + 2] + even_p(record[r_index + 2]);
		break;
	    case (SETPENVIS) :
		r_index += 2;
		break;
	    case (SETPENMODE) :
		r_index += 4;
		break;
	    case (SETPENCOLOR) :
		r_index += 6;
		break;
	    case (SETPENSIZE) :
		r_index += 6;
		break;
	    case (SETPENPATTERN) :
		r_index += 10;
		break;
	    case (FILLERUP) :
		r_index += 2;
		break;
	}
}
