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
#include "ddk502_chip.h"
#include "ddk502_clock.h"
#include "ddk502_hardware.h"
#include "ddk502_mode.h"
#include "ddk502_power.h"
#include "ddk502_help.h"
#include "../smi_common.h"
/*
 *  Timing parameter for some popular modes.
 *  Note that this table is made according to standard VESA parameters for the
 *  popular modes.
 */
static mode_parameter_t gModeParamTable[] =
{
/* 320 x 240 */
 { 352, 320, 335,  8, NEG, 265, 240, 254, 2, NEG,  5600000, 15909, 60, NEG},

/* 400 x 300 */
 { 528, 400, 420, 64, NEG, 314, 300, 301, 2, NEG,  9960000, 18864, 60, NEG},

/* 640 x 480  [4:3] */
 { 800, 640, 656, 96, NEG, 525, 480, 490, 2, NEG, 25175000, 31500, 60, NEG},
 { 840, 640, 680, 64, NEG, 500, 480, 481, 3, NEG, 31500000, 37500, 75, NEG},
 { 832, 640, 700, 56, NEG, 509, 480, 481, 3, NEG, 36000000, 43269, 85, NEG},

/* 720 x 480  [3:2] */
 { 889, 720, 738,108, POS, 525, 480, 490, 2, NEG, 28000000, 31496, 60, NEG},

/* 720 x 540  [4:3] */
 { 886, 720, 740, 96, POS, 576, 540, 545, 2, POS, 30600000, 34537, 60, NEG},

/* 800 x 480  [5:3] */
 { 973, 800, 822, 56, POS, 524, 480, 490, 2, NEG, 30600000, 31449, 60, NEG},

/* 800 x 600  [4:3] */
 {1062, 800, 840,128, POS, 628, 600, 601, 4, POS, 40000000, 37665, 60, NEG},
 {1056, 800, 816, 80, POS, 625, 600, 601, 3, POS, 49500000, 46875, 75, NEG},
 {1054, 800, 842, 64, POS, 625, 600, 601, 3, POS, 56000000, 53131, 85, NEG},

/* 960 x 720  [4:3] */
 {1245, 960, 992, 64, POS, 750, 720, 721, 3, POS, 56000000, 44980, 60, NEG},
      
/* 1024 x 600  [16:9] 1.7 */
 {1313,1024,1064,104, POS, 622, 600, 601, 3, POS, 49000000, 37319, 60, NEG},
     
/* 1024 x 768  [4:3] */
 {1340,1024,1060,136, NEG, 809, 768, 772, 6, NEG, 65000000, 48507, 60, NEG},
 {1337,1024,1072, 96, NEG, 808, 768, 780, 3, NEG, 81000000, 60583, 75, NEG},
 {1376,1024,1070, 96, POS, 808, 768, 769, 3, POS, 94500000, 68677, 85, NEG},
  
/* 1152 x 864  [4:3] */
 {1475,1152,1208, 96, NEG, 888, 864, 866, 3, NEG, 78600000, 53288, 60, NEG},

/* 1280 x 720 [16:9] */
 {1664,1280,1336,136, POS, 746, 720, 721, 3, POS, 74481000, 44760, 60, NEG},

/* 1280 x 768  [5:3] */
 {1678,1280,1350,136, POS, 795, 768, 769, 3, POS, 80000000, 47676, 60, NEG},

/* 1280 x 800  [8:5] */
 {1650,1280,1344,136, NEG, 824, 800, 800, 3, NEG, 81600000, 49455, 60, NEG},

/* 1280 x 960  [4:3] */
 {1618,1280,1330, 96, NEG, 977, 960, 960, 2, NEG, 94500000, 59259, 60, NEG},
    
/* 1280 x 1024 [5:4] */
 {1626,1280,1332,112, POS,1046,1024,1024, 2, POS,102000000, 62731, 60, NEG},

/* 1360 x 768 [16:9] */
 {1776,1360,1424,144, POS, 795, 768, 769, 3, POS, 84715000, 47700, 60, NEG},
/* 1366 x 768  [16:9] */
 {1722,1366,1424,112, NEG, 784, 768, 769, 3, NEG, 81000000, 47038, 60, NEG},

/* 1440 x 900 */
 {1904,1440,1520,152, NEG, 932, 900, 901, 3, POS,106470000, 55919, 60, NEG},
 
/* 1440 x 960 [16:9] */
 {1920,1440,1528,152, POS, 994, 960, 961, 3, POS,114509000, 59640, 60, NEG},

/* 1600 x 1200 [4:3] */
/* Not supported yet.
 {2160,1600,1724,112, NEG,1220,1200,1200, 2, NEG,158000000, 73148, 60, POS},
 */

/* End of table. */
 { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG},
};

