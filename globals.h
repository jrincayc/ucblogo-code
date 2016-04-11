/*
 *      globals.h       logo global references module           dvb
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

/* extern double atof();
extern long atol();
extern char *strncpy(), *strcpy(), *getenv(), *malloc();
extern int strncmp(); */

/* main.c */
extern NODE *current_line, *exec_list;
extern main();

/* logodata.c */
extern char *strnzcpy(), *mend_strnzcpy(), *low_strnzcpy(), *cap_strnzcpy();
extern char *mend_nosemi(), *noparity_strnzcpy();
extern char *noparitylow_strnzcpy(), *word_strnzcpy();
extern int low_strncmp(), noparity_strncmp(), noparitylow_strncmp();
extern NODE *make_strnode(char *, char *, int, NODETYPES, char *(*)());
extern NODE *make_quote(), *make_colon(), *make_strnode_prefixed();
extern NODE *make_caseobj(), *make_intnode(), *make_floatnode();
extern NODE *maybe_quote(), *cnv_node_to_strnode(), *make_static_strnode();
extern NODE *cons_list(int, ...);
extern void make_runparse();
extern NODE *cnv_node_to_numnode(), *nextname(), *llowercase(), *luppercase();
extern NODE *lgprop(), *lpprop(), *lremprop(), *lplist(), *make_array();
extern NODE *copy_list();
#ifdef ecma
extern char ecma_array[], special_chars[];
extern char ecma_set(), ecma_clear();
extern int ecma_size, ecma_get();
#endif

/* mem.c */
extern NODE *newnode(NODETYPES type);
extern NODE *_reref(), *unref(), *cons();
extern void setcar(), setcdr(), setobject(), gc();
extern NODETYPES nodetype();
extern NODE *free_list;
extern struct segment *segment_list;
extern void fill_reserve_tank(), use_reserve_tank(), check_reserve_tank();

/* parse.c */
extern FILE *loadstream, *writestream, *readstream, *dribblestream;
extern int input_blocking;
extern NODE *reader();
extern NODE *parser(), *runparse(), *lparse(), *lrunparse();
extern NODE *current_line;

/* math.c */
extern int numberp(), compare_node();
extern NODE *ladd(), *lmul(), *lsub(), *ldivide(), *lrandom(), *lrerandom();
extern NODE *lremainder(), *lbitand(), *lbitor(), *lashift(), *llshift();
extern NODE *lbitnot(), *lsin(), *lcos(), *latan(), *lsqrt(), *linteg();
extern NODE *lround(), *lexp(), *llog10(), *lln(), *lpower(), *lbitxor();
extern NODE *lradsin(), *lradcos(), *lradatan();
extern NODE *lequalp(), *llessp(), *lgreaterp();
extern NODE *lnumberp(), *l_eq(), *lbeforep(), *torf();
extern BOOLEAN equalp_help();

/* intern.c */
extern void map_oblist();
extern NODE *intern(), *make_instance();

/* debug.c */
extern show_node(), debug();

/* print.c */
extern void print_node(), print_help(), new_line(), print_space();
extern void print_char(FILE *strm, char ch);
extern void print_nobrak(), ndprintf(FILE *, char *, ...);
extern int print_stringlen;
extern char *print_stringptr;
extern int x_margin, y_margin;

/* init.c */
extern NODE *True, *False, *Left_Paren, *Right_Paren, *Null_Word, *Toplevel, *System, *Error;
extern NODE *End, *Redefp, *Caseignoredp, *Erract, *Printdepthlimit, *Printwidthlimit, *Pause;
extern NODE *If, *Ifelse, *To, *Macro, *Unbound, *Not_Enough_Node;
extern NODE *Minus_Sign, *Minus_Tight, *Startup, *Query, *Output, *Op, *Stop;
extern NODE *valref();
extern void init();

/* wrksp.c */
extern NODE *lto(), *lmake(), *llocal(), *lcontents(), *lpo(), *lpot();
extern NODE *lprocedures(), *lnames(), *lplists(), *lerase(), *ledit();
extern NODE *lbury(), *lunbury(), *lburied(), *lthing(), *lpots(), *lnamep();
extern NODE *ltrace(), *luntrace(), *lstep(), *lunstep(), *lmacrop(), *lhelp();
extern NODE *lprocedurep(), *lprimitivep(), *ldefinedp(), *lcopydef();
extern NODE *ltext(), *lfulltext(), *ldefine(), *lmacro(), *ldefmacro();
extern char *editor, *editorname, *tempdir, *addsep();
extern int to_pending;
extern NODE *anonymous_function();

/* error.c */
extern void err_print();
extern NODE *err_logo(ERR_TYPES, NODE *);
extern NODE *throw_node;
extern NODE *err_mesg;
extern NODE *lpause(), *lcontinue(), *lerror();
extern ERR_TYPES erract_errtype;

/* eval.c */
extern NODE *err_eval_driver(), *lapply(), *lqm();
extern void eval_driver(), tell_shadow();
extern int not_local();
extern NODE *fun, *ufun, *last_ufun, *this_line, *last_line, *didnt_get_output;
extern NODE *var, *var_stack, *output_node, *last_call, *didnt_output_name;
extern CTRLTYPE stopping_flag;
extern char *logolib, *helpfiles;
extern FIXNUM tailcall, val_status, dont_fix_ift;
void spush(),spop();

/* print.c */
extern NODE *lshow(), *lprint(), *ltype();
extern BOOLEAN print_backslashes;
extern void update_coords(char ch);

