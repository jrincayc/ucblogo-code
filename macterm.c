/*
 *      macterm.c          macintosh screen module          mak
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
#include "globals.h"
#include "macterm.h"
#include <console.h>
#include <math.h>
#include <Sound.h>
#include <Types.h>
#include <GestaltEqu.h>
#include <Palettes.h>

char windowtitle[100];
FILE *graphics, *console;
WindowPtr graphics_window, listener_window;
GrafPtr savePort;
extern WindowPtr myWindow; /* the editor window */
FIXNUM pen_color = 7, back_ground = 0;
extern void save_color();

/************************************************************/

void nop()
{
}

int can_do_color = 0, want_color = 0;
PaletteHandle the_palette;

void get_can_do_color() {
	long ans;

	if (!Gestalt(gestaltQuickdrawVersion, &ans))
		can_do_color = (ans != 0);
}

void init_mac_memory()
{
    unsigned long AZ, AL;
    /* SetApplLimit((Ptr)(GetApplLimit() - 150000L)); MaxApplZone(); */
    
    AL = (unsigned long)GetApplLimit(); AZ = (unsigned long)ApplicZone();
/*    SetApplLimit((Ptr) ( (AZ + ((AL-AZ)*3)/4) & 0x00fffffe )); */
		/* SetApplLimit((Ptr)(AZ + 300000L)); */
    SetApplLimit((Ptr)((AZ + ((AL-AZ)*3)/4) & -2L)); /* PCB */
    MaxApplZone();
}

BOOLEAN check_mac_stop()
{
/*
	unsigned long   theStackPointer;
    unsigned long   AL;
 */
    char	    the_key_map[16];

/*
    asm {
	Move.L  SP,theStackPointer
    }
    AL = (unsigned long)(GetApplLimit());
    AL = AL + 5000;
    if (theStackPointer < AL || FreeMem() < 3000) {
*/
    if (FreeMem() < 3000) {
	err_logo(STACK_OVERFLOW, NIL);
	return(1);
    }
    GetKeys(&the_key_map);
    if (the_key_map[5] & 128 && the_key_map[6] & 128) {
		    /* period and command are down */
	FlushEvents(everyEvent, 0);
	logo_stop();
	return 1;
    } else if (the_key_map[5] & 8 && the_key_map[6] & 128) {
		    /* comma and command are down */
	FlushEvents(everyEvent, 0);
	logo_pause();
//	return 1;
    }
    return(0);
}

void term_init_mac()
{
    MenuHandle menu_handle;
    
	get_can_do_color();
    tty_charmode = 0;
    x_max = 80;
    y_max = 24;
    console_options.title = windowtitle;
    console_options.top-= 5;
    console_options.left-= 5;
    strncpy(console_options.title, "\pGraphics", 9);
	want_color = 1;
    graphics = fopenc();
	want_color = 0;
    cgotoxy(1, 1, graphics);
    graphics_window = FrontWindow();
	if (can_do_color) {
		the_palette = GetNewPalette(129);
		SetPalette(graphics_window, the_palette, 1);
		ActivatePalette(graphics_window);
	}
    SizeWindow(graphics_window, graphics_window->portRect.right,
				graphics_window->portRect.bottom - 1, 0);
    cs_helper(TRUE);

    console_options.top+= 10;
    console_options.left+= 10;
    strncpy(console_options.title, "\pBerkeley Logo", 14);
    console = fopenc();
    lregulartext();
    cinverse(1,stdout);
    listener_window = FrontWindow();
    
    console_options.title[0] = 0;
    
    menu_handle = GetMHandle(3);
    AppendMenu(menu_handle,
	       "\p(-;(Accept Editor Changes/A;(Cancel Editor Changes");
    AppendMenu(menu_handle, "\p(-;(Page Setup;(Print/P");
    SetUpWindows();
    prepare_to_draw;
    mac_set_bg(0L);
    mac_set_pc(7L);
    save_color();
    done_drawing;

    x_coord = y_coord = 0;
    so_arr[0] = '\1'; so_arr[1] = '\0';
    se_arr[0] = '\2'; se_arr[1] = '\0';
}

void mac_gotoxy(int x, int y)
{
    if (x_coord < 0) x_coord = 0;
    if (x_coord >= console_options.ncols) x_coord = console_options.ncols - 1;
    if (y_coord < 0) y_coord = 0;
    if (y_coord >= console_options.nrows) y_coord = console_options.nrows - 1;
    cgotoxy(x_coord + 1, y_coord + 1, stdout);
}

