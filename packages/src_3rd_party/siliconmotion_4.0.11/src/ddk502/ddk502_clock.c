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

#include "ddk502_voyager.h"
#include "ddk502_hardware.h"
#include "ddk502_power.h"
#include "ddk502_clock.h"
#include "ddk502_help.h"

/* Perform a rounded division. */
long roundedDiv(long num, long denom)
{
    /* n / d + 1 / 2 = (2n + d) / 2d */
    return (2 * num + denom) / (2 * denom);
}

/* Absolute differece between two numbers */
unsigned long absDiff(unsigned long a, unsigned long b)
{
    if ( a >= b )
        return(a - b);
    else
        return(b - a);
}

/*
 * Given a requested output clock frequency, this function calculates the 
 * best M & N values for Programmable PLL.
 * 
 * Note: 
 * 1) SM501 DON'T have programmable PLL at all.
 * 2) The programmable PLL is for SM502 panel display only, while CRT display uses divider clock same as SM501.
 * 3) SM502 can select between two inputs to the programmable PLL, either input 
 *    can use this function to work out the M & N values.
 *
 * PLL mechanism:
 * 1) The programmable PLL uses this formula to operate: requestClk = inputFreq * M / N .
 *
 * 2) It has a Divided by 2 feature at it's output because
 *    for some frequency, it is better to use (requestClk x 2) to work out the M & N values,
 *    and then divide them by 2 at output.
 *
 */
unsigned long calcProgrammablePllCtrl(
unsigned long ulRequestClk, /* Required panel pixel clock in Hz unit */
pll_value_t *pPLL          /* Structure to hold the value to be set in PLL */
)
{
    unsigned long M, N, diff, pllClk, i;
    unsigned long bestDiff = 0xffffffff; /* biggest 32 bit unsigned number */

    /* Sanity check */
    if (pPLL->inputFreq < MHz(5) || pPLL->inputFreq > MHz(100))
    {
        /* Input clock frequency to PLL has not been set properly or out of range.
           We have nothing to calculate. */
        return 0;
    }

    /* Convert everything in Khz range in order to avoid calculation overflow */
    pPLL->inputFreq /= 1000;
    ulRequestClk /= 1000;

    /* Try both ulRequestClk and (ulRequestClk x 2) to work out the best M & N values. */
    for (i=0; i<2; i++)
    {
        /* N must between 2 and 24, according to design */
        for (N=2; N<25; N++)
        {
            /* The formula for PLL is ulRequestClk = inputFreq * M / N .
               For Voyager family, SMI reference design uses 24000000 Hz (or 24 Mhz) for inputFreq.
               Depending on mode, requrestChk ranging from 25 Mhz to 110 Mhz.
               In the following steps, we try to work out an M value given the others are known.
               To avoid decimal calculation, we use 1000 as multiplier for up to 3 decimal places of accuracy.
            */
            M = ulRequestClk * N * 1000 / pPLL->inputFreq;
            M = roundedDiv(M, 1000);
        
            pllClk = pPLL->inputFreq * M / N; /* Calculate the actual clock for a given M & N */
            diff = absDiff(pllClk, ulRequestClk); /* How much are we different from the requirement */
        
            if (diff < bestDiff)
            {
                bestDiff = diff;

                /* Store M and N values */
                pPLL->M = M;
                pPLL->N = N;
                pPLL->divBy2 = i;
            }
        }

        /* Try (ulRequestClk x 2) */
        ulRequestClk *= 2;
    }

    /* Restore input frequency from Khz to hz unit */
    pPLL->inputFreq *= 1000;

    /* Return actual frequency that the PLL can set */
    return pPLL->divBy2 == 0 ? (pPLL->inputFreq * pPLL->M / pPLL->N) : (pPLL->inputFreq * pPLL->M / pPLL->N / 2);
}        

/*
 * Given a requested clock, this function calculates the value of divider clock
 * to be put in Power Mode Clock register (0x00003c of SM50x data book).
 * 
 * Input: Pointer to a pll_value_t structure with the following fields set properly:
 *        1) clockType
 *        2) inputFreq
 *
 * Output: Fill up the empty fields of structure pll_value_t according to clockType.
 *
 * Note: 
 * 1) This is the only clock type for SM501.
 * 2) SM502 panel timing can use programmable PLL.
 *
 */
