#!/usr/bin/env python3

##########################################################################
#
#         Copyright (c) 2003-2020 Brad Martin.
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

import argparse
import math
import os
import random
import select
import signal
import subprocess
import sys

try:
    import openspc
except ImportError:
    # Fixup path if we're not installed to run from the build directory.
    sys.path.append(os.path.join(os.path.dirname(os.path.dirname(__file__)),
                                 'libopenspc'))
    import openspc

_SAMPLE_FREQ = 32000

_BUFS_PER_SEC = 13
_BUF_SAMPLES = _SAMPLE_FREQ // _BUFS_PER_SEC
_BYTES_PER_SAMPLE = 2 * 2
_BUF_BYTES = _BUF_SAMPLES * _BYTES_PER_SAMPLE


def play_file(filename, outfile, mask=None, seconds=None):
    with open(filename, 'rb') as f:
        buf = f.read()
    openspc.init(buf)
    if mask is not None:
        openspc.set_channel_mask(mask)

    buf_limit = None
    if seconds is not None:
        buf_limit = int(math.ceil(seconds * _BUFS_PER_SEC))
    bufs_emitted = 0
    while True:
        if (buf_limit is not None) and (bufs_emitted >= buf_limit):
            break
        if (sys.stdin in select.select([sys.stdin], [], [], 0)[0]) and \
           (sys.stdin.read(1) == '\n'):
            break
        outfile.write(openspc.run(-1, _BUF_BYTES))
        outfile.flush()
        bufs_emitted += 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('-r', '--randomize', action='store_true',
                        help='Randomize song order')
    parser.add_argument('-m', '--mask', type=int,
                        help='Set muted channel bitmask (0-255)')
    parser.add_argument('-s', '--seconds', type=float,
                        help='End playback after this many seconds')
    parser.add_argument('-o', '--output',
                        help='Write to output file, or "-" for stdout')
    parser.add_argument('filename', nargs='+',
                        help='One or more files to play')
    args = parser.parse_args()

    if args.randomize:
        random.shuffle(args.filename)

    if args.output is None:
        aplay = subprocess.Popen(
            ['aplay',
             '-f', 'S16_LE',
             '-c', '2',
             '-r', str(_SAMPLE_FREQ),
             '--buffer-size', str(_BUF_SAMPLES)],
            stdin=subprocess.PIPE, stderr=open(os.devnull, 'w'))
        outfile = aplay.stdin
    elif args.output == '-':
        outfile = sys.stdout
    else:
        outfile = open(args.output, 'wb')

    # You would think try/except on KeyboardInterrupt would be sufficient, but
    # that only seems to work some of the time.  Perhaps it depends on if we
    # are in python or the ctypes lib when it happens.  In any case, this seems
    # to catch the cases that still slip through.
    signal.signal(signal.SIGINT, lambda signal, frame: sys.exit(0))

    print('Press RETURN to change songs', file=sys.stderr)
    for filename in args.filename:
        print("Now playing '%s'..." % filename, file=sys.stderr)
        try:
            play_file(filename, outfile, mask=args.mask, seconds=args.seconds)
        except KeyboardInterrupt:
            sys.exit(0)


if __name__ == '__main__':
    main()
