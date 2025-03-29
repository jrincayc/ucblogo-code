/*
 *      mem.c           logo memory management module           dvb 6/28/88
 *
 *	Copyright (C) 1993 by the Regents of the University of California
 *
 *      This program is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation, either version 3 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdarg.h>

#define WANT_EVAL_REGS 1
#include "logo.h"
#include "globals.h"
extern NODE *stack, *numstack, *expresn, *val, *parm, *catch_tag, *arg;

#ifdef __SANITIZE_ADDRESS__
#include <sanitizer/asan_interface.h>
#define POISON_NODE(nd) (ASAN_POISON_MEMORY_REGION(&((nd)->nunion), sizeof((nd)->nunion)))
#define UNPOISON_NODE(nd) (ASAN_UNPOISON_MEMORY_REGION(&((nd)->nunion), sizeof((nd)->nunion)))
#else
#define POISON_NODE(nd) ((void)0)
#define UNPOISON_NODE(nd) ((void)0)
#endif

#ifdef PUNY
#define GCMAX 1000
#else
#define GCMAX 16000

#endif

NODE *gcstack[GCMAX];

NODE **mark_gcstack = gcstack;
NODE **gctop = gcstack;
NODE **gcbottom = gcstack;

long int mem_nodes = 0, mem_max = 0;	/* for Logo NODES primitive */

/* GC heuristic parameters. These parameters can be modified to fine tune
   the performance of the GC program. The values below are a good set of
   default parameters that should work well for most data */

/* Number of times to collect at the current GC state before going to
   the next state. Basically the number of times a given generation is
   collected before its members are moved to an older generation */
#define gc_age_threshold 4

FIXNUM seg_size = SEG_SIZE;

/* A new segment of nodes is added if fewer than freed_threshold nodes are
   freed in one GC run */
#define freed_threshold ((size_t)(seg_size * 0.4))

NODE *free_list = NIL;                /* global ptr to free node list */
struct segment *segment_list = NULL;  /* global ptr to segment list */

long int mem_allocated = 0, mem_freed = 0;

/* The number of generations */
#define NUM_GENS 4

/* ptr to list of Nodes in the same generation */
NODE *generation[NUM_GENS] = {NIL};

/* ptr to list of nodes that point to younger nodes */
NODE *oldyoungs = NIL;
BOOLEAN oldyoungs_dirty = FALSE;
#define DEBUGSTREAM     (dribblestream ? dribblestream : stdout)

long int current_gc = 0;

long int gc_stack_malloced = 0;

long int gc_stack_size = GCMAX;

long int gc_overflow_flag = 0;

NODE *reserve_tank = NIL;

BOOLEAN inside_gc = 0, int_during_gc = 0;

int next_gen_gc = 0, max_gen = 0;

int mark_gen_gc;

#if 0
#define GC_DEBUG 1 /* */
#define GC_TWOBYTE 1 /* Use 2-byte stack offset in mark phase */
#endif

#ifdef SERIALIZE_OBJECTS
unsigned long long int next_node_id = 0;
#endif

#ifdef GC_DEBUG
long int num_examined, num_visited;
#endif

NODE *lsetsegsz(NODE *args) {
    NODE *num = pos_int_arg(args);

    if (NOT_THROWING)
	seg_size = getint(num);
    return UNBOUND;
}

BOOLEAN addseg(void) {
    long int p;
    struct segment *newseg;

    if ((newseg = (struct segment *)malloc(sizeof(struct segment)
					   + seg_size*sizeof(struct logo_node)))
            != NULL) {
        newseg->next = segment_list;
	newseg->size = seg_size;
        segment_list = newseg;
        for (p = 0; p < seg_size; p++) {
            newseg->nodes[p].next = free_list;
            free_list = &newseg->nodes[p];
	    settype(&newseg->nodes[p], NTFREE);
		POISON_NODE(&newseg->nodes[p]);

	}
	return 1;
    } else
  	return 0;
}

#ifdef WIN32
#pragma optimize("",off)
#endif
/* Think C tries to load ptr_val->node_type early if optimized */

#define NILP(x)         (NIL == (x))

/* GC_OPT seems to work, but valid_pointer might be needed */
// #define GC_OPT          1

#ifdef GC_OPT
#define VALID_PTR(x)    (!NILP(x))
#else
#define VALID_PTR(x)    (valid_pointer(x))
#endif

