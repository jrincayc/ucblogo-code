/*
 *      logodata.c      logo data management module             dvb
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
#include <stdarg.h>
#ifdef ibm
#if !defined(__RZTC__) && !defined(_MSC_VER)
#include <alloc.h>
#endif
#endif

char special_chars[] = " \t\n(?????+++~)[]-*/=<>\"\\:;|{}";

#ifdef ecma

#include <ctype.h>

#define upper_p(ch)	(isupper((ch) & 0377))
#define lower_p(ch)	(islower((ch) & 0377))

char ecma_array[128];

int ecma_size = sizeof(special_chars)-1;

char ecma_set(int ch) {
    ch &= 0377;
    if (ch >= 128) return(ch);
    return(ecma_array[ch]);
}

char ecma_clear(int ch) {
    ch &= 0377;
    if (ch < ecma_begin || ch >= ecma_begin+sizeof(special_chars)-1)
	return(ch);
    if (ch >= 007 && ch <= 015 && ch != 013) return(ch);
    return(special_chars[ch - ecma_begin]);
}

int ecma_get(int ch) {
    ch &= 0377;
    return ((ch >= ecma_begin && ch < ecma_begin+sizeof(special_chars)-1)
	    && (ch < 007 || ch > 015 || ch == 013));
}

#else

#define upper_p(c)     (c >= 'A' && c <= 'Z')
#define lower_p(c)     (c >= 'a' && c <= 'z')

#endif

char *strnzcpy(char *s1, char *s2, int n) {
    strncpy(s1, s2, n);
    s1[n] = '\0';
    return(s1);
}

char *word_strnzcpy(char *s1, NODE *kludge, int n) {  /* KLUDGE! */
    char *temp = s1;

    while (kludge != NIL) {
	strncpy(s1, getstrptr(car(kludge)), getstrlen(car(kludge)));
	s1 += getstrlen(car(kludge));
	kludge = cdr(kludge);
    }
    temp[n] = '\0';
    return(temp);
}

char *noparity_strnzcpy(char *s1, char *s2, int n) {
    int i;

    for (i = 0; i < n; i++)
	s1[i] = clearparity(s2[i]);
    s1[n] = '\0';
    return(s1);
}

char *backslashed_strnzcpy(char *s1, char *s2, int n) {
    int i,j;

    for (i = 0, j = 0; i < n; i++) {
	if (getparity(s2[i]))
	    s1[j++] = '\\';
	s1[j++] = clearparity(s2[i]);
    }
    s1[j] = '\0';
    return(s1);
}

char *mend_strnzcpy(char *s1, char *s2, int n) {
    int i, vbar = 0;

    for (i = 0; i < n; ) {
	while (*s2 == '|') {
	    vbar = !vbar;
	    s2++;
	}
	if (vbar) {
	    if (strchr(special_chars,(int)*s2))
		s1[i++] = setparity(*s2++);
	    else
		s1[i++] = *s2++;
	} else {
	    while (*s2 == ';' || (*s2 == '~' && *(s2 + 1) == '\n')) {
		while (*s2 == '~' && *(s2 + 1) == '\n') s2 += 2;
		if (*s2 == ';')
		    do {
			s2++;
		    } while (*s2 != '\0' && *s2 != '~' && *(s2 + 1) != '\n');
	    }
	    if (*s2 != '|') s1[i++] = *s2++;
	}
    }
    s1[n] = '\0';
    return(s1);
}

char *mend_nosemi(char *s1, char *s2, int n) {
    int i, vbar = 0;

    for (i = 0; i < n; ) {
	while (*s2 == '|') {
	    vbar = !vbar;
	    s2++;
	}
	if (vbar) {
	    if (strchr(special_chars,(int)*s2))
		s1[i++] = setparity(*s2++);
	    else
		s1[i++] = *s2++;
	} else {
	    while ((*s2 == '~' && *(s2 + 1) == '\n')) {
		while (*s2 == '~' && *(s2 + 1) == '\n') s2 += 2;
	    }
	    if (*s2 != '|') s1[i++] = *s2++;
	}
    }
    s1[n] = '\0';
    return(s1);
}

char *quote_strnzcpy(char *s1, char *s2, int n) {
    s1[0] = '"';
    strncpy(s1 + 1, s2, n - 1);
    s1[n] = '\0';
    return(s1);
}

char *colon_strnzcpy(char *s1, char *s2, int n) {
    s1[0] = ':';
    strncpy(s1 + 1, s2, n - 1);
    s1[n] = '\0';
    return(s1);
}