/* Local variable to store the current set mode. 
   Note:
        The current set mode timing does not necessarily matching the mode parameter table. 
        Instead, the value is based on the actual values of the mode being set after
        calculation and adjustment.
 */
static mode_parameter_t gPanelCurrentModeParam = { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG};
static mode_parameter_t gCRTCurrentModeParam = { 0, 0, 0, 0, NEG, 0, 0, 0, 0, NEG, 0, 0, 0, NEG};

/* Flag to force 2x Display clock control */
static unsigned char gForce2xDisplayClock = 0;

/* 
 * Return a point to the gModeParamTable.
 * Function in other files used this to get the mode table pointer.
 */
mode_parameter_t *getStockModeParamTable()
{
    return(gModeParamTable);
}

/*
 * Return the size of the Stock Mode Param Table
 */
unsigned long getStockModeParamTableSize()
{
    return (sizeof(gModeParamTable) / sizeof(mode_parameter_t) - 1);
}

/* 
 * This function returns the current mode.
 */
mode_parameter_t getCurrentModeParam(disp_control_t dispCtrl)
{
    if (dispCtrl == PANEL_CTRL)
        return gPanelCurrentModeParam;
    else
        return gCRTCurrentModeParam;
}

/* 
 * This function sets the flag to force the display clock to use 2x.
 * Each module that needs to use 2x has to enable it by setting to 1 and 
 * disable it by setting to 0. It is the caller module's responsibility
 * to set it to 0 when they don't use the 2x clock anymore.
 */
void set2xDisplayClock(unsigned char ucForce2xDisplayClock)
{
    if (ucForce2xDisplayClock)
        gForce2xDisplayClock++;
    else
    {
        if (gForce2xDisplayClock)
            gForce2xDisplayClock--;
    }
}

/* 
 * This function checks either the 2x Display Clock flag is enabled or not
 */
unsigned char get2xDisplayClock(void)
{
    return (gForce2xDisplayClock ? 1 : 0);
}

/* This function get the Display Clock Multiplier */
unsigned char getDisplayClkMultiplier(disp_control_t dispControl)
{
    unsigned long ulCurrentClock;
    
    ulCurrentClock = peekRegisterDWord(CURRENT_POWER_CLOCK);
    if (dispControl == PANEL_CTRL)
    {
        if (FIELD_GET(ulCurrentClock, CURRENT_POWER_CLOCK, P2XCLK_DIS) == CURRENT_POWER_CLOCK_P2XCLK_DIS_1X)
            return 1;
    }
    else
    {
        if (FIELD_GET(ulCurrentClock, CURRENT_POWER_CLOCK, V2XCLK_DIS) == CURRENT_POWER_CLOCK_V2XCLK_DIS_1X)
            return 1;  
    }
    
    /* Default is 2 since SM501 chip */
    return 2;
}

/*
 *  Locate in-stock parameter table for the requested mode.
 *  Success: return a pointer to the mode_parameter_t entry.
 *  Fail: a NULL pointer.
 *
 */
