/************************************************************************

        Copyright (c) 2003-2020 Brad Martin.

This file is part of OpenSPC.

OpenSPC is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

OpenSPC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with OpenSPC; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



main.cc: implements functions intended for external use of the libopenspc
library.

 ************************************************************************/

#include "openspc.h"

#include <cstdint>
#include <cstring>
#include <memory>

#include "dsp.h"
#include "spc_cpu.h"

namespace {

/// Class to maintain the context for one SPC simulation.
// TODO(bmartin) Currently this class operates on some global state, which
// means no more than one of these is allowed to exist in a process.
// Eventually this should change.
class SpcContext {
 public:
  SpcContext() {
    channel_mask = 0;
    DSP_Reset();
  }

  /// Loads a savestate from file content in memory.  State file format is
  /// auto-detected.
  ///
  /// @param buf points to the file content already in memory.
  /// @param size is the number of bytes in the file.
  /// @param clear_echo if true, the echo region in SPC RAM will be cleared
  ///        after state loading.  This is in order to support state files
  ///        created with emulators that don't properly support echo, which
  ///        tend to leave garbage in this area.
  /// @return true if successful.
  bool Load(const uint8_t* buf, size_t size, bool clear_echo = true) {
    bool success = LoadSpc(buf, size);
    if (!success) {
      success = LoadZst(buf, size);
    }
    // New file formats could go on from here.

    if (success && clear_echo) {
      const int start = DSPregs[0x6D] << 8;
      int len = DSPregs[0x7D] << 11;
      if (start + len > 0x10000) {
        len = 0x10000 - start;
        // TODO(bmartin) Does this wrap around?  Do we need to clear at the
        // beginning of memory too?
      }
      std::memset(&spc_cpu_.ram()[start], 0, len);
    }

    return success;
  }

  /// Run the emulation, emitting sound samples to the given buffer, until
  /// either the given cycle limit is reached, or the given amount of buffer
  /// space is filled, whichever comes first.
  ///
  /// @param cycle_limit the maximum number of SPC CPU cycles to run.  If
  ///        negative, no limit is imposed.
  /// @param buf a pointer to a buffer to mix sound data into.  May be null.
  /// @param buf_size the size of the given buffer in bytes.  Ignored if buf
  ///        is null.
  /// @return the number of bytes that were either written to the buffer, or
  ///         would have been had @p buf not been null.
  int Run(int cycle_limit, int16_t* buf, size_t buf_size) {
    static constexpr int kWordsPerSample = 2;
    static constexpr int kBytesPerSample = kWordsPerSample * sizeof(*buf);

    const int buf_cycles = (buf_size / kBytesPerSample) * TS_CYC + mix_left_;
    const int buf_inc = buf ? kWordsPerSample : 0;

    if ((cycle_limit < 0) || (buf && (cycle_limit >= buf_cycles))) {
      // Buffer size is the limiting factor.
      buf_size &= ~(kBytesPerSample - 1);
      if (mix_left_) {
        spc_cpu_.Run(mix_left_);
      }
      for (size_t i = 0; i < buf_size; i += kBytesPerSample, buf += buf_inc) {
        DSP_Update(buf);
        spc_cpu_.Run(TS_CYC);
      }
      mix_left_ = 0;
      return buf_size;
    }

    // Otherwise, use the cycle limit.
    if (cycle_limit < mix_left_) {
      spc_cpu_.Run(cycle_limit);
      mix_left_ -= cycle_limit;
      return 0;
    }
    if (mix_left_) {
      spc_cpu_.Run(mix_left_);
      cycle_limit -= mix_left_;
    }
    int samples_written = 0;
    for (; cycle_limit >= TS_CYC; cycle_limit -= TS_CYC) {
      DSP_Update(buf);
      spc_cpu_.Run(TS_CYC);
      ++samples_written;
      buf += buf_inc;
    }
    if (cycle_limit) {
      DSP_Update(buf);
      spc_cpu_.Run(cycle_limit);
      mix_left_ = TS_CYC - cycle_limit;
      ++samples_written;
    }
    return kBytesPerSample * samples_written;
  }

  /// Perform a write to one of the SPC-CPU's four incoming communication
  /// ports, as if the SNES-CPU had written to the SPC.
  void WritePort(int index, uint8_t data) { spc_cpu_.WritePort(index, data); }

  /// Perform a read from one of the SPC-CPU's four outgoing communication
  /// ports, as if the SNES-CPU had read from the SPC.
  uint8_t ReadPort(int index) { return spc_cpu_.ReadPort(index); }

