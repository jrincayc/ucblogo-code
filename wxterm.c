/*
 *	term.c		terminal cursor control			dvb
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef mac
#include <console.h>
#endif

#ifdef HAVE_TERMIO_H
#include <termios.h>
#else
#ifdef HAVE_SGTTY_H
#include <sgtty.h>
#endif
#endif

#undef TRUE
#undef FALSE

#ifdef HAVE_TERMCAP_H
#include <termcap.h>
#else
#ifdef HAVE_TERMLIB_H
#include <termlib.h>
#else
#ifdef HAVE_CURSES_H
#include <curses.h>
#endif
#endif
#endif

#undef TRUE
#undef FALSE

#define FALSE	0
#define TRUE	1

char PC;
char *BC;
char *UP;
short ospeed;
char bp[1024];
char cl_arr[40];
char cm_arr[40];
char so_arr[40];
char se_arr[40];

#ifdef HAVE_TERMIO_H
struct termios tty_cooked, tty_cbreak;
#else
#ifdef HAVE_SGTTY_H
struct sgttyb tty_cooked, tty_cbreak;
#endif
#endif

int interactive, tty_charmode;
int getTermInfo(int type);
int setTermInfo(int type, int val);

extern char **environ, *tgoto(), *tgetstr();

char *termcap_ptr;

int termcap_putter(char ch) {
    *termcap_ptr++ = ch;
    return 0;
}

#ifdef unix
void termcap_getter(char *cap, char *buf) {
	
}
#endif

void term_init(void) {
  

    interactive = 1;
 
  
}

void setCharMode(int);

void charmode_on() {
    setCharMode(1);
}

void charmode_off() {
  setCharMode(0);;
}

extern void wxClearText();

NODE *lcleartext(NODE *args) {
	int x_coord, y_coord;
	x_coord=getTermInfo(X_COORD);
	y_coord=getTermInfo(Y_COORD);
	
    wxClearText();
    x_coord = x_margin;
    y_coord = y_margin;
	setTermInfo(X_COORD,x_coord);
	setTermInfo(Y_COORD,y_coord);
    return(UNBOUND);
}

NODE *lcursor(NODE *args) {
	int x_coord, y_coord;
	x_coord=getTermInfo(X_COORD);
	y_coord=getTermInfo(Y_COORD);

	
  return(cons(make_intnode((FIXNUM)(x_coord-x_margin)),
	      cons(make_intnode((FIXNUM)(y_coord-y_margin)), NIL)));
}

extern void wxSetCursor();
extern void wx_refresh();

NODE *lsetcursor(NODE *args) {

	int x_coord, y_coord, x_max, y_max;
	NODE *arg;
	fix_turtle_shownness();
        wx_refresh();

	x_coord=getTermInfo(X_COORD);
	y_coord=getTermInfo(Y_COORD);
	x_max=getTermInfo(X_MAX);
	y_max=getTermInfo(Y_MAX);
	
  

    arg = pos_int_vector_arg(args);
    if (NOT_THROWING) {
	x_coord = x_margin + getint(car(arg));
	y_coord = y_margin + getint(cadr(arg));
	while ((x_coord >= x_max || y_coord >= y_max) && NOT_THROWING) {
	    setcar(args, err_logo(BAD_DATA, arg));
	    if (NOT_THROWING) {
		arg = pos_int_vector_arg(args);
		x_coord = x_margin + getint(cadr(arg));
		y_coord = y_margin + getint(car(arg));
	    }
	}
    }
    if (NOT_THROWING) {
      wxSetCursor(x_coord,y_coord);
    }
    return(UNBOUND);

}

NODE *lsetmargins(NODE *args) {
    NODE *arg;

    arg = pos_int_vector_arg(args);
    if (NOT_THROWING) {
	x_margin = getint(cadr(arg));
	y_margin = getint(car(arg));
	lcleartext(NIL);
    }
    return(UNBOUND);
}

NODE *lstandout(NODE *args) {
  char textbuf[300];
  char fmtbuf[100];

    sprintf(fmtbuf,"%c%%p%c",17,17);
    print_stringptr = textbuf;
    print_stringlen = 300;
    ndprintf((FILE *)NULL,fmtbuf,car(args));
    *print_stringptr = '\0';
    return(make_strnode(textbuf,NULL,(int)strlen(textbuf),STRING,strnzcpy));
}
