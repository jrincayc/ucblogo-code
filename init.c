/*
 *      init.c	  logo init module			dvb
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
#include <string.h>
#include <time.h>

typedef struct priminfo {
    char *name;
    short minargs;
    short defargs;
    short maxargs;
    short priority;
    NODE *(*prim) ();
} PRIMTYPE;

NODE *True, *False, *Right_Paren, *Left_Paren, *Toplevel, *System, *Error,
     *End, *Redefp, *Caseignoredp, *Erract, *Printdepthlimit,
     *Printwidthlimit, *Pause, *LoadNoisily, *AllowGetSet,
     *UnburyOnEdit, *Fullprintp, *Make, *Listvalue, *Dotsvalue,
     *If, *Ifelse, *To, *Macro, *Unbound, *Not_Enough_Node,
     *Minus_Sign, *Minus_Tight, *Startup, *Query, *Output, *Op, *Stop,
     *Goto, *Tag;
NODE *Null_Word = NIL;

PRIMTYPE prims[] = {
    {"*", 1, 1, 1, PREFIX_PRIORITY + 3, lmul},
    {"+", 1, 1, 1, PREFIX_PRIORITY + 2, ladd},
    {"-", 1, 1, 1, PREFIX_PRIORITY + 2, lsub},
    {"--", 1, 1, 1, PREFIX_PRIORITY + 4, lsub},
    {".defmacro", 2, 2, 2, PREFIX_PRIORITY, ldefmacro},
    {".eq", 2, 2, 2, PREFIX_PRIORITY, l_eq},
    {".macro", -1, -1, -1, PREFIX_PRIORITY, lmacro},
    {".maybeoutput", 1, 1, 1, MAYBE_PRIORITY, loutput},
    {".setbf", 2, 2, 2, PREFIX_PRIORITY, l_setbf},
    {".setfirst", 2, 2, 2, PREFIX_PRIORITY, l_setfirst},
    {".setitem", 3, 3, 3, PREFIX_PRIORITY, l_setitem},
    {"/", 1, 1, 1, PREFIX_PRIORITY + 3, ldivide},
    {"<", 2, 2, 2, PREFIX_PRIORITY + 1, llessp},
    {"=", 2, 2, 2, PREFIX_PRIORITY + 1, lequalp},
    {">", 2, 2, 2, PREFIX_PRIORITY + 1, lgreaterp},
    {"?", 0, 0, 1, PREFIX_PRIORITY, lqm},
    {"allopen", 0, 0, 0, PREFIX_PRIORITY, lallopen},
    {"and", 0, 2, -1, PREFIX_PRIORITY, land},
    {"apply", 2, 2, 2, MACRO_PRIORITY, lapply},
    {"arc", 2, 2, 2, PREFIX_PRIORITY, larc},
    {"arctan", 1, 1, 2, PREFIX_PRIORITY, latan},
    {"array", 1, 1, 2, PREFIX_PRIORITY, larray},
    {"arrayp", 1, 1, 1, PREFIX_PRIORITY, larrayp},
    {"arraytolist", 1, 1, 1, PREFIX_PRIORITY, larraytolist},
    {"array?", 1, 1, 1, PREFIX_PRIORITY, larrayp},
    {"ascii", 1, 1, 1, PREFIX_PRIORITY, lascii},
    {"ashift", 2, 2, 2, PREFIX_PRIORITY, lashift},
    {"back", 1, 1, 1, PREFIX_PRIORITY, lback},
    {"background", 0, 0, 0, PREFIX_PRIORITY, lbackground},
    {"backslashedp", 1, 1, 1, PREFIX_PRIORITY, lbackslashedp},
    {"backslashed?", 1, 1, 1, PREFIX_PRIORITY, lbackslashedp},
    {"beforep", 2, 2, 2, PREFIX_PRIORITY, lbeforep},
    {"before?", 2, 2, 2, PREFIX_PRIORITY, lbeforep},
    {"bf", 1, 1, 1, PREFIX_PRIORITY, lbutfirst},
    {"bfs", 1, 1, 1, PREFIX_PRIORITY, lbfs},
    {"bg", 0, 0, 0, PREFIX_PRIORITY, lbackground},
    {"bitand", 0, 2, -1, PREFIX_PRIORITY, lbitand},
    {"bitnot", 1, 1, 1, PREFIX_PRIORITY, lbitnot},
    {"bitor", 0, 2, -1, PREFIX_PRIORITY, lbitor},
    {"bitxor", 0, 2, -1, PREFIX_PRIORITY, lbitxor},
    {"bk", 1, 1, 1, PREFIX_PRIORITY, lback},
    {"bl", 1, 1, 1, PREFIX_PRIORITY, lbutlast},
    {"buried", 0, 0, 0, PREFIX_PRIORITY, lburied},
    {"buriedp", 1, 1, 1, PREFIX_PRIORITY, lburiedp},
    {"buried?", 1, 1, 1, PREFIX_PRIORITY, lburiedp},
    {"bury", 1, 1, 1, PREFIX_PRIORITY, lbury},
    {"butfirst", 1, 1, 1, PREFIX_PRIORITY, lbutfirst},
    {"butfirsts", 1, 1, 1, PREFIX_PRIORITY, lbfs},
    {"butlast", 1, 1, 1, PREFIX_PRIORITY, lbutlast},
    {"buttonp", 0, 0, 0, PREFIX_PRIORITY, lbuttonp},
    {"button?", 0, 0, 0, PREFIX_PRIORITY, lbuttonp},
    {"bye", 0, 0, 0, PREFIX_PRIORITY, lbye},
    {"catch", 2, 2, 2, MACRO_PRIORITY, lcatch},
    {"char", 1, 1, 1, PREFIX_PRIORITY, lchar},
    {"clean", 0, 0, 0, PREFIX_PRIORITY, lclean},
    {"clearscreen", 0, 0, 0, PREFIX_PRIORITY, lclearscreen},
    {"cleartext", 0, 0, 0, PREFIX_PRIORITY, lcleartext},
    {"close", 1, 1, 1, PREFIX_PRIORITY, lclose},
    {"co", OK_NO_ARG, 1, 1, PREFIX_PRIORITY, lcontinue},
    {"contents", 0, 0, 0, PREFIX_PRIORITY, lcontents},
    {"continue", OK_NO_ARG, 1, 1, PREFIX_PRIORITY, lcontinue},
    {"copydef", 2, 2, 2, PREFIX_PRIORITY, lcopydef},
    {"cos", 1, 1, 1, PREFIX_PRIORITY, lcos},
    {"count", 1, 1, 1, PREFIX_PRIORITY, lcount},
    {"cs", 0, 0, 0, PREFIX_PRIORITY, lclearscreen},
    {"ct", 0, 0, 0, PREFIX_PRIORITY, lcleartext},
    {"cursor", 0, 0, 0, PREFIX_PRIORITY, lcursor},
    {"define", 2, 2, 2, PREFIX_PRIORITY, ldefine},
    {"definedp", 1, 1, 1, PREFIX_PRIORITY, ldefinedp},
    {"defined?", 1, 1, 1, PREFIX_PRIORITY, ldefinedp},
    {"difference", 2, 2, 2, PREFIX_PRIORITY, lsub},
    {"dribble", 1, 1, 1, PREFIX_PRIORITY, ldribble},
    {"ed", OK_NO_ARG, 1, 1, PREFIX_PRIORITY, ledit},
    {"edit", OK_NO_ARG, 1, 1, PREFIX_PRIORITY, ledit},
    {"editfile", 1, 1, 1, PREFIX_PRIORITY, leditfile},
    {"emptyp", 1, 1, 1, PREFIX_PRIORITY, lemptyp},
    {"empty?", 1, 1, 1, PREFIX_PRIORITY, lemptyp},
    {"eofp", 0, 0, 0, PREFIX_PRIORITY, leofp},
    {"eof?", 0, 0, 0, PREFIX_PRIORITY, leofp},
    {"epspict", 1, 1, 1, PREFIX_PRIORITY, lepspict},
    {"equalp", 2, 2, 2, PREFIX_PRIORITY, lequalp},
    {"equal?", 2, 2, 2, PREFIX_PRIORITY, lequalp},
    {"er", 1, 1, 1, PREFIX_PRIORITY, lerase},
    {"erall", 0, 0, 0, PREFIX_PRIORITY, lerall},
    {"erase", 1, 1, 1, PREFIX_PRIORITY, lerase},
    {"erasefile", 1, 1, 1, PREFIX_PRIORITY, lerasefile},
    {"erf", 1, 1, 1, PREFIX_PRIORITY, lerasefile},
    {"erns", 0, 0, 0, PREFIX_PRIORITY, lerns},
    {"erpls", 0, 0, 0, PREFIX_PRIORITY, lerpls},
    {"erps", 0, 0, 0, PREFIX_PRIORITY, lerps},
    {"error", 0, 0, 0, PREFIX_PRIORITY, lerror},
    {"exp", 1, 1, 1, PREFIX_PRIORITY, lexp},
    {"fd", 1, 1, 1, PREFIX_PRIORITY, lforward},
    {"fence", 0, 0, 0, PREFIX_PRIORITY, lfence},
    {"fill", 0, 0, 0, PREFIX_PRIORITY, lfill},
    {"first", 1, 1, 1, PREFIX_PRIORITY, lfirst},
    {"firsts", 1, 1, 1, PREFIX_PRIORITY, lfirsts},
    {"forever", 1, 1, 1, MACRO_PRIORITY, lforever},
    {"form", 3, 3, 3, PREFIX_PRIORITY, lform},
    {"forward", 1, 1, 1, PREFIX_PRIORITY, lforward},
    {"fput", 2, 2, 2, PREFIX_PRIORITY, lfput},
    {"fs", 0, 0, 0, PREFIX_PRIORITY, lfullscreen},
    {"fullscreen", 0, 0, 0, PREFIX_PRIORITY, lfullscreen},
    {"fulltext", 1, 1, 1, PREFIX_PRIORITY, lfulltext},
    {"gc", 0, 0, 1, PREFIX_PRIORITY, lgc},
    {"goto", 1, 1, 1, MACRO_PRIORITY, lgoto},
    {"gprop", 2, 2, 2, PREFIX_PRIORITY, lgprop},
    {"greaterp", 2, 2, 2, PREFIX_PRIORITY, lgreaterp},
    {"greater?", 2, 2, 2, PREFIX_PRIORITY, lgreaterp},
    {"heading", 0, 0, 0, PREFIX_PRIORITY, lheading},
    {"help", OK_NO_ARG, 1, 1, PREFIX_PRIORITY, lhelp},
    {"hideturtle", 0, 0, 0, PREFIX_PRIORITY, lhideturtle},
    {"home", 0, 0, 0, PREFIX_PRIORITY, lhome},
    {"ht", 0, 0, 0, PREFIX_PRIORITY, lhideturtle},
    {"if", 2, 2, 3, MACRO_PRIORITY, lif},
    {"ifelse", 3, 3, 3, MACRO_PRIORITY, lifelse},
    {"iff", 1, 1, 1, MACRO_PRIORITY, liffalse},
    {"iffalse", 1, 1, 1, MACRO_PRIORITY, liffalse},
    {"ift", 1, 1, 1, MACRO_PRIORITY, liftrue},
    {"iftrue", 1, 1, 1, MACRO_PRIORITY, liftrue},
    {"int", 1, 1, 1, PREFIX_PRIORITY, linteg},
    {"item", 2, 2, 2, PREFIX_PRIORITY, litem},
    {"keyp", 0, 0, 0, PREFIX_PRIORITY, lkeyp},
    {"key?", 0, 0, 0, PREFIX_PRIORITY, lkeyp},
    {"label", 1, 1, 1, PREFIX_PRIORITY, llabel},
    {"last", 1, 1, 1, PREFIX_PRIORITY, llast},
    {"left", 1, 1, 1, PREFIX_PRIORITY, lleft},
    {"lessp", 2, 2, 2, PREFIX_PRIORITY, llessp},
    {"less?", 2, 2, 2, PREFIX_PRIORITY, llessp},
    {"list", 0, 2, -1, PREFIX_PRIORITY, llist},
    {"listp", 1, 1, 1, PREFIX_PRIORITY, llistp},
    {"listtoarray", 1, 1, 2, PREFIX_PRIORITY, llisttoarray},
    {"list?", 1, 1, 1, PREFIX_PRIORITY, llistp},
    {"ln", 1, 1, 1, PREFIX_PRIORITY, lln},
    {"load", 1, 1, 1, PREFIX_PRIORITY, lload},
    {"loadpict", 1, 1, 1, PREFIX_PRIORITY, lloadpict},
    {"local", 1, 1, -1, PREFIX_PRIORITY, llocal},
    {"log10", 1, 1, 1, PREFIX_PRIORITY, llog10},
    {"lowercase", 1, 1, 1, PREFIX_PRIORITY, llowercase},
    {"lput", 2, 2, 2, PREFIX_PRIORITY, llput},
    {"lshift", 2, 2, 2, PREFIX_PRIORITY, llshift},
    {"lt", 1, 1, 1, PREFIX_PRIORITY, lleft},
    {"macrop", 1, 1, 1, PREFIX_PRIORITY, lmacrop},
    {"macro?", 1, 1, 1, PREFIX_PRIORITY, lmacrop},
    {"make", 2, 2, 2, PREFIX_PRIORITY, lmake},
    {"member", 2, 2, 2, PREFIX_PRIORITY, lmember},
    {"memberp", 2, 2, 2, PREFIX_PRIORITY, lmemberp},
    {"member?", 2, 2, 2, PREFIX_PRIORITY, lmemberp},
    {"minus", 1, 1, 1, PREFIX_PRIORITY, lsub},
    {"modulo", 2, 2, 2, PREFIX_PRIORITY, lmodulo},
    {"mousepos", 0, 0, 0, PREFIX_PRIORITY, lmousepos},
    {"namep", 1, 1, 1, PREFIX_PRIORITY, lnamep},
    {"name?", 1, 1, 1, PREFIX_PRIORITY, lnamep},
    {"names", 0, 0, 0, PREFIX_PRIORITY, lnames},
    {"nodes", 0, 0, 0, PREFIX_PRIORITY, lnodes},
    {"nodribble", 0, 0, 0, PREFIX_PRIORITY, lnodribble},
    {"norefresh", 0, 0, 0, PREFIX_PRIORITY, lnorefresh},
    {"not", 1, 1, 1, PREFIX_PRIORITY, lnot},
    {"numberp", 1, 1, 1, PREFIX_PRIORITY, lnumberp},
    {"number?", 1, 1, 1, PREFIX_PRIORITY, lnumberp},
    {"op", 1, 1, 1, OUTPUT_PRIORITY, loutput},
    {"openappend", 1, 1, 1, PREFIX_PRIORITY, lopenappend},
    {"openread", 1, 1, 1, PREFIX_PRIORITY, lopenread},
    {"openupdate", 1, 1, 1, PREFIX_PRIORITY, lopenupdate},
    {"openwrite", 1, 1, 1, PREFIX_PRIORITY, lopenwrite},
    {"or", 0, 2, -1, PREFIX_PRIORITY, lor},
    {"output", 1, 1, 1, OUTPUT_PRIORITY, loutput},
    {"palette", 1, 1, 1, PREFIX_PRIORITY, lpalette},
    {"parse", 1, 1, 1, PREFIX_PRIORITY, lparse},
    {"pause", 0, 0, 0, PREFIX_PRIORITY, lpause},	       
    {"pc", 0, 0, 0, PREFIX_PRIORITY, lpencolor},
    {"pd", 0, 0, 0, PREFIX_PRIORITY, lpendown},
    {"pe", 0, 0, 0, PREFIX_PRIORITY, lpenerase},
    {"pencolor", 0, 0, 0, PREFIX_PRIORITY, lpencolor},
    {"pendown", 0, 0, 0, PREFIX_PRIORITY, lpendown},
    {"pendownp", 0, 0, 0, PREFIX_PRIORITY, lpendownp},
    {"pendown?", 0, 0, 0, PREFIX_PRIORITY, lpendownp},
    {"penerase", 0, 0, 0, PREFIX_PRIORITY, lpenerase},
    {"penmode", 0, 0, 0, PREFIX_PRIORITY, lpenmode},
    {"penpaint", 0, 0, 0, PREFIX_PRIORITY, lpenpaint},
    {"penpattern", 0, 0, 0, PREFIX_PRIORITY, lpenpattern},
    {"penreverse", 0, 0, 0, PREFIX_PRIORITY, lpenreverse},
    {"pensize", 0, 0, 0, PREFIX_PRIORITY, lpensize},
    {"penup", 0, 0, 0, PREFIX_PRIORITY, lpenup},
    {"plist", 1, 1, 1, PREFIX_PRIORITY, lplist},
    {"plistp", 1, 1, 1, PREFIX_PRIORITY, lplistp},
    {"plist?", 1, 1, 1, PREFIX_PRIORITY, lplistp},
    {"plists", 0, 0, 0, PREFIX_PRIORITY, lplists},
    {"po", 1, 1, 1, PREFIX_PRIORITY, lpo},
    {"pos", 0, 0, 0, PREFIX_PRIORITY, lpos},
    {"pot", 1, 1, 1, PREFIX_PRIORITY, lpot},
    {"power", 2, 2, 2, PREFIX_PRIORITY, lpower},
    {"pprop", 3, 3, 3, PREFIX_PRIORITY, lpprop},
    {"ppt", 0, 0, 0, PREFIX_PRIORITY, lpenpaint},
    {"pr", 0, 1, -1, PREFIX_PRIORITY, lprint},
    {"prefix", 0, 0, 0, PREFIX_PRIORITY, lprefix},
    {"primitivep", 1, 1, 1, PREFIX_PRIORITY, lprimitivep},
    {"primitive?", 1, 1, 1, PREFIX_PRIORITY, lprimitivep},
    {"print", 0, 1, -1, PREFIX_PRIORITY, lprint},
    {"procedurep", 1, 1, 1, PREFIX_PRIORITY, lprocedurep},
    {"procedure?", 1, 1, 1, PREFIX_PRIORITY, lprocedurep},
    {"procedures", 0, 0, 0, PREFIX_PRIORITY, lprocedures},
    {"product", 0, 2, -1, PREFIX_PRIORITY, lmul},
    {"pu", 0, 0, 0, PREFIX_PRIORITY, lpenup},
    {"px", 0, 0, 0, PREFIX_PRIORITY, lpenreverse},
    {"quotient", 1, 2, 2, PREFIX_PRIORITY, ldivide},
    {"radarctan", 1, 1, 2, PREFIX_PRIORITY, lradatan},
    {"radcos", 1, 1, 1, PREFIX_PRIORITY, lradcos},
    {"radsin", 1, 1, 1, PREFIX_PRIORITY, lradsin},
    {"random", 1, 1, 1, PREFIX_PRIORITY, lrandom},
    {"rawascii", 1, 1, 1, PREFIX_PRIORITY, lrawascii},
    {"rc", 0, 0, 0, PREFIX_PRIORITY, lreadchar},
    {"rcs", 1, 1, 1, PREFIX_PRIORITY, lreadchars},
    {"readchar", 0, 0, 0, PREFIX_PRIORITY, lreadchar},
    {"readchars", 1, 1, 1, PREFIX_PRIORITY, lreadchars},
    {"reader", 0, 0, 0, PREFIX_PRIORITY, lreader},
    {"readlist", 0, 0, 0, PREFIX_PRIORITY, lreadlist},
    {"readpos", 0, 0, 0, PREFIX_PRIORITY, lreadpos},
    {"readrawline", 0, 0, 0, PREFIX_PRIORITY, lreadrawline},
    {"readword", 0, 0, 0, PREFIX_PRIORITY, lreadword},
    {"refresh", 0, 0, 0, PREFIX_PRIORITY, lrefresh},
    {"remainder", 2, 2, 2, PREFIX_PRIORITY, lremainder},
    {"remprop", 2, 2, 2, PREFIX_PRIORITY, lremprop},
    {"repcount", 0, 0, 0, PREFIX_PRIORITY, lrepcount},
    {"repeat", 2, 2, 2, MACRO_PRIORITY, lrepeat},
    {"rerandom", 0, 0, 1, PREFIX_PRIORITY, lrerandom},
    {"right", 1, 1, 1, PREFIX_PRIORITY, lright},
    {"rl", 0, 0, 0, PREFIX_PRIORITY, lreadlist},
    {"round", 1, 1, 1, PREFIX_PRIORITY, lround},
    {"rt", 1, 1, 1, PREFIX_PRIORITY, lright},
    {"run", 1, 1, 1, MACRO_PRIORITY, lrun},
    {"runparse", 1, 1, 1, PREFIX_PRIORITY, lrunparse},
    {"runresult", 1, 1, 1, MACRO_PRIORITY, lrunresult},
    {"rw", 0, 0, 0, PREFIX_PRIORITY, lreadword},
    {"save", 1, 1, 1, PREFIX_PRIORITY, lsave},
    {"savepict", 1, 1, 1, PREFIX_PRIORITY, lsavepict},
    {"scrunch", 0, 0, 0, PREFIX_PRIORITY, lscrunch},
    {"se", 0, 2, -1, PREFIX_PRIORITY, lsentence},
    {"sentence", 0, 2, -1, PREFIX_PRIORITY, lsentence},
    {"setbg", 1, 1, 1, PREFIX_PRIORITY, lsetbackground},
    {"setbackground", 1, 1, 1, PREFIX_PRIORITY, lsetbackground},
    {"setcursor", 1, 1, 1, PREFIX_PRIORITY, lsetcursor},
    {"seth", 1, 1, 1, PREFIX_PRIORITY, lsetheading},
    {"setheading", 1, 1, 1, PREFIX_PRIORITY, lsetheading},
    {"setitem", 3, 3, 3, PREFIX_PRIORITY, lsetitem},
    {"setmargins", 1, 1, 1, PREFIX_PRIORITY, lsetmargins},
    {"setpalette", 2, 2, 2, PREFIX_PRIORITY, lsetpalette},
    {"setpc", 1, 1, 1, PREFIX_PRIORITY, lsetpencolor},
    {"setpencolor", 1, 1, 1, PREFIX_PRIORITY, lsetpencolor},
    {"setpenpattern", 1, 1, 1, PREFIX_PRIORITY, lsetpenpattern},
    {"setpensize", 1, 1, 1, PREFIX_PRIORITY, lsetpensize},
    {"setpos", 1, 1, 1, PREFIX_PRIORITY, lsetpos},
    {"setprefix", 1, 1, 1, PREFIX_PRIORITY, lsetprefix},
    {"setread", 1, 1, 1, PREFIX_PRIORITY, lsetread},
    {"setreadpos", 1, 1, 1, PREFIX_PRIORITY, lsetreadpos},
    {"setscrunch", 2, 2, 2, PREFIX_PRIORITY, lsetscrunch},
#if defined(WIN32)|defined(ibm)
    {"settc", 2, 2, 2, PREFIX_PRIORITY, set_text_color},
    {"settextcolor", 2, 2, 2, PREFIX_PRIORITY, set_text_color},
#endif
    {"setwrite", 1, 1, 1, PREFIX_PRIORITY, lsetwrite},
    {"setwritepos", 1, 1, 1, PREFIX_PRIORITY, lsetwritepos},
    {"setx", 1, 1, 1, PREFIX_PRIORITY, lsetx},
    {"setxy", 2, 2, 2, PREFIX_PRIORITY, lsetxy},
    {"sety", 1, 1, 1, PREFIX_PRIORITY, lsety},
    {"shell", 1, 1, 2, PREFIX_PRIORITY, lshell},
    {"show", 0, 1, -1, PREFIX_PRIORITY, lshow},
    {"shownp", 0, 0, 0, PREFIX_PRIORITY, lshownp},
    {"shown?", 0, 0, 0, PREFIX_PRIORITY, lshownp},
    {"showturtle", 0, 0, 0, PREFIX_PRIORITY, lshowturtle},
    {"sin", 1, 1, 1, PREFIX_PRIORITY, lsin},
    {"splitscreen", 0, 0, 0, PREFIX_PRIORITY, lsplitscreen},
    {"sqrt", 1, 1, 1, PREFIX_PRIORITY, lsqrt},
    {"ss", 0, 0, 0, PREFIX_PRIORITY, lsplitscreen},
    {"st", 0, 0, 0, PREFIX_PRIORITY, lshowturtle},
    {"standout", 1, 1, 1, PREFIX_PRIORITY, lstandout},
    {"step", 1, 1, 1, PREFIX_PRIORITY, lstep},
    {"stepped", 0, 0, 0, PREFIX_PRIORITY, lstepped},
    {"steppedp", 1, 1, 1, PREFIX_PRIORITY, lsteppedp},
    {"stepped?", 1, 1, 1, PREFIX_PRIORITY, lsteppedp},
    {"stop", 0, 0, 0, STOP_PRIORITY, lstop},
    {"substringp", 2, 2, 2, PREFIX_PRIORITY, lsubstringp},
    {"substring?", 2, 2, 2, PREFIX_PRIORITY, lsubstringp},
    {"sum", 0, 2, -1, PREFIX_PRIORITY, ladd},
    {"tag", 1, 1, 1, PREFIX_PRIORITY, ltag},
    {"test", 1, 1, 1, PREFIX_PRIORITY, ltest},
    {"text", 1, 1, 1, PREFIX_PRIORITY, ltext},
    {"textscreen", 0, 0, 0, PREFIX_PRIORITY, ltextscreen},
    {"thing", 1, 1, 1, PREFIX_PRIORITY, lthing},
    {"throw", 1, 1, 2, PREFIX_PRIORITY, lthrow},
    {"to", -1, -1, -1, PREFIX_PRIORITY, lto},
    {"tone", 2, 2, 2, PREFIX_PRIORITY, ltone},
    {"towards", 1, 1, 1, PREFIX_PRIORITY, ltowards},
    {"trace", 1, 1, 1, PREFIX_PRIORITY, ltrace},
    {"traced", 0, 0, 0, PREFIX_PRIORITY, ltraced},
    {"tracedp", 1, 1, 1, PREFIX_PRIORITY, ltracedp},
    {"traced?", 1, 1, 1, PREFIX_PRIORITY, ltracedp},
    {"ts", 0, 0, 0, PREFIX_PRIORITY, ltextscreen},
    {"type", 0, 1, -1, PREFIX_PRIORITY, ltype},
    {"unbury", 1, 1, 1, PREFIX_PRIORITY, lunbury},
    {"unstep", 1, 1, 1, PREFIX_PRIORITY, lunstep},
    {"untrace", 1, 1, 1, PREFIX_PRIORITY, luntrace},
    {"uppercase", 1, 1, 1, PREFIX_PRIORITY, luppercase},
    {"wait", 1, 1, 1, PREFIX_PRIORITY, lwait},
    {"window", 0, 0, 0, PREFIX_PRIORITY, lwindow},
    {"word", 0, 2, -1, PREFIX_PRIORITY, lword},
    {"wordp", 1, 1, 1, PREFIX_PRIORITY, lwordp},
    {"word?", 1, 1, 1, PREFIX_PRIORITY, lwordp},
    {"wrap", 0, 0, 0, PREFIX_PRIORITY, lwrap},
    {"writepos", 0, 0, 0, PREFIX_PRIORITY, lwritepos},
    {"writer", 0, 0, 0, PREFIX_PRIORITY, lwriter},

#ifdef mac
    {"setwindowtitle", 1, 1, 1, PREFIX_PRIORITY, lsetwindowtitle},
    {"settextfont", 1, 1, 1, PREFIX_PRIORITY, lsettextfont},
    {"settextsize", 1, 1, 1, PREFIX_PRIORITY, lsettextsize},
    {"settextstyle", 1, 1, 1, PREFIX_PRIORITY, lsettextstyle},
    {"setwindowsize", 1, 1, 1, PREFIX_PRIORITY, lsetwindowsize},
    {"setwindowxy", 1, 1, 1, PREFIX_PRIORITY, lsetwindowxy},
    {"newconsole", 0, 0, 0, PREFIX_PRIORITY, lnewconsole},
    {"graphtext", 0, 0, 0, PREFIX_PRIORITY, lgraphtext},
    {"regulartext", 0, 0, 0, PREFIX_PRIORITY, lregulartext},
    {"caninverse", 1, 1, 1, PREFIX_PRIORITY, lcaninverse},
#endif

    {0, 0, 0, 0, 0, 0}
};

NODE *intern_p(NODE *caseobj) {
    NODE *result = intern(caseobj);
    setflag__caseobj(result, PERMANENT);
    return result;
}

void init(void) {
    int i = 0;
    NODE *proc = NIL, *pname = NIL, *cnd = NIL;
    FILE *fp;
    char linebuf[100];

    readstream = stdin;
    writestream = stdout;
    loadstream = stdin;

    fill_reserve_tank();
    oldyoungs = Unbound = newnode(PUNBOUND);

#ifdef HAVE_SRANDOM
    srandom((int)time((time_t *)NULL));
#else
    srand((int)time((time_t *)NULL));
#endif
#ifdef ecma
    for (i=0; i<128; i++)
	ecma_array[i] = i;
    for (i=0; i<ecma_size; i++)
	ecma_array[(int)special_chars[i]] = ecma_begin+i;
    i = 0;
#endif
    logolib = getenv("LOGOLIB");
    if (logolib == NULL) logolib = libloc;
    helpfiles = getenv("LOGOHELP");
    if (helpfiles == NULL) helpfiles = helploc;
    editor = getenv("EDITOR");
    if (editor == NULL) editor = "jove";
    editorname = strrchr(editor, (int)'/');
    editorname = (editorname ? editorname+1 : editor);
    tempdir = getenv("TEMP");
    if (tempdir == NULL) tempdir = temploc;
    while (prims[i].name) {
	if (prims[i].priority == MACRO_PRIORITY)
	    proc = newnode(MACRO);
	else if (prims[i].priority <= TAIL_PRIORITY)
	    proc = newnode(TAILFORM);
	else if ((prims[i].priority & ~4) == (PREFIX_PRIORITY & ~4))
	    proc = newnode(PRIM); /* incl. -- */
	else
	    proc = newnode(INFIX);
	setprimpri(proc, prims[i].priority);
	setprimfun(proc, prims[i].prim);
	setprimdflt(proc, prims[i].defargs);
	setprimmax(proc, prims[i].maxargs);
	setprimmin(proc, prims[i].minargs);
	pname = make_static_strnode(prims[i].name);
	cnd = make_instance(pname, pname);
	setprocnode__caseobj(cnd, proc);
	if (nodetype(proc) == MACRO)
	    setflag__caseobj(cnd, PROC_MACRO);
	setflag__caseobj(cnd, PERMANENT);
	i++;
    }
    True = intern_p(make_static_strnode("true"));
    False = intern_p(make_static_strnode("false"));
    Left_Paren = intern_p(make_static_strnode("("));
    Right_Paren = intern_p(make_static_strnode(")"));
    Minus_Sign = intern_p(make_static_strnode("-"));
    Minus_Tight = intern_p(make_static_strnode("--"));
    Query = intern_p(make_static_strnode("?"));
    Null_Word = intern_p(make_static_strnode("\0"));
    To = intern_p(make_static_strnode("to"));
    Macro = intern_p(make_static_strnode(".macro"));
    Toplevel = intern_p(make_static_strnode("toplevel"));
    System = intern_p(make_static_strnode("system"));
    Error = intern_p(make_static_strnode("error"));
    End = intern_p(make_static_strnode("end"));
    If = intern_p(make_static_strnode("if"));
    Ifelse = intern_p(make_static_strnode("ifelse"));
    Redefp = intern_p(make_static_strnode("redefp"));
    Caseignoredp = intern_p(make_static_strnode("caseignoredp"));
    setvalnode__caseobj(Caseignoredp, True);
    setflag__caseobj(Caseignoredp, VAL_BURIED);
    Erract = intern_p(make_static_strnode("erract"));
    Printdepthlimit = intern_p(make_static_strnode("printdepthlimit"));
    Printwidthlimit = intern_p(make_static_strnode("printwidthlimit"));
    LoadNoisily = intern_p(make_static_strnode("loadnoisily"));
    AllowGetSet = intern_p(make_static_strnode("allowgetset"));
    setvalnode__caseobj(AllowGetSet, True);
    setflag__caseobj(AllowGetSet, VAL_BURIED);
    Fullprintp = intern_p(make_static_strnode("fullprintp"));
    setvalnode__caseobj(Fullprintp, False);
    setflag__caseobj(Fullprintp, VAL_BURIED);
    UnburyOnEdit = intern_p(make_static_strnode("unburyonedit"));
    setvalnode__caseobj(UnburyOnEdit, True);
    setflag__caseobj(UnburyOnEdit, VAL_BURIED);
    Make = intern_p(make_static_strnode("make"));
    Listvalue = cons(intern_p(make_static_strnode("value")),NIL);
    Dotsvalue = make_colon(car(Listvalue));
    Pause = intern_p(make_static_strnode("pause"));
    Startup = intern_p(make_static_strnode("startup"));
    Output = intern_p(make_static_strnode("output"));
    Op = intern_p(make_static_strnode("op"));
    Stop = intern_p(make_static_strnode("stop"));
    Goto = intern_p(make_static_strnode("goto"));
    Tag = intern_p(make_static_strnode("Tag"));
    the_generation = cons(NIL, NIL);
    Not_Enough_Node = cons(NIL, NIL);

    sprintf(linebuf,"%s%sMessages", logolib, separator);
    fp = fopen(linebuf, "r");
    if (fp == NULL) {
	printf("Error -- Can't read Messages file.\n");
	exit(1);
    }
    for (i=0; i<MAX_MESSAGE; i++) {
	fgets(linebuf, 99, fp);
	linebuf[strlen(linebuf)-1] = '\0';
	message_texts[i] = (char *) malloc(1+strlen(linebuf));
	strcpy(message_texts[i], linebuf);
    }
    fclose(fp);
}
