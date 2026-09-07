/*
 *      init.c	  logo init module			dvb
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
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WIN32
#include <windows.h>
#endif

//#ifdef HAVE_UNISTD_H
#ifndef WIN32
#include <unistd.h>
#endif

#define WANT_EVAL_REGS 1
#include "logo.h"
#include "globals.h"
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef HAVE_WX
void getExecutableDir(char * path, int maxlen);
#endif

typedef struct priminfo {
    char *name;
    short minargs;
    short defargs;
    short maxargs;
    short priority;
    NODE *(*prim) (NODE *);
} PRIMTYPE;

NODE *Right_Paren, *Left_Paren, *Redefp, *Caseignoredp, *Erract, *Printdepthlimit,
     *Printwidthlimit, *Pause, *LoadNoisily, *AllowGetSet,
     *UnburyOnEdit, *Fullprintp, *Make, *Listvalue, *Dotsvalue,
     *Unbound, *Not_Enough_Node, *Buttonact, *LogoVersion, *LogoPlatform,
     *LogoLogo, *CommandLine, *Keyact,
#ifdef OBJECTS
     *askexist,
#endif
     *Minus_Sign, *Minus_Tight, *Startup, *Startuplg,
     *Query, *UseAlternateNames;
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
    {".setsegmentsize", 1, 1, 1, PREFIX_PRIORITY, lsetsegsz},
    {"/", 1, 1, 1, PREFIX_PRIORITY + 3, ldivide},
    {"<", 2, 2, 2, PREFIX_PRIORITY + 1, llessp},
    {"=", 2, 2, 2, PREFIX_PRIORITY + 1, lequalp},
    {">", 2, 2, 2, PREFIX_PRIORITY + 1, lgreaterp},
    {"<=", 2, 2, 2, PREFIX_PRIORITY + 1, llessequalp},
    {"<>", 2, 2, 2, PREFIX_PRIORITY + 1, lnotequalp},
    {">=", 2, 2, 2, PREFIX_PRIORITY + 1, lgreaterequalp},
    {"?", 0, 0, 1, PREFIX_PRIORITY, lqm},
    {"allopen", 0, 0, 0, PREFIX_PRIORITY, lallopen},
    {"and", 0, 2, -1, PREFIX_PRIORITY, land},
#ifdef ENABLE_NL
    {"en", 0, 2, -1, PREFIX_PRIORITY, land},
#endif
    {"apply", 2, 2, 2, MACRO_PRIORITY, lapply},
    {"arc", 2, 2, 2, PREFIX_PRIORITY, larc},
    {"arctan", 1, 1, 2, PREFIX_PRIORITY, latan},
#ifdef ENABLE_NL
    {"arctangens", 1, 1, 2, PREFIX_PRIORITY, latan},
#endif
    {"arity", 1, 1, 1, PREFIX_PRIORITY, larity},
    {"array", 1, 1, 2, PREFIX_PRIORITY, larray},
    {"arrayp", 1, 1, 1, PREFIX_PRIORITY, larrayp},
    {"arraytolist", 1, 1, 1, PREFIX_PRIORITY, larraytolist},
    {"array?", 1, 1, 1, PREFIX_PRIORITY, larrayp},
    {"ascii", 1, 1, 1, PREFIX_PRIORITY, lascii},
    {"ashift", 2, 2, 2, PREFIX_PRIORITY, lashift},
#ifdef OBJECTS
    {"ask", 2, 2, 2, MACRO_PRIORITY, lask},
#ifdef ENABLE_NL
    {"neem", 2, 2, 2, MACRO_PRIORITY, lask},
#endif
#endif
    {"back", 1, 1, 1, PREFIX_PRIORITY, lback},
#ifdef ENABLE_NL
    {"achteruit", 1, 1, 1, PREFIX_PRIORITY, lback},
    {"at", 1, 1, 1, PREFIX_PRIORITY, lback},
#endif
    {"background", 0, 0, 0, PREFIX_PRIORITY, lbackground},
#ifdef ENABLE_NL
    {"achtergrond", 0, 0, 0, PREFIX_PRIORITY, lbackground},
    {"ag", 0, 0, 0, PREFIX_PRIORITY, lbackground},
#endif
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
#ifdef ENABLE_NL
    {"verborg", 0, 0, 0, PREFIX_PRIORITY, lburied},
#endif
    {"buriedp", 1, 1, 1, PREFIX_PRIORITY, lburiedp},
    {"buried?", 1, 1, 1, PREFIX_PRIORITY, lburiedp},
    {"bury", 1, 1, 1, PREFIX_PRIORITY, lbury},
#ifdef ENABLE_NL
    {"verberg", 1, 1, 1, PREFIX_PRIORITY, lbury},
#endif
    {"butfirst", 1, 1, 1, PREFIX_PRIORITY, lbutfirst},
#ifdef ENABLE_NL
    {"me", 1, 1, 1, PREFIX_PRIORITY, lbutfirst},
    {"mineerste", 1, 1, 1, PREFIX_PRIORITY, lbutfirst},
#endif
    {"butfirsts", 1, 1, 1, PREFIX_PRIORITY, lbfs},
    {"butlast", 1, 1, 1, PREFIX_PRIORITY, lbutlast},
#ifdef ENABLE_NL
    {"minlaatste", 1, 1, 1, PREFIX_PRIORITY, lbutlast},
    {"ml", 1, 1, 1, PREFIX_PRIORITY, lbutlast},
#endif
    {"button", 0, 0, 0, PREFIX_PRIORITY, lbutton},
    {"buttonp", 0, 0, 0, PREFIX_PRIORITY, lbuttonp},
    {"button?", 0, 0, 0, PREFIX_PRIORITY, lbuttonp},
    {"bye", 0, 0, 0, PREFIX_PRIORITY, lbye},
#ifdef ENABLE_NL
    {"totziens", 0, 0, 0, PREFIX_PRIORITY, lbye},
#endif
    {"catch", 2, 2, 2, MACRO_PRIORITY, lcatch},
#ifdef ENABLE_NL
    {"vang", 2, 2, 2, MACRO_PRIORITY, lcatch},
#endif
    {"char", 1, 1, 1, PREFIX_PRIORITY, lchar},
#ifdef ENABLE_NL
    {"teken", 1, 1, 1, PREFIX_PRIORITY, lchar},
#endif
    {"clean", 0, 0, 0, PREFIX_PRIORITY, lclean},
#ifdef ENABLE_NL
    {"wegtekening", 0, 0, 0, PREFIX_PRIORITY, lclean},
#endif
    {"clearscreen", 0, 0, 0, PREFIX_PRIORITY, lclearscreen},
#ifdef ENABLE_NL
    {"wistekening", 0, 0, 0, PREFIX_PRIORITY, lclearscreen},
    {"wt", 0, 0, 0, PREFIX_PRIORITY, lclearscreen},
#endif
    {"cleartext", 0, 0, 0, PREFIX_PRIORITY, lcleartext},
#ifdef ENABLE_NL
    {"wisopdracht", 0, 0, 0, PREFIX_PRIORITY, lcleartext},
    {"wo", 0, 0, 0, PREFIX_PRIORITY, lcleartext},
#endif
#ifdef HAVE_WX
    {"clickpos", 0, 0, 0, PREFIX_PRIORITY, lclickpos},
#endif
    {"close", 1, 1, 1, PREFIX_PRIORITY, lclose},
#ifdef ENABLE_NL
    {"sluit", 1, 1, 1, PREFIX_PRIORITY, lclose},
#endif
    {"co", OK_NO_ARG, 1, 1, PREFIX_PRIORITY, lcontinue},
    {"contents", 0, 0, 0, PREFIX_PRIORITY, lcontents},
    {"continue", OK_NO_ARG, 1, 1, PREFIX_PRIORITY, lcontinue},
    {"copydef", 2, 2, 2, PREFIX_PRIORITY, lcopydef},
    {"cos", 1, 1, 1, PREFIX_PRIORITY, lcos},
#ifdef ENABLE_NL
    {"cosinus", 1, 1, 1, PREFIX_PRIORITY, lcos},
    {"tan", 1, 1, 1, PREFIX_PRIORITY, ltan},
    {"tangens", 1, 1, 1, PREFIX_PRIORITY, ltan},
    {"abs", 1, 1, 1, PREFIX_PRIORITY, labsx},
    {"absolutewaarde", 1, 1, 1, PREFIX_PRIORITY, labsx},
#endif
    {"count", 1, 1, 1, PREFIX_PRIORITY, lcount},
#ifdef ENABLE_NL
    {"aantal", 1, 1, 1, PREFIX_PRIORITY, lcount},
#endif
    {"cs", 0, 0, 0, PREFIX_PRIORITY, lclearscreen},
    {"cslsload", 1, 1, 1, PREFIX_PRIORITY, lcslsload},
    {"ct", 0, 0, 0, PREFIX_PRIORITY, lcleartext},
    {"cursor", 0, 0, 0, PREFIX_PRIORITY, lcursor},
#ifdef HAVE_WX
    {"decreasefont", 0, 0, 0, PREFIX_PRIORITY, DecreaseFont},
#endif
    {"define", 2, 2, 2, PREFIX_PRIORITY, ldefine},
#ifdef ENABLE_NL
    {"definieer", 2, 2, 2, PREFIX_PRIORITY, ldefine},
#endif
    {"definedp", 1, 1, 1, PREFIX_PRIORITY, ldefinedp},
    {"defined?", 1, 1, 1, PREFIX_PRIORITY, ldefinedp},
#ifdef ENABLE_NL
    {"definitie?", 1, 1, 1, PREFIX_PRIORITY, ldefinedp},
#endif
    {"difference", 2, 2, 2, PREFIX_PRIORITY, lsub},
#ifdef ENABLE_NL
    {"verschil", 2, 2, 2, PREFIX_PRIORITY, lsub},
#endif
    {"dribble", 1, 1, 1, PREFIX_PRIORITY, ldribble},
    {"ed", OK_NO_ARG, 1, 1, PREFIX_PRIORITY, ledit},
    {"edit", OK_NO_ARG, 1, 1, PREFIX_PRIORITY, ledit},
#ifdef ENABLE_NL
    {"bewerk", OK_NO_ARG, 1, 1, PREFIX_PRIORITY, ledit},
    {"bw", OK_NO_ARG, 1, 1, PREFIX_PRIORITY, ledit},
#endif
    {"editfile", 1, 1, 1, PREFIX_PRIORITY, leditfile},
#ifdef ENABLE_NL
    {"bb", 1, 1, 1, PREFIX_PRIORITY, leditfile},
    {"bewerkbestand", 1, 1, 1, PREFIX_PRIORITY, leditfile},
#endif
    {"emptyp", 1, 1, 1, PREFIX_PRIORITY, lemptyp},
#ifdef ENABLE_NL
    {"leeg?", 1, 1, 1, PREFIX_PRIORITY, lemptyp},
#endif
    {"empty?", 1, 1, 1, PREFIX_PRIORITY, lemptyp},
    {"eofp", 0, 0, 0, PREFIX_PRIORITY, leofp},
    {"eof?", 0, 0, 0, PREFIX_PRIORITY, leofp},
    {"epspict", 1, 1, 1, PREFIX_PRIORITY, lepspict},
    {"equalp", 2, 2, 2, PREFIX_PRIORITY, lequalp},
#ifdef ENABLE_NL
    {"gelijk?", 2, 2, 2, PREFIX_PRIORITY, lequalp},
#endif
    {"equal?", 2, 2, 2, PREFIX_PRIORITY, lequalp},
    {"er", 1, 1, 1, PREFIX_PRIORITY, lerase},
    {"erall", 0, 0, 0, PREFIX_PRIORITY, lerall},
#ifdef ENABLE_NL
    {"wisalles", 0, 0, 0, PREFIX_PRIORITY, lerall},
#endif
    {"erase", 1, 1, 1, PREFIX_PRIORITY, lerase},
#ifdef ENABLE_NL
    {"wis", 1, 1, 1, PREFIX_PRIORITY, lerase},
#endif
    {"erasefile", 1, 1, 1, PREFIX_PRIORITY, lerasefile},
#ifdef ENABLE_NL
    {"wb", 1, 1, 1, PREFIX_PRIORITY, lerasefile},
    {"wisbestand", 1, 1, 1, PREFIX_PRIORITY, lerasefile},
#endif
    {"erf", 1, 1, 1, PREFIX_PRIORITY, lerasefile},
    {"erns", 0, 0, 0, PREFIX_PRIORITY, lerns},
    {"erpls", 0, 0, 0, PREFIX_PRIORITY, lerpls},
    {"erps", 0, 0, 0, PREFIX_PRIORITY, lerps},
    {"error", 0, 0, 0, PREFIX_PRIORITY, lerror},
#ifdef ENABLE_NL
    {"fout", 0, 0, 0, PREFIX_PRIORITY, lerror},
#endif
#ifdef OBJECTS
    {"exist", 0, 0, 0, PREFIX_PRIORITY, lexist},
#endif
    {"exp", 1, 1, 1, PREFIX_PRIORITY, lexp},
    {"fd", 1, 1, 1, PREFIX_PRIORITY, lforward},
#ifdef ENABLE_NL
    {"vooruit", 1, 1, 1, PREFIX_PRIORITY, lforward},
    {"vt", 1, 1, 1, PREFIX_PRIORITY, lforward},
#endif
    {"fence", 0, 0, 0, PREFIX_PRIORITY, lfence},
    {"fill", 0, 0, 0, PREFIX_PRIORITY, lfill},
#ifdef ENABLE_NL
    {"vul", 0, 0, 0, PREFIX_PRIORITY, lfill},
#endif
#ifdef HAVE_WX
    {"filled", 2, 2, 2, PREFIX_PRIORITY, lfilled},
#endif
    {"first", 1, 1, 1, PREFIX_PRIORITY, lfirst},
#ifdef ENABLE_NL
    {"eerste", 1, 1, 1, PREFIX_PRIORITY, lfirst},
#endif
    {"firsts", 1, 1, 1, PREFIX_PRIORITY, lfirsts},
#ifdef HAVE_WX
    {"font", 0, 0, 0, PREFIX_PRIORITY, lfont},
#endif
    {"forever", 1, 1, 1, MACRO_PRIORITY, lforever},
    {"form", 3, 3, 3, PREFIX_PRIORITY, lform},
    {"forward", 1, 1, 1, PREFIX_PRIORITY, lforward},
    {"fput", 2, 2, 2, PREFIX_PRIORITY, lfput},
#ifdef ENABLE_NL
    {"plaatservoor", 2, 2, 2, PREFIX_PRIORITY, lfput},
#endif
    {"fs", 0, 0, 0, PREFIX_PRIORITY, lfullscreen},
    {"fullscreen", 0, 0, 0, PREFIX_PRIORITY, lfullscreen},
    {"fulltext", 1, 1, 1, PREFIX_PRIORITY, lfulltext},
    {"gc", 0, 0, 1, PREFIX_PRIORITY, lgc},
    {"global", 1, 1, -1, PREFIX_PRIORITY, lglobal},
    {"goto", 1, 1, 1, MACRO_PRIORITY, lgoto},
    {"gprop", 2, 2, 2, PREFIX_PRIORITY, lgprop},
#ifdef ENABLE_NL
    {"neemkenmerk", 2, 2, 2, PREFIX_PRIORITY, lgprop},
#endif
    {"greaterp", 2, 2, 2, PREFIX_PRIORITY, lgreaterp},
#ifdef ENABLE_NL
    {"groter?", 2, 2, 2, PREFIX_PRIORITY, lgreaterp},
#endif
    {"greater?", 2, 2, 2, PREFIX_PRIORITY, lgreaterp},
    {"greaterequalp", 2, 2, 2, PREFIX_PRIORITY, lgreaterequalp},
    {"greaterequal?", 2, 2, 2, PREFIX_PRIORITY, lgreaterequalp},
#ifdef OBJECTS
    {"have", 1, 1, 1, PREFIX_PRIORITY, lhave},
#endif
    {"heading", 0, 0, 0, PREFIX_PRIORITY, lheading},
#ifdef ENABLE_NL
    {"graden", 0, 0, 0, PREFIX_PRIORITY, lheading},
#endif
    {"help", OK_NO_ARG, 1, 1, PREFIX_PRIORITY, lhelp},
    {"hideturtle", 0, 0, 0, PREFIX_PRIORITY, lhideturtle},
#ifdef ENABLE_NL
    {"wegturtle", 0, 0, 0, PREFIX_PRIORITY, lhideturtle},
    {"wegt", 0, 0, 0, PREFIX_PRIORITY, lhideturtle},
#endif
    {"home", 0, 0, 0, PREFIX_PRIORITY, lhome},
#ifdef ENABLE_NL
    {"naarbegin", 0, 0, 0, PREFIX_PRIORITY, lhome},
    {"nb", 0, 0, 0, PREFIX_PRIORITY, lhome},
#endif
    {"ht", 0, 0, 0, PREFIX_PRIORITY, lhideturtle},
    {"if", 2, 2, 3, MACRO_PRIORITY, lif},
#ifdef ENABLE_NL
    {"als", 2, 2, 3, MACRO_PRIORITY, lif},
#endif
    {"ifelse", 3, 3, 3, MACRO_PRIORITY, lifelse},
#ifdef ENABLE_NL
    {"alsanders", 3, 3, 3, MACRO_PRIORITY, lifelse},
#endif
    {"iff", 1, 1, 1, MACRO_PRIORITY, liffalse},
    {"iffalse", 1, 1, 1, MACRO_PRIORITY, liffalse},
#ifdef ENABLE_NL
    {"alsnietwaar", 1, 1, 1, MACRO_PRIORITY, liffalse},
#endif
    {"ift", 1, 1, 1, MACRO_PRIORITY, liftrue},
    {"iftrue", 1, 1, 1, MACRO_PRIORITY, liftrue},
#ifdef ENABLE_NL
    {"alswaar", 1, 1, 1, MACRO_PRIORITY, liftrue},
#endif
#ifdef HAVE_WX
    {"increasefont", 0, 0, 0, PREFIX_PRIORITY, IncreaseFont},
#endif
    {"int", 1, 1, 1, PREFIX_PRIORITY, linteg},
#ifdef ENABLE_NL
    {"integer", 1, 1, 1, PREFIX_PRIORITY, linteg},
#endif
    {"item", 2, 2, 2, PREFIX_PRIORITY, litem},
#ifdef ENABLE_NL
    {"element", 2, 2, 2, PREFIX_PRIORITY, litem},
#endif
    {"keyp", 0, 0, 0, PREFIX_PRIORITY, lkeyp},
    {"key?", 0, 0, 0, PREFIX_PRIORITY, lkeyp},
#ifdef ENABLE_NL
    {"toets?", 0, 0, 0, PREFIX_PRIORITY, lkeyp},
#endif
#ifdef OBJECTS
    {"kindof", 1, 1, -1, PREFIX_PRIORITY, lkindof},
#endif
    {"label", 1, 1, 1, PREFIX_PRIORITY, llabel},
#ifdef HAVE_WX
    {"labelsize", 0, 0, 0, PREFIX_PRIORITY, llabelsize},	
#endif
    {"last", 1, 1, 1, PREFIX_PRIORITY, llast},
#ifdef ENABLE_NL
    {"laatste", 1, 1, 1, PREFIX_PRIORITY, llast},
#endif
    {"left", 1, 1, 1, PREFIX_PRIORITY, lleft},
#ifdef ENABLE_NL
    {"links", 1, 1, 1, PREFIX_PRIORITY, lleft},
    {"li", 1, 1, 1, PREFIX_PRIORITY, lleft},
#endif
    {"lessp", 2, 2, 2, PREFIX_PRIORITY, llessp},
#ifdef ENABLE_NL
    {"kleiner?", 2, 2, 2, PREFIX_PRIORITY, llessp},
#endif
    {"less?", 2, 2, 2, PREFIX_PRIORITY, llessp},
    {"lessequalp", 2, 2, 2, PREFIX_PRIORITY, llessequalp},
    {"lessequal?", 2, 2, 2, PREFIX_PRIORITY, llessequalp},
    {"linep", 0, 0, 0, PREFIX_PRIORITY, llinep},
    {"line?", 0, 0, 0, PREFIX_PRIORITY, llinep},
    {"list", 0, 2, -1, PREFIX_PRIORITY, llist},
#ifdef ENABLE_NL
    {"lijst", 0, 2, -1, PREFIX_PRIORITY, llist},
#endif
    {"listp", 1, 1, 1, PREFIX_PRIORITY, llistp},
#ifdef ENABLE_NL
    {"lijst?", 1, 1, 1, PREFIX_PRIORITY, llistp},
#endif
    {"listtoarray", 1, 1, 2, PREFIX_PRIORITY, llisttoarray},
    {"list?", 1, 1, 1, PREFIX_PRIORITY, llistp},
    {"ln", 1, 1, 1, PREFIX_PRIORITY, lln},
    {"load", 1, 1, 1, PREFIX_PRIORITY, lload},
#ifdef ENABLE_NL
    {"open", 1, 1, 1, PREFIX_PRIORITY, lload},
    {"laad", 1, 1, 1, PREFIX_PRIORITY, lload},
#endif
    {"loadpict", 1, 1, 1, PREFIX_PRIORITY, lloadpict},
    {"local", 1, 1, -1, PREFIX_PRIORITY, llocal},
#ifdef ENABLE_NL
    {"lokaal", 1, 1, -1, PREFIX_PRIORITY, llocal},
#endif
    {"log10", 1, 1, 1, PREFIX_PRIORITY, llog10},
#ifdef OBJECTS
    {"logo", 0, 0, 0, PREFIX_PRIORITY, llogo},
#endif
    {"lowercase", 1, 1, 1, PREFIX_PRIORITY, llowercase},
    {"lput", 2, 2, 2, PREFIX_PRIORITY, llput},
#ifdef ENABLE_NL
    {"plaatserachter", 2, 2, 2, PREFIX_PRIORITY, llput},
#endif
    {"lshift", 2, 2, 2, PREFIX_PRIORITY, llshift},
    {"lt", 1, 1, 1, PREFIX_PRIORITY, lleft},
    {"macrop", 1, 1, 1, PREFIX_PRIORITY, lmacrop},
    {"macro?", 1, 1, 1, PREFIX_PRIORITY, lmacrop},
    {"make", 2, 2, 2, PREFIX_PRIORITY, lmake},
#ifdef ENABLE_NL
    {"maak", 2, 2, 2, PREFIX_PRIORITY, lmake},
#endif
#ifndef HAVE_WX
#if defined(WIN32)
    {"maximize.window", 1, 1, 1, PREFIX_PRIORITY, maximize},
#endif
#endif
    {"member", 2, 2, 2, PREFIX_PRIORITY, lmember},
    {"memberp", 2, 2, 2, PREFIX_PRIORITY, lmemberp},
#ifdef ENABLE_NL
    {"erbij?", 2, 2, 2, PREFIX_PRIORITY, lmemberp},
#endif
    {"member?", 2, 2, 2, PREFIX_PRIORITY, lmemberp},
    {"minus", 1, 1, 1, PREFIX_PRIORITY, lsub},
    {"modulo", 2, 2, 2, PREFIX_PRIORITY, lmodulo},
#ifdef ENABLE_NL
    {"mod", 2, 2, 2, PREFIX_PRIORITY, lmodulo},
#endif
    {"mousepos", 0, 0, 0, PREFIX_PRIORITY, lmousepos},
#ifdef OBJECTS
    {"mynamep", 1, 1, 1, PREFIX_PRIORITY, lmynamep},
    {"mynames", 0, 0, 0, PREFIX_PRIORITY, lmynames},
    {"myprocp", 1, 1, 1, PREFIX_PRIORITY, lmyprocp},
    {"myprocs", 0, 0, 0, PREFIX_PRIORITY, lmyprocs},
#endif
    {"namep", 1, 1, 1, PREFIX_PRIORITY, lnamep},
    {"name?", 1, 1, 1, PREFIX_PRIORITY, lnamep},
#ifdef ENABLE_NL
    {"naam?", 1, 1, 1, PREFIX_PRIORITY, lnamep},
#endif
    {"names", 0, 0, 0, PREFIX_PRIORITY, lnames},
#ifdef ENABLE_NL
    {"namen", 0, 0, 0, PREFIX_PRIORITY, lnames},
#endif
    {"nodes", 0, 0, 0, PREFIX_PRIORITY, lnodes},
    {"nodribble", 0, 0, 0, PREFIX_PRIORITY, lnodribble},
    {"norefresh", 0, 0, 0, PREFIX_PRIORITY, lnorefresh},
    {"not", 1, 1, 1, PREFIX_PRIORITY, lnot},
#ifdef ENABLE_NL
    {"niet", 1, 1, 1, PREFIX_PRIORITY, lnot},
#endif
    {"notequalp", 2, 2, 2, PREFIX_PRIORITY, lnotequalp},
    {"notequal?", 2, 2, 2, PREFIX_PRIORITY, lnotequalp},
    {"numberp", 1, 1, 1, PREFIX_PRIORITY, lnumberp},
#ifdef ENABLE_NL
    {"getal?", 1, 1, 1, PREFIX_PRIORITY, lnumberp},
#endif
    {"number?", 1, 1, 1, PREFIX_PRIORITY, lnumberp},
#ifdef OBJECTS
    {"oneof", 1, 1, -1, MACRO_PRIORITY, loneof},
#endif
    {"op", 1, 1, 1, OUTPUT_PRIORITY, loutput},
    {"openappend", 1, 1, 1, PREFIX_PRIORITY, lopenappend},
#ifdef ENABLE_NL
    {"opentoevoeg", 1, 1, 1, PREFIX_PRIORITY, lopenappend},
#endif
    {"openread", 1, 1, 1, PREFIX_PRIORITY, lopenread},
#ifdef ENABLE_NL
    {"openlees", 1, 1, 1, PREFIX_PRIORITY, lopenread},
#endif
    {"openupdate", 1, 1, 1, PREFIX_PRIORITY, lopenupdate},
#ifdef ENABLE_NL
    {"openbijwerk", 1, 1, 1, PREFIX_PRIORITY, lopenupdate},
#endif
    {"openwrite", 1, 1, 1, PREFIX_PRIORITY, lopenwrite},
#ifdef ENABLE_NL
    {"openschrijf", 1, 1, 1, PREFIX_PRIORITY, lopenwrite},
#endif
    {"or", 0, 2, -1, PREFIX_PRIORITY, lor},
#ifdef ENABLE_NL
    {"of", 0, 2, -1, PREFIX_PRIORITY, lor},
#endif
    {"output", 1, 1, 1, OUTPUT_PRIORITY, loutput},
#ifdef ENABLE_NL
    {"uitvoer", 1, 1, 1, OUTPUT_PRIORITY, loutput},
    {"uv", 1, 1, 1, OUTPUT_PRIORITY, loutput},
#endif
    {"palette", 1, 1, 1, PREFIX_PRIORITY, lpalette},
#ifdef OBJECTS
    {"parents", 0, 0, 0, PREFIX_PRIORITY, lparents},
#endif
    {"parse", 1, 1, 1, PREFIX_PRIORITY, lparse},
    {"pause", 0, 0, 0, PREFIX_PRIORITY, lpause},	       
#ifdef ENABLE_NL
    {"pauze", 0, 0, 0, PREFIX_PRIORITY, lpause},
#endif
    {"pc", 0, 0, 0, PREFIX_PRIORITY, lpencolor},
    {"pd", 0, 0, 0, PREFIX_PRIORITY, lpendown},
    {"pe", 0, 0, 0, PREFIX_PRIORITY, lpenerase},
    {"pencolor", 0, 0, 0, PREFIX_PRIORITY, lpencolor},
#ifdef ENABLE_NL
    {"pk", 0, 0, 0, PREFIX_PRIORITY, lpencolor},
    {"penkleur", 0, 0, 0, PREFIX_PRIORITY, lpencolor},
#endif
    {"pendown", 0, 0, 0, PREFIX_PRIORITY, lpendown},
#ifdef ENABLE_NL
    {"penneer", 0, 0, 0, PREFIX_PRIORITY, lpendown},
    {"pn", 0, 0, 0, PREFIX_PRIORITY, lpendown},
#endif
    {"pendownp", 0, 0, 0, PREFIX_PRIORITY, lpendownp},
    {"pendown?", 0, 0, 0, PREFIX_PRIORITY, lpendownp},
    {"penerase", 0, 0, 0, PREFIX_PRIORITY, lpenerase},
#ifdef ENABLE_NL
    {"gum", 0, 0, 0, PREFIX_PRIORITY, lpenerase},
#endif
    {"penmode", 0, 0, 0, PREFIX_PRIORITY, lpenmode},
    {"penpaint", 0, 0, 0, PREFIX_PRIORITY, lpenpaint},
    {"penpattern", 0, 0, 0, PREFIX_PRIORITY, lpenpattern},
    {"penreverse", 0, 0, 0, PREFIX_PRIORITY, lpenreverse},
#ifdef ENABLE_NL
    {"penomgekeerd", 0, 0, 0, PREFIX_PRIORITY, lpenreverse},
#endif
    {"pensize", 0, 0, 0, PREFIX_PRIORITY, lpensize},
    {"penup", 0, 0, 0, PREFIX_PRIORITY, lpenup},
#ifdef ENABLE_NL
    {"penop", 0, 0, 0, PREFIX_PRIORITY, lpenup},
    {"pp", 0, 0, 0, PREFIX_PRIORITY, lpenup},
#endif
    {"plist", 1, 1, 1, PREFIX_PRIORITY, lplist},
#ifdef ENABLE_NL
    {"kenmerklijst", 1, 1, 1, PREFIX_PRIORITY, lplist},
#endif
    {"plistp", 1, 1, 1, PREFIX_PRIORITY, lplistp},
    {"plist?", 1, 1, 1, PREFIX_PRIORITY, lplistp},
    {"plists", 0, 0, 0, PREFIX_PRIORITY, lplists},
    {"po", 1, 1, 1, PREFIX_PRIORITY, lpo},
#ifdef ENABLE_NL
    {"printdefinitie", 1, 1, 1, PREFIX_PRIORITY, lpo},
#endif
    {"pos", 0, 0, 0, PREFIX_PRIORITY, lpos},
#ifdef ENABLE_NL
    {"positie", 0, 0, 0, PREFIX_PRIORITY, lpos},
    {"xcor", 0, 0, 0, PREFIX_PRIORITY, lxcor},
    {"xcoordinaat", 0, 0, 0, PREFIX_PRIORITY, lxcor},
    {"ycor", 0, 0, 0, PREFIX_PRIORITY, lycor},
    {"ycoordinaat", 0, 0, 0, PREFIX_PRIORITY, lycor},
#endif
    {"pot", 1, 1, 1, PREFIX_PRIORITY, lpot},
    {"power", 2, 2, 2, PREFIX_PRIORITY, lpower},
#ifdef ENABLE_NL
    {"macht", 2, 2, 2, PREFIX_PRIORITY, lpower},
#endif
    {"pprop", 3, 3, 3, PREFIX_PRIORITY, lpprop},
#ifdef ENABLE_NL
    {"plaatskenmerk", 3, 3, 3, PREFIX_PRIORITY, lpprop},
#endif
    {"ppt", 0, 0, 0, PREFIX_PRIORITY, lpenpaint},
    {"pr", 0, 1, -1, PREFIX_PRIORITY, lprint},
    {"prefix", 0, 0, 0, PREFIX_PRIORITY, lprefix},
    {"primitivep", 1, 1, 1, PREFIX_PRIORITY, lprimitivep},
    {"primitive?", 1, 1, 1, PREFIX_PRIORITY, lprimitivep},
#ifdef ENABLE_NL
    {"primitief?", 1, 1, 1, PREFIX_PRIORITY, lprimitivep},
#endif
    {"primitives", 0, 0, 0, PREFIX_PRIORITY, lprimitives},
    {"print", 0, 1, -1, PREFIX_PRIORITY, lprint},
#ifdef ENABLE_NL
    {"printperregel", 0, 1, -1, PREFIX_PRIORITY, lprint},
#endif
    {"printout", 1, 1, 1, PREFIX_PRIORITY, lpo},
#ifdef HAVE_WX
    {"printpict", 0, 0, 1, PREFIX_PRIORITY, lprintpict},
    {"printtext", 0, 0, 1, PREFIX_PRIORITY, lprinttext},
#endif
    {"procedurep", 1, 1, 1, PREFIX_PRIORITY, lprocedurep},
    {"procedure?", 1, 1, 1, PREFIX_PRIORITY, lprocedurep},
    {"procedures", 0, 0, 0, PREFIX_PRIORITY, lprocedures},
    {"product", 0, 2, -1, PREFIX_PRIORITY, lmul},
#ifdef ENABLE_NL
    {"vermenigvuldig", 0, 2, -1, PREFIX_PRIORITY, lmul},
#endif
    {"pu", 0, 0, 0, PREFIX_PRIORITY, lpenup},
    {"px", 0, 0, 0, PREFIX_PRIORITY, lpenreverse},
    {"quotient", 1, 2, 2, PREFIX_PRIORITY, ldivide},
#ifdef ENABLE_NL
    {"deeldoor", 1, 2, 2, PREFIX_PRIORITY, ldivide},
#endif
    {"radarctan", 1, 1, 2, PREFIX_PRIORITY, lradatan},
    {"radcos", 1, 1, 1, PREFIX_PRIORITY, lradcos},
    {"radsin", 1, 1, 1, PREFIX_PRIORITY, lradsin},
    {"random", 1, 1, 2, PREFIX_PRIORITY, lrandom},
#ifdef ENABLE_NL
    {"gok", 1, 1, 2, PREFIX_PRIORITY, lrandom},
#endif
    {"rawascii", 1, 1, 1, PREFIX_PRIORITY, lrawascii},
    {"rc", 0, 0, 0, PREFIX_PRIORITY, lreadchar},
    {"rcs", 1, 1, 1, PREFIX_PRIORITY, lreadchars},
    {"readchar", 0, 0, 0, PREFIX_PRIORITY, lreadchar},
#ifdef ENABLE_NL
    {"leesteken", 0, 0, 0, PREFIX_PRIORITY, lreadchar},
#endif
    {"readchars", 1, 1, 1, PREFIX_PRIORITY, lreadchars},
    {"reader", 0, 0, 0, PREFIX_PRIORITY, lreader},
    {"readlist", 0, 0, 0, PREFIX_PRIORITY, lreadlist},
#ifdef ENABLE_NL
    {"leeslijst", 0, 0, 0, PREFIX_PRIORITY, lreadlist},
    {"ll", 0, 0, 0, PREFIX_PRIORITY, lreadlist},
#endif
    {"readpos", 0, 0, 0, PREFIX_PRIORITY, lreadpos},
    {"readrawline", 0, 0, 0, PREFIX_PRIORITY, lreadrawline},
    {"readword", 0, 0, 0, PREFIX_PRIORITY, lreadword},
#ifdef ENABLE_NL
    {"leeswoord", 0, 0, 0, PREFIX_PRIORITY, lreadword},
    {"lw", 0, 0, 0, PREFIX_PRIORITY, lreadword},
#endif
    {"refresh", 0, 0, 0, PREFIX_PRIORITY, lrefresh},
    {"remainder", 2, 2, 2, PREFIX_PRIORITY, lremainder},
    {"remprop", 2, 2, 2, PREFIX_PRIORITY, lremprop},
#ifdef ENABLE_NL
    {"wiskenmerk", 2, 2, 2, PREFIX_PRIORITY, lremprop},
#endif
    {"repcount", 0, 0, 0, PREFIX_PRIORITY, lrepcount},
#ifdef ENABLE_NL
    {"telherhaal", 0, 0, 0, PREFIX_PRIORITY, lrepcount},
#endif
    {"repeat", 2, 2, 2, MACRO_PRIORITY, lrepeat},
#ifdef ENABLE_NL
    {"herhaal", 2, 2, 2, MACRO_PRIORITY, lrepeat},
#endif
#ifdef OBJECTS
    {"representation", 0, 0, 0, PREFIX_PRIORITY, lrepresentation},
#endif
    {"rerandom", 0, 0, 1, PREFIX_PRIORITY, lrerandom},
    {"right", 1, 1, 1, PREFIX_PRIORITY, lright},
#ifdef ENABLE_NL
    {"rechts", 1, 1, 1, PREFIX_PRIORITY, lright},
    {"re", 1, 1, 1, PREFIX_PRIORITY, lright},
#endif
    {"rl", 0, 0, 0, PREFIX_PRIORITY, lreadlist},
    {"round", 1, 1, 1, PREFIX_PRIORITY, lroundx},
#ifdef ENABLE_NL
    {"afronding", 1, 1, 1, PREFIX_PRIORITY, lroundx},
#endif
    {"rt", 1, 1, 1, PREFIX_PRIORITY, lright},
    {"run", 1, 1, 1, MACRO_PRIORITY, lrun},
#ifdef ENABLE_NL
    {"doe", 1, 1, 1, MACRO_PRIORITY, lrun},
#endif
    {"runparse", 1, 1, 1, PREFIX_PRIORITY, lrunparse},
    {"runresult", 1, 1, 1, MACRO_PRIORITY, lrunresult},
    {"rw", 0, 0, 0, PREFIX_PRIORITY, lreadword},
    {"save", OK_NO_ARG, 1, 1, PREFIX_PRIORITY, lsave},
#ifdef ENABLE_NL
    {"opslaan", OK_NO_ARG, 1, 1, PREFIX_PRIORITY, lsave},
#endif
    {"savepict", 1, 1, 1, PREFIX_PRIORITY, lsavepict},
    {"screenmode", 0, 0, 0, PREFIX_PRIORITY, lscreenmode},
    {"scrunch", 0, 0, 0, PREFIX_PRIORITY, lscrunch},
    {"se", 0, 2, -1, PREFIX_PRIORITY, lsentence},
#ifdef OBJECTS
    {"self", 0, 0, 0, PREFIX_PRIORITY, lself},
#endif
    {"sentence", 0, 2, -1, PREFIX_PRIORITY, lsentence},
#ifdef ENABLE_NL
    {"samen", 0, 2, -1, PREFIX_PRIORITY, lsentence},
    {"sa", 0, 2, -1, PREFIX_PRIORITY, lsentence},
#endif
    {"setbg", 1, 1, 1, PREFIX_PRIORITY, lsetbackground},
    {"setbackground", 1, 1, 1, PREFIX_PRIORITY, lsetbackground},
#ifdef ENABLE_NL
    {"zag", 1, 1, 1, PREFIX_PRIORITY, lsetbackground},
    {"zetachtergrond", 1, 1, 1, PREFIX_PRIORITY, lsetbackground},
#endif
    {"setcursor", 1, 1, 1, PREFIX_PRIORITY, lsetcursor},
#ifdef ENABLE_NL
    {"zetcursor", 1, 1, 1, PREFIX_PRIORITY, lsetcursor},
#endif
    {"setcslsloc", 1, 1, 1, PREFIX_PRIORITY, lsetcslsloc},
#ifdef HAVE_WX
    {"setfont", 1, 1, 1, PREFIX_PRIORITY, lsetfont},
#endif
    {"seteditor", 1, 1, 1, PREFIX_PRIORITY, lseteditor},
    {"seth", 1, 1, 1, PREFIX_PRIORITY, lsetheading},
    {"setheading", 1, 1, 1, PREFIX_PRIORITY, lsetheading},
#ifdef ENABLE_NL
    {"zetrichting", 1, 1, 1, PREFIX_PRIORITY, lsetheading},
    {"zr", 1, 1, 1, PREFIX_PRIORITY, lsetheading},
#endif
    {"sethelploc", 1, 1, 1, PREFIX_PRIORITY, lsethelploc},
    {"setitem", 3, 3, 3, PREFIX_PRIORITY, lsetitem},
#ifdef HAVE_WX
    {"setlabelheight", 1, 1, 1, PREFIX_PRIORITY, lsetlabelheight},
#endif
    {"setlibloc", 1, 1, 1, PREFIX_PRIORITY, lsetlibloc},
    {"setmargins", 1, 1, 1, PREFIX_PRIORITY, lsetmargins},
    {"setpalette", 2, 2, 2, PREFIX_PRIORITY, lsetpalette},
    {"setpc", 1, 1, 1, PREFIX_PRIORITY, lsetpencolor},
    {"setpencolor", 1, 1, 1, PREFIX_PRIORITY, lsetpencolor},
#ifdef ENABLE_NL
    {"zetpenkleur", 1, 1, 1, PREFIX_PRIORITY, lsetpencolor},
#endif
    {"setpenpattern", 1, 1, 1, PREFIX_PRIORITY, lsetpenpattern},
    {"setpensize", 1, 1, 1, PREFIX_PRIORITY, lsetpensize},
#ifdef ENABLE_NL
    {"zetpendikte", 1, 1, 1, PREFIX_PRIORITY, lsetpensize},
#endif
    {"setpos", 1, 1, 1, PREFIX_PRIORITY, lsetpos},
#ifdef ENABLE_NL
    {"zetpos", 1, 1, 1, PREFIX_PRIORITY, lsetpos},
#endif
    {"setprefix", 1, 1, 1, PREFIX_PRIORITY, lsetprefix},
    {"setread", 1, 1, 1, PREFIX_PRIORITY, lsetread},
#ifdef ENABLE_NL
    {"zetlees", 1, 1, 1, PREFIX_PRIORITY, lsetread},
#endif
    {"setreadpos", 1, 1, 1, PREFIX_PRIORITY, lsetreadpos},
    {"setscrunch", 2, 2, 2, PREFIX_PRIORITY, lsetscrunch},
    {"setshape", 1, 1, 1, PREFIX_PRIORITY, lsetshape},
#if defined(WIN32)|defined(HAVE_WX)
    {"settc", 2, 2, 2, PREFIX_PRIORITY, set_text_color},
#ifdef ENABLE_NL
    {"zetkleurov", 2, 2, 2, PREFIX_PRIORITY, set_text_color},
#endif
    {"settextcolor", 2, 2, 2, PREFIX_PRIORITY, set_text_color},
#endif
#ifdef HAVE_WX
    {"settextsize", 1, 1, 1, PREFIX_PRIORITY, lsettextsize},
#endif
    {"settemploc", 1, 1, 1, PREFIX_PRIORITY, lsettemploc},
    {"setwrite", 1, 1, 1, PREFIX_PRIORITY, lsetwrite},
#ifdef ENABLE_NL
    {"zetschrijf", 1, 1, 1, PREFIX_PRIORITY, lsetwrite},
#endif
    {"setwritepos", 1, 1, 1, PREFIX_PRIORITY, lsetwritepos},
    {"setx", 1, 1, 1, PREFIX_PRIORITY, lsetx},
#ifdef ENABLE_NL
    {"zetx", 1, 1, 1, PREFIX_PRIORITY, lsetx},
#endif
    {"setxy", 2, 2, 2, PREFIX_PRIORITY, lsetxy},
#ifdef ENABLE_NL
    {"zetxy", 2, 2, 2, PREFIX_PRIORITY, lsetxy},
#endif
    {"sety", 1, 1, 1, PREFIX_PRIORITY, lsety},
#ifdef ENABLE_NL
    {"zety", 1, 1, 1, PREFIX_PRIORITY, lsety},
#endif
    {"shape", 0, 0, 0, PREFIX_PRIORITY, lshape},
    {"shell", 1, 1, 2, PREFIX_PRIORITY, lshell},
    {"show", 0, 1, -1, PREFIX_PRIORITY, lshow},
    {"shownp", 0, 0, 0, PREFIX_PRIORITY, lshownp},
    {"shown?", 0, 0, 0, PREFIX_PRIORITY, lshownp},
#ifdef ENABLE_NL
    {"zichtbaar?", 0, 0, 0, PREFIX_PRIORITY, lshownp},
#endif
    {"showturtle", 0, 0, 0, PREFIX_PRIORITY, lshowturtle},
#ifdef ENABLE_NL
    {"kom", 0, 0, 0, PREFIX_PRIORITY, lshowturtle},
    {"laatzien", 0, 1, -1, PREFIX_PRIORITY, lshow},
    {"lz", 0, 1, -1, PREFIX_PRIORITY, lshow},
#endif
    {"sin", 1, 1, 1, PREFIX_PRIORITY, lsin},
#ifdef ENABLE_NL
    {"sinus", 1, 1, 1, PREFIX_PRIORITY, lsin},
#endif
#ifdef OBJECTS
    {"something", 0, 0, 0, PREFIX_PRIORITY, lsomething},
#endif
    {"splitscreen", 0, 0, 0, PREFIX_PRIORITY, lsplitscreen},
#ifdef ENABLE_NL
    {"splitsvenster", 0, 0, 0, PREFIX_PRIORITY, lsplitscreen},
#endif
    {"sqrt", 1, 1, 1, PREFIX_PRIORITY, lsqrt},
#ifdef ENABLE_NL
    {"wortel", 1, 1, 1, PREFIX_PRIORITY, lsqrt},
#endif
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
#ifdef ENABLE_NL
    {"telop", 0, 2, -1, PREFIX_PRIORITY, ladd},
#endif
    {"tag", 1, 1, 1, PREFIX_PRIORITY, ltag},
#ifdef OBJECTS
    {"talkto", 1, 1, 1, PREFIX_PRIORITY, ltalkto},
#endif
    {"test", 1, 1, 1, PREFIX_PRIORITY, ltest},
    {"text", 1, 1, 1, PREFIX_PRIORITY, ltext},
#ifdef ENABLE_NL
    {"printtekst", 1, 1, 1, PREFIX_PRIORITY, ltext},
#endif
    {"textscreen", 0, 0, 0, PREFIX_PRIORITY, ltextscreen},
#ifdef ENABLE_NL
    {"opdrachtvenster", 0, 0, 0, PREFIX_PRIORITY, ltextscreen},
    {"ov", 0, 0, 0, PREFIX_PRIORITY, ltextscreen},
#endif
#ifdef HAVE_WX
    {"textsize", 0, 0, 0, PREFIX_PRIORITY, ltextsize},
#endif
    {"thing", 1, 1, 1, PREFIX_PRIORITY, lthing},
#ifdef ENABLE_NL
    {"ding", 1, 1, 1, PREFIX_PRIORITY, lthing},
#endif
    {"throw", 1, 1, 2, PREFIX_PRIORITY, lthrow},
#ifdef ENABLE_NL
    {"gooi", 1, 1, 2, PREFIX_PRIORITY, lthrow},
#endif
    {"time", 0, 0, 0, PREFIX_PRIORITY, ltime},
    {"to", -1, -1, -1, PREFIX_PRIORITY, lto},
#ifdef ENABLE_NL
    {"leer", -1, -1, -1, PREFIX_PRIORITY, lto},
#endif
    {"tone", 2, 2, 2, PREFIX_PRIORITY, ltone},
    {"towards", 1, 1, 1, PREFIX_PRIORITY, ltowards},
#ifdef ENABLE_NL
    {"naar", 1, 1, 1, PREFIX_PRIORITY, ltowards},
#endif
    {"trace", 1, 1, 1, PREFIX_PRIORITY, ltrace},
    {"traced", 0, 0, 0, PREFIX_PRIORITY, ltraced},
    {"tracedp", 1, 1, 1, PREFIX_PRIORITY, ltracedp},
    {"traced?", 1, 1, 1, PREFIX_PRIORITY, ltracedp},
    {"ts", 0, 0, 0, PREFIX_PRIORITY, ltextscreen},
    {"turtlemode", 0, 0, 0, PREFIX_PRIORITY, lturtlemode},
    {"type", 0, 1, -1, PREFIX_PRIORITY, ltype},
#ifdef ENABLE_NL
    {"printnaarov", 0, 1, -1, PREFIX_PRIORITY, ltype},
#endif
    {"unbury", 1, 1, 1, PREFIX_PRIORITY, lunbury},
#ifdef ENABLE_NL
    {"verbergniet", 1, 1, 1, PREFIX_PRIORITY, lunbury},
#endif
    {"unstep", 1, 1, 1, PREFIX_PRIORITY, lunstep},
    {"untrace", 1, 1, 1, PREFIX_PRIORITY, luntrace},
    {"uppercase", 1, 1, 1, PREFIX_PRIORITY, luppercase},
    {"vbarredp", 1, 1, 1, PREFIX_PRIORITY, lvbarredp},
    {"vbarred?", 1, 1, 1, PREFIX_PRIORITY, lvbarredp},
    {"wait", 1, 1, 1, PREFIX_PRIORITY, lwait},
#ifdef ENABLE_NL
    {"wacht", 1, 1, 1, PREFIX_PRIORITY, lwait},
#endif
    {"window", 0, 0, 0, PREFIX_PRIORITY, lwindow},
#ifdef ENABLE_NL
    {"venster", 0, 0, 0, PREFIX_PRIORITY, lwindow},
#endif
    {"word", 0, 2, -1, PREFIX_PRIORITY, lword},
#ifdef ENABLE_NL
    {"woord", 0, 2, -1, PREFIX_PRIORITY, lword},
#endif
    {"wordp", 1, 1, 1, PREFIX_PRIORITY, lwordp},
#ifdef ENABLE_NL
    {"woord?", 1, 1, 1, PREFIX_PRIORITY, lwordp},
#endif
    {"word?", 1, 1, 1, PREFIX_PRIORITY, lwordp},
    {"wrap", 0, 0, 0, PREFIX_PRIORITY, lwrap},
#ifdef ENABLE_NL
    {"rand", 0, 0, 0, PREFIX_PRIORITY, lwrap},
#endif
    {"writepos", 0, 0, 0, PREFIX_PRIORITY, lwritepos},
    {"writer", 0, 0, 0, PREFIX_PRIORITY, lwriter},

    {0, 0, 0, 0, 0, 0}
};

struct wdtrans translations[NUM_WORDS];

NODE *intern_p(NODE *caseobj) {
    NODE *result = intern(caseobj);
    setflag__caseobj(result, PERMANENT);
    return result;
}

void init(void) {
    int i = 0;
    NODE *iproc = NIL, *pname = NIL, *cnd = NIL;
    FILE *fp;
    char linebuf[256];
    char *sugar;
    static char sugarlib[100], sugarhelp[100], sugarcsls[100];
#ifdef WIN32
    HKEY regKey1, regKey2, regKey3;
    int got_hklm = 0;
    char buf[200];
    unsigned long int bufsiz;
    char *envp;
#endif

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

    sugar = getenv("SUGAR_BUNDLE_PATH");
    if (sugar != NULL) {
	strcpy(sugarlib,sugar);
	strcat(sugarlib,"/logolib");
	logolib = sugarlib;
	fp = fopen(logolib,"r");
	if (fp == NULL) goto nosugar;
	fclose(fp);
	chdir(getenv("SUGAR_ACTIVITY_ROOT"));
	chdir("data");
	strcpy(sugarhelp,sugar);
	strcat(sugarhelp,"/helpfiles");
	helpfiles = sugarhelp;
	strcpy(sugarcsls,sugar);
	strcat(sugarcsls,"/csls");
	csls = sugarcsls;
    } else {
nosugar:
	logolib = getenv("LOGOLIB");
	helpfiles = getenv("LOGOHELP");
	csls = getenv("CSLS");
    }
#ifndef HAVE_WX
    editor = getenv("EDITOR");
#endif /* HAVE_WX */
#ifdef WIN32
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software", 0,
		     KEY_READ, &regKey1) == ERROR_SUCCESS) {
	if (RegOpenKeyEx(regKey1, "UCB", 0, 
			 KEY_READ, &regKey2) == ERROR_SUCCESS) {
	    if (RegOpenKeyEx(regKey2, "UCBLogo", 0,
			     KEY_READ, &regKey3) == ERROR_SUCCESS) {
		got_hklm = 1;
	/*	logolib = getenv("LOGOLIB"); */
		if (logolib == NULL) {
		    bufsiz=200;
		    if (RegQueryValueEx(regKey3, "LOGOLIB", NULL,
					NULL, buf, &bufsiz) == ERROR_SUCCESS) {
					    logolib = malloc(bufsiz);
					    strcpy(logolib, buf);
		    }
		}
	/*	helpfiles = getenv("LOGOHELP"); */
		if (helpfiles == NULL) {
		    bufsiz=200;
		    if (RegQueryValueEx(regKey3, "HELPFILE", NULL,
					NULL, buf, &bufsiz) == ERROR_SUCCESS) {
			helpfiles = malloc(bufsiz);
			strcpy(helpfiles, buf);
		    }
		}
	/*	csls = getenv("CSLS"); */
		if (csls == NULL) {
		    bufsiz=200;
		    if (RegQueryValueEx(regKey3, "CSLS", NULL,
					NULL, buf, &bufsiz) == ERROR_SUCCESS) {
			csls = malloc(bufsiz);
			strcpy(csls, buf);
		    }
		}
#ifndef HAVE_WX
	/*	editor = getenv("EDITOR"); */
		if (editor == NULL) {
		    bufsiz=200;
		    if (RegQueryValueEx(regKey3, "EDITOR", NULL,
					NULL, buf, &bufsiz) == ERROR_SUCCESS) {
			editor = malloc(bufsiz);
			strcpy(editor, buf);
		    }
		}
#endif /* HAVE_WX */
		RegCloseKey(regKey3);
	    }
	    RegCloseKey(regKey2);
	}
	RegCloseKey(regKey1);
    }
    if (!got_hklm && RegOpenKeyEx(HKEY_CURRENT_USER, "Software", 0,
				  KEY_READ, &regKey1) == ERROR_SUCCESS) {
	if (RegOpenKeyEx(regKey1, "UCB", 0, 
			 KEY_READ, &regKey2) == ERROR_SUCCESS) {
	    if (RegOpenKeyEx(regKey2, "UCBLogo", 0,
			     KEY_READ, &regKey3) == ERROR_SUCCESS) {
		got_hklm = 1;
	/*	logolib = getenv("LOGOLIB"); */
		if (logolib == NULL) {
		    bufsiz=200;
		    if (RegQueryValueEx(regKey3, "LOGOLIB", NULL,
					NULL, buf, &bufsiz) == ERROR_SUCCESS) {
					    logolib = malloc(bufsiz);
					    strcpy(logolib, buf);
		    }
		}
	/*	helpfiles = getenv("LOGOHELP"); */
		if (helpfiles == NULL) {
		    bufsiz=200;
		    if (RegQueryValueEx(regKey3, "HELPFILE", NULL,
					NULL, buf, &bufsiz) == ERROR_SUCCESS) {
			helpfiles = malloc(bufsiz);
			strcpy(helpfiles, buf);
		    }
		}
	/*	csls = getenv("CSLS"); */
		if (csls == NULL) {
		    bufsiz=200;
		    if (RegQueryValueEx(regKey3, "CSLS", NULL,
					NULL, buf, &bufsiz) == ERROR_SUCCESS) {
			csls = malloc(bufsiz);
			strcpy(csls, buf);
		    }
		}
#ifndef HAVE_WX
	/*	editor = getenv("EDITOR"); */
		if (editor == NULL) {
		    bufsiz=200;
		    if (RegQueryValueEx(regKey3, "EDITOR", NULL,
					NULL, buf, &bufsiz) == ERROR_SUCCESS) {
			editor = malloc(bufsiz);
			strcpy(editor, buf);
		    }
		}
#endif /* HAVE_WX */
		RegCloseKey(regKey3);
	    }
	    RegCloseKey(regKey2);
	}
	RegCloseKey(regKey1);
    }
