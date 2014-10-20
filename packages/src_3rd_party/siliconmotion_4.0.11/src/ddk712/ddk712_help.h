/*******************************************************************
 
Copyright (c) 2012 by Silicon Motion, Inc. (SMI)

Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights to 
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
of the Software, and to permit persons to whom the Software is furnished to 
do so, subject to the following conditions:

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
NONINFRINGEMENT.  IN NO EVENT SHALL MILL CHEN, MONK LIU, ALEX YAO, 
SUNNY YANG, ILENA ZHOU, MANDY WANG OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
OTHER DEALINGS IN THE SOFTWARE.
 
*******************************************************************/

#ifndef DDK712_HELP_H__
#define DDK712_HELP_H__
#include "../smi_common.h"
#include "ddk712_chip.h"
#include	"compiler.h"
#ifndef USE_INTERNAL_REGISTER_ACCESS



/* mmio712 start from framebuffer + 4mega bytes */
extern volatile unsigned char * mmio712 ;
extern int io_offset712;
/* DPR and VPR memory mapped registers,treated as 32bit access */
extern int dpr_offset712;
extern int vpr_offset712;
extern int dataPort_offset712;

#define peek32(addr) MMIO_IN32(mmio712,addr)
#define poke32(addr,data) MMIO_OUT32(mmio712,(addr),(data))

#define peek8(addr) MMIO_IN8(mmio712,addr)
#define poke8(addr,data) MMIO_OUT8(mmio712,(addr),(data))

#define peek8_io(addr) peek8(addr+io_offset712)
#define poke8_io(addr,data) poke8(addr+io_offset712,data)

static inline char peek_scr(int idx)
{
	poke8_io(0x3c4,idx);
	return peek8_io(0x3c5);
}

static inline void poke_scr(int idx,char val)
{
	poke8_io(0x3c4,idx);
	poke8_io(0x3c5,val);
}

static inline char peek_crt(int idx)
{
	poke8_io(0x3d4,idx);
	return peek8_io(0x3d5);
}

static inline void poke_crt(int idx,char val)
{
	poke8_io(0x3d4,idx);
	poke8_io(0x3d5,val);
}

/* below registers equal to sequence registers */
#define peek_pdr(a) peek_scr(a)
#define peek_fpr(a) peek_scr(a)
#define peek_mcr(a) peek_scr(a)
#define peek_ccr(a) peek_scr(a)
#define peek_gpr(a) peek_scr(a)
#define peek_phr(a) peek_scr(a)
#define peek_pop(a) peek_scr(a)
#define peek_hcr(a) peek_scr(a)

#define poke_pdr(a,b) poke_scr(a,b)
#define poke_fpr(a,b) poke_scr(a,b)
#define poke_mcr(a,b) poke_scr(a,b)
#define poke_ccr(a,b) poke_scr(a,b)
#define poke_gpr(a,b) poke_scr(a,b)
#define poke_phr(a,b) poke_scr(a,b)
#define poke_pop(a,b) poke_scr(a,b)
#define poke_hcr(a,b) poke_scr(a,b)

/* below registers equal to crtc registers*/
#define peek_svr(a) peek_crt(a)
#define poke_svr(a,b) poke_crt(a,b)


static inline unsigned int peek32_dpr(int index)
{
	return peek32(index + dpr_offset712);
}

static inline void poke32_dpr(int index,unsigned int val)
{
	poke32(index + dpr_offset712,val);
}

static inline unsigned int peek32_vpr(int index)
{
	return peek32(index + vpr_offset712);
}

static inline void poke32_vpr(int index,unsigned int val)
{
	poke32(index + vpr_offset712,val);
}
#else
/* implement if you want use it*/
#endif

_X_EXPORT void ddk712_set_mmio(volatile unsigned char * addr,int devid);

#endif
