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

#ifndef _SMI_OUTPUT_H
#define _SMI_OUTPUT_H
#include "smi_common.h"
#include "smi_dbg.h"
#ifdef SMI_DBG
static char * dpms_str[]={"DPMSModeOn","DPMSModeStandby","DPMSModeSuspend","DPMSModeOff"};
static char * head_str[]={"TFT1","DVI","VGA","TFT2"};
static char * tft_str[] ={"18bit","24bit","36bit"};
#endif

typedef enum
{
	HEAD_TFT0,	
	HEAD_DVI,	
	HEAD_VGA,
	HEAD_TFT1,
}output_head_t;

extern void ddk750_swPanelPowerSequence(int disp,int delay);
int SMI_OutputModeValid(xf86OutputPtr output, DisplayModePtr mode);
xf86OutputStatus SMI_OutputDetect_PNL_CRT(xf86OutputPtr);
Bool SMI_SetHead(ScrnInfoPtr pScrn,output_head_t head,panel_state_t state);
#endif
