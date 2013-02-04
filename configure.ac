AC_PREREQ([2.69])
AC_INIT([waitpid], [0.0.1], [https://github.com/andrea-corbellini/waitpid])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([waitpid.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([getopt.h],,[
    echo "getopt.h is required for this program"
    exit 1
])
AC_CHECK_HEADERS([sys/ptrace.h],,[
    echo "sys/ptrace.h is required for this program"
    exit 1
])
AC_CHECK_HEADERS([sys/types.h],,[
    echo "sys/types.h is required for this program"
    exit 1
])
AC_CHECK_HEADERS([sys/wait.h],,[
    echo "sys/wait.h is required for this program"
    exit 1
])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_PID_T

# Checks for library functions.

AC_OUTPUT
