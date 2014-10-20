#ifdef HAVE_XORG_CONFIG_H
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
#include <xorg-config.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include	"compiler.h"

#include "xf86Module.h"
#if XORG_VERSION_CURRENT < 10706000
#include "xf86Resources.h"
#endif

#include "ddk750_help.h"

volatile unsigned char * mmio750 = NULL;
unsigned long revId750 = 0;
unsigned short devId750 = 0;

_X_EXPORT int PEEK32(addr) 
{
	return MMIO_IN32(mmio750,addr);
}
	
_X_EXPORT void POKE32(addr,data)
{
	MMIO_OUT32(mmio750,(addr),(data));
}
_X_EXPORT int PEEK8(addr) 
{
	return MMIO_IN8(mmio750,addr);
}
	
_X_EXPORT void POKE8(addr,data)
{
	MMIO_OUT8(mmio750,(addr),(data));
}

/* after driver mapped io registers, use this function first */
_X_EXPORT void ddk750_set_mmio(volatile unsigned char * addr,unsigned short devId,unsigned long revId)
{
	mmio750 = addr;
	devId750 = devId;
	revId750 = revId;
	if(revId == 0xfe){
		/*printk("found sm750le\n");*/
	}
}


