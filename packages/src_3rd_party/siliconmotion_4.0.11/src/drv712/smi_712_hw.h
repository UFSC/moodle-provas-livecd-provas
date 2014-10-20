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

#ifndef SMI_712_HW_INC
#define SMI_712_HW_INC
#include "../smi_common.h"
#include "../smi_driver.h"
/********************/
/* SMI712 REGISTERS */
/********************/
/*General Graphics Command Register 1*/
#define SCR17 0x17
/*System Control Registers*/
#define SCR18   0x18

/*Power Down Control Registers*/
#define PDR21 0x21
#define PDR22 0x22
/*Flat Panel Type Select*/
#define FPR30 0x30
/*Virtual Refresh and Auto Shut Down Control*/
#define FPR31 0x31
/*Dithering Engine Select, Polarity, and Expansion Control*/
#define FPR32 0x32
#define FPR33 0x33
#define FPR35 0x35
#define FPR36 0x36
#define FPR37 0x37
#define FPR38 0x38
#define FPR39 0x39
#define FPR3A 0x3a
#define FPR3B 0x3b
#define FPR40 0x40
#define FPR41 0x41
#define FPR42 0x42
#define FPR43 0x43
#define FPR44 0x44
#define FPR45 0x45
/*Panel M-Signal Control Register*/
#define FPR59 0x59
/*RAM LUT On/Off Control*/
#define CCR66 0x66
/*MCLK Numerator Register*/
#define CCR6A 0x6A
/*MCLK Denominator Register*/
#define CCR6B 0x6B
/*VCLK Numerator Register*/
#define CCR6C 0x6C
/*VCLK Denominator Register*/
#define CCR6D 0x6D
/*VCLK2 Numerator Register*/
#define CCR6E 0x6E
/*VCLK2 Denominator Register*/
#define CCR6F 0x6F
/*Scratch Pad Register 1*/
#define GPR70 0x70
/*Hardware Cursor Enable & PI/HWC Pattern Location High*/
#define PHR81 0x81

/*DPR*/
/*Source Row Pitch*/
#define DPR_10 0x10
/*Stretch Source Height Y*/
#define DPR_1C 0x1C
/*Color Compare*/
#define DPR_20 0x20
/*Color Compare Masks*/
#define DPR_24 0x24
/*Bit Mask*/
#define DPR_28 0x28
/*Scissors Left and Control*/
#define DPR_2C 0x2C
/*Scissors Right*/
#define DPR_30 0x30
/*XY Addressing Destination & Source Window Widths*/
#define DPR_3C 0x3C
/*Source Base Address*/
#define DPR_40 0x40
/*Destination Base Address*/
#define DPR_44 0x44

/*VPR*/
/*Miscellaneous Graphics and Video Control*/
#define VPR_00 0x00 
/*Data Source Start Address for Extended Graphics Modes*/
#define VPR_0C 0x0C
/*Data Source Width and Offset for Extended Graphics Modes*/ 
#define VPR_10 0x10

/*CPR*/
/*Capture Port Control*/ 
#define CPR_00 0x00 

/*Panel HW Video Control*/
#define FPRA0 0xA0

typedef struct
{    
	Bool    modeInit;
	CARD16	mode;

    CARD8 CR66;
    CARD8 SR17, SR18;
    CARD8 SR20, SR21, SR22, SR23, SR24;
    CARD8 SR30, SR31, SR32, SR34;
    CARD8 SR40, SR41, SR42, SR43, SR44, SR45, SR48, SR49, SR4A, SR4B, SR4C;
    CARD8 SR50, SR51, SR52, SR53, SR54, SR55, SR56, SR57, SR5A;
    CARD8 SR66, SR68, SR69, SR6A, SR6B, SR6C, SR6D, SR6E, SR6F;
    CARD8 SR81, SRA0;

    CARD8 CR30, CR33, CR33_2, CR3A;
    CARD8 CR40[14], CR40_2[14];
    CARD8 CR90[15], CR9F, CR9F_2;
    CARD8 CRA0[14];

    CARD8 smiDACMask, smiDacRegs[256][3];
    CARD8 smiFont[8192];

	CARD32 DPR10, DPR1C, DPR20, DPR24, DPR28, DPR2C, DPR30, DPR3C, DPR40, DPR44;
	CARD32 VPR00, VPR0C, VPR10;
	CARD32 CPR00;
	CARD32 FPR00_, FPR0C_, FPR10_;

	/* entity information */
	Bool DualHead;
	ScrnInfoPtr pSecondaryScrn;
	ScrnInfoPtr pPrimaryScrn;
	int lastInstance;
	
	/* shared resource */
	int mmio_require;
	volatile unsigned char * MMIOBase;	/* Base of MMIO */
	int MapSize;	                    /* how many mmio should map and unmap */
	int total_videoRam;			        /* memory count in bytes */

	/* vga stuffs */
	char * fonts;	
} SMI712RegRec, *SMI712RegPtr;
CARD8 VGAIN8_INDEX_712(SMIHWPtr pHw, int indexPort, int dataPort, CARD8 index);
void VGAOUT8_INDEX_712(SMIHWPtr pHw, int indexPort, int dataPort, CARD8 index, CARD8 data);
CARD8 VGAIN8_712(SMIHWPtr pHw, int port);
void VGAOUT8_712(SMIHWPtr pHw, int port, CARD8 data);
void save_reg_712(SMIHWPtr);
void restore_reg_712(SMIHWPtr);
void save_dpr_address(SMIHWPtr);
void wait_for_not_busy_712(SMIPtr);
void set_backlight_712(int x);
#endif  /*  ----- #ifndef SMI_712_HW_INC   -----*/
