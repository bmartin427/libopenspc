#!/usr/bin/env python3

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

"""Regression test for libopenspc.  A number of test .spc files are executed,
and a checksum is performed on the output waveforms and compared against an
expected value.  This is meant purely to detect if the binary output changes;
there is otherwise no particular guarantee that the expected values are
'correct'.  It is assumed that in the event of this test failing, the test
case will be reviewed manually, with the option of updating the expected value
if the new behavior truly is intended."""

import argparse
import lzma
import hashlib
import os.path
import sys

# This script is intended to be run on the build directory and not on an
# installed copy.
this_dir = os.path.dirname(__file__)
sys.path.append(os.path.join(os.path.dirname(this_dir), 'libopenspc'))

import openspc


RUNTIME_S = 120

# List of test cases.  Each entry in this list is a tuple consisting of:
# * Filename, expected to be found in the data/ subdirectory with a .spc.xz
#   suffix,
# * Expected MD5 of the waveform through RUNTIME_S seconds of playtime.
TESTS = [
    # Random song testing for nothing specific.
    ('basic', '39a192ba5ecc7e5b72a16bb7f92399dc'),
    # Test built-in noise generation.
    ('noise', '017966aa138044486f39842501e1d2b1'),
    # Test pitch modulation.
    ('pitch_mod', '731237f5c50f95c54d071874a5787c40'),
    # Test for noise generated by intentionally making a BRR filter go
    # unstable.
    ('brr_noise', 'd289407278f2788a4a8bd73db7b393b1'),
    # If the BRR filter implementation is incorrect, this song may generate
    # noise where it shouldn't.
    ('brr_no_noise', '6755c5deb6585a09347ee4f5a6626095'),
    # This song changes envelope modes partway through a note, and hence
    # depends on a specific timing relationship between the DSP and the SPC in
    # order to hit the correct envelope levels.
    ('env_timing', '11e10a64915495d50f4eb4a6eaba6045'),
    # Test using direct envelope mode.
    ('direct_env', '130a83c49b3af1d4930cf3cd4b4def4e'),
]


def _select_tests(test_str):
    if test_str is None:
        test_str = '-'
    test_nos = set()

    for test_range in test_str.split(','):
        if '-' in test_range:
            begin, end = test_range.split('-')
            begin = int(begin) if begin else 0
            end = (int(end) + 1) if end else len(TESTS)
            test_nos.update(set(range(begin, end)))
        else:
            test_nos.add(int(test_range))

    return [(i, TESTS[i]) for i in sorted(test_nos)]


def run_test(name, output_dir):
    with lzma.open(os.path.join(this_dir, 'data',
                                name + '.spc.xz')) as spcfile:
        spc_content = spcfile.read()
    openspc.init(spc_content)

    out_file = None
    if output_dir is not None:
        out_file = open(os.path.join(output_dir, '%s.raw' % name), 'wb')

    hasher = hashlib.md5()
    for _ in range(RUNTIME_S):
        data = openspc.run(openspc.SAMPLE_FREQ * openspc.BYTES_PER_SAMPLE)
        hasher.update(data)
        if out_file is not None:
            out_file.write(data)

    if out_file is not None:
        out_file.close()

    return hasher.hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        '-l', '--list', default=False, action='store_true',
        help='List test cases and exit')
    parser.add_argument(
        '-t', '--tests',
        help=('Comma-separated list of test case numbers to run; all tests '
              'are run by default'))
    parser.add_argument(
        '-o', '--output-dir',
        help=('Write raw waveform output for each test case to this directory '
              'to support debugging'))
    parser.add_argument(
        '-v', '--verbose', default=False, action='store_true',
        help='Print detailed test results')
    args = parser.parse_args()

    if args.list:
        for test_no, test in enumerate(TESTS):
            print('%d: %s' % (test_no, test[0]))
        return

    failed = False
    for test_no, (name, expected_md5) in _select_tests(args.tests):
        # TODO(bmartin) Could run these in parallel with multiprocessing.
        actual_md5 = run_test(name, args.output_dir)
        ok = (actual_md5 == expected_md5)
        if args.verbose:
            print('%d (%s): %s' %
                  (test_no, name,
                   'OK' if ok else
                   ('FAILED (expected %r got %r)' %
                    (expected_md5, actual_md5))))
        if not ok:
            print('*** FAILED test %d (%s)' % (test_no, name),
                  file=sys.stderr)
            failed = True

    if failed:
        print('\n*** SOME SPC REGRESSION TESTS FAILED ***', file=sys.stderr)
        sys.exit(1)
    print('\nALL SPC REGRESSION TESTS PASSED')


if __name__ == '__main__':
    main()