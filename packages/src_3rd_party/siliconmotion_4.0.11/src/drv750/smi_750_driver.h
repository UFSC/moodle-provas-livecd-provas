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
#ifndef  SMI_750_INC
#define  SMI_750_INC

#include "smi_750_hw.h"
#include	"../ddk750/ddk750.h"
#include	"../ddk750/ddk750_chip.h"
#include	"../ddk750/ddk750_mode.h"

#if VALIDATION_CHIP==1
#else
#define VALIDATION_CHIP		1
#endif

#define  PLL_INDEX_MAX 	12
#define DEFAULT_INPUT_CLOCK  14318181


typedef struct {
	/* put common structure here */
	SMIHWRec base;
	/* put chip dependent stuffs here*/
	SMI750RegPtr pRegSave;
	/* vga stuffs */
	char * fonts;	
}SMI750HWRec,*SMI750HWPtr;

typedef struct {
	/* we best put base in header of structure*/
	SMIRec base;
	/* chip dependent stuffs of this pSmi*/
	int csc;/* use csc video or not */	
}SMI750Rec,*SMI750Ptr;

SMIPtr sm750_genSMI(pointer priv,int);
void sm750_vgaAndpciInit(ScrnInfoPtr pScrn,int entityIndex);
void sm750_entityInit(int,pointer);
void sm750_entityEnter(int,pointer);
void sm750_entityLeave(int,pointer);
void sm750_getMapResult(ScrnInfoPtr);
void sm750_handleOptions(ScrnInfoPtr);
ModeStatus sm750_validMode(ScrnInfoPtr,DisplayModePtr);
void sm750_loadPalette (ScrnInfoPtr , int , int *, LOCO * , VisualPtr );
void sm750_setMode(ScrnInfoPtr,DisplayModePtr);
void sm750_adjustFrame(ScrnInfoPtr,int);
void sm750_setDisplay(ScrnInfoPtr pScrn, DisplayModePtr mode);
void sm750_pcDeepmap(SMIHWPtr pHw);
void sm750_saveText(ScrnInfoPtr pScrn);
static void sm750_saveFonts(SMIHWPtr pHw);
static void sm750_restoreFonts(SMIHWPtr pHw);
void sm750_closeAllScreen(SMIHWPtr pHw);
int sm750_totalFB(SMIHWPtr pHw);
void sm750_VideoReset(ScrnInfoPtr pScrn);

extern unsigned int ddk750_getVMSize();
extern int ddk750_initHw(initchip_param_t * pInitParam);
extern void ddk750_setLogicalDispOut(disp_output_t output);
extern int ddk750_initDVIDisp();
extern int PEEK32(addr) ;
extern void POKE32(addr,data);
extern void ddk750_set_mmio(volatile unsigned char * addr,unsigned short devId,unsigned long revId);
extern int ddk750_setModeTiming(mode_parameter_t * parm,clock_type_t clock);
extern void ddk750_setDPMS(DPMS_t state);
extern Bool SMI750_CrtcPreInit(ScrnInfoPtr pScrn);
extern Bool SMI750_OutputPreInit(ScrnInfoPtr pScrn);
void sm750_I2CInit(ScrnInfoPtr pScrn);
void set_display_750(SMIHWPtr pHw,display_t display,int bpp,int pitch,int ulBaseAddress,int HDisplay,int HTotal);
#endif   /* ----- #ifndef SMI_750_INC  ----- */
