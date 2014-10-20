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

#ifndef _MODE_H_
#define _MODE_H_

#include "ddk502_display.h" /* This file uses a definition in DISPLAY.H */

typedef enum _spolarity_t
{
    POS, /* positive */
    NEG, /* negative */
}
spolarity_t;

typedef struct _mode_parameter_t
{
    /* Horizontal timing. */
    unsigned long horizontal_total;
    unsigned long horizontal_display_end;
    unsigned long horizontal_sync_start;
    unsigned long horizontal_sync_width;
    spolarity_t horizontal_sync_polarity;

    /* Vertical timing. */
    unsigned long vertical_total;
    unsigned long vertical_display_end;
    unsigned long vertical_sync_start;
    unsigned long vertical_sync_height;
    spolarity_t vertical_sync_polarity;

    /* Refresh timing. */
    unsigned long pixel_clock;
    unsigned long horizontal_frequency;
    unsigned long vertical_frequency;
    
    /* Clock Phase. This clock phase only applies to Panel. */
    spolarity_t clock_phase_polarity;
}
mode_parameter_t;

typedef struct _logicalMode_t
{
    long x;            /* X resolution */
    long y;            /* Y resolution */
    long bpp;          /* Bits per pixel */
    long hz;           /* Refresh rate */

    long baseAddress;  /* Offset from beginning of frame buffer.
                          It is used to control the starting location of a mode.
                          Calling function must initialize this field.
                        */

    long pitch;        /* Mode pitch in byte.
                          If initialized to 0, setMode function will set
                          up this field.
                          If not zero, setMode function will use this value.
                        */

    disp_control_t dispCtrl;     /* CRT or PANEL display control channel */

    long virtual;       /* 0 = off, 1 = on */

    /* When virtual is off, xLCD and yLCD are not used bacause assuming
       xLCD = x and yLCD = y.

       When virtual is on, xLCD != x and yLCD != y.
       if xLCD and yLCD are smaller than x and y, then we have panning.
       if xLCD are yLCD are bigger than x and y, we have wrap around.
    */
    long xLCD;          /* LCD width */
    long yLCD;          /* LCD height */

    void *userData;     /* Not used now, for future only */
}
logicalMode_t;

/* 
 * This function sets the flag to force the clock to use 2x.
 * Set it to 1 to force 2x clock.
 */
void set2xDisplayClock(unsigned char ucForce2xDisplayClock);

/* This function gets the 2x Display Clock flag */
unsigned char get2xDisplayClock(void);

/* Get the display clock multiplier */
unsigned char getDisplayClkMultiplier(disp_control_t dispControl);

/* 
 * Return a point to the gModeParamTable.
 * Function in other files used this to get the mode table pointer.
 */
mode_parameter_t *getStockModeParamTable(void);

/*
 * Return the size of the Stock Mode Param Table
 */
unsigned long getStockModeParamTableSize();

/* 
 * This function returns the current mode.
 */
mode_parameter_t getCurrentModeParam(
    disp_control_t dispCtrl
);

long setCustomMode(
    logicalMode_t *pLogicalMode, 
    mode_parameter_t *pUserModeParam
);

long ddk502_setModeTiming(
    logicalMode_t *pLogicalMode
);

mode_parameter_t *findVesaModeParam(
    unsigned long width, 
    unsigned long height, 
    unsigned long refresh_rate
);

/*
 *	This function sets the display base address
 *
 *	Input:
 *		dispControl		- display control of which base address to be set.
 *		ulBaseAddress	- Base Address value to be set.
 */
void setDisplayBaseAddress(
	disp_control_t dispControl,
	unsigned long ulBaseAddress
);

/*
 *	This function gets the display status
 *
 *	Input:
 *		dispControl		- display control of which display status to be retrieved.
 *
 *  Output:
 *      0   - Display is pending
 *     -1   - Display is not pending
 */
long isCurrentDisplayPending(
    disp_control_t dispControl
);

#endif /* _MODE_H_ */
