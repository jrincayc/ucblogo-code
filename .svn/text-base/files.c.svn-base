/*
 *      files.c         logo file management module             dvb
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

#ifdef WIN32
#include <windows.h>
#endif /* WIN32 */

#include "logo.h"
#include "globals.h"

#ifdef HAVE_WX
#define getc getFromWX_2
#define ungetc wxUnget_c
int reading_char_now = 0;
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef mac
#include <console.h>
#endif
#ifdef ibm
#ifndef _MSC_VER
#include <bios.h>
#ifndef __RZTC__
#include <alloc.h>
#endif /* ZTC */
#endif /* MSC_VER */
extern int getch(), kbhit();
#ifdef __RZTC__
#include <conio.h>
#endif
#endif

NODE *file_list = NULL;
NODE *reader_name = NIL, *writer_name = NIL, *file_prefix = NIL;
int need_save = 0;  /* nonzero if workspace changed since save */
NODE *save_name = NIL;	/* filename of last save or load command */

char *asciiz(NODE *arg) {
    char *out = (char *)malloc(getstrlen(arg)+1);
    return noparity_strnzcpy(out, getstrptr(arg), getstrlen(arg));
}

NODE *lseteditor(NODE *args) {
    NODE *arg;

    arg = cnv_node_to_strnode(car(args));
    if (arg == UNBOUND) err_logo(BAD_DATA_UNREC, arg);
    else {
	editor = asciiz(arg);
    editorname = strrchr(editor, (int)'/');
    if (editorname == NULL) editorname = strrchr(editor, (int)'\\');
    if (editorname == NULL) editorname = strrchr(editor, (int)':');
    editorname = (editorname ? editorname+1 : editor);
    }
    return UNBOUND;
}

NODE *lsetlibloc(NODE *args) {
    NODE *arg;

    arg = cnv_node_to_strnode(car(args));
    if (arg == UNBOUND) err_logo(BAD_DATA_UNREC, arg);
    else logolib = asciiz(arg);
    return UNBOUND;
}

NODE *lsethelploc(NODE *args) {
    NODE *arg;

    arg = cnv_node_to_strnode(car(args));
    if (arg == UNBOUND) err_logo(BAD_DATA_UNREC, arg);
    else helpfiles = asciiz(arg);
    return UNBOUND;
}

NODE *lsetcslsloc(NODE *args) {
    NODE *arg;

    arg = cnv_node_to_strnode(car(args));
    if (arg == UNBOUND) err_logo(BAD_DATA_UNREC, arg);
    else csls = asciiz(arg);
    return UNBOUND;
}

NODE *lsettemploc(NODE *args) {
    NODE *arg;

    arg = cnv_node_to_strnode(car(args));
    if (arg == UNBOUND) err_logo(BAD_DATA_UNREC, arg);
    else tempdir = asciiz(arg);
    return UNBOUND;
}

NODE *lsetprefix(NODE *args) {
    NODE *arg;

    if (car(args) == NIL) file_prefix = NIL;
    else {
	arg = cnv_node_to_strnode(car(args));
	if (arg == UNBOUND) err_logo(BAD_DATA_UNREC, arg);
	else file_prefix = arg;
    }
    return UNBOUND;
}

NODE *lprefix(NODE *args) {
    return file_prefix;
}

FILE *open_file(NODE *arg, char *access) {
    char *fnstr;
    FILE *tstrm;
    char *old_stringptr = print_stringptr;
    int old_stringlen = print_stringlen;

    if (is_list(arg)) { /* print to string */
	if (*access != 'w') {
	    err_logo(BAD_DATA_UNREC, arg);
	    return NULL;
	} else {
	    FIXNUM i = int_arg(cdr(arg));
	    if (NOT_THROWING && i > 0 && cddr(arg) == NIL) {
		char *tmp = (char *)malloc(i);
		*tmp = '\0';
		return (FILE *)tmp;
	    }
	    err_logo(BAD_DATA_UNREC, car(arg));
	    return NULL;
	}
    }

    arg = cnv_node_to_strnode(arg);
    if (arg == UNBOUND) return(NULL);
    if (file_prefix != NIL) {
	print_stringlen = getstrlen(file_prefix) +
			    getstrlen(arg) + 2;
	fnstr = (char *)malloc((size_t)print_stringlen + 1);
    } else
	fnstr = (char *) malloc((size_t)getstrlen(arg) + 1);
    if (fnstr == NULL) {
	err_logo(FILE_ERROR, make_static_strnode(message_texts[MEM_LOW]));
	print_stringptr = old_stringptr;
	print_stringlen = old_stringlen;
	return NULL;
    }
    if (file_prefix != NIL) {
	print_stringptr = fnstr;
	ndprintf((FILE *)NULL, "%p%t%p", file_prefix, separator, arg);
	*print_stringptr = '\0';
	print_stringptr = old_stringptr;
	print_stringlen = old_stringlen;
    } else
	noparity_strnzcpy(fnstr, getstrptr(arg), getstrlen(arg));
    tstrm = fopen(fnstr, access);
    free(fnstr);
    return(tstrm);
}

