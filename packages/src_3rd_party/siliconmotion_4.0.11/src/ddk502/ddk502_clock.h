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

#ifndef _CLOCK_H_
#define _CLOCK_H_

#define DEFAULT_INPUT_CLOCK 24000000 /* Default reference clock */
#define MHz(x) (x*1000000) /* Don't use this macro if x is fraction number */

typedef enum _clock_type_t
{
    /* Divider clocks */
    P2XCLK,  /* x2 panel divider clock */
    V2XCLK,  /* x2 CRT divider clock */
    MCLK,    /* x1 master clock */
    M1XCLK,  /* x1 memory clock */

    /* The following are for SM502 and later */
    P1XCLK,       /* x1 panel divider clock */
    V1XCLK,       /* x1 CRT divider clock */
    PANEL_PLL,    /* Programmable panel pixel clock */
    PANEL_PLL_2X, /* For TV, 2X clock is needed */
}
clock_type_t;

typedef struct pll_value_t
{
    clock_type_t clockType;
    unsigned long inputFreq; /* Input clock frequency to the PLL */

    /* Use this for all clockType != PANEL_PLL */
    unsigned long dividerClk; /* either x12 or x14 of the input reference crystal */
    unsigned long divider;
    unsigned long shift;

    /* Use this when clockType = PANEL_PLL */    
    unsigned long M;
    unsigned long N;
    unsigned long divBy2;
}
pll_value_t;

/* Perform a rounded division. */
long roundedDiv(long num, long denom);

/* Absolute differece between two numbers */
unsigned long absDiff(unsigned long a, unsigned long b);

/*
 * This functions set up the proper values in the PLL structure.
 */
unsigned long calcPllValue(unsigned long ulPixelClock, pll_value_t *pPLL);

/*
 * Set up the corresponding bit field of the Power Mode X Clock register.
 */
unsigned long formatModeClockReg(pll_value_t *pPLL);

/*
 * Set up the corresponding bit field of the programmable PLL register.
 */
unsigned long formatPllReg(pll_value_t *pPLL);

/*
 * This function get the Panel Pixel Clock value.
 *
 * Output:
 *      The Panel Pixel Clock value in whole number.
 */
unsigned long getPanelClock();

/*
 * This function get the CRT Pixel Clock value.
 *
 * Output:
 *      The CRT Pixel Clock value in whole number.
 */
unsigned long getCRTClock();

/*
 * This function set up the memory clock.
 *
 * Input: Frequency in MHz unit.
 */
void setMemoryClock(unsigned long megaHz);

/*
 * This function set up the master clock (MCLK).
 *
 * Input: Frequency in MHz unit.
 */
void setMasterClock(unsigned long megaHz);

/*
 *  getMasterClock
 *      This function gets the Master Clock value.
 *
 *  Output:
 *      The Master Clock value in whole number.
 */
unsigned long getMasterClock();

/*
 *  getMemoryClock
 *      This function gets the Memory Clock value.
 *
 *  Output:
 *      The Memory Clock value in whole number.
 */
unsigned long getMemoryClock();

#endif /*_CLOCK_H_*/
