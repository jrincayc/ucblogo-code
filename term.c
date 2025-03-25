/*
 *	term.c		terminal cursor control			dvb
 *
 *	Copyright (C) 1993 by the Regents of the University of California
 *
 *      This program is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "logo.h"
#include "globals.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(HAVE_TERMIOS_H)
#include <termios.h>
#elif defined(HAVE_TERMIO_H)
#include <termio.h>
#endif

#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
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

int x_coord, y_coord, x_max, y_max;

char PC;
char *BC;
char *UP;
/* short ospeed; */
char bp[1024];
char cl_arr[40];
char cm_arr[40];
char so_arr[40];
char se_arr[40];

struct termio tty_cooked, tty_cbreak;

int interactive, tty_charmode;

extern char **environ;

char *termcap_ptr;

int termcap_putter(int ch) {
    /* XXX: Should this check for any non-char values? */
    *termcap_ptr++ = (char)ch;
    return 0;
}

void termcap_getter(char *cap, char *buf) {
    char temp[40];
    char *str;
    char *temp_ptr = temp;

    termcap_ptr = buf;
    str=tgetstr(cap,&temp_ptr);
    /* if (str == NULL) str = temp; */
    tputs(str,1,termcap_putter);
}

void term_init(void) {
    char *emacs; /* emacs change */
    int term_sg;
    int tgetent_result;

    interactive = isatty(0);

    if (interactive) {
	ioctl(0,TCGETA,(char *)(&tty_cooked));
	tty_cbreak = tty_cooked;
	tty_cbreak.c_cc[VMIN] = '\01';
	tty_cbreak.c_cc[VTIME] = '\0';
	tty_cbreak.c_lflag &= ~(ECHO|ICANON);
    }
    tty_charmode = 0;

    /* The following section assumes
     * x_max, y_max, cm_arr, cl_arr, so_arr, se_arr
     * are preinitialized to 0 */

    /* query terminal information from termcap database, if available */
    tgetent_result = tgetent(bp, getenv("TERM"));
    if (tgetent_result == 1) {
      x_max = tgetnum("co");
      y_max = tgetnum("li");

      term_sg = tgetnum("sg");

      x_coord = y_coord = 0;
      termcap_getter("cm", cm_arr);
      termcap_getter("cl", cl_arr);

      if (term_sg <= 0) {
          termcap_getter("so", so_arr);
          termcap_getter("se", se_arr);
      } else { /* no standout modes */
	so_arr[0] = se_arr[0] = '\0';
      }
    }
    
    /* emacs detection */
    emacs = getenv("EMACS");
    if (emacs && *emacs == 't') { /* started from emacs */
      emacs = getenv("EMACS");
      emacs = getenv("COLUMNS");
      if (!emacs) x_max = 0;
      else x_max = atoi(emacs);
      emacs = getenv("LINES");
      if (!emacs) y_max = 0;
      else y_max = atoi(emacs);
    }
    /* end emacs detection */

    /* if we still don't know our size, set some defaults */
    if (x_max <= 0) x_max = 80;
    if (y_max <= 0) y_max = 24;
}

void charmode_on() {
    if ((readstream == stdin) && interactive && !tty_charmode) {
	ioctl(0,TCSETA,(char *)(&tty_cbreak));
	tty_charmode++;
    }
}

void charmode_off() {
    if (tty_charmode) {
	ioctl(0,TCSETA,(char *)(&tty_cooked));
	tty_charmode = 0;
    }
}

NODE *lcleartext(NODE *args) {
    printf("%s", cl_arr);
    printf("%s", tgoto(cm_arr, x_margin, y_margin));

	fflush(stdout); /* do it now! */
	fix_turtle_shownness();

    x_coord = x_margin;
    y_coord = y_margin;
    return(UNBOUND);
}

NODE *lcursor(NODE *args) {

    // Flush buffer so it doesn't impact cursor position.
    fflush(stdout);

    return(cons(make_intnode((FIXNUM)(x_coord-x_margin)),
		cons(make_intnode((FIXNUM)(y_coord-y_margin)), NIL)));
}

NODE *lsetcursor(NODE *args) {
	fix_turtle_shownness();
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
	printf("%s", tgoto(cm_arr, x_coord, y_coord));
	fflush(stdout);
    }
    return(UNBOUND);
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
