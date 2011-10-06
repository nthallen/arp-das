#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cmdgen], [1.9], [allen@huarp.harvard.edu])
AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([cmdalgo.h])
AC_CONFIG_HEADER([config.h])

# Defined CPPFLAGS and LDFLAGS if they are not already defined
CPPFLAGS=${CPPFLAGS--I/usr/local/include}
LDFLAGS=${LDFLAGS--L/usr/local/lib}

# Checks for programs.
AC_PROG_CC
AC_PROG_YACC
which ${YACC%% *} 2>/dev/null >/dev/null || AC_MSG_ERROR([YACC is required],1)
AM_PROG_LEX
which ${LEX%% *} 2>/dev/null >/dev/null || AC_MSG_ERROR([LEX is required],1)
AC_CHECK_PROG(HAVE_USEMSG,usemsg,[true])
AM_CONDITIONAL([USE_USEMSG],[test x$HAVE_USEMSG = xtrue ])

# Checks for libraries.
AC_SEARCH_LIBS(compile_init_options,[nort nortlib],[],[AC_MSG_ERROR(nortlib is required)])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])
AC_CHECK_HEADERS([compiler.h nortlib.h],[],[AC_MSG_ERROR([cannot locate required header])])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strdup stricmp strcasecmp])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
