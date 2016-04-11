/*
 *      parse.c         logo parser module              dvb
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

#ifdef HAVE_TERMIO_H
#include <termio.h>
#else
#ifdef HAVE_SGTTY_H
#include <sgtty.h>
#endif
#endif

#include <ctype.h>

#ifdef ibm
#include <bios.h>
extern int getch(void);
#endif
#ifdef __ZTC__
#include <disp.h>
#endif

#ifndef TIOCSTI
#include <setjmp.h>
extern jmp_buf iblk_buf;
#endif

FILE *readstream = stdin;
FILE *writestream = stdout;
FILE *loadstream = stdin;
FILE *dribblestream = NULL;
int input_blocking = 0;

int rd_getc(FILE *strm)
{
    int c;

#ifdef __ZTC__
    if (strm == stdin) zflush();
    c = ztc_getc(strm);
#else
    c = getc(strm);
#endif
    if (strm == stdin && c != EOF) update_coords(c);
#ifdef ibm
    if (c == 17 && interactive && strm==stdin) { /* control-q */
	to_pending = 0;
	err_logo(STOP_ERROR,NIL);
    }
    if (c == 23 && interactive && strm==stdin) { /* control-w */
#ifndef __ZTC__
	getc(strm); /* eat up the return */
#endif
	logo_pause();
	return(rd_getc(strm));
    }
#endif
#ifdef ecma
    return(ecma_clear(c));
#else
    return(c);
#endif
}

void rd_print_prompt(char *str)
{
    int ch;

#ifdef ibm
#ifdef __ZTC__
    if (in_graphics_mode && !in_splitscreen)
#else
    if (in_graphics_mode && ibm_screen_top == 0)
#endif
	lsplitscreen();
#endif
    ndprintf(stdout,"%t",str);
#ifdef __ZTC__
    zflush();
#endif
}

#ifdef __ZTC__
void zrd_print_prompt(char *str) {
    newline_bugfix();
    rd_print_prompt(str);
}
#else
#define zrd_print_prompt rd_print_prompt
#endif

#define into_line(chr) {if (phys_line >= p_end) { \
							p_len += MAX_PHYS_LINE; \
							p_pos = phys_line - p_line; \
							p_line = realloc(p_line, p_len); \
							p_end = &p_line[p_len-1]; \
							phys_line = &p_line[p_pos]; \
						} \
						*phys_line++ = (chr);}

char *p_line = 0, *p_end;
int p_len = MAX_PHYS_LINE;

NODE *reader(FILE *strm, char *prompt)
{
    int c, dribbling, vbar = 0, paren = 0, bracket = 0, brace = 0, p_pos, contin=1;
	static char ender[] = "\nEND\n";
    char *phys_line, *lookfor = ender;
    NODETYPES this_type = STRING;
    NODE *ret;

	if (!strcmp(prompt, "RW")) {	/* called by readword */
		prompt = "";
		contin = 0;
	}
    charmode_off();
    dribbling = (dribblestream != NULL && strm == stdin);
    if (p_line == 0) {
    	p_line = malloc(MAX_PHYS_LINE);
    	p_end = &p_line[MAX_PHYS_LINE-1];
    }
    phys_line = p_line;
    if (strm == stdin && *prompt) {
	if (interactive) rd_print_prompt(prompt);
	if (dribblestream != NULL)
	    fprintf(dribblestream, "%s", prompt);
    }
    if (strm == stdin) {
	input_blocking++;
	erract_errtype = FATAL;
    }

#ifndef TIOCSTI
    if (!setjmp(iblk_buf)) {
#endif
    c = rd_getc(strm);
    while (c != EOF && (vbar || paren || bracket || brace || c != '\n')) {
	if (dribbling) putc(c, dribblestream);
	if (c == '\\' && (c = rd_getc(strm)) != EOF) {
	    if (dribbling) putc(c, dribblestream);
	    c = setparity(c);
	    this_type = BACKSLASH_STRING;
	    if (c == setparity('\n') && strm == stdin) {
		if (interactive) zrd_print_prompt("\\ ");
		if (dribbling)
		    fprintf(dribblestream, "\\ ");
	    }
	}
	if (c != EOF) into_line(c);
		if (*prompt && (c&0137) == *lookfor) {
			lookfor++;
			if (*lookfor == 0) {
				err_logo(DEEPEND, NIL);
				break;
			}
		} else lookfor = ender;
	if (c == '|') vbar = !vbar;
	else if (contin && !vbar) {
		if (c == '(') paren++;
		else if (paren && c == ')') paren--;
		else if (c == '[') bracket++;
		else if (bracket && c == ']') bracket--;
		else if (c == '{') brace++;
		else if (brace && c == '}') brace--;
	}
	if (/* (vbar || paren ...) && */ c == '\n') {
	    if (strm == stdin) {
		if (interactive) zrd_print_prompt(vbar ? "| " : "~ ");
		if (dribbling)
		    fprintf(dribblestream, vbar ? "| " : "~ ");
	    }
	}
	while (!vbar && c == '~' && (c = rd_getc(strm)) != EOF) {
	    while (c == ' ' || c == '\t')
		c = rd_getc(strm);
	    if (dribbling) putc(c, dribblestream);
	    into_line(c);
	    if (c == '\n' && strm == stdin) {
		if (interactive) zrd_print_prompt("~ ");
		if (dribbling)
		    fprintf(dribblestream, "~ ");
	    }
	}
	if (c != EOF) c = rd_getc(strm);
    }
#ifndef TIOCSTI
    }
#endif
    *phys_line = '\0';
    input_blocking = 0;
#ifdef __ZTC__
    fix_cursor();
    if (interactive && strm == stdin) newline_bugfix();
#endif
    if (dribbling)
	putc('\n', dribblestream);
    if (c == EOF && strm == stdin) {
	if (interactive) clearerr(stdin);
	rd_print_prompt("\n");
    }
    if (phys_line == p_line) return(Null_Word); /* so emptyp works */
    ret = make_strnode(p_line, (char *)NULL, (int)strlen(p_line),
		       this_type, strnzcpy);
    return(ret);
}

