SUBDIRS = docs logolib csls helpfiles

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

# makehelp_SOURCES = makehelp.c

helpfiles/HELPCONTENTS helpfiles/ALL_NAMES: regenerate-help-index

# Redirection with this phony target avoids the stanza being run
# twice, thus avoiding unecessary work and a race condition.

.PHONY: regenerate-help-index

regenerate-help-index: makehelp usermanual
	./makehelp
	LC_ALL=C sort helptemp | pr -5 -t -l999 -w80 >> helpfiles/HELPCONTENTS
	-rm helptemp

all: regenerate-help-index