#define uncapital(c)    ((c) - 'A' + 'a')

char *low_strnzcpy(char *s1, char *s2, int n) {
    char *temp = s1;
    int i;

    for (i = 0; i < n; i++) {
	if (upper_p(*s2))
	    *s1++ = uncapital(*s2++);
	else
	    *s1++ = *s2++;
    }
    *s1 = '\0';
    return(temp);
}

#define capital(c)    ((c) - 'a' + 'A')

char *cap_strnzcpy(char *s1, char *s2, int n) {
    char *temp = s1;
    int i;

    for (i = 0; i < n; i++) {
	if (lower_p(*s2))
	    *s1++ = capital(*s2++);
	else
	    *s1++ = *s2++;
    }
    *s1 = '\0';
    return(temp);
}

char *noparitylow_strnzcpy(char *s1, char *s2, int n) {
    int i;
    char c, *temp = s1;

    for (i = 0; i < n; i++) {
	c = clearparity(*s2++);
	if (upper_p(c))
	    *s1++ = uncapital(c);
	else
	    *s1++ = c;
    }
    *s1 = '\0';
    return(temp);
}

int low_strncmp(char *s1, char *s2, int n) {
    int i;

    for (i = 0; i < n; i++) {
	if (*s1 != *s2) {
	    if (upper_p(*s2)) {
		if (upper_p(*s1)) {
		    if (uncapital(*s1) != uncapital(*s2))
			return(uncapital(*s1) - uncapital(*s2));
		} else {
		    if (*s1 != uncapital(*s2))
			return(*s1 - uncapital(*s2));
		}
	    } else if (upper_p(*s1)) {
		if (uncapital(*s1) != *s2)
		    return(uncapital(*s1) - *s2);
	    } else return(*s1 - *s2);
	}
	s1++, s2++;
    }
    return(0);
}

int noparity_strncmp(char *s1, char *s2, int n) {
    int i;

    for (i = 0; i < n; i++) {
	if (clearparity(*s1) != clearparity(*s2))
	    return(clearparity(*s1) - clearparity(*s2));
	s1++, s2++;
    }
    return(0);
}

int noparitylow_strncmp(char *s1, char *s2, int n) {
    int i;
    char c1, c2;

    for (i = 0; i < n; i++) {
	c1 = clearparity(*s1);
	c2 = clearparity(*s2);
	if (c1 != c2) {
	    if (upper_p(c2)) {
		if (upper_p(c1)) {
		    if (uncapital(c1) != uncapital(c2))
			return(uncapital(c1) - uncapital(c2));
		} else {
		    if (c1 != uncapital(c2))
			return(c1 - uncapital(c2));
		}
	    } else if (upper_p(c1)) {
		if (uncapital(c1) != c2)
		    return(uncapital(c1) - c2);
	    } else return(c1 - c2);
	}
	s1++, s2++;
    }
    return(0);
}

NODE *make_strnode(char *strptr, struct string_block *strhead, int len,
		   NODETYPES typ, char *(*copy_routine)())
{
    NODE *strnode;

    if (len == 0 && Null_Word != NIL) return(Null_Word);
    strnode = newnode(typ);
    if (strhead == NULL) {
	strhead = (struct string_block *) malloc((size_t)len
						 + sizeof(FIXNUM) + 1);
	if (strhead == NULL) {
	    err_logo(OUT_OF_MEM, NIL);
	    return UNBOUND;
	}
	(*copy_routine) (strhead->str_str, strptr, len);
	strptr = strhead->str_str;
	setstrrefcnt(strhead, 0);
    }
    setstrlen(strnode, len);
    setstrptr(strnode, strptr);
    setstrhead(strnode, strhead);
    incstrrefcnt(strhead);
    return(strnode);
}

void make_runparse(NODE *ndi) {
    NODE *rp_list;

    rp_list = runparse(ndi);
    setobject(ndi, rp_list);
    settype(ndi, RUN_PARSE);
}

NODE *make_quote(NODE *qnd) {
    NODE *nd;

    nd = cons(qnd, NIL);
    settype(nd, QUOTE);
    return(nd);
}

NODE *maybe_quote(NODE *nd) {
    if (nodetype(nd) == CONT) nd = cdr(nd);
    if (nd == UNBOUND || aggregate(nd) || numberp(nd)) return(nd);
    return(make_quote(nd));
}