NODE *ldribble(NODE *arg) {
    if (dribblestream != NULL)
	err_logo(ALREADY_DRIBBLING, NIL);
    else {
	dribblestream = open_file(car(arg), "w+");
	if (dribblestream == NULL) err_logo(CANT_OPEN_ERROR, car(arg));
    }
    return(UNBOUND);
}

NODE *lnodribble(NODE *args) {
    if (dribblestream != NULL) {
	fclose(dribblestream);
	dribblestream = NULL;
    }
    return(UNBOUND);
}

FILE *find_file(NODE *arg, BOOLEAN remove) {
    NODE *t, *prev = NIL;
    FILE *fp = NULL;

    t = file_list;
    while (t != NIL) {
	if ((is_list(arg) && arg == car(t)) ||
	    (!is_list(arg) && (compare_node(arg, car(t), FALSE) == 0))) {
	    fp = (FILE *)t->n_obj;
	    if (remove) {
		t->n_obj = NIL;
		if (prev == NIL)
		    file_list = cdr(t);
		else
		    setcdr(prev, cdr(t));
	    }
	    break;
	}
	prev = t;
	t = cdr(t);
    }
    return fp;
}

NODE *lopen(NODE *arg, char *mode) {
    FILE *tmp;

    arg = car(arg);
    if (find_file(arg, FALSE) != NULL)
	err_logo(ALREADY_OPEN_ERROR, arg);
    else if ((tmp = open_file(arg, mode)) != NULL) {
	push(arg, file_list);
	file_list->n_obj = (NODE *)tmp;
    }
    else
	err_logo(CANT_OPEN_ERROR, arg);
    return(UNBOUND);
}

NODE *lopenread(NODE *arg) {
    return(lopen(arg,"r"));
}

NODE *lopenwrite(NODE *arg) {
    return(lopen(arg,"w"));
}

NODE *lopenappend(NODE *arg) {
    return(lopen(arg,"a"));
}

NODE *lopenupdate(NODE *arg) {
    return(lopen(arg,"a+"));
}

NODE *lallopen(NODE *args) {
    return(file_list);
}

NODE *lclose(NODE *arg) {
    FILE *tmp;
    NODE *margs;

    if ((tmp = find_file(car(arg), TRUE)) == NULL)
	err_logo(NOT_OPEN_ERROR, car(arg));
    else if (is_list (car(arg))) {
	margs = cons(caar(arg),
		     cons(make_strnode((char *)tmp, NULL, strlen((char *)tmp),
				       STRING, strnzcpy),
			  NIL));
	lmake(margs);
	free((char *)tmp);
    } else
	fclose(tmp);
    if ((is_list(car(arg)) && car(arg) == writer_name) ||
	(!is_list(car(arg)) &&
	 (compare_node(car(arg), writer_name, FALSE) == 0))) {
	    writer_name = NIL;
	    writestream = stdout;
    }
    if ((is_list(car(arg)) && car(arg) == reader_name) ||
	(!is_list(car(arg)) &&
	 (compare_node(car(arg), reader_name, FALSE) == 0))) {
	    reader_name = NIL;
	    readstream = stdin;
    }
    return(UNBOUND);
}

char *write_buf = 0;

NODE *lsetwrite(NODE *arg) {
    FILE *tmp;
    NODE *margs;

    if (writestream == NULL) {
	/* Any setwrite finishes earlier write to string */
	*print_stringptr = '\0';
	writestream = stdout;
	if (find_file(writer_name, FALSE) == NULL) {
	    /* pre-5.4 compatibility mode, implicitly close string */
	    margs = cons(car(writer_name),
			 cons(make_strnode(write_buf, NULL, strlen(write_buf),
					   STRING, strnzcpy),
			      NIL));
	    lmake(margs);
	    free(write_buf);
	}
	writer_name = NIL;
    }
    if (car(arg) == NIL) {
	writestream = stdout;
	writer_name = NIL;
    } else if (is_list(car(arg))) { /* print to string */
	FIXNUM i = int_arg(cdar(arg));
	if ((tmp = find_file(car(arg), FALSE)) != NULL) {
	    writestream = NULL;
	    writer_name = car(arg);
	    print_stringptr = (char *)tmp + strlen((char *)tmp);
	    print_stringlen = i - strlen((char *)tmp);
	} else if (NOT_THROWING && i > 0 && cddr(car(arg)) == NIL) {
	    writestream = NULL;
	    writer_name = copy_list(car(arg));
	    print_stringptr = write_buf = (char *)malloc(i);
	    print_stringlen = i;
	} else err_logo(BAD_DATA_UNREC, car(arg));
    } else if ((tmp = find_file(car(arg), FALSE)) != NULL) {
	writestream = tmp;
	writer_name = car(arg);
    } else
	err_logo(NOT_OPEN_ERROR, car(arg));
    return(UNBOUND);
}

