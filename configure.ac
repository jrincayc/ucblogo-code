dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.69])
AC_INIT([ucblogo],[6.2.5-WASM],[],[],[https://github.com/jrincayc/ucblogo-code])
AC_CONFIG_SRCDIR([logodata.c])
AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE([subdir-objects foreign -Wall])
AM_SILENT_RULES([yes])

AC_LANG([C])

dnl Web Edition and runtime-check options come before AC_PROG_CC so they
dnl can choose default CFLAGS.

AC_ARG_ENABLE([wasm],
  [AS_HELP_STRING([--enable-wasm],
    [build the Web Edition with Emscripten; run under emconfigure (default=no)])],,
  [enable_wasm=no])

AC_ARG_WITH([debug-info],
  [AS_HELP_STRING([--with-debug-info=KIND],
    [debug info: no, dwarf, or (Web Edition only) profiling,
     separate-dwarf, source-map (default=no)])],,
  [with_debug_info=no])
AS_CASE([$with_debug_info],
  [yes], [with_debug_info=dwarf],
  [no|dwarf|profiling|separate-dwarf|source-map], [],
  [AC_MSG_ERROR([unknown --with-debug-info=$with_debug_info])])

AC_ARG_ENABLE([asan],
  [AS_HELP_STRING([--enable-asan],[build with AddressSanitizer (default=no)])],,
  [enable_asan=no])
AC_ARG_ENABLE([ubsan],
  [AS_HELP_STRING([--enable-ubsan],
    [build with UndefinedBehaviorSanitizer (default=no)])],,
  [enable_ubsan=no])
AC_ARG_ENABLE([safe-heap],
  [AS_HELP_STRING([--enable-safe-heap],
    [Web Edition: Emscripten SAFE_HEAP memory access checks (default=no)])],,
  [enable_safe_heap=no])
AC_ARG_ENABLE([assertions],
  [AS_HELP_STRING([--enable-assertions@<:@=1|2@:>@],
    [Web Edition: Emscripten runtime ASSERTIONS level (default=no)])],,
  [enable_assertions=no])
AC_ARG_ENABLE([stack-overflow-check],
  [AS_HELP_STRING([--enable-stack-overflow-check@<:@=1|2@:>@],
    [Web Edition: Emscripten STACK_OVERFLOW_CHECK level (default=no)])],,
  [enable_stack_overflow_check=no])
AC_ARG_ENABLE([service-worker],
  [AS_HELP_STRING([--enable-service-worker],
    [Web Edition: generate and register the offline service worker;
     production builds only (default=yes for production builds)])],,
  [enable_service_worker=auto])

AS_CASE([$enable_assertions],
  [yes], [enable_assertions=1],
  [no|1|2], [],
  [AC_MSG_ERROR([--enable-assertions must be 1 or 2])])
AS_CASE([$enable_stack_overflow_check],
  [yes], [enable_stack_overflow_check=1],
  [no|1|2], [],
  [AC_MSG_ERROR([--enable-stack-overflow-check must be 1 or 2])])

AS_IF([test x$enable_wasm != xyes],
 [AS_CASE([$with_debug_info], [profiling|separate-dwarf|source-map],
   [AC_MSG_ERROR([--with-debug-info=$with_debug_info requires --enable-wasm])])
  for opt in safe-heap assertions stack-overflow-check; do
    eval val=\$enable_`echo $opt | tr - _`
    AS_IF([test x$val != xno],
     [AC_MSG_ERROR([--enable-$opt requires --enable-wasm])])
  done
  AS_IF([test x$enable_service_worker = xyes],
   [AC_MSG_ERROR([--enable-service-worker requires --enable-wasm])])])

AS_IF([test x$enable_asan = xyes && test x$enable_safe_heap = xyes],
 [AC_MSG_ERROR([--enable-asan and --enable-safe-heap cannot be combined;
Emscripten's ASan does not work with SAFE_HEAP])])

web_production=yes
AS_IF([test x$with_debug_info != xno || test x$enable_asan != xno ||
       test x$enable_ubsan != xno || test x$enable_safe_heap != xno ||
       test x$enable_assertions != xno ||
       test x$enable_stack_overflow_check != xno],
 [web_production=no])

AS_CASE([$enable_service_worker],
  [auto], [AS_IF([test x$enable_wasm = xyes && test x$web_production = xyes],
            [enable_service_worker=yes], [enable_service_worker=no])],
  [yes], [AS_IF([test x$web_production != xyes],
           [AC_MSG_ERROR([--enable-service-worker is only allowed in production builds
(no --with-debug-info, sanitizers, or Emscripten runtime checks)])])],
  [no], [],
  [AC_MSG_ERROR([bad value for --enable-service-worker: $enable_service_worker])])

AS_IF([test x$enable_wasm = xyes],
 [AS_IF([test "x$prefix" = xNONE], [prefix=`pwd`/dist])
  AS_IF([test x$web_production = xyes || test x$with_debug_info = xprofiling],
   [web_default_opt=-O3], [web_default_opt=-Og])
  : ${CFLAGS="$web_default_opt -std=gnu23 -Wno-comment -Wno-typedef-redefinition"}
  : ${CXXFLAGS="$web_default_opt"}])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
dnl makehelp runs during the build, so it needs a native compiler under emconfigure.
AX_PROG_CC_FOR_BUILD

AS_IF([test x$enable_wasm = xyes],
 [AC_MSG_CHECKING([whether $CC targets Emscripten])
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#ifndef __EMSCRIPTEN__
#error not emscripten
#endif]])],
   [AC_MSG_RESULT([yes])],
   [AC_MSG_RESULT([no])
    AC_MSG_ERROR([--enable-wasm requires Emscripten; run: emconfigure ./configure --enable-wasm])])])

dnl Process Options

AC_MSG_CHECKING([enable_objects])
AC_ARG_ENABLE([objects],
 [AS_HELP_STRING([--enable-objects],
  [enable object oriented logo features (default is no)])],,
 [enable_objects=no])
AC_MSG_RESULT([$enable_objects])
AS_IF([test x${enable_objects} = xyes],
 [AC_DEFINE([OBJECTS],[1],[enable object-oriented features])])
AM_CONDITIONAL([OBJECTS], [test x${enable_objects} = xyes])

AC_MSG_CHECKING([enable_gitid])
AC_ARG_ENABLE([gitid],
 [AS_HELP_STRING([--enable-gitid],
  [enable including git id in version (default is no; yes with --enable-wasm)])],,
  [enable_gitid=$enable_wasm])
AC_MSG_RESULT([$enable_gitid])
AS_IF([test x${enable_gitid} = xyes],
 [AC_DEFINE([GITID],[1],[enable gitid in version])])
AM_CONDITIONAL([GITID], [test x${enable_gitid} = xyes])

AC_MSG_CHECKING([enable_wx])
AC_ARG_ENABLE([wx],
       [AS_HELP_STRING([--enable-wx],[try to use wx library for graphics (default=yes; no with --enable-wasm)])],,
       [AS_IF([test x$enable_wasm = xyes], [enable_wx=no], [enable_wx=yes])])
AC_MSG_RESULT([$enable_wx])
AS_IF([test x$enable_wasm = xyes && test x$enable_wx = xyes],
 [AC_MSG_ERROR([--enable-wx cannot be combined with --enable-wasm])])

AS_IF([test x${enable_wx} = xyes],
[m4_ifdef([WX_CONFIG_CHECK], [
WX_CONFIG_OPTIONS

WX_CONFIG_CHECK([3.0.0], [wxWin=1])

AM_CONDITIONAL([WX], [test x${wxWin} = x1])
AM_COND_IF([WX],
 [AC_DEFINE([HAVE_WX],[1],[Use wx library for GUI])
  AC_MSG_NOTICE([using wx library])
  CC="`$WX_CONFIG_PATH --cc`"
  CXX="`$WX_CONFIG_PATH --cxx`"
  CPPFLAGS="$WX_CPPFLAGS $CPPFLAGS"
  CXXFLAGS="$WX_CXXFLAGS_ONLY $CXXFLAGS"
  CFLAGS="$WX_CFLAGS_ONLY $CFLAGS"
  LIBS="$WX_LIBS $LIBS"])
  ], [
  AC_MSG_ERROR([aclocal did not find wxwin.m4, so wxWidgets cannot be detected.
If wxWidgets is already installed, wxwin.m4 is outside aclocal's search path.
Re-run autoreconf with ACLOCAL_PATH set to the directory holding wxwin.m4.

Or configure with --disable-wx to build without graphics.])
  AM_CONDITIONAL([WX], [test 1 = 0])
])],
 AM_CONDITIONAL([WX], [test 1 = 0]))


AC_MSG_CHECKING([enable_wasm])
AC_MSG_RESULT([$enable_wasm])
AM_CONDITIONAL([WASM], [test x${enable_wasm} = xyes])
AM_COND_IF([WASM],
 [AC_DEFINE([__EMSCRIPTEN__],[1],[Target Web Assembly])],
 [AC_DEFINE([WASM_MISSING],[1],[ix-nay on the asmway])])

AC_MSG_CHECKING([enable_service_worker])
AC_MSG_RESULT([$enable_service_worker])
AM_CONDITIONAL([SERVICE_WORKER], [test x$enable_service_worker = xyes])
AS_IF([test x$enable_service_worker = xyes],
 [WEB_SERVICE_WORKER=true], [WEB_SERVICE_WORKER=false])
AC_SUBST([WEB_SERVICE_WORKER])
AM_CONDITIONAL([SEPARATE_DWARF],
 [test x$enable_wasm = xyes && test x$with_debug_info = xseparate-dwarf])


AC_MSG_CHECKING([enable_x11])
AC_ARG_ENABLE([x11],
  [AS_HELP_STRING([--enable-x11],[use x11Widgets (default=no)])],,
  [enable_x11=no])
AC_MSG_RESULT([$enable_x11])
AS_IF([test x$enable_wasm = xyes && test x$enable_x11 = xyes],
 [AC_MSG_ERROR([--enable-x11 cannot be combined with --enable-wasm])])
AM_CONDITIONAL([X11], [test x${enable_x11} = xyes])
AM_COND_IF([X11],
 [AC_DEFINE([HAVE_X11],[1],[Use x11 library for GUI])],
 [AC_DEFINE([X_DISPLAY_MISSING],[1],[ix-nay on the x11-isplayday])])

AC_MSG_CHECKING([enable_docs])
AC_ARG_ENABLE([docs],
       [AS_HELP_STRING([--enable-docs],[build and install TeXi-format documentation (default=yes; no with --enable-wasm)])],,
       [AS_IF([test x$enable_wasm = xyes], [enable_docs=no], [enable_docs=yes])])
AC_MSG_RESULT([$enable_docs])
AM_CONDITIONAL([DOCS], [test x${enable_docs} = xyes])

dnl The manual is built by makeinfo (info and html) and by texi2dvi,
dnl which drives pdftex for the PDF.  Check for all three: texi2dvi and
dnl makeinfo come from GNU Texinfo, pdftex from a TeX distribution, and
dnl they are packaged separately on most systems.
AS_IF([test x${enable_docs} = xyes],
 [docs_missing=
  AC_CHECK_PROG([HAVE_MAKEINFO], [makeinfo], [yes], [no])
  AS_IF([test x$HAVE_MAKEINFO = xno], [docs_missing="$docs_missing makeinfo"])
  AC_CHECK_PROG([HAVE_TEXI2DVI], [texi2dvi], [yes], [no])
  AS_IF([test x$HAVE_TEXI2DVI = xno], [docs_missing="$docs_missing texi2dvi"])
  AC_CHECK_PROGS([DOCS_PDFTEX], [pdfetex pdftex], [no])
  AS_IF([test x$DOCS_PDFTEX = xno], [docs_missing="$docs_missing pdftex"])
  AS_IF([test -n "$docs_missing"],
   [AC_MSG_ERROR([missing tools required to build the manual:$docs_missing
makeinfo and texi2dvi come from GNU Texinfo; pdftex comes from a TeX
distribution such as TeX Live or MacTeX.
Install them, or pass --disable-docs to skip documentation.])])])

dnl Enable Compiler Warnings
AX_CFLAGS_WARN_ALL
dnl Disable a warning with many false positives; perhaps rethink later
AX_CHECK_COMPILE_FLAG([-Wno-write-strings],
[CFLAGS="$CFLAGS -Wno-write-strings"
CXXFLAGS="$CXXFLAGS -Wno-write-strings"])
dnl Disable voluminous cloud of rather harmless warnings
AX_CHECK_COMPILE_FLAG([-Wno-unused-variable],
[CFLAGS="$CFLAGS -Wno-unused-variable"
CXXFLAGS="$CXXFLAGS -Wno-unused-variable"])

dnl Checks for libraries.
AC_SEARCH_LIBS(sqrt,[m])
AC_SEARCH_LIBS(atan,[m])
AC_SEARCH_LIBS(atan2,[m])
AC_SEARCH_LIBS(hypot,[m])
AC_SEARCH_LIBS(signal, [BSD bsd])

AM_COND_IF([WX],,
 [AC_SEARCH_LIBS(tgetstr, [termcap termlib curses ncurses])])

AM_COND_IF([X11],
 [AC_SEARCH_LIBS(XOpenDisplay, [X11])])

dnl Checks for header files.
AC_CHECK_HEADERS([termios.h termio.h termcap.h termlib.h curses.h sys/ioctl.h unistd.h string.h])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_CHECK_FUNCS([usleep srandom sigsetmask drem irint memcpy gettimeofday tgoto])

AC_SUBST(WXOFILES)
AC_SUBST(WXSRCFILES)
AC_SUBST(TERMOFILE)
AC_SUBST(TERMFILE)
AC_SUBST(LINKER)
AC_SUBST(GRAPHICSOFILE)
AC_SUBST(GRAPHICSFILE)

dnl Debug and sanitizer flags are appended last so they do not disturb the checks above.
sanitize_flags=
AS_IF([test x$enable_asan = xyes], [sanitize_flags="$sanitize_flags -fsanitize=address"])
AS_IF([test x$enable_ubsan = xyes], [sanitize_flags="$sanitize_flags -fsanitize=undefined"])
AS_CASE([$with_debug_info], [dwarf|separate-dwarf|source-map], [debug_cflags=-g], [debug_cflags=])
CFLAGS="$CFLAGS $debug_cflags$sanitize_flags"
CXXFLAGS="$CXXFLAGS $debug_cflags$sanitize_flags"
LDFLAGS="$LDFLAGS$sanitize_flags"

WEB_LDFLAGS=
AS_IF([test x$enable_wasm = xyes],
 [web_link_opt=
  for flag in $CFLAGS; do AS_CASE([$flag], [-O*], [web_link_opt=$flag]); done
  WEB_LDFLAGS="$web_link_opt -ferror-limit=0 -sASYNCIFY -sASYNCIFY_STACK_SIZE=20000000"
  # stops the animation loop on a Wasm exception
  WEB_LDFLAGS="$WEB_LDFLAGS -sABORT_ON_WASM_EXCEPTIONS"
  AS_CASE([$with_debug_info],
   [dwarf], [WEB_LDFLAGS="$WEB_LDFLAGS -g"],
   [profiling], [WEB_LDFLAGS="$WEB_LDFLAGS --profiling-funcs"],
   [source-map], [WEB_LDFLAGS="$WEB_LDFLAGS -gsource-map"])
  AS_IF([test x$enable_safe_heap = xyes], [WEB_LDFLAGS="$WEB_LDFLAGS -sSAFE_HEAP=1"])
  AS_IF([test x$enable_assertions != xno],
   [WEB_LDFLAGS="$WEB_LDFLAGS -sASSERTIONS=$enable_assertions"])
  AS_IF([test x$enable_stack_overflow_check != xno],
   [WEB_LDFLAGS="$WEB_LDFLAGS -sSTACK_OVERFLOW_CHECK=$enable_stack_overflow_check"])])
AC_SUBST([WEB_LDFLAGS])

AC_CONFIG_FILES([Makefile docs/Makefile logolib/Makefile csls/Makefile helpfiles/Makefile tests/Makefile])
AC_OUTPUT

AS_IF([test x$enable_wasm = xyes],
 [AC_MSG_NOTICE([Web Edition: prefix=$prefix debug-info=$with_debug_info service-worker=$enable_service_worker])
  AC_MSG_NOTICE([Web Edition link flags: $WEB_LDFLAGS$sanitize_flags])])
