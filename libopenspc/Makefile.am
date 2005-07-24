SUBDIRS = SNEeSe

AM_CFLAGS = -W -Wall -pedantic -Wno-long-long
INCLUDES  = -ISNEeSe

lib_LTLIBRARIES = libopenspc.la
libopenspc_la_LIBADD = -lm -lz SNEeSe/libSPCimpl.la
libopenspc_la_SOURCES = main.c dsp.c dsp.h gauss.h 
libopenspc_la_LDFLAGS = -version-info 0:3

include_HEADERS = openspc.h