NODE *lsetread(NODE *arg) {
    FILE *tmp;

    if (car(arg) == NIL) {
	readstream = stdin;
	reader_name = NIL;
    }
    else if ((tmp = find_file(car(arg), FALSE)) != NULL) {
	readstream = tmp;
	reader_name = car(arg);
    }
    else
	err_logo(NOT_OPEN_ERROR, car(arg));
    return(UNBOUND);
}

NODE *lreader(NODE *args) {
    return(reader_name);
}

NODE *lwriter(NODE *args) {
    return(writer_name);
}

NODE *lerasefile(NODE *arg) {
    char *fnstr;

    arg = cnv_node_to_strnode(car(arg));
    if (arg == UNBOUND) return(UNBOUND);
    fnstr = malloc((size_t)getstrlen(arg) + 1);
    if (fnstr == NULL) {
	err_logo(FILE_ERROR, make_static_strnode(message_texts[MEM_LOW]));
	return UNBOUND;
    }
    strnzcpy(fnstr, getstrptr(arg), getstrlen(arg));
    unlink(fnstr);
    free(fnstr);
    return(UNBOUND);
}

NODE *lsave(NODE *arg) {
    FILE *tmp;
    NODE *name;

    if (arg == NIL) name = save_name;
    else name = car(arg);
    tmp = writestream;
    if (name != NIL) {
	writestream = open_file(name, "w+");
	if (writestream != NULL) {
	    lpo(cons(lcontents(NIL), NIL));
	    fclose(writestream);
	    need_save = 0;
	    save_name = name;
	}
	else
	    err_logo(CANT_OPEN_ERROR, car(arg));
    } else
	err_logo(NOT_ENOUGH, NIL);
    writestream = tmp;
    return(UNBOUND);
}

void runstartup(NODE *oldst) {
    NODE *st;

    st = valnode__caseobj(Startup);
    if (st != oldst && st != NIL && is_list(st)) {
	eval_driver(st);
    }
}

void silent_load(NODE *arg, char *prefix) {
    FILE *tmp_stream;
    NODE *tmp_line, *exec_list;
    char load_path[200];
    NODE *st = valnode__caseobj(Startup);

    /* This procedure is called three ways:
     *    silent_load(NIL,*argv)	loads *argv
     *    silent_load(proc,logolib)     loads logolib/proc
     *    silent_load(proc,NULL)	loads proc.lg
     * The "/" or ".lg" is supplied by this procedure as needed.
     */

    /*     [This is no longer true!  But for Windows we change FOO? to FOOQ.]
     * In the case that this procedure is called to load a procedure from the
     * logo library, it must first truncate the name of the procedure to
     * eight characters, to find the filename (so as to be compatible with
     * MS-DOS)
     */

    if (prefix == NULL && arg == NIL) return;
    strcpy(load_path, (prefix == NULL ? "" :
    			  (arg == NIL ? prefix : addsep(prefix))));
    if (arg != NIL) {
	arg = cnv_node_to_strnode(arg);
	if (arg == UNBOUND) return;
	if (!strncmp(getstrptr(arg), ".", getstrlen(arg))) return;
	if (!strncmp(getstrptr(arg), "..", 2)) return;
	if (getstrlen(arg) > 150) return;
	noparitylow_strnzcpy(load_path + (int)strlen(load_path),
			     getstrptr(arg), getstrlen(arg));
	if (prefix == NULL)
	  strcat(load_path, ".lg");
/* #ifdef WIN32 */
	else if (arg != NIL) {
	    char *cp;
	    for (cp = load_path; *cp != '\0'; cp++)
		if (*cp == '?') *cp = 'Q';
	}
	/*  strcpy(load_path, eight_dot_three(load_path));  */
/* #endif */
    }
    tmp_stream = loadstream;
    tmp_line = current_line;
    loadstream = fopen(load_path, "r");
    if (loadstream != NULL) {
	while (!(feof(loadstream)) && NOT_THROWING) {
	    current_line = reader(loadstream, "");
	    exec_list =parser(current_line, TRUE);
	    if (exec_list != NIL) eval_driver(exec_list);
	}
	fclose(loadstream);
	runstartup(st);
    } else if (arg == NIL || prefix == csls)
	err_logo(CANT_OPEN_ERROR,
		 make_strnode(load_path, NULL, strlen(load_path), STRING,
			      strnzcpy));
    loadstream = tmp_stream;
    current_line = tmp_line;
}

