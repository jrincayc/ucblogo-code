/*
 *      error.c         logo error module                       dvb
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

#define WANT_EVAL_REGS 1
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef mac
#include <console.h>
#endif

NODE *throw_node = NIL;
NODE *err_mesg = NIL;
ERR_TYPES erract_errtype;

char *message_texts[MAX_MESSAGE+NUM_WORDS];

void err_print(char *buffer) {
    int save_flag = stopping_flag;
    int errtype;
    NODE *errargs, *oldfullp;
    FILE *fp;
    char *old_stringptr = print_stringptr;
    int old_stringlen = print_stringlen;

    if (err_mesg == NIL) return;

    if (buffer == NULL) {
	fp = stdout;
    } else {
	if (writestream == NULL) lsetwrite(the_generation); /* setwrite [] */
	print_stringptr = buffer;
	print_stringlen = 200;
	fp = NULL;
    }

    stopping_flag = RUN;
	oldfullp = valnode__caseobj(Fullprintp);
    setvalnode__caseobj(Fullprintp, TrueName());

    errtype = getint(car(err_mesg));
    errargs = cadr(err_mesg);

    force_printdepth = 5;
    force_printwidth = 80;
    if (errargs == NIL)
	ndprintf(fp, message_texts[errtype]);
    else if (cdr(errargs) == NIL)
	ndprintf(fp, message_texts[errtype], car(errargs));
    else
	ndprintf(fp, message_texts[errtype], car(errargs), cadr(errargs));

    if (car(cddr(err_mesg)) != NIL && buffer == NULL) {
	ndprintf(fp, message_texts[ERROR_IN], car(cddr(err_mesg)),
						  cadr(cddr(err_mesg)));
    }
    err_mesg = NIL;
    if (buffer == NULL)
	new_line(fp);
    else {
	*print_stringptr = '\0';
	print_stringptr = old_stringptr;
	print_stringlen = old_stringlen;
    }
    setvalnode__caseobj(Fullprintp, oldfullp);
    stopping_flag = save_flag;
}

NODE *err_logo(ERR_TYPES error_type, NODE *error_desc) {
    BOOLEAN recoverable = FALSE, warning = FALSE, uplevel = FALSE;
    NODE *err_act, *val = UNBOUND;

    switch(error_type) {
	case FATAL:
	    prepare_to_exit(FALSE);
	    ndprintf(stdout,"%t\n",message_texts[FATAL]);
	    exit(1);
	case OUT_OF_MEM_UNREC:
	    prepare_to_exit(FALSE);
	    ndprintf(stdout,"%t\n",message_texts[OUT_OF_MEM_UNREC]);
	    exit(1);
	case OUT_OF_MEM:
	    use_reserve_tank();
	    break;
	case DIDNT_OUTPUT:
	    if (didnt_output_name != NIL) {
		last_call = didnt_output_name;
	    }
	    if (error_desc == NIL) {
		error_desc = car(didnt_get_output);
		ufun = cadr(didnt_get_output);
		this_line = cadr(cdr(didnt_get_output));
	    }
	    err_mesg = cons_list(0, last_call, error_desc, END_OF_LIST);
	    recoverable = TRUE;
	    break;
	case NOT_ENOUGH:
	    if (error_desc == NIL)
		err_mesg = cons_list(0, fun, END_OF_LIST);
	    else
		err_mesg = cons_list(0, error_desc, END_OF_LIST);
	    break;
	case BAD_DATA:
	    recoverable = TRUE;
	case BAD_DATA_UNREC:
	    err_mesg = cons_list(0, fun, error_desc, END_OF_LIST);
	    break;
	case DK_WHAT_UP:
	    uplevel = TRUE;
	case DK_WHAT:
	    err_mesg = cons_list(0, error_desc, END_OF_LIST);
	    break;
	case DK_HOW:
	case APPLY_BAD_DATA:
	case NO_VALUE:
	    recoverable = TRUE;
	case DK_HOW_UNREC:
	case NO_CATCH_TAG:
	case ALREADY_DEFINED:
	case FILE_ERROR:
	case IS_PRIM:
	case AT_TOPLEVEL:
	case ERR_MACRO:
	case RUNNABLE_ARG:
	case TOO_MUCH:
	case DEEPEND:
	case BAD_DEFAULT:
	case CANT_OPEN_ERROR:
	case ALREADY_OPEN_ERROR:
	case NOT_OPEN_ERROR:
	    err_mesg = cons_list(0, error_desc, END_OF_LIST);
	    break;
	case SHADOW_WARN:
	    err_mesg = cons_list(0, error_desc, END_OF_LIST);
	case IF_WARNING:
	    warning = TRUE;
	    break;
	case MISSING_SPACE:
	    warning = TRUE;
	    err_mesg = error_desc;
	    break;
	case USER_ERR_MESSAGE:
	    uplevel = TRUE;
	    err_mesg = cons_list(0, error_desc, END_OF_LIST);
	    break;
	case NO_TEST:
	    err_mesg = cons_list(0, fun, END_OF_LIST);
	    break;
	default:
	    err_mesg = NIL;
    }
    didnt_output_name = NIL;
    if (uplevel && ufun != NIL) {
	ufun = last_ufun;
	this_line = last_line;
    }
    if (ufun != NIL)
	err_mesg = cons_list(0, err_mesg, ufun, this_line, END_OF_LIST);
    else
	err_mesg = cons_list(0, err_mesg, NIL, NIL, END_OF_LIST);
    err_mesg = cons(make_intnode((FIXNUM)error_type), err_mesg);
    if (warning) {
	err_print(NULL);
	return(UNBOUND);
    }
    err_act = valnode__caseobj(Erract);
    if (err_act != NIL && err_act != UNDEFINED) {
	if (error_type != erract_errtype) {

	    erract_errtype = error_type;
	    setvalnode__caseobj(Erract, NIL);
	    val = err_eval_driver(err_act, recoverable);
	    setvalnode__caseobj(Erract, err_act);
	    if (recoverable == TRUE && val != UNBOUND) {
		return(val);
	    } else if (recoverable == FALSE && val != UNBOUND) {
		ndprintf(stdout, message_texts[DK_WHAT], val);
		ndprintf(stdout, "\n");
		val = UNBOUND;
		throw_node = theName(Name_toplevel);
	    } else {
		/* if (err_mesg != NIL) */  {	/* is this ever wrong? */
		    throw_node = theName(Name_error);
		    stopping_flag = THROWING;
		    output_node = UNBOUND;
		}
		return(UNBOUND);
	    }
	} else {
	    ndprintf(stdout,"%t\n", message_texts[ERRACT_LOOP]);
	    throw_node = theName(Name_toplevel);
	}
    } else {	/* no erract */
	throw_node = theName(Name_error);
    }
    stopping_flag = THROWING;
    output_node = UNBOUND;
    return(val);
}

