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

AC_ARG_ENABLE([objects],
[AS_HELP_STRING([--enable-objects],
  [enable object oriented logo features (default is no)])],
  [AC_DEFINE(OBJECTS)])


WXCONFIG=wx-config
AC_ARG_WITH(wx-config,
[[  --with-wx-config=FILE   Use the given path to wx-config when determining
                            wxWidgets configuration; defaults to "wx-config"]],
[
    if test "$withval" != "yes" -a "$withval" != ""; then
        WXCONFIG=$withval
    fi
])

TERMOFILE="term.o"
TERMFILE="term.c"
GRAPHICSOFILE=" xgraphics.o "
GRAPHICSFILE=" xgraphics.c "

# Debian-specific: Pre-define SIGRET as "return 0;"
AC_DEFINE([SIGRET], [return 0;], [Pre-define SIGRET macro.])

AC_MSG_CHECKING([wxWidgets version])
if wxversion=`$WXCONFIG --version`; then
    AC_MSG_RESULT([$wxversion])
    AC_DEFINE([HAVE_WX], [], [Have wxWidgets library])
    WXOFILES="wxMain.o wxTerminal.o wxTurtleGraphics.o  TextEditor.o"
    WXSRCFILES="wxMain.cpp wxTerminal.cpp wxTurtleGraphics.cpp  TextEditor.cpp"
    LIBS="$LIBS "`$WXCONFIG --libs`;
    CC="`$WXCONFIG --cc`"
    CXX="`$WXCONFIG --cxx`"
    CXXFLAGS="$CXXFLAGS `$WXCONFIG --cxxflags`";
#    CFLAGS="$CFLAGS -DHAVE_WX "
    TERMOFILE="wxterm.o"
    TERMFILE="wxterm.c"
    GRAPHICSOFILE=""
    GRAPHICSFILE=""
    LINKER="`$WXCONFIG --ld`"
else
    AC_MSG_RESULT([not found])
    LINKER='$(CC)'
fi


dnl Checks for libraries.

AC_SEARCH_LIBS(sqrt,[m])
AC_SEARCH_LIBS(atan,[m])
dnl AC_SEARCH_LIBS(atan2,[m])
AC_SEARCH_LIBS(hypot,[m])
AC_SEARCH_LIBS(signal, [BSD bsd])

AC_SEARCH_LIBS(tgetstr, [termcap termlib curses ncurses])

AC_SEARCH_LIBS(XOpenDisplay, [X11])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([sgtty.h termio.h unistd.h string.h termcap.h termlib.h curses.h])

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

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