/************************************************************/
/* These are primitives that can only exist on the mac and/or are ad hoc
   things Michael invented that we probably don't want to keep around in Berkeley Logo. */

NODE *lsetwindowtitle(arg)
NODE *arg;
{
    NODE *name;
    
    name = string_arg(arg);
    if (name != UNBOUND) {
	noparity_strnzcpy((char *)(windowtitle + 1), getstrptr(name), (int)getstrlen(name));
	windowtitle[0] = (char)getstrlen(name);
    }
    return(UNBOUND);
}

option_helper(var, arg)
short *var;
NODE *arg;
{
    NODE *val;
    
    val = integer_arg(arg);
    if (NOT_THROWING)
	*var = (short)getint(val);
}

NODE *lsettextfont(arg)
NODE *arg;
{
    option_helper(&console_options.txFont, arg);
    return(UNBOUND);
}

NODE *lsettextsize(arg)
NODE *arg;
{
    option_helper(&console_options.txSize, arg);
    return(UNBOUND);
}

NODE *lsettextstyle(arg)
NODE *arg;
{
    option_helper(&console_options.txFace, arg);
    return(UNBOUND);
}

NODE *lsetwindowsize(args)
NODE *args;
{
    NODE *xnode, *ynode = UNBOUND, *arg;

    arg = pos_int_vector_arg(args);
    if (NOT_THROWING) {
	xnode = car(arg);
	ynode = cadr(arg);
	console_options.ncols =  max((int)getint(xnode), 40);
	console_options.nrows =  max((int)getint(ynode), 5);
    }
    return(UNBOUND);
}

NODE *lsetwindowxy(args)
NODE *args;
{
    NODE *xnode, *ynode = UNBOUND, *arg;

    arg = pos_int_vector_arg(args);
    if (NOT_THROWING) {
	xnode = car(arg);
	ynode = cadr(arg);
	console_options.left = (int)getint(xnode);
	console_options.top = (int)getint(ynode);
    }
    return(UNBOUND);
}

NODE *lnewconsole()
{
    FILE *c, *old;
    int was_graphics;
    
    was_graphics = (FrontWindow() == graphics_window);
    
    chide(stdin);
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);
    c = fopenc();
    fclose(stdin);
    fclose(stdout);
    freopenc(c, stdin);
    freopenc(c, stdout);
    freopenc(c, stderr);
    lcleartext();
    cinverse(1,stdout);
    
    if (was_graphics) {
	graphics_window = FrontWindow();
	graphics = c;
    } else {
	listener_window = FrontWindow();
	console = c;
	x_max = console_options.ncols;
	y_max = console_options.nrows;
    }
    return(UNBOUND);
}

NODE *lgraphtext()
{
    freopenc(graphics, stdin);
    freopenc(graphics, stdout);
    freopenc(graphics, stderr);
    return(UNBOUND);
}

NODE *lregulartext()
{
    freopenc(console, stdin);
    freopenc(console, stdout);
    freopenc(console, stderr);
    return(UNBOUND);
}

NODE *lcaninverse(NODE *args) {
    FIXNUM onoff = int_arg(args);

    if (NOT_THROWING) cinverse(onoff, stdin);
    return(UNBOUND);
}

/************************************************************/
/* These are the machine-specific graphics definitions.  All versions must provide
   a set of functions analogous to these. */

void save_pen(pen_info *p)
{
    GetPort(&savePort);
    SetPort(graphics_window);
    GetPenState(&(p->ps));
    p->vis = graphics_window->pnVis;
//    p->color = graphics_window->fgColor;
	p->color = pen_color;
    SetPort(savePort);
}

void restore_pen(pen_info *p)
{
    GetPort(&savePort);
    SetPort(graphics_window);
    SetPenState(&(p->ps));
    graphics_window->pnVis = p->vis;
//    graphics_window->fgColor = p->color;
	mac_set_pc(p->color);
    SetPort(savePort);
}

void plain_xor_pen()
{
    PenNormal();
    PenMode(patXor);
}

FIXNUM color_table[8] = {33, 409, 341, 273, 205, 137, 69, 30};