NODE *list_to_array(NODE *list)
{
    NODE *np = list, *result;
    int len = 0, i;

    for (; np; np = cdr(np)) len++;

    result = make_array(len);
    setarrorg(result,1);

    for (i = 0, np = list; np; np = cdr(np))
	(getarrptr(result))[i++] = vref(car(np));

    return(result);
}

#define parens(ch)      (ch == '(' || ch == ')' || ch == ';')
#define infixs(ch)      (ch == '*' || ch == '/' || ch == '+' || ch == '-' || ch == '=' || ch == '<' || ch == '>')
#define white_space(ch) (ch == ' ' || ch == '\t' || ch == '\n')

NODE *parser_iterate(char **inln, char *inlimit, char *inhead,
		     BOOLEAN semi, int endchar)
{
    char ch, *wptr = NULL;
    static char terminate = '\0';   /* KLUDGE */
    NODE *outline = NIL, *lastnode = NIL, *tnode = NIL;
    int windex = 0, vbar = 0;
    NODETYPES this_type = STRING;
    BOOLEAN broken = FALSE;

    do {
	/* get the current character and increase pointer */
	ch = **inln;
	if (!vbar && windex == 0) wptr = *inln;
	if (++(*inln) >= inlimit) *inln = &terminate;

	/* skip through comments and line continuations */
	while (!vbar && ((semi && ch == ';') ||
		(ch == '~' && **inln == '\n'))) {
	    while (ch == '~' && **inln == '\n') {
		if (++(*inln) >= inlimit) *inln = &terminate;
		ch = **inln;
		if (windex == 0) wptr = *inln;
		else {
		    if (**inln == ']' || **inln == '[' ||
		    			 **inln == '{' || **inln == '}') {
			ch = ' ';
			break;
		    } else {
			broken = TRUE;
		    }
		}
		if (++(*inln) >= inlimit) *inln = &terminate;
	    }

	    if (semi && ch == ';') {
		do {
		    ch = **inln;
		    if (windex == 0) wptr = *inln;
		    else broken = TRUE;
		    if (++(*inln) >= inlimit) *inln = &terminate;
		} while (ch != '\0' && ch != '~' && **inln != '\n');
		if (ch != '\0' && ch != '~') ch = '\n';
	    }
	}

	/* flag that this word will be of BACKSLASH_STRING type */
	if (getparity(ch)) this_type = BACKSLASH_STRING;

	if (ch == '|') {
	    vbar = !vbar;
	    this_type = VBAR_STRING;
	    broken = TRUE; /* so we'll copy the chars */
	}

	else if (vbar || (!white_space(ch) && ch != ']' &&
		    ch != '{' && ch != '}' &&
		    ch != '[' && ch != '\0'))
	    windex++;

	if (vbar) continue;

	else if (ch == endchar) break;

	else if (ch == ']') err_logo(UNEXPECTED_BRACKET, NIL);
	else if (ch == '}') err_logo(UNEXPECTED_BRACE, NIL);

	/* if this is a '[', parse a new list */
	else if (ch == '[') {
	    tnode = cons(parser_iterate(inln,inlimit,inhead,semi,']'), NIL);
	    if (**inln == '\0') ch = '\0';
	}

	else if (ch == '{') {
	    tnode = cons(list_to_array
			 (parser_iterate(inln,inlimit,inhead,semi,'}')), NIL);
	    if (**inln == '@') {
		int i = 0, sign = 1;

		(*inln)++;
		if (**inln == '-') {
		    sign = -1;
		    (*inln)++;
		}
		while ((ch = **inln) >= '0' && ch <= '9') {
		    i = (i*10) + ch - '0';
		    (*inln)++;
		}
		setarrorg(car(tnode),sign*i);
	    }
	    if (**inln == '\0') ch = '\0';
	}

/* if this character or the next one will terminate string, make the word */
	else if (white_space(ch) || ch == '\0' ||
	     **inln == ']' || **inln == '[' ||
	     **inln == '{' || **inln == '}') {
		if (windex > 0) {
		    if (broken == FALSE)
			 tnode = cons(make_strnode(wptr, inhead, windex,
						   this_type, strnzcpy),
				      NIL);
		    else {
			 tnode = cons(make_strnode(wptr, (char *)NULL, windex,
				 this_type, (semi ? mend_strnzcpy : mend_nosemi)),
				 NIL);
			 broken = FALSE;
		    }
		    this_type = STRING;
		    windex = 0;
		}
	}

	/* put the word onto the end of the return list */
	if (tnode != NIL) {
	    if (outline == NIL) outline = vref(tnode);
	    else setcdr(lastnode, tnode);
	    lastnode = tnode;
	    tnode = NIL;
	}
    } while (ch);
    return(unref(outline));
}

