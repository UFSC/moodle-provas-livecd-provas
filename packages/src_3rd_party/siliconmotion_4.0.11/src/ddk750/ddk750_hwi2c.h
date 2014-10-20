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
#ifndef DDK750_HWI2C_H__
#define DDK750_HWI2C_H__

#define USE_HW_I2C	1

void ddk750_I2CPutBits_panel(I2CBusPtr bus,int clock,int data);
void ddk750_I2CGetBits_panel(I2CBusPtr bus,int* clock,int* data);
void ddk750_I2CPutBits_crt(I2CBusPtr bus,int clock,int data);
void ddk750_I2CGetBits_crt(I2CBusPtr bus,int* clock,int* data);

/* hwi2c functions */
int hwI2CInit(unsigned char busSpeedMode);
void hwI2CClose(void);

unsigned char hwI2CReadReg(unsigned char deviceAddress,unsigned char registerIndex);
int hwI2CWriteReg(unsigned char deviceAddress,unsigned char registerIndex,unsigned char data);
#endif
