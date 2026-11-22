SUBDIRS = logolib csls helpfiles tests
if DOCS
SUBDIRS += docs
endif
AM_MAKEFLAGS = --no-print-directory

if WASM
# Linked by the ucblogo.html and ucblogo-node.js rules below instead.
EXTRA_PROGRAMS = ucblogo
else
bin_PROGRAMS = ucblogo
endif

logolibdir=$(pkgdatadir)/logolib
helpfilesdir=$(pkgdatadir)/helpfiles
cslsdir=$(pkgdatadir)/csls
tempdir=/tmp

ucblogo_CPPFLAGS = $(AM_CPPFLAGS)	\
 -DLIBLOC='"$(logolibdir)"'		\
 -DHELPLOC='"$(helpfilesdir)"'		\
 -DCSLSLOC='"$(cslsdir)"'		\
 -DTEMPLOC='"$(tempdir)"'

# FIXME add header files
ucblogo_SOURCES = coms.c error.c eval.c files.c graphics.c init.c	\
 intern.c libloc.c lists.c logodata.c math.c mem.c paren.c	\
 parse.c print.c turtleshape.c wrksp.c

if WASM
ucblogo_LDFLAGS = $(AM_LDFLAGS) --js-library liblogo.js 
ucblogo_SOURCES += webmain.c
ucblogo_SOURCES += weblib.c
else
ucblogo_SOURCES += main.c
endif

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
	@echo 'char* GIT = "('`git describe --tags --always --dirty||echo -n NA`')";' > git-c-temp
	@if cmp --quiet git-c-temp $@; then rm -f git-c-temp; else mv git-c-temp $@; fi

if WX
# need C++ linker
ucblogo_SOURCES += wxMain.cpp wxTerminal.cpp wxCommandHistory.cpp wxTurtleGraphics.cpp
ucblogo_SOURCES += TextEditor.cpp
ucblogo_SOURCES += wxterm.c
else
# do not need C++ linker, no good way to avoid it though
if WASM
ucblogo_SOURCES += webterm.c
else
ucblogo_SOURCES += term.c
endif
if X11
ucblogo_SOURCES += xgraphics.c
else
if WASM
ucblogo_SOURCES += webgraphics.c
else
ucblogo_SOURCES += nographics.c
endif
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

# Co-authored by GitHub Copilot. Scope: Phase 1 inspector ABI/build wiring.
# Model: GPT-6 Luna; reasoning setting unavailable (maximum effort requested).
# Task wall time: 4m 33s (as of 2026-11-17 18:16:27 UTC).
# Energy (kWh) and CO2: unavailable.

# #include files used in build
EXTRA_DIST = eval.h globals.h gpl_text.h LogoFrame.h logo.h		\
 nographics.h obj.h TextEditor.h turtleshape.h wxGlobals.h		\
 wxGraphics.h wxMain.h wxTerminal.h wxCommandHistory.h			\
 wxTurtleGraphics.h xgraphics.h webgraphics.h storage-status.js logo-inspect.js
 
# # Source for utility program for regenerating ./helpfiles/ from ./usermanual
# # See below; needs to run in build environment.
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

makehelp: $(srcdir)/makehelp.c
	$(CC_FOR_BUILD) $(CPPFLAGS_FOR_BUILD) $(CFLAGS_FOR_BUILD) $(LDFLAGS_FOR_BUILD) -o $@ $(srcdir)/makehelp.c

# Make sure pdf built for distributions
EXTRA_DIST += docs/ucblogo.pdf

docs/ucblogo.pdf: docs/ucblogo.texi
	cd docs && $(MAKE) ucblogo.pdf

helpfiles/HELPCONTENTS helpfiles/ALL_NAMES: regenerate-help-index

# Redirection with this phony target avoids the stanza being run
# twice, thus avoiding unecessary work and a race condition.

.PHONY: regenerate-help-index

regenerate-help-index: makehelp usermanual
	$(AM_V_GEN)./makehelp
	@# Grave accent as filename is impossible with automake so work around it.
	@mv helpfiles/'`' helpfiles/RENAME-GRAVE-ACCENT
	@LC_ALL=C sort helptemp | pr -5 -t -l999 -w80 >> helpfiles/HELPCONTENTS
	@-rm helptemp

all: 

WEB_TARGETS =
WEB_STALE =
if WASM
WEB_TARGETS += ucblogo-node.js ucblogo.html
if SERVICE_WORKER
WEB_TARGETS += sw.js
else
WEB_STALE += sw.js
endif
endif

if SEPARATE_DWARF
WEB_NODE_DEBUG = -gseparate-dwarf=ucblogo-node.debug.wasm -sSEPARATE_DWARF_URL=ucblogo-node.debug.wasm
WEB_HTML_DEBUG = -gseparate-dwarf=ucblogo.debug.wasm -sSEPARATE_DWARF_URL=ucblogo.debug.wasm
endif

all-local: $(WEB_TARGETS)
	rm -f $(WEB_STALE)

web-config.js: Makefile
	$(AM_V_GEN)echo 'Module.serviceWorker = $(WEB_SERVICE_WORKER);' > $@

sw.js: ucblogo.html workbox-config.js
	@rm -f workbox-[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f].js workbox-[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f].js.map
	$(AM_V_GEN)npx --no -- workbox-cli generateSW workbox-config.js >/dev/null

if WASM
check-local: ucblogo-node.js
	node ./ucblogo-node.js $(srcdir)/tests/test.lg
