/**************************************************************************

        Copyright (c) 2005-2020 Brad Martin.
        Some portions copyright (c) 1998-2005 Charles Bilyue'.

This file is part of OpenSPC.

sneese_spc.h: This file defines the interface between the SNEeSe SPC700
core and the associated wrapper files.  As the licensing rights for SNEeSe
are different from the rest of OpenSPC, none of the files in this directory
are LGPL.  Although this file was created by me (Brad Martin), it contains
some code derived from SNEeSe and therefore falls under its license.  See
the file 'LICENSE' in this directory for more information.

 **************************************************************************/

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SPC_CTRL (SPCRAM[0xF1])
#define SPC_DSP_ADDR (SPCRAM[0xF2])

// SPC_START()'s argument is actually supposed to be in SNES-CPU cycles, not
// SPC-CPU cycles.  The value of these two symbols then dictates the
// relationship between the requested cycles and how many are run.  We
// actually just want to specify SPC cycles directly, so we'll hard-code these
// values for a 1:1 relationship.
#define SPC_CPU_cycle_divisor 1
#define SPC_CPU_cycle_multiplicand 1

typedef union {
  uint16_t w;
  struct {
    uint8_t l;
    uint8_t h;
  } b;
} Word2B;

typedef struct {
  // The following contents of this struct should be functionally equivalent
  // to what appears in include/apu/spc.h in the SNEeSe source.

  // Number of cycles to execute for SPC.
  uint32_t Cycles;
  uint32_t last_cycles;

  uint32_t TotalCycles;
  int32_t WorkCycles;

  uint8_t PORT_R[4];

  uint8_t PORT_W[4];

  const uint8_t* FFC0_Address;

  Word2B PC;
  Word2B YA;
  Word2B address;
  Word2B address2;
  Word2B direct_page;
  Word2B data16;
  uint8_t SP;
  uint8_t X;
  uint8_t cycle;

  uint8_t opcode;
  uint8_t data;
  uint8_t data2;
  uint8_t offset;

  // Processor status word.
  uint8_t PSW;

  uint8_t N_flag;
  uint8_t V_flag;
  uint8_t P_flag;
  uint8_t B_flag;

  uint8_t H_flag;
  uint8_t I_flag;
  uint8_t Z_flag;
  uint8_t C_flag;

  struct {
    uint32_t cycle_latch;
    int16_t position;
    int16_t target;
    uint8_t counter;
  } timers[3];

  // The following symbols have been moved into the context to avoid
  // dependence on global state.

  // These variables are used for debug printing of error conditions.
  uint32_t map_address;
  uint32_t map_byte;
  // Pointer to DSP register file.  MUST BE SET EXTERNALLY BEFORE USE!
  uint8_t* dsp_regs;
  // Data for DSP register transactions.
  uint8_t dsp_data;
  uint8_t ram[65536];
} SPC700_CONTEXT;

// Aliases for stuff moved into the context.
#define Map_Address (active_context->map_address)
#define Map_Byte (active_context->map_byte)
#define SPC_DSP (active_context->dsp_regs)
#define SPC_DSP_DATA (active_context->dsp_data)
#define SPCRAM (active_context->ram)

// sneese_spc.c variables.

// The value of this variable doesn't matter at all.  Its value is saved and
// set to zero when entering SPC_START(), restored afterwards, and otherwise
// never used outside of SNEeSe.
extern uint8_t In_CPU;
// This value is only ever set to zero and never used.
extern uint8_t SPC_CPU_cycles;
// This value is also regularly set to only zero, though it is read from.
extern uint8_t SPC_CPU_cycles_mul;
// Only set to zero and never read.
extern uint8_t sound_cycle_latch;

// spc700.c variables.

extern SPC700_CONTEXT* active_context;

// Stubs for functions called that we don't need.
#define Wrap_SDSP_Cyclecounter()
#define update_sound()

// Other functions called by spc700.c, defined in sneese_spc.c.
void DisplaySPC(void);
void InvalidSPCOpcode(void);
void SPC_READ_DSP(void);
void SPC_WRITE_DSP(void);

// Functions in spc700.c that we need to be able to call.
void Reset_SPC(void);
uint8_t SPC_READ_PORT_W(uint16_t address);
void SPC_START(unsigned cycles);
void SPC_WRITE_PORT_R(uint16_t address, uint8_t data);
uint8_t get_SPC_PSW(void);
void spc_restore_flags(void);

#ifdef __cplusplus
}  // extern "C"
#endif