NODE *lerror(NODE *args) {
    NODE *val, *save_err = err_mesg;
    char buffer[200];

    if (err_mesg == NIL) return NIL;
    err_print(buffer);
    err_mesg = save_err;
    setcar(cdr(err_mesg), make_strnode(buffer, (struct string_block *)NULL,
				       strlen(buffer), STRING, strnzcpy));
    val = err_mesg;
    err_mesg = NIL;
    return(val);
}

/*
#ifndef HAVE_MEMCPY
void memcpy(char *to, char *from, size_t len) {
    while (--len >= 0)
	*to++ = *from++;
}
#endif
*/

NODE *lpause(NODE *args) {
    NODE *elist = NIL, *val = UNBOUND, *uname = NIL;
    int sav_input_blocking;
#ifndef TIOCSTI
    jmp_buf sav_iblk;
#endif

    if (err_mesg != NIL) err_print(NULL);
    ndprintf(stdout, "%t\n", message_texts[PAUS_ING]);
    if (inside_evaluator) {
	uname = ufun;
	ufun = NIL;
#ifndef TIOCSTI
	memcpy((char *)(&sav_iblk), (char *)(&iblk_buf), sizeof(jmp_buf));
#endif
	sav_input_blocking = input_blocking;
	input_blocking = 0;
#ifdef mac
	csetmode(C_ECHO, stdin);
	fflush(stdin);
#endif
	while (RUNNING) {
	    if (uname != NIL) print_node(stdout, uname);
	    elist = reader(stdin, "? ");
	    if (NOT_THROWING) elist = parser(elist, TRUE);
	    else elist = NIL;
#ifndef WIN32
	    if (feof(stdin) && !isatty(0)) lbye(NIL);
#endif
#ifdef __RZTC__
	    if (feof(stdin)) rewind(stdin);
#endif
	    if (elist != NIL) eval_driver(elist);
	    if (stopping_flag == THROWING) {
		if (compare_node(throw_node, Pause, TRUE) == 0) {
		    val = output_node;
		    output_node = UNBOUND;
		    stopping_flag = RUN;
#ifndef TIOCSTI
		    memcpy((char *)(&iblk_buf), (char *)(&sav_iblk),
			   sizeof(jmp_buf));
#endif
		    input_blocking = sav_input_blocking;
			if (uname != NIL) {
				ufun = uname;
			}
		    return(val);
		} else if (isName(throw_node, Name_error)) {
		    err_print(NULL);
		    stopping_flag = RUN;
		}
	    }
	}
#ifndef TIOCSTI
	memcpy((char *)(&iblk_buf), (char *)(&sav_iblk), sizeof(jmp_buf));
#endif
	input_blocking = sav_input_blocking;
	unblock_input();
	if (uname != NIL) {
		ufun = uname;
	}
/*  } else {
	stopping_flag = THROWING;
	throw_node = theName(Name_toplevel);
 */
    } else {
	ndprintf(stdout, "? ");
#ifdef HAVE_WX
	flushFile(stdout);
#else
	fflush(stdout);
#endif
}
    return(val);
}

NODE *lcontinue(NODE *args) {
    stopping_flag = THROWING;
    throw_node = Pause;
    if (args != NIL)
	output_node = car(args);
    else
	output_node = UNBOUND;
    return(UNBOUND);
}
