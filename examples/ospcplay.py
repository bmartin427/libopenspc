#!/usr/bin/env python

##########################################################################
#
#         Copyright (c) 2003-2014 Brad Martin.
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

"""This script implements an example, rudimentary SPC player.  Output is piped
to the 'aplay' utility by default, but it can optionally be redirected to a
file or to stdout.  Output format is 16-bit stereo 32kHz raw PCM.  Can handle
multiple files on the command line; they can either be played in order or
shuffled.
"""

import math
import optparse
import os
import random
import select
import signal
import subprocess
import sys

import openspc

_SAMPLE_FREQ = 32000

_BUFS_PER_SEC = 13
_BYTES_PER_SEC = _SAMPLE_FREQ * 2 * 2
_BUF_BYTES = _BYTES_PER_SEC / _BUFS_PER_SEC


def play_file(filename, outfile, mask=None, seconds=None):
    with open(filename, 'rb') as f:
        buf = f.read()
    result = openspc.init(buf)
    if result:
        print >>sys.stderr, 'openspc.init returned %d!', result
        return

    if mask is not None:
        openspc.set_channel_mask(mask)

    buf_limit = None
    if seconds is not None:
        buf_limit = math.ceil(seconds * _BUFS_PER_SEC)
    bufs_emitted = 0
    while True:
        if (buf_limit is not None) and (bufs_emitted >= buf_limit):
            break
        if ((sys.stdin in select.select([sys.stdin], [], [], 0)[0]) and
            (sys.stdin.read(1) == '\n')):
            break
        outfile.write(openspc.run(-1, _BUF_BYTES))
        outfile.flush()
        bufs_emitted += 1


def main():
    parser = optparse.OptionParser(
        description=__doc__,
        usage="Usage: %prog [options] <file1> [file2...]")
    parser.add_option('-r', '--randomize', action='store_true',
                      help='Randomize song order')
    parser.add_option('-m', '--mask', type='int',
                      help='Set muted channel bitmask (0-255)')
    parser.add_option('-s', '--seconds', type='float',
                      help='End playback after this many seconds')
    parser.add_option('-o', '--output',
                      help='Write to output file, or "-" for stdout')
    opts, args = parser.parse_args()
    if not args:
        parser.error('Please specify at least one filename to play!')

    if opts.randomize:
        random.shuffle(args)

    if opts.output is None:
        aplay = subprocess.Popen(
            ['aplay',
             '-f', 'S16_LE',
             '-c', '2',
             '-r', str(_SAMPLE_FREQ),
             '--buffer-size', str(_SAMPLE_FREQ / _BUFS_PER_SEC)],
            stdin=subprocess.PIPE, stderr=open(os.devnull, 'w'))
        outfile = aplay.stdin
    elif opts.output == '-':
        outfile = sys.stdout
    else:
        outfile = open(opts.output, 'wb')

    # You would think try/except on KeyboardInterrupt would be sufficient, but
    # that only seems to work some of the time.  Perhaps it depends on if we
    # are in python or the ctypes lib when it happens.  In any case, this seems
    # to catch the cases that still slip through.
    signal.signal(signal.SIGINT, lambda signal, frame: sys.exit(0))

    print >>sys.stderr, 'Press RETURN to change songs'
    for filename in args:
        print >>sys.stderr, "Now playing '%s'..." % filename
        try:
            play_file(filename, outfile, mask=opts.mask, seconds=opts.seconds)
        except KeyboardInterrupt:
            sys.exit(0)

if __name__ == '__main__':
    main()
