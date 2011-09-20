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

# Checks for libraries.
# AC_LANG(C++)

# Add non-empty action-if-found to avoid adding libs to LIBS
# I add the libs explicitly in src/Makefile.am
AC_CHECK_LIB([ph], [PtInit], [ ],
  AC_MSG_ERROR([libph is required]))
AC_CHECK_LIB([nort], [nl_error], [ ],
  AC_MSG_ERROR([libnort is required]))
AC_CHECK_LIB([tm], [tm_dev_name], [ ],
  AC_MSG_ERROR([libtm is required]), [-lnort])
AC_CHECK_LIB([tmph], [nlph_cmdclt_init], [ ],
  AC_MSG_ERROR([libtmph is required]), [-lnort -ltm])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
