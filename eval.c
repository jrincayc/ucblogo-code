/*
 *      eval.c          logo eval/apply module                  dko
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

#define save(register)	    push(register, stack)
#define restore(register)   (register = car(stack), pop(stack))

#define save2(reg1,reg2)    (push(reg1,stack),stack->n_obj=reg2)
#define restore2(reg1,reg2) (reg2 = getobject(stack), \
			     reg1 = car(stack), pop(stack))

/* saving and restoring FIXNUMs rather than NODEs */

#define numsave(register)   numpush(register,&numstack)
#define numrestore(register) (register=(FIXNUM)car(numstack), numstack=cdr(numstack))

#define num2save(reg1,reg2) (numpush(reg1,&numstack),numstack->n_obj=(NODE *)reg2)
#define num2restore(reg1,reg2) (reg2=(FIXNUM)getobject(numstack), \
				reg1=(FIXNUM)car(numstack), numstack=cdr(numstack))

/* save and restore a FIXNUM (reg1) and a NODE (reg2) */

#define mixsave(reg1,reg2)  (numsave(reg1), save(reg2))
#define mixrestore(reg1,reg2)	(numrestore(reg1), restore(reg2))

#define newcont(tag)	    (numsave(cont), cont = (FIXNUM)tag)

/* These variables are all externed in globals.h */

NODE
*fun		= NIL,  /* current function name */
*ufun		= NIL,	/* current user-defined function name */
*last_ufun	= NIL,	/* the function that called this one */
*this_line	= NIL,	/* the current instruction line */
*last_line	= NIL,	/* the line that called this one */
*var_stack	= NIL,	/* the stack of variables and their bindings */
*var		= NIL,	/* frame pointer into var_stack */
*upvar		= NIL,  /* for LOCAL, one stack frame up */
*last_call	= NIL,	/* the last proc called */
*didnt_output_name = NIL,   /* the name of the proc that didn't OP */
*didnt_get_output  = NIL,   /* the name of the proc that wanted the OP */
*output_node    = NIL;	/* the output of the current function */

CTRLTYPE    stopping_flag = RUN;
char	    *logolib, *helpfiles;
FIXNUM	    tailcall; /* 0 in sequence, 1 for tail, -1 for arg */
FIXNUM	    val_status;	    /* 0 means no value allowed (body of cmd),
			       1 means value required (arg),
			       2 means OUTPUT ok (body of oper),
			       3 means val or no val ok (fn inside catch),
			       4 means no value in macro (repeat),
			       5 means value maybe ok in macro (catch)
			       6 means value required in macro (show run...)
			       7 means value required *now* (tail call of 6)
			     */
FIXNUM	    dont_fix_ift = 0;
FIXNUM	    user_repcount = -1;

/* These variables are local to this file. */
NODE *qm_list = NIL;	/* question mark list */
static int trace_level = 0;	/* indentation level when tracing */

/* These first few functions are externed in globals.h */

void numpush(FIXNUM obj, NODE **stack) {
    NODE *temp = newnode(CONT); /*GC*/

    temp->n_car = (NODE *)obj;
    temp->n_cdr = *stack;
    *stack = temp;
}

/* forward declaration */
NODE *evaluator(NODE *list, enum labels where);

/* Evaluate a line of input. */
void eval_driver(NODE *line) {
    evaluator(line, begin_line);
}

/* Evaluate a sequence of expressions until we get a value to return.
 * (Called from erract.)
 */ 
NODE *err_eval_driver(NODE *seq) {
    val_status = 5;
    return evaluator(seq, begin_seq);
}

/* The logo word APPLY. */
NODE *lapply(NODE *args) {
    return make_cont(begin_apply, args);
}

/* The logo word ? <question-mark>. */
NODE *lqm(NODE *args) {
    FIXNUM argnum = 1, i;
    NODE *np = qm_list;

    if (args != NIL) argnum = getint(pos_int_arg(args));
    if (stopping_flag == THROWING) return(UNBOUND);
    i = argnum;
    while (--i > 0 && np != NIL) np = cdr(np);
    if (np == NIL)
	return(err_logo(BAD_DATA_UNREC,make_intnode(argnum)));
    return(car(np));
}

/* The rest of the functions are local to this file. */

/* Warn the user if a local variable shadows a global one. */
void tell_shadow(NODE *arg) {
    if (flag__caseobj(arg, VAL_STEPPED))
	err_logo(SHADOW_WARN, arg);
}

