/*
 *      obj.c          logo object functions module              scc
 *
 *  Copyright (C) 1993 by the Regents of the University of California
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

#ifdef OBJECTS

NODE *logo_object, *current_object;
NODE *name_arg(NODE *args);
NODE *assoc(NODE *name, NODE *alist);

FIXNUM gensymnum = 1;

/* Creates new license plate for object */
NODE *newplate(void) {
    char buffer[20];
    sprintf(buffer,"G%d", gensymnum++);
    return make_strnode(buffer, NULL, strlen(buffer), STRING, strnzcpy);
    return UNBOUND;
}

/* Creates a new object */
NODE *newobj(void) {
    NODE *result = newnode(OBJECT);
    NODE *binding = newnode(CONS);
    setcar(binding, theName(Name_licenseplate));
    setobject(binding, newplate());
    setvars(result, binding);
    return result;
}

/* Initializes Object Logo */
void obj_init(void) {
    logo_object = newobj();
    current_object = logo_object;
    setvars(logo_object, cons(theName(Name_name), getvars(logo_object)));
    setobject(getvars(logo_object), make_static_strnode("Logo"));

    /* [[initlist] [exist output self]] */
    askexist = cons(cons(theName(Name_initlist), NIL),
		    cons(cons(theName(Name_exist),
			      cons(theName(Name_output),
				   cons(theName(Name_self), NIL))),
			 NIL));
}

/* Outputs the Logo object, ancestor of all other objects.
 * @params - none
 */
NODE *llogo(NODE *args) {
    return(logo_object);
}

/* Creates and outputs an object whose parent is the Logo object.
 * @params - none
 */
NODE *lsomething(NODE *args) {
    NODE *val;

    val = newobj();
    setparents(val, cons(logo_object, NIL));
    return val;
}

/* Creates and outputs an object, whose parent is Object, or whose parents
 * are Object1, Object2, etc., or the elements of ObjectList.
 * @params - Object or ObjectList or (Object1 Object2 etc.)
 */

NODE *lkindof(NODE *args) {
    NODE *argcopy = args;
    NODE *val = UNBOUND;

    if (is_list(car(args))) {
        if (cdr(args) != NIL) {
            err_logo(TOO_MUCH, NIL); /* too many inputs */
        }
        args = car(args);
    }
    /* now args is always a list of objects */
    /* make sure they're all really objects */

    for (argcopy = args; (argcopy != NIL && NOT_THROWING); 
			 argcopy = cdr(argcopy)) {
        while (!is_object(car(argcopy)) && NOT_THROWING) {
            setcar(argcopy, err_logo(BAD_DATA, car(argcopy)));
        }
    }

    if (NOT_THROWING) {
        val = newobj();
        setparents(val, args);
    }

    return val;
}

/* Creates an object whose parent is Object or whose parents are the elements
 * of ObjectList, Asks the new object to Exist, and then outputs the object.
 * Any remaining inputs after the first are collected into a list and made
 * the value of the public variable InitList, used to initialize the newly
 * created object.
 * @params - Object or ObjectList or (Object Input1 Input2 etc.) or
 * (ObjectList Input1 Input2 etc.)
 */

NODE *loneof(NODE *args) {
    NODE *val = UNBOUND, *argcopy;

    if (!is_list(car(args))) {
        setcar(args, cons(car(args), NIL));
    }
    /* now the first arg is always a list of objects */

    /* make sure they're really objects */
    argcopy = car(args);
    while (argcopy != NIL && NOT_THROWING) {
        while (!is_object(car(argcopy)) && NOT_THROWING) {
            setcar(argcopy, err_logo(BAD_DATA, car(argcopy)));
        }
        argcopy = cdr(argcopy);
    }

    if (NOT_THROWING) {
        val = newobj();
        setparents(val, car(args));

        /* apply [[InitList] [Exist Output Self]] cdr(args) */
	return make_cont(withobject_continuation,
			 cons(val,
			      make_cont(begin_apply,
					cons(askexist,
					     cons(cons(cdr(args), NIL),
						  NIL)))));
    }

    return val;
}

/* Each object will be given an Exist method by the user.
 * The global Exist does nothing at all.
 */
NODE *lexist(NODE *args) {
    return UNBOUND;
}

/* Creates the object variable named Symbol, or the object variables named                ,
 * SymbolList within the current object.
 * @params - Symbol or SymbolList
 */
NODE *lhave(NODE *args) {

    if (is_list(car(args))) {
        if (cdr(args) != NIL) {
            err_logo(TOO_MUCH, NIL); /* too many inputs */
        }
        args = car(args);
    }
    /* now args is always a list of symbols. args should not equal to NIL
       because that is checked for before. */

    while (args != NIL && NOT_THROWING) {
        NODE *sym = intern(car(args));
        NODE *binding = assoc(sym, getvars(current_object));
        if (binding == NIL) {
            setvars(current_object, cons(sym, getvars(current_object)));
            setobject(getvars(current_object), UNBOUND);
        }
        args = cdr(args);
    }

    return UNBOUND;
}