NODE *make_caseobj(NODE *cstrnd, NODE *obj) {
    NODE *nd;

    nd = cons(cstrnd, obj);
    settype(nd, CASEOBJ);
    return(nd);
}

NODE *make_colon(NODE *cnd) {
    NODE *nd;

    nd = cons(cnd, NIL);
    settype(nd, COLON);
    return(nd);
}

NODE *make_intnode(FIXNUM i) {
    NODE *nd = newnode(INT);

    setint(nd, i);
    return(nd);
}

NODE *make_floatnode(FLONUM f) {
    NODE *nd = newnode(FLOATT);

    setfloat(nd, f);
    return(nd);
}

NODE *cnv_node_to_numnode(NODE *ndi) {
    NODE *val;
    int dr;
    char s2[MAX_NUMBER], *s = s2;

    if (is_number(ndi))
	return(ndi);
    ndi = cnv_node_to_strnode(ndi);
    if (ndi == UNBOUND) return(UNBOUND);
    if (((getstrlen(ndi)) < MAX_NUMBER) && (dr = numberp(ndi))) {
	if (backslashed(ndi))
	    noparity_strnzcpy(s, getstrptr(ndi), getstrlen(ndi));
	else
	    strnzcpy(s, getstrptr(ndi), getstrlen(ndi));
	if (*s == '+') ++s;
	if (s2[getstrlen(ndi)-1] == '.') s2[getstrlen(ndi)-1] = 0;
	if (dr - 1 || getstrlen(ndi) > 9) {
	    val = newnode(FLOATT);
	    setfloat(val, atof(s));
	} else {
	    val = newnode(INT);
	    setint(val, atol(s));
	}
	return(val);
    } else {
	return(UNBOUND);
    }
}

NODE *cnv_node_to_strnode(NODE *nd) {
    char s[MAX_NUMBER];

    if (nd == UNBOUND || aggregate(nd)) {
	return(UNBOUND);
    }
    switch(nodetype(nd)) {
	case STRING:
	case BACKSLASH_STRING:
	case VBAR_STRING:
	    return(nd);
	case CASEOBJ:
	    return strnode__caseobj(nd);
	case QUOTE:
	    nd = cnv_node_to_strnode(node__quote(nd));
	    nd = make_strnode(getstrptr(nd), (struct string_block *)NULL,
			      getstrlen(nd) + 1, nodetype(nd), quote_strnzcpy);
	    return(nd);
	case COLON:
	    nd = cnv_node_to_strnode(node__colon(nd));
	    nd = make_strnode(getstrptr(nd), (struct string_block *)NULL,
			      getstrlen(nd) + 1, nodetype(nd), colon_strnzcpy);
	    return(nd);
	case INT:
	    sprintf(s, "%ld", getint(nd));
	    return(make_strnode(s, (struct string_block *)NULL,
				(int)strlen(s), STRING, strnzcpy));
	case FLOATT:
	    sprintf(s, "%0.15g", getfloat(nd));
	    return(make_strnode(s, (struct string_block *)NULL,
				(int)strlen(s), STRING, strnzcpy));
    }
    return(UNBOUND);	/* Can't get here, but makes compiler happy */
}

NODE *make_static_strnode(char *strptr) {
    NODE *strnode = newnode(STRING);

    setstrptr(strnode, strptr);
    setstrhead(strnode, NULL);
    setstrlen(strnode, (int)strlen(strptr));
    return(strnode);
}

NODE *cons_list(int dummy, ...) {
    va_list ap;
    NODE *nptr, *outline = NIL, *lastnode = NIL, *val;

     va_start(ap, dummy);
     while ( (nptr = va_arg(ap, NODE *)) != END_OF_LIST) {
	val = cons(nptr, NIL);
	if (outline == NIL) {
	    outline = val;
	    lastnode = outline;
	} else {
	    setcdr(lastnode, val);
	    lastnode = val;
	}
     }
    va_end(ap);
    return(outline);
}

NODE *make_array(FIXNUM len) {
    NODE *node;
    NODE **data;

    node = newnode(ARRAY);
    setarrorg(node,1);
    setarrdim(node,len);
    data = (NODE **)malloc((size_t)len * sizeof(NODE *));
    if (data == NULL) {
        err_logo(OUT_OF_MEM, NIL);
	return UNBOUND;
    }
    setarrptr(node,data);
    while (--len >= 0) *data++ = NIL;
    return(node);
}