mode_parameter_t *findVesaModeParam(
unsigned long width, 
unsigned long height, 
unsigned long refresh_rate)
{
    mode_parameter_t *mode_table = gModeParamTable;

    /* Walk the entire mode table. */
    while (mode_table->pixel_clock != 0)
    {
        if ((mode_table->horizontal_display_end == width)
        && (mode_table->vertical_display_end == height)
        && (mode_table->vertical_frequency == refresh_rate))
        {
            return(mode_table);
        }
        mode_table++; /* Next entry */
    }

    /* No match, return NULL pointer */
    return((mode_parameter_t *)0);
}

/*
 *  Convert the VESA timing into possible Voyager timing.
 *  If actual pixel clock is not equal to Vesa timing pixel clock.
 *  other parameter like horizontal total and sync have to be changed.
 *
 *  Input: Pointer to a Vesa mode parameters.
 *         Pointer to a an empty mode parameter structure to be filled.
 *         Actual pixel clock generated by SMI hardware.
 *
 *  Output:
 *      1) Fill up input structure mode_parameter_t with possible timing for Voyager.
 */
long adjustVesaModeParam(
mode_parameter_t *pVesaMode, /* Pointer to Vesa mode parameter */
mode_parameter_t *pMode,     /* Pointer to Vogager mode parameter to be updated here */
unsigned long ulPClk         /* real pixel clock feasible by VGX */
)
{
    unsigned long blank_width, sync_start, sync_width;

    /* Senity check */
    if ( pVesaMode == (mode_parameter_t *)0 ||
         pMode     == (mode_parameter_t *)0 ||
         ulPClk  == 0)
    {
        return -1;
    }

    /* Copy VESA mode into Voyager mode. */
    *pMode = *pVesaMode;

    /* If VGX can generate the vesa reqiured pixel clock, nothing to change */
    if (ulPClk == pVesaMode->pixel_clock) return 0;

    pMode->pixel_clock = ulPClk; /* Update actual pixel clock into mode */

    /* Calculate the sync percentages of the VESA mode. */
    blank_width = pVesaMode->horizontal_total - pVesaMode->horizontal_display_end;
    sync_start = roundedDiv((pVesaMode->horizontal_sync_start -
                       pVesaMode->horizontal_display_end) * 100, blank_width);
    sync_width = roundedDiv(pVesaMode->horizontal_sync_width * 100, blank_width);

     /* Calculate the horizontal total based on the actual pixel clock and VESA line frequency. */
    pMode->horizontal_total = roundedDiv(pMode->pixel_clock,
                                    pVesaMode->horizontal_frequency);

    /* Calculate the sync start and width based on the VESA percentages. */
    blank_width = pMode->horizontal_total - pMode->horizontal_display_end;
    pMode->horizontal_sync_start = pMode->horizontal_display_end + roundedDiv(blank_width * sync_start, 100);
    pMode->horizontal_sync_width = roundedDiv(blank_width * sync_width, 100);

    /* Calculate the line and screen frequencies. */
    pMode->horizontal_frequency = roundedDiv(pMode->pixel_clock,
                                        pMode->horizontal_total);
    pMode->vertical_frequency = roundedDiv(pMode->horizontal_frequency,
                                      pMode->vertical_total);
    return 0;
}

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
)
{
    unsigned long value;

    /* Get the display status */
    if (dispControl == PANEL_CTRL)
    {
        if (FIELD_GET(peekRegisterDWord(PANEL_FB_ADDRESS), PANEL_FB_ADDRESS, STATUS) == PANEL_FB_ADDRESS_STATUS_PENDING)
            return 0;
    }
	else if (dispControl == CRT_CTRL)
    {
        if (FIELD_GET(peekRegisterDWord(CRT_FB_ADDRESS), CRT_FB_ADDRESS, STATUS) == CRT_FB_ADDRESS_STATUS_PENDING)
            return 0;
    }

    return (-1);
}

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
)
{
	if (dispControl == PANEL_CTRL)
	{
		/* Frame buffer base for this mode */
	    pokeRegisterDWord(PANEL_FB_ADDRESS,
	          FIELD_SET(0, PANEL_FB_ADDRESS, STATUS, PENDING)
	        | FIELD_SET(0, PANEL_FB_ADDRESS, EXT, LOCAL)
	        | FIELD_VALUE(0, PANEL_FB_ADDRESS, ADDRESS, ulBaseAddress));
	}
	else if (dispControl == CRT_CTRL)
	{
		/* Frame buffer base for this mode */
        pokeRegisterDWord(CRT_FB_ADDRESS,
              FIELD_SET(0, CRT_FB_ADDRESS, STATUS, PENDING)
            | FIELD_SET(0, CRT_FB_ADDRESS, EXT, LOCAL)
            | FIELD_VALUE(0, CRT_FB_ADDRESS, ADDRESS, ulBaseAddress));
	}
}

