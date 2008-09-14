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
#ifdef HAVE_WX
#include <termios.h>
#else
#include <termio.h>
#endif
#else
#ifdef HAVE_SGTTY_H
#include <sgtty.h>
#endif
#endif

#undef TRUE
#undef FALSE

#ifdef HAVE_TERMCAP_H
#include <termcap.h>
#ifdef HAVE_SGTTY_H
#include <sgtty.h>
#endif
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

int x_coord, y_coord, x_max, y_max;

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
struct termio tty_cooked, tty_cbreak;
#else
#ifdef HAVE_SGTTY_H
struct sgttyb tty_cooked, tty_cbreak;
#endif
#endif

int interactive, tty_charmode;

extern char **environ, *tgoto(), *tgetstr();

char *termcap_ptr;

int termcap_putter(char ch) {
    *termcap_ptr++ = ch;
    return 0;
}

#ifdef unix
void termcap_getter(char *cap, char *buf) {
    char temp[40];
    char *str;
    char *temp_ptr = temp;

    termcap_ptr = buf;
    str=tgetstr(cap,&temp_ptr);
    /* if (str == NULL) str = temp; */
    tputs(str,1,termcap_putter);
}
#endif

void term_init(void) {
#ifdef unix
    char *emacs; /* emacs change */
    int term_sg;
#endif

#ifdef WIN32
    interactive = 1;
#else
    interactive = isatty(0);
#endif

#ifdef mac
    term_init_mac();
    return;
#else
#ifdef ibm
#ifndef WIN32
    term_init_ibm();
#endif /* WIN32 */
#else
    if (interactive) {
#ifdef HAVE_TERMIO_H
	ioctl(0,TCGETA,(char *)(&tty_cooked));
	tty_cbreak = tty_cooked;
	tty_cbreak.c_cc[VMIN] = '\01';
	tty_cbreak.c_cc[VTIME] = '\0';
	tty_cbreak.c_lflag &= ~(ECHO|ICANON);
#else
	ioctl(0,TIOCGETP,(char *)(&tty_cooked));
	tty_cbreak = tty_cooked;
	tty_cbreak.sg_flags |= CBREAK;
	tty_cbreak.sg_flags &= ~ECHO;
#endif
    }
    tty_charmode = 0;
    tgetent(bp, getenv("TERM"));

    /* emacs changes */

    emacs = getenv("EMACS");

    /* check if started from emacs */
    if (!emacs || *emacs != 't') { /* read from termcap */
      x_max = tgetnum("co");
      y_max = tgetnum("li");
    }
    else { /* read environment variables */
      emacs = getenv("COLUMNS");
      if (!emacs) x_max = 0;
      else x_max = atoi(emacs);
      emacs = getenv("LINES");
      if (!emacs) y_max = 0;
      else y_max = atoi(emacs);
    } 

    /* end emacs changes */

    if (x_max <= 0) x_max = 80;
    if (y_max <= 0) y_max = 24;
    term_sg = tgetnum("sg");

    x_coord = y_coord = 0;
    termcap_getter("cm", cm_arr);
    termcap_getter("cl", cl_arr);

    if (term_sg <= 0) {
	termcap_getter("so", so_arr);
	termcap_getter("se", se_arr);
    } else	/* don't mess with stupid standout modes */
	so_arr[0] = se_arr[0] = '\0';
#endif
#endif
}

void charmode_on() {
#ifdef unix
    if ((readstream == stdin) && interactive && !tty_charmode) {
#ifdef HAVE_TERMIO_H
	ioctl(0,TCSETA,(char *)(&tty_cbreak));
#else /* !HAVE_TERMIO_H */
	ioctl(0,TIOCSETP,(char *)(&tty_cbreak));
#endif /* HAVE_TERMIO_H */
	tty_charmode++;
    }
#endif /* unix */
#ifdef WIN32
    win32_charmode_on();
#endif
}

void charmode_off() {
#ifdef unix
    if (tty_charmode) {
#ifdef HAVE_TERMIO_H
	ioctl(0,TCSETA,(char *)(&tty_cooked));
#else /* !HAVE_TERMIO_H */
	ioctl(0,TIOCSETP,(char *)(&tty_cooked));
#endif /* HAVE_TERMIO_H */
	tty_charmode = 0;
    }
#endif /* unix */
#ifdef WIN32
    win32_charmode_off();
#endif
}

NODE *lcleartext(NODE *args) {
#ifdef mac
    cgotoxy(x_margin + 1, y_margin + 1, stdout);
    ccleos(stdout);
#else
#ifdef ibm
#ifndef WIN32 /* sowings */
    ibm_clear_text();
    ibm_gotoxy(x_margin, y_margin);
#else /* WIN32 */
    win32_clear_text();
#endif /* WIN32 || !Win32 */
#else /* !ibm */
    printf("%s", cl_arr);
    printf("%s", tgoto(cm_arr, x_margin, y_margin));
#endif /* ibm */
#endif /* mac */

#ifdef WIN32
	win32_update_text();
#else
	fflush(stdout); /* do it now! */
#endif
	fix_turtle_shownness();
#if defined(__RZTC__)
	zflush();
#endif

    x_coord = x_margin;
    y_coord = y_margin;
    return(UNBOUND);
}

NODE *lcursor(NODE *args) {
    return(cons(make_intnode((FIXNUM)(x_coord-x_margin)),
		cons(make_intnode((FIXNUM)(y_coord-y_margin)), NIL)));
}

NODE *lsetcursor(NODE *args) {
	fix_turtle_shownness();
#ifdef WIN32
    return (win32_lsetcursor(args));
#else /* !win32 */
    NODE *arg;

    arg = pos_int_vector_arg(args);
    if (NOT_THROWING) {
	x_coord = x_margin + getint(car(arg));
	y_coord = y_margin + getint(cadr(arg));
	while ((x_coord >= x_max || y_coord >= y_max) && NOT_THROWING) {
	    setcar(args, err_logo(BAD_DATA, arg));
	    if (NOT_THROWING) {
		arg = pos_int_vector_arg(args);
		x_coord = x_margin + getint(car(arg));
		y_coord = y_margin + getint(cadr(arg));
	    }
	}
    }
    if (NOT_THROWING) {
#ifdef mac
	mac_gotoxy(x_coord, y_coord);
#else
#ifdef ibm
	ibm_gotoxy(x_coord, y_coord);
#else
	printf("%s", tgoto(cm_arr, x_coord, y_coord));
#endif
#endif
	fflush(stdout);
#ifdef __RZTC__
	zflush();
#endif
    }
    return(UNBOUND);
#endif /* !win32 (for non-windows version of this code) */
}

NODE *lsetmargins(NODE *args) {
    NODE *arg;

    arg = pos_int_vector_arg(args);
    if (NOT_THROWING) {
	x_margin = getint(car(arg));
	y_margin = getint(cadr(arg));
	lcleartext(NIL);
    }
    return(UNBOUND);
}

NODE *lstandout(NODE *args) {
    char textbuf[300];
    char fmtbuf[100];
    char *old_stringptr = print_stringptr;
    int old_stringlen = print_stringlen;

    sprintf(fmtbuf,"%s%%p%s",so_arr,se_arr);
    print_stringptr = textbuf;
    print_stringlen = 300;
    ndprintf((FILE *)NULL,fmtbuf,car(args));
    *print_stringptr = '\0';
    print_stringptr = old_stringptr;
    print_stringlen = old_stringlen;
    return(make_strnode(textbuf,NULL,(int)strlen(textbuf),STRING,strnzcpy));
}
