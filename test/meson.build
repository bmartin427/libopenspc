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

# NOTE: This doesn't use the `test()` target, because that:
#  * *Never* runs by default (just invoking `ninja`), and
#  * *Always* runs when `ninja test` or `meson test` are invoked.
#
# Instead, I want something that only runs if it hasn't run successfully since
# its dependencies were last updated, but also runs in that case without the
# user explicitly asking for testing.
custom_target('regression_test.passed',
              build_by_default: true,
              command: [find_program('regression_test.py'),
                        '--libpath', '@INPUT@',
                        '--passed-file', '@OUTPUT@',
                        '--depfile', '@DEPFILE@'],
              depfile: 'regression_test.deps',
              input: [libopenspc_lib],
              output: ['regression_test.passed'])
