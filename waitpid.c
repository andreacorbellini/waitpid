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

#include "waitpid.h"

static const char *program_name;

static bool exit_on_error = true;

static bool verbose = false;

static inline void
__print (FILE *f, const char *prefix, const char *suffix, const char *format, va_list ap)
{
  if (prefix) {
    if (format || suffix)
      fprintf (f, "%s: ", prefix);
    else
      fputs (prefix, f);
  }

  if (format)
    vfprintf (f, format, ap);

  if (suffix) {
    if (format)
      fprintf (f, ": %s\n", suffix);
    else
      fprintf (f, "%s\n", suffix);
  }
  else
    fputc ('\n', f);
}

static inline void
__error (const char *format, va_list ap)
{
  int errsv;

  /* Save the value of `errno'. We will be doing some I/O, thus there is
     probability that `errno' will change. */
  errsv = errno;

  fflush (stdout);

  if (format) {
    if (errsv)
      __print (stderr, "error", strerror (errsv), format, ap);
    else
      __print (stderr, "error", NULL, format, ap);
  }
  else {
    if (errsv)
      __print (stderr, "error", strerror (errsv), NULL, ap);
    else
      abort ();
  }

  fflush (stderr);
}

static inline void
info (int pid_count, pid_t pid, const char *format, ...)
{
  if (!verbose)
    return;

  va_list ap;

  va_start (ap, format);

  if (pid_count == 1)
    __print (stdout, NULL, NULL, format, ap);
  else {
    char prefix[256];
    sprintf (prefix, "%ld", (long)pid);
    __print (stdout, prefix, NULL, format, ap);
  }

  va_end (ap);
}

static inline void
error (const char *format, ...)
{
  va_list ap;

  va_start (ap, format);
  __error (format, ap);
  va_end (ap);

  if (exit_on_error)
    exit (EXIT_FAILURE);
}

static inline void
fatal (const char *format, ...)
{
  va_list ap;

  va_start (ap, format);
  __error (format, ap);
  va_end (ap);

  exit (EXIT_FAILURE);
}

static inline void
print_usage (void)
{
  printf ("Usage: %s [OPTION]... PID...\n", program_name);
}

static inline void
print_invalid_usage (void)
{
  fprintf (stderr, "Try `%s --help' for more information.\n", program_name);
}

static inline void
print_help ()
{
  print_usage ();

  puts ("\
Wait for process termination.\n\
\n\
This program will exit as soon as all the traced\n\
processes exit. The exit status will be the same\n\
of the process that exited last.\n\
\n\
Options:\n\
  -h, --help      show this help message and exit\n\
  --version       show version information and exit\n\
  -f, --force     do not fail if one of the given\n\
                  PIDs cannot be traced\n\
  -v, --verbose   print information on status\n\
                  changes of the processes\n\
\n\
Report bugs to " PACKAGE_BUGREPORT);
}

static inline void
print_version (void)
{
  puts (PACKAGE_STRING);
}

static int
parse_arguments (int argc, char **argv)
{
  bool show_help;
  bool show_version;

  show_help = 0;
  show_version = 0;

  int c;
  int option_index;
  static struct option long_options[] = {
    { "force",    no_argument,  0,  'f' },
    { "help",     no_argument,  0,  'h' },
    { "verbose",  no_argument,  0,  'v' },
    { "version",  no_argument,  0,  'V' },
    { 0,          0,            0,  0   }
  };

  for (;;) {
    c = getopt_long (argc, argv, "fhv", long_options, &option_index);

    if (c == -1)
      break;

    switch (c) {
      case 'f':
        exit_on_error = false;
        break;
      case 'h':
        show_help = true;
        break;
      case 'v':
        verbose = true;
        break;
      case 'V':
        show_version = true;
        break;
      case '?':
        print_invalid_usage ();
        exit (EXIT_FAILURE);
      default:
        abort ();
    }
  }

  if (show_help) {
    print_help ();
    exit (EXIT_SUCCESS);
  }

  if (show_version) {
    print_version ();
    exit (EXIT_SUCCESS);
  }

  int pid_count;

  pid_count = argc - optind;
  if (!pid_count) {
    if (!exit_on_error)
      exit (EXIT_SUCCESS);

    fprintf (stderr, "%s: expected at least an argument\n", program_name);
    print_invalid_usage ();
    exit (EXIT_FAILURE);
  }

  return pid_count;
}

static inline pid_t
parse_pid (const char *s)
{
  long pid;
  char *end;

  errno = 0;
  pid = strtol (s, &end, 10);

  if (errno) {
    if (errno != ERANGE)
      fatal ("strtoul");
  }
  else {
    if (*end != '\0')
      errno = EINVAL;
    else if (pid < 1 || pid > PID_T_MAX)
      errno = ERANGE;
  }

  if (errno)
    fatal ("invalid PID: %s", s);

  return (pid_t)pid;
}

static int
parse_pid_list (int argc, char **argv, pid_t *pid_list)
{
  int i;
  int j;
  pid_t pid;
  int pid_count;

  pid_count = 0;

  for (i = optind; i < argc; i++) {
    pid = parse_pid (argv[i]);

    for (j = 0; j < pid_count; j++) {
      if (pid_list[j] == pid)
        break;
    }

    if (j >= pid_count) {
      pid_list[pid_count] = pid;
      pid_count++;
    }
  }

  return pid_count;
}

int
main (int argc, char **argv)
{
  program_name = argv[0];

  int pid_count;
  pid_t *pid_list;

  pid_count = parse_arguments (argc, argv);
  pid_list = alloca (pid_count * sizeof (pid_t));
  pid_count = parse_pid_list (argc, argv, pid_list);

  pid_t pid;
  int exit_status;
  int active_processes_count;

  exit_status = 0;
  active_processes_count = 0;

  for (int i = 0; i < pid_count; i++) {
    pid = pid_list[i];

    if (ptrace (PTRACE_ATTACH, pid, NULL, NULL) < 0) {
      error ("cannot attach to %ld", (long)pid);
      continue;
    }

    if (waitpid (pid, NULL, 0) < 0)
      fatal ("waitpid failed");

    if (ptrace (PTRACE_CONT, pid, NULL, NULL) < 0)
      fatal ("ptrace failed");

    if (verbose)
      info (pid_count, pid, "process attached");

    active_processes_count++;
  }

  while (active_processes_count > 0) {
    int status;
    int signal;

    if ((pid = wait (&status)) < 0)
      fatal ("wait failed");

    if (WIFEXITED (status)) {
      signal = 0;
      exit_status = WEXITSTATUS (status);
      active_processes_count--;

      if (verbose)
        info (pid_count, pid, "exited with status %d", exit_status);
    }
    else if (WIFSIGNALED (status)) {
      signal = WTERMSIG (status);
      exit_status = signal | 0x80;
      active_processes_count--;

      if (verbose)
        info (pid_count, pid, "killed by %s%s",
              signame (signal), WCOREDUMP (status) ? " (core dumped)" : "");
    }
    else if (WIFSTOPPED (status)) {
      signal = WSTOPSIG (status);

      if (verbose)
        info (pid_count, pid, "received %s: %s",
              signame (signal), strsignal (signal));
    }
    else
      abort ();

    if (!WIFEXITED (status) && !WIFSIGNALED (status))
      if (ptrace (PTRACE_CONT, pid, NULL, (void *)(long)signal) < 0)
        fatal ("ptrace failed");
  }

  return exit_status;
}
