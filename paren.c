/*
 *      paren.c		    logo parenthesizing module		dko
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
#include <ctype.h>

#ifdef OBJECTS
#undef procnode__caseobj
#define procnode__caseobj procValue
#endif

NODE *the_generation;

void check_library(NODE *first) {
    if (procnode__caseobj(first) == UNDEFINED && NOT_THROWING &&
	first != Null_Word)
	    silent_load(first, NULL);    /* try ./<first>.lg */
    if (procnode__caseobj(first) == UNDEFINED && NOT_THROWING &&
	first != Null_Word)
	    silent_load(first, logolib); /* try <logolib>/<first> */
}

/* Set the line pointer for a tree.
 */ 
void make_line(NODE *tree, NODE *line) {
    setobject(tree, line);
    settype(tree, LINE);
}

void untreeify(NODE *node) {
    if (node==NIL) return;
    settreepair__tree(node, NIL);
    settype(node, CONS);
}

void untreeify_line(NODE *line) {
    if (line != NIL && is_list(line)) {
	untreeify_line(car(line));
	untreeify_line(cdr(line));
	untreeify(line);
    }
}

void untreeify_body(NODE *body) {
    NODE *body_ptr;

    for (body_ptr = body; body_ptr != NIL; body_ptr = cdr(body_ptr)) {
	untreeify_line(car(body_ptr));
    }
    untreeify(body);
}

void untreeify_proc(NODE *tproc) {
    untreeify_body(bodylist__procnode(tproc));
}

/* Treeify a body by appending the trees of the lines.
 */ 
void make_tree_from_body(NODE *body) {

    NODE *body_ptr, *end_ptr = NIL, *tree = NIL;

    if (body == NIL ||
	(is_tree(body) && generation__tree(body) == the_generation))
	    return;
    if (is_tree(body)) untreeify_body(body);
    for (body_ptr = body; body_ptr != NIL; body_ptr = cdr(body_ptr)) {
	tree = car(body_ptr);
	if (tree == NIL) continue;  /* skip blank line */
	this_line = tree;
	make_tree(tree);
	if (is_tree(tree)) {
	    tree = tree__tree(tree);
	    make_line(tree, car(body_ptr));
	    if (end_ptr == NIL)
		settree__tree(body, tree);
	    else
		setcdr(end_ptr, tree);
	    if (generation__tree(car(body_ptr)) == UNBOUND)
		setgeneration__tree(body, UNBOUND);
/*	    untreeify(car(body_ptr));	*/
	    while (cdr(tree) != NIL)
		tree = cdr(tree);
	    end_ptr = tree;
	} else {    /* error while treeifying */
	    untreeify(body);
	    return;
	}
    }
    settype(body, TREE);
}

NODE *tree_dk_how;

/* Treeify a list of tokens (runparsed or not).
 */ 
void make_tree(NODE *list) {

    NODE *tree = NIL;
    NODE *paren_line(NODE *);

    if (list == NIL ||
	(is_tree(list) && generation__tree(list) == the_generation))
	    return;
    if (!runparsed(list)) make_runparse(list);
    tree_dk_how = NIL;
    tree = paren_line(parsed__runparse(list));
    if (tree != NIL && tree != UNBOUND) {
	settype(list, TREE);
	settree__tree(list, tree);
	if (tree_dk_how != NIL || stopping_flag==THROWING)
	    setgeneration__tree(list, UNBOUND);
    }
}

NODE *gather_args(NODE *, NODE *, NODE **, BOOLEAN, NODE **);
NODE *paren_infix(NODE *, NODE **, int, BOOLEAN);
NODE *gather_some_args(int, int, NODE **, BOOLEAN, NODE **);

/* Fully parenthesize a complete line, i.e. transform it from a flat list
 * to a tree.
 */ 
NODE *paren_line(NODE *line) {

    NODE *retval = NIL;
    NODE *paren_expr(NODE **expr, BOOLEAN inparen);
    NODE *paren_infix(NODE *left, NODE **rest, int old_pri, BOOLEAN inparen);

    if (line == NIL) return line;
    retval = paren_expr(&line, FALSE);
    if (NOT_THROWING && retval != UNBOUND) {
	retval = paren_infix(retval, &line, -1, FALSE);
	retval = cons(retval, paren_line(line));
    }
    return retval;
}

