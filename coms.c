/*
 *      coms.c	  program execution control module	dvb
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
#include <math.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef ibm
#include "process.h"
#endif
#ifdef mac
#include <console.h>
#endif
#ifdef __ZTC__
#include <time.h>
#include <controlc.h>
#include <dos.h>
#endif

#ifdef HAVE_TERMIO_H
#include <termio.h>
#else
#ifdef HAVE_SGTTY_H
#include <sgtty.h>
#endif
#endif

FIXNUM ift_iff_flag = -1;

NODE *make_cont(enum labels cont, NODE *val) {
#ifdef __ZTC__
    union { enum labels lll;
	    NODE *ppp;} cast;
#endif
    NODE *retval = cons(NIL, val);
#ifdef __ZTC__
    cast.lll = cont;
    retval->n_car = cast.ppp;
#else
    retval->n_car = (NODE *)cont;
#endif
    settype(retval, CONT);
    return retval;
}

NODE *loutput(NODE *arg) {
    if (NOT_THROWING) {
	stopping_flag = OUTPUT;
	output_node = car(arg);
    }
    return(UNBOUND);
}

NODE *lstop(NODE *args) {
    if (NOT_THROWING)
	stopping_flag = STOP;
    return(UNBOUND);
}

NODE *lthrow(NODE *arg) {
    if (NOT_THROWING) {
	if (compare_node(car(arg),Error,TRUE) == 0) {
	    if (cdr(arg) != NIL)
		err_logo(USER_ERR, cadr(arg));
	    else
		err_logo(USER_ERR, UNBOUND);
	} else {
	    stopping_flag = THROWING;
	    throw_node = car(arg);
	    if (cdr(arg) != NIL)
		output_node = cadr(arg);
	    else
		output_node = UNBOUND;
	}
    }
    return(UNBOUND);
}

NODE *lcatch(NODE *args) {
    return make_cont(catch_continuation, cons(car(args), lrun(cdr(args))));
}

int torf_arg(NODE *args) {
    NODE *arg = car(args);

    while (NOT_THROWING) {
	if (compare_node(arg, True, TRUE) == 0) return TRUE;
	if (compare_node(arg, False, TRUE) == 0) return FALSE;
	setcar(args, err_logo(BAD_DATA, arg));
	arg = car(args);
    }
    return -1;
}

NODE *lgoto(NODE *args) {
    return make_cont(goto_continuation, car(args));
}

NODE *ltag(NODE *args) {
    return UNBOUND;
}

NODE *lnot(NODE *args) {
    int arg = torf_arg(args);

    if (NOT_THROWING) {
	if (arg) return(False);
	else return(True);
    }
    return(UNBOUND);
}

NODE *land(NODE *args) {
    int arg;

    if (args == NIL) return(True);
    while (NOT_THROWING) {
	arg = torf_arg(args);
	if (arg == FALSE)
	    return(False);
	args = cdr(args);
	if (args == NIL) break;
    }
    if (NOT_THROWING) return(True);
    else return(UNBOUND);
}

NODE *lor(NODE *args) {
    int arg;

    if (args == NIL) return(False);
    while (NOT_THROWING) {
	arg = torf_arg(args);
	if (arg == TRUE)
	    return(True);
	args = cdr(args);
	if (args == NIL) break;
    }
    if (NOT_THROWING) return(False);
    else return(UNBOUND);
}

NODE *runnable_arg(NODE *args) {
    NODE *arg = car(args);

    if (!aggregate(arg)) {
	setcar(args, parser(arg, TRUE));
	arg = car(args);
    }
    while (!is_list(arg) && NOT_THROWING) {
	setcar(args, err_logo(BAD_DATA, arg));
	arg = car(args);
    }
    return(arg);
}

NODE *lif(NODE *args) {	/* macroized */
    NODE *yes;
    int pred;

    if (cddr(args) != NIL) return(lifelse(args));

    pred = torf_arg(args);
    yes = runnable_arg(cdr(args));
    if (NOT_THROWING) {
	if (pred) return(yes);
	return(NIL);
    }
    return(UNBOUND);
}

