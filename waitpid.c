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

static inline void
vfail (const char *format, va_list ap)
{
  int errsv;

  /* Save the value of `errno'. We will be doing some I/O, thus there is
     probability that `errno' will change. */
  errsv = errno;

  fflush (stdout);

  /* Output an error message in one of the following formats (depending on the
     value of `format' and `errsv'):

     "error: <custom message>\n"
     "error: <error string>\n"
     "error: <custom message>: <error string>\n"
     ""

     The last one is used in case both `format' and `errsv' are zero. */

  if (format) {
    fputs ("error: ", stderr);
    vfprintf (stderr, format, ap);
    if (errsv)
      fprintf (stderr, ": %s\n", strerror (errsv));
    else
      fputc ('\n', stderr);
  }
  else {
    if (errsv)
      fprintf (stderr, "error: %s\n", strerror (errsv));
  }

  fflush (stderr);
}

static inline void __noreturn
fail (const char *format, ...)
{
  va_list ap;

  va_start (ap, format);
  vfail (format, ap);
  va_end (ap);

  exit (EXIT_FAILURE);
}

static inline void
warn (const char *format, ...)
{
  va_list ap;

  va_start (ap, format);
  vfail (format, ap);
  va_end (ap);
}

static void
show_usage (void)
{
  printf ("Usage: %s [OPTION]... PID...\n", program_name);
}

static void __noreturn
fail_invalid_usage (void)
{
  show_usage ();
  printf ("Try `%s --help' for more information.\n", program_name);
  exit (EXIT_FAILURE);
}

static void __noreturn
show_help ()
{
  show_usage ();

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

  exit (EXIT_SUCCESS);
}

static void __noreturn
show_version (void)
{
  puts (PACKAGE_STRING);
  exit (EXIT_SUCCESS);
}

static void
parse_pids (int argc, char **argv, pid_t *pid_list)
{
  int i;

  for (i = 0; (optind + i) < argc; i++) {
    long pid;
    char *end;
    const char *s;

    errno = 0;
    s = argv[optind + i];
    pid = strtol (s, &end, 10);

    if (errno) {
      if (errno != ERANGE)
        fail ("strtoul");
    }
    else {
      if (*end != '\0')
        errno = EINVAL;
      else if (pid < 1 || pid > PID_T_MAX)
        errno = ERANGE;
    }

    if (errno)
      fail ("invalid PID: %s", s);

    pid_list[i] = (pid_t)pid;
  }
}

static int
parse_args (int argc, char **argv, int *force_flag, int *verbose_flag)
{
  int help_flag;
  int version_flag;

  *force_flag = 0;
  help_flag = 0;
  *verbose_flag = 0;
  version_flag = 0;

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
        *force_flag = 1;
        break;
      case 'h':
        help_flag = 1;
        break;
      case 'v':
        *verbose_flag = 1;
        break;
      case 'V':
        version_flag = 1;
        break;
      case '?':
        break;
      default:
        abort ();
    }
  }

  if (help_flag)
    show_help ();

  if (version_flag)
    show_version ();

  int pid_count;

  pid_count = argc - optind;
  if (!pid_count) {
    fprintf (stderr, "%s: expected at least an argument\n", program_name);
    show_usage ();
    printf ("Try `%s --help' for more information.\n", program_name);
    exit (EXIT_FAILURE);
  }

  return pid_count;
}

int
main (int argc, char **argv)
{
  program_name = argv[0];

  int force;
  int verbose;
  int pid_count;
  pid_t *pid_list;

  pid_count = parse_args (argc, argv, &force, &verbose);
  pid_list = alloca (pid_count * sizeof (pid_t));
  parse_pids (argc, argv, pid_list);

  pid_t pid;
  int exit_status;
  int active_proc_count;

  exit_status = 0;
  active_proc_count = 0;

  for (int i = 0; i < pid_count; i++) {
    pid = pid_list[i];

    if (ptrace (PTRACE_ATTACH, pid, NULL, NULL) < 0) {
      void (*f)(const char *, ...);
      f = force ? warn : fail;
      f ("cannot attach to %ld", (long)pid);
      continue;
    }

    if (waitpid (pid, NULL, 0) < 0)
      fail ("waitpid");

    if (ptrace (PTRACE_CONT, pid, NULL, NULL) < 0)
      fail ("ptrace");

    if (verbose)
      printf ("attached to %ld\n", (long)pid);

    active_proc_count++;
  }

  while (active_proc_count > 0) {
    int status;
    int signal;

    if ((pid = wait (&status)) < 0)
      fail ("wait");

    if (verbose && pid_count > 1)
      printf ("[%ld]  ", (long)pid);

    if (WIFEXITED (status)) {
      active_proc_count--;
      signal = 0;
      exit_status = WEXITSTATUS (status);

      if (verbose)
        printf ("exited (status %d)\n", exit_status);
    }
    else if (WIFSIGNALED (status)) {
      active_proc_count--;
      signal = WTERMSIG (status);
      exit_status = signal | 0x80;

      if (verbose)
        printf ("killed by %s (%s)\n", signame (signal), strsignal (signal));
    }
    else if (WIFSTOPPED (status)) {
      signal = WSTOPSIG (status);

      if (verbose)
        printf ("received %s (%s)\n", signame (signal), strsignal (signal));
    }
    else
      abort ();

    if (!WIFEXITED (status) && !WIFSIGNALED (status))
      if (ptrace (PTRACE_CONT, pid, NULL, (void *)(long)signal) < 0)
        fail ("ptrace");
  }

  return exit_status;
}
