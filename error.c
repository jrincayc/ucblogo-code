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

#include "logo.h"
#include "globals.h"

#ifdef HAVE_TERMIO_H
#include <termio.h>
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

void err_print(void) {
    int save_flag = stopping_flag;
    
    if (!err_mesg) return;

    stopping_flag = RUN;
    print_backslashes = TRUE;

    real_print_help(stdout, cadr(err_mesg), 5, 80);
    if (car(cddr(err_mesg)) != NIL) {
	ndprintf(stdout, "  in %s\n%s",car(cddr(err_mesg)),
		 cadr(cddr(err_mesg)));
    }
    err_mesg = NIL;
    new_line(stdout);

    print_backslashes = FALSE;
    stopping_flag = save_flag;
}

NODE *err_logo(ERR_TYPES error_type, NODE *error_desc) {
    BOOLEAN recoverable = FALSE, warning = FALSE, uplevel = FALSE;
    NODE *err_act, *val = UNBOUND;

    switch(error_type) {
	case FATAL:
	    prepare_to_exit(FALSE);
	    ndprintf(stdout,"Logo: Fatal Internal Error.\n");
	    exit(1);
	case OUT_OF_MEM_UNREC:
	    prepare_to_exit(FALSE);
	    ndprintf(stdout,"Logo: Out of Memory.\n");
	    exit(1);
	case OUT_OF_MEM:
	    use_reserve_tank();
	    err_mesg = cons_list(0, make_static_strnode("out of space"),
				 END_OF_LIST);
	    break;
	case STACK_OVERFLOW:
	    err_mesg = cons_list(0, make_static_strnode("stack overflow"),
				 END_OF_LIST);
	    break;
	case TURTLE_OUT_OF_BOUNDS:
	    err_mesg = cons_list(0,make_static_strnode("turtle out of bounds"),
				 END_OF_LIST);
	    break;
	case BAD_GRAPH_INIT:
	    err_mesg = cons_list(0,
			  make_static_strnode("couldn't initialize graphics"),
			  END_OF_LIST);
	    break;
	case BAD_DATA_UNREC:
	    err_mesg = cons_list(0, fun,
		make_static_strnode("doesn\'t like"), error_desc,
		make_static_strnode("as input"), END_OF_LIST);
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
	    err_mesg = cons_list(0, last_call,
				 make_static_strnode("didn\'t output to"),
				 error_desc, END_OF_LIST);
	    recoverable = TRUE;
	    break;
	case NOT_ENOUGH:
	    if (error_desc == NIL)
		err_mesg = cons_list(0,make_static_strnode("not enough inputs to"),
				 fun, END_OF_LIST);
	    else
		err_mesg = cons_list(0,make_static_strnode("not enough inputs to"),
				 error_desc, END_OF_LIST);
	    break;
	case BAD_DATA:
	    err_mesg = cons_list(0, fun,
		make_static_strnode("doesn\'t like"), error_desc,
		make_static_strnode("as input"), END_OF_LIST);
	    recoverable = TRUE;
	    break;
	case APPLY_BAD_DATA:
	    err_mesg = cons_list(0, make_static_strnode("APPLY doesn\'t like"),
				 error_desc,
				 make_static_strnode("as input"), END_OF_LIST);
	    recoverable = TRUE;
	    break;
	case TOO_MUCH:
	    err_mesg = cons_list(0,
				 make_static_strnode("too much inside ()\'s"),
				 END_OF_LIST);
	    break;
	case DK_WHAT_UP:
	    uplevel = TRUE;
	case DK_WHAT:
	    err_mesg = cons_list(0,
		make_static_strnode("You don\'t say what to do with"),
		error_desc, END_OF_LIST);
	    break;
	case PAREN_MISMATCH:
	    err_mesg = cons_list(0, make_static_strnode("too many (\'s"),
				 END_OF_LIST);
	    break;
	case NO_VALUE:
	    err_mesg = cons_list(0, error_desc,
				 make_static_strnode("has no value"),
				 END_OF_LIST);
	    recoverable = TRUE;
	    break;
	case UNEXPECTED_PAREN:
	    err_mesg = cons_list(0, make_static_strnode("unexpected \')\'"),
				 END_OF_LIST);
	    break;
	case UNEXPECTED_BRACKET:
	    err_mesg = cons_list(0, make_static_strnode("unexpected \']\'"),
				 END_OF_LIST);
	    break;
	case UNEXPECTED_BRACE:
	    err_mesg = cons_list(0, make_static_strnode("unexpected \'}\'"),
				 END_OF_LIST);
	    break;
	case DK_HOW:
	    recoverable = TRUE;
	case DK_HOW_UNREC:
	    err_mesg = cons_list(0,
				 make_static_strnode("I don\'t know how  to"),
				 error_desc, END_OF_LIST);
	    break;
	case NO_CATCH_TAG:
	    err_mesg = cons_list(0,
			make_static_strnode("Can't find catch tag for"),
			error_desc, END_OF_LIST);
	    break;
	case ALREADY_DEFINED:
	    err_mesg = cons_list(0, error_desc,
				 make_static_strnode("is already defined"),
				 END_OF_LIST);
	    break;
	case STOP_ERROR:
	    err_mesg = cons_list(0, make_static_strnode("Stopping..."),
				 END_OF_LIST);
	    break;
	case ALREADY_DRIBBLING:
	    err_mesg = cons_list(0, make_static_strnode("Already dribbling"),
				 END_OF_LIST);
	    break;
	case FILE_ERROR:
	    err_mesg = cons_list(0, make_static_strnode("File system error:"),
				 error_desc, END_OF_LIST);
	    break;
	case IF_WARNING:
	    err_mesg = cons_list(0,
		make_static_strnode("Assuming you mean IFELSE, not IF"),
		END_OF_LIST);
	    warning = TRUE;
	    break;
	case SHADOW_WARN:
	    err_mesg = cons_list(0, error_desc,
		make_static_strnode("shadowed by local in procedure call"),
		END_OF_LIST);
	    warning = TRUE;
	    break;
	case USER_ERR:
	    if (error_desc == UNBOUND)
		err_mesg = cons_list(0, make_static_strnode("Throw \"Error"),
				     END_OF_LIST);
	    else {
		uplevel = TRUE;
		if (is_list(error_desc))
		    err_mesg = error_desc;
		else
		    err_mesg = cons_list(0, error_desc, END_OF_LIST);
	    }
	    break;
	case IS_PRIM:
	    err_mesg = cons_list(0, error_desc,
				 make_static_strnode("is a primitive"),
				 END_OF_LIST);
	    break;
	case NOT_INSIDE:
	    err_mesg = cons_list(0,
		make_static_strnode("Can't use TO inside a procedure"),
		END_OF_LIST);
	    break;
	case AT_TOPLEVEL:
	    err_mesg = cons_list(0, make_static_strnode("Can only use"),
				 error_desc,
				 make_static_strnode("inside a procedure"),
				 END_OF_LIST);
	    break;
	case NO_TEST:
	    err_mesg = cons_list(0, fun, make_static_strnode("without TEST"),
				 END_OF_LIST);
	    break;
	case ERR_MACRO:
	    err_mesg = cons_list(0, make_static_strnode("Macro returned"),
				 error_desc,
				 make_static_strnode("instead of a list"),
				 END_OF_LIST);
	    break;
	case DEEPEND:
	    if (error_desc == NIL) {
		err_mesg = cons_list(0,
		   make_static_strnode("END inside multi-line instruction.\n"),
		   END_OF_LIST);
	    } else {
		err_mesg = cons_list(0,
		   make_static_strnode("END inside multi-line instruction in"),
		   error_desc,
		   END_OF_LIST);
	    }
	    break;
	default:
	    prepare_to_exit(FALSE);
	    ndprintf(stdout,"Unknown error condition - internal error.\n");
	    exit(1);
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
	err_print();
	return(UNBOUND);
    }
    err_act = valnode__caseobj(Erract);
    if (err_act != NIL && err_act != UNDEFINED) {
	if (error_type != erract_errtype) {
	    int sv_val_status = val_status;

	    erract_errtype = error_type;
	    setvalnode__caseobj(Erract, NIL);
	    val_status = 5;
	    val = err_eval_driver(err_act);
	    val_status = sv_val_status;
	    setvalnode__caseobj(Erract, err_act);
	    if (recoverable == TRUE && val != UNBOUND) {
		return(val);
	    } else if (recoverable == FALSE && val != UNBOUND) {
		ndprintf(stdout,"You don't say what to do with %s\n", val);
		val = UNBOUND;
		throw_node = Toplevel;
	    } else {
		return(UNBOUND);
	    }
	} else {
	    ndprintf(stdout,"Erract loop\n");
	    throw_node = Toplevel;
	}
    } else {	/* no erract */
	throw_node = Error;
    }
    stopping_flag = THROWING;
    output_node = UNBOUND;
    return(val);
}

