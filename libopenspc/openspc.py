##########################################################################
#
#         Copyright (c) 2014-2020 Brad Martin.
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
# This file contains python bindings for the libopenspc library.
#
############################################################################

import ctypes


try:
    # If we're installed this should work.
    libopenspc = ctypes.cdll.LoadLibrary("libopenspc.so.0")
except OSError:
    # See if we're running from the build directory.
    import os.path
    lib = os.path.join(os.path.dirname(__file__), ".libs", "libopenspc.so.0")
    if not os.path.exists(lib):
        raise
    libopenspc = ctypes.cdll.LoadLibrary(lib)


SAMPLE_FREQ = 32000       # Hz
BYTES_PER_SAMPLE = 2 * 2  # 16-bit, stereo


def init(buf):
    """Load a new state into the emulator.

    `buf` is a bytes instance containing the image to be loaded.  It can be an
    SPC file, or a ZSNES or Snes9x savestate (autodetected).
    """
    assert isinstance(buf, bytes)
    ret = libopenspc.OSPC_Init(ctypes.c_char_p(buf), ctypes.c_ulong(len(buf)))
    if ret > 0:
        raise ValueError('Unable to recognize supplied file format')
    if ret < 0:
        raise RuntimeError('Error %d from OSPC_Init()' % ret)


def run(s_size, cyc=None):
    """Perform the actual emulation.

    `s_size` is the maximum of bytes (not samples) of audio output to generate.

    `cyc` is an optional limit on the number of CPU cycles to execute.
    Execution will stop when either cyc cycles have been executed, or s_size
    bytes have been output, whichever comes first.

    Returns a bytes instance containing the output data.
    """
    out_buf = bytes(s_size)
    out_size = libopenspc.OSPC_Run(
        ctypes.c_int(cyc if cyc is not None else -1),
        ctypes.c_char_p(out_buf),
        ctypes.c_int(s_size))
    return out_buf[:out_size]


def write_port(port, data):
    """Communicate with the SPC.

    `port` should be in the range 0-3, corresponding to the SPC's four ports
    used to receive input from the CPU, available to read from $F4-$F7 in the
    SPC's address space, and to write from $2140-$2143 in the CPU's address
    space.

    `data` is the 8-bit value you wish to make appear on the specified port
    for the SPC to read.
    """
    # data may be specified either signed or unsigned.
    assert (data >= -128) and (data < 256)
    f = {0: libopenspc.OSPC_WritePort0,
         1: libopenspc.OSPC_WritePort1,
         2: libopenspc.OSPC_WritePort2,
         3: libopenspc.OSPC_WritePort3}.get(port)
    assert f is not None, 'Illegal port %d' % port
    f(ctypes.c_byte(data))


def read_port(port):
    """Receive communications from the SPC.

    `port` should be in the range 0-3, corresponding to the SPC's four ports
    used to send output to the CPU, available to write from $F4-$F7 in the
    SPC's address space, and to read from $2140-$2143 in the CPU's address
    space.

    Returns the data written to the corresponding port by the SPC.  Note that
    these ports are entirely separate from the ports written to with
    write_port().  Data written with write_port() will not be available to be
    read using read_port().  Only that data which the SPC posts on these ports
    is visible from the outside.
    """
    f = {0: libopenspc.OSPC_ReadPort0,
         1: libopenspc.OSPC_ReadPort1,
         2: libopenspc.OSPC_ReadPort2,
         3: libopenspc.OSPC_ReadPort3}.get(port)
    assert f is not None, 'Illegal port %d' % port
    return f()


def set_channel_mask(mask):
    """Selectively disable some DSP channels.

    Channels which have a corresponding bit set in the channel mask will *not*
    be heard.
    """
    libopenspc.OSPC_SetChannelMask(ctypes.c_int(mask))


def get_channel_mask():
    """Retrieve the current channel mask value."""
    return libopenspc.OSPC_GetChannelMask()
