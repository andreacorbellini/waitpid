bin_PROGRAMS = waitpid
waitpid_SOURCES = waitpid.c signame.h

waitpid_LDADD = $(LIBINTL)

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
ACLOCAL_AMFLAGS = -I m4

dist_man1_MANS = waitpid.1
EXTRA_DIST = config.rpath
