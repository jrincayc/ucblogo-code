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

#include "logo.h"
#include "globals.h"

#ifdef HAVE_TERMIO_H
#include <termio.h>
#else
#ifdef HAVE_SGTTY_H
#include <sgtty.h>
#endif
#endif

#ifdef __ZTC__
#include <signal.h>
#define SIGQUIT SIGTERM
#include <controlc.h>
#endif

#ifndef TIOCSTI
#include <setjmp.h>
jmp_buf iblk_buf;
#endif

#ifdef mac
#include <console.h>
#endif

NODE *current_line = NIL;

unblock_input() {
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

#ifdef __ZTC__
void logo_stop(int sig)
#else
void logo_stop()
#endif
{
    to_pending = 0;
#if 1   /* was #ifndef unix */
    err_logo(STOP_ERROR,NIL);
#else
    if (ufun != NIL) {
	err_logo(STOP_ERROR,NIL);
    } else {
	new_line(stdout);
    }
#endif
#ifndef bsd
    signal(SIGINT, logo_stop);
#endif
    unblock_input();
}

#ifdef __ZTC__
void logo_pause(int sig)
#else
void logo_pause()
#endif
{
    to_pending = 0;
#ifdef bsd
    sigsetmask(0);
#else
#ifndef mac
    signal(SIGQUIT, logo_pause);
#endif
#endif
#if 1 /* was #ifndef unix */
    lpause();
#else
    if (ufun != NIL) {
	lpause();
    } else {
	new_line(stdout);
	unblock_input();
    }
#endif
}

#ifdef __ZTC__
extern volatile int ctrl_c_count;

void _far _cdecl do_ctrl_c(void) {
    ctrl_c_count++;
}
#endif

main(int argc, char *argv[])
{
    NODE *exec_list = NIL;
#ifdef MEM_DEBUG
    extern long int mem_allocated, mem_freed;
#endif

#ifdef x_window
    x_window_init(argc, argv);
#endif
#ifdef mac
    init_mac_memory();
#endif
    term_init();
    if (argc < 2) {
	if (isatty(1)) lcleartext();
	printf("Welcome to Berkeley Logo version 3.3");
	new_line(stdout);
    }
    init();
#ifdef ibm
    signal(SIGINT, SIG_IGN);
#ifdef __ZTC__
    _controlc_handler = do_ctrl_c;
    controlc_open();
#endif
#else
    signal(SIGINT, logo_stop);
#endif
#ifdef mac
    signal(SIGQUIT, SIG_IGN);
#else
    signal(SIGQUIT, logo_pause);
#endif
    /* SIGQUITs never happen on the IBM */
    argv++;
    while (--argc > 0 && NOT_THROWING) {
	silent_load(NIL,*argv++);
    }
#ifdef MEM_DEBUG
    clear_alloced_records();
#endif
    for (;;) {
	if (NOT_THROWING) {
#ifdef MEM_DEBUG
	    fprintf(stderr, "New nodes allocated:\n");
	    print_alloced_pointers();
	    printf("alloc=%d, freed=%d, used=%d\n",
		   mem_allocated, mem_freed, mem_allocated-mem_freed);
#endif
	    check_reserve_tank();
	    current_line = reref(current_line, reader(stdin,"? "));
#ifdef __ZTC__
		(void)feof(stdin);
		if (!in_graphics_mode)
		    printf(" \b");
		fflush(stdout);
#endif
	    if (!isatty(0) && feof(stdin)) lbye();
#ifdef __ZTC__
	    if (feof(stdin)) clearerr(stdin);
#endif
	    if (NOT_THROWING) {
		exec_list = parser(current_line, TRUE);
		val_status = 0;
		if (exec_list != NIL) eval_driver(exec_list);
	    }
	}
	if (stopping_flag == THROWING) {
	    if (compare_node(throw_node, Error, TRUE) == 0) {
		err_print();
	    } else if (compare_node(throw_node, System, TRUE) == 0)
		break;
	    else if (compare_node(throw_node, Toplevel, TRUE) != 0) {
		err_logo(NO_CATCH_TAG, throw_node);
		err_print();
	    }
	    stopping_flag = RUN;
	}
	if (stopping_flag == STOP || stopping_flag == OUTPUT) {
	    print_node(stdout, make_static_strnode(
		"You must be in a procedure to use OUTPUT or STOP.\n"));
	    stopping_flag = RUN;
	}
    }
    prepare_to_exit(TRUE);
}
