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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "openspc.h"
#include "dsp.h"
#include "SPCimpl.h"

#undef NO_CLEAR_ECHO

static int mix_left;

/**** Internal (static) functions ****/

static int Load_SPC(void *buf,size_t size)
{
    const char ident[]="SNES-SPC700 Sound File Data";
    struct SPC_FILE
    {
        unsigned char ident[37],
                      PCl,PCh,
                      A,
                      X,
                      Y,
                      P,
                      SP,
                      junk[212],
                      RAM[65536],
                      DSP[128];
    } *spc_file;
    if(size<sizeof(spc_file))
        return 1;
    spc_file=(struct SPC_FILE *)buf;

    if(memcmp(buf,ident,strlen(ident)))
        return 1;
    SPC_SetState(((int)spc_file->PCh<<8)+spc_file->PCl,spc_file->A,
     spc_file->X,spc_file->Y,spc_file->P,0x100+spc_file->SP,spc_file->RAM);
    memcpy(DSPregs,spc_file->DSP,128);
    return 0;
}

static int Load_ZST(void *buf,size_t size)
{
    int p;
    const char ident[]="ZSNES Save State File";
    struct ZST_FILE
    {
        unsigned char ident[26],
                 junk[199673],
                 RAM[65536],
                 junk2[16],
                 PCl,PCh,PCj[2],
                 A,Aj[3],
                 X,Xj[3],
                 Y,Yj[3],
                 P,Pj[3],
                 P2[4],
                 SP,SPj[3],
                 junk3[420],
                 v_on[8],
                 junk4[916],
                 DSP[256];
    } *zst_file;

    if(size<sizeof(struct ZST_FILE))
        return 1;
    zst_file=(struct ZST_FILE *)buf;

    if(memcmp(buf,ident,strlen(ident)))
        return 1;
    p=zst_file->P;
    if((zst_file->P2[0]|zst_file->P2[1]|zst_file->P2[2]|zst_file->P2[3])==0)
        p|=2;
    else
        p&=~2;
    if(zst_file->P2[0]&0x80)
        p|=0x80;
    else
        p&=~0x80;
    SPC_SetState(((int)zst_file->PCh<<8)+zst_file->PCl,zst_file->A,
     zst_file->X,zst_file->Y,p,0x100+zst_file->SP,zst_file->RAM);
    memcpy(DSPregs,zst_file->DSP,256);
    /* Little hack to turn on voices that were already on when state
       was saved.  Doesn't restore the entire state of the voice, just
       starts it over from the beginning. */
    for(p=0;p<8;p++)
        if(zst_file->v_on[p])
            DSPregs[0x4C]|=(1<<p);
    return 0;
}

static int GZ_Read(void *buf,size_t size,z_streamp zsp)
{
    zsp->next_out=reinterpret_cast<Bytef*>(buf);
    zsp->avail_out=size;
    return inflate(zsp,Z_SYNC_FLUSH);
}

static z_streamp GZ_Open(unsigned char *buf,size_t size)
{
    struct gz_header
    {
        unsigned char id1,
                      id2,
                      cm,
                      flg,
                      junk[6];
    } *gzh=(struct gz_header *)buf;
    z_streamp zsp;
    size_t skip=sizeof(struct gz_header);
    /* First, verify the GZ header */
    if((gzh->id1!=0x1F)||(gzh->id2!=0x8B)||(gzh->cm!=0x08)||(gzh->flg&0xE0))
        return NULL;
    if(gzh->flg&0x04)
        skip+=(int)buf[skip]+(int)(buf[skip+1]<<8)+2;
    if(gzh->flg&0x08)
        while((skip<size)&&(buf[skip]!='\0'))
            skip++;
    if(gzh->flg&0x10)
        while((skip<size)&&(buf[skip]!='\0'))
            skip++;
    if(gzh->flg&0x02)
        skip+=2;
    if(skip>=size)
        return NULL;
    zsp=reinterpret_cast<z_streamp>(malloc(sizeof(z_stream)));
    zsp->next_in=buf+skip;
    zsp->avail_in=size-skip;
    zsp->zalloc=Z_NULL;
    zsp->zfree=Z_NULL;
    zsp->opaque=Z_NULL;
    if(inflateInit2(zsp,-MAX_WBITS)!=Z_OK)
    {
        fprintf(stderr,"ZLib init error: '%s'\n",zsp->msg);
        return NULL;
    }
    return zsp;
}

static void GZ_Close(z_streamp zsp)
{
    inflateEnd(zsp);
    free(zsp);
}

