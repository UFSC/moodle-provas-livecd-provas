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
#include "smi_750_hw.h"
void SMI750_OutputDPMS_PNL(xf86OutputPtr output, int dpms);

void SMI750_OutputDPMS_CRT(xf86OutputPtr output, int dpms);

#if SMI_RANDR
void (*pfn_outputDPMS[])(xf86OutputPtr,int)=
{SMI750_OutputDPMS_PNL,SMI750_OutputDPMS_CRT};
char* outputName[]={"PNL","CRT"}; 


void
SMI750_OutputDPMS_PNL(xf86OutputPtr output, int dpms)
{
    ScrnInfoPtr	pScrn = output->scrn;
	int idx;	
    ENTER();

    switch (dpms) 
	{
    case DPMSModeOn:		
		idx = 0;
		ddk750_swPanelPowerSequence(PANEL_ON,4);
		break;
    case DPMSModeStandby:		
		idx=1;
		break;
    case DPMSModeSuspend:
		idx=2;
		break;
    case DPMSModeOff:
		idx=3;		
		ddk750_swPanelPowerSequence(PANEL_OFF,4);	
		break;
    }
	
    DEBUG("Set PNL DPMS ==> %s \n",dpms_str[idx]);
    LEAVE();
}
void
SMI750_OutputDPMS_CRT(xf86OutputPtr output,int dpms)
{
    ScrnInfoPtr	pScrn = output->scrn;
    int idx;	
    ENTER();

    switch (dpms) 
	{
    case DPMSModeOn:		
		idx = 0;
		ddk750_setDPMS(SMI_DPMS_ON);
		break;
    case DPMSModeStandby:
		idx = 1;
		ddk750_setDPMS(SMI_DPMS_STANDBY);
		break;
    case DPMSModeSuspend:
		idx = 2;
		ddk750_setDPMS(SMI_DPMS_SUSPEND);
		break;
    case DPMSModeOff:
		idx = 3;
		ddk750_setDPMS(SMI_DPMS_OFF);		
		break;
    }
    
	DEBUG("Set CRT DPMS ==> %s \n",dpms_str[idx]);
	LEAVE();	
}


Bool SMI750_LinkPathCrtcc(ScrnInfoPtr pScrn,display_t path,channel_t control)
{
	/*	just select the contoller for the path and do nothing else	*/
	uint32_t ulreg;
	ENTER();

	DEBUG("%s path <== %s controller \n",
				path == PANEL?"Panel":"Crt",
				control == PRIMARY_CTRL?"Primary":"Secondary");

	
	if(path == PANEL)
	{
		int times = 30;
		ulreg = PEEK32(PANEL_DISPLAY_CTRL);
		/* monk: a bug for 718,bit 29:28 can be se correctly by only one time 
				need a while loop to handle this hardware limitation
				seems for 718,only the vertical timing finish sync 
				is a right period for writing bit 29:28
		*/
		while(times --)
		{
			ulreg = FIELD_VALUE(ulreg,PANEL_DISPLAY_CTRL,SELECT,control == PRIMARY_CTRL?0:2);
			POKE32(PANEL_DISPLAY_CTRL,ulreg);
			if(PEEK32(PANEL_DISPLAY_CTRL) == ulreg)
				break;
		}
		
		if(times < 0){
			XERR("Time out when setting select of RPIMARY_DISPLAY_CTRL\n");
			LEAVE(FALSE);
		}
		
	}
	else
	{
		ulreg = PEEK32(CRT_DISPLAY_CTRL);
		ulreg=FIELD_VALUE(ulreg,CRT_DISPLAY_CTRL,SELECT,control == PRIMARY_CTRL?0:2);
		POKE32(CRT_DISPLAY_CTRL,ulreg);
	}
	LEAVE(TRUE);
}

