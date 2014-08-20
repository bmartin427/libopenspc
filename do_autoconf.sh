#!/bin/bash
# Copyright (c) 2014 Brad Martin.

set -e
libtoolize
aclocal
autoheader
automake --add-missing --foreign
autoconf