static int Load_S9X(void *buf,size_t size)
{
    struct S9X_APU_BLOCK
    {
        unsigned char junk1[11],
                      DSP[0x80],
                      junk2[82];
    };
    struct S9X_APUREGS_BLOCK
    {
        unsigned char P,
                      A,
                      Y,
                      X,
                      S,
                      PCh,
                      PCl;
    } SnapAPURegisters;
    const char ident[]="#!snes9";
    const int bufsize=65536;
    char *obuf=reinterpret_cast<char*>(malloc(bufsize)),
      *RAM=reinterpret_cast<char*>(malloc(65536));
    z_streamp zsp=GZ_Open(reinterpret_cast<unsigned char*>(buf),size);
    int i,blen,foundRAM=0,foundRegs=0;

    if(zsp==NULL)
    {
        free(obuf);
        free(RAM);
        return 1;
    }
    i=GZ_Read(obuf,14,zsp);
    if(memcmp(ident,obuf,strlen(ident)))
    {
        GZ_Close(zsp);
        free(obuf);
        free(RAM);
        return 1;
    }
    while(GZ_Read(obuf,11,zsp)!=Z_STREAM_END)
    {
        for(i=0;(i<11)&&(obuf[i]!=':');i++);
        blen=strtol(&obuf[i+1],NULL,10);
        if(!memcmp(obuf,"APU",3))
        {
            if(blen>=bufsize)
            {
                GZ_Read(obuf,bufsize,zsp);
                blen-=bufsize;
            }
            else
            {
                GZ_Read(obuf,blen,zsp);
                blen=0;
            }
            memcpy(DSPregs,
             ((struct S9X_APU_BLOCK *)obuf)->DSP,0x80);
        }
        else if(!memcmp(obuf,"ARE",3))
        {
            if(blen>=(int)sizeof(struct S9X_APUREGS_BLOCK))
            {
                GZ_Read(&SnapAPURegisters,sizeof(struct S9X_APUREGS_BLOCK),
                  zsp);
                blen-=sizeof(struct S9X_APUREGS_BLOCK);
            }
            else
            {
                GZ_Read(&SnapAPURegisters,blen,zsp);
                blen=0;
            }
            foundRegs=1;
        }
        else if(!memcmp(obuf,"ARA",3))
        {
            if(blen>=65536)
            {
                GZ_Read(RAM,65536,zsp);
                blen-=65536;
            }
            else
            {
                GZ_Read(RAM,blen,zsp);
                blen=0;
            }
            foundRAM=1;
        }

        while(blen>bufsize)
        {
            GZ_Read(obuf,bufsize,zsp);
            blen-=bufsize;
        }
        GZ_Read(obuf,blen,zsp);
    }

    GZ_Close(zsp);
    free(obuf);
    if(!foundRAM||!foundRegs)
    {
        free(RAM);
        return 1;
    }
    /* Now that we have all the info, load the state */
    SPC_SetState(((int)SnapAPURegisters.PCh<<8)+SnapAPURegisters.PCl,
     SnapAPURegisters.A,SnapAPURegisters.X,SnapAPURegisters.Y,
     SnapAPURegisters.P,(int)SnapAPURegisters.S+0x100,RAM);
    free(RAM);
    return 0;
}

/**** Exported library interfaces ****/

extern "C" int OSPC_Init(void *buf, size_t size)
{
    int ret;
#ifndef NO_CLEAR_ECHO
    int start,len;
#endif
    mix_left=0;
    channel_mask = 0;
    SPC_Reset();
    DSP_Reset();
    ret=Load_SPC(buf,size);
    if(ret==1)  /* Return 1 means wrong format */
        ret=Load_ZST(buf,size);
    if(ret==1)
        ret=Load_S9X(buf,size);

/* New file formats could go on from here, for example:
    if(ret==1)
        ret=Load_FOO(buf,size);
    ...
*/
#ifndef NO_CLEAR_ECHO
    /* Because the emulator that generated the SPC file most likely did
       not correctly support echo, it is probably necessary to zero out
       the echo region of memory to prevent pops and clicks as playback
       begins. */
    if(!(DSPregs[0x6C]&0x20))
    {
        start=(unsigned char)DSPregs[0x6D]<<8;
        len=(unsigned char)DSPregs[0x7D]<<11;
        if(start+len>0x10000)
            len=0x10000-start;
        memset(&SPC_RAM[start],0,len);
    }
#endif
    return ret;
}

extern "C" int OSPC_Run(int cyc, short *s_buf, int s_size)
{
    int i,buf_inc=s_buf?2:0;

    if((cyc<0)||((s_buf!=NULL)&&(cyc>=(s_size>>2)*TS_CYC+mix_left)))
    {   /* Buffer size is the limiting factor */
        s_size&=~3;
        if(mix_left)
            SPC_Run(mix_left);
        for(i=0;i<s_size;i+=4,s_buf+=buf_inc)
        {
            DSP_Update(s_buf);
            SPC_Run(TS_CYC);
        }
        mix_left=0;
        return s_size;
    }

    /* Otherwise, use the cycle count */
    if(cyc<mix_left)
    {
        SPC_Run(cyc);
        mix_left-=cyc;
        return 0;
    }
    if(mix_left)
    {
        SPC_Run(mix_left);
        cyc-=mix_left;
    }
    for(i=0;cyc>=TS_CYC;i+=4,cyc-=TS_CYC,s_buf+=buf_inc)
    {
        DSP_Update(s_buf);
        SPC_Run(TS_CYC);
    }
    if(cyc)
    {
        DSP_Update(s_buf);
        SPC_Run(cyc);
        mix_left=TS_CYC-cyc;
        i+=4;
    }
    return i;
}

extern "C" void OSPC_WritePort0(char data)
{
    WritePort0(data);
}

extern "C" void OSPC_WritePort1(char data)
{
    WritePort1(data);
}

extern "C" void OSPC_WritePort2(char data)
{
    WritePort2(data);
}

extern "C" void OSPC_WritePort3(char data)
{
    WritePort3(data);
}

extern "C" char OSPC_ReadPort0(void)
{
    return ReadPort0();
}

extern "C" char OSPC_ReadPort1(void)
{
    return ReadPort1();
}

extern "C" char OSPC_ReadPort2(void)
{
    return ReadPort2();
}

extern "C" char OSPC_ReadPort3(void)
{
    return ReadPort3();
}

extern "C" void OSPC_SetChannelMask(int mask) {
  channel_mask = mask;
}

extern "C" int OSPC_GetChannelMask(void) {
  return channel_mask;
}
