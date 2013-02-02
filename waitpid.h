/*
 * waitpid - wait for process termination
 * Copyright (C) 2012, 2013  Andrea Corbellini <corbellini.andrea@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <config.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <alloca.h>
#include <getopt.h>
#include <sched.h>

#ifndef __GNUC__
# define __attribute__(x)
#endif
#define __noreturn __attribute__ ((noreturn))

#ifndef PID_T_MAX
# ifdef PID_MAX
#  define PID_T_MAX PID_MAX
# else
#  define PID_T_MAX (~(1 << (CHAR_BIT * sizeof (pid_t) - 1)))
# endif
#endif

#include "signame.h"
