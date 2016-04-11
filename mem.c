/*
 *      mem.c           logo memory management module           dvb 6/28/88
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
 */

#include "logo.h"
#include "globals.h"
#ifdef ibm
#ifndef __ZTC__
#include <alloc.h>
#endif
#endif

NODE *free_list = NIL;                /* global ptr to free node list */
struct segment *segment_list = NULL;  /* global ptr to segment list */

#ifdef PUNY
#define GCMAX 1000
#else
#ifdef THINK_C
#define GCMAX 8000
#else
#ifdef __ZTC__
#define GCMAX 6000
#else
#define GCMAX 16000
#endif
#endif
#endif

#ifdef THINK_C
extern NODE *gcstack[];
#else
NODE *gcstack[GCMAX];
#endif

#ifdef MEM_DEBUG
long int mem_allocated = 0, mem_freed = 0;
#endif

NODE **gctop = gcstack;

#ifdef MEM_DEBUG
#define MAX_RECORD_ALLOC 10000

NODE *allocated[MAX_RECORD_ALLOC];
int allocated_id[MAX_RECORD_ALLOC];
int current_allocated_id;

void record_alloced_pointer(NODE *nd)
{
  int i;
  if (nd == NIL) return;
  for (i= 0; i< MAX_RECORD_ALLOC; i++) {
    if (!allocated[i]) {
      allocated[i]= nd;
      allocated_id[i]= ++current_allocated_id;
      break;
    }
  }
}

void record_freed_pointer(NODE *nd)
{
  int i;
  if (nd == NIL) return;
  for (i= 0; i< MAX_RECORD_ALLOC; i++) {
    if (allocated[i] == nd) {
      allocated[i]= 0;
      break;
    }
  }
}

void clear_alloced_records(NODE *nd)
{
  bzero(allocated, sizeof(allocated));
}

void print_alloced_pointers(void)
{
  int i;
  for (i= 0; i< MAX_RECORD_ALLOC; i++) {
    if (allocated[i]) {
      fprintf(stderr, "%d: ", allocated_id[i]);
      ndprintf(stderr, "%s\n", allocated[i]);
    }
  }
}
#endif

NODETYPES nodetype(NODE *nd)
{
    if (nd == NIL) return (PNIL);
    return((NODETYPES)nd->node_type);
}

void setobject(NODE *nd, NODE *newobj)
{
    NODE *oldobj = getobject(nd);

    if (newobj != NIL) increfcnt(newobj);
    if (oldobj != NIL && decrefcnt(oldobj) == 0)
	gc(oldobj);
    nd->n_obj = newobj;
}

void setcar(NODE *nd, NODE *newcar)
{
    NODE *oldcar = car(nd);

    if (newcar != NIL) increfcnt(newcar);
    if (oldcar != NIL && decrefcnt(oldcar) == 0)
	gc(oldcar);
    nd->n_car = newcar;
}

void setcdr(NODE *nd, NODE *newcdr)
{
    NODE *oldcdr = cdr(nd);

    if (newcdr != NIL) increfcnt(newcdr);
    if (oldcdr != NIL && decrefcnt(oldcdr) == 0)
	gc(oldcdr);
    nd->n_cdr = newcdr;
}

NODE *_reref(NODE *proc_var, NODE *newval)
{
    if (newval != NIL) increfcnt(newval);
    if (proc_var != NIL && decrefcnt(proc_var) == 0)
	gc(proc_var);
    return(newval);
}

NODE *unref(NODE *ret_var)
{
    if (ret_var != NIL) decrefcnt(ret_var);
    return(ret_var);
}

void addseg()
{
    int p;
    struct segment *newseg;

    if ((newseg = (struct segment *) malloc((size_t)sizeof(struct segment)))
	    != NULL) {
	newseg->next = segment_list;
	segment_list = newseg;
	for (p = 0; p < SEG_SIZE; p++) {
	    newseg->nodes[p].n_cdr = free_list;
#ifdef MEM_DEBUG
	    settype(&(newseg->nodes[p]), NTFREE);
#endif
	    free_list = &newseg->nodes[p];
	}
    }
}

NODE *newnode(NODETYPES type)
{
    NODE *newnd;

    if ((newnd = free_list) == NIL) {
	addseg();
	if ((newnd = free_list) == NIL) {
	    err_logo(OUT_OF_MEM, NIL);
	    if ((newnd = free_list) == NIL)
		err_logo(OUT_OF_MEM_UNREC, NIL);
	}
    }
    free_list = cdr(newnd);
    settype(newnd, type);
    setrefcnt(newnd, 0);
    newnd->n_car = NIL;
    newnd->n_cdr = NIL;
    newnd->n_obj = NIL;
#ifdef MEM_DEBUG
    mem_allocated++;
#endif
    return(newnd);
}

NODE *cons(NODE *x, NODE *y)
{
    NODE *val = newnode(CONS);

    setcar(val, x);
    setcdr(val, y);
    return(val);
}

void gc(NODE *nd)
{
    NODE *tcar, *tcdr, *tobj;
    int i;
    NODE **pp;

    for (;;) {
	switch (nodetype(nd)) {
	    case PUNBOUND:
		setrefcnt(nd,10000);    /* save some time */
	    case PNIL:
		if (gctop == gcstack) return;
		nd = *--gctop;
		continue;
	    case LINE:
		nd->n_obj = NIL;
	    case CONS:
	    case CASEOBJ:
	    case RUN_PARSE:
	    case QUOTE:
	    case COLON:
	    case TREE:
	    case CONT:
		tcdr = cdr(nd);
		tcar = car(nd);
		tobj = getobject(nd);
		break;
	    case ARRAY:
		pp = getarrptr(nd);
		i = getarrdim(nd);
		while (--i >= 0) {
		    tobj = *pp++;
		    deref(tobj);
		}
		free((char *)getarrptr(nd));
		tcar = tcdr = tobj = NIL;
		break;
	    case STRING:
	    case BACKSLASH_STRING:
	    case VBAR_STRING:
		if (getstrhead(nd) != NULL && decstrrefcnt(getstrhead(nd)) == 0)
		    free(getstrhead(nd));
	    default:
		tcar = tcdr = tobj = NIL;
	}
#ifdef MEM_DEBUG
	settype(nd, NTFREE);
#endif
	nd->n_cdr = free_list;
	free_list = nd;
#ifdef MEM_DEBUG
	record_freed_pointer(nd);
	mem_freed++;
#endif
	if (tcdr != NIL && decrefcnt(tcdr) == 0)
	    if (gctop < &gcstack[GCMAX])
		*gctop++ = tcdr;
	if (tcar != NIL && decrefcnt(tcar) == 0)
	    if (gctop < &gcstack[GCMAX])
		*gctop++ = tcar;
	if (tobj != NIL && decrefcnt(tobj) == 0)
	    if (gctop < &gcstack[GCMAX])
		*gctop++ = tobj;
	if (gctop == gcstack) return;
	nd = *--gctop;
    }
}

NODE *reserve_tank;

void fill_reserve_tank() {
    NODE *p = NIL;
    int i = 50;

    while (--i >= 0) {
	p = reref(p, cons(NIL, p));
    }
    reserve_tank = p;
}

void use_reserve_tank() {
    reserve_tank = reref(reserve_tank, NIL);
}

void check_reserve_tank() {
    if (reserve_tank == NIL) fill_reserve_tank();
}
