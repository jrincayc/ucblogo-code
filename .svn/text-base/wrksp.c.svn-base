/*
 *      wrksp.c         logo workspace management module                dvb
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

#ifdef HAVE_WX
#define fgets wx_fgets
#endif

#ifdef WIN32
#include <windows.h>
#endif

#define WANT_EVAL_REGS 1
#include "logo.h"
#include "globals.h"

#ifdef  HAVE_WX
int wxEditFile(char *);
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef ibm
#include "process.h"
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

char *editor, *editorname, *tempdir;
int to_pending = 0;

NODE *make_procnode(NODE *lst, NODE *wrds, int min, int df, int max) {
    return(cons_list(0, lst, wrds, make_intnode((FIXNUM)min),
		     make_intnode((FIXNUM)df), make_intnode((FIXNUM)max),
		     END_OF_LIST));
}

NODE *get_bodywords(NODE *pproc, NODE *name) {
    NODE *val = bodywords__procnode(pproc);
    NODE *head = NIL, *tail = NIL;

    if (val != NIL) return(val);
    name = intern(name);
    head = cons_list(0, (is_macro(name) ? theName(Name_macro) : theName(Name_to)),
			name, END_OF_LIST);
    tail = cdr(head);
    val = formals__procnode(pproc);
    while (val != NIL) {
	if (is_list(car(val)))
	    setcdr(tail, cons(cons(make_colon(caar(val)), cdar(val)), NIL));
	else if (nodetype(car(val)) == INT)
	    setcdr(tail, cons(car(val),NIL));
	else
	    setcdr(tail, cons(make_colon(car(val)),NIL));
	tail = cdr(tail);
	val = cdr(val);
    }
    head = cons(head, NIL);
    tail = head;
    val = bodylist__procnode(pproc);
    while (val != NIL) {
	setcdr(tail, cons(runparse(car(val)), NIL));
	tail = cdr(tail);
	val = cdr(val);
    }
    setcdr(tail, cons(cons(theName(Name_end), NIL), NIL));
/*  setbodywords__procnode(pproc,head);    */   /* confuses copydef */
    return(head);
}

NODE *name_arg(NODE *args) {
    while (aggregate(car(args)) && NOT_THROWING)
	setcar(args, err_logo(BAD_DATA, car(args)));
    return car(args);
}

NODE *proc_name_arg(NODE *args) {
    while ((aggregate(car(args)) || numberp(car(args))) && NOT_THROWING)
	setcar(args, err_logo(BAD_DATA, car(args)));
    return car(args);
}

NODE *ltext(NODE *args) {
    NODE *name, *val = UNBOUND;

    name = proc_name_arg(args);
    if (NOT_THROWING) {
	name=intern(name);
	check_library(name);
	val = procnode__caseobj(name);
	if (val == UNDEFINED) {
	    err_logo(DK_HOW_UNREC,name);
	    return UNBOUND;
	} else if (is_prim(val)) {
	    err_logo(IS_PRIM,name);
	    return UNBOUND;
	} else 
	    return text__procnode(val);
    }
    return UNBOUND;
}

NODE *lfulltext(NODE *args) {
    NODE *name, *val = UNBOUND;

    name = proc_name_arg(args);
    if (NOT_THROWING) {
	name=intern(name);
	check_library(name);
	val = procnode__caseobj(name);
	if (val == UNDEFINED) {
	    err_logo(DK_HOW_UNREC,name);
	    return UNBOUND;
	} else if (is_prim(val)) {
	    err_logo(IS_PRIM,name);
	    return UNBOUND;
	} else 
	    return get_bodywords(val,name);
    }
    return UNBOUND;
}

BOOLEAN all_lists(NODE *val) {
    if (val == NIL) return TRUE;
    if (!is_list(car(val))) return FALSE;
    return all_lists(cdr(val));
}


#ifdef OBJECTS

BOOLEAN proc_exists(NODE *name) {
    if (current_object == logo_object)
	return procnode__caseobj(name) != UNDEFINED;
    else
	return assoc(name, getprocs(current_object)) != NIL;
}

BOOLEAN prim_exists(NODE *name) {
    NODE *binding;

    if (current_object == logo_object)
	return is_prim(procnode__caseobj(name));
    else
	binding = assoc(name, getprocs(current_object));
	return (binding==NIL ? FALSE : is_prim(getobject(binding)));
}

int find_old_default(NODE *name) {
    NODE *p;

    if (flag__caseobj(name, MIXED_ARITY)) return -2;
    p = procValue(name);
    if (p == UNDEFINED) return -1;
    return (is_prim(p) ? getprimdflt(p) : getint(dfltargs__procnode(p)));
}

#endif /* OBJECTS */

NODE *define_helper(NODE *args, BOOLEAN macro_flag) {
    /* macro_flag is -1 for anonymous function */
    NODE *name = NIL, *val = NIL, *arg = NIL;
    int minimum = 0, deflt = 0, maximum = 0, old_default = -1;
    int redef = (varTrue(Redefp));

    if (macro_flag >= 0) {
	name = proc_name_arg(args);
	if (NOT_THROWING) {
	    name = intern(name);
	    val = procnode__caseobj(name);
	    if (!redef && is_prim(val)) {
		err_logo(IS_PRIM,name);
		return UNBOUND;
	    } else if (val != UNDEFINED) {
		old_default = (is_prim(val) ? getprimdflt(val) :
					      getint(dfltargs__procnode(val)));
	    }
	}
	if (NOT_THROWING) {
	    val = cadr(args);
	    while ((val == NIL || !is_list(val) || !all_lists(val)) &&
			    NOT_THROWING) {
		setcar(cdr(args), err_logo(BAD_DATA, val));
		val = cadr(args);
	    }
	    if (NOT_THROWING) val = deep_copy(val);
	    /* 5.4 fixes bug about defined procedures sharing tree form */
	}
    } else {	/* lambda */
	val = args;
    }
    if (NOT_THROWING) {
	args = car(val);
	if (args != NIL) {
	    make_runparse(args);
	    args = parsed__runparse(args);
	}
	setcar(val, args);
	while (args != NIL) {
	    arg = car(args);
	    if (arg != NIL && is_list(arg) && maximum != -1) {
		make_runparse(arg);
		arg = parsed__runparse(arg);
		setcar(args, arg);
		setcar(arg, intern(car(arg)));	/* fixes crash for # as arg */
		maximum++;
		if (arg == NIL || !is_word(car(arg))) {
		    err_logo(BAD_DATA_UNREC, arg);
		    break;
		}
		if (cdr(arg) == NIL)
		    maximum = -1;
	    } else if (nodetype(arg) == INT) {
		if ((unsigned)getint(arg) <= (unsigned) maximum &&
		     getint(arg) >= minimum) {
			deflt = getint(arg);
		} else {
		    err_logo(BAD_DATA_UNREC, arg);
		    break;
		}
	    } else if (is_word(arg) && maximum == minimum) {
		minimum++;
		maximum++;
		deflt++;
	    } else {
		err_logo(BAD_DATA_UNREC, arg);
		break;
	    }
	    args = cdr(args);
	    if (check_throwing) break;
	}
    }
    if (macro_flag < 0) {
	return make_procnode(val, NIL, minimum, deflt, maximum);
    } else if (NOT_THROWING) {
	setprocnode__caseobj(name,
			     make_procnode(val, NIL, minimum, deflt, maximum));
	if (macro_flag)
	    setflag__caseobj(name, PROC_MACRO);
	else
	    clearflag__caseobj(name, PROC_MACRO);
	if (deflt != old_default && old_default >= 0) {
	    the_generation = cons(NIL, NIL);
	}
	need_save = 1;
    }
    return(UNBOUND);
}