#endif

    if (logolib == NULL) logolib = libloc;
    if (helpfiles == NULL) helpfiles = helploc;
    if (csls == NULL) csls = cslsloc;

#ifdef HAVE_WX
    //Check for Messages in logolib
    {
      FILE *message_fp;
      char executable_dir[256], message_fn[256], new_logolib[256];
      snprintf(message_fn, 256,"%s%sMessages", logolib, separator);
      message_fp = fopen(message_fn, "r");
      if (message_fp == NULL) {
        //Messages not in logolib, try relative to executable
        getExecutableDir(executable_dir, 256);
        snprintf(new_logolib, 256, "%s%slogolib", executable_dir, separator);
        snprintf(message_fn, 256, "%s%sMessages", new_logolib, separator);
        message_fp = fopen(message_fn, "r");
        if (message_fp != NULL) {
          printf("Changing logolib from %s to %s\n", logolib, new_logolib);
          logolib = malloc(sizeof(char)*(strlen(new_logolib)+1));
          strcpy(logolib, new_logolib);
        }
      }
      if (message_fp != NULL) {
        fclose(message_fp);
      }
    }
#ifndef __WXMSW__
	// have to do this because we don't have __WXMAC__ in C
	// if we might be running on a Mac, call C++ functions that will return
	// an appropriate path if __WXMAC__ is defined and 0 otherwise.

	const char* wxMacGetCslsloc();
	const char* wxMacGetHelploc();
	const char* wxMacGetLibloc();
	const char* newlib;
	const char* newcsls;
	const char* newhelp;

	newlib = wxMacGetLibloc();
	if (newlib) {
	  logolib = (char *)newlib;
	}

	newcsls = wxMacGetCslsloc();
	if (newcsls) {
	  csls = (char *)newcsls;
	}

	newhelp = wxMacGetHelploc();
	if (newhelp) {
	  helpfiles = (char *)newhelp;
	}
