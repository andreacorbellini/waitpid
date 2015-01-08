# waitpid

This package is a simple utility program traces the execution of the processes
with the given PIDs and exits as soon as all the traced processes have
terminated.

This package comes with two programs: **waitpid** and **waitall**. The first
can be used to wait for the specified PIDs, the latter can be used to wait for
processes with the specified names.

**Examples:**

    $ waitpid -v 5323 5266
    5323: waiting
    5266: waiting
    5323: received SIGINT
    5323: exited with status 0
    5266: received SIGSEGV
    5266: killed by SIGSEGV (core dumped)

    $ waitall -v bash
    27012: waiting
    26911: waiting
    25610: waiting
    5543: waiting
    5485: waiting
    2960: waiting
    2773: waiting
    5485: received SIGCHLD
    5485: received SIGWINCH
    5485: exited with status 2

**Homepage:** https://github.com/andrea-corbellini/waitpid

**Get the source:** `git clone git://github.com/andrea-corbellini/waitpid.git`

**Build it:** `./autogen.sh && ./configure && make`
