#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(libopenspc, 0.3.99, ospc@fatlxception.no-ip.org)
AC_CONFIG_SRCDIR([libopenspc/dsp.c])
AC_CONFIG_SRCDIR([examples/ospcplay.c])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_MAKE_SET
AC_PATH_PROG([NASM],[nasm],[missing])
if test "$NASM" = "missing"; then
        AC_MSG_ERROR([unable to find nasm])
fi

# Check libraries.
AC_CHECK_LIB([z],[gzopen],,AC_MSG_ERROR([libopenspc requires zlib]))

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_OFF_T
AC_TYPE_SIZE_T
AM_PROG_CC_C_O

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_CHECK_FUNCS([memset munmap strtol])

AC_CONFIG_FILES([
	libopenspc/Makefile 
	libopenspc/SNEeSe/Makefile
	examples/Makefile
	Makefile])
AC_OUTPUT
