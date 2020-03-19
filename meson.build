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

project('libopenspc',
        ['c', 'cpp'],
        default_options: ['buildtype=release',
                          'warning_level=3',
                          'werror=true',
                          'c_std=c11',
                          'cpp_std=c++17'],
        license: ['LGPL2', 'CAL'],
        meson_version: '>=0.46',
        version: '0.3.99',
)
subdir('libopenspc')
subdir('test')
subdir('examples')