/* Check if a local variable is already in this frame */
int not_local(NODE *name, NODE *sp) {
    for ( ; sp != var; sp = cdr(sp)) {
	if (compare_node(car(sp),name,TRUE) == 0) {
	    return FALSE;
	}
    }
    return TRUE;
}

/* reverse a list destructively */
NODE *reverse(NODE *list) {
    NODE *ret = NIL, *temp;

    while (list != NIL) {
	temp = list;
	list = cdr(list);
	setcdr(temp, ret);
	ret = temp;
    }
    return ret;
}

/* nondestructive append */
NODE *append(NODE *a, NODE *b) {
    if (a == NIL) return b;
    return cons(car(a), append(cdr(a), b));
}

/* nondestructive flatten */
NODE *flatten(NODE *a) {
    if (a == NIL) return NIL;
    return append(car(a), flatten(cdr(a)));
}

/* Reset the var stack to the previous place holder.
 */
void reset_args(NODE *old_stack) {
    for (; var_stack != old_stack; pop(var_stack))
	setvalnode__caseobj(car(var_stack), getobject(var_stack));
}

NODE *bf3(NODE *name) {
    NODE *string = cnv_node_to_strnode(name);
    return make_strnode(getstrptr(string)+3, getstrhead(string),
			getstrlen(string)-3, nodetype(string), strcpy);
}

NODE *deep_copy(NODE *exp) {
    NODE *val, **p, **q;
    FIXNUM arridx;

    if (exp == NIL) return NIL;
    else if (is_list(exp)) {
	val = cons(deep_copy(car(exp)), deep_copy(cdr(exp)));
	val->n_obj = deep_copy(exp->n_obj);
	settype(val, nodetype(exp));
    } else if (nodetype(exp) == ARRAY) {
	val = make_array(getarrdim(exp));
	setarrorg(val, getarrorg(exp));
	for (p = getarrptr(exp), q = getarrptr(val), arridx=0;
	     arridx < getarrdim(exp); arridx++, p++)
	*q++ = deep_copy(*p);
    } else val = exp;
    return val;
}



/* An explicit control evaluator, taken almost directly from SICP, section
 * 5.2.  list is a flat list of expressions to evaluate.  where is a label to
 * begin at.  Return value depends on where.
 */ 
