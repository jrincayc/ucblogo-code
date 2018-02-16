bin_PROGRAMS = ucblogo

logolibdir=$(pkgdatadir)/logolib
helpfilesdir=$(pkgdatadir)/helpfiles
cslsdir=$(pkgdatadir)/csls
tempdir=/tmp

ucblogo_CPPFLAGS = $(AM_CPPFLAGS)	\
 -DUSE_OLD_TTY				\
 -DLIBLOC='"$(logolibdir)"'		\
 -DHELPLOC='"$(helpfilesdir)"'		\
 -DCSLSLOC='"$(cslsdir)"'		\
 -DTEMPLOC='"$(tempdir)"'

ucblogo_SOURCES = coms.c error.c eval.c files.c graphics.c init.c	\
 intern.c libloc.c lists.c logodata.c main.c math.c mem.c paren.c	\
 parse.c print.c wrksp.c nographics.c term.c xgraphics.c

# TERM_FILE = term.c
# GRAPHICS_FILE = xgraphics.c

#  -lbsd -lm  -ltermcap -lX11

# logolib/Messages: makelib Messages
# 	./makelib
# 	cp Messages logolib/

# makehelp_SOURCES = makehelp.c

# TextEditor_SOURCES = TextEditor.cpp
