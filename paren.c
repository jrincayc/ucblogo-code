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

#include "logo.h"
#include "globals.h"

NODE *the_generation;

/* Set the line pointer for a tree.
 */ 
void make_line(NODE *tree, NODE *line) {
    setobject(tree, line);
    settype(tree, LINE);
}

void untreeify(NODE *node) {
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

void untreeify_proc(NODE *proc) {

    NODE *body = bodylist__procnode(proc);
    NODE *body_ptr;

    for (body_ptr = body; body_ptr != NIL; body_ptr = cdr(body_ptr)) {
	untreeify_line(car(body_ptr));
    }
    untreeify(body);
}

/* Treeify a body by appending the trees of the lines.
 */ 
void make_tree_from_body(NODE *body) {

    NODE *body_ptr, *end_ptr = NIL, *tree = NIL;

    if (body == NIL ||
	(is_tree(body) && generation__tree(body) == the_generation))
	    return;
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
/*	    untreeify(car(body_ptr));	*/   /* from mac, why? */
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

BOOLEAN tree_dk_how;

/* Treeify a list of tokens (runparsed or not).
 */ 
void make_tree(NODE *list) {

    NODE *tree = NIL;
    NODE *paren_line(NODE *);

    if (list == NIL ||
	(is_tree(list) && generation__tree(list) == the_generation))
	    return;
    if (!runparsed(list)) make_runparse(list);
    tree_dk_how = FALSE;
    tree = paren_line(parsed__runparse(list));
    if (tree != NIL && tree != UNBOUND) {
	settype(list, TREE);
	settree__tree(list, tree);
	if (tree_dk_how || stopping_flag==THROWING)
	    setgeneration__tree(list, UNBOUND);
    }
}


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

/* Parenthesize an expression.  Set expr to the node after the first full
 * expression.
 */ 
NODE *paren_expr(NODE **expr, BOOLEAN inparen) {

    NODE *first = NIL, *tree = NIL, *proc, *retval;
    NODE **ifnode = (NODE **)NIL;
    NODE *gather_args(NODE *, NODE **, BOOLEAN, NODE **);
    NODE *paren_infix(NODE *, NODE **, int, BOOLEAN);

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
	    else if (car(*expr) != Right_Paren)
	    {
		int parens;

		if (NOT_THROWING) err_logo(TOO_MUCH, NIL);	/* throw the rest away */
		for (parens = 0; *expr; pop(*expr))
		    if (car(*expr) == Left_Paren)
			parens++;
		    else if (car(*expr) == Right_Paren)
			if (parens-- == 0) break;
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
	    if (procnode__caseobj(first) == UNDEFINED && NOT_THROWING &&
		first != Null_Word)
		    silent_load(first, NULL);    /* try ./<first>.lg */
	    if (procnode__caseobj(first) == UNDEFINED && NOT_THROWING &&
		first != Null_Word)
		    silent_load(first, logolib); /* try <logolib>/<first> */
	    proc = procnode__caseobj(first);
	    if (proc == UNDEFINED && NOT_THROWING) {
		retval = cons(first, NIL);
		tree_dk_how = TRUE;
	    } else if (nodetype(proc) == INFIX && NOT_THROWING) {
		err_logo(NOT_ENOUGH, first);
		retval = cons(first, NIL);
	    } else {
		/* Kludge follows to turn IF to IFELSE sometimes. */
		if (first == If) {
		    ifnode = &first;
		}
		retval = gather_args(proc, expr, inparen, ifnode);
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
NODE *gather_args(NODE *proc, NODE **args, BOOLEAN inparen, NODE **ifnode) {

    int min, max;
    NODE *gather_some_args(int, int, NODE **, BOOLEAN, NODE **);
    
    if (nodetype(proc) == CONS) {
	min = (inparen ? getint(minargs__procnode(proc))
		       : getint(dfltargs__procnode(proc)));
	max = (inparen ? getint(maxargs__procnode(proc))
		       : getint(dfltargs__procnode(proc)));
    } else { /* primitive */
	min = (inparen ? getprimmin(proc) : getprimdflt(proc));
	if (min < 0) {	    /* special form */
	    return (*getprimfun(proc))(*args);
	}
    /* Kludge follows to allow EDIT and CO without input without paren */ 
	if (getprimmin(proc) == OK_NO_ARG) min = 0;
	max = (inparen ? getprimmax(proc) : getprimdflt(proc));
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
	    *ifnode = Ifelse;
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

    NODE *proc;

    if (proc_obj == Minus_Tight) return PREFIX_PRIORITY+4;
    if (nodetype(proc_obj) != CASEOBJ ||
	(proc = procnode__caseobj(proc_obj)) == UNDEFINED ||
	nodetype(proc) != INFIX)
	    return 0;
    return getprimpri(proc);
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
