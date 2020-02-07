/*
 *      obj.c          logo object functions module              scc
 *
 *  Copyright (C) 1993 by the Regents of the University of California
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
 *
 */

#include "logo.h"
#include "globals.h"

#ifdef OBJECTS

extern NODE *make_object(NODE *canonical, NODE *oproc, NODE *val,
			 NODE *plist, NODE *casestrnd);

NODE *logo_object, *current_object;
NODE *name_arg(NODE *args);
NODE *assoc(NODE *name, NODE *alist);
BOOLEAN memq(NODE *item, NODE *list);

FIXNUM gensymnum = 1;

/* Creates new license plate for object */
NODE *newplate(void) {
    char buffer[20];
    sprintf(buffer,"G%ld", gensymnum++);
    return make_strnode(buffer, NULL, strlen(buffer), STRING, strnzcpy);
    return UNBOUND;
}

/* -----------------------------
   OBJECT ADT 
   ----------------------------- */

/* Looks for a variable in the specified
   object only. Is aware of the special case
   where the OOP Object is Logo, in which it
   looks up the variable in the global hash table.
   
   name: a caseobj node
   oop_obj: an OOP object node
   returns: a hashtable entry, or UNBOUND
*/
NODE* get_var(NODE* name, NODE* oop_obj) {
  if (oop_obj == logo_object) {
    NODE* result = object__caseobj(name);
    
    // only non-local values should be considered
    // as part of the Logo object
    return result;
    /*
    if (!flag__object(result, IS_LOCAL_VALUE)
	&& flag__object(result, HAS_GLOBAL_VALUE)) {
      return result;
    }else{
      return UNBOUND;
    }
    */
  }else{
    // search the variable list for the name
    NODE* val = assoc(name, getvars(oop_obj));

    if(val == NIL){
      return UNBOUND;
    }else{
      return getobject(val);
    }
  }
}

NODE* get_varvalue(NODE* name, NODE* oop_obj){
  NODE* binding = get_var(name, oop_obj);
  if (binding != UNBOUND)
    return valnode__object(binding);
  else
    return binding;
}

/* Create a new variable in the OOP object
   which entails creating a new hashtable entry
   and adding it to the var list for the OOP 
   object. Like get_var, it is aware of the global
   Logo object, and will create the variable
   in the global hashtable.

   name: a caseobj node, or a string node (which will be interned)
   value: any node, or possibly UNBOUND
   oop_obj: an OOP object node
   returns: a newly created hashtable entry
 */
NODE* create_var(NODE* name, NODE* value, NODE* oop_obj) {
  // first intern the name, incase it's a string
  NODE *binding;
  NODE *sym = intern(name);
    
  if (oop_obj == logo_object){
    // because this is a logo symbol, the hash
    // entry will have been created as part of 
    // the intern() process, so we can look up
    // the entry right away
    binding = object__caseobj(sym);
  }else{    
    // now look up the caseobj in the OOP obj
    binding = get_var(sym, oop_obj);

    // if unbound, then bind it
    if (binding == UNBOUND) {
      NODE* lownd = make_strnode(getstrptr(strnode__caseobj(name)), 
				 (struct string_block *)NULL,
				 getstrlen(strnode__caseobj(name)), 
				 STRING, 
				 noparitylow_strnzcpy);

      binding = make_object(lownd, UNDEFINED, value, NIL,name);
      setvars(oop_obj, cons(lownd, getvars(oop_obj)));
      setobject(getvars(oop_obj), binding);

      // can return early because the binding is created
      // and the value has been set, so there is nothing
      // more to do
      return binding;
    }
  }

  setflag__object(binding, HAS_GLOBAL_VALUE);

  // TODO: does this mean that values can never be explicitly
  //       set to NIL?
  if (value){
    // assertion: this should work for Logo or generic OOP object
    setvalnode__object(binding, value);
  }      
  
  return binding;
}

/* Gets a procedure by name for a given object
   name: a caseobj node, the name of the procedure
   oop_obj: an OOP object node
   returns: either the procedure node or UNDEFINED  
 */
NODE* get_proc(NODE* name, NODE* oop_obj){
  if (oop_obj == logo_object){
    return procnode__caseobj(name);
  }else{
    // assoc returns a list spine pair, where the car
    // points to the proc's cannonical name and the
    // object points to the hash table entry, which
    // contains the proc-node internally
    NODE* proc = assoc(name, getprocs(oop_obj));
    if (proc == NIL){
      return UNDEFINED;
    }else{
      return procnode__object(getobject(proc));
    }
  }
}

