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

# The file git.c is a "built source" (see automake info page "Built
# Sources Example"), a conditional source file, and should not be
# included in distribution tarballs. It also may need to be
# regenerated even if it seems up to date, since make cannot tell if a
# git commit has occurred since git.c was generated. This is
# accomplished by making it depend on a dummy phony target, which is
# by definition never up to date. During regeneration, if git.c exists
# it is replaced only if the new version differs, in order to avoid
# unnecessary recompilation or linking.
if GITID
nodist_ucblogo_SOURCES = git.c
endif
MOSTLYCLEANFILES = git.c
.PHONY: FORCE
FORCE:
git.c: FORCE
	echo 'char* GIT = "('`git describe --always||echo -n NA`')";' > git-c-temp
	if cmp --quiet git-c-temp $@; then echo "$@ already up to date"; else echo "Refreshing $@"; mv git-c-temp $@; fi

if WX
# need C++ linker
ucblogo_SOURCES += wxMain.cpp wxTerminal.cpp wxCommandHistory.cpp wxTurtleGraphics.cpp
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

# Desktop file, see https://freedesktop.org/wiki/Howto_desktop_files/

desktopdir = $(datadir)/applications
dist_desktop_DATA = ucblogo.desktop

# Icons

icons256dir = $(datadir)/icons/hicolor/256x256/apps
pixmapsdir = $(datadir)/pixmaps
dist_icons256_DATA = ucblogo.png
dist_pixmaps_DATA = ucblogo.xpm

# Include otherwise-skipped files in distribution tarball

# #include files used in build
EXTRA_DIST = eval.h globals.h gpl_text.h LogoFrame.h logo.h		\
 nographics.h obj.h TextEditor.h wxGlobals.h wxGraphics.h wxMain.h	\
 wxTerminal.h wxCommandHistory.h wxTurtleGraphics.h xgraphics.h

# Source for utility program for regenerating ./helpfiles/ from ./usermanual
# See below; needs to run in build environment.
EXTRA_DIST += makehelp.c
CLEANFILES = makehelp

# Misc information, documentation, historical files, etc
EXTRA_DIST += changes.txt config.h.msys diffscript.pl .gitignore	\
 gpl.html Info.plist kdbgrc.logo LICENSE logo.icns logo-Info.plist	\
 logologo.ico logologo.png logo_win.rc makefile.msys newtermnotes	\
 pbdevelopment.plist PkgInfo plm README.md ucblogo.bmp ucblogo.dsp	\
 ucblogo.dsw usermanual inno/ucblogo.iss

# Diagram of evaluator, in three formats because hey why not
EXTRA_DIST += evaluator.ps evaluator.pdf evaluator.svg

# Win32 API compliant graphics routines (unused)
EXTRA_DIST += win32trm.c win32trm.h

# makehelp_SOURCES = makehelp.c

# Make sure pdf built for distributions
EXTRA_DIST += docs/ucblogo.pdf

docs/ucblogo.pdf: docs/ucblogo.texi
	cd docs && $(MAKE) ucblogo.pdf

helpfiles/HELPCONTENTS helpfiles/ALL_NAMES: regenerate-help-index

# Redirection with this phony target avoids the stanza being run
# twice, thus avoiding unecessary work and a race condition.

.PHONY: regenerate-help-index

regenerate-help-index: makehelp usermanual
	@echo "Regenerates entire contents of helpfiles/, including index, from usermanual."
	./makehelp
	@echo "Grave accent as filename is impossible with automake so work around it."
	mv -v helpfiles/'`' helpfiles/RENAME-GRAVE-ACCENT
	LC_ALL=C sort helptemp | pr -5 -t -l999 -w80 >> helpfiles/HELPCONTENTS
	-rm helptemp

all: regenerate-help-index

mac: ucblogo helpfiles/HELPCONTENTS helpfiles/ALL_NAMES
	mkdir -p UCBLogo.app
	mkdir -p UCBLogo.app/Contents
	cp Info.plist UCBLogo.app/Contents/
	cp PkgInfo UCBLogo.app/Contents/
	cp pbdevelopment.plist UCBLogo.app/Contents/
	mkdir -p UCBLogo.app/Contents/Resources/csls
	cp csls/[a-z]* UCBLogo.app/Contents/Resources/csls
	cp -r helpfiles UCBLogo.app/Contents/Resources/
	cp -r logolib UCBLogo.app/Contents/Resources/
	mv -v UCBLogo.app/Contents/Resources/logolib/RENAME-GRAVE-ACCENT 'UCBLogo.app/Contents/Resources/logolib/`'
	mv -v UCBLogo.app/Contents/Resources/logolib/RENAME-NUMBER-SIGN 'UCBLogo.app/Contents/Resources/logolib/#'
	cp logo.icns UCBLogo.app/Contents/Resources/
	mkdir -p UCBLogo.app/Contents/MacOS/
	cp ucblogo UCBLogo.app/Contents/MacOS/UCBLogo

ucblogo.dmg : mac docs/ucblogo.pdf
	rm -f ucblogo.dmg ucblogo_base.dmg
	hdiutil create -size 20m -fs HFS+ -volname "UCBLogo" ucblogo_base.dmg
	hdiutil attach ucblogo_base.dmg
	cp -a UCBLogo.app /Volumes/UCBLogo/
	cp docs/ucblogo.pdf /Volumes/UCBLogo/UCBLogoUserManual.pdf
	hdiutil detach /Volumes/UCBLogo/
	hdiutil convert ucblogo_base.dmg -format UDZO -o ucblogo.dmg
