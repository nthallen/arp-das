#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# I used autoscan to generate this

AC_PREREQ(2.61)
AC_INIT(tmpplib, 1.0, allen@huarp.harvard.edu)
AC_CONFIG_SRCDIR([src/DC.cc])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdint.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_STRUCT_TM
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([memmove memset strerror])

AC_CONFIG_FILES([src/Makefile])
AC_OUTPUT