NODE *lcslsload(NODE *arg) {
    NODE *save_prefix = file_prefix;
    file_prefix = NIL;
    silent_load(car(arg),csls);
    file_prefix = save_prefix;
    save_name = car(arg);
    return UNBOUND;
}

NODE *lload(NODE *arg) {
    FILE *tmp;
    NODE *tmp_line, *exec_list;
    NODE *st = valnode__caseobj(Startup);

    tmp = loadstream;
    tmp_line = current_line;
    loadstream = open_file(car(arg), "r");
    if (loadstream != NULL) {
	while (!(feof(loadstream)) && NOT_THROWING) {
	    current_line = reader(loadstream, "");
	    exec_list = parser(current_line, TRUE);
	    if (exec_list != NIL) eval_driver(exec_list);
	}
	fclose(loadstream);
	save_name = car(arg);
	runstartup(st);
    } else
	err_logo(CANT_OPEN_ERROR, car(arg));
    loadstream = tmp;
    current_line = tmp_line;
    return(UNBOUND);
}

NODE *lreadlist(NODE *args) {
    NODE *val;

    val = parser(reader(readstream, ""), FALSE);
    if (feof(readstream)) {
	return(Null_Word);
    }
    return(val);
}

NODE *lreadword(NODE *args) {
    NODE *val;

    val = reader(readstream, "RW");	/* fake prompt flags no auto-continue */
    if (feof(readstream)) {
	return(NIL);
    }
    return(val);
}

NODE *lreadrawline(NODE *args) {
    NODE *val;

    val = reader(readstream, "RAW");	/* fake prompt flags no specials */
    if (feof(readstream)) {
	return(NIL);
    }
    return(val);
}

int readchar_lookahead_buf = -1;