/* USUAL.FOO has to be recognized in paren.c as a reference to the FOO method
   in the parent(s), provided we are in the body of FOO, so that we know how
   many inputs it takes.  Then in eval.c we have to look for the parent(s)'
   FOO method.  So there is no lusual() -- it's not a Logo primitive */

/* Changes to Object the object in which subsequent top level instruction will
 * be run until the next time TalkTo is run
 * @params - Object
 */
NODE *ltalkto(NODE *args) {
    while (!is_object(car(args)) && NOT_THROWING)
	setcar(args, err_logo(BAD_DATA, car(args)));

    if (NOT_THROWING)
	current_object = car(args);

    return UNBOUND;
}

/* Runs RunList, with Object as the current object for the duration of the
 * Ask. After RunList finishes, the current object reverts to what it was
 * before the Ask.
 * @params - Object RunList
 */
NODE *lask(NODE *args) {
    while (!is_object(car(args)) && NOT_THROWING)
        setcar(args, err_logo(BAD_DATA, car(args)));

    if (NOT_THROWING) {
	return make_cont(withobject_continuation,
			 cons(car(args),
			      make_cont(begin_seq, cadr(args))));
    }

    return UNBOUND;
}

/* Outputs the current object (the object itself, not its name)
 * @params - none
 */
NODE *lself(NODE *args) {
    return current_object;
}

/* Outputs a list containing the parent(s) of the current Object. The Logo
 * Object has no parents.
 * @params - none
 */
NODE *lparents(NODE *args) {
    return getparents(current_object);
}

/* Outputs a list of the names of the object variables owned by (not
 * inherited by) the current object.
 * @params - none
 */
NODE *lmynames(NODE *args) {
    return getvars(current_object);
}

/* Outputs TRUE if Symbol is the name of an object variable owned by the
 * current object, FALSE otherwise.
 * @params - Symbol
 */
NODE *lmynamep(NODE *args) {
    NODE *arg;
    arg = name_arg(args);

    if (NOT_THROWING) {
        arg = intern(arg);

    if (current_object == logo_object) {
        return torf(flag__caseobj(arg, HAS_GLOBAL_VALUE));
    }

    else
        return torf(assoc(arg, getvars(current_object)) != NIL);
    }

    return UNBOUND;
}

/* Outputs a list of the names of the procedures owned by (not inherited
 * by) the current object.
 * @params - none
 */
NODE *lmyprocs(NODE *args) {
    return getprocs(current_object);
}

/* Outputs TRUE if Symbol is the name of a procedure owned by the current
 * object, FALSE otherwise.
 * @params - Symbol
 */
NODE *lmyprocp(NODE *args) {
    NODE *arg;

    if (current_object == logo_object)
        return lprocedurep(args); /* return lprocp or just call it? */
    else {
        arg = name_arg(args);
        if (NOT_THROWING)
          return torf(assoc(arg, getprocs(current_object)) != NIL);
    }

    return UNBOUND;
}

/* Outputs the object that owns the accessible variable named Symbol. If there
 * is no such accessible variable, or if it is not an object or global
 * variable, an error is signalled.
 * @params - Symbol
 */

/*
    to whosename :name
    if mynamep :name [output self]
    if equalp self Logo [(throw "error se [No accessible variable] :name)]
    output usual.whosename :name
    end
*/

NODE *lwhosename(NODE *args) {
    NODE *arg = name_arg(args);

    return varInThisObject(arg, TRUE);
}

/* Outputs the object that owns the accessible procedure named Symbol. If
 * there is no such accessible procedure, an error is signalled.
 * @params - Symbol
 */

/*
    to whoseproc :name
    if myprocp :name [output self]
    if equalp self Logo [(throw "error [No procedure named] :name)]
    output usual.whoseproc :name
    end
*/

/* Helper function */
NODE *assoc(NODE *name, NODE *alist) {
    while (alist != NIL) {
        if (compare_node(name, car(alist), TRUE) == 0)
            return alist;
        alist = cdr(alist);
    }
    return(NIL);
}

/* Looks up value in dynamic bindings and then in Object Hierarchy. If the
 * value is found in one place but not the other, then the value is
 * returned. Otherwise, an error is signalled.
 * @params - name
 */
NODE *varValue(NODE *name) {
    NODE *val;

    name = intern(name);
    val = valnode__caseobj(name);

    if ((val != UNBOUND) && flag__caseobj(name, IS_LOCAL_VALUE)) {
        if (varInObjectHierarchy(name, FALSE) != (NODE *)(-1)) {
            err_logo(LOCAL_AND_OBJ, name);
            return UNBOUND;
        }
        else {
            return val;    /* local binding */
        }
    }

    val = varInObjectHierarchy(name, TRUE);
    return((val == (NODE *)(-1)) ? UNBOUND : val);
}

/* Returns the value associated with name, depends on if the logo_object is
 * included in the hierarchy or not.
 */