NODE *ldefine(NODE *args) {
    return define_helper(args, FALSE);
}

NODE *ldefmacro(NODE *args) {
    return define_helper(args, TRUE);
}

NODE *anonymous_function(NODE *text) {
    return define_helper(text, -1);
}

char *strncasestr(char *big, char *little, FIXNUM len) {
    char *p, *q, pc, qc;
    FIXNUM i;

    while (*big != '\0') {
	while ((pc = *big++) != '\0' && tolower(pc) != tolower(*little)) ;
	if (pc == '\0') return NULL;
	p = big; q = little+1; i = len;
	while (--i > 0 && (qc = *q++) != '\0') {
	    if ((pc = *p++) == '\0') return NULL;
	    if (pc == '~') {
		while ((pc = *p++) != '\0' && pc != '\n') ;
		if (pc == '\0') return NULL;
		pc = *p++;
	    }
	    if (tolower(pc) != tolower(qc)) break;
	}
	if (i == 0) return big-1;
    }
    return NULL;    /* not reached, I think */
}

NODE *find_to(NODE *line) {
    char *lp = getstrptr(line);
    NODE *funn = cnv_node_to_strnode(fun);
    char *fp = getstrptr(funn);
    FIXNUM len = getstrlen(funn);
    char *p, c;

    p = lp;
    while (p != NULL) {
	p = strncasestr(p, fp, len);
	if (p == NULL) return(line);    /* punt */
	if ((c = *(p+len)) == ' ' || c == '\t' || c == '\n') {
	    if (p == lp ||
		    ((c = *(p-1)) == ' ' || c == '\t' || c == '\n'))
		return make_strnode(p, getstrhead(line),
				    getstrlen(line)-(p-lp),
				    nodetype(line), strcpy);
	    if (c == '[')
		return make_strnode(p, getstrhead(line),
				    strchr(p, ']')-p,
				    nodetype(line), strcpy);
	}
	p++;
    }
    return line;
}

