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

#ifdef HAVE_TERMIO_H
#include <termio.h>
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
#ifndef __ZTC__
#include <alloc.h>
#endif /* ZTC */
#endif /* MSC_VER */
extern int getch(), kbhit();
#ifdef __ZTC__
#include <conio.h>
#endif
#endif

NODE *file_list = NULL;
NODE *reader_name = NIL, *writer_name = NIL;

FILE *open_file(NODE *arg, char *access) {
    char *fnstr;
    FILE *tstrm;

    arg = cnv_node_to_strnode(arg);
    if (arg == UNBOUND) return(NULL);
    fnstr = (char *) malloc((size_t)getstrlen(arg) + 1);
    if (fnstr == NULL) {
	err_logo(FILE_ERROR, make_static_strnode("Not enough memory"));
	return NULL;
    }
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
	if (dribblestream == NULL) err_logo(FILE_ERROR, NIL);
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
	if (compare_node(arg, car(t), FALSE) == 0) {
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
	err_logo(FILE_ERROR, make_static_strnode("File already open"));
    else if ((tmp = open_file(arg, mode)) != NULL) {
	push(arg, file_list);
	file_list->n_obj = (NODE *)tmp;
    }
    else
	err_logo(FILE_ERROR, make_static_strnode("I can't open that file"));
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

    if ((tmp = find_file(car(arg), TRUE)) == NULL)
	err_logo(FILE_ERROR, make_static_strnode("File not open"));
    else
	fclose(tmp);
    return(UNBOUND);
}

NODE *lsetwrite(NODE *arg) {
    FILE *tmp;

    if (car(arg) == NIL) {
	writestream = stdout;
	writer_name = NIL;
    }
    else if ((tmp = find_file(car(arg), FALSE)) != NULL) {
	writestream = tmp;
	writer_name = car(arg);
    }
    else
	err_logo(FILE_ERROR, make_static_strnode("File not open"));
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
	err_logo(FILE_ERROR, make_static_strnode("File not open"));
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
	err_logo(FILE_ERROR, make_static_strnode("Not enough memory"));
	return UNBOUND;
    }
    strnzcpy(fnstr, getstrptr(arg), getstrlen(arg));
    unlink(fnstr);
    free(fnstr);
    return(UNBOUND);
}

NODE *lsave(NODE *arg) {
    FILE *tmp;

    tmp = writestream;
    writestream = open_file(car(arg), "w+");
    if (writestream != NULL) {
	setcar(arg, cons(lcontents(NIL), NIL));
	lpo(car(arg));
	fclose(writestream);
    }
    else
	err_logo(FILE_ERROR, make_static_strnode("Could not open file"));
    writestream = tmp;
    return(UNBOUND);
}

void runstartup(NODE *oldst) {
    NODE *st;

    st = valnode__caseobj(Startup);
    if (st != oldst && st != NIL && is_list(st)) {
	val_status = 0;
	eval_driver(st);
    }
}