NODE *varInObjectHierarchy(NODE *name, BOOLEAN includeLogo) {
    NODE *result, *parentList;

    result = assoc(name, getvars(current_object));

    if (result != NIL) {
        return getobject(result);
    }

    for (parentList = parent_list(current_object);
        parentList != NIL;
        parentList = cdr(parentList)) {
	    result = assoc(name, getvars(car(parentList)));
	    if (result != NIL) {
		return getobject(result);
	    }
    }

    if (!includeLogo) {
        return (NODE *)(-1);
    }

    result = intern(name);
    if (flag__caseobj(result, IS_LOCAL_VALUE)) {
        return (NODE *)(-1);
    }
    return valnode__caseobj(result);
}

/* Returns the object which contains the name, depends on if the
 * logo_object is included in the hierarchy or not.
 */
NODE *varInThisObject(NODE *name, BOOLEAN includeLogo) {
    NODE *object, *result, *parentList;

    result = assoc(name, getvars(current_object));
    if (result != NIL) return current_object;

    for (parentList = parent_list(current_object);
        parentList != NIL && result == NIL;
        parentList = cdr(parentList)) {
	    result = assoc(name, getvars(car(parentList)));
	    object = car(parentList);
	    if (result != NIL) return object;
    }

    if (!includeLogo) return NIL;

    result = intern(name);
    if (flag__caseobj(result, IS_LOCAL_VALUE)) return NIL;
    return logo_object;
}

/* Returns the procedure associated with name.
 */
NODE *procValue(NODE *name) {
    NODE *result, *parentList;

    result = assoc(name, getprocs(current_object));
    if (result != NIL) return getobject(result);

    for (parentList = parent_list(current_object);
        parentList != NIL && result == NIL;
        parentList = cdr(parentList)) {
    result = assoc(name, getprocs(car(parentList)));
    }
    if (result != NIL) return getobject(result);

    result = intern(name);
    return procnode__caseobj(result);
}

/*
(define (parent-list obj)
  (define (help obj)
    (let ((p (parents obj)))
      (if (null? p)
      (list obj)
      (cons obj (flatten (map parent-list p))))))
  (remdup (cdr (help obj))))
*/

extern NODE* remdup(NODE *seq);

NODE *parent_list_help(NODE *obj) {
    NODE *p, *out, *tail;

    out = tail = cons(obj, NIL);
    for (p = getparents(obj); p != NIL; p = cdr(p)) {
        setcdr(tail, parent_list_help(car(p)));
        while (cdr(tail) != NIL) tail = cdr(tail);
    }
    return out;
}
NODE *parent_list(NODE *obj) {
    return remdup(cdr(parent_list_help(obj)));
}

/*
(define (remdup seq)
  (cond ((null? seq) '())
    ((memq (car seq) (cdr seq)) (remdup (cdr seq)))
    (else (cons (car seq) (remdup (cdr seq))))))
*/

NODE* remdup(NODE *seq) {
    NODE* okay;
    
    if (seq == NIL)
        return seq;

    /* finds the first element of new seq list */
    while (memq(car(seq), cdr(seq))) {
        seq = cdr(seq);
    }

    for (okay = seq; cdr(okay) != NIL; okay = cdr(okay)) {
        while (memq(cadr(okay), cddr(okay))) {
            setcdr(okay, cddr(okay));
        }
    }

    return seq;
}

/* returns true if item is a member of list, returns false otherwise */
BOOLEAN memq(NODE *item, NODE *list) {
    while (list != NIL) {
        if (item == car(list))
            return TRUE;
        list = cdr(list);
    }
    return FALSE;
}

/* representation of an object */
NODE *lrepresentation(NODE *args) {
    NODE *license, *binding, *classbind;
    char buffer[200];
    char *old_stringptr = print_stringptr;
    int old_stringlen = print_stringlen;

    print_stringlen = 200;
    print_stringptr = buffer;

    license = assoc(theName(Name_licenseplate), getvars(current_object));

    ndprintf(NULL, "${Object %p", getobject(license));
    binding = assoc(theName(Name_name), getvars(current_object));
    if (binding != NIL && getobject(binding) != UNBOUND) {
	ndprintf(NULL, ": %p", getobject(binding));
    }
    classbind = assoc(theName(Name_class), getvars(current_object));
    if (classbind != NIL) {
	if (binding == NIL || getobject(binding) == UNBOUND) {
	    ndprintf(NULL, ":");
	}else {
	    ndprintf(NULL, ",");
	}
	ndprintf(NULL, " the class %p", getobject(classbind));
    } else {
        classbind = varInObjectHierarchy(theName(Name_class), FALSE);
        if (classbind != UNBOUND && classbind != (NODE *)(-1)) {
            if (binding == NIL) {
                ndprintf(NULL, ":");
            } else {
                ndprintf(NULL, ",");
            }
            ndprintf(NULL, " a %p", classbind);
        }
    }
    ndprintf(NULL, "}");
    print_stringptr* = '\0';
    print_stringptr = old_stringptr;
    print_stringlen = old_stringlen;
    return make_strnode(buffer, NULL, strlen(buffer), STRING, strnzcpy);
}

#endif
