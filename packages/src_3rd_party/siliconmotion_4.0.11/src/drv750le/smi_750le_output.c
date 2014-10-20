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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "../smi_common.h"
#include "../smi_driver.h"
#include "../smi_dbg.h"
#include "../smi_crtc.h"
#include "../smi_output.h"
#include "../ddk750/ddk750_display.h"
#include "../ddk750/ddk750.h"
#include "../ddk750/ddk750_edid.h"
#include "../ddk750/ddk750_hwi2c.h"
#include "../ddk750/ddk750_swi2c.h"

void SMI750_OutputDPMS_PNL(xf86OutputPtr output, int dpms);
void SMI750_OutputDPMS_CRT(xf86OutputPtr output,int dpms);


#if SMI_RANDR

void SMI_OutputCommit(xf86OutputPtr);
void SMI_OutputDestroy(xf86OutputPtr);
void SMI_OutputCreateResources(xf86OutputPtr);
Bool SMI_OutputModeFixup(xf86OutputPtr, DisplayModePtr, DisplayModePtr);
void SMI_OutputPrepare(xf86OutputPtr);
mode_parameter_t *findVesaModeParam(unsigned long width,unsigned long height,unsigned long refresh_rate);
#if 0//SMI_DEBUG
static char * dpms_str[]={"DPMSModeOn","DPMSModeStandby","DPMSModeSuspend","DPMSModeOff"};
static char * head_str[]={"TFT1","DVI","VGA","TFT2"};
static char * tft_str[] ={"18bit","24bit","36bit"};
#endif


static int ModeFound(xf86OutputPtr,DisplayModePtr);
static Bool SMI750LE_LinkPathCrtc(ScrnInfoPtr, display_t, channel_t);
static void SMI750LE_OutputModeSet(xf86OutputPtr,DisplayModePtr,DisplayModePtr);

extern char *outputName[];


/*   return 0 if the mode can be found in DDK  */
static int ModeFound(xf86OutputPtr output,DisplayModePtr mode)
{
    uint hz;
    ScrnInfoPtr	pScrn = output->scrn;
    ENTER();
    if(mode->HSync > 71.0f && mode->HSync < 77.1f)
        hz = 75;
    else
        hz = 60;        

    XMSG("mode->X,Y,H = %d,%d,%d\n",mode->HDisplay,mode->VDisplay,hz);
    if((mode_parameter_t *)findVesaModeParam(mode->HDisplay, mode->VDisplay, hz)!=NULL)
        LEAVE(0);
    XMSG("mode not supported by DDK\n");        
    LEAVE(-1);
}

xf86OutputStatus SMI750LE_OutputDetect(xf86OutputPtr output)
{
    ENTER();
    LEAVE(XF86OutputStatusConnected);
}