BOOLEAN valid_pointer ( NODE *ptr_val) {
    struct segment* current_seg;
    size_t ptr = (size_t)ptr_val;
    FIXNUM size;
   
    if (ptr_val == NIL) return 0;
    for (current_seg = segment_list; current_seg != NULL;
		current_seg = current_seg->next) {
	size = current_seg->size;
	if ((ptr >= (size_t)&current_seg->nodes[0]) &&
	    (ptr <= (size_t)&current_seg->nodes[size-1]) &&
	    ((ptr - (size_t)&current_seg->nodes[0])%
	                 (sizeof(struct logo_node)) == 0))
	    return (ptr_val->node_type != NTFREE);
    }
    return 0;
}

#ifdef WIN32
/* #pragma optimize("",on) */
#endif

NODETYPES nodetype(NODE *nd) {
    if (nd == NIL) return (PNIL);
    return(nd->node_type);
}

void check_oldyoung(NODE *old, NODE *new) {
    if (VALID_PTR(new) && (new->my_gen < old->my_gen) &&
			      old->oldyoung_next == NIL) {
	old->oldyoung_next = oldyoungs;
	oldyoungs = old;
    }
}

void check_valid_oldyoung(NODE *old, NODE *new) {
    if (new == NIL) return;
    if ((new->my_gen < old->my_gen) && old->oldyoung_next == NIL) {
	old->oldyoung_next = oldyoungs;
	oldyoungs = old;
    }
}

/* clean_oldyoungs removes the free nodes from the oldyoungs list,
   traversing the list just once. */
void clean_oldyoungs(void) {
    NODE **prev;
    NODE *nd, *next;
    long int num_cleaned;

    if (FALSE == oldyoungs_dirty)
        return;

    num_cleaned = 0;
    prev = &oldyoungs;
    for (nd = oldyoungs; nd != NIL; ) {
        if (NTFREE != nodetype(nd)) {
             *prev = nd; prev = &(nd->oldyoung_next);
             nd = nd->oldyoung_next;
        } else {
             next = nd->oldyoung_next;
             nd->oldyoung_next = NIL;
             nd = next;
             num_cleaned++;
        }
    }
    oldyoungs_dirty = FALSE;

#ifdef GC_DEBUG
    fprintf(DEBUGSTREAM, "oldyoungs %ld + ", num_cleaned); fflush(DEBUGSTREAM);
#endif

}

/* setcar/cdr/object should be called only when the new pointee is really
 * a node.  Otherwise just directly assign to the field (e.g. for CONTs). */

void setobject(NODE *nd, NODE *newobj) {
    nd->n_obj = newobj;
    check_valid_oldyoung(nd, newobj);
}

void setcar(NODE *nd, NODE *newcar) {
    nd->n_car = newcar;
    check_valid_oldyoung(nd, newcar);
}

void setcdr(NODE *nd, NODE *newcdr) {
    nd->n_cdr = newcdr;
    check_valid_oldyoung(nd, newcdr);
}

#ifdef WIN32
#pragma optimize("",off)
#endif



void do_gc(BOOLEAN full) {
#if 1
    jmp_buf env;
    setjmp(env);
#else
    register NODE *pa, *pb, *pc, *pd, *pe;	/* get registers onto stack */
    register int aa, bb, cc, dd, ee;
#endif

    int_during_gc = 0;
    inside_gc++;
    gc(full);
    inside_gc = 0;
    if (int_during_gc != 0) {
	delayed_int();
    }
}

NODE *newnode(NODETYPES type) {
    register NODE *newnd;
    static NODE phony;

    while ((newnd = free_list) == NIL && NOT_THROWING) {
	do_gc(FALSE);
    }
    if (newnd != NIL) {
	UNPOISON_NODE(newnd);
	free_list = newnd->next;
#ifdef SERIALIZE_OBJECTS
	newnd->id = next_node_id++;
#endif
	newnd->n_car = NIL;
	newnd->n_cdr = NIL;
	newnd->n_obj = NIL;
	newnd->my_gen = 0;
	newnd->gen_age = gc_age_threshold;
	newnd->mark_gc = 0;
	newnd->next = generation[0];
	generation[0] = newnd;
	newnd->oldyoung_next = NIL;
	settype(newnd, type);
	mem_nodes++;
	if (mem_nodes > mem_max) mem_max = mem_nodes;
	return(newnd);
    } else return &phony;
}

