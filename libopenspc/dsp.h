/************************************************************************

        Copyright (c) 2003 Brad Martin.

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



dsp.h: defines functions that emulate the DSP part of the SPC module.
The functions and data defined in this file must be shared because the
SPC core needs access to them, however they are not intended for external
library use, and their specific implementations and prototypes are subject
to change.

 ************************************************************************/

#if !defined _DSP_H
#define _DSP_H

/*========== TYPES ==========*/

typedef enum                        /* ADSR state type              */
{
    ATTACK,
    DECAY,
    SUSTAIN,
    RELEASE
} env_state_t32;

typedef struct                      /* Voice state type             */
{
    unsigned long    samp_id;       /* Sample ID#                   */
    signed long     smp1;           /* Last sample (for BRR filter) */
    signed long     smp2;           /* Second-to-last sample decoded*/
    int             mem_ptr;        /* Sample data memory pointer   */
    int             header_cnt;     /* Bytes before new header (0-8)*/
    int             range;          /* Last header's range          */
    int             filter;         /* Last header's filter         */
    int             end;            /* End or loop after block      */
    int             half;           /* Active nybble of BRR         */
    int             mixfrac;        /* Fractional part of smpl pstn */
    int             envcnt;         /* Counts to envelope update    */
    int             envx;           /* Last env height (0-0x7FFF)   */
    int             pitch;          /* Sample pitch (4096->32000Hz) */
    int             sampptr;        /* Where in sampbuf we are      */
    int             on_cnt;         /* Is it time to turn on yet?   */
    env_state_t32   envstate;       /* Current envelope state       */
    short           sampbuf[ 4 ];   /* Buffer for Gaussian interp   */
} voice_state_type;

typedef struct                      /* Source directory entry       */
{
    unsigned short  vptr;           /* Ptr to start of sample data  */
    unsigned short  lptr;           /* Loop pointer in sample data  */
} src_dir_type;

/*========== CONSTANTS ==========*/

extern const int    TS_CYC;

/*========== VARIABLES ==========*/

extern int          keyed_on;
extern int          keys;           /* 8-bits for 8 voices          */
extern voice_state_type
                    voice_state[ 8 ];

/*========== MACROS ==========*/

/* The functions to actually read and write to the DSP registers must be
   implemented by the specific SPC core implementation, as this is too 
   specific to generalize.  However, by defining these macros, we can
   generalize the DSP's behavior while staying out of the SPC's internals,
   by requiring that the SPC core must use these macros at the appropriate
   times. */

/* All reads simply return the contents of the addressed register. */

/* This macro must be used INSTEAD OF a normal write to register 0x7C
   (ENDX) */
#define DSP_WRITE_7C(x) DSPregs[0x7C]=0

/* All other writes should store the value in the addressed register as
   expected. */

/*========== PROCEDURES ==========*/

void DSP_Update                     /* Mix one sample of audio      */
    (
    short *             sound_ptr   /* Pointer to mix audio into    */
    );

void DSP_Reset                      /* Reset emulated DSP           */
    ( void );

#endif  /* _DSP_H */
