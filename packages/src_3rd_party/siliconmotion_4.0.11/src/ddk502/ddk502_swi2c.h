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

#ifndef _SWI2C_H_
#define _SWI2C_H_

/* Default i2c CLK and Data GPIO. These are the default i2c pins */
#define DEFAULT_I2C_SCL                     46
#define DEFAULT_I2C_SDA                     47

_X_EXPORT unsigned char ddk502_swI2CInit(
    unsigned char i2cClkGPIO, 
    unsigned char i2cDataGPIO
);

_X_EXPORT unsigned char ddk502_swI2CReadReg(
    unsigned char deviceAddress, 
    unsigned char registerIndex
);

unsigned char swI2CWriteReg(
    unsigned char deviceAddress, 
    unsigned char registerIndex, 
    unsigned char data
);
_X_EXPORT void ddk750_I2CPutBits_panel(I2CBusPtr bus,int clock,int data);
_X_EXPORT void ddk750_I2CGetBits_panel(I2CBusPtr bus,int* clock,int* data);
_X_EXPORT void ddk750_I2CPutBits_crt(I2CBusPtr bus,int clock,int data);
_X_EXPORT void ddk750_I2CGetBits_crt(I2CBusPtr bus,int* clock,int* data);

#endif  /* _SWI2C_H_ */