NODE* set_proc(NODE* name, NODE* procnode, NODE* oop_obj){
  if (oop_obj != logo_object) {
    NODE* lownd = make_strnode(getstrptr(strnode__caseobj(name)), 
			       (struct string_block *)NULL,
			       getstrlen(strnode__caseobj(name)), 
			       STRING, 
			       noparitylow_strnzcpy);

    NODE* binding = make_object(lownd, procnode, UNBOUND, NIL,name);
    setprocs(oop_obj, cons(name, getprocs(oop_obj)));
    setobject(getprocs(oop_obj), binding); //procnode);
    return binding;
  }else{
    setprocnode__caseobj(name, procnode);
    return object__caseobj(name);
  }
}

// ----------------------------- //





/* Creates a new object */
NODE *newobj(void) {
	/* obj-upgrade:
	 *  this currently creates a new object with a linked list of 
	 *  variables. the new object layout will create a linked list
	 *  of hash table entries instead, and the first entry will be
	 *  the licenseplate of the object.
	 * 
	 * make_obj(can, proc, val, plist, casestrnd) can be used to
	 * to create a new hashtable entry
	 * the same binding code can be used, but it will 
	 *
	 * Questions: 
	 *   - Seems like parents should be uneffected, is this right?
	 */ 
    NODE *result = newnode(OBJECT);
    create_var(theName(Name_licenseplate), newplate(), result);

    //NODE *binding = newnode(CONS);
    //setcar(binding, theName(Name_licenseplate));
    //setobject(binding, newplate());
    //setvars(result, binding);
    return result;
}

