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
#ifndef DDK502_HELP_H__
#define DDK502_HELP_H__
#include "ddk502_chip.h"
#include "../smi_common.h"

#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif

//#include	"compiler.h"

#ifndef NULL
#define NULL 0
#endif



#define outb_p outb
#define inb_p inb

unsigned long ddk502_PEEK32(addr) ;

void ddk502_POKE32(addr,data);
void ddk502_POKEfb32(addr,data);
void ddk502_set_mmio(volatile unsigned char * addr,unsigned short devId,unsigned long revId);

/*
_X_EXPORT void ddk750_set_mmio(volatile unsigned char * addr,unsigned short devId,char revId);
*/

#endif
