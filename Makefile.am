bin_PROGRAMS = waitpid
waitpid_SOURCES = waitpid.c signame.h

ACLOCAL_AMFLAGS = -I m4

dist_bin_SCRIPTS = waitall
dist_man1_MANS = waitpid.1 waitall.1
EXTRA_DIST = waitall.in

# The following has been inspired by the snippet in this page:
# https://www.gnu.org/savannah-checkouts/gnu/autoconf/manual/autoconf-2.69/html_node/Installation-Directory-Variables.html

waitall: Makefile
	rm -f $@
	srcdir=''; \
	  test -f ./$@.in || srcdir=$(srcdir)/; \
	  sed -e 's|@bindir[@]|$(bindir)|g; s|@PACKAGE_STRING[@]|$(PACKAGE_STRING)|g' $${srcdir}$@.in >$@
	chmod +x $@

waitall: $(srcdir)/waitall.in
