#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([io-services], [2.0], [mikej253@me.com])
AC_CONFIG_SRCDIR([TcpServer.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

AM_INIT_AUTOMAKE()

# Checks for libraries.
# ranlib obsoleted by libtool
# AC_PROG_RANLIB
# instruct libtool to create both dynamic and static libraries
LT_INIT
AC_PROG_LIBTOOL

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_CHECK_FUNCS([socket])

AC_DEFINE([IO_SERVICES_HAS_LOCAL_SOCKETS], [], [supports Unix sockets])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
