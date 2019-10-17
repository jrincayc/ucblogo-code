SUBDIRS = docs

bin_PROGRAMS = ucblogo

logolibdir=$(pkgdatadir)/logolib
helpfilesdir=$(pkgdatadir)/helpfiles
cslsdir=$(pkgdatadir)/csls
tempdir=/tmp

ucblogo_CPPFLAGS = $(AM_CPPFLAGS)	\
 -DLIBLOC='"$(logolibdir)"'		\
 -DHELPLOC='"$(helpfilesdir)"'		\
 -DCSLSLOC='"$(cslsdir)"'		\
 -DTEMPLOC='"$(tempdir)"'

ucblogo_SOURCES = coms.c error.c eval.c files.c graphics.c init.c	\
 intern.c libloc.c lists.c logodata.c main.c math.c mem.c paren.c	\
 parse.c print.c wrksp.c

if WX
# need C++ linker
ucblogo_SOURCES += wxMain.cpp wxTerminal.cpp wxTurtleGraphics.cpp
ucblogo_SOURCES += TextEditor.cpp
ucblogo_SOURCES += wxterm.c
else
# do not need C++ linker, no good way to avoid it though
ucblogo_SOURCES += term.c
if X11
ucblogo_SOURCES += xgraphics.c
else
ucblogo_SOURCES += nographics.c
endif
endif

# logolib/Messages: makelib Messages
# 	./makelib
# 	cp Messages logolib/

# makehelp_SOURCES = makehelp.c

logolib_DATA = logolib/RENAME-GRAVE-ACCENT logolib/RENAME-NUMBER-SIGN	\
 logolib/backslashedp logolib/backslashedQ logolib/buryall		\
 logolib/buryname logolib/cascade logolib/cascade.2 logolib/case	\
 logolib/closeall logolib/combine logolib/cond logolib/crossmap		\
 logolib/dequeue logolib/do.until logolib/do.while logolib/edall	\
 logolib/edn logolib/edns logolib/edpl logolib/edpls logolib/edps	\
 logolib/emacs.debug logolib/ern logolib/erpl logolib/filep		\
 logolib/fileQ logolib/filter logolib/find logolib/for			\
 logolib/foreach logolib/gensym logolib/ignore logolib/invoke		\
 logolib/iseq logolib/localmake logolib/macroexpand logolib/map		\
 logolib/map.se logolib/mdarray logolib/mditem logolib/mdsetitem	\
 logolib/Messages logolib/Messages.fr logolib/Messages.sp		\
 logolib/Messages.zh_TW logolib/name logolib/namelist logolib/pen	\
 logolib/pick logolib/pllist logolib/poall logolib/pon logolib/pons	\
 logolib/pop logolib/popl logolib/popls logolib/pops logolib/pots	\
 logolib/push logolib/Qrest logolib/queue logolib/quoted		\
 logolib/reduce logolib/remdup logolib/remove logolib/reverse		\
 logolib/rseq logolib/savel logolib/setpen logolib/transfer		\
 logolib/unburyall logolib/unburyname logolib/until logolib/while	\
 logolib/xcor logolib/ycor

csls_DATA = csls/algs csls/basic csls/buttons csls/cards csls/crypto	\
 csls/diff csls/doctor csls/dotgame csls/format csls/fsm csls/master	\
 csls/match csls/math csls/mines csls/multi csls/pascal csls/playfair	\
 csls/plot csls/poker csls/pour csls/psort csls/solitaire		\
 csls/streams csls/student csls/tower csls/ttt

