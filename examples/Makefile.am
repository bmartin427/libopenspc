# Copyright (c) 2003-2014 Brad Martin.

AM_CFLAGS = -std=c11 -W -Wall -pedantic -Werror \
            -Wno-unused-result

bin_PROGRAMS = ospcplay
noinst_PROGRAMS = spccomm

AM_CPPFLAGS = -I../libopenspc
LDADD = $(top_builddir)/libopenspc/libopenspc.la