/* 
 * Program the hardware for a specific video mode
 */
void programModeRegisters(
mode_parameter_t *pModeParam,   /* mode information about pixel clock, horizontal total, etc. */
unsigned long ulBpp,            /* Color depth for this mode */
unsigned long ulBaseAddress,    /* Offset in frame buffer */
unsigned long ulPitch,          /* Mode pitch value in byte: no of bytes between two lines. */
pll_value_t *pPLL               /* Pre-calculated values for the PLL */
)
{
    unsigned long ulTmpValue, ulReg, ulReservedBits;
    unsigned long palette_ram;
    unsigned long offset;

    /* Enable display power gate */
    ulTmpValue = peekRegisterDWord(CURRENT_POWER_GATE);
    ulTmpValue = FIELD_SET(ulTmpValue, CURRENT_POWER_GATE, DISPLAY, ENABLE);
    ddk502_setCurrentGate(ulTmpValue);

    if (pPLL->clockType==V1XCLK || pPLL->clockType==V2XCLK)
    {
        /* CRT display */
        setCurrentClock(formatModeClockReg(pPLL));

        /* Frame buffer base for this mode */
		setDisplayBaseAddress(CRT_CTRL, ulBaseAddress);

        /* Pitch value (Sometime, hardware people calls it Offset) */
        pokeRegisterDWord(CRT_FB_WIDTH,
              FIELD_VALUE(0, CRT_FB_WIDTH, WIDTH, ulPitch)
            | FIELD_VALUE(0, CRT_FB_WIDTH, OFFSET, ulPitch));

        pokeRegisterDWord(CRT_HORIZONTAL_TOTAL,
              FIELD_VALUE(0, CRT_HORIZONTAL_TOTAL, TOTAL, pModeParam->horizontal_total - 1)
            | FIELD_VALUE(0, CRT_HORIZONTAL_TOTAL, DISPLAY_END, pModeParam->horizontal_display_end - 1));

        pokeRegisterDWord(CRT_HORIZONTAL_SYNC,
              FIELD_VALUE(0, CRT_HORIZONTAL_SYNC, WIDTH, pModeParam->horizontal_sync_width)
            | FIELD_VALUE(0, CRT_HORIZONTAL_SYNC, START, pModeParam->horizontal_sync_start - 1));

        pokeRegisterDWord(CRT_VERTICAL_TOTAL,
              FIELD_VALUE(0, CRT_VERTICAL_TOTAL, TOTAL, pModeParam->vertical_total - 1)
            | FIELD_VALUE(0, CRT_VERTICAL_TOTAL, DISPLAY_END, pModeParam->vertical_display_end - 1));

        pokeRegisterDWord(CRT_VERTICAL_SYNC,
              FIELD_VALUE(0, CRT_VERTICAL_SYNC, HEIGHT, pModeParam->vertical_sync_height)
            | FIELD_VALUE(0, CRT_VERTICAL_SYNC, START, pModeParam->vertical_sync_start - 1));

        /* Set control register value */
        ulTmpValue =
            (pModeParam->vertical_sync_polarity == POS
            ? FIELD_SET(0, CRT_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, CRT_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW))
          | (pModeParam->horizontal_sync_polarity == POS
            ? FIELD_SET(0, CRT_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, CRT_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW))
          | FIELD_SET(0, CRT_DISPLAY_CTRL, SELECT, CRT)
          | FIELD_SET(0, CRT_DISPLAY_CTRL, TIMING, ENABLE)
          | FIELD_SET(0, CRT_DISPLAY_CTRL, PLANE, ENABLE)
          | (ulBpp == 8
            ? FIELD_SET(0, CRT_DISPLAY_CTRL, FORMAT, 8)
            : (ulBpp == 16
            ? FIELD_SET(0, CRT_DISPLAY_CTRL, FORMAT, 16)
            : FIELD_SET(0, CRT_DISPLAY_CTRL, FORMAT, 32)));

        ulReg = peekRegisterDWord(CRT_DISPLAY_CTRL)
              & FIELD_CLEAR(CRT_DISPLAY_CTRL, VSYNC_PHASE)
              & FIELD_CLEAR(CRT_DISPLAY_CTRL, HSYNC_PHASE)
              & FIELD_CLEAR(CRT_DISPLAY_CTRL, SELECT)
              & FIELD_CLEAR(CRT_DISPLAY_CTRL, TIMING)
              & FIELD_CLEAR(CRT_DISPLAY_CTRL, PLANE)
              & FIELD_CLEAR(CRT_DISPLAY_CTRL, FORMAT);

        pokeRegisterDWord(CRT_DISPLAY_CTRL, ulTmpValue | ulReg);

        /* Palette RAM. */
        palette_ram = CRT_PALETTE_RAM;
    }
    else 
    {
        /* Panel display: PANEL_PLL, PANEL_PLL_2X, P1XCLK or P2XCLK */

        setCurrentClock(formatModeClockReg(pPLL));

        /* Program panel PLL, if applicable */
        if (pPLL->clockType==PANEL_PLL || pPLL->clockType==PANEL_PLL_2X)
            pokeRegisterDWord(PROGRAMMABLE_PLL_CONTROL, formatPllReg(pPLL));

        /* Frame buffer base for this mode */
		setDisplayBaseAddress(PANEL_CTRL, ulBaseAddress);

        /* Pitch value (Sometime, hardware people calls it Offset) */
        pokeRegisterDWord(PANEL_FB_WIDTH,              FIELD_VALUE(0, PANEL_FB_WIDTH, WIDTH, ulPitch)           | FIELD_VALUE(0, PANEL_FB_WIDTH, OFFSET, ulPitch));

        pokeRegisterDWord(PANEL_WINDOW_WIDTH,              FIELD_VALUE(0, PANEL_WINDOW_WIDTH, WIDTH, pModeParam->horizontal_display_end)            | FIELD_VALUE(0, PANEL_WINDOW_WIDTH, X, 0));

        pokeRegisterDWord(PANEL_WINDOW_HEIGHT,              FIELD_VALUE(0, PANEL_WINDOW_HEIGHT, HEIGHT, pModeParam->vertical_display_end)            | FIELD_VALUE(0, PANEL_WINDOW_HEIGHT, Y, 0));

        pokeRegisterDWord(PANEL_PLANE_TL,              FIELD_VALUE(0, PANEL_PLANE_TL, TOP, 0)            | FIELD_VALUE(0, PANEL_PLANE_TL, LEFT, 0));

        pokeRegisterDWord(PANEL_PLANE_BR,               FIELD_VALUE(0, PANEL_PLANE_BR, BOTTOM, pModeParam->vertical_display_end - 1)            | FIELD_VALUE(0, PANEL_PLANE_BR, RIGHT, pModeParam->horizontal_display_end - 1));

        pokeRegisterDWord(PANEL_HORIZONTAL_TOTAL,              FIELD_VALUE(0, PANEL_HORIZONTAL_TOTAL, TOTAL, pModeParam->horizontal_total - 1)            | FIELD_VALUE(0, PANEL_HORIZONTAL_TOTAL, DISPLAY_END, pModeParam->horizontal_display_end - 1));

        pokeRegisterDWord(PANEL_HORIZONTAL_SYNC,              FIELD_VALUE(0, PANEL_HORIZONTAL_SYNC, WIDTH, pModeParam->horizontal_sync_width)            | FIELD_VALUE(0, PANEL_HORIZONTAL_SYNC, START, pModeParam->horizontal_sync_start - 1));

        pokeRegisterDWord(PANEL_VERTICAL_TOTAL,              FIELD_VALUE(0, PANEL_VERTICAL_TOTAL, TOTAL, pModeParam->vertical_total - 1)            | FIELD_VALUE(0, PANEL_VERTICAL_TOTAL, DISPLAY_END, pModeParam->vertical_display_end - 1));

        pokeRegisterDWord(PANEL_VERTICAL_SYNC,              FIELD_VALUE(0, PANEL_VERTICAL_SYNC, HEIGHT, pModeParam->vertical_sync_height)            | FIELD_VALUE(0, PANEL_VERTICAL_SYNC, START, pModeParam->vertical_sync_start - 1));

        /* Set control register value */
        ulTmpValue =
            (pModeParam->clock_phase_polarity == POS
            ? FIELD_SET(0, PANEL_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, PANEL_DISPLAY_CTRL, CLOCK_PHASE, ACTIVE_LOW))
          | (pModeParam->vertical_sync_polarity == POS
            ? FIELD_SET(0, PANEL_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, PANEL_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW))
          | (pModeParam->horizontal_sync_polarity == POS
            ? FIELD_SET(0, PANEL_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH)
            : FIELD_SET(0, PANEL_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW))
          | FIELD_SET(0, PANEL_DISPLAY_CTRL, TIMING, ENABLE)
          | FIELD_SET(0, PANEL_DISPLAY_CTRL, PLANE, ENABLE)
          | (ulBpp == 8
            ? FIELD_SET(0, PANEL_DISPLAY_CTRL, FORMAT, 8)
            : (ulBpp == 16
            ? FIELD_SET(0, PANEL_DISPLAY_CTRL, FORMAT, 16)
            : FIELD_SET(0, PANEL_DISPLAY_CTRL, FORMAT, 32)));

        /* Added some masks to mask out the reserved bits. 
         * Sometimes, the reserved bits are set/reset randomly when 
         * writing to the PANEL_DISPLAY_CTRL, therefore, the register
         * reserved bits are needed to be masked out. RA 2008.01.02
         */
        ulReservedBits = FIELD_SET(0, PANEL_DISPLAY_CTRL, RESERVED_1_MASK, ENABLE) |
                         FIELD_SET(0, PANEL_DISPLAY_CTRL, RESERVED_2_MASK, ENABLE);

        ulReg = (peekRegisterDWord(PANEL_DISPLAY_CTRL) & ~ulReservedBits)
              & FIELD_CLEAR(PANEL_DISPLAY_CTRL, CLOCK_PHASE)
              & FIELD_CLEAR(PANEL_DISPLAY_CTRL, VSYNC_PHASE)
              & FIELD_CLEAR(PANEL_DISPLAY_CTRL, HSYNC_PHASE)
              & FIELD_CLEAR(PANEL_DISPLAY_CTRL, TIMING)
              & FIELD_CLEAR(PANEL_DISPLAY_CTRL, VERTICAL_PAN)
              & FIELD_CLEAR(PANEL_DISPLAY_CTRL, HORIZONTAL_PAN)
              & FIELD_CLEAR(PANEL_DISPLAY_CTRL, PLANE)
              & FIELD_CLEAR(PANEL_DISPLAY_CTRL, FORMAT);

        pokeRegisterDWord(PANEL_DISPLAY_CTRL, ulTmpValue | ulReg);

        /* May a hardware bug or just my test chip (not confirmed).
         * PANEL_DISPLAY_CTRL register seems requiring few writes
         * before a value can be succesfully written in.
         * Added some masks to mask out the reserved bits. RA 2007.10.17
         */
        while((peekRegisterDWord(PANEL_DISPLAY_CTRL) & ~ulReservedBits) != (ulTmpValue|ulReg))
        {
            pokeRegisterDWord(PANEL_DISPLAY_CTRL, ulTmpValue | ulReg);
        }

        /* Palette RAM */
        palette_ram = PANEL_PALETTE_RAM;
    }

    /* In case of 8-bpp, fill palette */
    if (ulBpp==8)
    {
        /* Start with RGB = 0,0,0. */
        unsigned char red = 0, green = 0, blue = 0;
        unsigned long gray = 0;
        for (offset = 0; offset < 256 * 4; offset += 4)
        {
            /* Store current RGB value. */
            pokeRegisterDWord(palette_ram + offset, gray
                                ? RGB((gray + 50) / 100,
                                      (gray + 50) / 100,
                                      (gray + 50) / 100)
                                : RGB(red, green, blue));

            if (gray)
            {
                /* Walk through grays (40 in total). */
                gray += 654;
            }

            else
            {
                /* Walk through colors (6 per base color). */
                if (blue != 255)
                {
                    blue += 51;
                }
                else if (green != 255)
                {
                    blue = 0;
                    green += 51;
                }
                else if (red != 255)
                {
                    green = blue = 0;
                    red += 51;
                }
                else
                {
                    gray = 1;
                }
            }
        }
    }

    /* For 16- and 32-bpp,  fill palette with gamma values. */
    else
    {
        /* Start with RGB = 0,0,0. */
        ulTmpValue = 0x000000;
        for (offset = 0; offset < 256 * 4; offset += 4)
        {
            pokeRegisterDWord(palette_ram + offset, ulTmpValue);

            /* Advance RGB by 1,1,1. */
            ulTmpValue += 0x010101;
        }
    }
}

