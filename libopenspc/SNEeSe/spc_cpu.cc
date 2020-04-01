/**************************************************************************

        Copyright (c) 2003-2020 Brad Martin.
        Some portions copyright (c) 1998-2005 Charles Bilyue'.

This file is part of OpenSPC.

spc_cpu.cc: This file is a bridge between the OpenSPC library and the specific
SPC core implementation (in this case, SNEeSe's).  As the licensing rights for
SNEeSe are different from the rest of OpenSPC, none of the files in this
directory are LGPL.  Although this file was created by me (Brad Martin), it
contains some code derived from SNEeSe and therefore falls under its license.
See the file 'LICENSE' in this directory for more information.

 **************************************************************************/

#include "spc_cpu.h"

#include <cstring>

#include "SNEeSe/sneese_spc.h"

namespace openspc {

class SpcCpu::Impl {
 public:
  Impl() {
    Reset_SPC();
    SPC_CPU_cycle_multiplicand = 1;
    SPC_CPU_cycle_divisor = 1;
    SPC_CPU_cycles_mul = 0;
  }

  void SetState(uint16_t pc, uint8_t a, uint8_t x, uint8_t y, uint8_t psw,
                uint8_t sp, const uint8_t* ram) {
    std::memcpy(SPCRAM, ram, kRamSize);

    // Initialize the state of the 0xFFC0 ROM being switched in or out.
    if (!(SPC_CTRL & 0x80)) {
      active_context->FFC0_Address = SPCRAM;
    }

    // Initialize SPC timers to the values the saved RAM indicates were active.
    for (int i = 0; i < 3; ++i) {
      active_context->timers[i].target =
          static_cast<uint8_t>(SPCRAM[0xFA + i] - 1) + 1;
      active_context->timers[i].counter = SPCRAM[0xFD + i] & 0xF;
    }

    // Initialize SPC <-> CPU communications registers to the values the saved
    // RAM indicates were active.
    for (int i = 0; i < 4; ++i) {
      active_context->PORT_R[i] = SPCRAM[0xF4 + i];
    }

    // Initialize SPC registers and associated values.
    active_context->PC.w = pc;
    active_context->YA.b.l = a;
    active_context->X = x;
    active_context->YA.b.h = y;
    active_context->SP = sp;

    // Now we have to set up the PSW.  Fortunately, SNEeSe now has a function
    // to set its internal state up for us.
    active_context->PSW = psw;
    spc_restore_flags();
  }

  void Run(int cycles) { SPC_START(cycles); }
  uint8_t* ram() { return SPCRAM; }
  void WritePort(int index, uint8_t data) { SPC_WRITE_PORT_R(index, data); }
  uint8_t ReadPort(int index) { return SPC_READ_PORT_W(index); }

 private:
  // TODO(bmartin) Make our own SPC700_CONTEXT and keep it here.  Make it
  // active before calling any SNEeSe code, and null it out after.  (Blocked
  // on dsp.c dependency on the context being set currently.)
};

SpcCpu::SpcCpu() : impl_(std::make_unique<Impl>()) {}
SpcCpu::~SpcCpu() {}

void SpcCpu::SetState(uint16_t pc, uint8_t a, uint8_t x, uint8_t y, uint8_t psw,
                      uint8_t sp, const uint8_t* ram) {
  impl_->SetState(pc, a, x, y, psw, sp, ram);
}

void SpcCpu::Run(int cycles) { impl_->Run(cycles); }
uint8_t* SpcCpu::ram() { return impl_->ram(); }

void SpcCpu::WritePort(int index, uint8_t data) {
  impl_->WritePort(index, data);
}

uint8_t SpcCpu::ReadPort(int index) { return impl_->ReadPort(index); }

}  // namespace openspc