helpfiles_DATA = helpfiles/RENAME-GRAVE-ACCENT 				\
 helpfiles/allopen helpfiles/allowgetset helpfiles/and			\
 helpfiles/apply helpfiles/arc helpfiles/arctan helpfiles/arity		\
 helpfiles/array helpfiles/arrayp helpfiles/arraytolist			\
 helpfiles/ascii helpfiles/ashift helpfiles/back helpfiles/background	\
 helpfiles/backslashedp helpfiles/beforep helpfiles/bf helpfiles/bfs	\
 helpfiles/bg helpfiles/bitand helpfiles/bitnot helpfiles/bitor		\
 helpfiles/bitxor helpfiles/bk helpfiles/bl helpfiles/buried		\
 helpfiles/buriedp helpfiles/bury helpfiles/buryall			\
 helpfiles/buryname helpfiles/butfirst helpfiles/butfirsts		\
 helpfiles/butlast helpfiles/button helpfiles/buttonact			\
 helpfiles/buttonp helpfiles/bye helpfiles/cascade helpfiles/case	\
 helpfiles/caseignoredp helpfiles/catch helpfiles/char			\
 helpfiles/clean helpfiles/clearscreen helpfiles/cleartext		\
 helpfiles/clickpos helpfiles/close helpfiles/closeall helpfiles/co	\
 helpfiles/combine helpfiles/commandline helpfiles/cond			\
 helpfiles/contents helpfiles/continue helpfiles/copydef		\
 helpfiles/cos helpfiles/count helpfiles/crossmap helpfiles/cs		\
 helpfiles/cslsload helpfiles/ct helpfiles/cursor helpfiles/ddefmacro	\
 helpfiles/decreasefont helpfiles/define helpfiles/definedp		\
 helpfiles/deq helpfiles/dequeue helpfiles/difference			\
 helpfiles/dmacro helpfiles/dmaybeoutput helpfiles/doduntil		\
 helpfiles/dodwhile helpfiles/dribble helpfiles/dsetbf			\
 helpfiles/dsetfirst helpfiles/dsetitem helpfiles/dsetsegmentsize	\
 helpfiles/ed helpfiles/edall helpfiles/edit helpfiles/editfile		\
 helpfiles/edn helpfiles/edns helpfiles/edpl helpfiles/edpls		\
 helpfiles/edps helpfiles/emptyp helpfiles/eofp helpfiles/epspict	\
 helpfiles/equalp helpfiles/er helpfiles/erall helpfiles/erase		\
 helpfiles/erasefile helpfiles/erf helpfiles/ern helpfiles/erns		\
 helpfiles/erpl helpfiles/erpls helpfiles/erps helpfiles/erract		\
 helpfiles/error helpfiles/exp helpfiles/fd helpfiles/fence		\
 helpfiles/filep helpfiles/fill helpfiles/filled helpfiles/filter	\
 helpfiles/find helpfiles/first helpfiles/firsts helpfiles/font		\
 helpfiles/for helpfiles/foreach helpfiles/forever helpfiles/form	\
 helpfiles/forward helpfiles/fput helpfiles/fs helpfiles/fullprintp	\
 helpfiles/fullscreen helpfiles/fulltext helpfiles/gc			\
 helpfiles/gensym helpfiles/global helpfiles/goto helpfiles/gprop	\
 helpfiles/greaterequalp helpfiles/greaterp helpfiles/heading		\
 helpfiles/help helpfiles/hideturtle helpfiles/home helpfiles/ht	\
 helpfiles/if helpfiles/ifelse helpfiles/iff helpfiles/iffalse		\
 helpfiles/ift helpfiles/iftrue helpfiles/ignore			\
 helpfiles/increasefont helpfiles/int helpfiles/invoke helpfiles/iseq	\
 helpfiles/item helpfiles/keyact helpfiles/keyp helpfiles/label		\
 helpfiles/labelsize helpfiles/last helpfiles/left			\
 helpfiles/lessequalp helpfiles/lessp helpfiles/list helpfiles/listp	\
 helpfiles/listtoarray helpfiles/ln helpfiles/load			\
 helpfiles/loadnoisily helpfiles/loadpict helpfiles/local		\
 helpfiles/localmake helpfiles/log10 helpfiles/logoplatform		\
 helpfiles/logoversion helpfiles/lowercase helpfiles/lput		\
 helpfiles/lshift helpfiles/lt helpfiles/macroexpand helpfiles/macrop	\
 helpfiles/make helpfiles/map helpfiles/mapdse helpfiles/mdarray	\
 helpfiles/mditem helpfiles/mdsetitem helpfiles/member			\
 helpfiles/memberp helpfiles/minus helpfiles/modulo			\
 helpfiles/mousepos helpfiles/name helpfiles/namelist helpfiles/namep	\
 helpfiles/names helpfiles/nodes helpfiles/nodribble			\
 helpfiles/norefresh helpfiles/not helpfiles/notequalp			\
 helpfiles/numberp helpfiles/op helpfiles/openappend			\
 helpfiles/openread helpfiles/openupdate helpfiles/openwrite		\
 helpfiles/or helpfiles/output helpfiles/palette helpfiles/parse	\
 helpfiles/pause helpfiles/pc helpfiles/pd helpfiles/pe helpfiles/pen	\
 helpfiles/pencolor helpfiles/pendown helpfiles/pendownp		\
 helpfiles/penerase helpfiles/penmode helpfiles/penpaint		\
 helpfiles/penpattern helpfiles/penreverse helpfiles/pensize		\
 helpfiles/penup helpfiles/pick helpfiles/plist helpfiles/plistp	\
 helpfiles/plists helpfiles/pllist helpfiles/po helpfiles/poall		\
 helpfiles/pon helpfiles/pons helpfiles/pop helpfiles/popl		\
 helpfiles/popls helpfiles/pops helpfiles/pos helpfiles/pot		\
 helpfiles/pots helpfiles/power helpfiles/pprop helpfiles/ppt		\
 helpfiles/pr helpfiles/prefix helpfiles/primitivep			\
 helpfiles/primitives helpfiles/print helpfiles/printdepthlimit		\
 helpfiles/printout helpfiles/printwidthlimit helpfiles/procedurep	\
 helpfiles/procedures helpfiles/product helpfiles/pu helpfiles/push	\
 helpfiles/px helpfiles/queue helpfiles/quoted helpfiles/quotient	\
 helpfiles/radarctan helpfiles/radcos helpfiles/radsin			\
 helpfiles/random helpfiles/rawascii helpfiles/rc helpfiles/rcs		\
 helpfiles/readchar helpfiles/readchars helpfiles/reader		\
 helpfiles/readlist helpfiles/readpos helpfiles/readrawline		\
 helpfiles/readword helpfiles/redefp helpfiles/reduce			\
 helpfiles/refresh helpfiles/remainder helpfiles/remdup			\
 helpfiles/remove helpfiles/remprop helpfiles/repcount			\
 helpfiles/repeat helpfiles/rerandom helpfiles/reverse			\
 helpfiles/right helpfiles/rl helpfiles/round helpfiles/rseq		\
 helpfiles/rt helpfiles/run helpfiles/runparse helpfiles/runresult	\
 helpfiles/rw helpfiles/save helpfiles/savel helpfiles/savepict		\
 helpfiles/screenmode helpfiles/scrunch helpfiles/se			\
 helpfiles/sentence helpfiles/setbackground helpfiles/setbg		\
 helpfiles/setcslsloc helpfiles/setcursor helpfiles/seteditor		\
 helpfiles/setfont helpfiles/seth helpfiles/setheading			\
 helpfiles/sethelploc helpfiles/setitem helpfiles/setlabelheight	\
 helpfiles/setlibloc helpfiles/setmargins helpfiles/setpalette		\
 helpfiles/setpc helpfiles/setpen helpfiles/setpencolor			\
 helpfiles/setpenpattern helpfiles/setpensize helpfiles/setpos		\
 helpfiles/setprefix helpfiles/setread helpfiles/setreadpos		\
 helpfiles/setscrunch helpfiles/settc helpfiles/settemploc		\
 helpfiles/settextcolor helpfiles/settextfont helpfiles/settextsize	\
 helpfiles/setwrite helpfiles/setwritepos helpfiles/setx		\
 helpfiles/setxy helpfiles/sety helpfiles/shell helpfiles/show		\
 helpfiles/shownp helpfiles/showturtle helpfiles/sin			\
 helpfiles/splitscreen helpfiles/sqrt helpfiles/ss helpfiles/st		\
 helpfiles/standout helpfiles/startup helpfiles/step			\
 helpfiles/stepped helpfiles/steppedp helpfiles/stop			\
 helpfiles/substringp helpfiles/sum helpfiles/tag helpfiles/test	\
 helpfiles/text helpfiles/textscreen helpfiles/textsize			\
 helpfiles/thing helpfiles/throw helpfiles/to helpfiles/towards		\
 helpfiles/trace helpfiles/traced helpfiles/tracedp			\
 helpfiles/transfer helpfiles/ts helpfiles/turtlemode helpfiles/type	\
 helpfiles/unbury helpfiles/unburyall helpfiles/unburyname		\
 helpfiles/unburyonedit helpfiles/unstep helpfiles/until		\
 helpfiles/untrace helpfiles/uppercase helpfiles/usealternatenames	\
 helpfiles/vbarredp helpfiles/wait helpfiles/while helpfiles/window	\
 helpfiles/word helpfiles/wordp helpfiles/wrap helpfiles/writepos	\
 helpfiles/writer helpfiles/xcor helpfiles/ycor

helpfiles_DATA += helpfiles/HELPCONTENTS helpfiles/ALL_NAMES

helpfiles/HELPCONTENTS helpfiles/ALL_NAMES: regenerate-help-index

# Redirection with this phony target avoids the stanza being run
# twice, thus avoiding unecessary work and a race condition.

.PHONY: regenerate-help-index

regenerate-help-index: makehelp usermanual
	./makehelp
	LC_ALL=C sort helptemp | pr -5 -t -l999 -w80 >> helpfiles/HELPCONTENTS
	-rm helptemp

install-data-hook:
	(cd "$(DESTDIR)$(logolibdir)" && \
	mv --verbose RENAME-GRAVE-ACCENT '`' &&
	mv --verbose RENAME-NUMBER-SIGN '#')
	(cd "$(DESTDIR)$(helpfilesdir)" && \
	mv --verbose RENAME-GRAVE-ACCENT '`')

uninstall-hook:
	(cd "$(DESTDIR)$(logolibdir)" && \
	rm -f '`' && \
	rm -f '#')
	(cd "$(DESTDIR)$(helpfilesdir)" && \
	rm -f '`')