/* 
 * This function gets the available clock type
 *
 */
clock_type_t getClockType(disp_control_t dispCtrl)
{
    clock_type_t clockType;
    
    switch(getChipType())
    {
        case SM501:
        {
            clockType = (dispCtrl == PANEL_CTRL)? P2XCLK:V2XCLK;
        }
        break;

        case SM107:
        case SM502:
        {
            clockType = (dispCtrl == PANEL_CTRL)? PANEL_PLL:V1XCLK;
        }
        break;

        default:
        {
            clockType = P1XCLK;
        }
    }
    
    /* For 2x Display clock, there are currently only 2 choices (P2XCLK or V2XCLK). 
       For future enhancement, we can use 2x on the PANEL_PLL. More code modification is
       needed.
     */
    if (gForce2xDisplayClock)
    {
        clockType = (dispCtrl == PANEL_CTRL)? P2XCLK:V2XCLK;
    }
        
    return clockType;
}

/*
 * Input:
 *     1) pLogicalMode contains information such as x, y resolution and bpp.
 *     2) A user defined parameter table for the mode.
 *
 * This function calculate and programs the hardware to set up the
 * requested mode.
 *
 * This function allows the use of user defined parameter table if
 * predefined Vesa parameter table (gModeParamTable) does not fit.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
long setCustomMode(logicalMode_t *pLogicalMode, mode_parameter_t *pUserModeParam)
{
    mode_parameter_t pModeParam; /* physical parameters for the mode */
    pll_value_t pll;
    unsigned long ulActualPixelClk, ulAddress, ulTemp;

    /*
     * Minimum check on mode base address.
     * At least it shouldn't be bigger than the size of frame buffer.
     */
    if (ddk502_getFrameBufSize() <= pLogicalMode->baseAddress)
        return -1;

    /*
     * Set up PLL, a structure to hold the value to be set in clocks.
     */
    pll.inputFreq = DEFAULT_INPUT_CLOCK; /* Defined in CLOCK.H */

    /* Get the Clock Type */
    pll.clockType = getClockType(pLogicalMode->dispCtrl);

    /* 
     * Call calcPllValue() to fill up the other fields for PLL structure.
     * Sometime, the chip cannot set up the exact clock required by User.
     * Return value from calcPllValue() gives the actual possible pixel clock.
     */
    ulActualPixelClk = calcPllValue(pUserModeParam->pixel_clock, &pll);

    /* 
     * Adjust Vesa mode parameter to feasible mode parameter for SMI hardware.
     */
    if (adjustVesaModeParam(pUserModeParam, &pModeParam, ulActualPixelClk) != 0 )
    {
        return -1;
    }

    /* If calling function don't have a preferred pitch value, 
       work out a 16 byte aligned pitch value.
    */
    if (pLogicalMode->pitch == 0)
    {
        /* 
         * Pitch value calculation in Bytes.
         * Usually, it is (screen width) * (byte per pixel).
         * However, there are cases that screen width is not 16 pixel aligned, which is
         * a requirement for some OS and the hardware itself.
         * For standard 4:3 resolutions: 320, 640., 800, 1024 and 1280, they are all
         * 16 pixel aligned and pitch is simply (screen width) * (byte per pixel).
         *   
         * However, 1366 resolution, for example, has to be adjusted for 16 pixel aligned.
         */

        ulTemp = (pLogicalMode->x + 15) & ~15; /* This calculation has no effect on 640, 800, 1024 and 1280. */
        pLogicalMode->pitch = ulTemp * (pLogicalMode->bpp / 8);
    }