unsigned long calcDividerClock(
unsigned long ulRequestClk, /* Requested clock (pixel, memory or master) in Hz unit */
pll_value_t *pPLL          /* Structure to hold the value to be set in Power Mode Clock register */
)
{
    unsigned long pll1, pll2, pllDiff, dividerClk, diff;
    unsigned short divider, divider_limit, shift;
    unsigned long best_diff = 0xffffffff; /* biggest 32 bit unsigned number */

    /* Sanity check */
    if (pPLL->inputFreq < MHz(5) || pPLL->inputFreq > MHz(100))
    {
        /* Input clock frequency to PLL has not been set properly or out of range.
           We have nothing to calculate. */
        return 0;
    }

    /* Convert everything in Khz range in order to avoid overflow */
    pPLL->inputFreq /= 1000;
    ulRequestClk /= 1000;

    /* Input clock will pass through two PLL's. One PLL multiply the input frequency
       by 12 and the other PLL multiply it by 14.
       We need to choose the best fit (either x12 or x14) for the requested frequency.
       Note that reference design uses 24 Mhz input clock.
       Therefore, output of Pll 1 is 288 Mhz and Pll 2 is 336 Mhz in SMI demo board.
    */
    pll1 = pPLL->inputFreq * 12;
    pll2 = pPLL->inputFreq * 14; 
    pllDiff = pll2 - pll1;

    /* For panel clock, try divider 1, 3 and 5, while all others only have 1 and 3 */
    if ( pPLL->clockType == P1XCLK || pPLL->clockType == P2XCLK )
        divider_limit = 5;
    else
        divider_limit = 3;

    for(dividerClk = pll1; dividerClk <= pll2; dividerClk += pllDiff)
    {
        for(divider = 1; divider <= divider_limit; divider += 2)
        {
            /* Try all 8 shift values. */
            for(shift = 0; shift < 8; shift++)
            {
                /* Calculate difference with requested clock. */
                diff = absDiff((roundedDiv(dividerClk, divider << shift)), ulRequestClk);

                /* If the difference is less than the current, use it. */
                if(diff < best_diff)
                {
                    /* Store best difference. */
                    best_diff = diff;

                    /*  Store clock values. */
                    pPLL->dividerClk = dividerClk;
                    pPLL->divider = divider;
                    pPLL->shift = shift;
                }
            }
        }
    }

    /* Restore frequency from Khz to hz unit */
    pPLL->dividerClk *= 1000; 
    pPLL->inputFreq *= 1000;

    /* Return actual frequency that the PLL can set */
    return pPLL->dividerClk / (pPLL->divider << pPLL->shift);
}

/*
 * This functions set up the proper values in the PLL structure.
 * Input: Requested pixel clock.
 * Output: Calculate the values to be put in divider clock or programmable PLL.
 * Return: The actual clock that SM50x is able to set up.
 *
 */
unsigned long calcPllValue(unsigned long ulPixelClock, pll_value_t *pPLL)
{
    /* Init PLL structure to know states */
    pPLL->dividerClk = 0;
    pPLL->divider = 0;
    pPLL->shift = 0;
    pPLL->M = 0;
    pPLL->N = 0;
    pPLL->divBy2 = 0;

    if (pPLL->clockType == PANEL_PLL)
    {
        /* Use the programmable PLL as panel pixel clock (new for SM502) */
        return( calcProgrammablePllCtrl(ulPixelClock, pPLL));
    }
    else if (pPLL->clockType == PANEL_PLL_2X)
    {
        /* Use the programmable PLL to calculate 2X clock for TV */
        return( calcProgrammablePllCtrl(ulPixelClock * 2, pPLL) / 2);
    }
    else if (pPLL->clockType == P2XCLK || pPLL->clockType == V2XCLK)
    {
        /* Use 2x clock in the divider clock (valid for both 501 and 502) */
        return( calcDividerClock(ulPixelClock * 2, pPLL) / 2);
    }
    else
    {
        /* MCLK, M1XCLK, PIXCLK and V1XCLK comes to here. */
        return( calcDividerClock(ulPixelClock, pPLL));
    }
}

/*
 * Set up the corresponding bit field of the Power Mode X Clock register.
 * Note that this function only updates the affected bit fields. Others will be untouched.
 *
 * Input: Pointer to PLL structure with type and values set up properly.
 *        Usually, calcPllValue() function will be called before this to calculate the values first.
 *
 */
