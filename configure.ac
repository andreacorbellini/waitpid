#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([waitpid], [0.2], [https://github.com/andrea-corbellini/waitpid])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_CONFIG_SRCDIR([waitpid.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AM_PROG_CC_C_O

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDBOOL
AC_CHECK_HEADER([getopt.h],,[
    AC_MSG_ERROR([unable to find the getopt.h header])
])
AC_CHECK_HEADERS([sys/types.h sys/wait.h])
AC_CHECK_HEADERS([sys/ptrace.h],,,
[#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif])
AS_IF([test "x$ac_cv_header_sys_ptrace_h" == "xyes"],
      [AC_DEFINE([HAVE_PTRACE], [1], [Define if ptrace is available])])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.

AC_OUTPUT
