##########################################################################
#
#         Copyright (c) 2020 Brad Martin.
#
# This file is part of OpenSPC.
#
# OpenSPC is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# OpenSPC is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with OpenSPC; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
############################################################################

libspcimpl_lib = static_library(
    'spcimpl',
    ['sneese_spc.c', 'spc700.c', 'spc_cpu.cc'],
    # SNEeSe has some warnings I don't intend to resolve.
    c_args: ['-Wno-array-bounds',
             '-Wno-implicit-fallthrough',
             '-Wno-sign-compare',
             '-Wno-unused-parameter'],
    include_directories: libopenspc_inc,
    pic: true)

libspcimpl = declare_dependency(
    include_directories: include_directories('.'),
    link_with: libspcimpl_lib)