NODE *parser(NODE *nd, BOOLEAN semi)
{
    NODE *rtn;
    int slen;
    char *lnsav;

    rtn = cnv_node_to_strnode(nd);
    ref(rtn);
    gcref(nd);
    slen = getstrlen(rtn);
    lnsav = getstrptr(rtn);
    rtn = reref(rtn,
		parser_iterate(&lnsav,lnsav + slen,getstrhead(rtn),semi,-1));
    return(unref(rtn));
}

NODE *lparse(NODE *args)
{
    NODE *arg, *val = UNBOUND;

    arg = string_arg(args);
    if (NOT_THROWING) {
	val = parser(arg, FALSE);
    }
    return(val);
}

NODE *runparse_node(NODE *nd, NODE **ndsptr)
{
    NODE *outline = NIL, *tnode = NIL, *lastnode = NIL, *snd;
    char *wptr, *tptr, *whead;
    int wlen, wcnt, tcnt, isnumb;
    NODETYPES wtyp;
    BOOLEAN monadic_minus = FALSE;

    if (nd == Minus_Tight) return cons(nd, NIL);
    snd = cnv_node_to_strnode(nd);
    ref(snd);
    wptr = getstrptr(snd);
    wlen = getstrlen(snd);
    wtyp = nodetype(snd);
    wcnt = 0;
    whead = getstrhead(snd);

    while (wcnt < wlen) {
	if (*wptr == ';') {
	    *ndsptr = NIL;
	    break;
	}
	if (*wptr == '"') {
	    tcnt = 0;
	    tptr = ++wptr;
	    wcnt++;
	    while (wcnt < wlen && !parens(*wptr)) {
		if (wtyp == BACKSLASH_STRING && getparity(*wptr))
		    wtyp = PUNBOUND;    /* flag for "\( case */
		wptr++, wcnt++, tcnt++;
	    }
	    if (wtyp == PUNBOUND) {
		wtyp = BACKSLASH_STRING;
		tnode = cons(make_quote(intern(make_strnode(tptr, NULL,
					tcnt, wtyp, noparity_strnzcpy))), NIL);
	    } else
		tnode = cons(make_quote(intern(make_strnode(tptr, whead, tcnt,
				        wtyp, strnzcpy))), NIL);
	} else if (*wptr == ':') {
	    tcnt = 0;
	    tptr = ++wptr;
	    wcnt++;
	    while (wcnt < wlen && !parens(*wptr) && !infixs(*wptr))
		wptr++, wcnt++, tcnt++;
	    tnode = cons(make_colon(intern(make_strnode(tptr, whead, tcnt,
				    wtyp, strnzcpy))), NIL);
	} else if (wcnt == 0 && *wptr == '-' && monadic_minus == FALSE &&
		   !white_space(*(wptr+1))) {
	/* minus sign with space before and no space after is unary */
	    tnode = cons(make_intnode((FIXNUM)0), NIL);
	    monadic_minus = TRUE;
	} else if (parens(*wptr) || infixs(*wptr)) {
	    if (monadic_minus)
		tnode = cons(Minus_Tight, NIL);
	    else
		tnode = cons(intern(make_strnode(wptr, whead, 1,
						 STRING, strnzcpy)), NIL);
	    monadic_minus = FALSE;
	    wptr++, wcnt++;
	} else {
	    tcnt = 0;
	    tptr = wptr;
	    /* isnumb 4 means nothing yet;
		 * 0 means digits so far, 1 means just saw
	     * 'e' so minus can be next, 2 means no longer
	     * eligible even if an 'e' comes along */
	    isnumb = 4;
	    if (*wptr == '?') {
		isnumb = 3; /* turn ?5 to (? 5) */
		wptr++, wcnt++, tcnt++;
	    }
	    while (wcnt < wlen && !parens(*wptr) &&
		   (!infixs(*wptr) || (isnumb == 1 && (*wptr == '-' || *wptr == '+')))) {
		if (isnumb == 4 && isdigit(*wptr)) isnumb = 0;
		if (isnumb == 0 && tcnt > 0 && (*wptr == 'e' || *wptr == 'E'))
		    isnumb = 1;
		else if (!(isdigit(*wptr) || *wptr == '.') || isnumb == 1)
		    isnumb = 2;
		wptr++, wcnt++, tcnt++;
	    }
	    if (isnumb == 3 && tcnt > 1) {    /* ?5 syntax */
		NODE *qmtnode;

		qmtnode = cons_list(0, Left_Paren, Query,
				    cnv_node_to_numnode
					(make_strnode(tptr+1, whead,
						      tcnt-1, wtyp, strnzcpy)),
				    END_OF_LIST);
		if (outline == NIL) {
		    outline = vref(qmtnode);
		} else {
		    setcdr(lastnode, qmtnode);
		}
		lastnode = cddr(qmtnode);
		tnode = cons(Right_Paren, NIL);
	    } else if (isnumb < 2 && tcnt > 0) {
		tnode = cons(cnv_node_to_numnode(make_strnode(tptr, whead, tcnt,
							      wtyp, strnzcpy)),
			     NIL);
	    } else
		tnode = cons(intern(make_strnode(tptr, whead, tcnt,
						 wtyp, strnzcpy)),
			     NIL);
	}

	if (outline == NIL) outline = vref(tnode);
	else setcdr(lastnode, tnode);
	lastnode = tnode;
    }
    deref(snd);
    return(unref(outline));
}

