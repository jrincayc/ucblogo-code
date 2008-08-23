/*
 *      main.c          logo main procedure module              dvb
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
#include <process.h>  /* needed? */
#endif

#include "logo.h"
#include "globals.h"

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

#ifdef __RZTC__
#include <signal.h>
#define SIGQUIT SIGTERM
#include <controlc.h>
#endif

#ifndef TIOCSTI
#include <setjmp.h>
jmp_buf iblk_buf;
#endif
 
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef mac
#include <console.h>
#endif

NODE *current_line = NIL;
NODE **bottom_stack; /*GC*/
NODE *command_line = NIL;   /* 6.0 command line args after files */

int stop_quietly_flag=0;

void unblock_input(void) {
    if (input_blocking) {
	input_blocking = 0;
#ifdef mac
	csetmode(C_ECHO, stdin);
	fflush(stdin);
#endif
#ifdef TIOCSTI
	ioctl(0,TIOCSTI,"\n");
#else
	longjmp(iblk_buf,1);
#endif
    }
}

extern int in_eval_save;

#ifdef SIG_TAKES_ARG
#define sig_arg 0
RETSIGTYPE logo_stop(int sig)
#else
#define sig_arg 
RETSIGTYPE logo_stop()
#endif
{
    if (inside_gc || in_eval_save) {
	int_during_gc = 1;
    } else {
	charmode_off();
	to_pending = 0;
        if (!stop_quietly_flag)
            err_logo(STOP_ERROR,NIL);
        stop_quietly_flag = 0;
#ifdef __RZTC__
	if (!input_blocking)
#endif
	  signal(SIGINT, logo_stop);
	unblock_input();
    }
    SIGRET
}

#ifdef SIG_TAKES_ARG
#define sig_arg 0
RETSIGTYPE logo_pause(int sig)
#else
#define sig_arg 
RETSIGTYPE logo_pause()
#endif
{
    if (inside_gc || in_eval_save) {
	int_during_gc = 2;
    } else {
	charmode_off();
	to_pending = 0;
#ifdef bsd
	sigsetmask(0);
#else
#if !defined(mac) && !defined(_MSC_VER)
	signal(SIGQUIT, logo_pause);
#endif
#endif
	lpause(NIL);
    }
    SIGRET
}

#ifdef SIG_TAKES_ARG
#define sig_arg 0
RETSIGTYPE mouse_down(int sig)
#else
#define sig_arg 
RETSIGTYPE mouse_down()
#endif
{
    NODE *line;

    if (inside_gc || in_eval_save) {
	if (int_during_gc == 0) int_during_gc = 3;
    } else {
	line = valnode__caseobj(Buttonact);
	if (line != UNBOUND && line != NIL) {
	    if (inside_evaluator) {
		eval_buttonact = line;
	    } else {
		eval_driver(line);
		fix_turtle_shownness();
	    }
	}
    }
    SIGRET
}

int keyact_set() {
    NODE *line;

    line = valnode__caseobj(Keyact);
    return (line != UNBOUND && line != NIL);
}

void do_keyact(int);

#ifdef SIG_TAKES_ARG
#define sig_arg 0
RETSIGTYPE delayed_keyact(int sig)
#else
#define sig_arg 
RETSIGTYPE delayed_keyact()
#endif
{
    do_keyact(readchar_lookahead_buf);
    SIGRET
}

void do_keyact(int ch) {
    NODE *line;

    readchar_lookahead_buf = ch;
    if (inside_gc || in_eval_save) {
	if (int_during_gc == 0) int_during_gc = 4;
    } else {
	line = valnode__caseobj(Keyact);
	if (line != UNBOUND && line != NIL) {
	    if (inside_evaluator) {
		eval_buttonact = line;
	    } else {
		eval_driver(line);
		fix_turtle_shownness();
	    }
	}
    }
}


RETSIGTYPE (*intfuns[])() = {0, logo_stop, logo_pause, mouse_down,
			     delayed_keyact};

void delayed_int() {
#ifdef SIG_TAKES_ARG
    (void)(*intfuns[int_during_gc])(0);
#else
    (void)(*intfuns[int_during_gc])();
#endif
}

#if defined(__RZTC__) && !defined(WIN32) /* sowings */
void _far _cdecl do_ctrl_c(void) {
    ctrl_c_count++;
}
#endif