/* See if an unknown procedure name starts with SET */
int is_setter(NODE *name) {
    NODE *string = cnv_node_to_strnode(name);

    if (getstrlen(string) < 4) return FALSE;
    return !low_strncmp(getstrptr(string), "set", 3);
}

/* Check for FD100, give warning, insert space */

NODE *missing_alphabetic, *missing_numeric;

int missing_space(NODE *name) {
    NODE *str = strnode__caseobj(name);
    char *s = getstrptr(str);
    FIXNUM len = getstrlen(str);
    char *t;
    char ch;
    char alpha[100], numer[100];
    int i;
    NODE *first;

    t = s+len-1;
    ch = *t;
    if (!isdigit(ch)) return 0;
    i = 1;
    while ((t>s) && (isdigit(*--t))) i++;
    if (t<=s) return 0;
    strncpy(numer,t+1,i);
    numer[i] = '\0';
    strncpy(alpha,s,len-i);
    alpha[len-i] = '\0';
    first = intern(make_strnode(alpha, 0, len-i, STRING, strnzcpy));
    check_library(first);
    if (procnode__caseobj(first) == UNDEFINED) return 0;
    missing_alphabetic = first;
    missing_numeric = make_intnode(atoi(numer));
    err_logo(MISSING_SPACE,
	     cons_list(0, cons_list(0, missing_alphabetic, missing_numeric,
				    END_OF_LIST),
		          name, END_OF_LIST));
    return 1;
}

/* Parenthesize an expression.  Set expr to the node after the first full
 * expression.
 */ 
NODE *paren_expr(NODE **expr, BOOLEAN inparen) {

    NODE *first = NIL, *tree = NIL, *pproc, *retval;
    NODE **ifnode = (NODE **)NIL;

    if (*expr == NIL) {
	if (inparen) err_logo(PAREN_MISMATCH, NIL);
	return *expr;
    }
    first = car(*expr);
    pop(*expr);
    if (nodetype(first) == CASEOBJ && !numberp(first)) {
	if (first == Left_Paren) {
	    tree = paren_expr(expr, TRUE);
	    tree = paren_infix(tree, expr, -1, TRUE);
	    if (*expr == NIL)
		err_logo(PAREN_MISMATCH, NIL);
	    else if (car(*expr) != Right_Paren) {   /* throw the rest away */
		int parens;

		for (parens = 0; *expr; pop(*expr)) {
		    if (car(*expr) == Left_Paren)
			parens++;
		    else if (car(*expr) == Right_Paren)
			if (parens-- == 0) {
			    pop(*expr);
			    break;
			}
		}
		first = tree /* car(tree) */ ;  /* 6.0 */
		tree = cons(Not_Enough_Node, NIL);  /* tell eval */
		tree_dk_how=UNBOUND;
		if (is_list(first))
		    first = car(first);
		if (nodetype(first) != CASEOBJ ||
		    procnode__caseobj(first) == UNDEFINED)
			err_logo(DK_HOW, first);
		else
		    err_logo(TOO_MUCH, first);
	    }
	    else
		pop(*expr);
	    retval = tree;
	} else if (first == Right_Paren) {
	    err_logo(UNEXPECTED_PAREN, NIL);
	    if (inparen) push(first, *expr);
	    retval = NIL;
	} else if (first == Minus_Sign) {
	    push(Minus_Tight, *expr);
	    retval = paren_infix(make_intnode((FIXNUM) 0), expr, -1, inparen);
	} else {	/* it must be a procedure */
	    check_library(first);
	    pproc = procnode__caseobj(first);
	    if (pproc == UNDEFINED) {
		if (missing_space(first)) {
		    push(missing_numeric, *expr);
		    first = missing_alphabetic;
		    pproc = procnode__caseobj(first);
		    retval = gather_args(first, pproc, expr, inparen, ifnode);
		    if (retval != UNBOUND) {
			retval = cons(first, retval);
		    }
		} else if (is_setter(first)) {
		    retval = gather_some_args(0, 1, expr, inparen, ifnode);
		    if (retval != UNBOUND) {
			retval = cons(first, retval);
		    }
		} else {
		    retval = cons(first, NIL);
		    tree_dk_how = first;
		}
	    } else if (nodetype(pproc) == INFIX && NOT_THROWING) {
		err_logo(NOT_ENOUGH, first);
		retval = cons(first, NIL);
	    } else {
		/* Kludge follows to turn IF to IFELSE sometimes. */
		if (isName(first, Name_if)) {
		    ifnode = &first;
		}
		retval = gather_args(first, pproc, expr, inparen, ifnode);
		if (retval != UNBOUND) {
		    retval = cons(first, retval);
		}
	    }
	}
    } else if (is_list(first)) {   /* quoted list */
	retval = make_quote(first);
    } else {
	return first;
    }
    return retval;
}