NODE *lifelse(NODE *args) {    /* macroized */
    NODE *yes, *no;
    int pred;

    pred = torf_arg(args);
    yes = runnable_arg(cdr(args));
    no = runnable_arg(cddr(args));
    if (NOT_THROWING) {
	if (pred) return(yes);
	return(no);
    }
    return(UNBOUND);
}

NODE *lrun(NODE *args) {    /* macroized */
    NODE *arg = runnable_arg(args);

    if (NOT_THROWING) return(arg);
    return(UNBOUND);
}

NODE *lrunresult(NODE *args) {
    return make_cont(runresult_continuation, lrun(args));
}

NODE *pos_int_arg(NODE *args) {
    NODE *arg = car(args), *val;
    FIXNUM i;
    FLONUM f;

    val = cnv_node_to_numnode(arg);
    while ((nodetype(val) != INT || getint(val) < 0) && NOT_THROWING) {
	if (nodetype(val) == FLOATT &&
		    fmod((f = getfloat(val)), 1.0) == 0.0 &&
		    f >= 0.0 && f < (FLONUM)MAXLOGOINT) {
#if HAVE_IRINT
	    i = irint(f);
#else
	    i = f;
#endif
	    val = make_intnode(i);
	    break;
	}
	setcar(args, err_logo(BAD_DATA, arg));
	arg = car(args);
	val = cnv_node_to_numnode(arg);
    }
    setcar(args,val);
    if (nodetype(val) == INT) return(val);
    return UNBOUND;
}

NODE *lrepeat(NODE *args) {
    NODE *cnt, *torpt, *retval = NIL;

    cnt = pos_int_arg(args);
    torpt = lrun(cdr(args));
    if (NOT_THROWING) {
	retval = make_cont(repeat_continuation, cons(cnt,torpt));
    }
    return(retval);
}

NODE *lrepcount(NODE *args) {
    return make_intnode(user_repcount);
}

NODE *lforever(NODE *args) {
    NODE *torpt = lrun(args);

    if (NOT_THROWING)
    return make_cont(repeat_continuation, cons(make_intnode(-1), torpt));
    return NIL;
}

NODE *ltest(NODE *args) {
    int arg = torf_arg(args);

    if (tailcall != 0) return UNBOUND;
    if (NOT_THROWING) {
	ift_iff_flag = arg;
	dont_fix_ift = 1;
    }
    return(UNBOUND);
}

NODE *liftrue(NODE *args) {
    if (ift_iff_flag < 0)
	return(err_logo(NO_TEST,NIL));
    else if (ift_iff_flag > 0)
	return(lrun(args));
    else
	return(NIL);
}

NODE *liffalse(NODE *args) {
    if (ift_iff_flag < 0)
	return(err_logo(NO_TEST,NIL));
    else if (ift_iff_flag == 0)
	return(lrun(args));
    else
	return(NIL);
}

void prepare_to_exit(BOOLEAN okay) {
#ifdef mac
    if (okay) {
	console_options.pause_atexit = 0;
	exit(0);
    }
#endif
#ifndef WIN32 /* sowings */
#ifdef ibm
    ltextscreen(NIL);
    ibm_plain_mode();
#ifdef __ZTC__
    zflush();
    controlc_close();
#endif
#endif
#endif /* !WIN32 */

#ifdef unix
#ifndef HAVE_UNISTD_H
    extern int getpid();
#endif
    char ef[30];

    charmode_off();
    sprintf(ef, "/tmp/logo%d", (int)getpid());
    unlink(ef);
#endif
}

NODE *lbye(NODE *args) {
    prepare_to_exit(TRUE);
    if (ufun != NIL || loadstream != stdin) exit(0);
#ifndef WIN32
    if (isatty(0) && isatty(1))
#endif
	lcleartext(NIL);
    ndprintf(stdout, "Thank you for using Logo.\n");
    ndprintf(stdout, "Have a nice day.\n");
#ifdef __ZTC__
    printf("\n");
#endif
    exit(0);
    return UNBOUND;	/* not reached, but makes compiler happy */
}

