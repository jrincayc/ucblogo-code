helpfilesdir=$(pkgdatadir)/helpfiles

dist_helpfiles_DATA = allopen allowgetset and apply arc arctan arity	\
 array arrayp arraytolist ascii ashift back background backslashedp	\
 beforep bf bfs bg bitand bitnot bitor bitxor bk bl buried buriedp	\
 bury buryall buryname butfirst butfirsts butlast button buttonact	\
 buttonp bye cascade case caseignoredp catch char clean clearscreen	\
 cleartext clickpos close closeall co combine commandline cond		\
 contents continue copydef cos count crossmap cs cslsload ct cursor	\
 ddefmacro decreasefont define definedp deq dequeue difference dmacro	\
 dmaybeoutput doduntil dodwhile dribble dsetbf dsetfirst dsetitem	\
 dsetsegmentsize ed edall edit editfile edn edns edpl edpls edps	\
 emptyp eofp epspict equalp er erall erase erasefile erf ern erns	\
 erpl erpls erps erract error exp fd fence filep fill filled filter	\
 find first firsts font for foreach forever form forward fput fs	\
 fullprintp fullscreen fulltext gc gensym global goto gprop		\
 greaterequalp greaterp heading help hideturtle home ht if ifelse iff	\
 iffalse ift iftrue ignore increasefont int invoke iseq item keyact	\
 keyp label labelsize last left lessequalp lessp list listp		\
 listtoarray ln load loadnoisily loadpict local localmake log10		\
 logoplatform logoversion lowercase lput lshift lt macroexpand macrop	\
 make map mapdse mdarray mditem mdsetitem member memberp minus modulo	\
 mousepos name namelist namep names nodes nodribble norefresh not	\
 notequalp numberp op openappend openread openupdate openwrite or	\
 output palette parse pause pc pd pe pen pencolor pendown pendownp	\
 penerase penmode penpaint penpattern penreverse pensize penup pick	\
 plist plistp plists pllist po poall pon pons pop popl popls pops pos	\
 pot pots power pprop ppt pr prefix primitivep primitives print		\
 printdepthlimit printout printwidthlimit procedurep procedures		\
 product pu push px queue quoted quotient radarctan radcos radsin	\
 random rawascii rc rcs readchar readchars reader readlist readpos	\
 readrawline readword redefp reduce refresh remainder remdup remove	\
 remprop repcount repeat rerandom reverse right rl round rseq rt run	\
 runparse runresult rw save savel savepict screenmode scrunch se	\
 sentence setbackground setbg setcslsloc setcursor seteditor setfont	\
 seth setheading sethelploc setitem setlabelheight setlibloc		\
 setmargins setpalette setpc setpen setpencolor setpenpattern		\
 setpensize setpos setprefix setread setreadpos setscrunch settc	\
 settemploc settextcolor settextfont settextsize setwrite setwritepos	\
 setx setxy sety shell show shownp showturtle sin splitscreen sqrt ss	\
 st standout startup step stepped steppedp stop substringp sum tag	\
 test text textscreen textsize thing throw to towards trace traced	\
 tracedp transfer ts turtlemode type unbury unburyall unburyname	\
 unburyonedit unstep until untrace uppercase usealternatenames		\
 vbarredp wait while window word wordp wrap writepos writer xcor ycor

dist_helpfiles_DATA += RENAME-GRAVE-ACCENT

nodist_helpfiles_DATA = HELPCONTENTS ALL_NAMES

install-data-hook:
	(cd "$(DESTDIR)$(helpfilesdir)" && \
	mv --verbose RENAME-GRAVE-ACCENT '`')

uninstall-hook:
	(cd "$(DESTDIR)$(helpfilesdir)" && \
	rm -f '`')