NODE *lreadchar(NODE *args) {
#ifdef WIN32
    MSG msg;
#endif /* WIN32 */
    char c;

    if (readchar_lookahead_buf >= 0) {
	c = (char)readchar_lookahead_buf;
	readchar_lookahead_buf = -1;
	return(make_strnode((char *)&c, (struct string_block *)NULL, 1,
		(getparity(c) ? BACKSLASH_STRING : STRING), strnzcpy));
    }
    charmode_on();
    input_blocking++;
#ifdef HAVE_WX
	if (interactive && readstream==stdin) {
	    reading_char_now = 1;
	    c = getFromWX();
	    reading_char_now = 0;
	}
	else
	    c = (char)getc(readstream);
#else
#ifndef TIOCSTI
    if (!setjmp(iblk_buf))
#endif
    {
#ifdef mac
	csetmode(C_RAW, stdin);
	while ((c = (char)getc(readstream)) == EOF && readstream == stdin);
	csetmode(C_ECHO, stdin);
#else /* !mac */
#ifdef ibm
	if (interactive && readstream==stdin)
#ifndef WIN32
		c = (char)getch();
#else /* WIN32 */
	{
	  win32_update_text();
	  if (!char_avail)
	    while(GetMessage(&msg, NULL, 0, 0))
	    {
	      TranslateMessage(&msg);
	      DispatchMessage(&msg);
	      if (char_avail)
		break;
	    }
	  c = buffered_char;
	  char_avail = 0;
	}
#endif /* WIN32 */	
	else
		c = (char)getc(readstream);

	if (c == 17) { /* control-q */
	    to_pending = 0;
	    err_logo(STOP_ERROR,NIL);
	}
	if (c == 23) { /* control-w */

#ifdef SIG_TAKES_ARG
	    logo_pause(0);
#else
	    logo_pause();
#endif
	    return(lreadchar(NIL));
	}
#else /* !ibm */
	c = (char)getc(readstream);
#endif /* ibm */
#endif /* mac */
    }
#endif /* wx */
    input_blocking = 0;
#ifdef HAVE_WX
    if ((!interactive || readstream!=stdin) && feof(readstream)) {
#else
    if (feof(readstream)) {
#endif
	return(NIL);
    }
    return(make_strnode((char *)&c, (struct string_block *)NULL, 1,
	    (getparity(c) ? BACKSLASH_STRING : STRING), strnzcpy));
}

NODE *lreadchars(NODE *args) {
    unsigned int c, i;
    struct string_block *strhead = NULL;
    char *strptr= NULL;
    NODETYPES type = STRING;
    char *cp;

    c = (unsigned int)getint(pos_int_arg(args));
    if (stopping_flag == THROWING) return UNBOUND;
    charmode_on();
    input_blocking++;
#ifndef TIOCSTI
    if (!setjmp(iblk_buf))
#endif
    {
	strhead = malloc((size_t)(c + sizeof(FIXNUM) + 1));
	if (strhead == NULL) {
	    err_logo(FILE_ERROR, make_static_strnode(message_texts[MEM_LOW]));
	    return UNBOUND;
	}
	strptr = strhead->str_str;
#ifdef HAVE_WX
	if (interactive && readstream==stdin) {
	    reading_char_now = 1;
	    cp=strptr;
	    for (i=c; i != 0; --i) {
		*cp++ = getFromWX();
	    }
	    reading_char_now = 0;
	} else
	    c = (unsigned int)fread(strptr, 1, (size_t)c, readstream);
#else
	c = (unsigned int)fread(strptr, 1, (size_t)c, readstream);
#endif
	setstrrefcnt(strhead, 0);
    }
    input_blocking = 0;
#ifndef TIOCSTI
    if (stopping_flag == THROWING) return(UNBOUND);
#endif
    if (c <= 0) {
	free(strhead);
	return(NIL);
    }
    for (i = 0; i < c; i++)
	if (getparity(strptr[i])) type = BACKSLASH_STRING;
    return(make_strnode(strptr, strhead, (int)c, type, strnzcpy));
}

NODE *leofp(NODE *args) {
    int c;

#ifdef HAVE_WX
    if (interactive && readstream==stdin) return FalseName();
#endif
    c = getc(readstream);
    if (c == EOF) return(TrueName());

    ungetc(c,readstream);
    return(FalseName());
}

NODE *lkeyp(NODE *args) {
#if defined(unix) | defined(__WXMSW__)
    long nc;
#endif
    int c;
#ifdef WIN32
    MSG msg;
    int old_mode;
#endif

    if (readchar_lookahead_buf >= 0) return TrueName();
    if (readstream == stdin && interactive) {
	charmode_on();
	fflush(stdout);
	fix_turtle_shownness();
#ifdef HAVE_WX
	return(wxKeyp() ? TrueName() : FalseName());
#else
#if defined(__RZTC__) && !defined(WIN32) /* sowings */
	zflush();
#endif

#ifdef WIN32
	win32_update_text();
#endif

#if defined(mac)
	csetmode(C_RAW, stdin);
	c = ungetc(getc(readstream), readstream);
	csetmode(C_ECHO, stdin);
	return(c == EOF ? FalseName() : TrueName());
#elif defined(ibm)
#ifdef WIN32
	old_mode = char_mode;
	char_mode = 1;
	while (PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE))
	{
	  TranslateMessage(&msg);
	  DispatchMessage(&msg);
	  if (char_avail/* ||cur_index */ )
	    break;
	}
	char_mode = old_mode;
	return ((char_avail /* ||cur_index */ ) ? TrueName() : FalseName());
#else
	return(kbhit() ? TrueName() : FalseName());
#endif
#else
#ifdef FIONREAD
	ioctl(0,FIONREAD,(int *)(&nc));
#else
	ndprintf(stdout, "%t\n", message_texts[NO_FIONREAD]);
	nc = 1;    /* pretend there's a char so we don't loop */
#endif
	if (nc > 0)
	    return(TrueName());
	else
	    return(FalseName());
#endif
#endif /* wx */
    }
    c = getc(readstream);
    if (feof(readstream))
	return(FalseName());
    else {
	ungetc(c, readstream);
	return(TrueName());
    }
}

NODE *lreadpos(NODE *args) {
    return(make_intnode(ftell(readstream)));
}

NODE *lsetreadpos(NODE *arg) {
    NODE *val = pos_int_arg(arg);

    if (NOT_THROWING) {
	fseek(readstream,getint(val),0);
    }
    return(UNBOUND);
}

NODE *lwritepos(NODE *args) {
    return(make_intnode(ftell(writestream)));
}

NODE *lsetwritepos(NODE *arg) {
    NODE *val = pos_int_arg(arg);

    if (NOT_THROWING) {
	fseek(writestream,getint(val),0);
    }
    return(UNBOUND);
}