NODE *lwait(NODE *args) {
    NODE *num;
    unsigned int n;

    num = pos_int_arg(args);
    if (NOT_THROWING) {
#ifdef WIN32
      win32_update_text();
#else
	fflush(stdout); /* csls v. 1 p. 7 */
#endif
#if defined(__ZTC__) && !defined(WIN32) /* sowings */
	zflush();
#endif
	if (getint(num) > 0) {
#ifdef unix
#ifdef HAVE_USLEEP
	    n = (unsigned int)getint(num) * 16667;
	    usleep(n);
#else
	    n = (unsigned int)getint(num) / 60;
	    sleep(n);
#endif
#else	/* not unix */
#if defined(__ZTC__) && !defined(_MSC_VER)
	    usleep(getint(num) * 16667L);
#else	/* not DOS */
		if (!setjmp(iblk_buf)) {
			input_blocking++;
#ifndef _MSC_VER
			n = ((unsigned int)getint(num)+30) / 60;
#ifdef mac
			while (n > 1) {
				sleep(1);
				n -= 1;
				if (check_throwing) n = 0;
			}
#endif
			if (n > 0) sleep(n);
#else
			n = (unsigned int)getint(num);
			while (n > 60) {
				_sleep(1000);
				n -= 60;
				if (check_throwing) n = 0;
			}
			if (n > 0) _sleep(n*1000/60);
#endif
		}
		input_blocking = 0;
#endif	/* DOS */
#endif	/* Unix */
	}
    }
    return(UNBOUND);
}

NODE *lshell(NODE *args) {
#ifdef mac
    printf("Sorry, no shell on the Mac.\n");
    return(UNBOUND);
#else    
#ifdef ibm
    NODE *arg;
    char doscmd[200];
/*  union REGS r;     */

    arg = car(args);
    while (!is_list(arg) && NOT_THROWING) {
	setcar(args, err_logo(BAD_DATA, arg));
	arg = car(args);
    }
    if (arg == NIL) {
	ndprintf(stdout,"Type EXIT to return to Logo.\n");
	if (spawnlp(P_WAIT, "command", "command", NULL))
	    err_logo(FILE_ERROR,
	      make_static_strnode
		 ("Could not open shell (probably due to low memory)"));
    }
    else {
	print_stringlen = 199;
	print_stringptr = doscmd;
	ndprintf((FILE *)NULL,"%p",arg);
	*print_stringptr = '\0';
	if (system(doscmd))
	    err_logo(FILE_ERROR,
	      make_static_strnode
		 ("Could not open shell (probably due to low memory)"));
    }
/*
    r.h.ah = 0x3;
    r.h.al = 0;
    r.h.dh = 0; r.h.dl = 0;
    int86(0x21, &r, &r);
    x_coord = x_margin;
    y_coord = r.h.dh;
 */
#ifndef WIN32
    x_coord = x_margin;
    y_coord = y_max;
    ibm_gotoxy(x_coord, y_coord);
#else
    win32_repaint_screen();
#endif
    return(UNBOUND);
#else
    extern FILE *popen();
    char cmdbuf[300];
    FILE *strm;
    NODE *head = NIL, *tail = NIL, *this;
    BOOLEAN wordmode = FALSE;
    int len;

    if (cdr(args) != NIL) wordmode = TRUE;
    print_stringptr = cmdbuf;
    print_stringlen = 300;
    ndprintf((FILE *)NULL,"%p\n",car(args));
    *print_stringptr = '\0';
    strm = popen(cmdbuf,"r");
    fgets(cmdbuf,300,strm);
    while (!feof(strm)) {
	len = (int)strlen(cmdbuf);
	if (cmdbuf[len-1] == '\n')
	    cmdbuf[--len] = '\0';
	if (wordmode)
	    this = make_strnode(cmdbuf, (struct string_block *)NULL, len,
			STRING, strnzcpy);
	else
	    this = parser(make_static_strnode(cmdbuf), FALSE);
	if (head == NIL) {
	    tail = head = cons(this,NIL);
	} else {
	    setcdr(tail, cons(this,NIL));
	    tail = cdr(tail);
	}
	fgets(cmdbuf,300,strm);
    }
    pclose(strm);
    return(head);
#endif
#endif
}