#endif
#endif

#ifdef HAVE_WX
    if (editor == NULL) editor = "";
#else
#ifdef unix
    if (editor == NULL) editor = "emacs";
#else
    if (editor == NULL) editor = "jove";
#endif /* unix */
#endif /* HAVE_WX */
    editorname = strrchr(editor, (int)'/');
    if (editorname == NULL) editorname = strrchr(editor, (int)'\\');
#ifdef WIN32
    if (editorname == NULL) {
	putenv("DESCRIBE=CMDS.DOC");
	putenv("JOVERC=JOVE.RC");
    } else {
	bufsiz = sprintf(buf, "DESCRIBE=%.*s\\CMDS.DOC", editorname-editor, editor);
	envp = malloc(bufsiz+1);
	strcpy(envp, buf);
	putenv(envp);
	bufsiz = sprintf(buf, "JOVERC=%.*s\\JOVE.RC", editorname-editor, editor);
	envp = malloc(bufsiz+1);
	strcpy(envp, buf);
	putenv(envp);
    }
#endif
    if (editorname == NULL) editorname = strrchr(editor, (int)':');
    editorname = (editorname ? editorname+1 : editor);
    tempdir = getenv("TEMP");
    if (tempdir == NULL) tempdir = temploc;
    while (prims[i].name) {
	if (prims[i].priority == MACRO_PRIORITY)
	    iproc = newnode(MACRO);
	else if (prims[i].priority <= TAIL_PRIORITY)
	    iproc = newnode(TAILFORM);
	else if ((prims[i].priority & ~4) == (PREFIX_PRIORITY & ~4))
	    iproc = newnode(PRIM); /* incl. -- */
	else
	    iproc = newnode(INFIX);
	setprimpri(iproc, prims[i].priority);
	setprimfun(iproc, prims[i].prim);
	setprimdflt(iproc, prims[i].defargs);
	setprimmax(iproc, prims[i].maxargs);
	setprimmin(iproc, prims[i].minargs);
	pname = make_static_strnode(prims[i].name);
	cnd = make_instance(pname, pname);
	setprocnode__caseobj(cnd, iproc);
	if (nodetype(iproc) == MACRO)
	    setflag__caseobj(cnd, PROC_MACRO);
	if (prims[i].minargs < 0)
	    setflag__caseobj(cnd, PROC_SPECFORM);
	setflag__caseobj(cnd, PERMANENT);
	i++;
    }
    Left_Paren = intern_p(make_static_strnode("("));
    Right_Paren = intern_p(make_static_strnode(")"));
    Minus_Sign = intern_p(make_static_strnode("-"));
    Minus_Tight = intern_p(make_static_strnode("--"));
    Query = intern_p(make_static_strnode("?"));
    Null_Word = intern_p(make_static_strnode("\0"));
    Redefp = intern_p(make_static_strnode("redefp"));
    Caseignoredp = intern_p(make_static_strnode("caseignoredp"));
    Erract = intern_p(make_static_strnode("erract"));
    Buttonact = intern_p(make_static_strnode("buttonact"));
    Keyact = intern_p(make_static_strnode("Keyact"));
    Printdepthlimit = intern_p(make_static_strnode("printdepthlimit"));
    Printwidthlimit = intern_p(make_static_strnode("printwidthlimit"));
    LoadNoisily = intern_p(make_static_strnode("loadnoisily"));
    AllowGetSet = intern_p(make_static_strnode("allowgetset"));
    Fullprintp = intern_p(make_static_strnode("fullprintp"));
    UnburyOnEdit = intern_p(make_static_strnode("unburyonedit"));
    UseAlternateNames = intern_p(make_static_strnode("usealternatenames"));
    Make = intern_p(make_static_strnode("make"));
    Listvalue = cons(intern_p(make_static_strnode("value")),NIL);
    Dotsvalue = make_colon(car(Listvalue));
    Pause = intern_p(make_static_strnode("pause"));
    Startup = intern_p(make_static_strnode("startup"));
    Startuplg = intern_p(make_static_strnode("startup.lg"));
    LogoVersion = intern_p(make_static_strnode("logoversion"));
    LogoPlatform = intern_p(make_static_strnode("logoplatform"));
    LogoLogo = intern_p(make_static_strnode("logo-logo"));
    CommandLine = intern_p(make_static_strnode("command.line"));
    setflag__caseobj(CommandLine, VAL_BURIED);
    the_generation = cons(NIL, NIL);
    Not_Enough_Node = cons(NIL, NIL);

    snprintf(linebuf, 256, "%s%sMessages", logolib, separator);
    fp = fopen("Messages", "r");
    if (fp == NULL)
	fp = fopen(linebuf, "r");
    if (fp == NULL)
	fp = fopen("C:\\cygwin\\usr\\local\\lib\\logo\\logolib\\Messages", "r");
    if (fp == NULL) {
	printf("Error -- Can't read Messages file.\n");
	printf("logolib: %s\n", logolib);
	exit(1);
    }

    for (i=0; i<(MAX_MESSAGE+NUM_WORDS); i++) {
	while (fgets(linebuf, 99, fp) != NULL && linebuf[0] == ';') ;
	linebuf[strlen(linebuf)-1] = '\0';
	message_texts[i] = (char *) malloc(1+strlen(linebuf));
	strcpy(message_texts[i], linebuf);
    }

    fclose(fp);

