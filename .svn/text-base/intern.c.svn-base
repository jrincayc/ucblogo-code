/*
 *      intern.c        logo data interning module              dvb
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

NODE *hash_table[HASH_LEN] = {NIL};

void map_oblist(void (*fcn)()) {
    int i;
    NODE *nd;

    for (i = 0; i < HASH_LEN; i++)
	for (nd = hash_table[i]; nd != NIL; nd = cdr(nd))
	    (*fcn) (car(nd));
}

FIXNUM hash(char *s, int len) {
    /* Map S to an integer in the range 0 .. HASH_LEN-1. */
    /* Method attributed to Peter Weinberger, adapted from Aho, Sethi, */
    /* and Ullman's book, Compilers: Principles, Techniques, and */
    /* Tools; figure 7.35. */

    unsigned FIXNUM h = 0, g;

    while (--len >= 0) {
	h = (h << 4) + (FIXNUM)(*s++);
	g = h & (0xf << (WORDSIZE-4));
	if (g != 0) {
	    h ^= g ^ (g >> (WORDSIZE-8));
	}
    }
    return h % HASH_LEN;
}

NODE *make_case(NODE *casestrnd, NODE *obj) {
    NODE *new_caseobj, *clistptr;

    clistptr = caselistptr__object(obj);
    new_caseobj = make_caseobj(casestrnd, obj);
    setcdr(clistptr, cons(new_caseobj, cdr(clistptr)));
    return(new_caseobj);
}

NODE *make_object(NODE *canonical, NODE *oproc, NODE *val,
		  NODE *plist, NODE *casestrnd) {
    NODE *temp;

    temp = cons_list(0, canonical, oproc, val, plist,
		     make_intnode((FIXNUM)0), END_OF_LIST);
    make_case(casestrnd, temp);
    return(temp);
}

NODE *make_instance(NODE *casend, NODE *lownd) {
    NODE *obj;
    FIXNUM hashind;

    /* Called only if arg isn't already in hash table */

    obj = make_object(lownd, UNDEFINED, UNBOUND, NIL, casend);
    hashind = hash(getstrptr(lownd), getstrlen(lownd));
    push(obj,(hash_table[hashind]));
    return car(caselist__object(obj));
}

NODE *find_instance(NODE *lownd) {
    NODE *hash_entry, *thisobj = NIL;
    int cmpresult;

    hash_entry = hash_table[hash(getstrptr(lownd), getstrlen(lownd))];

    while (hash_entry != NIL) {
	thisobj = car(hash_entry);
	cmpresult = compare_node(lownd, canonical__object(thisobj), FALSE);
	if (cmpresult == 0)
	    break;
	else
	    hash_entry = cdr(hash_entry);
    }
    if (hash_entry == NIL) return(NIL);
    else return(thisobj);
}

int case_compare(NODE *nd1, NODE *nd2) {
    if (backslashed(nd1) && backslashed(nd2)) {
	if (getstrlen(nd1) != getstrlen(nd2)) return(1);
	return(strncmp(getstrptr(nd1), getstrptr(nd2),
		       getstrlen(nd1)));
    }
    if (backslashed(nd1) || backslashed(nd2))
	return(1);
    return(compare_node(nd1, nd2, FALSE));
}

NODE *find_case(NODE *strnd, NODE *obj) {
    NODE *clist;

    clist = caselist__object(obj);
    while (clist != NIL &&
	    case_compare(strnd, strnode__caseobj(car(clist))))
	clist = cdr(clist);
    if (clist == NIL) return(NIL);
    else return(car(clist));
}

NODE *intern(NODE *nd) {
    NODE *obj, *casedes, *lownd;

    if (nodetype(nd) == CASEOBJ) return(nd);
    nd = cnv_node_to_strnode(nd);
    lownd = make_strnode(getstrptr(nd), (struct string_block *)NULL,
			 getstrlen(nd), STRING, noparitylow_strnzcpy);
    if ((obj = find_instance(lownd)) != NIL) {
	if ((casedes = find_case(nd, obj)) == NIL)
	    casedes = make_case(nd, obj);
    } else
	casedes = make_instance(nd, lownd);
    return(casedes);
}