#if 0
    /* Clean the video memory to 0 */
    ulTemp = pLogicalMode->pitch * pLogicalMode->y;
    for (ulAddress = 0; ulAddress < ulTemp; ulAddress+=4)
        pokeDWord(pLogicalMode->baseAddress + ulAddress, 0);
#endif
    /* Program the hardware to set up the mode. */
    programModeRegisters( 
        &pModeParam,
        pLogicalMode->bpp, 
        pLogicalMode->baseAddress, 
        pLogicalMode->pitch, 
        &pll);

    /* Adjust window width for virtual mode, if applied */
    if (pLogicalMode->virtual == 1 && pLogicalMode->dispCtrl == PANEL_CTRL)
    {
        pokeRegisterDWord(PANEL_WINDOW_WIDTH,
              FIELD_VALUE(0, PANEL_WINDOW_WIDTH, WIDTH, pLogicalMode->x)
            | FIELD_VALUE(0, PANEL_WINDOW_WIDTH, X, 0));

        pokeRegisterDWord(PANEL_WINDOW_HEIGHT,
              FIELD_VALUE(0, PANEL_WINDOW_HEIGHT, HEIGHT, pLogicalMode->y)
            | FIELD_VALUE(0, PANEL_WINDOW_HEIGHT, Y, 0));
    }

    /* Save the current mode */
    if (pLogicalMode->dispCtrl == PANEL_CTRL)
        gPanelCurrentModeParam = pModeParam;
    else
        gCRTCurrentModeParam = pModeParam;

    //by ilena
    pokeRegisterDWord(MISC_CTRL, FIELD_SET(peekRegisterDWord(MISC_CTRL),
                          MISC_CTRL, DAC_POWER, ENABLE));
    return(0);
}