unsigned long formatModeClockReg(pll_value_t *pPLL)
{
    unsigned long ulModeClockReg;
    unsigned long ulModeClockField;

    ulModeClockReg = peekRegisterDWord(CURRENT_POWER_CLOCK);

    if (pPLL->clockType==PANEL_PLL)
    {
        /* If use programmable PLL, disable divider clock and
           disable 2X clock.
         */
        ulModeClockReg &= FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_DIS)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, SEL)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_SELECT)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_DIVIDER)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_SHIFT);

        ulModeClockField = 
            FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_DIS, 1X)
          | FIELD_SET(0, CURRENT_POWER_CLOCK, SEL, PLL);
    }
    else if (pPLL->clockType==PANEL_PLL_2X)
    {
        /* If use programmable PLL x 2, disable divider clock.
         */
        ulModeClockReg &= FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_DIS)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, SEL)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_SELECT)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_DIVIDER)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_SHIFT);

        ulModeClockField = 
            FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_DIS, NORMAL)
          | FIELD_SET(0, CURRENT_POWER_CLOCK, SEL, PLL);
    }
    else if (pPLL->clockType==P2XCLK || pPLL->clockType==P1XCLK)
    {
        /* Use either 1X or 2X panel divider clock */
        ulModeClockReg &= FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_DIS)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, SEL)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_SELECT)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_DIVIDER)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, P2XCLK_SHIFT);

        ulModeClockField =
            FIELD_SET(0, CURRENT_POWER_CLOCK, SEL, PXCLK)
          | (pPLL->clockType == P2XCLK
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_DIS, NORMAL)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_DIS, 1X))
          | (pPLL->dividerClk == (pPLL->inputFreq * 12)
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_SELECT, 12X)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_SELECT, 14X))
          | (pPLL->divider == 1
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_DIVIDER, 1)
            : (pPLL->divider == 3
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_DIVIDER, 3)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, P2XCLK_DIVIDER, 5)))
          | FIELD_VALUE(0, CURRENT_POWER_CLOCK, P2XCLK_SHIFT, pPLL->shift);

    }
    else if (pPLL->clockType==V2XCLK || pPLL->clockType==V1XCLK)
    { 
        /* Use either 1x or 2x CRT divider clock */
        ulModeClockReg &= FIELD_CLEAR(CURRENT_POWER_CLOCK, V2XCLK_DIS)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, V2XCLK_SELECT)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, V2XCLK_DIVIDER)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, V2XCLK_SHIFT);

        ulModeClockField =
            (pPLL->clockType == V2XCLK
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, V2XCLK_DIS, NORMAL)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, V2XCLK_DIS, 1X))
          | (pPLL->dividerClk == (pPLL->inputFreq * 12)
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, V2XCLK_SELECT, 12X)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, V2XCLK_SELECT, 14X))
          | (pPLL->divider == 1
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, V2XCLK_DIVIDER, 1)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, V2XCLK_DIVIDER, 3))
          | FIELD_VALUE(0, CURRENT_POWER_CLOCK, V2XCLK_SHIFT, pPLL->shift);
    }
    else if (pPLL->clockType==MCLK)
    {
        /* 1X Master Clock */
        ulModeClockReg &= FIELD_CLEAR(CURRENT_POWER_CLOCK, MCLK_SELECT)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, MCLK_DIVIDER)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, MCLK_SHIFT);

        ulModeClockField =
             (pPLL->dividerClk == (pPLL->inputFreq * 12)
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, MCLK_SELECT, 12X)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, MCLK_SELECT, 14X))
          | (pPLL->divider == 1
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, MCLK_DIVIDER, 1)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, MCLK_DIVIDER, 3))
          | FIELD_VALUE(0, CURRENT_POWER_CLOCK, MCLK_SHIFT, pPLL->shift);
    }
    else
    {
        /* 1X Memroy Clock */
        ulModeClockReg &= FIELD_CLEAR(CURRENT_POWER_CLOCK, M2XCLK_SELECT)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, M2XCLK_DIVIDER)
                       &  FIELD_CLEAR(CURRENT_POWER_CLOCK, M2XCLK_SHIFT);

        ulModeClockField =
             (pPLL->dividerClk == (pPLL->inputFreq * 12)
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, M2XCLK_SELECT, 12X)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, M2XCLK_SELECT, 14X))
          | (pPLL->divider == 1
            ? FIELD_SET(0, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER, 1)
            : FIELD_SET(0, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER, 3))
          | FIELD_VALUE(0, CURRENT_POWER_CLOCK, M2XCLK_SHIFT, pPLL->shift);
    }

    return(ulModeClockReg | ulModeClockField);
}

/*
 * Set up the corresponding bit field of the programmable PLL register.
 *
 * Input: Pointer to PLL structure with type and values set up properly.
 *        Usually, calcPllValue() function will be called before this to calculate the values first.
 *
 */
