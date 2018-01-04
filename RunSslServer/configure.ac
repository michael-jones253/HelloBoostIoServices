#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ssl-hello-server], [1.0], [mikej253@me.com])
AC_CONFIG_SRCDIR([stdafx.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

AM_INIT_AUTOMAKE()
# Checks for libraries.
AC_CHECK_LIB([boost_program_options], [main])
AC_CHECK_LIB([boost_system], [main])
AC_CHECK_LIB([boost_thread], [main])
AC_CHECK_LIB([crypto], [main])
AC_CHECK_LIB([dl], [main])
AC_CHECK_LIB([ioservices], [main])
AC_CHECK_LIB([rt], [main])
AC_CHECK_LIB([ssl], [main])

LT_INIT
AC_PROG_LIBTOOL

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
