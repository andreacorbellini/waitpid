/* waitpid -- wait for process termination
   Copyright (C) 2012-2014 Andrea Corbellini <corbellini.andrea@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <config.h>

#include <sys/types.h>

#include <errno.h>
#include <getopt.h>
#include <libintl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifdef HAVE_SYS_PTRACE_H
# include <sys/ptrace.h>
#endif

#ifdef HAVE_PTRACE
# include <signame.h>
#endif

#define _(s) (s)

/* POSIX states that pid_t is a signed integer type of size no
   greater than the size of long. This is why we are defining
   PID_T_MAX in this way and is also why we will print PIDs
   using %ld.  */
#define PID_T_MAX (~((pid_t)1 << (sizeof (pid_t) * CHAR_BIT - 1)))

/* The value of argv[0].  */
static const char *program_name;

static struct option const long_options[] =
{
  {"force", no_argument, NULL, 'f'},
  {"sleep-interval", required_argument, NULL, 's'},
  {"verbose", no_argument, NULL, 'v'},
  {"help", no_argument, NULL, 'h'},
  {"version", no_argument, NULL, 'V'}
};

/* Whether --force was specified or not.  */
static bool allow_invalid_pids;

/* The value of --sleep-interval.  */
static double sleep_interval;
#define DEFAULT_SLEEP_INTERVAL .5

/* The value of --verbose.  */
static bool verbose;

/* The list of PIDs specified on the command line.  */
static pid_t *pid_list;
static size_t pid_list_size;

/* The number of processes that are still alive. It is initially
   set by either ptrace_visit() or kill_visit() and used by
   ptrace_wait() and kill_wait().  */
static size_t active_pid_count;

static void
print_usage (int status)
{
  if (status != EXIT_SUCCESS)
    fprintf (stderr, _("Try '%s --help' for more information.\n"),
             program_name);
  else
    {
      printf (_("Usage: %s [OPTION]... PID...\n"), program_name);

      puts (_("Wait until all the specified processes have exited.\n"));

      puts (_("\
      -f, --force     do not fail if one of the PID specified does not\n\
                      correspond to a running process"));
      printf(_("\
      -s, --sleep-interval=N\n\
                      when ptrace(2) is not available, check for the\n\
                      existence of the processes every N seconds\n\
                      (default: %.1f)\n"),
             DEFAULT_SLEEP_INTERVAL);
      puts (_("\
      -v, --verbose   display a message on the standard output everytime a\n\
                      process exits or receives a signal\n\
      -h, --help      display this help and exit\n\
          --version   output version information and exit"));

      puts (_("\
\n\
When possible, this program will use the ptrace(2) system call to wait for\n\
programs. With ptrace(2) the '--sleep-interval' option is ignored, as events\n\
are reported immediately. Additionally, if '--verbose' is specified, the\n\
program will display exit statuses and signals received by the processes.\n\
\n\
If ptrace(2) is not available, processes are checked periodically,\n\
'--sleep-interval' is not ignored and '--verbose' does not report detailed\n\
information about exit statuses and signals received."));
    }

  exit (status);
}

static void
print_version (int status)
{
  puts (PACKAGE_STRING);
  exit (status);
}

static void
parse_options (int argc, char **argv)
{
  int c;
  char *end;

  program_name = argv[0];
  allow_invalid_pids = false;
  sleep_interval = DEFAULT_SLEEP_INTERVAL;
  verbose = false;

  for (;;)
    {
      c = getopt_long (argc, argv, "fs:vh", long_options, NULL);

      if (c == -1)
        break;

      switch (c)
        {
          case 'f': /* --force */
            allow_invalid_pids = true;
            break;

          case 's': /* --sleep-interval */
            errno = 0;
            sleep_interval = strtod (optarg, &end);
            if (optarg[0] == '\0' || *end != '\0' || errno != 0)
              {
                fprintf (stderr, _("%s: %s: invalid number of seconds\n"),
                         program_name, optarg);
                exit (EXIT_FAILURE);
              }
            break;

          case 'v': /* --verbose */
            verbose = true;
            break;

          case 'h': /* --help */
            print_usage (EXIT_SUCCESS);

          case 'V': /* --version */
            print_version (EXIT_SUCCESS);

          case '?':
            print_usage (EXIT_FAILURE);

          default:
            abort ();
        }
    }

  if (optind >= argc)
    {
      if (allow_invalid_pids)
        {
          pid_list_size = 0;
          return;
        }
      else
        {
          fprintf (stderr, _("%s: missing PID\n"), program_name);
          print_usage (EXIT_FAILURE);
        }
    }

  pid_list_size = (size_t)(argc - optind);
  pid_list = calloc (pid_list_size, sizeof (pid_t));

  if (pid_list == NULL)
    {
      fprintf (stderr, _("%s: cannot allocate memory: %s\n"),
               program_name, strerror (errno));
      exit (EXIT_FAILURE);
    }

  pid_t self;

  self = getpid ();

  for (int i = optind; i < argc; i++)
    {
      /* pid_t is signed, but negative PIDs are not valid
         process identifiers, hence we use unsigned long and
         strtoul(). */
      unsigned long x;

      errno = 0;
      x = strtoul (argv[i], &end, 10);

      if (argv[i][0] == '\0' || *end != '\0' || errno != 0 || x > PID_T_MAX)
        {
          fprintf (stderr, _("%s: %s: invalid PID\n"),
                   program_name, argv[i]);
          exit (EXIT_FAILURE);
        }

      if (x == self)
        {
          fprintf (stderr, _("%s: %s: refusing to trace self\n"),
                   program_name, argv[i]);
          if  (!allow_invalid_pids)
            exit (EXIT_FAILURE);
          x = -1;
        }

      pid_list[i - optind] = (pid_t)x;
    }
}

