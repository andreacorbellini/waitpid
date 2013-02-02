bin_PROGRAMS = waitpid

waitpid_SOURCES = waitpid.c waitpid.h signame.h
waitpid_CFLAGS = -Wall -Wextra -std=c99 -D_POSIX_C_SOURCE=200809L -D_GNU_SOURCE

dist_man1_MANS = waitpid.1

signame.h: signame.h.pre
	sh -c ./$<

clean-local:
	rm -f signame.h
