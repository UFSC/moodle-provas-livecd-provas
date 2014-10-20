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

#include "ddk712_help.h"

volatile unsigned char * mmio712 = NULL;
/* below offset used for sm712/sm722 */
int io_offset712;
/* DPR and VPR memory mapped registers,treated as 32bit access */
int dpr_offset712;
int vpr_offset712;
int dataPort_offset712;

/* after driver mapped io registers, use this function first */
_X_EXPORT void ddk712_set_mmio(volatile unsigned char * addr,int devid)
{
	mmio712 = addr;
	if(devid == 0x712){
		/* sm712 register offset stuffs */
		io_offset712 = MB(3);
		dpr_offset712 = KB(32); 
		vpr_offset712 = KB(48);
		dataPort_offset712 = MB(1);
	}else if(devid == 0x720){
		/* sm722 register offset stuffs */
		io_offset712 = KB(768);
		dpr_offset712 = KB(0);
		vpr_offset712 = KB(2);
		dataPort_offset712 = MB(1);
	}
}

