/*
Copyright (C) 1994-1999 The XFree86 Project, Inc.  All Rights Reserved.
Copyright (C) 2000 Silicon Motion, Inc.  All Rights Reserved.
Copyright (C) 2008 Mandriva Linux.  All Rights Reserved.
Copyright (C) 2008 Francisco Jerez.  All Rights Reserved. 
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
#include "../smi_common.h"
#include "../smi_driver.h"
#include "../smi_dbg.h"
#include "../smi_crtc.h"
#include "../smi_output.h"
#include "ddk502/ddk502_regdc.h"
#include "ddk502/ddk502_display.h"
#include "ddk502/ddk502_help.h"
#include "smi_502_hw.h"

#define EDID_DEVICE_I2C_ADDRESS_502		0xa0
#define EDID_TOTAL_RETRY_COUNTER_502	4
#define TOTAL_EDID_REGISTERS_502		128
#define SM502_DEFAULT_I2C_SCL			46
#define SM502_DEFAULT_I2C_SDA			47

void SMI502_OutputDPMS_PNL(xf86OutputPtr output, int dpms);
void SMI502_OutputDPMS_CRT(xf86OutputPtr output,int dpms);


#if SMI_RANDR

char* sm502_outputName[]={"PNL","CRT"}; 


static void (*pfn_sm502outputDPMS[])(xf86OutputPtr,int)=
{SMI502_OutputDPMS_PNL,SMI502_OutputDPMS_CRT};


void
SMI502_OutputDPMS_PNL(xf86OutputPtr output, int dpms)
{
    ScrnInfoPtr	pScrn = output->scrn;
	int idx;	
    ENTER();

    switch (dpms) 
	{
    case DPMSModeOn:		
		idx = 0;
		ddk502_swPanelPowerSequence(PANEL_ON,4);
		break;
    case DPMSModeStandby:		
		idx = 1;
		break;
    case DPMSModeSuspend:
		idx = 2;
		break;
    case DPMSModeOff:
		idx = 3;		
		ddk502_swPanelPowerSequence(PANEL_OFF,4);	
		break;
    }
	
    DEBUG("Set PNL DPMS ==> %s \n",dpms_str[idx]);
    LEAVE();
}
//extern void ddk502_setDPMS(DPMS_t state);
void SMI502_OutputDPMS_CRT(xf86OutputPtr output,int dpms)
{
    ScrnInfoPtr	pScrn = output->scrn;
    int idx;	
    ENTER();
/*
    switch (dpms) 
	{
    case DPMSModeOn:		
		idx = 0;
		ddk502_setDPMS(SMI_DPMS_ON);
		break;
    case DPMSModeStandby:
		idx = 1;
		ddk502_setDPMS(SMI_DPMS_STANDBY);
		break;
    case DPMSModeSuspend:
		idx = 2;
		ddk502_setDPMS(SMI_DPMS_SUSPEND);
		break;
    case DPMSModeOff:
		idx = 3;
		ddk502_setDPMS(SMI_DPMS_OFF);		
		break;
    }
  */  
	DEBUG("Set CRT DPMS ==> %s \n",dpms_str[idx]);
	LEAVE();	
}


/*   return 0 if the mode can be found in DDK  */
unsigned char  SM502_edidGetVersion(
    unsigned char *pEDIDBuffer,
    unsigned char *pRevision)
{
	unsigned char version;
    
    	if (pEDIDBuffer != (unsigned char *)0)
    	{
        /* Check the header */
        	if ((pEDIDBuffer[0] == 0x00) && (pEDIDBuffer[1] == 0xFF) && (pEDIDBuffer[2] == 0xFF) &&
            	(pEDIDBuffer[3] == 0xFF) && (pEDIDBuffer[4] == 0xFF) && (pEDIDBuffer[5] == 0xFF) &&
            	(pEDIDBuffer[6] == 0xFF) && (pEDIDBuffer[7] == 0x00))
        	{
            /* 
             * EDID Structure Version 1.
             */
        
            /* Read the version field from the buffer. It should be 1 */
            	version  = pEDIDBuffer[18];
        
            	if (version == 1)
            	{
                /* Copy the revision first */
                	if (pRevision != (unsigned char *)0)
                    		*pRevision = pEDIDBuffer[19];
          			return version;
            	}
        }
        else
        {
            /* 
             * EDID Structure Version 2 
             */
             
            /* Read the version and revision field from the buffer. */
            	version = pEDIDBuffer[0];
        
            	if ((version >> 4) == 2)
            	{
                /* Copy the revision */
                	if (pRevision != (unsigned char *)0)
                    		*pRevision = version & 0x0F;
                		 return (version >> 4);
            	}
        }
    }   
    return 0;    
}