NODE *to_helper(NODE *args, BOOLEAN macro_flag) {
    NODE *arg = NIL, *tnode = NIL, *proc_name, *forms = NIL, *lastnode = NIL,
	 *body_words, *lastnode2, *body_list;
    int minimum = 0, deflt = 0, maximum = 0, old_default = -1;

    if (ufun != NIL && loadstream == stdin) {
	err_logo(NOT_INSIDE,NIL);
	return(UNBOUND);
    }

    if (args == NIL) {
	err_logo(NOT_ENOUGH,NIL);
	return(UNBOUND);
    }

    deepend_proc_name = proc_name = car(args);
    args = cdr(args);

    if (nodetype(proc_name) != CASEOBJ)
	err_logo(BAD_DATA_UNREC, proc_name);
#ifdef OBJECTS
    else if ((proc_exists(proc_name) && loadstream == stdin)
	     || prim_exists(proc_name))
#else /* OBJECTS */
    else if ((procnode__caseobj(proc_name) != UNDEFINED && loadstream == stdin)
	     || is_prim(procnode__caseobj(proc_name)))
#endif /* OBJECTS */
	err_logo(ALREADY_DEFINED, proc_name);
    else {
#ifdef OBJECTS
	old_default = find_old_default(proc_name);
#else /* OBJECTS */
	NODE *old_proc = procnode__caseobj(proc_name);
	if (old_proc != UNDEFINED) {
	    old_default = (is_prim(old_proc) ? getprimdflt(old_proc) :
					      getint(dfltargs__procnode(old_proc)));
	}
#endif /* OBJECTS */
	while (args != NIL) {
	    arg = car(args);
	    args = cdr(args);
	    if (nodetype(arg) == CONS && maximum != -1) {
		make_runparse(arg);
		arg = parsed__runparse(arg);
		maximum++;
		if (arg == NIL || !is_word(car(arg))) {
		    err_logo(BAD_DATA_UNREC, arg);
		    break;
		}
		if (nodetype(car(arg)) == COLON)
		    setcar(arg, node__colon(car(arg)));
		if (nodetype(car(arg)) == QUOTE)
		    setcar(arg, node__quote(car(arg)));
		if (cdr(arg) == NIL)
		    maximum = -1;
	    } else if (nodetype(arg) == INT) {
		if ((unsigned)getint(arg) <= (unsigned) maximum &&
		     getint(arg) >= minimum) {
			deflt = getint(arg);
		} else {
		    err_logo(BAD_DATA_UNREC, arg);
		    break;
		}
	    } else if (is_word(arg) && maximum == minimum) {
		if (nodetype(arg) == COLON)
		    arg = node__colon(arg);
		if (nodetype(arg) == QUOTE)
		    arg = node__quote(arg);
		minimum++;
		maximum++;
		deflt++;
	    } else {
		err_logo(BAD_DATA_UNREC, arg);
		break;
	    }
	    tnode = cons(arg, NIL);
	    if (forms == NIL) forms = tnode;
	    else setcdr(lastnode, tnode);
	    lastnode = tnode;
	}
    }

    if (NOT_THROWING) {
	body_words = cons(find_to(current_line), NIL);
	lastnode2 = body_words;
	body_list = cons(forms, NIL);
	lastnode = body_list;
	to_pending++;    /* for int or quit signal */
	while (NOT_THROWING && to_pending && (!feof(loadstream))) {
	    tnode = cons(reader(loadstream, "> "), NIL);
	    if ((feof(loadstream))) {
		tnode = cons(theName(Name_end), NIL);
	    }
	    setcdr(lastnode2, tnode);
	    lastnode2 = tnode;
	    tnode = cons(parser(car(tnode), TRUE), NIL);
	    if (car(tnode) != NIL && isName(caar(tnode), Name_end))
		break;
	    else if (car(tnode) != NIL) {
		setcdr(lastnode, tnode);
		lastnode = tnode;
	    }
	}
	if (to_pending && NOT_THROWING) {
#ifdef OBJECT
	    if (current_object != logo_object) {
		setprocs(current_object,
			 cons(make_procnode(body_list, body_words, minimum,
                                               deflt, maximum),
			        getprocs(current_object)));
#else
	    setprocnode__caseobj(proc_name,
				 make_procnode(body_list, body_words, minimum,
					       deflt, maximum));
	    if (macro_flag)
		setflag__caseobj(proc_name, PROC_MACRO);
	    else
		clearflag__caseobj(proc_name, PROC_MACRO);
#endif
	    if (deflt != old_default && old_default >= 0) {
		the_generation = cons(NIL, NIL);
	    }
	    if (loadstream == stdin || varTrue(LoadNoisily)) {
		ndprintf(stdout, message_texts[LOAD_DEF], proc_name);
	    }
	    if (loadstream != stdin && varTrue(UnburyOnEdit)) {
		clearflag__caseobj(proc_name, PROC_BURIED);
	    }
	}
	to_pending = 0;
	need_save = 1;
    }
    deepend_proc_name = NIL;
    return(UNBOUND);
}

NODE *lto(NODE *args) {
    return to_helper(args, FALSE);
}

NODE *lmacro(NODE *args) {
    return to_helper(args, TRUE);
}

#ifdef OBJECTS
/* If binding found in object hierarchy and as local, flag error, same
   as varValue.  If binding found only in object hierarchy, set it.
   Otherwise (including if no binding found at all) set valnode in
   symbol table. Need to distinguish var local to --this-- procedure from var 
   inherited from caller.  Former is allowed to conflict with object variable. 
 */

NODE *lmake(NODE *args) {
    NODE *what, *object, *bindings, *binding;

    what = name_arg(args);
    if (NOT_THROWING) {
        what = intern(what);

        if (varInObjectHierarchy(what, FALSE) != (NODE *)(-1)) {
            if (flag__caseobj(what, IS_LOCAL_VALUE)) {
                err_logo(LOCAL_AND_OBJ, what);
                return UNBOUND;
            } else {
		need_save = 1;
                object = varInThisObject(what, FALSE);
		for (bindings = getvars(object); bindings != NIL;
			    bindings = cdr(bindings)) {
                    if (car(bindings) == what) {
                        setobject(bindings, cadr(args));
			break;
                    }
                }
            }
        } else {
            setvalnode__caseobj(what, cadr(args));
	    if (!flag__caseobj(what, IS_LOCAL_VALUE)) {
		setflag__caseobj(what, HAS_GLOBAL_VALUE);
		need_save = 1;
	    }
        }

        if (flag__caseobj(what, VAL_TRACED)) {
            NODE *tvar = maybe_quote(cadr(args));
            ndprintf(writestream, message_texts[TRACE_MAKE],
                    make_quote(what), tvar);
            if (ufun != NIL) {
                ndprintf(writestream,message_texts[ERROR_IN],ufun,this_line);
            }
            new_line(writestream);
        }
    }
    return(UNBOUND);
}

#else /* OBJECTS */

NODE *lmake(NODE *args) {
    NODE *what;

    what = name_arg(args);
    if (NOT_THROWING) {
	what = intern(what);
	setvalnode__caseobj(what, cadr(args));
	if (!flag__caseobj(what, IS_LOCAL_VALUE)) {
	    setflag__caseobj(what, HAS_GLOBAL_VALUE);
	    need_save = 1;
	}
	if (flag__caseobj(what, VAL_TRACED)) {
	    NODE *tvar = maybe_quote(cadr(args));
	    ndprintf(writestream, message_texts[TRACE_MAKE],
				  make_quote(what), tvar);
	    if (ufun != NIL) {
		ndprintf(writestream,message_texts[ERROR_IN],ufun,this_line);
	    }
	    new_line(writestream);
	}
    }
    return(UNBOUND);
}

#endif /* OBJECTS */

NODE *llocal(NODE *args) {
    NODE *arg = NIL;

    if (tailcall != 0) return UNBOUND;
    if (args==NIL) return UNBOUND;
    while (is_list(car(args)) && cdr(args) != NIL && NOT_THROWING)
	setcar(args, err_logo(BAD_DATA, car(args)));
    if (is_list(car(args)))
	args = car(args);
    while (args != NIL && NOT_THROWING) {
	arg = car(args);
	while (!is_word(arg) && NOT_THROWING) {
	    arg = err_logo(BAD_DATA, arg);
	    setcar(args, arg); /* prevent crash in lapply */
	}
	if (NOT_THROWING) {
	    arg = intern(arg);
	    setcar(args, arg); /* local [a b] faster next time */
	    if (not_local(arg,var_stack)) {
		push(arg, var_stack);
		if (flag__caseobj(arg, IS_LOCAL_VALUE))
		    settype(var_stack, LOCALSAVE);
		setobject(var_stack, valnode__caseobj(arg));
		setflag__caseobj(arg, IS_LOCAL_VALUE);
	    }
	    setvalnode__caseobj(arg, UNBOUND);
	    tell_shadow(arg);
	    args = cdr(args);
	}
	if (check_throwing) break;
    }
    return(UNBOUND);
}

NODE *lglobal(NODE *args) {
    NODE *arg = NIL;

    if (args==NIL) return UNBOUND;
    while (is_list(car(args)) && cdr(args) != NIL && NOT_THROWING)
	setcar(args, err_logo(BAD_DATA, car(args)));
    if (is_list(car(args)))
	args = car(args);
    while (args != NIL && NOT_THROWING) {
	arg = car(args);
	while (!is_word(arg) && NOT_THROWING) {
	    arg = err_logo(BAD_DATA, arg);
	    setcar(args, arg); /* prevent crash in lapply */
	}
	if (NOT_THROWING) {
	    arg = intern(arg);
	    setcar(args, arg); /* local [a b] faster next time */
	    setflag__caseobj(arg, HAS_GLOBAL_VALUE);
	    args = cdr(args);
	}
	if (check_throwing) break;
    }
    return(UNBOUND);
}

NODE *cnt_list = NIL;
NODE *cnt_last = NIL;
int want_buried = 0;

typedef enum {c_PROCS, c_VARS, c_PLISTS, c_PRIMS, c_PROCSnPRIMS} CNTLSTTYP;
CNTLSTTYP contents_list_type;

#ifdef OBJECTS4
/* For ancestry lists */
typedef enum {NORMAL, ANCESTRY} LSTFORM;
/* For type of list wanted */
typedef enum {ACCESSIBLE, OWNED, INHERITED} LSTTYP;

/* Depending on the current contents_list_type, this returns the variables
   or procedures of the current object. Signals error if plists  */
NODE *contentsListType(NODE *obj) {
    switch (contents_list_type) {
    case c_VARS:
        return getvars(obj);
    case c_PROCS:
        return getprocs(obj);
    case c_PLISTS:
        err_logo(BAD_DATA, make_static_strnode("objects don't have plists!"));
    return;
    }
}

void normal_special_contents_map(NODE *sym) {
    special_contents_map(sym, NORMAL);
}

void ancestry_special_contents_map(NODE *sym) {
    special_contents_map(sym, ANCESTRY);
}

void fmt_map_oblist(LSTFORM format) {
    if (format == NORMAL)
    map_oblist(normal_special_contents_map);
    else
    map_oblist(ancestry_special_contents_map);
}

/** new things **/
/* get_contents with special format and normal format */
NODE *get_special_contents(LSTFORM format, LSTTYP type) {
    cnt_list = NIL;
    cnt_last = NIL;

    if (current_object == logo_object)
        fmt_map_oblist(format);
    else {
        /* for accessible, owned, or inherited, the current object's
           vars/procs have to be in cnt_list */
        NODE *mylist = contentsListType(current_object); /* contains vars or procs of current object */
        while (mylist != NIL) {
            putname(caar(mylist), current_object, format);
            mylist = cdr(mylist);
        }
        /* do not need to add parents' vars/procs for owned, but do for
           accessible and inherited */
        if (type == ACCESSIBLE || type == INHERITED) {
        /* add your parents */
        NODE *parlst = parent_list(current_object);
        while (parlst != NIL) {
            if (car(parlst) == logo_object) {
                fmt_map_oblist(format);
            } else {
                mylist = contentsListType(car(parlst));
                while (mylist != NIL) {
                    putname(caar(mylist), car(parlst), format);
                    /* put name of var or proc, associated with parent object */
                    mylist = cdr(mylist);
                }
            }
            parlst = cdr(parlst);
        }
        /* if accessible, then remove the shadowed vars/procs */
        if (type == ACCESSIBLE) {
            cnt_list = removeShadowed(cnt_list);
        }
        }
    }
    return(cnt_list);
}

/* used for getting the normal plist contents */
NODE *get_plistcontents() {
    cnt_list = NIL;
    cnt_last = NIL;
    map_oblist(contents_map);
    cnt_list = mergesrt(cnt_list);
    return(cnt_list);
}

/* puts name or name with object into cnt_list */
void putname(NODE *name, NODE *obj, LSTFORM format) {
    NODE *newNode;

    if (format == NORMAL)
    newNode = name;
    else
    newNode = cons(name, cons(obj, NIL));

    if (cnt_list == NIL) {
        cnt_list = cons(newNode, NIL);
        cnt_last = cnt_list;
    } else {
        setcdr(cnt_last, newNode);
        cnt_last = cdr(cnt_last);
    }
}

/* contents_map for special format */
void special_contents_map(NODE *sym, LSTFORM format) {
    int flag_check = PROC_BURIED;

    if (want_buried) flag_check = want_buried;
    switch(contents_list_type) {
    case c_PROCS:
	check_library(sym);
        if (procnode__object(sym) == UNDEFINED ||
            is_prim(procnode__object(sym)))
        return;
        if (bck(flag__object(sym,flag_check))) return;
        break;
    case c_VARS:
        flag_check <<= 1;
        if (valnode__object(sym) == UNBOUND) return;
        if (bck(flag__object(sym,flag_check))) return;
        break;
    case c_PLISTS:
        err_logo(BAD_DATA, contents_list_type);
    }
    putname(canonical__object(sym), logo_object, format);
    }
}