endif

ucblogo-node.js: $(ucblogo_OBJECTS) pre.js logo-inspect.js liblogo.js
	$(AM_V_CCLD)em++  \
	-o ucblogo-node.js \
	$(ucblogo_OBJECTS) \
	--pre-js pre.js \
	--pre-js logo-inspect.js \
	-s ALLOW_MEMORY_GROWTH \
	--emrun \
	-s ENVIRONMENT=node \
	-s STRICT_JS \
	-s EMIT_PRODUCERS_SECTION \
	-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='$$addOnExit' \
	-sEXPORTED_RUNTIME_METHODS=ccall,setValue,getValue \
	-sEXPORTED_FUNCTIONS=_animationFrameID,_LOGO_GIL,_main,_logo_stop,_logo_pause,_mouse_down,_delayed_keyact,_run_evaluator,\
_inside_gc,\
_in_eval_save,\
_int_during_gc,\
_inside_evaluator,\
_Buttonact,\
_Unbound,\
_hash_table,_segment_list,_regs,_var_stack,_the_generation,_special_chars,\
_logo_gc_debug_segment_list,_logo_gc_debug_segment_next,_logo_gc_debug_segment_size,\
_logo_gc_debug_segment_node,_logo_gc_debug_node_type,_logo_gc_debug_node_car,_logo_gc_debug_node_cdr,\
_logo_gc_debug_node_obj,_logo_gc_debug_node_next,_logo_gc_debug_node_generation,_logo_gc_debug_node_mark,\
_logo_gc_debug_node_id \
	-s NODERAWFS=1 \
	--js-library liblogo.js \
	$(WEB_NODE_DEBUG) $(WEB_LDFLAGS) $(LDFLAGS) $(LIBS)


help/index.json: scripts/build-help.js scripts/build-help-docs.js docs/ucblogo.texi usermanual webedition.md helpfiles/HELPCONTENTS
	$(AM_V_GEN)node scripts/build-help.js help >/dev/null

help/docs.json: help/index.json
	@test -f $@ || node scripts/build-help.js help >/dev/null

# Embeds the installed data tree, so stage it with install-data first.
ucblogo.html: $(ucblogo_OBJECTS) shell.html pre.js logo-inspect.js storage-status.js liblogo.js webstartup.lg web-config.js help/index.json help/docs.json
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -s install-data >/dev/null
	$(AM_V_CCLD)em++  \
	-o ucblogo.html \
	$(ucblogo_OBJECTS) \
	--pre-js web-config.js \
	--pre-js storage-status.js \
	--pre-js pre.js \
	--pre-js logo-inspect.js \
	--embed-file $(DESTDIR)$(pkgdatadir)@/share/ucblogo \
	--embed-file tests \
	--embed-file web.lgo \
	--embed-file help/index.json@/share/ucblogo/help-index.json \
	--embed-file logologo.ico \
	--embed-file webstartup.lg@/share/ucblogo/logolib/startup.lg \
	-s ALLOW_MEMORY_GROWTH \
	-s ENVIRONMENT=web \
	-s STRICT_JS \
	-s EMIT_PRODUCERS_SECTION \
	-sDEFAULT_LIBRARY_FUNCS_TO_INCLUDE='$$addOnExit' \
	--shell-file shell.html \
	-sEXPORTED_RUNTIME_METHODS=ccall,setValue,getValue \
	-sEXPORTED_FUNCTIONS=_animationFrameID,_LOGO_GIL,_main,_logo_stop,_logo_pause,_mouse_down,_delayed_keyact,_run_evaluator,\
_inside_gc,\
_in_eval_save,\
_int_during_gc,\
_inside_evaluator,\
_Buttonact,\
_Unbound,\
_hash_table,_segment_list,_regs,_var_stack,_the_generation,_special_chars,\
_logo_gc_debug_segment_list,_logo_gc_debug_segment_next,_logo_gc_debug_segment_size,\
_logo_gc_debug_segment_node,_logo_gc_debug_node_type,_logo_gc_debug_node_car,_logo_gc_debug_node_cdr,\
_logo_gc_debug_node_obj,_logo_gc_debug_node_next,_logo_gc_debug_node_generation,_logo_gc_debug_node_mark,\
_logo_gc_debug_node_id \
	-lidbfs.js \
	--js-library liblogo.js \
	$(WEB_HTML_DEBUG) $(WEB_LDFLAGS) $(LDFLAGS) $(LIBS)



# ucblogo-obj.o 
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
	mv UCBLogo.app/Contents/Resources/logolib/RENAME-GRAVE-ACCENT 'UCBLogo.app/Contents/Resources/logolib/`'
	mv UCBLogo.app/Contents/Resources/logolib/RENAME-NUMBER-SIGN 'UCBLogo.app/Contents/Resources/logolib/#'
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

CLEANFILES += ucblogo.html ucblogo.js ucblogo.mjs.map ucblogo.mjs ucblogo.js.map ucblogo.wasm.map ucblogo.wasm
CLEANFILES += ucblogo-node.js ucblogo-node.wasm ucblogo-node.wasm.map
CLEANFILES += help/index.json help/docs.json
CLEANFILES += ucblogo.debug.wasm ucblogo-node.debug.wasm web-config.js sw.js sw.js.map
# Hash-named Workbox runtime; the 8-hex pattern excludes the tracked workbox-config.js.
CLEANFILES += workbox-[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f].js workbox-[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f].js.map