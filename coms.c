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

#ifdef HAVE_WX
#define fgets wx_fgets
extern int check_wx_stop(int force_yield);
#endif

#define WANT_EVAL_REGS 1
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
#include <Events.h>
#endif
#ifdef __RZTC__
#include <time.h>
#include <controlc.h>
#include <dos.h>
#include <msmouse.h>
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

NODE *make_cont(enum labels cont, NODE *val) {
#ifdef __RZTC__
    union { enum labels lll;
	    NODE *ppp;} cast;
#endif
    NODE *retval = cons(NIL, val);
#ifdef __RZTC__
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
	if (ufun == NIL) {
	    err_logo(AT_TOPLEVEL, fun);
	} else if (val_status & OUTPUT_TAIL 
		 /* This happens if OP seen when val_status & STOP_OK */
		    || !(val_status & OUTPUT_OK)) {
			/* This happens on OP OP 3 */
	    if (didnt_output_name == NIL) didnt_output_name = fun;
	    if (didnt_get_output == UNBOUND) {
		didnt_get_output = cons_list(0,theName(Name_output),
					     ufun,this_line,END_OF_LIST);
		/* Not quite right; could be .maybeoutput */
		didnt_output_name = fun;
	    }
	    err_logo(DIDNT_OUTPUT, NIL);
	} else {
	    stopping_flag = OUTPUT;
	    output_unode = current_unode;
	    output_node = car(arg);
	}
    }
    return(UNBOUND);
}

NODE *lstop(NODE *args) {
  if (NOT_THROWING) {
	if (ufun == NIL)
	    err_logo(AT_TOPLEVEL, theName(Name_stop));
	else if (val_status & OUTPUT_TAIL 
		 || !(val_status & STOP_OK)) {
	    didnt_output_name = fun;
	    err_logo(DIDNT_OUTPUT, NIL);
	} else {
	    stopping_flag = STOP;
	    output_unode = current_unode;
	}
  }
    return(UNBOUND);
}