static int
ptrace_visit (void)
{
#ifdef HAVE_PTRACE
  pid_t pid;

  active_pid_count = 0;

  for (size_t i = 0; i < pid_list_size; i++)
    {
      pid = pid_list[i];

      if (pid < 0)
        continue;

      if (ptrace (PTRACE_SEIZE, pid, NULL, NULL) < 0)
        {
          if (errno == EPERM)
            {
              /* We can't ptrace() one or more processes; detach from all the
                 traced processes.  */
              for (size_t j = 0; j < i; j++)
                {
                  pid = pid_list[j];

                  if (pid < 0)
                    continue;

                  if (ptrace (PTRACE_INTERRUPT, pid, NULL, NULL) < 0
                      || waitpid (pid, NULL, 0) < 0
                      || ptrace (PTRACE_DETACH, pid, NULL, NULL) < 0)
                    {
                      /* ESRCH (No such process) may be returned when doing
                         PTRACE_DETACH if the process exited in the meantime. */
                      if (errno != ESRCH)
                        {
                          fprintf (stderr, _("%s: %ld: cannot detach from process: %s\n"),
                                   program_name, (long)pid, strerror (errno));
                          exit (EXIT_FAILURE);
                        }
                    }
                }

              /* Tell main() to use the kill() implementation.  */
              return -1;
            }
          else if (errno == ESRCH)
            {
              fprintf (stderr, _("%s: %ld: no such process\n"),
                       program_name, (long)pid);
              if  (!allow_invalid_pids)
                exit (EXIT_FAILURE);
              pid_list[i] = -1;
              continue;
            }
          else
            {
              fprintf (stderr, _("%s: %ld: cannot attach to process: %s\n"),
                       program_name, (long)pid, strerror (errno));
              exit (EXIT_FAILURE);
            }
        }

      if (verbose)
        {
          printf (_("%ld: waiting\n"), (long)pid);
          fflush (stdout);
        }

      active_pid_count++;
    }

  return 0;
#else /* HAVE_PTRACE */
  return -1;
#endif /* HAVE_PTRACE */
}

static void
ptrace_wait (void)
{
#ifdef HAVE_PTRACE
  pid_t pid;
  int status;

  while (active_pid_count)
    {
      pid = wait (&status);

      if (pid < 0)
        {
          fprintf (stderr, _("%s: cannot wait: %s\n"),
                   program_name, strerror (errno));
          exit (EXIT_FAILURE);
        }

      if (WIFEXITED (status))
        {
          if (verbose)
            {
              printf (_("%ld: exited with status %d\n"),
                      (long)pid, WEXITSTATUS (status));
              fflush (stdout);
            }
          active_pid_count--;
        }
      else if (WIFSIGNALED (status))
        {
          if (verbose)
            {
              printf (
# ifdef WCOREDUMP
                      WCOREDUMP (status)
                      ? _("%ld: killed by %s (core dumped)\n")
                      :
# endif
                      _("%ld: killed by %s\n"),
                      (long)pid, signame (WTERMSIG (status)));
              fflush (stdout);
            }
          active_pid_count--;
        }
      else if (WIFSTOPPED (status))
        {
          if (verbose)
            {
              printf (_("%ld: received %s\n"),
                      (long)pid, signame (WSTOPSIG (status)));
              fflush (stdout);
            }

          if (ptrace (PTRACE_CONT, pid, NULL, (void *)(long)WSTOPSIG (status)) < 0)
            {
              fprintf (stderr, _("%s: %ld: cannot restart process: %s\n"),
                       program_name, (long)pid, strerror (errno));
              exit (EXIT_FAILURE);
            }
        }
      else
        abort ();
    }
#else /* HAVE_PTRACE */
  abort ();
#endif /* HAVE_PTRACE */
}

static void
kill_visit (void)
{
  pid_t pid;

  active_pid_count = 0;

  for (size_t i = 0; i < pid_list_size; i++)
    {
      pid = pid_list[i];

      if (pid < 0)
        continue;

      if (kill (pid, 0) < 0 && errno != EPERM)
        {
          fprintf (stderr, _("%s: %ld: no such process\n"),
          program_name, (long)pid);
          if  (!allow_invalid_pids)
            exit (EXIT_FAILURE);
          pid_list[i] = -1;
        }
      else
        {
          if (verbose)
            {
              printf (_("%ld: waiting\n"), (long)pid);
              fflush (stdout);
            }
          active_pid_count++;
        }
    }
}

static void
kill_wait (void)
{
  pid_t pid;
  struct timespec ts;

  ts.tv_sec = (time_t)sleep_interval;
  ts.tv_nsec = (sleep_interval - (double)ts.tv_sec) * 1000000.;

  while (active_pid_count)
    {
      if (nanosleep (&ts, NULL) < 0 && errno != EINTR)
        {
          fprintf (stderr, _("%s: cannot sleep: %s\n"),
                   program_name, strerror (errno));
          exit (EXIT_FAILURE);
        }

      for (size_t i = 0; i < pid_list_size; i++)
        {
          pid = pid_list[i];

          if (pid < 0)
            continue;

          if (kill (pid, 0) < 0 && errno != EPERM)
            {
              if (verbose)
                {
                  printf (_("%ld: exited\n"), (long)pid);
                  fflush (stdout);
                }
              active_pid_count--;
              pid_list[i] = -1;
            }
        }
    }
}

int
main (int argc, char **argv)
{
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  parse_options (argc, argv);

  if (ptrace_visit () == 0)
    {
      ptrace_wait ();
    }
  else
    {
      if (verbose)
        {
          fprintf (stderr, _("%s: unable to trace processes\n"), program_name);
        }
      kill_visit ();
      kill_wait ();
    }

  exit (EXIT_SUCCESS);
}