 private:
  /// Loads .spc file content into the simulation.
  ///
  /// @param buf points to the file content already in memory.
  /// @param size is the number of bytes in the file.
  /// @return true if successful.
  bool LoadSpc(const uint8_t* buf, size_t size) {
    static constexpr uint8_t kIdentStr[] = "SNES-SPC700 Sound File Data";
    enum {
      kIdentLen = 37,
      kPcOffset = kIdentLen,
      kPcLen = 2,
      kAOffset = kPcOffset + kPcLen,
      kXOffset,
      kYOffset,
      kPSWOffset,
      kSPOffset,
      kJunkOffset,
      kJunkLen = 212,
      kRamOffset = kJunkOffset + kJunkLen,
      kRamLen = 65536,
      kDspOffset = kRamOffset + kRamLen,
      kDspLen = 128,
      kSpcFileLen = kDspOffset + kDspLen,
    };

    if ((size < kSpcFileLen) ||
        (std::memcmp(buf, kIdentStr, sizeof(kIdentStr) - 1) != 0)) {
      return false;
    }
    spc_cpu_.SetState(buf[kPcOffset] + (buf[kPcOffset + 1] << 8), buf[kAOffset],
                      buf[kXOffset], buf[kYOffset], buf[kPSWOffset],
                      0x100 + buf[kSPOffset], &buf[kRamOffset]);
    std::memcpy(DSPregs, &buf[kDspOffset], kDspLen);

    return true;
  }

  /// Loads .zst file content into the simulation.
  ///
  /// @param buf points to the file content already in memory.
  /// @param size is the number of bytes in the file.
  /// @return true if successful.
  bool LoadZst(const uint8_t* buf, size_t size) {
    static constexpr uint8_t kIdentStr[] = "ZSNES Save State File";
    enum {
      kIdentLen = 26,
      kRamOffset = kIdentLen + 199673,
      kRamLen = 65536,
      kPcOffset = kRamOffset + kRamLen + 16,
      kRegLen = 4,
      kAOffset = kPcOffset + kRegLen,
      kXOffset = kAOffset + kRegLen,
      kYOffset = kXOffset + kRegLen,
      kPSWOffset = kYOffset + kRegLen,
      kPSW2Offset = kPSWOffset + kRegLen,
      kSPOffset = kPSW2Offset + kRegLen,
      kVOnOffset = kSPOffset + kRegLen + 420,
      kVOnLen = 8,
      kDspOffset = kVOnOffset + kVOnLen + 916,
      kDspLen = 256,
      kZstFileLen = kDspOffset + kDspLen,
    };

    if ((size < kZstFileLen) ||
        (std::memcmp(buf, kIdentStr, sizeof(kIdentStr) - 1) != 0)) {
      return false;
    }
    // ZSNES stores the processor status word in a hyper-optimized (read:
    // awkward) way.
    uint8_t psw = buf[kPSWOffset];
    static constexpr uint8_t kZeroFlag = 0x02;
    static constexpr uint8_t kNegFlag = 0x80;
    if ((buf[kPSW2Offset] | buf[kPSW2Offset + 1] | buf[kPSW2Offset + 2] |
         buf[kPSW2Offset + 3]) == 0) {
      psw |= kZeroFlag;
    } else {
      psw &= ~kZeroFlag;
    }
    if (buf[kPSW2Offset] & kNegFlag) {
      psw |= kNegFlag;
    } else {
      psw &= ~kNegFlag;
    }

    spc_cpu_.SetState(buf[kPcOffset] + (buf[kPcOffset + 1] << 8), buf[kAOffset],
                      buf[kXOffset], buf[kYOffset], psw, 0x100 + buf[kSPOffset],
                      &buf[kRamOffset]);
    std::memcpy(DSPregs, &buf[kDspOffset], kDspLen);
    // This is a hack to turn on voices that were already on when the state
    // was saved.  This doesn't restore the entire state of the voice, it just
    // starts it over from the beginning.
    for (int v = 0; v < 8; ++v) {
      if (buf[kVOnOffset + v]) {
        DSPregs[0x4C] |= 1 << v;
      }
    }
    return true;
  }

  openspc::SpcCpu spc_cpu_;
  // Number of SPC CPU cycles remaining until the next DSP sample is due, for
  // use in between calls to Run().
  int mix_left_ = 0;
};

// TODO(bmartin) Eliminate this singleton once the context dependencies on
// global state are eliminated, and the API is modified to specify a context.
std::unique_ptr<SpcContext> g_spc_context;

}  // namespace

// Exported library interfaces

extern "C" int OSPC_Init(void *buf, size_t size) {
  g_spc_context = std::make_unique<SpcContext>();
  return g_spc_context->Load(reinterpret_cast<uint8_t*>(buf), size) ? 0 : 1;
}

extern "C" int OSPC_Run(int cyc, short *s_buf, int s_size) {
  return g_spc_context->Run(cyc, s_buf, s_size);
}

extern "C" void OSPC_WritePort0(char data) {
  g_spc_context->WritePort(0, data);
}

extern "C" void OSPC_WritePort1(char data) {
  g_spc_context->WritePort(1, data);
}

extern "C" void OSPC_WritePort2(char data) {
  g_spc_context->WritePort(2, data);
}

extern "C" void OSPC_WritePort3(char data) {
  g_spc_context->WritePort(3, data);
}

extern "C" char OSPC_ReadPort0(void) { return g_spc_context->ReadPort(0); }
extern "C" char OSPC_ReadPort1(void) { return g_spc_context->ReadPort(1); }
extern "C" char OSPC_ReadPort2(void) { return g_spc_context->ReadPort(2); }
extern "C" char OSPC_ReadPort3(void) { return g_spc_context->ReadPort(3); }

extern "C" void OSPC_SetChannelMask(int mask) { channel_mask = mask; }
extern "C" int OSPC_GetChannelMask(void) { return channel_mask; }
