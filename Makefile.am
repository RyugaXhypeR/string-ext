AUTOMAKE_OPTIONS = subdir-objects

lib_LIBRARIES = libstringext.a
libstringext_a_SOURCES = src/string_ext.c src/string_test_utils.c
include_HEADERS = include/string_dbg.h include/string_ext.h include/string_utils.h

D = NDEBUG -g
LINTER_FLAGS = -Wall -Wextra -Wpendantic
OPT_FLAG = -O3
C_FLAGS = $(LINTER_FLAGS) $(OPT_FLAG) -D$(D)
AM_CPPFLAGS = -I$(top_srcdir)/include

# Clean up automake-generated files
clean-local:
	-rm -rf autom4te.cache config.h config.h.in~ config.log config.status compile \
		Makefile.in aclocal.m4 install-sh missing depcomp configure configure\~ ar-lib 

# Make "make distcheck" work with non-GNU tar
DISTCHECK_CONFIGURE_FLAGS = --disable-dependency-tracking

EXTRA_DIST = $(top_srcdir)/include/* $(top_srcdir)/src/*
