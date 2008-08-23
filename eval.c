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

#define WANT_EVAL_REGS 1
#include "logo.h"
#include "globals.h"

/* evaluator registers that need saving around evals */
struct registers regs;
NODE *Regs_Node;
int num_saved_nodes;
int inside_evaluator = 0;
NODE *eval_buttonact = NIL;

/* node-value registers that don't need saving */

NODE    *exp    = NIL,  /* the current expression */
	*val    = NIL,  /* the value of the last expression */
	*stack  = NIL,  /* register stack */
	*numstack = NIL,/* stack whose elements aren't objects */
	*parm   = NIL,  /* the current formal */
	*catch_tag = NIL,
	*arg    = NIL;  /* the current actual */

NODE
*var_stack	= NIL,	/* the stack of variables and their bindings */
*last_call	= NIL,	/* the last proc called */
*output_node    = NIL,	/* the output of the current function */
*output_unode	= NIL;	/* the unode in which we saw the output */

#define DEBUGGING 0

#if DEBUGGING
#define DEB_STACK 0	    /* set to 1 to log save/restore */
#define DEB_CONT 0	    /* set to 1 to log newcont/fetch_cont */

#define do_debug(x) \
    x(exp) x(unev) x(val) x(didnt_get_output) x(didnt_output_name) x(fun) \
    x(proc)
