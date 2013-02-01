/*
 * waitpid - wait for process termination
 * Copyright (C) 2012  Andrea Corbellini <corbellini.andrea@gmail.com>
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

#ifndef GCC
# define __attribute__(x)
#endif

static inline void __attribute__ ((noreturn))
vfail (const char *format, va_list ap)
{
  int errsv;
  const char *errfmt;

  errsv = errno;

  fflush (stdout);

  if (format) {
    fputs ("error: ", stderr);
    vfprintf (stderr, format, ap);
    errfmt = ": %s";
  }
  else {
    errfmt = "error: %s";
  }

  if (errsv)
    fprintf (stderr, errfmt, strerror (errsv));

  fputc ('\n', stderr);
  fflush (stderr);

  exit (CHAR_MAX);
}

static inline void __attribute__ ((noreturn))
fail (const char *format, ...)
{
  va_list ap;

  va_start (ap, format);
  vfail (format, ap);
  /* It does not make any sense to call va_end() given that we will never reach
     this point. */
}

static void
show_usage (const char *program_name)
{
  printf ("Usage: %s [OPTION]... PID", program_name);
  putc ('\n', stdout);
}

static void __attribute__ ((noreturn))
show_help (const char *program_name)
{
  show_usage (program_name);

  puts ("\
Wait for process termination.\n\
\n\
This program will exit as soon as the traced process exits. The exit\n\
status will be the same of the traced process.");

  putc ('\n', stdout);

  puts ("\
Options:\n\
  -h, --help      show this help message and exit\n\
  -v, --version   show version information and exit\n");

  putc ('\n', stdout);

  printf ("Report bugs to %s", PACKAGE_BUGREPORT);

  putc ('\n', stdout);

  exit (0);
}

static void __attribute__ ((noreturn))
show_version (void)
{
  puts (PACKAGE_STRING);
  exit (0);
}

static void __attribute__ ((noreturn))
show_invalid_usage (const char *program_name)
{
  show_usage (program_name);

  printf ("Try `%s --help' for more information.", program_name);
  putc ('\n', stdout);
  fail (NULL);
}

static pid_t
parse_pid_string (const char *str)
{
  long n;
  pid_t pid;
  char *end;

  errno = 0;

  n = strtol (str, &end, 10);
  pid = (pid_t)n;

  if (errno && errno != ERANGE)
    fail ("strtol");

  if (errno == ERANGE || *end != '\0' || n != (long)pid)
    fail ("error: invalid pid: %s\n", str);

  return pid;
}

int
main (int argc, char **argv)
{
  int i;
  pid_t pid;
  int status;
  const char *pid_string;

  for (i = 1; i < argc; i++) {
    if (strcmp (argv[i], "-h") == 0 || strcmp (argv[i], "--help") == 0)
      show_help (argv[0]);

    if (strcmp (argv[i], "-v") == 0 || strcmp (argv[i], "--version") == 0)
      show_version ();
  }

  if (argc != 2) {
    show_usage (argv[0]);
    fail ("expected 2 arguments, got %d", argc);
  }

  pid_string = argv[1];
  pid = parse_pid_string (pid_string);

  if (ptrace (PTRACE_ATTACH, pid, NULL, NULL) < 0)
    err (-1, "error: cannot attach pid %s", pid_string);

  do {
    if (waitpid (pid, &status, 0) < 0)
      err (-1, "error: waitpid() failed");
  } while (!WIFSTOPPED (status));

  if (ptrace (PTRACE_SYSCALL, pid, NULL, NULL) < 0)
    err (-1, "error: ptrace(PTRACE_SYSCALL, ...) failed");

  do {
    if (waitpid (pid, &status, 0) < 0)
      err (-1, "error: waitpid() failed");
  } while (!WIFSTOPPED (status));

  if (ptrace (PTRACE_CONT, pid, NULL, NULL) < 0)
    err (-1, "error: ptrace(PTRACE_CONT, ...) failed");

  do {
    if (waitpid (pid, &status, 0) < 0)
      err (-1, "error: waitpid() failed");
  } while (!WIFEXITED (status));

  exit (WEXITSTATUS (status));
}
