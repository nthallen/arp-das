#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(displib, 1.0, [allen@huarp.harvard.edu])
AM_INIT_AUTOMAKE(displib, 1.0)
AC_CONFIG_SRCDIR([src/DCph.cc])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AM_PROG_LIBTOOL
AC_PROG_CXX
AC_PROG_CC

CC=cc
CFLAGS=-lang-c++

# Checks for libraries.
# AC_LANG(C++)
AC_CHECK_LIB([ph], [PtInit])
AC_CHECK_LIB([nort], [nl_error])
AC_CHECK_LIB([tm], [tm_dev_name])
AC_CHECK_LIB([tmph], [nlph_cmdclt_init])
# AC_CHECK_LIB([tmpp], [collector])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