/*
 * Input pLogicalMode contains information such as x, y resolution and bpp.
 * Refer to MODE.h for the details.
 *
 * This function calculate and programs the hardware to set up the
 * requested mode.
 *
 * Return: 0 (or NO_ERROR) if mode can be set successfully.
 *         -1 if any set mode error.
 */
_X_EXPORT long ddk502_setModeTiming(logicalMode_t *pLogicalMode)
{
    mode_parameter_t *pVesaModeParam; /* physical parameters for the mode */
    unsigned long modeX, modeY;

    if (pLogicalMode->virtual == 1 && pLogicalMode->dispCtrl == PANEL_CTRL)
    {
        /* Use panel size to set mode time */
        modeX = pLogicalMode->xLCD;
        modeY = pLogicalMode->yLCD;
    }
    else
    {
        /* Use resolution to set mode time */
        modeX = pLogicalMode->x;
        modeY = pLogicalMode->y;
    }

    /* 
     * Check if we already have physical timing parameter for this mode.
     */
    pVesaModeParam = (mode_parameter_t *)findVesaModeParam(modeX, modeY, pLogicalMode->hz);
    if (pVesaModeParam == (mode_parameter_t *)0)
        return -1;

    return(setCustomMode(pLogicalMode, pVesaModeParam));
}