#ifdef WIN32
/* #pragma optimize("",on) */
#endif

NODE *cons(NODE *x, NODE *y) {
    NODE *val = newnode(CONS);

    /* New node can't possibly point to younger one, so no need to check */
    val->n_car = x;
    val->n_cdr = y;
    return(val);
}

#define mmark(child) {if ((child)->my_gen < nd->my_gen) \
			 {mark(child); got_young++;}}

NODE **inter_gen_mark (NODE **prev) {
/* Mark/traverse pointers to younger generations only */
    NODE* nd = *prev;
    NODE** array_ptr;
    NODE* tmp_node;
    int loop;
    int got_young = 0;

    if (nd->my_gen <= mark_gen_gc) return &(nd->oldyoung_next);
    switch (nodetype(nd)) {
	case CONS:
	case CASEOBJ:
	case RUN_PARSE:
	case QUOTE:
	case COLON:
	case TREE:
	case LINE:
	case LOCALSAVE:
#ifdef OBJECTS
	case OBJECT:
	case METHOD:
#endif
	    if (VALID_PTR(nd->n_car))
		mmark(nd->n_car);
	    if (VALID_PTR(nd->n_obj))
		mmark(nd->n_obj);
	case CONT:
	    if (VALID_PTR(nd->n_cdr))
		mmark(nd->n_cdr);
	    break;
	case STACK:
	    if (VALID_PTR(nd->n_cdr))
		mmark(nd->n_cdr);
	    array_ptr = (NODE **)car(nd);
	    loop = num_saved_nodes;
	    while (--loop >= 0) {
		tmp_node = *array_ptr++;
		if (VALID_PTR(tmp_node))
		    mmark(tmp_node);
	    }
	    break;
	case ARRAY:
	    array_ptr = getarrptr(nd);
	    loop = getarrdim(nd);
	    while (--loop >= 0) {
		tmp_node = *array_ptr++;
		if (VALID_PTR(tmp_node))
		    mmark(tmp_node);
	    }
	    break;
        default:
	    //no special gc handling
    }
// #ifdef WHYDOESNTTHISWORK
    if (!got_young) {	/* nd no longer points to younger */
	*prev = nd->oldyoung_next;
   	nd->oldyoung_next = NIL;
   	return prev;
    }
// #endif
    return &(nd->oldyoung_next);
}

void gc_inc () {
    NODE **new_gcstack;
    long int loop;

    if (gc_overflow_flag == 1) return;

    if (gctop == &mark_gcstack[gc_stack_size-1])
	gctop = mark_gcstack;
    else
	gctop++;
    if (gctop == gcbottom) { /* gc STACK overflow */
#ifdef GC_DEBUG
	fprintf(DEBUGSTREAM,"\nAllocating new GC stack\n"); fflush(DEBUGSTREAM);
#endif
	if ((new_gcstack = (NODE**) malloc ((size_t) sizeof(NODE *) *
				(gc_stack_size + GCMAX))) == NULL) {

	    /* no room to increse GC Stack */
	    ndprintf(stdout, "\n%t\n", message_texts[CANT_GC]);
	    ndprintf(stdout, "%t\n", message_texts[EXIT_NOW]);

	    gc_overflow_flag = 1;
	} else {
	    /* transfer old stack to new stack */
	    new_gcstack[0] = *gcbottom;
	    if (gcbottom == &mark_gcstack[gc_stack_size-1])
		gcbottom = mark_gcstack;
	    else
		gcbottom++;

	    for (loop = 1 ; gcbottom != gctop ; loop++) {
		new_gcstack[loop] = *gcbottom;
		if (gcbottom == &mark_gcstack[gc_stack_size-1])
		    gcbottom = mark_gcstack;
		else
		    gcbottom++;
	    }
	    gc_stack_size = gc_stack_size + GCMAX;
	    if (gc_stack_malloced == 1) free(mark_gcstack);
	    gc_stack_malloced = 1;

	    mark_gcstack = new_gcstack;
	    gctop = &mark_gcstack[loop];
	    gcbottom = mark_gcstack;
	}
    }
}