/* lists.c */
extern NODE *lbutfirst(), *lfirst(), *lbutlast(), *llast(), *lsentence();
extern NODE *lfput(), *llput(), *llist(), *lcount(), *lemptyp(), *lword();
extern NODE *lmemberp(), *litem(), *lwordp(), *llistp(), *lascii(), *lchar();
extern NODE *lrawascii(), *lsubstringp();
extern NODE *lsetitem(), *larray(), *larrayp(), *lform(), *lnumberp();
extern NODE *l_setfirst(), *l_setbf(), *l_setitem(), *lbackslashedp();
extern NODE *lmember(), *lfirsts(), *lbfs(), *string_arg(), *integer_arg();
extern FIXNUM int_arg();

/* files.c */
extern NODE *ldribble(), *lnodribble(), *lsetwrite(), *lsave(), *lload();
extern NODE *lopenread(), *lopenwrite(), *lclose(), *lallopen();
extern NODE *lsetread(), *lreadlist(), *lerasefile();
extern NODE *lreadword(), *leofp(), *lreadchar(), *lreadchars(), *lkeyp();
extern NODE *lopenappend(), *lopenupdate(), *lreadpos(), *lwritepos();
extern NODE *lsetreadpos(), *lsetwritepos(), *lreader(), *lwriter();
extern void silent_load(NODE *, char *);

/* coms.c */
extern NODE *lif(), *lifelse(), *loutput(), *lstop(), *lnot(), *lrunresult();
extern NODE *lcatch(), *lthrow(), *lrun(), *lrepeat(), *lforever();
extern NODE *pos_int_arg(), *lbye(), *lwait();
extern NODE *land(), *lor(), *ltest(), *liftrue(), *liffalse(), *lshell();
extern NODE *make_cont(enum labels, NODE *);
extern void prepare_to_exit();
extern FIXNUM ift_iff_flag;

/* term.c */
extern NODE *lcursor(), *lcleartext(), *lsetcursor(), *lstandout();
extern void term_init(), charmode_on(), charmode_off();
extern int x_coord, y_coord, x_max, y_max;
extern int interactive;
extern NODE *lsetmargins();

/* libloc.c */
extern char *libloc, *helploc, *temploc, *separator;

/* paren.c */
extern NODE *the_generation;
extern void untreeify_proc(NODE *);
extern void make_tree(NODE *);
extern void make_tree_from_body(NODE *);

/* graphics.c */
extern mode_type current_mode;
extern FLONUM turtle_x, turtle_y, turtle_heading, x_scale, y_scale;
extern BOOLEAN turtle_shown;
extern FIXNUM g_round(FLONUM);
extern NODE *lright(), *lleft(), *lforward(), *lback();
extern NODE *lshowturtle(), *lhideturtle(), *lshownp();
extern NODE *lhome(), *lclearscreen(), *lclean();
extern NODE *lsetheading(), *lheading(), *ltowards();
extern NODE *lpos(), *lsetpos(), *lsetxy(), *lsetx(), *lsety();
extern NODE *lwrap(), *lfence(), *lwindow(), *lscrunch();
extern NODE *lfill(), *llabel(), *larc();
extern NODE *ltextscreen(), *lsplitscreen(), *lfullscreen();
extern NODE *lpendownp(), *lpenmode(), *lpencolor(), *lpensize(), *lpenpattern();
extern NODE *lpendown(), *lpenup(), *lpenpaint();
extern NODE *lpenerase(), *lpenreverse(), *lsetpencolor();
extern NODE *lsetbackground(), *lbackground(), *lpalette(), *lsetpalette();
extern NODE *lsetpensize(), *lsetpenpattern();
extern NODE *lrefresh(), *lnorefresh();
extern NODE *lsetscrunch(), *pos_int_vector_arg(), *numeric_arg();
extern NODE *lmousepos(), *lbuttonp(), *ltone();
extern void redraw_graphics();

#ifdef mac

#define SIGQUIT SIGABRT

/* macterm.c */
extern void init_mac_memory();
extern void  term_init_mac();
extern void mac_gotoxy();
extern NODE *lsetwindowtitle(), *lsettextfont(), *lsettextsize(), *lsettextstyle();
extern NODE *lsetwindowsize(), *lsetwindowxy(), *lnewconsole(), *lcaninverse();
extern NODE *lgraphtext(), *lregulartext();

extern BOOLEAN mac_edit();
extern WindowPtr graphics_window, listener_window;
extern BOOLEAN check_mac_stop();
#endif

#ifdef ibm
/* ibmterm.c/ztcterm.c */
extern void init_ibm_memory(), check_scroll();
extern void term_init_ibm(), erase_graphics_top();
extern void ibm_gotoxy(), ibm_clear_text(), outtext();
extern void ibm_clear_screen(),t_screen(),s_screen();
extern void ibm_bold_mode(void), ibm_plain_mode(void);
extern BOOLEAN check_ibm_stop(), in_graphics_mode;
extern int MaxX, MaxY;
#ifdef __ZTC__
extern int ibm_screen_bottom;
extern void ztc_put_char(int), zflush(void), ztc_getcr(void);
extern void fix_cursor(void), newline_bugfix(void), scroll_bugfix(void);
extern void ztc_set_penc(FIXNUM);
extern BOOLEAN in_splitscreen;
#else
extern int ibm_screen_top;
#define SIGQUIT 15
#endif
#endif

#ifdef x_window
/* xgraphics.c */
extern void x_window_init();
extern void handle_x_event();
#endif
