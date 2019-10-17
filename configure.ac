dnl Process this file with autoconf to produce a configure script.
AC_PREREQ([2.67])
AC_INIT([ucblogo],[6.1],[who@example.com],,[http://example.com])
AC_CONFIG_SRCDIR([logodata.c])
AC_CONFIG_HEADER(config.h)

AM_INIT_AUTOMAKE([subdir-objects foreign -Wall])

AC_LANG([C])

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX

dnl Process Options

AC_ARG_ENABLE([objects],
[AS_HELP_STRING([--enable-objects],
  [enable object oriented logo features (default is no)])],
  [AC_DEFINE(OBJECTS)])

AM_OPTIONS_WXCONFIG

AC_MSG_CHECKING([enable_wx])
AC_ARG_ENABLE([wx],
  [AS_HELP_STRING([--enable-wx],[use wxWidgets (default=no)])],,
  [enable_wx=no])
AC_MSG_RESULT([$enable_wx])
AM_CONDITIONAL([WX], [test x${enable_wx} = xyes])
AM_COND_IF([WX],
 [AC_DEFINE([HAVE_WX],[1],[Use wx library for GUI])
  AM_PATH_WXCONFIG
  CPPFLAGS="$WX_CPPFLAGS $CPPFLAGS"
  CXXFLAGS="$WX_CXXFLAGS_ONLY $CXXFLAGS"
  CFLAGS="$WX_CFLAGS_ONLY $CFLAGS"
  LIBS="$WX_LIBS $LIBS"])

AC_MSG_CHECKING([enable_x11])
AC_ARG_ENABLE([x11],
  [AS_HELP_STRING([--enable-x11],[use x11Widgets (default=no)])],,
  [enable_x11=no])
AC_MSG_RESULT([$enable_x11])
AM_CONDITIONAL([X11], [test x${enable_x11} = xyes])
AM_COND_IF([X11],
 [AC_DEFINE([HAVE_X11],[1],[Use x11 library for GUI])],
 [AC_DEFINE([X_DISPLAY_MISSING],[1],[ix-nay on the x11-isplayday])])

AC_MSG_CHECKING([enable_docs])
AC_ARG_ENABLE([docs],
       [AS_HELP_STRING([--enable-docs],[build and install TeXi-format documentation (default=yes)])],,
       [enable_docs=yes])
AC_MSG_RESULT([$enable_docs])
AM_CONDITIONAL([DOCS], [test x${enable_docs} = xyes])

dnl Enable Compiler Warnings
AX_CFLAGS_WARN_ALL
AX_CHECK_COMPILE_FLAG([-Wno-write-strings],
[CFLAGS="$CFLAGS -Wno-write-strings"
CXXFLAGS="$CXXFLAGS -Wno-write-strings"])

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
AC_HEADER_STDC
AC_CHECK_HEADERS([unistd.h string.h])
AM_COND_IF([WX],,
 [AC_CHECK_HEADERS([sgtty.h termio.h termcap.h termlib.h curses.h])])

dnl Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_CHECK_FUNCS([usleep srandom sigsetmask matherr drem irint memcpy])
AC_PROG_GCC_TRADITIONAL

AC_SUBST(WXOFILES)
AC_SUBST(WXSRCFILES)
AC_SUBST(TERMOFILE)
AC_SUBST(TERMFILE)
AC_SUBST(LINKER)
AC_SUBST(GRAPHICSOFILE)
AC_SUBST(GRAPHICSFILE)

AC_CONFIG_FILES([Makefile docs/Makefile])
AC_OUTPUT