/* Iterative mark procedure */
void mark(NODE* nd) {
    int loop;
    NODE** array_ptr;

    if (gc_overflow_flag == 1) return;
    if (!VALID_PTR(nd)) return; /* NIL pointer */
    if (nd->my_gen > mark_gen_gc) return; /* I'm too old */
    if (nd->mark_gc == current_gc) return; /* I'm already marked */

    *gctop = nd;
    gc_inc();

    while (gcbottom != gctop) {
	nd = *gcbottom;
	if ((VALID_PTR(nd)) && (nd->my_gen <= mark_gen_gc) &&
		(nd->mark_gc != current_gc)) {
	    if (nd->mark_gc == -1) {
		nd->mark_gc = 0;    /* this is a caseobj during gctwa */
		goto no_mark;	    /* so don't really mark yet */
	    }
	    nd->mark_gc = current_gc;
#ifdef GC_DEBUG
	    num_examined++;
#endif
	    switch (nodetype(nd)) {
		case CONS:
		case CASEOBJ:
		case RUN_PARSE:
		case QUOTE:
		case COLON:
		case TREE:
		case LINE:
		case LOCALSAVE:
#ifdef OBJECTS
		case OBJECT:
		case METHOD:
#endif
		    *gctop = nd->n_car;
		    gc_inc();
		    *gctop = nd->n_obj;
		    gc_inc();
		case CONT:
		    *gctop = nd->n_cdr;
		    gc_inc();
		    break;
		case STACK:
		    *gctop = nd->n_cdr;
		    gc_inc();
		    array_ptr = (NODE **)car(nd);
		    loop = num_saved_nodes;
		    while (--loop >= 0) {
			*gctop = *array_ptr++;
			gc_inc();
		    }
		    break;
		case ARRAY:
		    array_ptr = getarrptr(nd);
		    loop = getarrdim(nd);
		    while (--loop >= 0) {
			*gctop = *array_ptr++;
			gc_inc();
		    }
		break;
	        default:
		    //no special gc handling
	    }
	}
no_mark:
	if (gcbottom == &mark_gcstack[gc_stack_size-1])
	    gcbottom = mark_gcstack;
	else
	    gcbottom++;
    }
}

void __attribute__((no_sanitize_address)) mark_stack( NODE** top){
    /* Check Stack for NODE pointers */
     NODE** top_stack;
	 NODE** tmp_ptr;

#ifdef __SANITIZE_ADDRESS__
	 void* fake_stack = __asan_get_current_fake_stack();
	 void* fake_frame_beg;
	 void* fake_frame_end;
	 NODE** fake_ptr;
	 void* real_ptr;
	real_ptr = __asan_addr_is_in_fake_stack(
			__asan_get_current_fake_stack(), 
			(void*)top, NULL, NULL
		);
	top_stack = real_ptr ? real_ptr : top;
#else
    top_stack = top; /*GC*/
#endif

	// printf("top %llx bottom %llx\n", top_stack, bottom_stack);
    if (top_stack < bottom_stack) { /* check direction stack grows */
	for (tmp_ptr = top_stack; tmp_ptr <= bottom_stack; 
#if defined(GC_TWOBYTE)
	     tmp_ptr = (NODE **)(((unsigned long int)tmp_ptr)+2)
#else
	     tmp_ptr++
#endif
	     ) {
// Under ASAN stack frames may be allocated on the heap and require an
// extra level of indirection to access stack variables. The real stack
// frame only contains a pointer to the fake frame.
#ifdef __SANITIZE_ADDRESS__
		real_ptr = __asan_addr_is_in_fake_stack(
			fake_stack, (void*)*tmp_ptr, &fake_frame_beg, 
			&fake_frame_end
		);
		if (real_ptr) {
			// Pointer to fake stack frame
			for (fake_ptr = fake_frame_beg; 
				fake_ptr < fake_frame_end; 
				fake_ptr++) 
			{
				// Pointer on fake stack
				if (valid_pointer(*fake_ptr)) {
					mark(*fake_ptr);
				}
			}
		}
		else {
			// Pointer on real stack
			if (valid_pointer(*tmp_ptr)) {
				mark(*tmp_ptr);
			}
		}
#else
		if (valid_pointer(*tmp_ptr)) {
		    mark(*tmp_ptr);
		}
#endif
	}
    } else {
	for (tmp_ptr = top_stack; tmp_ptr >= bottom_stack; 
#if defined(GC_TWOBYTE)
	     tmp_ptr = (NODE **)(((unsigned long int)tmp_ptr)-2)
#else
		tmp_ptr--
#endif
	     ) {
#ifdef __SANITIZE_ADDRESS__
		real_ptr = __asan_addr_is_in_fake_stack(
			fake_stack, (void*)*tmp_ptr, &fake_frame_beg, 
			&fake_frame_end
		);
		if (real_ptr) {
			// Pointer to fake stack frame
			// fake stack always grows this way
			for (fake_ptr = fake_frame_beg; 
				fake_ptr < fake_frame_end; 
				fake_ptr++) 
			{
				// Pointer on fake stack
				if (valid_pointer(*fake_ptr)) {
					mark(*fake_ptr);
				}
			}
		}
		else {
			// Pointer on real stack
			if (valid_pointer(*tmp_ptr)) {
				mark(*tmp_ptr);
			}
		}
#else
		if (valid_pointer(*tmp_ptr)) {
		    mark(*tmp_ptr);
		}
#endif
	}
    }
}