#define deb_enum(x) \
    ndprintf(stdout, #x " = %s,  ", x);

void vs_print() {
    FIXNUM vs = val_status;
    int i;
    static char *vnames[] = {"VALUE_OK", "NO_VALUE_OK", "OUTPUT_OK",
			    "STOP_OK", "OUTPUT_TAIL", "STOP_TAIL"};
    static char *names[] = {"RUN", "STOP", "OUTPUT", "THROWING",
			    "MACRO_RETURN"};

    if (!varTrue(Redefp)) return;
    printf("Val_status = ");
    for (i=0; i<6; i++) {
	if (vs&1) {
	    printf(vnames[i]);
	    vs >>= 1;
	    if (vs != 0) printf("|");
	} else vs >>= 1;
	if (vs == 0) break; 
    }
    if (vs != 0) printf("0%o", vs<<6);
    printf(", stopping_flag = %s\n", names[stopping_flag]);
}

void debprint(char *name) {
    if (!varTrue(Redefp)) return;
    printf("%s: ", name);
    do_debug(deb_enum)
    vs_print();
    printf("current_unode=0x%x, output_unode=0x%x\n",current_unode,
	   output_unode);
}
#define debprint2(a,b) if (varTrue(Redefp)) ndprintf(stdout,a,b)
#else
#define debprint(name)
#define debprint2(a,b)
#define DEB_STACK 0
#define DEB_CONT 0
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

#if DEB_STACK
NODE *restname, *restline;
#define save(register) (    debprint2("saving " #register " = %s ", register), \
			    push(register, stack), \
			    push(make_intnode(__LINE__), stack), \
			    debprint2(" at line %s\n", car(stack)), \
			    push(make_static_strnode(#register), stack) )
#define restore(register) ( restname = car(stack), pop(stack), \
			    restline = car(stack), pop(stack), \
			    register = car(stack), pop(stack), \
			    ( (strcmp(getstrptr(restname), #register)) ? (\
				debprint2("*** Restoring " #register " but saved %s",\
					 restname), \
				debprint2(" at line %s! ***\n", restline) \
			    ) : 0), \
			    debprint2("restoring " #register " = %s ", register), \
			    debprint2(" at line %s\n", make_intnode(__LINE__)) )
#define save2(reg1,reg2) (  save(reg1), save(reg2)  )
#define restore2(reg1,reg2) ( restore(reg2), restore(reg1) )
#else
#define save(register)	    push(register, stack)
#define restore(register)   (register = car(stack), pop(stack))

#define save2(reg1,reg2)    (push(reg1,stack),stack->n_obj=reg2)
#define restore2(reg1,reg2) (reg2 = getobject(stack), \
			     reg1 = car(stack), pop(stack))
#endif

/* saving and restoring FIXNUMs rather than NODEs */

#define numsave(register)   numpush(register,&numstack)
#define numrestore(register) (register=(FIXNUM)car(numstack), numstack=cdr(numstack))

#define num2save(reg1,reg2) (numpush(reg1,&numstack),numstack->n_obj=(NODE *)reg2)
#define num2restore(reg1,reg2) (reg2=(FIXNUM)getobject(numstack), \
				reg1=(FIXNUM)car(numstack), numstack=cdr(numstack))

#if DEB_CONT
#define newcont(tag)	debprint("Newcont = " #tag); \
			 numsave(cont); cont = (FIXNUM)tag
#else
#define newcont(tag)	    (numsave(cont), cont = (FIXNUM)tag)
#endif

/* These variables are all externed in globals.h */

CTRLTYPE    stopping_flag = RUN;
char	    *logolib, *helpfiles, *csls;
FIXNUM	    dont_fix_ift = 0;

/* These variables are local to this file. */
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
NODE *err_eval_driver(NODE *seq, BOOLEAN recoverable) {
    val_status = (recoverable ? VALUE_OK : NO_VALUE_OK) |
		    (val_status & (OUTPUT_OK|STOP_OK));
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
    for (; var_stack != old_stack; pop(var_stack)) {
	if (nodetype(var_stack) & NT_LOCAL)
	    setflag__caseobj(car(var_stack), IS_LOCAL_VALUE);
	else
	    clearflag__caseobj(car(var_stack), IS_LOCAL_VALUE);
	setvalnode__caseobj(car(var_stack), getobject(var_stack));
    }
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

int in_eval_save = 0;

void eval_save() {
    push(NIL, stack);
    int_during_gc = 0;
    in_eval_save = 1;
    settype(stack, STACK);
    stack->n_car = (NODE *)malloc(sizeof(regs));
    if (car(stack) == NULL) {
	err_logo(OUT_OF_MEM_UNREC, NIL);
    } else {
	memcpy(car(stack), &regs, sizeof(regs));
    }
    in_eval_save = 0;
    if (int_during_gc != 0) {
	delayed_int();
    }
}

void eval_restore() {
    int_during_gc = 0;
    in_eval_save = 1;
    memcpy(&regs, car(stack), sizeof(regs));
    pop(stack);
    in_eval_save = 0;
    if (int_during_gc != 0) {
	delayed_int();
    }
}

/*
 #ifdef OBJECTS

NODE *val_eval_driver(NODE *seq) {
    val_status = VALUE_OK;
    return evaluator(seq, begin_seq);
}

 #endif
*/

/* An explicit control evaluator, taken almost directly from SICP, section
 * 5.2.  list is a flat list of expressions to evaluate.  where is a label to
 * begin at.  Return value depends on where.
 */ 
NODE *evaluator(NODE *list, enum labels where) {

    FIXNUM  cont   = 0;	    /* where to go next */
    int i;
    BOOLEAN tracing = FALSE; /* are we tracing the current procedure? */
	
    inside_evaluator++;
    eval_save();
    var = var_stack;
    newcont(all_done);
    newcont(where);
    goto fetch_cont;

all_done:
    reset_args(var);
    eval_restore();
    if (dont_fix_ift) {
        ift_iff_flag = dont_fix_ift-1;
        dont_fix_ift = 0;
    }
inside_evaluator--;
return(val);

begin_line:
    this_line = list;
    val_status = NO_VALUE_OK;
    newcont(end_line);
begin_seq:
    debprint("begin_seq");
    make_tree(list);
    if (!is_tree(list)) {
	val = UNBOUND;
	goto fetch_cont;
    }
    unev = tree__tree(list);
    goto eval_sequence;

end_line:
    if (val != UNBOUND) {
	if (NOT_THROWING) err_logo(DK_WHAT, val);
    }
/*    val = NIL;    */
    goto fetch_cont;


/* ----------------- EVAL ---------------------------------- */
/* Get here for actual argument, from eval_sequence (non-tail), or
   from tail call. */

tail_eval_dispatch:
    tailcall = 1;
eval_dispatch:
    debprint("eval_dispatch");
    switch (nodetype(exp)) {
	case QUOTE:			/* quoted literal */
	    val = /* deep_copy */ (node__quote(exp));
	    goto fetch_cont;
	case COLON:			/* variable */

#ifdef OBJECTS
	    val = varValue(node__colon(exp));
#else
	    val = valnode__colon(exp);
#endif
	    while (val == UNBOUND && NOT_THROWING)
		val = err_logo(NO_VALUE, node__colon(exp));
	    goto fetch_cont;
	case CONS:			/* procedure application */
	    if (tailcall == 1 && is_macro(car(exp)) &&
				 (is_list(procnode__caseobj(car(exp)))
		   || isName(car(exp), Name_goto))) {
		/* tail call to user-defined macro must be treated as non-tail
		 * because the expression returned by the macro
		 * remains to be evaluated in the caller's context */
		unev = NIL;
		goto non_tail_eval;
	    }
	    fun = car(exp);
	    if (fun == Not_Enough_Node) {
		err_logo(TOO_MUCH, NIL);    /* When does this happen? */
		val = UNBOUND;
		goto fetch_cont;
	    }
	    if (flag__caseobj(fun, PROC_SPECFORM)) {
		argl = cdr(exp);
		goto apply_dispatch;
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
eval_arg_loop:
    debprint("eval_arg_loop");
    if (unev == NIL) goto eval_args_done;
    exp = car(unev);
    if (exp == Not_Enough_Node) {
	if (NOT_THROWING)
	    err_logo(NOT_ENOUGH, NIL);
	goto eval_args_done;
    }
arg_from_macro:
    if (nodetype(exp) != CONS) {    /* Don't bother saving registers */
	newcont(after_const_arg);    /* if the exp isn't a proc call */
	goto eval_dispatch;
    }
    eval_save();
    save(current_unode);
    var = var_stack;
    tailcall = -1;
    didnt_output_name = NIL;
    didnt_get_output = cons_list(0, fun, ufun, this_line, END_OF_LIST);
    val_status = VALUE_OK;
	/* in case of apply or catch */
    newcont(accumulate_arg);
    goto eval_dispatch;	    /* evaluate the current argument */

accumulate_arg:
    debprint("accumulate_arg");
    /* Put the evaluated argument into the argl list. */
    reset_args(var);
    restore(current_unode);
    last_call = fun;
    if (current_unode != output_unode) {
	if (STOPPING || RUNNING) output_node = UNBOUND;
	if (stopping_flag == OUTPUT || STOPPING) {
	    stopping_flag = RUN;
	    val = output_node;
	}
    }
    if (stopping_flag == OUTPUT || STOPPING) {
	didnt_output_name = NIL;
	err_logo(DIDNT_OUTPUT, fun);
    }
    while (NOT_THROWING && val == UNBOUND) {
	val = err_logo(DIDNT_OUTPUT, NIL);
    }
    eval_restore();
    if (stopping_flag == MACRO_RETURN) {
	if (val == NIL || val == UNBOUND || cdr(val) != NIL) {
	    if (NOT_THROWING) {
		if (tree_dk_how != NIL && tree_dk_how != UNBOUND)
		    err_logo(DK_HOW_UNREC, tree_dk_how);
		else

		    err_logo((val!=NIL && val!=UNBOUND) ?
				 RUNNABLE_ARG : ERR_MACRO, val);
	    }
	    goto eval_args_done;
	}
	exp = car(val);
	stopping_flag = RUN;
	goto arg_from_macro;
    }
after_const_arg:
    if (stopping_flag == THROWING) goto eval_args_done;
    push(val, argl);
    pop(unev);
    goto eval_arg_loop;

eval_args_done:
    if (stopping_flag == THROWING) {
	val = UNBOUND;
	goto fetch_cont;
    }
    argl = reverse(argl);
/* --------------------- APPLY ---------------------------- */
apply_dispatch:
    debprint("apply_dispatch");
    /* Load in the procedure's definition and decide whether it's a compound
     * procedure or a primitive procedure.
     */
    proc = procnode__caseobj(fun);
    if (is_macro(fun)) {
	num2save(val_status,tailcall);
	save2(didnt_get_output,current_unode);
	didnt_get_output = the_generation; /* (cons nil nil) */
	    /* We want a value, but not as actual arg */
	newcont(macro_return);
    }
    if (proc == UNDEFINED) {	/* 5.0 punctuationless variables */
	if (!varTrue(AllowGetSet)) {    /* No getter/setter allowed, punt */
	    val = err_logo(DK_HOW, fun);
	    goto fetch_cont;
	} else if (argl == NIL) {	/* possible var getter */
	    val = valnode__caseobj(fun);
	    if (val == UNBOUND && NOT_THROWING)
		val = err_logo(DK_HOW, fun);
	    else if (val != UNBOUND) {
		(void)ldefine(cons(fun, cons(
		   cons(NIL,cons(cons(theName(Name_output),
				      cons(make_colon(fun),NIL)),
				 NIL)),
		  NIL)));    /* make real proc so no disk load next time */
		setflag__caseobj(fun,PROC_BURIED);
	    }
	    goto fetch_cont;
	} else {		/* var setter */
	    NODE *name = intern(bf3(fun));
	    if (valnode__caseobj(name) == UNBOUND &&
		!(flag__caseobj(name, (HAS_GLOBAL_VALUE|IS_LOCAL_VALUE)))) {
		    val = err_logo(DK_HOW, fun);
		    goto fetch_cont;
	    }
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
    debprint("primitive_apply");
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
	/* falls into fetch_cont */

#if DEB_CONT
#define do_case(x) case x: debprint("Fetch_cont = " #x); goto x;
#else
#define do_case(x) case x: goto x;
#endif

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


/* ----------------- COMPOUND_APPLY ---------------------------------- */

compound_apply:
    debprint("compound_apply");
#ifdef mac
    check_mac_stop();
#endif
#ifdef ibm
    check_ibm_stop();
#endif
#ifdef HAVE_WX
    check_wx_stop();
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
		    if (flag__caseobj(parm, IS_LOCAL_VALUE))
			settype(var_stack, LOCALSAVE);
		    var_stack->n_obj = valnode__caseobj(parm);
		    setflag__caseobj(parm, IS_LOCAL_VALUE);
		}
		tell_shadow(parm);
		setvalnode__caseobj(parm, arg);
		if (arg == UNBOUND)
		    err_logo(NOT_ENOUGH, fun);
	    } else if (nodetype(parm) == CONS) {
		/* parm is optional or rest */
		if (not_local(car(parm),vsp)) {
		    push(car(parm), var_stack);
		    if (flag__caseobj(car(parm), IS_LOCAL_VALUE))
			settype(var_stack, LOCALSAVE);
		    var_stack->n_obj = valnode__caseobj(car(parm));
		    setflag__caseobj(car(parm), IS_LOCAL_VALUE);
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
		    eval_save();
		    save(current_unode);
		    var = var_stack;
		    tailcall = -1;
		    list = cdr(parm);
		    didnt_get_output = cons_list(0, fun, ufun,
						 list, END_OF_LIST);
		    didnt_output_name = NIL;
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
		    exp = car(unev);
		    pop(unev);
		    if (unev != NIL) {
			err_logo(BAD_DEFAULT, parm);
			val = UNBOUND;
			goto set_args_continue;
		    }
		    newcont(set_args_continue);
		    goto eval_dispatch;

set_args_continue:
		    if (stopping_flag == MACRO_RETURN) {
			if (val == NIL || val == UNBOUND || cdr(val) != NIL) {
			    if (NOT_THROWING)
				err_logo((val!=NIL && val!=UNBOUND) ?
				 RUNNABLE_ARG : ERR_MACRO, val);
			} else {
			    reset_args(var);
			    exp = car(val);
			    stopping_flag = RUN;
			    didnt_get_output = cons_list(0, fun, ufun,
							 list, END_OF_LIST);
			    didnt_output_name = NIL;
			    tailcall = -1;
			    newcont(set_args_continue);
			    goto eval_dispatch;
			}
		    }
		    restore(current_unode);
		    last_call = fun;
		    if (current_unode != output_unode) {
			if (STOPPING || RUNNING) output_node = UNBOUND;
			if (stopping_flag == OUTPUT || STOPPING) {
			    stopping_flag = RUN;
			    val = output_node;
			}
		    }
		    if (stopping_flag == OUTPUT || STOPPING) {
			didnt_output_name = NIL;
			err_logo(DIDNT_OUTPUT, fun);
		    }
		    while (NOT_THROWING && val == UNBOUND) {
			val = err_logo(DIDNT_OUTPUT, NIL);
		    }
		    reset_args(var);
		    eval_restore();
		    parm = car(formals);
		    if (stopping_flag == THROWING) {
			val = UNBOUND;
			goto fetch_cont;
		    }
		    arg = val;
		}
		setvalnode__caseobj(car(parm), arg);
	    }
	    if (argl != NIL) pop(argl);
    }
    if (argl != NIL) {
	err_logo(TOO_MUCH, fun);
    }
    if (check_throwing) {
	val = UNBOUND;
	goto fetch_cont;
    }
    vsp = NIL;
    if ((tracing = !is_list(fun) && flag__caseobj(fun, PROC_TRACED))) {
	if (NOT_THROWING) print_char(writestream, ')');
	new_line(writestream);
	save(fun);
	newcont(compound_apply_continue);
    }
    last_ufun = ufun;
    if (!is_list(fun)) ufun = fun;
    last_line = this_line;
    this_line = NIL;
/*    proc = (is_list(fun) ? anonymous_function(fun) : procnode__caseobj(fun)); */
/*  If that's uncommented, begin_apply must get proc from fun, not exp  */
    list = bodylist__procnode(proc);	/* get the body ... */
    make_tree_from_body(list);
    if (!is_tree(list) || treepair__tree(list)==NIL) {
	val = UNBOUND;
	goto fetch_cont;
    }
    debprint("treeified body");
/*    printf("list = 0x%x = ",list); dbprint(list); */
    unev = tree__tree(list);
    if (NOT_THROWING) stopping_flag = RUN;
    output_node = UNBOUND;
    if (didnt_get_output == UNBOUND)
	val_status = NO_VALUE_OK | STOP_OK | STOP_TAIL;
    else if (didnt_get_output == NIL)
	val_status = NO_VALUE_OK | STOP_OK | STOP_TAIL |
		     OUTPUT_OK | OUTPUT_TAIL;
    else val_status = OUTPUT_OK | OUTPUT_TAIL;
    if (didnt_output_name == NIL) didnt_output_name = fun;
    current_unode = cons(NIL,NIL);  /* a marker for this proc call */

/* ----------------- EVAL_SEQUENCE ---------------------------------- */
/* Fall through from proc body, call from start or fsubr argument */

eval_sequence:
    debprint("eval_sequence");
    /* Evaluate each expression in the sequence.
       Most of the complexity is in recognizing tail calls.
     */
    if (eval_buttonact != NIL) {
	make_tree(eval_buttonact);
	if (NOT_THROWING) {
	    if (is_tree(eval_buttonact)) {
		unev = append(tree__tree(eval_buttonact), unev);
		eval_buttonact = NIL;
	    }
	}
    }
    if (!RUNNING) goto fetch_cont;
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
	if (cadr(exp) == Not_Enough_Node) {
	    err_logo(NOT_ENOUGH,car(exp));
	    val = UNBOUND;
	    goto fetch_cont;
	}
	didnt_output_name = NIL;
	if (val_status & OUTPUT_TAIL) {
	    didnt_get_output = cons_list(0,car(exp),ufun,this_line,
					 END_OF_LIST);
	    fun = car(exp);
	    exp = cadr(exp);
	    val_status = VALUE_OK;
	    goto tail_eval_dispatch;
	} else if (val_status & OUTPUT_OK) {
	    goto tail_eval_dispatch;
	} else if (ufun == NIL) {
	    err_logo(AT_TOPLEVEL,car(exp));
	    val = UNBOUND;
	    goto fetch_cont;
	} else if (val_status & STOP_OK) {
	    didnt_get_output = cons_list(0,car(exp),ufun,this_line,
					 END_OF_LIST);
	    val_status = VALUE_OK;
	    exp = cadr(exp);
	    newcont(op_want_stop);
	    goto eval_dispatch;
op_want_stop:
	    if (NOT_THROWING) err_logo(DK_WHAT_UP, val);
	    goto fetch_cont;
	} else if (val_status & VALUE_OK) {
	    /* pr apply [output ?] [3] */
	    debprint("Op with VALUE_OK");
	    didnt_output_name = fun;
	    goto tail_eval_dispatch;
	} else {
	    debprint("Op with none of the above");
	    goto tail_eval_dispatch;
	}
      } else if (i == STOP_PRIORITY) {
	if (ufun == NIL) {
	    err_logo(AT_TOPLEVEL,car(exp));
	} else if (val_status & STOP_TAIL) {
	} else if (val_status & STOP_OK) {
	    stopping_flag = STOP;
	    output_unode = current_unode;
	} else if (val_status & OUTPUT_OK) {
	    if (NOT_THROWING) {
		if (didnt_get_output == NIL || didnt_get_output == UNBOUND) {
		/*  actually can happen: PRINT FOREACH ...
		    will give didn't output message uplevel  */
		} else
		    err_logo(DIDNT_OUTPUT, NIL);
	    }
	} else {    /* show runresult [stop] inside a procedure */
	    didnt_output_name = car(exp);
	    if (NOT_THROWING) {
		if (didnt_get_output == NIL || didnt_get_output == UNBOUND) {
		/*  actually can happen: STOP during PAUSE */
		    err_logo(AT_TOPLEVEL, car(exp));
		} else
		    err_logo(DIDNT_OUTPUT, NIL);
	    }
	}
	val = UNBOUND;
	goto fetch_cont;
      } else { /* maybeoutput */
	debprint("maybeoutput");
	if (cadr(exp) == Not_Enough_Node) {
	    err_logo(NOT_ENOUGH,car(exp));
	    val = UNBOUND;
	    goto fetch_cont;
	}
	if (ufun == NIL) {
	    err_logo(AT_TOPLEVEL,car(exp));
	    val = UNBOUND;
	    goto fetch_cont;
	}
	if (val_status & OUTPUT_TAIL) {
	    didnt_output_name = NIL;
	    if (val_status & STOP_TAIL) {
		exp = cadr(exp);
		didnt_get_output = NIL;
		val_status = VALUE_OK | NO_VALUE_OK;
	    } else {
		didnt_get_output = cons_list(0,car(exp),ufun,
					     this_line,END_OF_LIST);
		exp = cadr(exp);
		val_status = VALUE_OK;
	    }
	    goto tail_eval_dispatch;
	} else if (val_status & OUTPUT_OK) {
	    didnt_output_name = NIL;
	    if (val_status & STOP_OK) {
		didnt_get_output = NIL;
		val_status = NO_VALUE_OK | VALUE_OK;
		exp = cadr(exp);
		newcont(after_maybeoutput);
		goto eval_dispatch;
after_maybeoutput:
		if (val == UNBOUND)
		    lstop(NIL);
		else
		    loutput(cons(val, NIL));
		goto fetch_cont;
	    } else {
		goto eval_dispatch;
	    }
	} else if (val_status & STOP_TAIL) {
	    exp = cadr(exp);
	    didnt_get_output = UNBOUND;
	    val_status = NO_VALUE_OK;
	    goto tail_eval_dispatch;
	} else if (val_status & STOP_OK) {
	    exp = cadr(exp);
	    didnt_get_output = UNBOUND;
	    val_status = NO_VALUE_OK;
	    newcont(after_maybeoutput);
	    goto eval_dispatch;
	} else {
	    goto tail_eval_dispatch;
	}
      }
    }

    if (unev == NIL) {	/* falling off tail of sequence */
	debprint("falling off");
	if (val_status & NO_VALUE_OK) {
	    if (val_status & VALUE_OK)	/* from runresult */
		didnt_get_output = NIL;
	    else
		didnt_get_output = UNBOUND;
	} else if (val_status & VALUE_OK) {
	} else if (val_status & OUTPUT_OK) {
next_stop_want_output:
	    save(didnt_get_output);
	    didnt_get_output = UNBOUND;
	    val_status &= ~OUTPUT_TAIL;
	    newcont(fall_off_want_output);
	    goto tail_eval_dispatch;
fall_off_want_output:
	    restore(didnt_get_output);
	    if (stopping_flag == OUTPUT) {
		goto fetch_cont;    /* repeat body did output */
	    }
	    if (NOT_THROWING && val != UNBOUND) {
		/* Don't allow just value expr w/o OUTPUT */
		err_logo(DK_WHAT, val);
	    }
	    goto fetch_cont;
	}
	goto tail_eval_dispatch;
    }

    if (car(unev) != NIL && is_list(car(unev)) &&   /* next is STOP */
	  (is_tailform(procnode__caseobj(car(car(unev))))) &&
	  getprimpri(procnode__caseobj(car(car(unev)))) == STOP_PRIORITY) {
	if (val_status & STOP_TAIL) {
	    didnt_get_output = UNBOUND;
	    goto tail_eval_dispatch;
	} else if (val_status & STOP_OK) {
	    goto non_tail_eval;
	} else if (val_status & OUTPUT_OK) {
	    goto next_stop_want_output;
	}   /* else treat as non-tail and the STOP will be caught later */
    }

non_tail_eval:
    debprint("non_tail_eval");
    if (nodetype(exp) != CONS) {    /* Don't bother saving registers */
	newcont(after_constant);    /* if the exp isn't a proc call */
	goto eval_dispatch;
    }
    eval_save();
    didnt_get_output = UNBOUND;    /* tell EVAL we don't want a value */
    tailcall = 0;
    if (nodetype(exp) == CONS && is_prim(procnode__caseobj(car(exp)))) {
	newcont(no_reset_args);	    /* primitive */
    } else {
	var = var_stack;
	newcont(eval_sequence_continue);
    }
    goto eval_dispatch;

eval_sequence_continue:
    reset_args(var);
no_reset_args:	/* allows catch "foo [local ...] to work */
    eval_restore();
    if (dont_fix_ift) {
	ift_iff_flag = dont_fix_ift-1;
	dont_fix_ift = 0;
    }
    debprint("eval_sequence_continue");
    if (stopping_flag == MACRO_RETURN) {
	if (val != NIL && is_list(val) && (isName(car(val), Name_tag)))
	    unev = cdr(val);	/* from goto */
	else
	    unev = append(val, unev);
	val = UNBOUND;
	stopping_flag = RUN;
	if (unev == NIL) goto fetch_cont;
    } else {
	if (current_unode != output_unode) {
	    if (STOPPING || RUNNING) output_node = UNBOUND;
	    if (stopping_flag == OUTPUT || STOPPING) {
		stopping_flag = RUN;
		val = output_node;
		goto fetch_cont;
	    }
	}
    }
after_constant:
    if (val != UNBOUND && NOT_THROWING) {
	err_logo(DK_WHAT, val);
	val = UNBOUND;
    }
    if (NOT_THROWING && unev == NIL) {
	goto fetch_cont;
    }
    goto eval_sequence;

compound_apply_continue:
    /* Only get here if tracing */
    restore(parm);  /* saved from fun */
    --trace_level;
    if (NOT_THROWING) {
	for (i = 0; i < trace_level; i++) print_space(writestream);
	print_node(writestream, parm);
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
    restore2(didnt_get_output,current_unode);
    num2restore(val_status,tailcall);
    debprint("macro_return");
    if (current_unode != output_unode) {
	if (STOPPING || RUNNING) output_node = UNBOUND;
	if (stopping_flag == OUTPUT || STOPPING) {
	    stopping_flag = RUN;
	    val = output_node;
	}
    }
    while (!is_list(val) && NOT_THROWING) {
	val = err_logo(ERR_MACRO,val);
    }
    if (NOT_THROWING) {
	if (didnt_get_output != UNBOUND)
	    didnt_output_name = fun;
	if (is_cont(val)) {
	    newcont(cont__cont(val));
	    val = val__cont(val);
	    goto fetch_cont;
	}
	if (tailcall <= 0) {
	    list = val;
	    make_tree(list);
	    if (NOT_THROWING) {
		stopping_flag = MACRO_RETURN;
		if (!is_tree(list)) val = NIL;
		else val = tree__tree(list);
	    } else val = UNBOUND;
	    goto fetch_cont;
	}
	list = val;
	goto begin_seq;
    }
    val = UNBOUND;
    goto fetch_cont;

#define RUNRESULT_OUTPUT_LEGAL 0

runresult_continuation:
    list = val;
#if RUNRESULT_OUTPUT_LEGAL
    val_status |= VALUE_OK | NO_VALUE_OK;
    val_status &= ~(STOP_TAIL | OUTPUT_TAIL);
#else
    val_status = VALUE_OK | NO_VALUE_OK | OUTPUT_OK | STOP_OK;
    /* output and stop are not okay, but we give our own err message */
#endif
    save(current_unode);
    newcont(runresult_followup);
    goto begin_seq;

runresult_followup:
    restore(current_unode);
    debprint("runresult_followup");
    if (current_unode != output_unode) {
	if (STOPPING || RUNNING) output_node = UNBOUND;
	if (stopping_flag == OUTPUT || STOPPING) {
	    stopping_flag = RUN;
	    val = output_node;
	}
    }
    if (STOPPING || stopping_flag == OUTPUT)
	err_logo(RUNRES_STOP, NIL);
    if (val == UNBOUND) {
	val = NIL;
    } else {
	val = cons(val, NIL);
    }
    goto fetch_cont;

repeat_continuation:
    list = cdr(val);
    num2save(repcount,user_repcount);
    repcount = getint(car(val));
    user_repcount = 0;
repeat_again:
    val = UNBOUND;
    if (repcount == 0) {
repeat_done:
	num2restore(repcount,user_repcount);
	goto fetch_cont;
    }
    user_repcount++;
    save2(list,var);
    var = var_stack;
    num2save(repcount,user_repcount);
    num2save(val_status,tailcall);
    val_status &= ~(VALUE_OK|OUTPUT_TAIL|STOP_TAIL);
    if (tailcall == 0) val_status |= NO_VALUE_OK;   /* embedded repeat */
    newcont(repeat_followup);
    goto begin_seq;

repeat_followup:
    if (val != UNBOUND && NOT_THROWING) {
	err_logo(DK_WHAT, val);
    }
    num2restore(val_status,tailcall);
    num2restore(repcount,user_repcount);
    reset_args(var);
    restore2(list,var);
    if (current_unode != output_unode) {
	debprint("rep_foll tailcall");
	if (STOPPING || RUNNING) output_node = UNBOUND;
	if (stopping_flag == OUTPUT || STOPPING) {
	    stopping_flag = RUN;
	    val = output_node;
	    goto repeat_done;
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
    goto repeat_done;

catch_continuation:
    list = cdr(val);
    catch_tag = car(val);
    if (isName(catch_tag, Name_error)) {
	push(Erract, var_stack);
	if (flag__caseobj(Erract, IS_LOCAL_VALUE))
	    settype(var_stack, LOCALSAVE);
	var_stack->n_obj = valnode__caseobj(Erract);
	setflag__caseobj(Erract, IS_LOCAL_VALUE);
	setvalnode__caseobj(Erract, UNBOUND);
    }
    save2(didnt_output_name,didnt_get_output);
    num2save(val_status,tailcall);
    save2(current_unode,catch_tag);
    newcont(catch_followup);
    val_status &= ~(STOP_TAIL | OUTPUT_TAIL);
    goto begin_seq;

catch_followup:
    restore2(current_unode,catch_tag);
    num2restore(val_status,tailcall);
    restore2(didnt_output_name,didnt_get_output);
    if (current_unode != output_unode) {
	if (STOPPING || RUNNING) output_node = UNBOUND;
	if (stopping_flag == OUTPUT || STOPPING) {
	    stopping_flag = RUN;
	    val = output_node;
	    goto fetch_cont;
	}
    }
    if (NOT_THROWING && val != UNBOUND && !(val_status & VALUE_OK))
	err_logo(DK_WHAT, val);
    if (stopping_flag == THROWING &&
	((compare_node(throw_node, catch_tag, TRUE) == 0) ||
	 (isName(throw_node, Name_error) && isName(catch_tag, Name_error)))) {
	    throw_node = UNBOUND;
	    stopping_flag = RUN;
	    val = output_node;
    }
    goto fetch_cont;

#ifdef OBJECTS

withobject_continuation:
    save2(didnt_output_name,didnt_get_output);
    num2save(val_status,tailcall);
    save2(current_unode,current_object);
    newcont(withobject_followup);
    current_object = car(val);
    newcont(cont__cont(cdr(val)));
    list = val = val__cont(cdr(val));
    val_status &= ~(STOP_TAIL | OUTPUT_TAIL);
    goto fetch_cont;

withobject_followup:
    restore2(current_unode,current_object);
    num2restore(val_status,tailcall);
    restore2(didnt_output_name,didnt_get_output);
    if (current_unode != output_unode) {
	if (STOPPING || RUNNING) output_node = UNBOUND;
	if (stopping_flag == OUTPUT || STOPPING) {
	    stopping_flag = RUN;
	    val = output_node;
	    goto fetch_cont;
	}
    }
    if (NOT_THROWING && val != UNBOUND && !(val_status & VALUE_OK))
	err_logo(DK_WHAT, val);
    goto fetch_cont;

#endif /* OBJECTS */

goto_continuation:
    if (NOT_THROWING) {
	if (ufun == NIL) {
	    err_logo(AT_TOPLEVEL, theName(Name_goto));
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
		    (isName(car(exp), Name_tag)) &&
		    (nodetype(cadr(exp)) == QUOTE) &&
		    compare_node(val, node__quote(cadr(exp)), TRUE) == 0) {
		val = cons(theName(Name_tag), unev);
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
    exp = car(val);
    while (nodetype(exp) == ARRAY && NOT_THROWING)
	exp = err_logo(APPLY_BAD_DATA, exp);
    argl = append(cadr(val), NIL);
    val = UNBOUND;
    while (!is_list(argl) && NOT_THROWING)
	argl = err_logo(APPLY_BAD_DATA, argl);
    if (NOT_THROWING && exp != NIL) {
	if (is_list(exp)) {		    /* template */
	    if (is_list(car(exp)) && cdr(exp) != NIL) {
		if (is_list(cadr(exp))) {
		    /* procedure text form [[param ...] [instr ...] ...] */
		    proc = anonymous_function(exp);
		    debprint("anon func");
		    if (stopping_flag == THROWING) goto fetch_cont;
		    tracing = 0;
		    if (tailcall <= 0) {
			save(var);
			var = var_stack;
			newcont(after_lambda);
		    }
		    goto lambda_apply;
		}
		/* lambda form [[param ...] instr ...] */
		formals = car(exp);
		if (tailcall <= 0) {
		    save(var);
		    var = var_stack;
		    newcont(after_lambda);
		}
/*		numsave(tailcall);  */
		tailcall = 0;
		llocal(formals);    /* bind the formals locally */
/*		numrestore(tailcall);	*/
		for ( ;
		      formals != NIL && argl != NIL && NOT_THROWING;
		      formals = cdr(formals),
		      argl = cdr(argl))
		    setvalnode__caseobj(car(formals), car(argl));
		if (formals != NIL) {
		    err_logo(NOT_ENOUGH, exp);
		    goto fetch_cont;
		} else if (argl != NIL) {
		    err_logo(DK_WHAT, car(argl));
		    goto fetch_cont;
		}
		list = cdr(exp);
		goto lambda_qm;
	    } else {		/* question-mark form [instr ...] */
		qm_list = argl;
		list = exp;
lambda_qm:
		make_tree(list);
		if (list == NIL || !is_tree(list)) {
		    goto fetch_cont;
		}
		unev = tree__tree(list);
		if (tailcall <= 0) {
		    val_status &= ~(STOP_TAIL | OUTPUT_TAIL);
			save(var);
			var = var_stack;
			newcont(after_lambda);
		}
		goto eval_sequence;
	    }
	} else {    /* name of procedure to apply */
	    int min, max, n;
	    NODE *arg;
	    fun = intern(exp);
	    check_library(fun);
	    proc = procnode__caseobj(fun);
	    while (proc == UNDEFINED && NOT_THROWING) {
		val = err_logo(DK_HOW_UNREC, fun);
	    }
	    if (NOT_THROWING) {
		if (nodetype(proc) == CONS) {
		    min = getint(minargs__procnode(proc));
		    max = getint(maxargs__procnode(proc));
		} else {
		    if (getprimdflt(proc) < 0) {        /* special form */
			err_logo(DK_HOW_UNREC, fun);    /* can't apply */
			goto fetch_cont;
		    } else {
			min = getprimmin(proc);
			if (min == OK_NO_ARG) min = 0;
			max = getprimmax(proc);
		    }
		}
		for (n = 0, arg = argl; arg != NIL; n++, arg = cdr(arg));
		if (n < min) {
		    err_logo(NOT_ENOUGH, NIL);
		} else if (n > max && max >= 0) {
		    err_logo(TOO_MUCH, fun);
		} else {
		    if (tailcall <= 0) {
			save(var);
			var = var_stack;
			newcont(after_lambda);
		    }
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
}