unsigned long formatPllReg(pll_value_t *pPLL)
{
    unsigned long ulPllReg = 0;

    ulPllReg =
        FIELD_SET(0, PROGRAMMABLE_PLL_CONTROL, TSTOE, DISABLE)
      | FIELD_SET(0, PROGRAMMABLE_PLL_CONTROL, PON,   ON)
      | FIELD_SET(0, PROGRAMMABLE_PLL_CONTROL, SEL, CRYSTAL)
      | FIELD_VALUE(0, PROGRAMMABLE_PLL_CONTROL, K, pPLL->divBy2)
      | FIELD_VALUE(0, PROGRAMMABLE_PLL_CONTROL, N, pPLL->N)
      | FIELD_VALUE(0, PROGRAMMABLE_PLL_CONTROL, M, pPLL->M);

    return(ulPllReg);
}

/*
 * This function set up the memory clock.
 *
 * Input: Frequency in MHz unit.
 */
void setMemoryClock(unsigned long megaHz)
{
    pll_value_t pll;
    unsigned long ulFreq;

    /* Set up PLL structure */
    pll.inputFreq = DEFAULT_INPUT_CLOCK;
    pll.clockType = M1XCLK; /* M1X is memory clock. */

    ulFreq = MHz(megaHz);

    /* Do not modify the clock if it is not requested (ulFreq == 0) */
    if (ulFreq != 0)
    {
        calcPllValue(ulFreq, &pll);
        setCurrentClock(formatModeClockReg(&pll));
    }
}

/*
 * This function set up the master clock (MCLK).
 *
 * Input: Frequency in MHz unit.
 */
void setMasterClock(unsigned long megaHz)
{
    pll_value_t pll;
    unsigned long ulFreq;

    /* Set up PLL structure */
    pll.inputFreq = DEFAULT_INPUT_CLOCK;
    pll.clockType = MCLK;

    ulFreq = MHz(megaHz);
    
    /* Do not modify the clock if it is not requested (ulFreq == 0) */
    if (ulFreq != 0)
    {
        calcPllValue(ulFreq, &pll);
        setCurrentClock(formatModeClockReg(&pll));
    }
}

/*
 * This function get the Panel Pixel Clock value.
 *
 * Output:
 *      The Panel Pixel Clock value in whole number.
 */
unsigned long getPanelClock()
{
    unsigned long value, multiplier, divider, clockValue = 0;
    
    value = peekRegisterDWord(CURRENT_POWER_CLOCK);
    
    /* Calculate the multiplier */
    if (FIELD_GET(value, CURRENT_POWER_CLOCK, P2XCLK_DIS) == CURRENT_POWER_CLOCK_P2XCLK_DIS_NORMAL)
        multiplier = 2;
    else
        multiplier = 1;
    
    /* Different calculation when using programmable PLL and using fixed PLL with frequency divider */
    if (FIELD_GET(value, CURRENT_POWER_CLOCK, SEL) == CURRENT_POWER_CLOCK_SEL_PLL)
    {
        /* Calculate the PLL value only when the PLL is enabled */
        value = peekRegisterDWord(PROGRAMMABLE_PLL_CONTROL);
        if (FIELD_GET(value, PROGRAMMABLE_PLL_CONTROL, PON) == PROGRAMMABLE_PLL_CONTROL_PON_ON)
        {
            unsigned long mValue, nValue, kValue;
            
            mValue = FIELD_GET(value, PROGRAMMABLE_PLL_CONTROL, M);
            nValue = FIELD_GET(value, PROGRAMMABLE_PLL_CONTROL, N);
            if (FIELD_GET(value, PROGRAMMABLE_PLL_CONTROL, K) == PROGRAMMABLE_PLL_CONTROL_K_DISABLE)
                kValue = 1;
            else
                kValue = 2;
                
            clockValue = (mValue * DEFAULT_INPUT_CLOCK) / (kValue * nValue);
        } 
    }
    else
    {
        /* Calculate the clock value based on the fixed frequency and frequency divider */
        if (FIELD_GET(value, CURRENT_POWER_CLOCK, P2XCLK_SELECT) == 
                      CURRENT_POWER_CLOCK_P2XCLK_SELECT_12X)
            clockValue = DEFAULT_INPUT_CLOCK * 12;
        else
            clockValue = DEFAULT_INPUT_CLOCK * 14;
            
        switch (FIELD_GET(value, CURRENT_POWER_CLOCK, P2XCLK_DIVIDER))
        {
            case CURRENT_POWER_CLOCK_P2XCLK_DIVIDER_1:
                divider = 1;
                break;
            case CURRENT_POWER_CLOCK_P2XCLK_DIVIDER_3:
                divider = 3;
                break;
            case CURRENT_POWER_CLOCK_P2XCLK_DIVIDER_5:
                divider = 5;
                break;
        }
        
        divider = divider << (FIELD_GET(value, CURRENT_POWER_CLOCK, P2XCLK_SHIFT));
        clockValue = clockValue / divider; 
    }
    
    return (clockValue * multiplier);
}