void gc (BOOLEAN no_error)  {
    NODE *top;
    NODE *nd, *tmpnd;
    long int num_freed = 0;
    NODE **tmp_ptr, **prev;
    long int freed_sofar = 0;
    NODE** array_ptr;
    NODE* tmp_node;
    NODE *obj, *caselist;
    int anygood;
    int i;
    short int loop;
    int gen_gc; /* deepest generation to garbage collect */
    int gctwa;	/* garbage collect truly worthless atoms */

    if (gc_overflow_flag == 1) {
	if (!addseg()) {
	    err_logo(OUT_OF_MEM, NIL);
	    if (free_list == NIL)
		err_logo(OUT_OF_MEM_UNREC, NIL);
	}
	return;
    }

    if (check_throwing)
        return;

    mark_gen_gc = gen_gc = (no_error ? max_gen : next_gen_gc);

    gctwa = (gen_gc == max_gen && max_gen > 1) || no_error;

    if (gctwa) {
	/* Every caseobj must be marked twice to count */
	for (loop = 0; loop < HASH_LEN ; loop++) {
	    for (nd = hash_table[loop]; nd != NIL; nd = cdr(nd)) {
		tmpnd = caselist__object(car(nd));
		while (tmpnd != NIL) {
		    (car(tmpnd))->mark_gc = -1;
		    tmpnd = cdr(tmpnd);
		}
	    }
	}
    }

re_mark:

    current_gc++;

#ifdef GC_DEBUG
    fprintf(DEBUGSTREAM, "gen = %d\n", gen_gc); fflush(DEBUGSTREAM);
    num_examined = 0;
#endif

    /* Begin Mark Phase */

    /* Check globals for NODE pointers */

    mark(current_line);
    mark(command_line);    //
    mark(deepend_proc_name);

    mark(Listvalue);
    mark(Dotsvalue);
    mark(Unbound);
    mark(Not_Enough_Node);

    mark(cnt_list);
    mark(cnt_last);
    mark(throw_node);
    mark(err_mesg);
    mark(var_stack);
    mark(output_node);
    mark(output_unode);
    mark(last_call);
    mark(Regs_Node);
    mark(eval_buttonact);       //
    mark(file_list);
    mark(reader_name);
    mark(writer_name);
    mark(file_prefix);
    mark(save_name);
    mark(the_generation);
    mark(tree_dk_how);          //
#ifdef OBJECTS
    mark(logo_object);
    mark(current_object);
    mark(askexist);
#endif

    mark(stack);
    mark(numstack);
    mark(expresn);
    mark(val);
    mark(parm);
    mark(catch_tag);
    mark(arg);

    mark(proc);
    mark(argl);
    mark(unev);
    mark(fun);
    mark(ufun);
    mark(var);
    mark(vsp);
    mark(qm_list);
    mark(formals);
    mark(last_ufun);
    mark(this_line);
    mark(last_line);
    mark(current_unode);
    mark(didnt_output_name);
    mark(didnt_get_output);
#ifdef OBJECTS
    mark(usual_parent);
    mark(usual_caller);
#endif




#ifdef GC_DEBUG
    fprintf(DEBUGSTREAM, "globals %ld + ", num_examined); fflush(DEBUGSTREAM);
    num_examined = 0;
#endif

    for (loop = 0; loop < HASH_LEN ; loop++)
	mark(hash_table[loop]);

#ifdef GC_DEBUG
    fprintf(DEBUGSTREAM, "oblist %ld + ", num_examined); fflush(DEBUGSTREAM);
    num_examined = 0;
#endif

mark_stack(&top);

#ifdef GC_DEBUG
    fprintf(DEBUGSTREAM, "stack %ld + ", num_examined); fflush(DEBUGSTREAM);
    num_examined = 0;
    num_visited = 0;
#endif

    /* check pointers from old generations to young */
    for (prev = &oldyoungs; *prev != Unbound; prev = inter_gen_mark(prev))
#ifdef GC_DEBUG
        num_visited++
#endif
        ;

#ifdef GC_DEBUG
    fprintf(DEBUGSTREAM, "inter_gen %ld marked %ld visited\n", num_examined, num_visited); fflush(DEBUGSTREAM);
    num_examined = 0;
#endif

    if (gc_overflow_flag) return;

    if (gctwa) {

#ifdef GC_DEBUG
    fprintf(DEBUGSTREAM, "GCTWA: "); fflush(DEBUGSTREAM);
    num_examined = 0;
#endif
	for (loop = 0; loop < HASH_LEN ; loop++) {
	    tmpnd = NIL;
	    for (nd = hash_table[loop]; nd != NIL; nd = cdr(nd)) {
		obj = car(nd);
		if (procnode__object(obj) == UNDEFINED &&
		    valnode__object(obj) == UNBOUND &&
		    plist__object(obj) == NIL &&
		    !flag__object(obj, PERMANENT)) {
#ifdef GC_DEBUG
			    num_examined++;
#endif
			anygood = 0;
			for (caselist = caselist__object(obj);
					caselist != NIL; caselist = cdr(caselist)) {
			    if ((car(caselist))->mark_gc == current_gc) {
				anygood = 1;
				break;
			    }
			}
			if (anygood) {	/* someone points here, don't gctwa */
			    tmpnd = nd;
			} else {	/* do gctwa */
			    if (tmpnd == NIL)
				hash_table[loop] = cdr(hash_table[loop]);
			    else
				setcdr(tmpnd, cdr(nd));
			}
		} else			/* has a value, don't gctwa */
		    tmpnd = nd;
	    }
	}

#ifdef GC_DEBUG
    fprintf(DEBUGSTREAM, "%ld collected\n", num_examined); fflush(DEBUGSTREAM);
    num_examined = 0;
#endif
	gctwa = 0;
	goto re_mark;
    }

    /* Begin Sweep Phase */
	// For deep debugging
	// goto skip_sweep;
   	
    for (loop = gen_gc; loop >= 0; loop--) {
	tmp_ptr = &generation[loop];
	for (nd = generation[loop]; nd != NIL; nd = *tmp_ptr) {
	    if (nd->mark_gc == current_gc) {
		if (--(nd->gen_age) == 0 && loop < NUM_GENS-1) {
		    /* promote to next gen */
		    *tmp_ptr = nd->next;
		    nd->next = generation[loop+1];
		    generation[loop+1] = nd;
		    nd->my_gen = loop+1;
		    if (max_gen == loop) max_gen++;
		    nd->gen_age = gc_age_threshold;
		    switch (nodetype(nd)) {
			case CONS:
			case CASEOBJ:
			case RUN_PARSE:
			case QUOTE:
			case COLON:
			case TREE:
			case LINE:
			case LOCALSAVE:
#ifdef OBJECTS
			case OBJECT:
			case METHOD:
#endif
                            clean_oldyoungs();
			    check_oldyoung(nd, nd->n_car);
			    check_oldyoung(nd, nd->n_obj);
			case CONT:
                            clean_oldyoungs();
			    check_oldyoung(nd, nd->n_cdr);
			    break;
			case STACK:
                            clean_oldyoungs();
			    check_oldyoung(nd, nd->n_cdr);
			    array_ptr = (NODE **)car(nd);
			    i = num_saved_nodes;
			    while (--i >= 0) {
				tmp_node = *array_ptr++;
				check_oldyoung(nd, tmp_node);
			    }
			    break;
			case ARRAY:
                            clean_oldyoungs();
			    array_ptr = getarrptr(nd);
			    i = getarrdim(nd);
			    while (--i >= 0) {
				tmp_node = *array_ptr++;
				check_oldyoung(nd, tmp_node);
			    }
			    break;
		        default:
			    //no special gc handling
		    }
       		} else {
		    /* keep in this gen */
		    tmp_ptr = &(nd->next);
         	}
	    } else {
		/* free */
		num_freed++;
		mem_nodes--;
         	*tmp_ptr = nd->next;
     		if (nd->oldyoung_next != NIL) {
#ifdef GC_OPT
                    oldyoungs_dirty = TRUE;
#else
		    for (prev = &oldyoungs; *prev != nd;
			    prev = &((*prev)->oldyoung_next))
		        ;
		    *prev = nd->oldyoung_next;
		    nd->oldyoung_next = NIL;
#endif
		}
        	switch (nodetype(nd)) {
		    case ARRAY:
			free((char *)getarrptr(nd));
             		break;
		    case STACK:
			free((char *)car(nd));
			break;
		    case STRING:
		    case BACKSLASH_STRING:
		    case VBAR_STRING:
			if (getstrhead(nd) != NULL &&
				    decstrrefcnt(getstrhead(nd)) == 0)
			    free(getstrhead(nd));
			break;
		    default:
		        //No special memory handleing done
		}
		settype (nd, NTFREE);
	 	nd->next = free_list;
	 	free_list = nd;
		POISON_NODE(nd);
	 }
	}
        clean_oldyoungs();
#ifdef GC_DEBUG
	fprintf(DEBUGSTREAM, "%ld + ", num_freed - freed_sofar); fflush(DEBUGSTREAM);
#endif
	freed_sofar = num_freed;
    }

#ifdef GC_DEBUG
    fprintf(DEBUGSTREAM, "= %ld freed\n", num_freed); fflush(DEBUGSTREAM);
#endif

    if (num_freed > freed_threshold)
	next_gen_gc = 0;
    else if (gen_gc < max_gen)
	next_gen_gc = gen_gc+1;
    else
	next_gen_gc = 0;

skip_sweep:
    if (num_freed < freed_threshold) {
	if (!addseg() && num_freed < 50 && gen_gc == max_gen && !no_error) {
	    err_logo(OUT_OF_MEM, NIL);
	    if (free_list == NIL)
		err_logo(OUT_OF_MEM_UNREC, NIL);
	}
    }

#ifdef GC_DEBUG
/*    getchar(); */
#endif
}