/* Gather the correct number of arguments to proc into a list.  Set args to
 * immediately after the last arg.
 */ 
NODE *gather_args(NODE *newfun, NODE *pproc, NODE **args, BOOLEAN inparen,
		  NODE **ifnode) {
    int min, max;
    NODE /* *oldfun, */ *result;
    
    if (nodetype(pproc) == CONS) {
	min = (inparen ? getint(minargs__procnode(pproc))
		       : getint(dfltargs__procnode(pproc)));
	max = (inparen ? getint(maxargs__procnode(pproc))
		       : getint(dfltargs__procnode(pproc)));
    } else { /* primitive */
	min = (inparen ? getprimmin(pproc) : getprimdflt(pproc));
	if (min < 0) {	    /* special form */
	    result = *args;
	    *args = NIL;
	    return result;
/*
	    oldfun = fun;
	    fun = newfun;
	    result = (*getprimfun(pproc))(*args);
	    fun = oldfun;
	    return result;
 */
	}
    /* Kludge follows to allow EDIT and CO without input without paren */ 
	if (getprimmin(pproc) == OK_NO_ARG) min = 0;
	max = (inparen ? getprimmax(pproc) : getprimdflt(pproc));
    }
    return gather_some_args(min, max, args, inparen, ifnode);
}

/* Make a list of the next n expressions, where n is between min and max.
 * Set args to immediately after the last expression.
 */ 
NODE *gather_some_args(int min, int max, NODE **args, BOOLEAN inparen,
		       NODE **ifnode) {
    NODE *paren_infix(NODE *left, NODE **rest, int old_pri, BOOLEAN inparen);

    if (*args == NIL || car(*args) == Right_Paren ||
	    (nodetype(car(*args)) == CASEOBJ &&
	     nodetype(procnode__caseobj(car(*args))) == INFIX)) {
	if (min > 0) return cons(Not_Enough_Node, NIL);
    } else if (max == 0) {
	if (ifnode != (NODE **)NIL && is_list(car(*args))) {
	    /* if -> ifelse kludge */
	    NODE *retval;
	    err_logo(IF_WARNING, NIL);
	    *ifnode = theName(Name_ifelse);
	    retval = paren_expr(args, FALSE);
	    retval = paren_infix(retval, args, -1, inparen);
	    return cons(retval, gather_some_args(min, max, args,
						 inparen, (NODE **)NIL));
	}
    } else {
	if (max < 0) max = 0;   /* negative max means unlimited */
	if (car(*args) != Right_Paren &&
		(nodetype(car(*args)) != CASEOBJ ||
		 nodetype(procnode__caseobj(car(*args))) != INFIX)) {
	    NODE *retval = paren_expr(args, FALSE);
	    retval = paren_infix(retval, args, -1, inparen);
	    return cons(retval, gather_some_args(min - 1, max - 1, args,
						 inparen, ifnode));
	}
    }
    return NIL;
}

/* Calculate the priority of a procedure.
 */ 
int priority(NODE *proc_obj) {

    NODE *pproc;

    if (proc_obj == Minus_Tight) return PREFIX_PRIORITY+4;
    if (nodetype(proc_obj) != CASEOBJ ||
	(pproc = procnode__caseobj(proc_obj)) == UNDEFINED ||
	nodetype(pproc) != INFIX)
	    return 0;
    return getprimpri(pproc);
}

/* Parenthesize an infix expression.  left_arg is the expression on the left
 * (already parenthesized), and rest is a pointer to the list starting with the
 * infix procedure, if it's there.  Set rest to after the right end of the
 * infix expression.
 */ 
NODE *paren_infix(NODE *left_arg, NODE **rest, int old_pri, BOOLEAN inparen) {

    NODE *infix_proc, *retval;
    int pri;

    if (*rest == NIL || !(pri = priority(infix_proc = car(*rest)))
		     || pri <= old_pri) 
	return left_arg;
    pop(*rest);
    retval = paren_expr(rest, inparen);
    retval = paren_infix(retval, rest, pri, inparen);
    retval = cons_list(0,infix_proc, left_arg, retval, END_OF_LIST);
    return paren_infix(retval, rest, old_pri, inparen);
}
