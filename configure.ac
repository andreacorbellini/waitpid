AC_PREREQ([2.69])
AC_INIT([waitpid], [0.0.1], [https://github.com/andrea-corbellini/waitpid])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([waitpid.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([err.h],,[
    echo "err.h is required for this program"
    exit 1
])
AC_CHECK_HEADERS([errno.h],,[
    echo "errno.h is required for this program"
    exit 1
])
AC_CHECK_HEADERS([inttypes.h],,[
    echo "inttypes.h is required for this program"
    exit 1
])
AC_CHECK_HEADERS([stdio.h],,[
    echo "stdio.h is required for this program"
    exit 1
])
AC_CHECK_HEADERS([stdlib.h],,[
    echo "stdlib.h is required for this program"
    exit 1
])
AC_CHECK_HEADERS([string.h],,[
    echo "string.h is required for this program"
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
AC_CHECK_FUNCS([strtol],,[
    echo "strtol() is required for this program"
    exit 1
])

AC_OUTPUT
