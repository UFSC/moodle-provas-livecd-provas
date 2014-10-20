/*
Copyright (C) 1994-1999 The XFree86 Project, Inc.  All Rights Reserved.
Copyright (C) 2000 Silicon Motion, Inc.  All Rights Reserved.
Copyright (C) 2008 Francisco Jerez. All Rights Reserved.
Copyright (c) 2012 by Silicon Motion, Inc. (SMI)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the names of The XFree86 Project and
Silicon Motion shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization from The XFree86 Project or Silicon Motion.
*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "smi_common.h"
#include "smi_driver.h"
#include "smi_crtc.h"
#include "smi_output.h"
#include "ddk502/ddk502_display.h"
#include "ddk502/ddk502_power.h"

xf86OutputStatus
SMI_OutputDetect_PNL_CRT(xf86OutputPtr output)
{
	ENTER();
    LEAVE(XF86OutputStatusConnected);   
}
Bool
SMI_OutputModeFixup(xf86OutputPtr output, DisplayModePtr mode, DisplayModePtr adjusted_mode)
{
    ENTER();

    /* Nothing */

    LEAVE(TRUE);
}
 int
SMI_OutputModeValid(xf86OutputPtr output, DisplayModePtr mode)
{
	ScrnInfoPtr pScrn = output->scrn;
	SMIPtr pSmi = SMIPTR(pScrn);	
    	SMIOutputPrivatePtr outputPriv;
	outputPriv = SMIOUTPUT(output);
	int xlcd,ylcd;

	ENTER();

	outputPriv->set_backlight(1);   

	LEAVE(MODE_OK);

	xlcd = pSmi->lcdWidth;
	ylcd = pSmi->lcdHeight;

	XMSG("mode:%dx%d@%d\n",mode->HDisplay,mode->VDisplay,mode->VRefresh);

    /* FIXME May also need to test for IS_MSOC(pSmi) here.
     * Only accept modes matching the panel size because the panel cannot
     * be centered neither shrinked/expanded due to hardware bugs.
     * Note that as int32_t as plane tr/br and plane window x/y are set to 0
     * and the mode height matches the panel height, it will work and
     * set the mode, but at offset 0, and properly program the crt.
     * But use panel dimensions so that "full screen" programs will do
     * their own centering. */
   // if (output->name && strcmp(output->name, "LVDS") == 0 &&
   //	(mode->HDisplay != pSmi->lcdWidth || mode->VDisplay != pSmi->lcdHeight))
  //	LEAVE(MODE_PANEL);

    /* The driver is actually programming modes, instead of loading registers
     * state from static tables. But still, only accept modes that should
     * be handled correctly by all hardwares. On the MSOC, currently, only
     * the crt can be programmed to different resolution modes.
     */
    //if (mode->HDisplay & 15)
	//LEAVE(MODE_BAD_WIDTH);

    if((mode->Clock < pSmi->clockRanges->minClock) ||
       (mode->Clock > pSmi->clockRanges->maxClock) ||
       ((mode->Flags & V_INTERLACE) && !pSmi->clockRanges->interlaceAllowed) ||
       ((mode->Flags & V_DBLSCAN) && (mode->VScan > 1) && !pSmi->clockRanges->doubleScanAllowed))
       {
		LEAVE(MODE_CLOCK_RANGE);
    	}    
	else if(xlcd > 0 && ylcd > 0 && 
			(mode->HDisplay > xlcd || mode->VDisplay > ylcd))
	{	
		LEAVE(MODE_CLOCK_RANGE);
	}

    LEAVE(MODE_OK);
}

Bool SMI_SetHead(ScrnInfoPtr pScrn,output_head_t head,panel_state_t state)
{
	/*
		Do minimal fundamental steps to reach the order!
		Possibaly affect other heads  !!
	*/
	ENTER();	
	
	DEBUG("%s head : %s\n",head_str[head],state == PANEL_ON?"on":"off");
	
	switch(head)
	{
		case HEAD_TFT0:
			ddk502_swPanelPowerSequence(state,4);
			break;
		case HEAD_DVI:
			if(state == PANEL_ON){
				ddk502_swPanelPowerSequence(PANEL_ON,4);
				setDAC(PANEL_ON);
			}else{
				setDAC(PANEL_OFF);
			}			
			break;
		case HEAD_VGA:
			if(state == PANEL_ON){
				setDAC(PANEL_OFF);
				ddk502_setDPMS(SMI_DPMS_ON);
			}else{
				ddk502_setDPMS(SMI_DPMS_OFF);
			}				
			break;
		default:
		case HEAD_TFT1:
			/*	nothing to turn on trun off for TFT2 head	*/
			break;		
	}
	LEAVE(TRUE);
}


void SMI_OutputCommit(xf86OutputPtr output)
{
    SMIOutputPrivatePtr outputPriv;
    outputPriv = SMIOUTPUT(output);
    ENTER();
    output->funcs->dpms(output,DPMSModeOn);
    outputPriv->set_backlight(1);   
    LEAVE();
}

static xf86OutputStatus
SMI_OutputDetect(xf86OutputPtr output)
{
    ENTER();

    LEAVE(XF86OutputStatusUnknown);
}
void SMI_OutputCreateResources(xf86OutputPtr output)
{
    ENTER();
    /* Nothing */	
    LEAVE();
}
xf86OutputStatus SMI_OutputDetect_lcd(xf86OutputPtr output)
{
    ENTER();

    LEAVE(XF86OutputStatusConnected);
}
void SMI_OutputPrepare(xf86OutputPtr output)
{
    ENTER();
    /* Nothing */
    LEAVE();
}
DisplayModePtr
SMI_OutputGetModes_native(xf86OutputPtr output)
{
    SMIPtr pSmi = SMIPTR(output->scrn);
    ENTER();
   
#ifdef HAVE_XMODES
//    XMSG("pSmi-> lcdWidth,lcdHeight = %d %d \n",pSmi->lcdWidth,pSmi->lcdHeight);
    LEAVE(xf86CVTMode(pSmi->lcdWidth, pSmi->lcdHeight, 60.0f, FALSE, FALSE));
/*reversed*/
#else
    LEAVE(NULL);
#endif
}

void
SMI_OutputDestroy(xf86OutputPtr output)
{
    SMIPtr pSmi = SMIPTR(output->scrn);
    ENTER();   
//	xfree((SMIOutputPrivatePtr)output->driver_private);    
    xfree(SMIOUTPUT(output));
    LEAVE();
}

Bool
SMI_OutputFuncsInit_base(xf86OutputFuncsPtr* outputFuncs)
{
	ENTER();
    *outputFuncs = xnfcalloc(sizeof(xf86OutputFuncsRec), 1);
	if(*outputFuncs == NULL)
		LEAVE(FALSE);

    (*outputFuncs)->create_resources = SMI_OutputCreateResources;
    (*outputFuncs)->mode_fixup = SMI_OutputModeFixup;
    (*outputFuncs)->prepare = SMI_OutputPrepare;
    (*outputFuncs)->commit = SMI_OutputCommit;
    (*outputFuncs)->detect = SMI_OutputDetect;
    (*outputFuncs)->destroy = SMI_OutputDestroy;
	
	LEAVE(TRUE);
}