NODE *lerror(NODE *args) {
    NODE *val;

    val = err_mesg;
    err_mesg = NIL;
    return(val);
}

#ifndef HAVE_MEMCPY
void memcpy(char *to, char *from, size_t len) {
    while (--len >= 0)
	*to++ = *from++;
}
#endif

NODE *lpause(NODE *args) {
    NODE *elist = NIL, *val = UNBOUND, *uname = NIL;
    int sav_input_blocking;
    int sv_val_status;
#ifndef TIOCSTI
    jmp_buf sav_iblk;
#endif

    if (err_mesg != NIL) err_print();
 /* if (ufun != NIL) */ {
	uname = ufun;
	ndprintf(stdout, "Pausing...\n");
#ifndef TIOCSTI
	memcpy((char *)(&sav_iblk), (char *)(&iblk_buf), sizeof(jmp_buf));
#endif
	sav_input_blocking = input_blocking;
	input_blocking = 0;
#ifdef mac
	csetmode(C_ECHO, stdin);
	fflush(stdin);
#endif
	sv_val_status = val_status;
	while (RUNNING) {
	    if (uname != NIL) print_node(stdout, uname);
	    elist = reader(stdin, "? ");
	    if (NOT_THROWING) elist = parser(elist, TRUE);
	    else elist = NIL;
#ifndef WIN32
	    if (feof(stdin) && !isatty(0)) lbye(NIL);
#endif
#ifdef __ZTC__
	    if (feof(stdin)) rewind(stdin);
#endif
	    val_status = 5;
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
		    val_status = sv_val_status;
			if (uname != NIL) {
				ufun = uname;
			}
		    return(val);
		} else if (compare_node(throw_node, Error, TRUE) == 0) {
		    err_print();
		    stopping_flag = RUN;
		}
	    }
	}
#ifndef TIOCSTI
	memcpy((char *)(&iblk_buf), (char *)(&sav_iblk), sizeof(jmp_buf));
#endif
	input_blocking = sav_input_blocking;
	unblock_input();
	val_status = sv_val_status;
	if (uname != NIL) {
		ufun = uname;
	}
/*  } else {
	stopping_flag = THROWING;
	throw_node = Toplevel;
 */ }
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