NODE *llowercase(NODE *args) {
    NODE *arg;

    arg = string_arg(args);
    if (NOT_THROWING) {
	return make_strnode(getstrptr(arg), (struct string_block *)NULL,
			    getstrlen(arg), nodetype(arg), low_strnzcpy);
    }
    return UNBOUND;
}

NODE *luppercase(NODE *args) {
    NODE *arg;

    arg = string_arg(args);
    if (NOT_THROWING) {
	return make_strnode(getstrptr(arg), (struct string_block *)NULL,
			    getstrlen(arg), nodetype(arg), cap_strnzcpy);
    }
    return UNBOUND;
}

/* property list stuff */

NODE *getprop(NODE *plist, NODE *name, BOOLEAN before)
{
    NODE *prev = NIL;
    BOOLEAN caseig = FALSE;

    if (varTrue(Caseignoredp))
	caseig = TRUE;
    while (plist != NIL) {
	if (compare_node(name,car(plist),caseig) == 0) {
	    return(before ? prev : plist);
	}
	prev = plist;
	plist = cddr(plist);
    }
    return(NIL);
}

NODE *lgprop(NODE *args) {
    NODE *plname, *pname, *plist, *val = NIL;

    plname = string_arg(args);
    pname = string_arg(cdr(args));
    if (NOT_THROWING) {
	plname = intern(plname);
	plist = plist__caseobj(plname);
	if (plist != NIL)
	    val = getprop(plist, pname, FALSE);
	if (val != NIL)
	    return cadr(val);
    }
    return NIL;
}

NODE *lpprop(NODE *args) {
    NODE *plname, *pname, *newval, *plist, *val = NIL;

    plname = string_arg(args);
    pname = string_arg(cdr(args));
    newval = car(cddr(args));
    if (NOT_THROWING) {
	plname = intern(plname);
	if (flag__caseobj(plname, PLIST_TRACED)) {
	    ndprintf(writestream, "%t %s %s %s",
		     message_texts[TRACE_PPROP], maybe_quote(plname),
		     maybe_quote(pname), maybe_quote(newval));
	    if (ufun != NIL)
		ndprintf(writestream,message_texts[ERROR_IN],ufun,this_line);
	    new_line(writestream);
	}
	plist = plist__caseobj(plname);
	if (plist != NIL)
	    val = getprop(plist, pname, FALSE);
	if (val != NIL)
	    setcar(cdr(val), newval);
	else
	    setplist__caseobj(plname, cons(pname, cons(newval, plist)));
	need_save = 1;
    }
    return(UNBOUND);
}

NODE *lremprop(NODE *args) {
    NODE *plname, *pname, *plist, *val = NIL;
    BOOLEAN caseig = FALSE;

    if (varTrue(Caseignoredp))
	caseig = TRUE;
    plname = string_arg(args);
    pname = string_arg(cdr(args));
    if (NOT_THROWING) {
	plname = intern(plname);
	plist = plist__caseobj(plname);
	if (plist != NIL) {
	    if (compare_node(car(plist), pname, caseig) == 0)
		setplist__caseobj(plname, cddr(plist));
	    else {
		val = getprop(plist, pname, TRUE);
		if (val != NIL)
		    setcdr(cdr(val), cddr(cddr(val)));
	    }
	}
    }
    return(UNBOUND);
}

NODE *copy_list(NODE *arg) {
    NODE *tnode, *lastnode = NIL, *val = NIL;

    while (arg != NIL) {
	tnode = cons(car(arg), NIL);
	arg = cdr(arg);
	if (val == NIL) {
	    lastnode = val = tnode;
	} else {
	    setcdr(lastnode, tnode);
	    lastnode = tnode;
	}
    }
    return(val);
}

NODE *lplist(NODE *args) {
    NODE *plname, *plist, *val = NIL;

    plname = string_arg(args);
    if (NOT_THROWING) {
	plname = intern(plname);
	plist = plist__caseobj(plname);
	if (plist != NIL)
	    val = copy_list(plist);
    }
    return(val);
}

/* Boolean foreign language stuff */

int isName(NODE *nod, enum words wd) {
    return ((compare_node(nod, translations[wd].English, TRUE) == 0) ||
	    (compare_node(nod, translations[wd].Alt, TRUE) == 0));
}

int varTrue(NODE *varb) {
    return isName(valnode__caseobj(varb), Name_true);
}

NODE *theName(enum words wd) {
    return(varTrue(UseAlternateNames) ? translations[wd].Alt :
					translations[wd].English);
}

NODE *TrueName() {
    return theName(Name_true);
}

NODE *FalseName() {
    return theName(Name_false);
}
