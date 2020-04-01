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

#pragma once

#include <cstdint>
#include <memory>

namespace openspc {

/// Module that simulates the CPU side of the SPC-700.
// TODO(bmartin) The current implementation interacts with global state, hence
// only one copy of this class may exist in a process.  Fix this eventually.
class SpcCpu {
 public:
  static constexpr int kRamSize = 65536;

  SpcCpu();
  ~SpcCpu();

  /// Initialize the CPU to the given state.  @p ram should point to kRamSize
  /// bytes of data.
  void SetState(uint16_t pc, uint8_t a, uint8_t x, uint8_t y, uint8_t psw,
                uint8_t sp, const uint8_t* ram);

  /// Run the CPU for the given number of cycles.
  void Run(int);

  /// Retrieve a mutable pointer to the memory space of the CPU.
  uint8_t* ram();

  /// Write to one of the CPU's four incoming communication ports.
  void WritePort(int index, uint8_t data);

  /// Read from one of the CPU's four outgoing communication ports.
  uint8_t ReadPort(int index);

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};

}  // namespace openspc
