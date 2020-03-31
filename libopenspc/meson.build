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

libopenspc_inc = include_directories('.')

subdir('SNEeSe')  # defines libspcimpl

libopenspc_lib = shared_library(
    'openspc',
    ['dsp.c', 'main.cc'],
    c_args: ['-Wno-array-bounds'],
    dependencies: [libspcimpl,
                   dependency('zlib', version: '>=1.2.0')],
    install: true,
    version: meson.project_version())

libopenspc = declare_dependency(
    include_directories: libopenspc_inc,
    link_with: libopenspc_lib)

install_headers('openspc.h')

py3 = import('python').find_installation('python3')
openspc_py = files('openspc.py')
py3.install_sources(openspc_py)
