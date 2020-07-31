SUBDIRS = logolib csls helpfiles tests
if DOCS
SUBDIRS += docs
endif

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

if OBJECTS
ucblogo_SOURCES += obj.c
endif

if GITID
ucblogo_SOURCES += git.c

git.c:	$(SOURCES)
	echo 'char* GIT = "('`git describe||echo NA|tr -d '\r'`')";' > git.c
endif

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

# Include otherwise-skipped files in distribution tarball

# #include files used in build
EXTRA_DIST = eval.h globals.h gpl_text.h LogoFrame.h logo.h		\
 nographics.h obj.h TextEditor.h wxGlobals.h wxGraphics.h wxMain.h	\
 wxTerminal.h wxTurtleGraphics.h xgraphics.h

# Source for utility program for regenerating ./helpfiles/ from ./usermanual
# See below; needs to run in build environment.
EXTRA_DIST += makehelp.c

# Misc information, documentation, historical files, etc
EXTRA_DIST += changes.txt config.h.msys diffscript.pl .gitignore	\
 gpl.html Info.plist kdbgrc.logo LICENSE logo.icns logo-Info.plist	\
 logologo.ico logologo.png logo_win.rc makefile.msys newtermnotes	\
 pbdevelopment.plist PkgInfo plm README.md ucblogo.bmp ucblogo.dsp	\
 ucblogo.dsw ucblogo.png ucblogo.xpm usermanual inno/ucblogo.iss

# Diagram of evaluator, in three formats because hey why not
EXTRA_DIST += evaluator.ps evaluator.pdf evaluator.svg

# Win32 API compliant graphics routines (unused)
EXTRA_DIST += win32trm.c win32trm.h

# IBM-specific graphics routines (unused)
EXTRA_DIST += ztcterm.c ztcterm.h

# Macintosh graphics routines (unused)
EXTRA_DIST += macterm.c macterm.h

# Something to do with fonts on a Macintosh (unused)
EXTRA_DIST += mac-fontmod/include/wx/mac/carbon/fontdlg.h	\
 mac-fontmod/include/wx/mac/carbon/private.h			\
 mac-fontmod/include/wx/mac/fontdlg.h				\
 mac-fontmod/src/mac/carbon/bfontdlg.cpp			\
 mac-fontmod/src/mac/carbon/bfontdlgosx.mm			\
 mac-fontmod/src/mac/carbon/fontdlg.cpp				\
 mac-fontmod/src/mac/carbon/fontdlgosx.mm

# makehelp_SOURCES = makehelp.c

helpfiles/HELPCONTENTS helpfiles/ALL_NAMES: regenerate-help-index

# Redirection with this phony target avoids the stanza being run
# twice, thus avoiding unecessary work and a race condition.

.PHONY: regenerate-help-index

regenerate-help-index: makehelp usermanual
	@echo "Regenerates entire contents of helpfiles/, including index, from usermanual."
	./makehelp
	@echo "Grave accent as filename is impossible with automake so work around it."
	mv --verbose helpfiles/'`' helpfiles/RENAME-GRAVE-ACCENT
	LC_ALL=C sort helptemp | pr -5 -t -l999 -w80 >> helpfiles/HELPCONTENTS
	-rm helptemp

all: regenerate-help-index

mac: all
	mkdir -p UCBLogo.app
	mkdir -p UCBLogo.app/Contents
	cp Info.plist UCBLogo.app/Contents/
	cp PkgInfo UCBLogo.app/Contents/
	cp pbdevelopment.plist UCBLogo.app/Contents/
	mkdir -p UCBLogo.app/Contents/Resources/csls
	cp csls/[a-z]* UCBLogo.app/Contents/Resources/csls
	cp -r helpfiles UCBLogo.app/Contents/Resources/
	cp -r logolib UCBLogo.app/Contents/Resources/
	cp logo.icns UCBLogo.app/Contents/Resources/
	mkdir -p UCBLogo.app/Contents/MacOS/
	cp ucblogo UCBLogo.app/Contents/MacOS/UCBLogo

ucblogo.dmg : mac
	rm -f ucblogo.dmg ucblogo_base.dmg
	hdiutil create -size 15m -fs HFS+ -volname "UCBLogo" ucblogo_base.dmg
	hdiutil attach ucblogo_base.dmg
	cp -a UCBLogo.app /Volumes/UCBLogo/
	cp docs/usermanual.pdf /Volumes/UCBLogo/UCBLogoUserManual.pdf
	hdiutil detach /Volumes/UCBLogo/
	hdiutil convert ucblogo_base.dmg -format UDZO -o ucblogo.dmg