NODE *lthrow(NODE *arg) {
    if (NOT_THROWING) {
	if (isName(car(arg), Name_error)) {
	    if (cdr(arg) != NIL)
		err_logo(USER_ERR_MESSAGE, cadr(arg));
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

extern NODE *evaluator(NODE *list, enum labels where);

int torf_arg(NODE *args) {
    NODE *arg = car(args);

    while (NOT_THROWING) {
	if (is_list(arg)) {	/* accept a list and run it */
	    val_status = VALUE_OK;
	    arg = evaluator(arg, begin_seq);
	}
	if (isName(arg, Name_true)) return TRUE;
	if (isName(arg, Name_false)) return FALSE;
	if (NOT_THROWING) arg = err_logo(BAD_DATA, arg);
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
	if (arg) return(FalseName());
	else return(TrueName());
    }
    return(UNBOUND);
}

NODE *land(NODE *args) {
    int arg;

    if (args == NIL) return(TrueName());
    while (NOT_THROWING) {
	arg = torf_arg(args);
	if (arg == FALSE)
	    return(FalseName());
	args = cdr(args);
	if (args == NIL) break;
    }
    if (NOT_THROWING) return(TrueName());
    else return(UNBOUND);
}

NODE *lor(NODE *args) {
    int arg;

    if (args == NIL) return(FalseName());
    while (NOT_THROWING) {
	arg = torf_arg(args);
	if (arg == TRUE)
	    return(TrueName());
	args = cdr(args);
	if (args == NIL) break;
    }
    if (NOT_THROWING) return(FalseName());
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
	    i = (FIXNUM)f;
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

    if (ufun != NIL && tailcall != 0) return UNBOUND;
    if (NOT_THROWING) {
	dont_fix_ift = arg+1;
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

#ifdef HAVE_WX
	extern void doClose();
	doClose();
	wxLogoExit (0);
#endif

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
#ifdef __RZTC__
    msm_term();
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
    ndprintf(stdout, "%t\n", message_texts[THANK_YOU]);
    ndprintf(stdout, "%t\n", message_texts[NICE_DAY]);
#ifdef __RZTC__
    printf("\n");
#endif
    
#ifdef HAVE_WX
    wx_leave_mainloop++;
    return UNBOUND;
#else    
    exit(0);
    return UNBOUND;	/* not reached, but makes compiler happy */
#endif
}

NODE *lwait(NODE *args) {
    NODE *num;
    unsigned int n;
#if defined(unix) && HAVE_USLEEP
	unsigned int seconds, microseconds;
#endif
#ifdef mac
    long target;
    extern void ProcessEvent(void);
#endif

    num = pos_int_arg(args);
    if (NOT_THROWING) {
      /*#ifdef HAVE_WX
      n = (unsigned int)getint(num) * 10; // milliseconds
      wxLogoSleep(n);
      return(UNBOUND);
      #endif*/
#ifndef HAVE_WX
#ifdef WIN32
      win32_update_text();
#else
      fflush(stdout); /* csls v. 1 p. 7 */
#endif
#else
      //doesn't seem to work in WX. now done in wxLogoSleep
      //fflush(stdout); /* csls v. 1 p. 7 */
#endif

#if defined(__RZTC__)
	zflush();
#endif
	fix_turtle_shownness();

#ifdef HAVE_WX
	n = (unsigned int)getint(num) * 100 / 6; // milliseconds
	wxLogoSleep(n);
	//check_throwing;
	return(UNBOUND); 
#endif

	if (getint(num) > 0) {
#ifdef unix
#ifdef HAVE_USLEEP
	    n = (unsigned int)getint(num);
		
		if (seconds = n / 60)
			sleep(seconds);
			
		if (microseconds = (n % 60) * 16667)
			usleep(microseconds);
#else
	    n = (unsigned int)getint(num) / 60;
	    sleep(n);
#endif
#elif defined(__RZTC__)
	    usleep(getint(num) * 16667L);
#elif defined(mac)
	    target = getint(num)+TickCount();
	    while (TickCount() < target) {
		if (check_throwing) break;
		ProcessEvent();
	    }
#elif defined(_MSC_VER)
	    n = (unsigned int)getint(num);
	    while (n > 60) {
		_sleep(1000);
		n -= 60;
		if (check_throwing) n = 0;
	    }
	    if (n > 0) _sleep(n*1000/60);
#else	/* unreachable, I think */
	    if (!setjmp(iblk_buf)) {
		input_blocking++;
		n = ((unsigned int)getint(num)+30) / 60;
		if (n > 0) sleep(n);
	    }
	    input_blocking = 0;
#endif
	}
    }
    return(UNBOUND);
}

NODE *lshell(NODE *args) {
#if defined(mac)
    printf("%s\n", message_texts[NOSHELL_MAC]);
    return(UNBOUND);
#else    
#ifdef ibm
    NODE *arg;
    char doscmd[200];
/*  union REGS r;     */
    char *old_stringptr = print_stringptr;
    int old_stringlen = print_stringlen;

    arg = car(args);
    while (!is_list(arg) && NOT_THROWING) {
	setcar(args, err_logo(BAD_DATA, arg));
	arg = car(args);
    }
    if (arg == NIL) {
	ndprintf(stdout, "%t\n", message_texts[TYPE_EXIT]);
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
	if (system(doscmd) < 0)
	    err_logo(FILE_ERROR,
	      make_static_strnode
		 ("Could not open shell (probably due to low memory)"));
	print_stringptr = old_stringptr;
	print_stringlen = old_stringlen;
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
    char *old_stringptr = print_stringptr;
    int old_stringlen = print_stringlen;

    if (cdr(args) != NIL) wordmode = TRUE;
    print_stringptr = cmdbuf;
    print_stringlen = 300;
    ndprintf((FILE *)NULL,"%p\n",car(args));
    *print_stringptr = '\0';
#ifdef __WXMSW__
	strm = _popen(cmdbuf,"r");
#else
    strm = popen(cmdbuf,"r");
#endif
    print_stringptr = old_stringptr;
    print_stringlen = old_stringlen;
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
#ifdef __WXMSW__
	_pclose(strm);
#else
    pclose(strm);
#endif
    return(head);
#endif
#endif
}
