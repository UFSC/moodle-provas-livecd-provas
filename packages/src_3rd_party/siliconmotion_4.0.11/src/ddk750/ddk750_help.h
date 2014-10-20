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

#ifndef DDK750_HELP_H__
#define DDK750_HELP_H__
#include "ddk750_chip.h"

#ifdef HAVE_CONFIG_H 
#include "config.h"
#endif

//#include	"compiler.h"

#ifndef NULL
#define NULL 0
#endif

#define outb_p outb
#define inb_p inb

/* DDK Interface
Ddk750_chip.c 	    unsigned int ddk750_getVMSize()	
Ddk750_chip.c	    int ddk750_initHw()	
Ddk750_display.c	void ddk750_setLogicalDispOut()	
Ddk750_display.c	int ddk750_initDVIDisp()
Ddk750_help.c	    void ddk750_set_mmio()
Ddk750_mode.c	    int ddk750_setModeTiming()
Ddk750_power.c	    void ddk750_setDPMS()
*/
int PEEK32(addr) ;
void POKE32(addr,data);
int PEEK8(addr);
void POKE8(addr,data);
void ddk750_set_mmio(volatile unsigned char * addr,unsigned short devId,unsigned long revId);
#endif