FIXNUM hw_color(FIXNUM c) {
    if (c >= 0 && c < 8) return color_table[c];
    if (c < 0) return c;
    return c-8;
}

int palette_color(FIXNUM c) {
	if (c == 7) return 0;
	if (c >= 0 && c < 7) return c+1;
	return c;
}

void mac_set_pc(FIXNUM c) {
    pen_color = c;
	if (can_do_color)
		PmForeColor(palette_color(c));
	else
		graphics_window->fgColor = hw_color(c);
}

void mac_set_bg(FIXNUM c) {
    back_ground = c;
	if (can_do_color)
		PmBackColor(palette_color(c));
	else
		graphics_window->bkColor = hw_color(c);
    redraw_graphics();
}

void set_palette(int slot, unsigned int r, unsigned int g, unsigned int b) {
	RGBColor rgb;

	if (can_do_color) {
		rgb.red = r;
		rgb.green = g;
		rgb.blue = b;
		SetEntryColor(the_palette, slot, &rgb);
		SetEntryUsage(the_palette, slot, pmTolerant, 2000);
		ActivatePalette(graphics_window);
		redraw_graphics();
	}
}

void get_palette(int slot, unsigned int *r, unsigned int *g, unsigned int *b) {
	RGBColor rgb;

	if (can_do_color) {
		GetEntryColor(the_palette, palette_color((FIXNUM)slot), &rgb);
		*r = rgb.red;
		*g = rgb.green;
		*b = rgb.blue;
	} else {
		*r = (slot&4 ? 65000 : 0);
		*g = (slot&2 ? 65000 : 0);
		*b = (slot&1 ? 65000 : 0);
	}
}

void set_pen_pattern(char *pat)
{
	PenPat(pat);
}

void set_list_pen_pattern(NODE *arg)
{
    NODE *cur_num, *temp;
    char p_arr[8];
    int count;

    cur_num = arg;
    for (count = 0 ; count <= 7 && cur_num != NIL && NOT_THROWING ; count++) {
	temp = cnv_node_to_numnode(car(cur_num));
	p_arr[count] = (char)getint(temp);
	cur_num = cdr(cur_num);
	gcref(temp);
    }
    PenPat(p_arr);
}

void get_pen_pattern(char *pat)
{
    PenState oil;
    int count;

    GetPenState(&oil);
    for (count = 0; count < 8; count++)
	 *(char *)(pat + count) = oil.pnPat[count];
}

NODE *Get_node_pen_pattern()
{
    PenState oil;
    int count;

    GetPenState(&oil);
    return(cons(make_intnode((FIXNUM)(oil.pnPat[0])),
	    cons(make_intnode((FIXNUM)(oil.pnPat[1])),
	     cons(make_intnode((FIXNUM)(oil.pnPat[2])),
	      cons(make_intnode((FIXNUM)(oil.pnPat[3])),
	       cons(make_intnode((FIXNUM)(oil.pnPat[4])),
		cons(make_intnode((FIXNUM)(oil.pnPat[5])),
		 cons(make_intnode((FIXNUM)(oil.pnPat[6])),
		  cons(make_intnode((FIXNUM)(oil.pnPat[7])),
		   NIL)))))))));
}

NODE *Get_node_pen_mode()
{
    switch(pen_mode) {
	case patCopy   : return(make_static_strnode("paint"));
	case patBic    : return(make_static_strnode("erase"));
	case patXor    : return(make_static_strnode("reverse"));
	default	       : return(make_static_strnode("unknown"));
    }
}

void label(char *s)
{
    GetPort(&savePort);
    SetPort(graphics_window);
    MoveTo(g_round(graphics_window->portRect.right/2.0 + turtle_x),
	   g_round(graphics_window->portRect.bottom/2.0 - turtle_y));
    TextFont(monaco); TextSize(9); TextMode(srcOr);
    DrawString(s);
    SetPort(savePort);
}

