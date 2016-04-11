CC	= gcc
CFLAGS	= -g -O -DHAVE_WX    -O0
CXX     = g++
CXXFLAGS = -g  -DHAVE_WX -I/usr/local/lib/wx/include/gtk2-ansi-release-static-2.8 -I/usr/local/include/wx-2.8 -D_FILE_OFFSET_BITS=64 -D_LARGE_FILES -D__WXGTK__ -pthread
LDFLAGS	= 
LIBS  =   -lbsd -lm  -L/usr/local/lib -pthread   /usr/local/lib/libwx_gtk2_richtext-2.8.a /usr/local/lib/libwx_gtk2_aui-2.8.a /usr/local/lib/libwx_gtk2_xrc-2.8.a /usr/local/lib/libwx_gtk2_qa-2.8.a /usr/local/lib/libwx_gtk2_html-2.8.a /usr/local/lib/libwx_gtk2_adv-2.8.a /usr/local/lib/libwx_gtk2_core-2.8.a /usr/local/lib/libwx_base_xml-2.8.a /usr/local/lib/libwx_base_net-2.8.a /usr/local/lib/libwx_base-2.8.a -pthread -L/lib -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lpango-1.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lglib-2.0 -lXinerama -lXxf86vm -lSM -lpng -ljpeg -ltiff -lexpat -lz -ldl -lm  -ltermcap -lX11 
prefix = /usr/local
BINDIR        = $(prefix)/bin
LIBLOC        = $(prefix)/lib/logo
LINKER = $(CXX)

# LIBLOC      = `pwd`

OBJS 	= coms.o error.o eval.o files.o graphics.o init.o intern.o \
	  libloc.o lists.o logodata.o main.o math.o mem.o paren.o parse.o \
	  print.o wrksp.o nographics.o svn.o wxMain.o wxTerminal.o wxTurtleGraphics.o  TextEditor.o wxterm.o 

SRCS	= coms.c error.c eval.c files.c graphics.c init.c intern.c \
	  libloc.c lists.c logodata.c main.c math.c mem.c paren.c parse.c \
	  print.c wrksp.c nographics.c wxMain.cpp wxTerminal.cpp wxTurtleGraphics.cpp  TextEditor.cpp wxterm.c 

HDRS	= globals.h logo.h xgraphics.h

logo:	$(OBJS)
	$(LINKER) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o logo

everything:	logo logolib/Messages helpfiles helpfiles/HELPCONTENTS
#logo-mode

mem.o:	mem.c
	$(CC) $(CFLAGS) -O0 -c mem.c

svn.c:	$(SRCS)
	echo 'char* SVN = "('`svnversion|tr -d '\r'`')";' > svn.c

tags:	$(SRCS)
	ctags --format=1 -N $(SRCS) $(HDRS)
# 	ctags -t $(SRCS) $(HDRS)

libloc.c:
	echo 'char *libloc="'$(LIBLOC)'/logolib";' > libloc.c
	echo 'char *helploc="'$(LIBLOC)'/helpfiles";' >> libloc.c
	echo 'char *cslsloc="'$(LIBLOC)'/csls";' >> libloc.c
	echo 'char *temploc="/tmp";' >> libloc.c
	echo 'char *separator="/";' >> libloc.c

logolib/Messages:	makelib Messages
	chmod +x makelib
	./makelib
	cp -f Messages logolib

helpfiles:
	mkdir helpfiles

helpfiles/HELPCONTENTS:	makehelp usermanual
	./makehelp
	sort helptemp | pr -5 -t -l999 -w80 >> helpfiles/HELPCONTENTS
	rm helptemp

makehelp:	makehelp.c
	$(CC) -o makehelp makehelp.c

clean:
	rm -f *.o libloc.c
#	cd emacs; $(MAKE) clean

ship:
	rm -f config.h config.cache config.log config.status
	rm -f makefile makehelp logo *.o libloc.c
#	cd emacs; $(MAKE) ship
	cd docs; $(MAKE) ship

install: all
	for d in $(BINDIR) $(LIBLOC) $(LIBLOC)/logolib $(LIBLOC)/helpfiles $(LIBLOC)/csls; do [ -d $$d ] || mkdir -p $$d || exit 1; done
	cp logo $(BINDIR)/.
	cp -f logolib/* $(LIBLOC)/logolib/.
	cp -f helpfiles/* $(LIBLOC)/helpfiles/.
	cp -f csls/* $(LIBLOC)/csls/.
#	(cd emacs; prefix=$(prefix) LIBLOC=$(LIBLOC) BINDIR=$(BINDIR) $(MAKE) install)
	(cd docs; prefix=$(prefix) LIBLOC=$(LIBLOC) BINDIR=$(BINDIR) $(MAKE) install)
#	prefix=$(prefix); LIBLOC=$(LIBLOC); BINDIR=$(BINDIR); export prefix LIBLOC BINDIR; cd emacs; $(MAKE) install

logo-mode: 
#	(cd emacs; prefix=$(prefix) LIBLOC=$(LIBLOC) BINDIR=$(BINDIR) $(MAKE))
#	@prefix=$(prefix); LIBLOC=$(LIBLOC); BINDIR=$(BINDIR); export prefix LIBLOC BINDIR; cd emacs; $(MAKE)

make-docs:
	(cd docs; prefix=$(prefix) LIBLOC=$(LIBLOC) $(MAKE) all)

mac: all
	mkdir -p UCBLogo.app
	mkdir -p UCBLogo.app/Contents
	cp Info.plist UCBLogo.app/Contents/
	cp PkgInfo UCBLogo.app/Contents/
	cp pbdevelopment.plist UCBLogo.app/Contents/
	mkdir -p UCBLogo.app/Contents/Resources
	cp csls/[a-z]* UCBLogo.app/Contents/Resources/csls
	cp -r helpfiles UCBLogo.app/Contents/Resources/
	cp -r logolib UCBLogo.app/Contents/Resources/
	cp logo.icns UCBLogo.app/Contents/Resources/
	mkdir -p UCBLogo.app/Contents/MacOS/
	cp logo UCBLogo.app/Contents/MacOS/UCBLogo