NODE *runparse(NODE *ndlist)
{
    NODE *curnd = NIL, *outline = NIL, *tnode = NIL, *lastnode = NIL;

    if (nodetype(ndlist) == RUN_PARSE)
		return parsed__runparse(ndlist);
	if (!is_list(ndlist)) {
		err_logo(BAD_DATA_UNREC, ndlist);
		return(NIL);
	}
    while (ndlist != NIL) {
	curnd = car(ndlist);
	ndlist = cdr(ndlist);
	if (!is_word(curnd))
	    tnode = cons(curnd, NIL);
	else {
	    if (!numberp(curnd))
		tnode = runparse_node(curnd, &ndlist);
	    else
		tnode = cons(cnv_node_to_numnode(curnd), NIL);
	}
	if (tnode != NIL) {
	    if (outline == NIL) outline = vref(tnode);
	    else setcdr(lastnode, tnode);
	    lastnode = tnode;
	    while (cdr(lastnode) != NIL) {
		lastnode = cdr(lastnode);
		if (check_throwing) break;
	    }
	}
	if (check_throwing) break;
    }
    return(unref(outline));
}

NODE *lrunparse(NODE *args)
{
    NODE *arg;

    arg = car(args);
    while (nodetype(arg) == ARRAY && NOT_THROWING) {
	setcar(args, err_logo(BAD_DATA, arg));
	arg = car(args);
    }
    if (NOT_THROWING && !aggregate(arg))
	arg = parser(arg, TRUE);
    if (NOT_THROWING)
	return runparse(arg);
    return UNBOUND;
}
