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
#define DEFAULT_I2C_SCL                     30
#define DEFAULT_I2C_SDA                     31

/*
 * This function initializes the i2c attributes and bus
 *
 * Parameters:
 *      i2cClkGPIO  - The GPIO pin to be used as i2c SCL
 *      i2cDataGPIO - The GPIO pin to be used as i2c SDA
 *
 * Return Value:
 *      -1   - Fail to initialize the i2c
 *       0   - Success
 */
long swI2CInit(
    unsigned char i2cClkGPIO, 
    unsigned char i2cDataGPIO
);

/*
 *  This function reads the slave device's register
 *
 *  Parameters:
 *      deviceAddress   - i2c Slave device address which register
 *                        to be read from
 *      registerIndex   - Slave device's register to be read
 *
 *  Return Value:
 *      Register value
 */
unsigned char swI2CReadReg(
    unsigned char deviceAddress, 
    unsigned char registerIndex
);

/*
 *  This function writes a value to the slave device's register
 *
 *  Parameters:
 *      deviceAddress   - i2c Slave device address which register
 *                        to be written
 *      registerIndex   - Slave device's register to be written
 *      data            - Data to be written to the register
 *
 *  Result:
 *          0   - Success
 *         -1   - Fail
 */
long swI2CWriteReg(
    unsigned char deviceAddress, 
    unsigned char registerIndex, 
    unsigned char data
);

/*
 *  These two functions are used to toggle the data on the SCL and SDA I2C lines.
 *  The used of these two functions are not recommended unless it is necessary.
 */

/*
 *  This function set/reset the SCL GPIO pin
 *
 *  Parameters:
 *      value	- Bit value to set to the SCL or SDA (0 = low, 1 = high)
 */ 
static void swI2CSCL(unsigned char value);

/*
 *  This function set/reset the SDA GPIO pin
 *
 *  Parameters:
 *      value	- Bit value to set to the SCL or SDA (0 = low, 1 = high)
 */
static void swI2CSDA(unsigned char value);
static unsigned char swI2CReadSCL();
static unsigned char swI2CReadSDA();
_X_EXPORT long swI2CInit(unsigned char i2cClkGPIO, unsigned char i2cDataGPIO);
_X_EXPORT void ddk750_I2CPutBits_panel(I2CBusPtr bus,int clock,int data);
_X_EXPORT void ddk750_I2CGetBits_panel(I2CBusPtr bus,int* clock,int* data);
_X_EXPORT void ddk750_I2CPutBits_crt(I2CBusPtr bus,int clock,int data);
_X_EXPORT void ddk750_I2CGetBits_crt(I2CBusPtr bus,int* clock,int* data);
#endif  /* _SWI2C_H_ */
