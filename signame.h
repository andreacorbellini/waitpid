static const char *
signame (int signal) {
  switch (signal) {
#ifdef SIGHUP
    case SIGHUP: return "SIGHUP";
#endif
#ifdef SIGINT
    case SIGINT: return "SIGINT";
#endif
#ifdef SIGQUIT
    case SIGQUIT: return "SIGQUIT";
#endif
#ifdef SIGILL
    case SIGILL: return "SIGILL";
#endif
#ifdef SIGTRAP
    case SIGTRAP: return "SIGTRAP";
#endif
#ifdef SIGABRT
    case SIGABRT: return "SIGABRT";
#endif
#ifdef SIGFPE
    case SIGFPE: return "SIGFPE";
#endif
#ifdef SIGKILL
    case SIGKILL: return "SIGKILL";
#endif
#ifdef SIGBUS
    case SIGBUS: return "SIGBUS";
#endif
#ifdef SIGSEGV
    case SIGSEGV: return "SIGSEGV";
#endif
#ifdef SIGPIPE
    case SIGPIPE: return "SIGPIPE";
#endif
#ifdef SIGALRM
    case SIGALRM: return "SIGALRM";
#endif
#ifdef SIGTERM
    case SIGTERM: return "SIGTERM";
#endif
#ifdef SIGURG
    case SIGURG: return "SIGURG";
#endif
#ifdef SIGSTOP
    case SIGSTOP: return "SIGSTOP";
#endif
#ifdef SIGTSTP
    case SIGTSTP: return "SIGTSTP";
#endif
#ifdef SIGCONT
    case SIGCONT: return "SIGCONT";
#endif
#ifdef SIGCHLD
    case SIGCHLD: return "SIGCHLD";
#endif
#ifdef SIGTTIN
    case SIGTTIN: return "SIGTTIN";
#endif
#ifdef SIGTTOU
    case SIGTTOU: return "SIGTTOU";
#endif
#ifdef SIGIO
    case SIGIO: return "SIGIO";
#endif
#ifdef SIGXCPU
    case SIGXCPU: return "SIGXCPU";
#endif
#ifdef SIGXFSZ
    case SIGXFSZ: return "SIGXFSZ";
#endif
#ifdef SIGVTALRM
    case SIGVTALRM: return "SIGVTALRM";
#endif
#ifdef SIGPROF
    case SIGPROF: return "SIGPROF";
#endif
#ifdef SIGWINCH
    case SIGWINCH: return "SIGWINCH";
#endif
#ifdef SIGUSR1
    case SIGUSR1: return "SIGUSR1";
#endif
#ifdef SIGUSR2
    case SIGUSR2: return "SIGUSR2";
#endif
#ifdef SIGEMT
    case SIGEMT: return "SIGEMT";
#endif
#ifdef SIGSYS
    case SIGSYS: return "SIGSYS";
#endif
#ifdef SIGSTKFLT
    case SIGSTKFLT: return "SIGSTKFLT";
#endif
#ifdef SIGINFO
    case SIGINFO: return "SIGINFO";
#endif
#ifdef SIGPWR
    case SIGPWR: return "SIGPWR";
#endif
#ifdef SIGLOST
    case SIGLOST: return "SIGLOST";
#endif
  }

  static char s[200];
  sprintf (s, "signal %d", signal);
  return s;
}
