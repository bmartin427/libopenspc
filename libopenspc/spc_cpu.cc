/************************************************************************

        Copyright (c) 2020 Brad Martin.

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

 ************************************************************************/

#include "spc_cpu.h"

#include "SNEeSe/SPCimpl.h"

namespace openspc {

SpcCpu::SpcCpu() { SPC_Reset(); }

void SpcCpu::SetState(uint16_t pc, uint8_t a, uint8_t x, uint8_t y, uint8_t psw,
                      uint8_t sp, const uint8_t* ram) {
  SPC_SetState(pc, a, x, y, psw, sp, ram);
}

void SpcCpu::Run(int cycles) { SPC_Run(cycles); }
uint8_t* SpcCpu::ram() { return SPC_RAM; }

void SpcCpu::WritePort(int index, uint8_t data) {
  SPC_WRITE_PORT_R(index, data);
}

uint8_t SpcCpu::ReadPort(int index) { return SPC_READ_PORT_W(index); }

}  // namespace openspc