extern unsigned char ddk502_swI2CInit(unsigned char,unsigned char);
extern unsigned char ddk502_swI2CReadReg(unsigned char,unsigned char);
int32_t SMI502_edidReadMonitorEx(
    display_t displayPath, 
    unsigned char *pEDIDBuffer,
    uint32_t bufferSize,
    unsigned char edidExtNo,
    unsigned char sclGpio,
    unsigned char sdaGpio)
{
    unsigned char value, retry, edidVersion, edidRevision;
    unsigned char edidBuffer[256];
    uint32_t offset;
    
    /* Initialize the i2c bus */
    ddk502_swI2CInit(sclGpio, sdaGpio);
    for (retry = 0; retry < EDID_TOTAL_RETRY_COUNTER_502; retry++)
    {
            
        /* Read the EDID from the monitor. */
        for (offset = 0; offset < TOTAL_EDID_REGISTERS_502; offset++)
	 		edidBuffer[offset] = ddk502_swI2CReadReg(EDID_DEVICE_I2C_ADDRESS_502, (unsigned char)offset);
            
        /* Check if the EDID is valid. */
        edidVersion = SM502_edidGetVersion((unsigned char *)&edidBuffer, (unsigned char *)&edidRevision);
        if (edidVersion != 0)
        	break;
    }
    
    /* 
     *  The monitor might not be DDC2B compliance. Therefore, need to use DDC1 protocol,
     *  which uses the Vertical Sync to clock in the EDID data.
     *  Currently this function return error. DDC1 protocol can be added later.  
     */
    if (retry == EDID_TOTAL_RETRY_COUNTER_502)
    {
        /* DDC1 uses the SDA line to transmit 9 bit data per byte. The last bit is
         * only an acknowledge flag, which could be high or low. However, SCL line
         * is not used. Instead the data is clock-in using vertical sync.
         */
        return (-1);
    }
    
    /* Copy the data to the given buffer */
    if (pEDIDBuffer != (unsigned char *)0)
    {
        for (offset = 0; offset < bufferSize; offset++)
            pEDIDBuffer[offset] = edidBuffer[offset];
    }
    return 0;
}
DisplayModePtr SMI502_OutputGetModes(xf86OutputPtr output)
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

	bus = (priv->path == PANEL ? pSmi->I2C_primary : pSmi->I2C_secondary);

	/* 	some thing weird: some times Xserver method to access EDID is okay while some times not
		but use hardware i2c to access DVI EDID should be always okay
	*/
	
    if(xf86LoaderCheckSymbol("xf86PrintEDID"))
	{		
		if(bus)
		{
			pMon = xf86OutputGetEDID(output,bus);
			if(pMon)
			{
				xf86PrintEDID(pMon);
				xf86OutputSetEDID(output,pMon);
				xfree(EdidBuffer);
				LEAVE(xf86OutputGetEDIDModes(output));
			}
			else{
				XERR("Seems monitor not found\n");	
			}
		}
		else{
			XERR("Seems bus not found\n");
		}
	}
	else{
		XERR("should me be seen ?\n");
	}

	XINF("try DDK method\n");
	long returnValue;
	if(priv->path != PANEL)
	{	
			
		returnValue = SMI502_edidReadMonitorEx(PANEL,EdidBuffer, 128, 
										0, SM502_DEFAULT_I2C_SCL, SM502_DEFAULT_I2C_SDA);
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
		returnValue = -1 ;
//		SMI502_edidReadMonitorEx(CRT,EdidBuffer,128,0, SM502_DEFAULT_I2C_SCL, SM502_DEFAULT_I2C_SDA); //502 do not support crt path edid
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
Bool SMI502_LinkPathCrtcc(ScrnInfoPtr pScrn,display_t path,channel_t control)
{
	/*	just select the contoller for the path and do nothing else	*/
	uint32_t ulreg;
	ENTER();

	DEBUG("%s path <== %s controller \n",
				path==PANEL?"Panel":"Crt",
				control==PRIMARY_CTRL?"Primary":"Secondary");

	
	if(path == PANEL)
	{		
	}
	else
	{
		ulreg = ddk502_PEEK32(CRT_DISPLAY_CTRL);
		ulreg = FIELD_VALUE(ulreg,CRT_DISPLAY_CTRL,SELECT,control == PRIMARY_CTRL?0:1);	
		ddk502_POKE32(CRT_DISPLAY_CTRL,ulreg);
	}
	LEAVE(TRUE);
}
static void SMI502_OutputModeSet(xf86OutputPtr output, DisplayModePtr mode, DisplayModePtr adjusted_mode)
{
	ScrnInfoPtr pScrn;
	xf86CrtcPtr crtc;	
	SMIPtr pSmi;
	SMIHWPtr pHw;
	channel_t channel;
	display_t path;
	output_head_t head;
	int okay;
	
    ENTER();
	pScrn = output->scrn;
	pSmi = SMIPTR(pScrn);
	pHw = pSmi->pHardware;
	SMIOutputPrivatePtr priv = (SMIOutputPrivatePtr)output->driver_private;
	
	RROutputPtr rrop = output->randr_output;

	if((crtc = output->crtc) != NULL)
	{	
		channel = ((SMICrtcPrivatePtr)crtc->driver_private)->controller;
		path = priv->path;
		head = priv->head;
#if 0	
		if(pSmi->DualView == FALSE){
        	SMI502_LinkPathCrtcc(pScrn,PANEL ,channel);	
		    SMI502_LinkPathCrtcc(pScrn,CRT ,channel);
		    SMI_SetHead(pScrn,HEAD_DVI,PANEL_ON);
		    SMI_SetHead(pScrn,HEAD_VGA,PANEL_ON);			
		}else{			
			SMI502_LinkPathCrtcc(pScrn,path ,channel);	
			SMI_SetHead(pScrn,head,DISP_ON);
			if(channel != PRIMARY_CTRL)
			setLogicalDispOutput(PANEL_CRT_DUAL);
        }
#endif
	}
    else
	{	
		XERR("Output->crtct == NULL !!\n");
	}	
    LEAVE();
}

Bool
SMI502_OutputPreInit(ScrnInfoPtr pScrn)
{
	SMIPtr pSmi;
	xf86OutputPtr		output;
	SMIOutputPrivatePtr outputPriv;
	xf86OutputFuncsPtr	outputFuncs;
	SMIHWPtr pHw;
	ENTER();
	
    pSmi = SMIPTR(pScrn);
	pHw = pSmi->pHardware;
	int c = (pSmi->DualView?2:1);
	int idx = 0;
	Bool expansion = (pSmi->lcdWidth>0 && pSmi->lcdHeight>0);
	
	while(idx < c)
	{	
		if(!SMI_OutputFuncsInit_base(&outputFuncs))
			LEAVE(FALSE);	
				//		outputFuncs->mode_valid = SMI_OutputModeValid;
			outputFuncs->mode_set = SMI502_OutputModeSet;
			outputFuncs->dpms		= pfn_sm502outputDPMS[idx];
			outputFuncs->get_modes = SMI502_OutputGetModes;
			outputFuncs->detect		= SMI_OutputDetect_PNL_CRT;
			outputFuncs->mode_valid = SMI_OutputModeValid;
			outputPriv = xnfcalloc(sizeof(SMIOutputPrivateRec),1);	
			outputPriv->index = idx;
			outputPriv->path = idx;
			outputPriv->set_backlight	= set_backlight_502;
		
		if(idx == 0)
		{   /* for output 0: it's always be Panel path */
			if(pSmi->TFTColor!= -1)
				outputPriv->head = HEAD_TFT0;
			else
				outputPriv->head = HEAD_DVI;
		}
		else
		{   /* for output 1: it's always be Crt path */
			if(pSmi->TFTColor == SMI_TFT_18BIT)
				outputPriv->head = HEAD_TFT1;
			else
				outputPriv->head = HEAD_VGA;
		}
	    	if (! (output = xf86OutputCreate(pScrn, outputFuncs, sm502_outputName[idx])))
			LEAVE(FALSE);

		output->driver_private = outputPriv;	
		if(idx == 0)
		{
			output->possible_crtcs = 1;
			output->possible_clones = 1;
				 
		}
		else
		{
        	output->possible_crtcs = pSmi->DualView?3:1;
			output->possible_clones = 3;

		}
		
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