static int SMI750LE_OutputModeValid(xf86OutputPtr output, DisplayModePtr mode)
{
	ScrnInfoPtr pScrn = output->scrn;

	ENTER();
	// XMSG("mode:%dx%d@%d\n",mode->HDisplay,mode->VDisplay,mode->VRefresh);
	//XMSG("mode->Clock = %d,pSmi->clockRange.minClock = %d,pSmi->clockRange.maxClock = %d\n",
	//mode->Clock,pSmi->clockRange.minClock,pSmi->clockRange.maxClock);
	// if (ModeFound(output, mode)) {
	//     LEAVE(MODE_BAD);
	//  }
	LEAVE(MODE_OK);
}
DisplayModePtr SMI750LE_OutputGetModes(xf86OutputPtr output)
{
	ScrnInfoPtr pScrn = output->scrn;
	SMIPtr pSmi = SMIPTR(pScrn);
	xf86MonPtr pMon = NULL;
	char * EdidBuffer;

	ENTER();

	EdidBuffer = xalloc(128);
	if(!EdidBuffer){
		XERR("not enough memory for allocating EDID buffer\n");
		LEAVE(NULL);
	}
	/*XF86 method works*/
	if(xf86LoaderCheckSymbol("xf86PrintEDID")) {		
		if(pSmi->I2C_primary) {
			swI2CInit(0, 1);
			pMon = xf86OutputGetEDID(output,pSmi->I2C_primary);
			if(pMon) {
				xf86PrintEDID(pMon);
				xf86OutputSetEDID(output,pMon);
				xfree(EdidBuffer);
				LEAVE(xf86OutputGetEDIDModes(output));
			}
		}

		XINF("try DDK method\n");
		int32_t returnValue;
		/*DDK method works,too*/		
		returnValue = edidReadMonitorEx_software(PANEL_PATH,EdidBuffer, 128, 
				0, 0, 1);
		if (returnValue == 0) {
			pMon = xf86InterpretEDID(pScrn->scrnIndex,EdidBuffer);
			if(pMon){
				xf86OutputSetEDID(output,pMon);
				XINF("Found monitor by DDK\n");
				xfree(EdidBuffer);
				LEAVE(xf86OutputGetEDIDModes(output));
			} else{
				XERR("DDK cannot get monitor\n");
			}
		}
		xfree(EdidBuffer);
		LEAVE(NULL);
	}
	LEAVE(NULL);
}
Bool SMI750le_OutputPreInit(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi;
    xf86OutputPtr output;
    SMIOutputPrivatePtr outputPriv;
    xf86OutputFuncsPtr outputFuncs;

    ENTER();

    pSmi =SMIPTR(pScrn);
    outputFuncs = xnfcalloc(sizeof(xf86OutputFuncsRec), 1);
    if(outputFuncs == NULL)
        LEAVE(FALSE);
    outputFuncs->create_resources = SMI_OutputCreateResources;
    outputFuncs->mode_fixup = SMI_OutputModeFixup;
    outputFuncs->prepare = SMI_OutputPrepare;
    outputFuncs->commit = SMI_OutputCommit;
    outputFuncs->mode_set = SMI750LE_OutputModeSet;
    outputFuncs->destroy = SMI_OutputDestroy;
    outputFuncs->dpms = SMI750_OutputDPMS_CRT;
    outputFuncs->get_modes = SMI750LE_OutputGetModes;
    outputFuncs->detect = SMI_OutputDetect_PNL_CRT;
    outputFuncs->mode_valid = SMI_OutputModeValid;
    outputPriv = xnfcalloc(sizeof(SMIOutputPrivateRec),1);	
    outputPriv->index = 0;
    outputPriv->path = 0; //Jason
    outputPriv->head = HEAD_VGA;

    if (! (output = xf86OutputCreate(pScrn, outputFuncs, outputName[1])))
        LEAVE(FALSE);
    output->driver_private = outputPriv;		
    output->possible_crtcs = 1;
    output->possible_clones = 1;
    output->interlaceAllowed = FALSE;
    output->doubleScanAllowed = FALSE;

    LEAVE(TRUE);
}

static Bool SMI750LE_LinkPathCrtc(ScrnInfoPtr pScrn,display_t path,
        channel_t control)
{
    /*	just select the contoller for the path and do nothing else	*/
    uint32_t ulreg;
    ENTER();

    DEBUG("%s path <== %s controller \n",
            path==PANEL?"Panel":"Crt",
            control==PRIMARY_CTRL?"Primary":"Secondary");
    ulreg = PEEK32(CRT_DISPLAY_CTRL);
    ulreg = FIELD_VALUE(ulreg,CRT_DISPLAY_CTRL,SELECT, 2);
    POKE32(CRT_DISPLAY_CTRL,ulreg);
    LEAVE(TRUE);
}

static void SMI750LE_OutputModeSet(xf86OutputPtr output, 
        DisplayModePtr mode, DisplayModePtr adjusted_mode)
{
    ScrnInfoPtr pScrn;
    xf86CrtcPtr crtc;	
    SMIPtr pSmi;
    channel_t channel;
    display_t path;
    output_head_t head;
    int okay;

    ENTER();
    pScrn = output->scrn;
    pSmi =SMIPTR(pScrn);
    SMIOutputPrivatePtr priv = (SMIOutputPrivatePtr)output->driver_private;
    RROutputPtr rrop = output->randr_output;
    if((crtc = output->crtc) != NULL)
    {	
        channel = ((SMICrtcPrivatePtr)crtc->driver_private)->controller;
        path = priv->path;
        head = priv->head;
//        SMI750LE_LinkPathCrtc(pScrn,CRT,channel);
  //      SMI_SetHead(pScrn,HEAD_VGA,PANEL_ON);			
    }
    else
    {	
        XERR("Output->crtct == NULL !!\n");
    }	
    LEAVE();
}

#endif /*#if SMI_RANDR*/