#define wd_copy(x) \
    translations[Name_ ## x].English = intern_p(make_static_strnode(#x)); \
    translations[Name_ ## x].Alt = \
	intern_p(make_static_strnode(message_texts[MAX_MESSAGE + Name_ ## x]));

    do_trans(wd_copy);

    translations[Name_macro].English = intern_p(make_static_strnode(".macro"));

#define True translations[Name_true].English
#define False translations[Name_false].English

    setvalnode__caseobj(Caseignoredp, True);
    setflag__caseobj(Caseignoredp, VAL_BURIED);
    setvalnode__caseobj(AllowGetSet, True);
    setflag__caseobj(AllowGetSet, VAL_BURIED);
    setvalnode__caseobj(Fullprintp, False);
    setflag__caseobj(Fullprintp, VAL_BURIED);
    setvalnode__caseobj(UnburyOnEdit, True);
    setflag__caseobj(UnburyOnEdit, VAL_BURIED);
    setvalnode__caseobj(UseAlternateNames, False);
    setflag__caseobj(UseAlternateNames, VAL_BURIED);

    setvalnode__caseobj(LogoPlatform, make_static_strnode(LogoPlatformName));
    setflag__caseobj(LogoPlatform, VAL_BURIED);

    user_repcount = -1;
    ift_iff_flag = -1;

    num_saved_nodes = (NODE **)(&(val_status)) - (NODE **)(&(proc));
    Regs_Node = newnode(STACK);
    Regs_Node->n_car = (NODE *)&regs;

//hist_inptr = hist_outptr = cmdHistory;

#ifdef OBJECTS
    obj_init();
#endif

/*  Uncomment these to print debugging messages right away! */
/*
    setvalnode__caseobj(Redefp, True);
    setflag__caseobj(Redefp, VAL_BURIED);
 */
}

