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
#ifdef HAVE_XORG_CONFIG_H
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

#include "ddk502_help.h"

volatile unsigned char * mmio502 = NULL;
unsigned long revId502 = 0x0;
unsigned short devId502 = 0;

volatile unsigned char * fb502 = NULL;

_X_EXPORT unsigned long ddk502_PEEK32(addr) 
{
	return MMIO_IN32(mmio502,addr);
}
	
_X_EXPORT void ddk502_POKE32(addr,data)
{
	MMIO_OUT32(mmio502,(addr),(data));
}
_X_EXPORT void ddk502_POKEfb32(addr,data)
{
    MMIO_OUT32(fb502,(addr),(data));
}
/* after driver mapped io registers, use this function first */
_X_EXPORT void ddk502_set_mmio(volatile unsigned char * addr,unsigned short devId,unsigned long revId)
{
	mmio502 = addr;
	devId502= devId;
	revId502 = revId;
}

_X_EXPORT void ddk502_set_fb(volatile unsigned char * addr,unsigned short devId,char revId)
{

}