#ifdef HAVE_WX
int wx_leave_mainloop = 0;
int  start (int argc,char ** argv) {
#else
int main(int argc, char *argv[]) {
#endif
    NODE *exec_list = NIL;
    NODE *cl_tail = NIL;
    int argc2;
    char **argv2;

#ifdef SYMANTEC_C
    extern void (*openproc)(void);
    extern void __open_std(void);
    openproc = &__open_std;
#endif

#ifdef mac
    init_mac_memory();
#endif

    bottom_stack = &exec_list; /*GC*/

#ifndef HAVE_WX
#ifdef x_window
    x_window_init(argc, argv);
#endif
#endif
    (void)addseg();
    term_init();
    init();

    math_init();

#ifdef ibm
    signal(SIGINT, SIG_IGN);
#if defined(__RZTC__) && !defined(WIN32) /* sowings */
    _controlc_handler = do_ctrl_c;
    controlc_open();
#endif
#else /* !ibm */
    signal(SIGINT, logo_stop);
#endif /* ibm */
#ifdef mac
    signal(SIGQUIT, SIG_IGN);
#else /* !mac */
	//signal(SIGQUIT, logo_pause);
#endif
    /* SIGQUITs never happen on the IBM */

    if (argc < 2) {
#ifndef WIN32
      if (1 || isatty(1))   // fix this.  for interactive from menu bar.
#endif
      {
#ifdef HAVE_WX
	extern char *SVN;
#endif
	char version[20];
	lcleartext(NIL);
#ifdef HAVE_WX
	strcpy(version,"6.0");
	strcat(version,SVN);
#else
	strcpy(version,"5.6");
#endif
	ndprintf(stdout, message_texts[WELCOME_TO], version);
	new_line(stdout);
      }
    }

#ifdef HAVE_WX
    setvalnode__caseobj(LogoVersion, make_floatnode(6.0));
#else
    setvalnode__caseobj(LogoVersion, make_floatnode(5.6));
#endif
    setflag__caseobj(LogoVersion, VAL_BURIED);

    argv2 = argv; argc2 = argc;

    if (!strcmp(*argv+strlen(*argv)-4, "logo")) {
	argv++;
	while (--argc > 0 && strcmp(*argv, "-") && NOT_THROWING) {
	    argv++;
	}
    }

    argv++;
    while (--argc > 0) {
	if (command_line == NIL) 
	    cl_tail = command_line = cons(make_static_strnode(*argv++), NIL);
	else {
	    setcdr(cl_tail, cons(make_static_strnode(*argv++), NIL));
	    cl_tail = cdr(cl_tail);
	}
    }

    setvalnode__caseobj(CommandLine, command_line);

    silent_load(Startuplg, logolib);
    silent_load(Startup, NULL); /* load startup.lg */
    if (!strcmp(*argv2+strlen(*argv2)-4, "logo")) {
	argv2++;
	while (--argc2 > 0 && strcmp(*argv2, "-") && NOT_THROWING) {
	    silent_load(NIL,*argv2++);
	    }
    }

    for (;;) {
	if (NOT_THROWING) {
	    check_reserve_tank();
	    current_line = reader(stdin,"? ");
#ifdef __RZTC__
		(void)feof(stdin);
		if (!in_graphics_mode)
		    printf(" \b");
		fflush(stdout);
#endif

#ifndef WIN32
	    if (feof(stdin) && !isatty(0)) lbye(NIL);
#endif

#ifdef __RZTC__
	    if (feof(stdin)) clearerr(stdin);
#endif
	    if (NOT_THROWING) {
		exec_list = parser(current_line, TRUE);
		if (exec_list != NIL) eval_driver(exec_list);
	    }
	}
#ifdef HAVE_WX
	if (wx_leave_mainloop) {
	  break;
	}
#endif	
	if (stopping_flag == THROWING) {
	    if (isName(throw_node, Name_error)) {
		err_print(NULL);
	    } else if (isName(throw_node, Name_system))
		break;
	    else if (!isName(throw_node, Name_toplevel)) {
		err_logo(NO_CATCH_TAG, throw_node);
		err_print(NULL);
	    }
	    stopping_flag = RUN;
	}
	if (stopping_flag == STOP || stopping_flag == OUTPUT) {
	/*    ndprintf(stdout, "%t\n", message_texts[CANT_STOP]);   */
	    stopping_flag = RUN;
	}
    }
    //prepare_to_exit(TRUE);
    exit(0);
    return 0;
}