/* checks if the car of the item is equal to the car of something in
   alist */
BOOLEAN carequal(NODE *item, NODE *alist) {
    while (alist != NIL) {
        if (car(item) == caar(alist)
            return true;
        alist = cdr(alist);
    }
    return false;
}
#endif

int bck(int flag) {
    return (want_buried ? !flag : flag);
}

void contents_map(NODE *sym) {
    int flag_check = PROC_BURIED;

    if (want_buried) flag_check = want_buried;
    switch(contents_list_type) {
	case c_PROCS:
	    check_library(sym);
	    if (procnode__object(sym) == UNDEFINED ||
			is_prim(procnode__object(sym)))
		return;
	    if (bck(flag__object(sym,flag_check))) return;
	    break;
	case c_PRIMS:
	    if (procnode__object(sym) == UNDEFINED ||
			!is_prim(procnode__object(sym)))
		return;
	    break;
	case c_PROCSnPRIMS:
	    check_library(sym);
	    if (procnode__object(sym) == UNDEFINED)
		return;
	    if (bck(flag__object(sym,flag_check))) return;
	    break;
	case c_VARS:
	    flag_check <<= 1;
	    if (valnode__object(sym) == UNBOUND) return;
	    if (bck(flag__object(sym,flag_check))) return;
	    break;
	case c_PLISTS:
	    flag_check <<= 2;
	    if (plist__object(sym) == NIL) return;
	    if (bck(flag__object(sym,flag_check))) return;
	    break;
    }
    if (cnt_list == NIL) {
	cnt_list = cons(canonical__object(sym), NIL);
	cnt_last = cnt_list;
    } else {
	setcdr(cnt_last, cons(canonical__object(sym), NIL));
	cnt_last = cdr(cnt_last);
    }
}

void ms_listlist(NODE *nd) {
    while (nd != NIL) {
	setcar(nd, cons(car(nd), NIL));
	nd = cdr(nd);
    }
}

NODE *merge(NODE *a, NODE *b) {
    NODE *ret, *tail;

    if (a == NIL) return(b);
    if (b == NIL) return(a);
    if (compare_node(car(a),car(b),FALSE) < 0) {
	ret = a;
	tail = a;
	a = cdr(a);
    } else {
	ret = b;
	tail = b;
	b = cdr(b);
    }

    while (a != NIL && b != NIL) {
	if (compare_node(car(a),car(b),FALSE) < 0) {
	    setcdr(tail, a);
	    a = cdr(a);
	} else {
	    setcdr(tail, b);
	    b = cdr(b);
	}
	tail = cdr(tail);
    }

    if (b == NIL) setcdr(tail, a);
    else setcdr(tail, b);

    return ret;
}

void mergepairs(NODE *nd) {
    while (nd != NIL && cdr(nd) != NIL) {
	setcar(nd, merge(car(nd), cadr(nd)));
	setcdr(nd, cddr(nd));
	nd = cdr(nd);
    }
}

NODE *mergesrt(NODE *nd) {    /* spelled funny to avoid library conflict */
    if (nd == NIL) return(NIL);
    if (cdr(nd) == NIL) return(nd);
    ms_listlist(nd);
    while (cdr(nd) != NIL)
	mergepairs(nd);
    return car(nd);
}

NODE *get_contents() {
    cnt_list = NIL;
    cnt_last = NIL;
    map_oblist(contents_map);
    cnt_list = mergesrt(cnt_list);
    return(cnt_list);
}

#ifdef OBJECTS4

/* calls to new special_contents */
NODE *lcontents(NODE *args) {
    NODE *ret;

    want_buried = 0;

    contents_list_type = c_PLISTS;
    ret = cons(get_plistcontents(), NIL);

    contents_list_type = c_VARS;
    push(get_special_contents(ACCESSIBLE, NORMAL), ret);

    contents_list_type = c_PROCS;
    push(get_special_contents(ACCESSIBLE, NORMAL), ret);

    cnt_list = NIL;
    return(ret);
}

NODE *lburied(NODE *args) {
    NODE *ret;

    want_buried = PROC_BURIED;

    contents_list_type = c_PLISTS;
    ret = cons(get_plistcontents(), NIL);

    contents_list_type = c_VARS;
    push(get_special_contents(INHERITED, ANCESTRY), ret);

    contents_list_type = c_PROCS;
    push(get_special_contents(INHERITED, ANCESTRY), ret);

    cnt_list = NIL;
    return(ret);
}

NODE *ltraced(NODE *args) {
    NODE *ret;

    want_buried = PROC_TRACED;

    contents_list_type = c_PLISTS;
    ret = cons(get_plistcontents(), NIL);

    contents_list_type = c_VARS;
    push(get_special_contents(INHERITED, ANCESTRY), ret); /* not sure: INHERITED? */

    contents_list_type = c_PROCS;
    push(get_special_contents(INHERITED, ANCESTRY), ret); /* not sure: INHERITED? */

    cnt_list = NIL;
    return(ret);
}

NODE *lstepped(NODE *args) {
    NODE *ret;

    want_buried = PROC_STEPPED;

    contents_list_type = c_PLISTS;
    ret = cons(get_plistcontents(), NIL);

    contents_list_type = c_VARS;
    push(get_special_contents(INHERITED, ANCESTRY), ret); /* not sure: INHERITED? */

    contents_list_type = c_PROCS;
    push(get_special_contents(INHERITED, ANCESTRY), ret); /* not sure: INHERITED? */

    cnt_list = NIL;
    return(ret);
}

NODE *lprocedures(NODE *args) {
    NODE *ret;

    want_buried = 0;

    contents_list_type = c_PROCS;
    ret = get_special_contents(ACCESSIBLE, NORMAL);
    cnt_list = NIL;
    return(ret);
}

NODE *lnames(NODE *args) {
    NODE *ret;

    want_buried = 0;

    contents_list_type = c_VARS;
    ret = cons(NIL, cons(get_special_contents(ACCESSIBLE, NORMAL), NIL));
    cnt_list = NIL;
    return(ret);
}

NODE *lplists(NODE *args) {
    NODE *ret;

    want_buried = 0;

    contents_list_type = c_PLISTS;
    ret = cons(NIL, cons(NIL, cons(get_plistcontents(), NIL)));
    cnt_list = NIL;
    return(ret);
}

#else /* OBJECTS */

NODE *lcontents(NODE *args) {
    NODE *ret;

    want_buried = 0;

    contents_list_type = c_PLISTS;
    ret = cons(get_contents(), NIL);

    contents_list_type = c_VARS;
    push(get_contents(), ret);

    contents_list_type = c_PROCS;
    push(get_contents(), ret);

    cnt_list = NIL;
    return(ret);
}

NODE *lburied(NODE *args) {
    NODE *ret;

    want_buried = PROC_BURIED;

    contents_list_type = c_PLISTS;
    ret = cons(get_contents(), NIL);

    contents_list_type = c_VARS;
    push(get_contents(), ret);

    contents_list_type = c_PROCS;
    push(get_contents(), ret);

    cnt_list = NIL;
    return(ret);
}

NODE *ltraced(NODE *args) {
    NODE *ret;

    want_buried = PROC_TRACED;

    contents_list_type = c_PLISTS;
    ret = cons(get_contents(), NIL);

    contents_list_type = c_VARS;
    push(get_contents(), ret);

    contents_list_type = c_PROCSnPRIMS;
    push(get_contents(), ret);

    cnt_list = NIL;
    return(ret);
}

NODE *lstepped(NODE *args) {
    NODE *ret;

    want_buried = PROC_STEPPED;

    contents_list_type = c_PLISTS;
    ret = cons(get_contents(), NIL);

    contents_list_type = c_VARS;
    push(get_contents(), ret);

    contents_list_type = c_PROCS;
    push(get_contents(), ret);

    cnt_list = NIL;
    return(ret);
}

NODE *lprocedures(NODE *args) {
    NODE *ret;

    want_buried = 0;

    contents_list_type = c_PROCS;
    ret = get_contents();
    cnt_list = NIL;
    return(ret);
}

NODE *lnames(NODE *args) {
    NODE *ret;

    want_buried = 0;

    contents_list_type = c_VARS;
    ret = cons(NIL, cons(get_contents(), NIL));
    cnt_list = NIL;
    return(ret);
}

NODE *lplists(NODE *args) {
    NODE *ret;

    want_buried = 0;

    contents_list_type = c_PLISTS;
    ret = cons(NIL, cons(NIL, cons(get_contents(), NIL)));
    cnt_list = NIL;
    return(ret);
}

#endif /* OBJECTS */

NODE *lprimitives(NODE *args) {
    NODE *ret;

    want_buried = 0;

    contents_list_type = c_PRIMS;
    ret = get_contents();
    cnt_list = NIL;
    return(ret);
}

NODE *one_list(NODE *nd) {
    if (!is_list(nd))
	return(cons(nd,NIL));
    return nd;
}

void three_lists(NODE *arg, NODE **proclst, NODE **varlst, NODE **plistlst) {
    if (nodetype(car(arg)) == CONS)
	arg = car(arg);

    if (!is_list(car(arg)))
	*proclst = arg;
    else {
	*proclst = car(arg);
	if (cdr(arg) != NIL) {
	    *varlst = one_list(cadr(arg));
	    if (cddr(arg) != NIL) {
		*plistlst = one_list(car(cddr(arg)));
	    }
	}
    }
    if (!is_list(*proclst) || !is_list(*varlst) || !is_list(*plistlst)) {
	err_logo(BAD_DATA_UNREC,arg);
	*plistlst = *varlst = *proclst = NIL;
    }
}

char *expand_slash(NODE *wd) {
	char *result, *cp, *cp2;
	int i, len = getstrlen(wd), j;

	for (cp = getstrptr(wd), i=0, j = len; --j >= 0; )
		if (getparity(*cp++)) i++;
	result = malloc(len+i+1);
	if (result == NULL) {
	    err_logo(OUT_OF_MEM, NIL);
	    return 0;
	}
	for (cp = getstrptr(wd), cp2 = result, j = len; --j >= 0; ) {
		if (getparity(*cp)) *cp2++ = '\\';
		*cp2++ = clearparity(*cp++);
	}
	*cp2 = '\0';
	return result;
}

NODE *po_helper(NODE *arg, int just_titles) {
    /* just_titles is -1 for EDIT, 0 for PO, 1 for HELP, 3 for POT */
    NODE *proclst = NIL, *varlst = NIL, *plistlst = NIL, *tvar = NIL;
    NODE *plist, *oldfullp;

	oldfullp = valnode__caseobj(Fullprintp);
    setvalnode__caseobj(Fullprintp, theName(Name_true));

    three_lists(arg, &proclst, &varlst, &plistlst);

    while (proclst != NIL) {
	if (aggregate(car(proclst))) {
	    err_logo(BAD_DATA_UNREC, car(proclst));
	    break;
	} else {
	    check_library(intern(car(proclst)));
	    tvar = procnode__caseobj(intern(car(proclst)));
	}

	if (tvar == UNDEFINED) {
	    if (just_titles < 0) {
		ndprintf(writestream,message_texts[EMPTY_PROC],car(proclst));
	    } else {
		err_logo(DK_HOW_UNREC, car(proclst));
		break;
	    }
	} else if (nodetype(tvar) & NT_PRIM) {
	    err_logo(IS_PRIM, car(proclst));
	    break;
	} else {
	    tvar = get_bodywords(tvar,car(proclst));
	    if (just_titles > 2) {
		if (is_list(car(tvar)))
			print_nobrak(writestream, car(tvar));
		else {
			char *str = expand_slash(car(tvar));
			ndprintf(writestream, "%t", str);
			free(str);
		}
	    } else while (tvar != NIL) {
			if (is_list(car(tvar))) {
				if (just_titles == 2) break;
				print_nobrak(writestream, car(tvar));
			} else {
				char *str = expand_slash(car(tvar));
				if (just_titles == 2 && *str != ';') break;
				ndprintf(writestream, "%t", str);
				free(str);
			}
			new_line(writestream);
			tvar = cdr(tvar);
			if (just_titles == 1) just_titles++;
	    }
	    new_line(writestream);
	}
	proclst = cdr(proclst);
	if (check_throwing) break;
    }

    while (varlst != NIL && NOT_THROWING) {
	if (aggregate(car(varlst))) {
	    err_logo(BAD_DATA_UNREC, car(varlst));
	    break;
	} else
	    tvar = maybe_quote(valnode__caseobj(intern(car(varlst))));

	if (tvar == UNBOUND) {
	    if (just_titles >= 0) {
		err_logo(NO_VALUE, car(varlst));
		break;
	    }
	} else {
	    ndprintf(writestream, message_texts[TRACE_MAKE],
		     make_quote(car(varlst)), tvar);
	    new_line(writestream);
	}
	varlst = cdr(varlst);
	if (check_throwing) break;
    }

    while (plistlst != NIL && NOT_THROWING) {
	if (aggregate(car(plistlst))) {
	    err_logo(BAD_DATA_UNREC, car(plistlst));
	    break;
	} else {
	    plist = plist__caseobj(intern(car(plistlst)));
	    if (plist != NIL && just_titles > 0) {
		ndprintf(writestream, message_texts[POT_PLIST],
			 maybe_quote(car(plistlst)), plist);
	    } else while (plist != NIL) {
		ndprintf(writestream, "%t %s %s %s\n",
			 message_texts[TRACE_PPROP],
			 maybe_quote(car(plistlst)),
			 maybe_quote(car(plist)),
			 maybe_quote(cadr(plist)));
		plist = cddr(plist);
	    }
	}
	plistlst = cdr(plistlst);
	if (check_throwing) break;
    }

    setvalnode__caseobj(Fullprintp, oldfullp);
    return(UNBOUND);
}

NODE *lpo(NODE *arg) {
    return(po_helper(arg,0));
}

NODE *lpot(NODE *arg) {
    return(po_helper(arg,3));
}

NODE *lerase(NODE *arg) {
    NODE *proclst = NIL, *varlst = NIL, *plistlst = NIL;
    NODE *nd, *what;
    int redef = (varTrue(Redefp));

    three_lists(arg, &proclst, &varlst, &plistlst);

    if (proclst != NIL)
	the_generation = cons(NIL, NIL);

    while (proclst != NIL) {
	if (aggregate(car(proclst))) {
	    err_logo(BAD_DATA_UNREC, car(proclst));
	    break;
	}
	nd = intern(car(proclst));
	if (!redef && is_prim(procnode__caseobj(nd))) {
	    err_logo(IS_PRIM, nd);
	    break;
	}
	setprocnode__caseobj(nd, UNDEFINED);
	proclst = cdr(proclst);
    }

    while (varlst != NIL && NOT_THROWING) {
	if (aggregate(car(varlst))) {
	    err_logo(BAD_DATA_UNREC, car(varlst));
	    break;
	}
	what = intern(car(varlst));
	setvalnode__caseobj(what, UNBOUND);
	if (!flag__caseobj(what, IS_LOCAL_VALUE))
	    clearflag__caseobj(what, HAS_GLOBAL_VALUE);
	varlst = cdr(varlst);
    }

    while (plistlst != NIL && NOT_THROWING) {
	if (aggregate(car(plistlst))) {
	    err_logo(BAD_DATA_UNREC, car(plistlst));
	    break;
	}
	setplist__caseobj(intern(car(plistlst)), NIL);
	plistlst = cdr(plistlst);
    }
    return(UNBOUND);
}

NODE *erall_helper(BOOLEAN procs, BOOLEAN vals, BOOLEAN plists) {
    NODE *nd, *obj;
    int loop;
    int redef = (varTrue(Redefp));

    for (loop = 0; loop < HASH_LEN ; loop++) {
	for (nd = hash_table[loop]; nd != NIL; nd = cdr(nd)) {
	    obj = car(nd);
	    if (procs && !flag__object(obj, PROC_BURIED) &&
			(procnode__object(obj) != UNDEFINED) &&
			(redef || !is_prim(procnode__object(obj))))
		setprocnode__object(obj, UNDEFINED);
	    if (vals && !flag__object(obj, VAL_BURIED))
		setvalnode__object(obj, UNBOUND);
	    if (plists && !flag__object(obj, PLIST_BURIED))
		setplist__object(obj, NIL);
	}
    }
    return UNBOUND;
}

NODE *lerall(NODE *args) {
    return erall_helper(TRUE, TRUE, TRUE);
}

NODE *lerps(NODE *args) {
    return erall_helper(TRUE, FALSE, FALSE);
}

NODE *lerns(NODE *args) {
    return erall_helper(FALSE, TRUE, FALSE);
}

NODE *lerpls(NODE *args) {
    return erall_helper(FALSE, FALSE, TRUE);
}

NODE *bury_helper(NODE *arg, BOOLEAN flag, BOOLEAN setp) {
    NODE *proclst = NIL, *varlst = NIL, *plistlst = NIL;

    three_lists(arg, &proclst, &varlst, &plistlst);

    while (proclst != NIL) {
	if (aggregate(car(proclst))) {
	    err_logo(BAD_DATA_UNREC, car(proclst));
	    break;
	}
	if (setp)
	    setflag__caseobj(intern(car(proclst)), flag);
	else
	    return torf(flag__caseobj(intern(car(proclst)), flag));
	proclst = cdr(proclst);
	if (check_throwing) break;
    }

    flag <<= 1;
    while (varlst != NIL && NOT_THROWING) {
	if (aggregate(car(varlst))) {
	    err_logo(BAD_DATA_UNREC, car(varlst));
	    break;
	}
	if (setp)
	    setflag__caseobj(intern(car(varlst)), flag);
	else
	    return torf(flag__caseobj(intern(car(varlst)), flag));
	varlst = cdr(varlst);
	if (check_throwing) break;
    }

    flag <<= 1;
    while (plistlst != NIL && NOT_THROWING) {
	if (aggregate(car(plistlst))) {
	    err_logo(BAD_DATA_UNREC, car(plistlst));
	    break;
	}
	if (setp)
	    setflag__caseobj(intern(car(plistlst)), flag);
	else
	    return torf(flag__caseobj(intern(car(plistlst)), flag));
	plistlst = cdr(plistlst);
	if (check_throwing) break;
    }
    if (!setp) err_logo(BAD_DATA_UNREC, NIL);
    return(UNBOUND);
}

NODE *lbury(NODE *arg) {
    return bury_helper(arg,PROC_BURIED,TRUE);
}

NODE *ltrace(NODE *arg) {
    return bury_helper(arg,PROC_TRACED,TRUE);
}

NODE *lstep(NODE *arg) {
    return bury_helper(arg,PROC_STEPPED,TRUE);
}

NODE *lburiedp(NODE *arg) {
    return bury_helper(arg,PROC_BURIED,FALSE);
}

NODE *ltracedp(NODE *arg) {
    return bury_helper(arg,PROC_TRACED,FALSE);
}

NODE *lsteppedp(NODE *arg) {
    return bury_helper(arg,PROC_STEPPED,FALSE);
}

NODE *unbury_helper(NODE *arg, int flag) {
    NODE *proclst = NIL, *varlst = NIL, *plistlst = NIL;

    three_lists(arg, &proclst, &varlst, &plistlst);

    while (proclst != NIL) {
	if (aggregate(car(proclst))) {
	    err_logo(BAD_DATA_UNREC, car(proclst));
	    break;
	}
	clearflag__caseobj(intern(car(proclst)), flag);
	proclst = cdr(proclst);
	if (check_throwing) break;
    }

    flag <<= 1;
    while (varlst != NIL && NOT_THROWING) {
	if (aggregate(car(varlst))) {
	    err_logo(BAD_DATA_UNREC, car(varlst));
	    break;
	}
	clearflag__caseobj(intern(car(varlst)), flag);
	varlst = cdr(varlst);
	if (check_throwing) break;
    }

    flag <<= 1;
    while (plistlst != NIL && NOT_THROWING) {
	if (aggregate(car(plistlst))) {
	    err_logo(BAD_DATA_UNREC, car(plistlst));
	    break;
	}
	clearflag__caseobj(intern(car(plistlst)), flag);
	plistlst = cdr(plistlst);
	if (check_throwing) break;
    }
    return(UNBOUND);
}

NODE *lunbury(NODE *arg) {
    return unbury_helper(arg,PROC_BURIED);
}

NODE *luntrace(NODE *arg) {
    return unbury_helper(arg,PROC_TRACED);
}

NODE *lunstep(NODE *arg) {
    return unbury_helper(arg,PROC_STEPPED);
}

char *addsep(char *path) {
    static char result[256];

    strcpy(result, path);
    if (result[0]) strcat(result, separator);
    return result;
}

char tmp_filename[500] = "";
int isEditFile = 0;
int setTermInfo(int type, int val);

NODE *leditfile(NODE *args) {

    NODE *arg = cnv_node_to_strnode(car(args));

	#ifdef HAVE_WX
	setTermInfo(EDIT_STATE, NO_INFO);
	isEditFile=1;
	#endif
	
    if (NOT_THROWING) {
    	noparity_strnzcpy(tmp_filename, getstrptr(arg), getstrlen(arg));
	return ledit(NIL);
    } else
    	return UNBOUND;
}

NODE *ledit(NODE *args) {
    FILE *holdstrm;
#ifdef HAVE_WX
	int doSave;
	int getTermInfo(int);
#endif
#ifdef unix
#ifndef HAVE_UNISTD_H
    extern int getpid();
#endif
#endif
#ifdef __RZTC__
    BOOLEAN was_graphics;
#endif
    NODE *tmp_line = NIL, *exec_list = NIL;

    if (tmp_filename[0] == '\0' || args != NIL) {
#ifndef unix
	sprintf(tmp_filename, "%stemp.txt", addsep(tempdir));
#else
	sprintf(tmp_filename, "%s/logo%d", tempdir, (int)getpid());
#endif
    }
#ifdef HAVE_WX
	if(!isEditFile){
		setTermInfo(EDIT_STATE,DO_LOAD);
	}
	isEditFile=0;
#endif

    if (args != NIL) {
	holdstrm = writestream;
	writestream = fopen(tmp_filename, "w");
	if (writestream != NULL) {
	    po_helper(args,-1);
	    fclose(writestream);
	    writestream = holdstrm;
	} else {
	    err_logo(FILE_ERROR,
	      make_static_strnode("Could not create editor file"));
	    writestream = holdstrm;
	    return(UNBOUND);
	}
    }
    if (stopping_flag == THROWING) return(UNBOUND);
#ifdef mac
    if (!mac_edit()) return(UNBOUND);
#else	    /* !mac */
#ifdef  HAVE_WX
	doSave = wxEditFile(tmp_filename);
    if(!doSave || getTermInfo(EDIT_STATE) != DO_LOAD)
	return(UNBOUND);
#else
#ifdef ibm
#ifdef __RZTC__
    was_graphics = in_graphics_mode;
    if (in_graphics_mode) t_screen();
    zflush();
#endif	/* ztc */
    if (spawnlp(P_WAIT, editor, editorname, tmp_filename, NULL)) {
	err_logo(FILE_ERROR, make_static_strnode
		 ("Could not launch the editor"));
	return(UNBOUND);
    }
#ifdef __RZTC__
    if (was_graphics) s_screen();
    else lcleartext(NIL);
#endif	/* ztc */
#ifdef WIN32
    win32_repaint_screen();
#endif
#else	/* !ibm (so unix) */
    if (fork() == 0) {
	execlp(editor, editorname, tmp_filename, 0);
	exit(1);
    }
    wait(0);
#endif	/* ibm */
#endif /* wx */
#endif	/* mac */
    holdstrm = loadstream;
    tmp_line = current_line;
    loadstream = fopen(tmp_filename, "r");
    if (loadstream != NULL) {
	while (!feof(loadstream) && NOT_THROWING) {
	    current_line = reader(loadstream, "");
	    exec_list = parser(current_line, TRUE);
	    if (exec_list != NIL) eval_driver(exec_list);
	}
	fclose(loadstream);
    } else
	err_logo(FILE_ERROR,
	      make_static_strnode("Could not read editor file"));
    loadstream = holdstrm;
    current_line = tmp_line;
    return(UNBOUND);
}

NODE *lthing(NODE *args) {
    NODE *val = UNBOUND, *arg;

    arg = name_arg(args);
#ifdef OBJECTS
    if (NOT_THROWING) val = varValue(arg);
#else
    if (NOT_THROWING) val = valnode__caseobj(intern(arg));
#endif
    while (val == UNBOUND && NOT_THROWING)
	val = err_logo(NO_VALUE, car(args));
    return(val);
}

NODE *lnamep(NODE *args) {
    NODE *arg;

    arg = name_arg(args);
    if (NOT_THROWING) 
#ifdef OBJECTS
	return torf(varValue(arg) != UNBOUND);
#else
	return torf(valnode__caseobj(intern(arg)) != UNBOUND);
#endif
    return UNBOUND;
}

NODE *lprocedurep(NODE *args) {
    NODE *arg;

    arg = name_arg(args);
    if (NOT_THROWING) {
	arg = intern(arg);
	check_library(arg);
	return torf(procnode__caseobj(arg) != UNDEFINED);
    }
    return UNBOUND;
}

NODE *lplistp(NODE *args) {
    NODE *arg;

    arg = name_arg(args);
    if (NOT_THROWING) 
	return torf(plist__caseobj(intern(arg)) != NIL);
    return UNBOUND;
}

NODE *check_proctype(NODE *args, int wanted) {
    NODE *arg, *cell = NIL;
    int isprim;

    arg = proc_name_arg(args);
    if (NOT_THROWING) check_library(intern(arg));
    if (NOT_THROWING &&
	    (cell = procnode__caseobj(intern(arg))) == UNDEFINED) {
	return(FalseName());
    }
    if (wanted == 2) return torf(is_macro(intern(arg)));
    isprim = is_prim(cell);
    if (NOT_THROWING) return torf((isprim != 0) == wanted);
    return(UNBOUND);
}

NODE *lprimitivep(NODE *args) {
    return(check_proctype(args,1));
}

NODE *ldefinedp(NODE *args) {
    return(check_proctype(args,0));
}

NODE *lmacrop(NODE *args) {
    return(check_proctype(args,2));
}

NODE *larity(NODE *args) {
    NODE *arg = proc_name_arg(args);
    FIXNUM min;

    if (NOT_THROWING) {
	arg = intern(arg);
	check_library(arg);
	arg = procnode__caseobj(arg);
	if is_prim(arg) {
	    min = getprimmin(arg);
	    if (min == OK_NO_ARG) min = 0;
	    return cons_list(0, make_intnode(min),
			     make_intnode(getprimdflt(arg)),
			     make_intnode(getprimmax(arg)), END_OF_LIST);
	} else if (arg == UNDEFINED) {
	    err_logo(DK_HOW_UNREC, car(args));
	    return UNBOUND;
	} else {
	    return cons_list(0, minargs__procnode(arg),
			     dfltargs__procnode(arg),
			     maxargs__procnode(arg), END_OF_LIST);
	}
    }
    return UNBOUND;
}

NODE *cpdf_newname(NODE *name, NODE*titleline) {
    NODE *nname=cnv_node_to_strnode(name);
    char *namestr=getstrptr(nname);
    char *titlestr=getstrptr(titleline);
    char buf[2000];
    char *p1, *p2;

    p1 = titlestr+strcspn(titlestr, " \t");
    p1 = p1+strspn(p1, " \t");
    p2 = p1+strcspn(p1, " \t");
    sprintf(buf, "%.*s%.*s%s",
	    p1-titlestr, titlestr, getstrlen(nname), namestr, p2);
    return make_strnode(buf, NULL, strlen(buf), STRING, strcpy);
}

NODE *lcopydef(NODE *args) {
    NODE *arg1, *arg2;
    int redef = (varTrue(Redefp));
    int old_default = -1, new_default;

    arg1 = proc_name_arg(args);
    arg2 = proc_name_arg(cdr(args));
    if (NOT_THROWING) {
	arg1 = intern(arg1);
	arg2 = intern(arg2);
	check_library(arg2);
    }
    if (NOT_THROWING && procnode__caseobj(arg2) == UNDEFINED)
	err_logo(DK_HOW, arg2);
    if (NOT_THROWING && !redef && is_prim(procnode__caseobj(arg1)))
	err_logo(IS_PRIM, arg1);
    if (NOT_THROWING) {
	NODE *old_proc = procnode__caseobj(arg1);
	NODE *new_proc = procnode__caseobj(arg2);
	if (old_proc != UNDEFINED) {
	    old_default = (is_prim(old_proc) ? getprimdflt(old_proc) :
		  		          getint(dfltargs__procnode(old_proc)));
	    }
	new_default = (is_prim(new_proc) ? getprimdflt(new_proc) :
					   getint(dfltargs__procnode(new_proc)));
	if (old_default != new_default && old_default >= 0) {
	    the_generation = cons(NIL, NIL);
	}
	if (is_prim(new_proc))
	    setprocnode__caseobj(arg1, new_proc);
	else {
	    NODE *bwds=get_bodywords(new_proc,arg1);	/* 5.5 */
	    setprocnode__caseobj(arg1,
		make_procnode(text__procnode(new_proc),
		    cons(cpdf_newname(arg1,car(bwds)),
			 cdr(bwds)),
		    getint(minargs__procnode(new_proc)),
		    getint(dfltargs__procnode(new_proc)),
		    getint(maxargs__procnode(new_proc))));
	}
	/* setflag__caseobj(arg1, PROC_BURIED); */
	if (is_macro(arg2)) setflag__caseobj(arg1, PROC_MACRO);
	else clearflag__caseobj(arg1, PROC_MACRO);
	if (flag__caseobj(arg2, PROC_SPECFORM))
	    setflag__caseobj(arg1, PROC_SPECFORM);
	else clearflag__caseobj(arg1, PROC_SPECFORM);
    }
    return(UNBOUND);
}
  
char *fixhelp(char *ptr, int len) {
    static char result[32];
    char *p, c;
    for (p = result; --len >= 0;  *p++ = c) {
        c = *ptr++;
        if (c == '?')
            c = 'p';
        else if (c == '.')
            c = 'd';
    }
    *p = '\0';
    return result;
}

char inops[] = "+-*/=<>";

NODE *lhelp(NODE *args) {
    NODE *arg = NIL, *pproc;
	char buffer[1024];
#ifndef WIN32
    char junk[20];
#endif
    FILE *fp;
    int lines;
#ifdef __RZTC__
    size_t len;
#endif

    if (args == NIL) {
/*
 #ifdef WIN32
	sprintf(buffer, "%sHELPCONT", addsep(helpfiles));
 #else
 */
	sprintf(buffer, "%sHELPCONTENTS", addsep(helpfiles));
/* #endif */
    } else if (is_word(car(args)) && car(args) != Null_Word) {
        arg = llowercase(args);
    /*	setcar(args, arg);  */
	if (getstrlen(arg) == 1) {
	    char *cp = strchr(inops,*(getstrptr(arg)));
	    if (cp != NULL) {
		arg=cnv_node_to_strnode(theName(Name_sum+(cp-inops)));
	    }
	}
	if (getstrlen(arg) == 2) {
	    if (!strncmp(getstrptr(arg), "<=", 2))
		arg=cnv_node_to_strnode(theName(Name_lessequalp));
	    if (!strncmp(getstrptr(arg), ">=", 2))
		arg=cnv_node_to_strnode(theName(Name_greaterequalp));
	    if (!strncmp(getstrptr(arg), "<>", 2))
		arg=cnv_node_to_strnode(theName(Name_notequalp));
	}
	//sprintf(result, "%s", fixhelp(getstrptr(arg), getstrlen(arg)));
	//sprintf(buffer, "%s%s",  addsep(helpfiles), result);
	
 sprintf(buffer, "%s%s", addsep(helpfiles),
		fixhelp(getstrptr(arg), getstrlen(arg)));
			//printf("Buffer: %s\n", buffer);

#ifdef __RZTC__    /* defined(ibm) || defined(WIN32) */
	if (strlen(buffer) > (len = strlen(addsep(helpfiles))+8)) {
	    buffer[len+5] = '\0';
	    buffer[len+4] = buffer[len+3];
	    buffer[len+3] = buffer[len+2];
	    buffer[len+2] = buffer[len+1];
	    buffer[len+1] = buffer[len];
	    buffer[len] = '.';
	}
#endif
    } else {
        err_logo(BAD_DATA_UNREC, car(args));
	return UNBOUND;
    }
    fp = fopen(buffer, "r");
    if (fp == NULL) {
	if (args == NIL)
	    ndprintf(writestream, message_texts[NO_HELP]);
	else {
	    check_library(intern(car(args)));
	    pproc = procnode__caseobj(intern(car(args)));
	    if (is_list(pproc)) {
		po_helper(args, 1);
	    }
	    else
		ndprintf(writestream, message_texts[NO_HELPON], arg);
	}
    } else {
	(void)ltextscreen(NIL);
	lines = 0;
	fgets(buffer, 200, fp);
	while (NOT_THROWING && !feof(fp)) {
#ifdef HAVE_WX
		int getTermInfo(int val);
		if (interactive && writestream==stdout && ++lines >= getTermInfo(Y_MAX)) {
#else
	    if (interactive && writestream==stdout && ++lines >= y_max) {
#endif
		ndprintf(writestream, message_texts[MORE_HELP]);
		input_blocking++;
#ifndef TIOCSTI
		if (!setjmp(iblk_buf))
#endif
#ifdef __RZTC__
		    ztc_getcr();
		    print_char(stdout, '\n');
#else
#ifdef WIN32
		    (void)reader(stdin, "");
#else
		    fgets(junk, 19, stdin);
#endif
#endif
		input_blocking = 0;
		update_coords('\n');
		lines = 1;
	    }
	    ndprintf(writestream, "%t", buffer);
	    fgets(buffer, 200, fp);
	}
	fclose(fp);
    }
    return UNBOUND;
}
