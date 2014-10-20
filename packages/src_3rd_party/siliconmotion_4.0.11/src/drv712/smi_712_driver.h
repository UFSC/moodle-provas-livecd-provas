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

#ifndef SMI_712_INC 
#define SMI_712_INC

#include "../smi_common.h"
#include "ddk712/ddk712_mode.h"
#include "ddk712/ddk712_chip.h"
#include "smi_712_hw.h"
#include "../smi_dbg.h"

/* Maximum hardware cursor dimensions */
#define SMI712_MAX_CURSOR	32

#define SM712_REG_OFFSET 	0x400000
#define SM712_REG_SIZE 		0x400000
#define SM722_FB_OFFSET		0x200000
#define SM722_REG_SIZE		0x200000

typedef struct
{    
    CARD16 h_res;
    CARD16 v_res;
    CARD16 vsync;	//vertical refresh rate:only 60,75,85 is valid for sm712   
    CARD8 SVR[14];	//shadow vga register:svr40==>svr4d    
    CARD8 CCR[2];	//clock control register: ccr6c ccr6d    
}SMI712CrtTiming,*SMI712CrtTimingPtr;

typedef struct 
{
	CARD16 h_res;
	CARD16 v_res;
	CARD16 vsync;
	CARD8 FPR[14];	//FPR50 => FPR57 + FPR 5A
	CARD8 CCR[2];	//CCR6E && CCR6F
}SMI712PnlTiming,*SMI712PnlTimingPtr;

typedef struct
{
	int x, y, bpp;
	CARD16 mode;
}modeTable_t;



typedef struct {
	/* put common structure here */
	SMIHWRec base;
	/* put chip dependent stuffs here*/
	SMI712RegPtr pRegSave;
	/* */
	SMI712RegPtr pRegMode;
    Bool	ModeStructInit;	/* Flag indicating ModeReg has  been duped from console state */

	/* vga stuffs */
	char * 	fonts;
	CARD8 	SR18Value;	/* PDR#521: original SR18
						   value */
	CARD8 	SR21Value;	/* PDR#521: original SR21
						   value */
}SMI712HWRec,*SMI712HWPtr;

typedef struct {
	/* we best put base in header of structure*/
	SMIRec base;
	/* chip dependent stuffs of this pSmi*/
	int csc;/* use csc video or not */
	/*  cursor offset for old lynx serials  */					   
    CARD32		FBCursorOffset;	/* Cursor storage location */
}SMI712Rec,*SMI712Ptr;


SMIPtr sm712_genSMI(pointer priv,int);
void sm712_vgaAndpciInit(ScrnInfoPtr pScrn,int entityIndex);
void sm712_hardwareInit(SMIHWPtr pHw);
void sm712_entityInit(int,pointer);
void sm712_DetectMCLK(SMIPtr pSmi );
void sm712_entityEnter(int,pointer);
void sm712_entityLeave(int,pointer);
void sm712_getMapResult(ScrnInfoPtr);
void sm712_handleOptions(ScrnInfoPtr);
ModeStatus sm712_validMode(ScrnInfoPtr,DisplayModePtr);
void sm712_LoadPalette (ScrnInfoPtr , int , int *, LOCO * , VisualPtr );
void sm712_setMode(ScrnInfoPtr,DisplayModePtr);
void sm712_adjustFrame(ScrnInfoPtr,int);
void sm712_setDisplay(ScrnInfoPtr pScrn, DisplayModePtr mode);
void sm712_pcDeepmap(SMIHWPtr pHw);
void sm712_saveText(ScrnInfoPtr pScrn);
void sm712_saveFonts(SMIHWPtr pHw);
void sm712_restoreFonts(SMIHWPtr pHw);
void sm712_closeAllScreen(SMIHWPtr pHw);
int sm712_totalFB(SMIHWPtr pHw);
void sm712_I2CGetBits(I2CBusPtr b, int *clock, int *data);
void sm712_I2CPutBits(I2CBusPtr b, int clock,  int data);

extern void ddk712_set_mmio(volatile unsigned char * addr,int devid);
extern void ddk712_hw_init(init_parm_712 * param);
extern void ddk712_setModeTiming(int channel,int x,int y,int hz);
extern Bool SMI712_CrtcPreInit(ScrnInfoPtr pScrn);
extern Bool SMI712_OutputPreInit(ScrnInfoPtr pScrn);
void sm712_I2CInit(ScrnInfoPtr pScrn);
#endif /*  ----- #ifndef SMI_712_INC   -----*/