#ifdef GC_DEBUG
void prname(NODE *foo) {
    fprintf(DEBUGSTREAM, "%s ", (char*) car(foo)); fflush(DEBUGSTREAM);
}
#endif

NODE *lgc(NODE *args) {
    do_gc(args != NIL);
    return UNBOUND;
}

NODE *lnodes(NODE *args) {
    long int temp_max, temp_nodes;

#ifdef GC_DEBUG
/*    map_oblist(&prname); */
#endif
    do_gc(TRUE);	/* get real in-use figures */
    temp_max = mem_max;
    temp_nodes = mem_nodes;
    mem_max = mem_nodes;
    return cons(make_intnode(temp_nodes),
		cons(make_intnode(temp_max), NIL));
}

void fill_reserve_tank(void) {
    NODE *newnd, *p = NIL;
    int i = 50;

    while (--i >= 0) {	/* make pairs not in any generation */
	if ((newnd = free_list) == NIL) break;
	UNPOISON_NODE(newnd);
	free_list = newnd->next;
	settype(newnd, CONS);
	newnd->n_car = NIL;
	newnd->n_cdr = p;
	newnd->n_obj = NIL;
	newnd->next = NIL;
	newnd->oldyoung_next = NIL;
	p = newnd;
    }
    reserve_tank = p;
}

void use_reserve_tank(void) {
    NODE *nd = reserve_tank;
	
    reserve_tank = NIL;
    for ( ; nd != NIL; nd = cdr(nd) ) {
    	settype(nd, NTFREE);
    	nd->next = free_list;
    	free_list = nd;
    }
}

void check_reserve_tank(void) {
    if (reserve_tank == NIL) fill_reserve_tank();
}
