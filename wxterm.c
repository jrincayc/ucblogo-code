/*
 *	wxterm.c	terminal cursor control			dvb
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

char* LogoPlatformName="wxWidgets";
char wx_font_face[300] = "Courier";   //300 matches lsetfont in wxterm.c
int wx_font_size = 12;	
int label_height = 15;

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

char* wx_fgets(char* s, int n, FILE* stream) {
	char c;
	char * orig = s;
	if (stream != stdin) {
	  return fgets(s, n, stream);
	}
  
	 charmode_on();

 
	 n --;
	 c = ' ';
  while (c != '\n' && n != 0) {
    c = getFromWX_2(stream);
    s[0] = c;
    s++;
    n--;
  }
  return orig;
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

extern void wxSetCursor(int, int);
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
    char *old_stringptr = print_stringptr;
    int old_stringlen = print_stringlen;

    sprintf(fmtbuf,"%c%%p%c",17,17);
    print_stringptr = textbuf;
    print_stringlen = 300;
    ndprintf((FILE *)NULL,fmtbuf,car(args));
    *print_stringptr = '\0';
    print_stringptr = old_stringptr;
    print_stringlen = old_stringlen;
    return make_strnode(textbuf,NULL,(int)strlen(textbuf),STRING,strnzcpy);
}

extern void wxSetFont(char *fm, int sz);



NODE *lfont(NODE *arg) {
  NODE *val;

  return make_static_strnode(wx_font_face);

}

NODE *lsetfont(NODE *arg) {
  char textbuf[300];                                                      
  print_stringptr = textbuf;
  print_stringlen = 300;
  ndprintf((FILE *)NULL, "%p", car(arg));
  *print_stringptr = '\0';

  if (NOT_THROWING) {
    wxSetFont(textbuf, wx_font_size);
  }

  return(UNBOUND);
}

NODE *lsettextsize(NODE *arg) {
  NODE *val = integer_arg(arg);
  if (NOT_THROWING) {
    wxSetFont(wx_font_face, getint(val));
  }

  return(UNBOUND);
}

NODE *ltextsize(NODE *arg) {
  return make_intnode(wx_font_size);
}

extern void wx_adjust_label_height();

NODE *lsetlabelheight(NODE *arg) {
  NODE *val = integer_arg(arg);
  label_height = getint(val);
  wx_adjust_label_height();

  return(UNBOUND);
}


extern void wx_get_label_size(int *w, int *h);

NODE *llabelsize(NODE *arg) {
  int w,h;
  wx_get_label_size(&w, &h);
  return cons(make_intnode(w/x_scale),
	      cons(make_intnode(h/y_scale), NIL));
}

extern void wxSetTextColor(FIXNUM, FIXNUM);

NODE *set_text_color(NODE *args) {
    NODE *fgcolor, *bgcolor;

    if (is_list(car(args))) {
	fgcolor = make_intnode(TEXT_FG_COLOR_OFFSET);
	lsetpalette(cons(fgcolor,args));
    } else {
	fgcolor = pos_int_arg(args);
    }

    if (is_list(cadr(args))) {
	bgcolor = make_intnode(TEXT_BG_COLOR_OFFSET);
	lsetpalette(cons(bgcolor,cdr(args)));
    } else {
	bgcolor = pos_int_arg(cdr(args));
    }

    if (NOT_THROWING) {
	wxSetTextColor(getint(fgcolor)+SPECIAL_COLORS,
		       getint(bgcolor)+SPECIAL_COLORS);
    }

    return UNBOUND;
}

void color_init() {
    wxSetTextColor(7+SPECIAL_COLORS, 0+SPECIAL_COLORS);
}