/*
 * This function get the CRT Pixel Clock value.
 *
 * Output:
 *      The CRT Pixel Clock value in whole number.
 */
unsigned long getCRTClock()
{
    unsigned long value, multiplier, divider, clockValue = 0;
    
    value = peekRegisterDWord(CURRENT_POWER_CLOCK);
    
    /* Calculate the multiplier */
    if (FIELD_GET(value, CURRENT_POWER_CLOCK, V2XCLK_DIS) == CURRENT_POWER_CLOCK_P2XCLK_DIS_NORMAL)
        multiplier = 2;
    else
        multiplier = 1;
    
    /* Calculate the clock value based on the fixed frequency and frequency divider */
    if (FIELD_GET(value, CURRENT_POWER_CLOCK, V2XCLK_SELECT) == 
                  CURRENT_POWER_CLOCK_V2XCLK_SELECT_12X)
        clockValue = DEFAULT_INPUT_CLOCK * 12;
    else
        clockValue = DEFAULT_INPUT_CLOCK * 14;
        
    switch (FIELD_GET(value, CURRENT_POWER_CLOCK, V2XCLK_DIVIDER))
    {
        case CURRENT_POWER_CLOCK_V2XCLK_DIVIDER_1:
            divider = 1;
            break;
        case CURRENT_POWER_CLOCK_V2XCLK_DIVIDER_3:
            divider = 3;
            break;
    }
    
    divider = divider << (FIELD_GET(value, CURRENT_POWER_CLOCK, V2XCLK_SHIFT));
    clockValue = clockValue / divider;
    
    return (clockValue * multiplier);
}

/*
 * This function gets the Master Clock value.
 *
 * Output:
 *      The Master Clock value in whole number.
 */
unsigned long getMasterClock()
{
    unsigned long value, divider, clockValue = 0;
    
    value = peekRegisterDWord(CURRENT_POWER_CLOCK);
    
    /* Calculate the clock value based on the fixed frequency and frequency divider */
    if (FIELD_GET(value, CURRENT_POWER_CLOCK, MCLK_SELECT) == CURRENT_POWER_CLOCK_MCLK_SELECT_12X)
        clockValue = DEFAULT_INPUT_CLOCK * 12;
    else
        clockValue = DEFAULT_INPUT_CLOCK * 14;
        
    switch (FIELD_GET(value, CURRENT_POWER_CLOCK, MCLK_DIVIDER))
    {
        case CURRENT_POWER_CLOCK_MCLK_DIVIDER_1:
            divider = 1;
            break;
        case CURRENT_POWER_CLOCK_MCLK_DIVIDER_3:
            divider = 3;
            break;
    }
    
    divider = divider << (FIELD_GET(value, CURRENT_POWER_CLOCK, MCLK_SHIFT));
    
    return (clockValue / divider);
}

/*
 * This function gets the Memory Clock value.
 *
 * Output:
 *      The Memory Clock value in whole number.
 */
unsigned long getMemoryClock()
{
    unsigned long value, divider, clockValue = 0;
    
    value = peekRegisterDWord(CURRENT_POWER_CLOCK);
    
    /* Calculate the clock value based on the fixed frequency and frequency divider */
    if (FIELD_GET(value, CURRENT_POWER_CLOCK, M2XCLK_SELECT) == 
                  CURRENT_POWER_CLOCK_M2XCLK_SELECT_12X)
        clockValue = DEFAULT_INPUT_CLOCK * 12;
    else
        clockValue = DEFAULT_INPUT_CLOCK * 14;
        
    switch (FIELD_GET(value, CURRENT_POWER_CLOCK, M2XCLK_DIVIDER))
    {
        case CURRENT_POWER_CLOCK_M2XCLK_DIVIDER_1:
            divider = 1;
            break;
        case CURRENT_POWER_CLOCK_M2XCLK_DIVIDER_3:
            divider = 3;
            break;
    }
    
    divider = divider << (FIELD_GET(value, CURRENT_POWER_CLOCK, M2XCLK_SHIFT));
    
    return (clockValue / divider);
}