/* Initializes Object Logo */
void obj_init(void) {
    // kludge: since logo_object is not set when newobj() is called
    //         the license plate gets created in the object instead of
    //         in the global hash table, so copy it over. The order
    //         of the following 3 lines is very important!
    current_object = newobj();
    NODE* logo_plate = get_varvalue(theName(Name_licenseplate), current_object);
    logo_object = current_object;

    // now copy over the license plate to the global hash-table
    create_var(theName(Name_licenseplate), logo_plate, logo_object);
    create_var(theName(Name_name), make_static_strnode("Logo"), logo_object);

    //setvars(logo_object, cons(theName(Name_name), getvars(logo_object)));
    //setobject(getvars(logo_object), make_static_strnode("Logo"));

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
      NODE *binding = get_var(sym, current_object);
      //NODE *binding = assoc(sym, getvars(current_object));
      if (binding == UNBOUND) {
	binding = create_var(sym, UNBOUND, current_object);
	//setvars(current_object, cons(sym, getvars(current_object)));
	//setobject(getvars(current_object), UNBOUND);
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

      return torf(get_var(arg, current_object) != UNBOUND);

      if (current_object == logo_object) {
        return torf(flag__caseobj(arg, HAS_GLOBAL_VALUE));
      }else{
	return torf(get_var(arg, current_object) != UNBOUND);
        //return torf(assoc(arg, getvars(current_object)) != NIL);
      }
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
  val = varInObjectHierarchy(name, TRUE);
  //get_varvalue(name, current_object); //valnode__caseobj(name);

  if ((val != UNBOUND) && flag__caseobj(name, IS_LOCAL_VALUE)) {
    if (varInObjectHierarchy(name, FALSE) != (NODE *)(-1)) {
      err_logo(LOCAL_AND_OBJ, name);
      return UNBOUND;
    }
    //    else {
    //  return val;    /* local binding */
    //}
  }

  return val;
  //val = varInObjectHierarchy(name, TRUE);
  //return((val == (NODE *)(-1)) ? UNBOUND : val);
}

/* Returns the value associated with name, depends on if the logo_object is
 * included in the hierarchy or not.
 */
NODE *varInObjectHierarchy(NODE *name, BOOLEAN includeLogo) {
    NODE *result, *parentList;

    result = get_var(name, current_object);
    //result = assoc(name, getvars(current_object));

    if (result != UNBOUND) {
      // i = includeLogo, c => cur = logo, result
      // i c, (i or !c)
      // 0 0, 1         don't include logo, c != logo
      // 0 1, 0         don't include logo, c = logo
      // 1 0, 1         include logo, c != logo wron
      // 1 1, 1         include logo, c = logo
      if (includeLogo || (current_object != logo_object)){
	return valnode__object(result);}}

    for (parentList = parent_list(current_object);
	 parentList != NIL;
	 parentList = cdr(parentList)) {
      
      result = get_var(name, car(parentList));
      if (result != UNBOUND){
	if (includeLogo || (car(parentList) != logo_object)){
	  return valnode__object(result);}}
    }

    return (NODE *)(-1);

    /*
    if (!includeLogo) {
        return (NODE *)(-1);
    }

    result = intern(name);
    if (flag__caseobj(result, IS_LOCAL_VALUE)) {
        return (NODE *)(-1);
    }
    return valnode__caseobj(result);
    */
}

/* Returns the object which contains the name, depends on if the
 * logo_object is included in the hierarchy or not.
 */
NODE *varInThisObject(NODE *name, BOOLEAN includeLogo) {
    NODE *object, *result, *parentList;

    result = get_var(name, current_object);

    //result = assoc(name, getvars(current_object));
    if (result != UNBOUND){
      if (includeLogo || (current_object != logo_object)){
	return current_object;}}


    for (parentList = parent_list(current_object);
	 parentList != NIL && result == UNBOUND;
	 parentList = cdr(parentList)) {

      object = car(parentList);
      result = get_var(name, object);

      if (result != UNBOUND){
	if (includeLogo || (object != logo_object)){
	  return object;}}

      //result = assoc(name, getvars(car(parentList)));
      //if (result != UNBOUND) return object;
    }

    return NIL;

    /*
    if (!includeLogo) return NIL;

    result = intern(name);
    if (flag__caseobj(result, IS_LOCAL_VALUE)) return NIL;
    return logo_object;
    */
}

/* Returns the procedure associated with name.
 */
extern void dbprint(NODE*);

/*
 * Finds an inherited procedure, returns it and also sets
 * the parent handle to the tail of the parent list the proc came from
 */
NODE *getInheritedProcWithParentList(NODE *name,
                                     NODE *objOrParents, NODE **parent){
    NODE *result;
    NODE *parentList;

    // initialize the return value
    // incase there are no parents
    result = UNDEFINED;

    if (NIL == objOrParents)
      return result;

    if (is_list(objOrParents)) {
      if (car(objOrParents) == logo_object) {
        result = get_proc(name, logo_object);
        if (parent != 0)
          *parent = objOrParents;
        return result;
      }
      
      parentList = cdr(objOrParents);
    } else {
      parentList = parent_list(objOrParents);
    }

    for (;
	 parentList != NIL && result == UNDEFINED;
	 parentList = cdr(parentList)) {
      result = get_proc(name, car(parentList));
      if (parent != 0){
	*parent = parentList;
      }
    }

    // result should be UNDEFINED or a proc at this point
    return result;
}

NODE *procValueWithParent(NODE *name, NODE *objOrParents, NODE **owner){
  NODE *result;
  NODE *obj, *parents;

  if (is_list(objOrParents)) {
    obj = car(objOrParents);
    parents = objOrParents;
  } else {
    obj = objOrParents;
    parents = objOrParents;
  }

  result = get_proc(name, obj);

  if (result != UNDEFINED) {
    if (owner != 0){
        *owner = parents;
    }
    return result;
  }
  
  // search all parents, will return UNDEFINED if not found
  return getInheritedProcWithParentList(name, objOrParents, owner);
}

NODE *procValue(NODE *name) {
  return procValueWithParent(name, current_object, (NODE**)0);
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

    license = get_varvalue(theName(Name_licenseplate), current_object); 
    //assoc(theName(Name_licenseplate), getvars(current_object));

    ndprintf(NULL, "${Object %p", license); //getobject(license));
    binding = get_varvalue(theName(Name_name), current_object); 
    //binding = assoc(theName(Name_name), getvars(current_object));
    if (binding != NIL && binding != UNBOUND) {
	ndprintf(NULL, ": %p", binding);
    }
    
    classbind = get_varvalue(theName(Name_class), current_object);
    //classbind = assoc(theName(Name_class), getvars(current_object));
    if (classbind != UNBOUND) {
	if (binding == UNBOUND) {
	    ndprintf(NULL, ":");
	}else {
	    ndprintf(NULL, ",");
	}
	ndprintf(NULL, " the class %p", classbind);
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
    /* print_stringptr = '\0'; */
    print_stringptr = old_stringptr;
    print_stringlen = old_stringlen;
    return make_strnode(buffer, NULL, strlen(buffer), STRING, strnzcpy);
}

static void objDump(const char *msg, NODE *obj) {
    fprintf(stderr,"%s: %p ", msg, obj);
    lprint(cons(obj,NIL));
}

void dbUsual(const char *msg) {
    fprintf(stderr,"%s: --->\n", msg);
    objDump("current_object", current_object);
    objDump("usual_parent", regs.r_usual_parent);
    // objDump("logo_object", logo_object);
    fprintf(stderr,"<--- \n");
}

#endif
