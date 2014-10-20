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

#ifndef _CHIP_H_
#define _CHIP_H_

#define MEMORY_DEFAULT        144

/* This is all the chips recognized by this library */
typedef enum _logical_chip_type_t
{
    SM_UNKNOWN,
    SM501,
    SM502,
    SM107,
    SM718,
    SM750,
}
logical_chip_type_t;


/* input struct to initChipParam() function */
typedef struct _initchip_param_t
{
    unsigned short powerMode;    /* Use power mode 0 or 1 */
    unsigned short memClock;     /* Speed of memory clock in MHz unit
                                    0 = keep the current clock setting
                                    Others = the new memory clock
                                  */
    unsigned short masterClock;  /* Speed of master clock in MHz unit 
                                    0 = keep the current clock setting
                                    Others = the new master clock
                                  */
    unsigned short setAllEngOff; /* 0 = leave all engine state untouched.
                                    1 = make sure they are off: 2D, Overlay,
                                    video alpha, alpha, hardware cursors
                                 */

    /* More initialization parameter can be added if needed */
}
initchip_param_t;

/*
 * This function returns frame buffer memory size in Byte units.
 */
//unsigned long getFrameBufSize(void);
int ddk502_getFrameBufSize();

/*
 * This function gets the Frame buffer location.
 */
unsigned char getFrameBufLocation();

/*
 * This function returns the logical chip type defined in chip.h
 * It is one of the following: SM501, SM502, SM107, SM718, SM750
 * or SM_UNKNOWN.
 */
logical_chip_type_t getChipType(void);

/*
 * Return a char string name of the current chip.
 * It's convenient for application need to display the chip name.
 */
char * getChipTypeString(void);

/*
 * Initialize a single chip and environment according to input parameters.
 *
 * Input: initchip_param_t structure.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 * Note:
 *      Caller needs to call the detectDevice and setCurrentDevice
 *      to set the device before calling this initChipParamEx.
 */
long initChipParamEx(initchip_param_t * pInitParam);

/*
 * Initialize every chip and environment according to input parameters.
 *
 * Input: initchip_param_t structure.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 */
long initChipParam(initchip_param_t * pInitParam);

/*
 * Initialize a single chip with default parameters.
 *
 * Input: none.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 * Note:
 *      Caller needs to call the detectDevice and setCurrentDevice
 *      to set the device before calling this initChipEx.
 */
long initChipEx();

/*
 * Initialize all available chips with default parameters.
 *
 * Input: none.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 * Note:
 *      This function initializes all the adapters at once.
 *      To initialize a selected adapters, please use initChipEx
 */
long ddk502_initHw();

#endif /* _CHIP_H_ */