void logofill() {
	BitMap mask;

	if (can_do_color) {
		prepare_to_draw;
		mask.bounds = (*(graphics_window->visRgn))->rgnBBox;
		mask.bounds.right = mask.bounds.right - mask.bounds.left;
		mask.bounds.left = 0;
		mask.bounds.bottom = mask.bounds.bottom - mask.bounds.top;
		mask.bounds.top = 0;
		mask.rowBytes = ((mask.bounds.right + 15) / 8) & ~1;
		mask.baseAddr = malloc(mask.rowBytes * mask.bounds.bottom);
		SeedCFill(&(graphics_window->portBits), &mask,
				  &((*(graphics_window->visRgn))->rgnBBox),
				  &(mask.bounds),
				  graphics_window->pnLoc.h, graphics_window->pnLoc.v,
				  0, 0L);
		CopyMask(&mask, &mask, &(graphics_window->portBits),
				 &(mask.bounds), &(mask.bounds),
				 &((*(graphics_window->visRgn))->rgnBBox));
		free(mask.baseAddr);
		done_drawing;
	}
}

void erase_screen()
{
    int old_vis;
    
    GetPort(&savePort);
    SetPort(graphics_window);
    old_vis = graphics_window->pnVis;
    graphics_window->pnVis = 0;
    EraseRect(&graphics_window->portRect);
    graphics_window->pnVis = old_vis;
    SetPort(savePort);
}

void t_screen()
{
    console_options.ncols = 80;
    console_options.nrows = 25;
    console_options.left = 15;
    console_options.top = 55;
    strncpy(console_options.title, "\pBerkeley Logo", 14);
    lnewconsole();

    MoveWindow(myWindow, 15, 55, TRUE);
    MySizeWindow(myWindow, 488, 283);
    SelectWindow(listener_window);
}

void s_screen()
{
    Rect bounds;
    int v;
    
	if (can_do_color)
		v = ((*(((CGrafPtr)graphics_window)->portPixMap))->bounds.bottom -
			 (*(((CGrafPtr)graphics_window)->portPixMap))->bounds.top) - 84;
	else
		v = (graphics_window->portBits.bounds.bottom -
			 graphics_window->portBits.bounds.top) - 84;

    console_options.ncols = 80;
    console_options.nrows = 6;
    console_options.left = 5;
    console_options.top = v + 6;
    strncpy(console_options.title, "\pBerkeley Logo", 14);
    lnewconsole();

    MoveWindow(myWindow, 5, v, TRUE);
    MySizeWindow(myWindow, 488, 80);
    SelectWindow(listener_window);
}

void f_screen()
{
    Rect bounds;
    int v;
    
	if (can_do_color)
		v = ((*(((CGrafPtr)graphics_window)->portPixMap))->bounds.bottom -
			 (*(((CGrafPtr)graphics_window)->portPixMap))->bounds.top) - 84;
	else
		v = (graphics_window->portBits.bounds.bottom -
			 graphics_window->portBits.bounds.top) - 84;

    console_options.ncols = 80;
    console_options.nrows = 0;
    console_options.left = 5;
    console_options.top = v + 52;
    strncpy(console_options.title, "\pBerkeley Logo", 14);
    lnewconsole();

    MoveWindow(myWindow, 5, v, TRUE);
    MySizeWindow(myWindow, 488, 80);
    SelectWindow(graphics_window);
}

FIXNUM mickey_x()
{
    Point the_mouse;
    
    GetPort(&savePort);
    SetPort(graphics_window);
    GetMouse(&the_mouse);
    SetPort(savePort);
    
    return((FIXNUM)(the_mouse.h - graphics_window->portRect.right/2));
}

FIXNUM mickey_y()
{
    Point the_mouse;
    
    GetPort(&savePort);
    SetPort(graphics_window);
    GetMouse(&the_mouse);
    SetPort(savePort);
    
    return((FIXNUM)(graphics_window->portRect.bottom/2 - the_mouse.v));
}

/* see Inside Macintosh vol. 2 pp. 237-241 for pitch values */
void tone(pitch, duration)
FIXNUM pitch, duration;
{
    struct { int mode;
	     int freq;
	     int amp;
	     int dur;
	   } sound_rec;
    
    sound_rec.mode = -1;
    sound_rec.freq = (int)(387205L/pitch);
    sound_rec.amp = 200;
    sound_rec.dur = (int)duration;
    
    StartSound(&sound_rec, (long)8, (char *)(-1));
    while (!SoundDone()) ;
}

/************************************************************/

void c_to_pascal_string(str, len)
char *str;
int len;
{
    int count = len;
    char prev;
    
    while (count > 0) {
	prev = str[count - 1];
	str[count] = clearparity(prev);
	count--;
    }
    str[0] = len;
}