NODE *evaluator(NODE *list, enum labels where) {

    /* registers */
    NODE    *exp    = NIL,  /* the current expression */
	    *val    = NIL,  /* the value of the last expression */
	    *proc   = NIL,  /* the procedure definition */
	    *argl   = NIL,  /* evaluated argument list */
	    *unev   = NIL,  /* list of unevaluated expressions */
	    *stack  = NIL,  /* register stack */
	    *numstack = NIL,/* stack whose elements aren't objects */
	    *parm   = NIL,  /* the current formal */
	    *catch_tag = NIL,
	    *arg    = NIL;  /* the current actual */

    NODE    *vsp    = 0,    /* temp ptr into var_stack */
	    *formals = NIL; /* list of formal parameters */
    FIXNUM  cont   = 0;	    /* where to go next */

    int i;
    BOOLEAN tracing = FALSE; /* are we tracing the current procedure? */
    FIXNUM oldtailcall;	    /* in case of reentrant use of evaluator */
    FIXNUM repcount;	    /* count for repeat */
    FIXNUM old_ift_iff;
	
    oldtailcall = tailcall;
    old_ift_iff = ift_iff_flag;
    save2(var,this_line);
    var = var_stack;
    save2(fun,ufun);
    cont = (FIXNUM)all_done;
    numsave((FIXNUM)cont);
    newcont(where);
    goto fetch_cont;
    
begin_line:
    this_line = list;
    newcont(end_line);
begin_seq:
    make_tree(list);
    if (!is_tree(list)) {
	val = UNBOUND;
	goto fetch_cont;
    }
    unev = tree__tree(list);
    val = UNBOUND;
    goto eval_sequence;

end_line:
    if (val != UNBOUND) {
	if (NOT_THROWING) err_logo(DK_WHAT, val);
    }
    val = NIL;
    goto fetch_cont;


/* ----------------- EVAL ---------------------------------- */

tail_eval_dispatch:
    tailcall = 1;
eval_dispatch:
    switch (nodetype(exp)) {
	case QUOTE:			/* quoted literal */
	    val = /* deep_copy */ (node__quote(exp));
	    goto fetch_cont;
	case COLON:			/* variable */
	    val = valnode__colon(exp);
	    while (val == UNBOUND && NOT_THROWING)
		val = err_logo(NO_VALUE, node__colon(exp));
	    goto fetch_cont;
	case CONS:			/* procedure application */
	    if (tailcall == 1 && is_macro(car(exp)) &&
				 (is_list(procnode__caseobj(car(exp)))
				    || !compare_node(car(exp), Goto, TRUE))) {
		/* tail call to user-defined macro must be treated as non-tail
		 * because the expression returned by the macro
		 * remains to be evaluated in the caller's context */
		unev = NIL;
		goto non_tail_eval;
	    }
	    fun = car(exp);
	    if (fun == Not_Enough_Node) {
		err_logo(TOO_MUCH, NIL);
		val = UNBOUND;
		goto fetch_cont;
	    }
	    if (cdr(exp) != NIL)
		goto ev_application;
	    else
		goto ev_no_args;
	case ARRAY:			/* array must be copied */
	    val = deep_copy(exp);
	    goto fetch_cont;
	default:
	    val = exp;		/* self-evaluating */
	    goto fetch_cont;
    }

ev_no_args:
    /* Evaluate an application of a procedure with no arguments. */
    argl = NIL;
    goto apply_dispatch;    /* apply the procedure */

ev_application:
    /* Evaluate an application of a procedure with arguments. */
    unev = cdr(exp);
    argl = NIL;
    mixsave(tailcall,var);
    num2save(val_status,ift_iff_flag);
    save2(didnt_get_output,didnt_output_name);
eval_arg_loop:
    if (unev == NIL) goto eval_args_done;
    exp = car(unev);
    if (exp == Not_Enough_Node) {
	if (NOT_THROWING)
	    err_logo(NOT_ENOUGH, NIL);
	goto eval_args_done;
    }
    save2(argl,proc);
    save2(unev,fun);
    save2(ufun,last_ufun);
    save2(this_line,last_line);
    var = var_stack;
    tailcall = -1;
    val_status = 1;
    didnt_get_output = cons_list(0, fun, ufun, this_line, END_OF_LIST);
    didnt_output_name = NIL;
    newcont(accumulate_arg);
    goto eval_dispatch;	    /* evaluate the current argument */

accumulate_arg:
    /* Put the evaluated argument into the argl list. */
    reset_args(var);
    restore2(this_line,last_line);
    restore2(ufun,last_ufun);
    last_call = fun;
    restore2(unev,fun);
    restore2(argl,proc);
    while (NOT_THROWING && val == UNBOUND) {
	val = err_logo(DIDNT_OUTPUT, NIL);
    }
    push(val, argl);
    pop(unev);
    goto eval_arg_loop;

eval_args_done:
    restore2(didnt_get_output,didnt_output_name);
    num2restore(val_status,ift_iff_flag);
    mixrestore(tailcall,var);
    if (stopping_flag == THROWING) {
	val = UNBOUND;
	goto fetch_cont;
    }
    argl = reverse(argl);
/* --------------------- APPLY ---------------------------- */
apply_dispatch:
    /* Load in the procedure's definition and decide whether it's a compound
     * procedure or a primitive procedure.
     */
    proc = procnode__caseobj(fun);
    if (is_macro(fun)) {
	num2save(val_status,tailcall);
	val_status = 1;
	newcont(macro_return);
    }
    if (proc == UNDEFINED) {	/* 5.0 punctuationless variables */
	if (compare_node(valnode__caseobj(AllowGetSet),True,TRUE)
	       != 0) {	    /* No getter/setter allowed, punt */
	    val = err_logo(DK_HOW, fun);
	    goto fetch_cont;
	} else if (argl == NIL) {	/* possible var getter */
	    val = valnode__caseobj(fun);
	    if (val == UNBOUND && NOT_THROWING)
		val = err_logo(DK_HOW, fun);
	    else if (val != UNBOUND) {
		(void)ldefine(cons(fun, cons(
		   cons(NIL,cons(cons(Output,cons(make_colon(fun),NIL)),NIL))
		  ,NIL)));    /* make real proc so no disk load next time */
		setflag__caseobj(fun,PROC_BURIED);
	    }
	    goto fetch_cont;
	} else {		/* var setter */
	    (void)ldefine(cons(fun, cons(
		cons(Listvalue,
		     cons(cons(Make,
			       cons(make_quote(bf3(fun)),
				    cons(Dotsvalue,NIL))),
			  NIL))
		,NIL)));
	    setflag__caseobj(fun,PROC_BURIED);
	    argl = cons(bf3(fun), argl);
	    if (NOT_THROWING)
		val = lmake(argl);
	    goto fetch_cont;
	}
    }
    if (is_list(proc)) goto compound_apply;
    /* primitive_apply */
    if (NOT_THROWING) {
	if ((tracing = flag__caseobj(fun, PROC_TRACED))) {
	    for (i = 0; i < trace_level; i++) {
		print_space(stdout);
	    }
	    ndprintf(stdout, "( %s ", fun);
	    if (argl != NIL) {
		arg = argl;
		while (arg != NIL) {
		    print_node(stdout, maybe_quote(car(arg)));
		    print_space(stdout);
		    arg = cdr(arg);
		}
	    }
	    print_char(stdout, ')');
	    new_line(stdout);
	}
	val = (*getprimfun(proc))(argl);
        if (tracing && NOT_THROWING) {
	    for (i = 0; i < trace_level; i++) {
		print_space(stdout);
	    }
	    print_node(stdout, fun);
	    if (val == UNBOUND)
	        ndprintf(stdout, " %t\n", message_texts[TRACE_STOPS]);
	    else {
	        ndprintf(stdout, " %t %s\n", message_texts[TRACE_OUTPUTS],
					     maybe_quote(val));
	    }
        }
    } else
	val = UNBOUND;
#define do_case(x) case x: goto x;
fetch_cont:
    {
	enum labels x = (enum labels)cont;
	cont = (FIXNUM)car(numstack);
	numstack=cdr(numstack);
	switch (x) {
	    do_list(do_case)
	    default: abort();
	}
    }

compound_apply:
#ifdef mac
    check_mac_stop();
#endif
#ifdef ibm
    check_ibm_stop();
#endif
    if ((tracing = flag__caseobj(fun, PROC_TRACED))) {
	for (i = 0; i < trace_level; i++) print_space(writestream);
	trace_level++;
	ndprintf(writestream, "( %s ", fun);
    }
/* Bind the actuals to the formals */
lambda_apply:
    vsp = var_stack;	/* remember where we came in */
    for (formals = formals__procnode(proc);
    	 formals != NIL;
	 formals = cdr(formals)) {
	    parm = car(formals);
	    if (nodetype(parm) == INT) break;	/* default # args */
	    if (argl != NIL) {
		arg = car(argl);
		if (tracing) {
		    print_node(writestream, maybe_quote(arg));
		    print_space(writestream);
		}
	    } else
		arg = UNBOUND;
	    if (nodetype(parm) == CASEOBJ) {
		if (not_local(parm,vsp)) {
		    push(parm, var_stack);
		    var_stack->n_obj = valnode__caseobj(parm);
		}
		tell_shadow(parm);
		setvalnode__caseobj(parm, arg);
		if (arg == UNBOUND)
		    err_logo(NOT_ENOUGH, fun);
	    } else if (nodetype(parm) == CONS) {
		/* parm is optional or rest */
		if (not_local(car(parm),vsp)) {
		    push(car(parm), var_stack);
		    var_stack->n_obj = valnode__caseobj(car(parm));
		}
		tell_shadow(car(parm));
		if (cdr(parm) == NIL) {		    /* parm is rest */
		    setvalnode__caseobj(car(parm), argl);
		    if (tracing) {
			if (argl != NIL) pop(argl);
			while (argl != NIL) {
			    arg = car(argl);
			    print_node(writestream, maybe_quote(arg));
			    print_space(writestream);
			    pop(argl);
			}
		    } else argl = NIL;
		    break;
		}
		if (arg == UNBOUND) {		    /* use default */
		    save(proc);
		    save2(fun,var);
		    save2(ufun,last_ufun);
		    save2(this_line,last_line);
		    save2(didnt_output_name,didnt_get_output);
		    num2save(ift_iff_flag,val_status);
		    var = var_stack;
		    tailcall = -1;
		    val_status = 1;
		    save2(formals,argl);
		    save(vsp);
		    list = cdr(parm);
		    if (NOT_THROWING)
			make_tree(list);
		    else
			list = NIL;
		    if (!is_tree(list)) {
			val = UNBOUND;
			goto set_args_continue;
		    }
		    unev = tree__tree(list);
		    val = UNBOUND;
		    newcont(set_args_continue);
		    goto eval_sequence;

set_args_continue:
		    restore(vsp);
		    restore2(formals,argl);
		    parm = car(formals);
		    reset_args(var);
		    num2restore(ift_iff_flag,val_status);
		    restore2(didnt_output_name,didnt_get_output);
		    restore2(this_line,last_line);
		    restore2(ufun,last_ufun);
		    restore2(fun,var);
		    restore(proc);
		    arg = val;
		}
		setvalnode__caseobj(car(parm), arg);
	    }
	    if (argl != NIL) pop(argl);
    }
    if (argl != NIL) {
	err_logo(TOO_MUCH, NIL);
    }
    if (check_throwing) {
	val = UNBOUND;
	goto fetch_cont;
    }
    vsp = NIL;
    if ((tracing = !is_list(fun)) && flag__caseobj(fun, PROC_TRACED)) {
	if (NOT_THROWING) print_char(writestream, ')');
	new_line(writestream);
	save(fun);
	newcont(compound_apply_continue);
    }
    val = UNBOUND;
    last_ufun = ufun;
    if (!is_list(fun)) ufun = fun;
    last_line = this_line;
    this_line = NIL;
    proc = (is_list(fun) ? anonymous_function(fun) : procnode__caseobj(fun));
    list = bodylist__procnode(proc);	/* get the body ... */
    make_tree_from_body(list);
    if (!is_tree(list)) {
	goto fetch_cont;
    }
    unev = tree__tree(list);
    if (NOT_THROWING) stopping_flag = RUN;
    output_node = UNBOUND;
    if (val_status == 1) val_status = 2;
    else if (val_status == 5) val_status = 3;
    else val_status = 0;
eval_sequence:
    /* Evaluate each expression in the sequence.  Stop as soon as
     * val != UNBOUND.
     */
    if (!RUNNING || val != UNBOUND) {
	goto fetch_cont;
    }
    if (nodetype(unev) == LINE) {
	if (the_generation != (generation__line(unev))) {
	    /* something redefined while we're running */
	    int linenum = 0;
	    this_line = tree__tree(bodylist__procnode(proc));
	    while (this_line != unev) {
		/* If redef isn't end of line, don't try to fix,
		   but don't blow up either. (Maybe not called from here.) */
		if (this_line == NULL) goto nofix;
		if (nodetype(this_line) == LINE) linenum++;
		this_line = cdr(this_line);
	    }
	    untreeify_proc(proc);
	    make_tree_from_body(bodylist__procnode(proc));
	    unev = tree__tree(bodylist__procnode(proc));
	    while (--linenum >= 0) {
		do pop(unev);
		while (unev != NIL && nodetype(unev) != LINE);
	    }
	}
nofix:	this_line = unparsed__line(unev);
	if (ufun != NIL && flag__caseobj(ufun, PROC_STEPPED)) {
	    if (tracing) {
		int i = 1;
		while (i++ < trace_level) print_space(stdout);
	    }
	    print_node(stdout, this_line);
	    (void)reader(stdin, " >>> ");
	}
    }
    exp = car(unev);
    pop(unev);
    if (exp != NIL &&
        is_list(exp) && (is_tailform(procnode__caseobj(car(exp))))) {
      i = (int)getprimpri(procnode__caseobj(car(exp)));
      if (i == OUTPUT_PRIORITY) {
	didnt_get_output = cons_list(0,car(exp),ufun,this_line,END_OF_LIST);
	didnt_output_name = NIL;
	if (cadr(exp) == Not_Enough_Node) {
	    err_logo(NOT_ENOUGH,car(exp));
	    val = UNBOUND;
	    goto fetch_cont;
	}
	if (val_status == 2 || val_status == 3) {
	    val_status = 1;
	    exp = cadr(exp);
	    goto tail_eval_dispatch;
	} else if (ufun == NIL) {
	    err_logo(AT_TOPLEVEL,car(exp));
	    val = UNBOUND;
	    goto fetch_cont;
	} else if (val_status < 4) {
	    val_status = 1;
	    exp = cadr(exp);
	    unev = NIL;
	    goto non_tail_eval;	    /* compute value then give error */
	}
      } else if (i == STOP_PRIORITY) {
	if (ufun == NIL) {
	    err_logo(AT_TOPLEVEL,car(exp));
	    val = UNBOUND;
	    goto fetch_cont;
	} else if (val_status == 0 || val_status == 3) {
	    val = UNBOUND;
	    goto fetch_cont;
	} else if (val_status < 4) {
	    didnt_output_name = fun;
	    val = UNBOUND;
	    goto fetch_cont;
	}
      } else { /* maybeoutput */
	exp = cadr(exp);
	val_status = 5;
	goto tail_eval_dispatch;
      }
    }
    if (unev == NIL) {
	if (val_status == 2 || val_status == 4) {
	    didnt_output_name = fun;
	    unev = UNBOUND;
	    goto non_tail_eval;
	} else {
	    goto tail_eval_dispatch;
	}
    }
    if (car(unev) != NIL && is_list(car(unev)) && 
		(is_tailform(procnode__caseobj(car(car(unev))))) &&
		getprimpri(procnode__caseobj(car(car(unev)))) == STOP_PRIORITY) {
	if ((val_status == 0 || val_status == 3) && ufun != NIL) {
	    goto tail_eval_dispatch;
	} else if (val_status < 4 && ufun != NIL) {
	    didnt_output_name = fun;
	    goto tail_eval_dispatch;
	}
    }
non_tail_eval:
    save2(unev,fun);
    num2save(ift_iff_flag,val_status);
    save2(ufun,last_ufun);
    save2(this_line,last_line);
    save2(var,proc);
    save(upvar);
    upvar = var;
    var = var_stack;
    tailcall = 0;
    newcont(eval_sequence_continue);
    goto eval_dispatch;

eval_sequence_continue:
    reset_args(var);
    restore(upvar);
    restore2(var,proc);
    restore2(this_line,last_line);
    restore2(ufun,last_ufun);
    if (dont_fix_ift) {
	num2restore(dont_fix_ift,val_status);
	dont_fix_ift = 0;
    } else
	num2restore(ift_iff_flag,val_status);
    restore2(unev,fun);
    if (stopping_flag == MACRO_RETURN) {
	if (unev == UNBOUND) unev = NIL;
	if (val != NIL && is_list(val) && (car(val) == Tag))
	    unev = cdr(val);	/* from goto */
	else
	    unev = append(val, unev);
	val = UNBOUND;
	stopping_flag = RUN;
	if (unev == NIL) goto fetch_cont;
    } else if (val_status < 4) {
	if (STOPPING || RUNNING) output_node = UNBOUND;
	if (stopping_flag == OUTPUT || STOPPING) {
	    stopping_flag = RUN;
	    val = output_node;
	    if (val != UNBOUND && val_status == 1 && NOT_THROWING) {
		didnt_output_name = Output;
		err_logo(DIDNT_OUTPUT,Output);
	    }
	    if (val == UNBOUND && val_status == 1 && NOT_THROWING) {
		didnt_output_name = Stop;
		err_logo(DIDNT_OUTPUT,Output);
	    }
	    goto fetch_cont;
	}
    }
    if (val != UNBOUND) {
	err_logo((unev == NIL ? DK_WHAT_UP : DK_WHAT), val);
	val = UNBOUND;
    }
    if (NOT_THROWING && (unev == NIL || unev == UNBOUND)) {
	if (val_status != 4)  err_logo(DIDNT_OUTPUT,NIL);
	goto fetch_cont;
    }
    goto eval_sequence;

compound_apply_continue:
    /* Only get here if tracing */
    restore(fun);
    --trace_level;
    if (NOT_THROWING) {
	for (i = 0; i < trace_level; i++) print_space(writestream);
	print_node(writestream, fun);
	if (val == UNBOUND)
	    ndprintf(writestream, " %t\n", message_texts[TRACE_STOPS]);
	else {
	    ndprintf(writestream, " %t %s\n", message_texts[TRACE_OUTPUTS],
					      maybe_quote(val));
	}
    }
    goto fetch_cont;

/* --------------------- MACROS ---------------------------- */

macro_return:
    num2restore(val_status,tailcall);
    while (!is_list(val) && NOT_THROWING) {
	val = err_logo(ERR_MACRO,val);
    }
    if (NOT_THROWING) {
	if (is_cont(val)) {
	    newcont(cont__cont(val));
	    val = val__cont(val);
	    goto fetch_cont;
	}
	if (tailcall == 0) {
	    make_tree(val);
	    if (NOT_THROWING) {
		stopping_flag = MACRO_RETURN;
		if (!is_tree(val)) val = NIL;
		else val = tree__tree(val);
	    } else val = UNBOUND;
	    goto fetch_cont;
	}
	list = val;
	goto begin_seq;
    }
    val = UNBOUND;
    goto fetch_cont;

run_continuation:
    list = val;
    val_status = 5;
    goto begin_seq;

runresult_continuation:
    list = val;
    newcont(runresult_followup);
    val_status = 5;
    goto begin_seq;

runresult_followup:
    if (val == UNBOUND) {
	val = NIL;
    } else {
	val = cons(val, NIL);
    }
    goto fetch_cont;

repeat_continuation:
    list = cdr(val);
    repcount = getint(car(val));
    user_repcount = 0;
repeat_again:
    val = UNBOUND;
    if (repcount == 0) {
	user_repcount = -1;
	goto fetch_cont;
    }
    user_repcount++;
    save(list);
    num2save(repcount,user_repcount);
    num2save(val_status,tailcall);
    val_status = 4;
    newcont(repeat_followup);
    goto begin_seq;

repeat_followup:
    if (val != UNBOUND && NOT_THROWING) {
	err_logo(DK_WHAT, val);
    }
    num2restore(val_status,tailcall);
    num2restore(repcount,user_repcount);
    restore(list);
    if (val_status < 4 && tailcall != 0) {
	if (STOPPING || RUNNING) output_node = UNBOUND;
	if (stopping_flag == OUTPUT || STOPPING) {
	    stopping_flag = RUN;
	    val = output_node;
	    if (val != UNBOUND && val_status < 2) {
		err_logo(DK_WHAT_UP,val);
	    }
	    goto fetch_cont;
	}
    }
    if (repcount > 0)    /* negative means forever */
	--repcount;
#ifdef mac
    check_mac_stop();
#endif
#ifdef ibm
    check_ibm_stop();
#endif
    if (RUNNING) goto repeat_again;
    val = UNBOUND;
    user_repcount = -1;
    goto fetch_cont;

catch_continuation:
    list = cdr(val);
    catch_tag = car(val);
    if (compare_node(catch_tag,Error,TRUE) == 0) {
	push(Erract, var_stack);
	var_stack->n_obj = valnode__caseobj(Erract);
	setvalnode__caseobj(Erract, UNBOUND);
    }
    save(catch_tag);
    save2(didnt_output_name,didnt_get_output);
    num2save(val_status,tailcall);
    newcont(catch_followup);
    val_status = 5;
    goto begin_seq;

catch_followup:
    num2restore(val_status,tailcall);
    restore2(didnt_output_name,didnt_get_output);
    restore(catch_tag);
    if (val_status < 4 && tailcall != 0) {
	if (STOPPING || RUNNING) output_node = UNBOUND;
	if (stopping_flag == OUTPUT || STOPPING) {
	    stopping_flag = RUN;
	    val = output_node;
	    if (val != UNBOUND && val_status < 2) {
		err_logo(DK_WHAT_UP,val);
	    }
	}
    }
    if (stopping_flag == THROWING &&
	compare_node(throw_node, catch_tag, TRUE) == 0) {
	    throw_node = UNBOUND;
	    stopping_flag = RUN;
	    val = output_node;
    }
    goto fetch_cont;

goto_continuation:
    if (NOT_THROWING) {
	if (ufun == NIL) {
	    err_logo(AT_TOPLEVEL, Goto);
	    val = UNBOUND;
	    goto fetch_cont;
	}
	proc = procnode__caseobj(ufun);
	list = bodylist__procnode(proc);
	unev = tree__tree(list);
	while (unev != NIL && !check_throwing) {
	    if (nodetype(unev) == LINE)
		this_line = unparsed__line(unev);
	    exp = car(unev);
	    pop(unev);
	    if (is_list (exp) &&
		    (object__caseobj(car(exp)) == object__caseobj(Tag)) &&
		    (nodetype(cadr(exp)) == QUOTE) &&
		    compare_node(val, node__quote(cadr(exp)), TRUE) == 0) {
		val = cons(Tag, unev);
		stopping_flag = MACRO_RETURN;
		goto fetch_cont;
	    }
	}
	err_logo(BAD_DATA_UNREC, val);
    }
    val = UNBOUND;
    goto fetch_cont;

begin_apply:
    /* This is for lapply. */
    fun = car(val);
    while (nodetype(fun) == ARRAY && NOT_THROWING)
	fun = err_logo(APPLY_BAD_DATA, fun);
    argl = cadr(val);
    val = UNBOUND;
    while (!is_list(argl) && NOT_THROWING)
	argl = err_logo(APPLY_BAD_DATA, argl);
    if (NOT_THROWING && fun != NIL) {
	if (is_list(fun)) {		    /* template */
	    if (is_list(car(fun)) && cdr(fun) != NIL) {
		if (is_list(cadr(fun))) {	/* procedure text form */
		    proc = anonymous_function(fun);
		    tracing = 0;
		    goto lambda_apply;
		}
		/* lambda form */
		formals = car(fun);
		save(var);
		numsave(tailcall);
		tailcall = 0;
		llocal(formals);    /* bind the formals locally */
		numrestore(tailcall);
		for ( ;
		     formals && argl && NOT_THROWING;
		     formals = cdr(formals),
		     argl = cdr(argl))
			setvalnode__caseobj(car(formals), car(argl));
		list = cdr(fun);
		save(qm_list);
		newcont(after_lambda);
		goto lambda_qm;
	    } else {		/* question-mark form */
		save(qm_list);
		qm_list = argl;
		list = fun;
lambda_qm:
		make_tree(list);
		if (list == NIL || !is_tree(list)) {
		    goto qm_failed;
		}
		unev = tree__tree(list);
		save2(didnt_output_name,didnt_get_output);
		num2save(val_status,tailcall);
		newcont(qm_continue);
		val_status = 5;
		goto eval_sequence;

qm_continue:
		num2restore(val_status,tailcall);
		restore2(didnt_output_name,didnt_get_output);
		if (val_status < 4 && tailcall != 0) {
		    if (STOPPING || RUNNING) output_node = UNBOUND;
		    if (stopping_flag == OUTPUT || STOPPING) {
			stopping_flag = RUN;
			val = output_node;
			if (val != UNBOUND && val_status < 2) {
			    err_logo(DK_WHAT_UP,val);
			}
		    }
		}
qm_failed:
		restore(qm_list);
		goto fetch_cont;
	    }
	} else {    /* name of procedure to apply */
	    int min, max, n;
	    NODE *arg;
	    fun = intern(fun);
	    if (procnode__caseobj(fun) == UNDEFINED && NOT_THROWING &&
		fun != Null_Word)
		    silent_load(fun, NULL);    /* try ./<fun>.lg */
	    if (procnode__caseobj(fun) == UNDEFINED && NOT_THROWING &&
		fun != Null_Word)
		    silent_load(fun, logolib); /* try <logolib>/<fun> */
	    proc = procnode__caseobj(fun);
	    while (proc == UNDEFINED && NOT_THROWING) {
		val = err_logo(DK_HOW_UNREC, fun);
	    }
	    if (NOT_THROWING) {
		if (nodetype(proc) == CONS) {
		    min = getint(minargs__procnode(proc));
		    max = getint(maxargs__procnode(proc));
		} else {
		    if (getprimdflt(proc) < 0) {	    /* special form */
			err_logo(DK_HOW_UNREC, fun);    /* can't apply */
			goto fetch_cont;
		    } else {
			min = getprimmin(proc);
			max = getprimmax(proc);
		    }
		}
		for (n = 0, arg = argl; arg != NIL; n++, arg = cdr(arg));
		if (n < min) {
		    err_logo(NOT_ENOUGH, NIL);
		} else if (n > max && max >= 0) {
		    err_logo(TOO_MUCH, NIL);
		} else {
		    goto apply_dispatch;
		}
	    }
	}
    }
    goto fetch_cont;

after_lambda:
    reset_args(var);
    restore(var);
    goto fetch_cont;

all_done:
    tailcall = oldtailcall;
    ift_iff_flag = old_ift_iff;
    restore2(fun,ufun);
    reset_args(var);
    restore2(var,this_line);
    return(val);
}
