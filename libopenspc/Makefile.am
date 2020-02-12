# Copyright (c) 2003-2020 Brad Martin

SUBDIRS = SNEeSe

AM_CFLAGS = -std=c11 -W -Wall -pedantic -Werror \
            -fno-strict-aliasing -Wno-array-bounds
AM_CPPFLAGS = -ISNEeSe

lib_LTLIBRARIES = libopenspc.la
libopenspc_la_LIBADD = -lm -lz SNEeSe/libSPCimpl.la
libopenspc_la_SOURCES = main.c dsp.c dsp.h gauss.h
libopenspc_la_LDFLAGS = -version-info 0:3

include_HEADERS = openspc.h