void silent_load(NODE *arg, char *prefix) {
    FILE *tmp_stream;
    NODE *tmp_line, *exec_list;
    char load_path[200];
    NODE *st = valnode__caseobj(Startup);
    int sv_val_status = val_status;

    /* This procedure is called three ways:
     *    silent_load(NIL,*argv)	loads *argv
     *    silent_load(proc,logolib)     loads logolib/proc
     *    silent_load(proc,NULL)	loads proc.lg
     * The "/" or ".lg" is supplied by this procedure as needed.
     */

    /*
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
	noparitylow_strnzcpy(load_path + (int)strlen(load_path),
			     getstrptr(arg), getstrlen(arg));
	if (prefix == NULL)
	  strcat(load_path, ".lg");
#ifdef WIN32
	else
	  strcpy(load_path, eight_dot_three(load_path));
#endif	
    }
    tmp_stream = loadstream;
    tmp_line = current_line;
    loadstream = fopen(load_path, "r");
    if (loadstream != NULL) {
	while (!feof(loadstream) && NOT_THROWING) {
	    current_line = reader(loadstream, "");
	    exec_list =parser(current_line, TRUE);
	    val_status = 0;
	    if (exec_list != NIL) eval_driver(exec_list);
	}
	fclose(loadstream);
	runstartup(st);
	val_status = sv_val_status;
    } else if (arg == NIL) ndprintf(stdout,"File not found: %t\n", prefix);
    loadstream = tmp_stream;
    current_line = tmp_line;
}

NODE *lload(NODE *arg) {
    FILE *tmp;
    NODE *tmp_line, *exec_list;
    NODE *st = valnode__caseobj(Startup);
    int sv_val_status = val_status;

    tmp = loadstream;
    tmp_line = current_line;
    loadstream = open_file(car(arg), "r");
    if (loadstream != NULL) {
	while (!feof(loadstream) && NOT_THROWING) {
	    current_line = reader(loadstream, "");
	    exec_list = parser(current_line, TRUE);
	    val_status = 0;
	    if (exec_list != NIL) eval_driver(exec_list);
	}
	fclose(loadstream);
	runstartup(st);
	val_status = sv_val_status;
    } else
	err_logo(FILE_ERROR, make_static_strnode("Could not open file"));
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

NODE *lreadchar(NODE *args) {
#ifdef WIN32
    MSG msg;
#endif /* WIN32 */
    char c;

    charmode_on();
    input_blocking++;
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

#if defined(__ZTC__) || defined(WIN32)
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
    input_blocking = 0;
    if (feof(readstream)) {
	return(NIL);
    }
    return(make_strnode(&c, (struct string_block *)NULL, 1,
	    (getparity(c) ? STRING : BACKSLASH_STRING), strnzcpy));
}

NODE *lreadchars(NODE *args) {
    unsigned int c, i;
    struct string_block *strhead = NULL;
    char *strptr= NULL;
    NODETYPES type = STRING;

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
	    err_logo(FILE_ERROR, make_static_strnode("Not enough memory"));
	    return UNBOUND;
	}
	strptr = strhead->str_str;
	fread(strptr, 1, (int)c, readstream);
	setstrrefcnt(strhead, 0);
    }
    input_blocking = 0;
#ifndef TIOCSTI
    if (stopping_flag == THROWING) return(UNBOUND);
#endif
    if (feof(readstream)) {
	free(strhead);
	return(NIL);
    }
    for (i = 0; i < c; i++)
	if (getparity(strptr[i])) type = BACKSLASH_STRING;
    return(make_strnode(strptr, strhead, (int)c, type, strnzcpy));
}

NODE *leofp(NODE *args) {
    int c;

    c = getc(readstream);
    if (c == EOF) return(True);

    ungetc(c,readstream);
    return(False);
}

NODE *lkeyp(NODE *args) {
    long nc;
    int c;
#ifdef WIN32
    MSG msg;
    int old_mode;
#endif

    if (readstream == stdin && interactive) {
	charmode_on();
	fflush(stdout);
#if defined(__ZTC__) && !defined(WIN32) /* sowings */
	zflush();
#endif

#ifdef WIN32
	win32_update_text();
#endif

#if defined(mac)
	csetmode(C_RAW, stdin);
	c = ungetc(getc(readstream), readstream);
	csetmode(C_ECHO, stdin);
	return(c == EOF ? False : True);
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
	return ((char_avail /* ||cur_index */ ) ? True : False);
#else
	return(kbhit() ? True : False);
#endif
#else
#ifdef FIONREAD
	ioctl(0,FIONREAD,(char *)(&nc));
#else
	ndprintf(stdout,"Can't KEYP, no FIONREAD on this system\n");
	nc = 1;    /* pretend there's a char so we don't loop */
#endif
	if (nc > 0)
	    return(True);
	else
	    return(False);
#endif
    }
    c = getc(readstream);
    if (feof(readstream))
	return(False);
    else {
	ungetc(c, readstream);
	return(True);
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
