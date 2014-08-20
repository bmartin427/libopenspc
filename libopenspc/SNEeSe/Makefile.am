# Copyright (c) 2003-2014 Brad Martin.

# SNEeSe has some warnings I don't intend to resolve.
AM_CFLAGS = -std=c11 -W -Wall -pedantic -Werror \
	    -Wno-unused-parameter -Wno-sign-compare
INCLUDES  = -I..

noinst_LTLIBRARIES = libSPCimpl.la

libSPCimpl_la_SOURCES = \
	SPCimpl.c \
	SPCimpl.h \
	sneese_spc.h \
	spc700.c

EXTRA_DIST = \
	LICENSE