static void 
SMI750_OutputModeSet(xf86OutputPtr output, DisplayModePtr mode, DisplayModePtr adjusted_mode)
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
	pSmi = SMIPTR(pScrn);
	SMIOutputPrivatePtr priv = (SMIOutputPrivatePtr)output->driver_private;
	
	RROutputPtr rrop = output->randr_output;

	if((crtc = output->crtc) != NULL) {	
		channel = ((SMICrtcPrivatePtr)crtc->driver_private)->controller;
		path = priv->path;
		head = priv->head;	
		if(pSmi->DualView == FALSE){
        	SMI750_LinkPathCrtcc(pScrn,PANEL_PATH ,channel);	
		    SMI750_LinkPathCrtcc(pScrn,CRT_PATH ,channel);			
		}else{			
    		SMI750_LinkPathCrtcc(pScrn,path,channel);
        }	
	}
    else
	{	
		XERR("Output->crtct == NULL !!\n");
	}	
    LEAVE();
}
DisplayModePtr SMI750_OutputGetModes(xf86OutputPtr output)
{
    ScrnInfoPtr pScrn = output->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    xf86MonPtr pMon = NULL;
    SMIOutputPrivatePtr priv = (SMIOutputPrivatePtr)output->driver_private;
    I2CBusPtr bus;
    char * EdidBuffer;

    ENTER();
    EdidBuffer = xalloc(128);
    if(!EdidBuffer){
        XERR("not enough memory for allocating EDID buffer\n");
        LEAVE(NULL);
    }
    XINF("i2c bus index == %d\n",priv->path);
#if 1/*bus is null in the current randr driver, so canel the following*/
   bus = (priv->path == PANEL_PATH ? pSmi->I2C_primary:pSmi->I2C_secondary);
    /* 	some thing weird: some times Xserver method to access EDID is okay while some times not
        but use hardware i2c to access DVI EDID should be always okay
     */
    if(xf86LoaderCheckSymbol("xf86PrintEDID"))
    {		
        if(bus)
        {
			if (priv->path == PANEL_PATH)
				swI2CInit(DEFAULT_I2C_SCL, DEFAULT_I2C_SDA);
			else
				swI2CInit(17, 18);
			
            pMon = xf86OutputGetEDID(output,bus);
            if(pMon)
            {
                xf86PrintEDID(pMon);
                xf86OutputSetEDID(output,pMon);
                xfree(EdidBuffer);
                LEAVE(xf86OutputGetEDIDModes(output));
            }
#ifdef SMI_DBG		
            else{
                XERR("Seems monitor not found\n");	
            }
#endif			
        }
#ifdef SMI_DBG
        else{
            XERR("Seems bus not found\n");
        }
#endif		
    }
#ifdef SMI_DBG
    else{
        XERR("should me be seen ?\n");
    }
#endif	
#endif
    XINF("try DDK method\n");
    int32_t returnValue;
    /*
       because 750ddk.so is loaded by siliconmotion_drv.so
       so if one function of 750ddk.so is referenced as function pointer
       it will be failed .
       but calling the function is okay.
       I suggest combine 750ddk.so into siliconmotion_drv.so 
       which is a must for function pointer passing method

       below code pass xorg_I2CUDelay address into edidRead
       but X will exit with error when it be executed
       while just calling xorg_I2CUDelay is okay
       monk  @  10/20/2010
     */
	if(priv->path == PANEL_PATH)
	{	
			
		returnValue = ddk750_edidReadMonitorEx(PANEL_PATH,EdidBuffer, 128, 
										0, DEFAULT_I2C_SCL, DEFAULT_I2C_SDA);
		if (returnValue == 0)
		{
			pMon = xf86InterpretEDID(pScrn->scrnIndex,EdidBuffer);
			if(pMon){
				xf86OutputSetEDID(output,pMon);
				XINF("Found monitor by DDK\n");
				xfree(EdidBuffer);
				LEAVE(xf86OutputGetEDIDModes(output));
			}else{
				XERR("DDK cannot get monitor\n");
			}
		}
		else
		{
			XERR("DDK cannot detect EDID Version\n");
		}
	}
	else
	{	
		/*
			because 750ddk.so is loaded by siliconmotion_drv.so
			so if one function of 750ddk.so is referenced as function pointer
			it will be failed .
			but calling the function is okay.
			I suggest combine 750ddk.so into siliconmotion_drv.so 
			which is a must for function pointer passing method

			below code pass xorg_I2CUDelay address into edidRead
			but X will exit with error when it be executed
			while just calling xorg_I2CUDelay is okay
				monk  @  10/20/2010
		*/
		returnValue = edidReadMonitorEx_software(CRT_PATH,EdidBuffer,128, 
										0, 17, 18
										);
		
		
		if (returnValue == 0)
		{
			pMon = xf86InterpretEDID(pScrn->scrnIndex, EdidBuffer);
			if(pMon){
				xf86OutputSetEDID(output,pMon);
				XINF("Found monitor by DDK\n");
				xfree(EdidBuffer);
				LEAVE(xf86OutputGetEDIDModes(output));
			}else{
				XERR("DDK cannot get monitor\n");
			}
		}
	}
    XMSG("DDK seems no mode found\n");
    xfree(EdidBuffer);
    LEAVE(NULL);	
}
Bool SMI750_OutputPreInit(ScrnInfoPtr pScrn)
{
	SMIPtr pSmi= SMIPTR(pScrn);
	xf86OutputPtr		output;
	SMIOutputPrivatePtr outputPriv;
	xf86OutputFuncsPtr	outputFuncs;
    ENTER();
	
	pSmi = SMIPTR(pScrn);
	int c = pSmi->DualView?2:1;
	int idx = 0;
	Bool expansion = (pSmi->lcdWidth>0 && pSmi->lcdHeight>0);
	
	while(idx < c)
	{	
		if(!SMI_OutputFuncsInit_base(&outputFuncs))
			LEAVE(FALSE);	

		outputFuncs->mode_set 	= SMI750_OutputModeSet;
		outputFuncs->dpms		= pfn_outputDPMS[idx];
		outputFuncs->get_modes 	= SMI750_OutputGetModes;
		outputFuncs->detect		= SMI_OutputDetect_PNL_CRT;
		outputFuncs->mode_valid = SMI_OutputModeValid;
		outputPriv = xnfcalloc(sizeof(SMIOutputPrivateRec),1);	
		outputPriv->set_backlight = set_backlight_750;
		outputPriv->index = idx;
		outputPriv->path = idx;
		
		if(idx == 0)
		{   /* for output 0: it's always be Panel path */
			if(pSmi->TFTColor != -1)
				outputPriv->head = HEAD_TFT0;
			else
				outputPriv->head = HEAD_DVI;
		}
		else
		{   /* for output 1: it's always be Crt path */
			if(pSmi->TFTColor== TFT_18BIT)
				outputPriv->head = HEAD_TFT1;
			else
				outputPriv->head = HEAD_VGA;
		}

	    if (! (output = xf86OutputCreate(pScrn, outputFuncs, outputName[idx])))
			LEAVE(FALSE);

		output->driver_private = outputPriv;		
       	output->possible_crtcs = pSmi->DualView?3:1;        
        output->possible_clones = 3;
		
    	if(outputPriv->path == 0 && expansion)
    	{
       		 /*  only link to crtc_1 [secondary channel]
                 but remember that if no expansion ,crtc_0 will be by default Primary channel
             */
    		output->possible_crtcs = 1;//note: Secondary controller will be ctrc 0 if expansion enabled  
    		output->possible_clones = 0;
    	}

		if(outputPriv->path == 1 && expansion)
			output->possible_clones = 1;      

		output->interlaceAllowed = FALSE;
		output->doubleScanAllowed = FALSE;
		XINF("output[%d]->possible_crtcs = %d \n",idx,output->possible_crtcs);

		idx++;
	}
	LEAVE(TRUE);
}

#endif /*#if SMI_RANDR*/
