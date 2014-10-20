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


#include	"../smi_common.h"
#include 	"../smi_driver.h"
#include	"smi_502_driver.h"
#include 	"smi_502_hw.h"
#include	"../smi_dbg.h"
#include	"ddk502/ddk502_regsc.h"
#include	"ddk502/ddk502_chip.h"

/* static functions */
static Bool smi_setdepbpp_502(ScrnInfoPtr pScrn);
static Bool smi_set_dualhead_502(ScrnInfoPtr pScrn, SMI502_Ptr pSmi);
static Bool smi_setvideomem_502(int config, ScrnInfoPtr pScrn, SMI502_Ptr pSmi);
static mode_table_t * findMode_502 (mode_table_t * mode_table, INT width, 
        INT height, INT refresh_rate);

static void SMI502_EnableVideo (ScrnInfoPtr pScrn);
static int SMI502_InternalScreenInit (int scrnIndex, ScreenPtr pScreen);
static void SMI502_EntityInit(int entityIndex,pointer private);

void (*pfn_I2CPutBits_502[])(I2CBusPtr,int,int)=
{NULL,NULL};

void (*pfn_I2CGetBits_502[])(I2CBusPtr,int*,int*)=
{NULL,NULL};



/* Global Vars */

static const SMI502HWRec sm502_hwrec = {
	.base = {
		.pcDeepMap = sm502_pcDeepmap,/* actually,NULL value can be just ignored,gcc will know it*/
		.pcEntityInit = sm502_entityInit,
		.pcEntityEnter = sm502_entityEnter,
		.pcEntityLeave = sm502_entityLeave,	
		.pcCloseAllScreen = sm502_closeAllScreen,
		.pcFBSize = sm502_totalFB,
		.pcInitHardware = NULL,
	},

};


static const SMI502_Rec sm502_rec = {
	.base = {
		.minHeight = 128,
		.psGetMapResult = sm502_getMapResult,
		.psVgaPciInit = sm502_vgaAndpciInit,
		.psHandleOptions = sm502_handleOptions,
		.psValidMode = sm502_validMode,
		.psSetMode = sm502_setMode,
		.psAdjustFrame = sm502_adjustFrame,	
		.psLoadPalette = sm502_LoadPalette,
		.psSetDisplay = sm502_setDisplay,
		.psSaveText = sm502_saveText,
	#if SMI_RANDR
		.psCrtcPreInit = SMI502_CrtcPreInit,
		.psOutputPreInit = SMI502_OutputPreInit,
		.psI2CInit = sm502_I2CInit,
	#endif	
	},
};

SMIPtr sm502_genSMI(pointer priv,int entityIndex)
{
    SMIPtr pSmi; 
#ifdef XSERVER_LIBPCIACCESS
	struct pci_device * pPci;
#else
	pciVideoPtr pPci;
#endif

	ENTER();
#ifdef XSERVER_LIBPCIACCESS	
	pPci = xf86GetPciInfoForEntity(entityIndex);
#endif

    pSmi = (SMIPtr)XNFcalloc(sizeof(SMI502_Rec));
    memset(pSmi,0, sizeof(SMI502_Rec));
	if(pSmi == NULL)
		return pSmi;

	memcpy(pSmi,&sm502_rec,sizeof(SMI502_Rec));
	/*This function is associated with screen
	, but maybe two screen belongs to one entity (dualview)*/
	pSmi->pHardware = priv;

	/* if priv is NULL,means we need create a pHw
	 * below code can handle 2 screen per entity case
	 * but not work for 3+ screen per entity cases
	 * */
	if(!pSmi->pHardware)
	{
        /*
		 * Allocate an 'Chip'Rec, and hook it into pScrn->driverPrivate.
		 * pScrn->driverPrivate is initialised to NULL, so we can check if
		 * the allocation has already been done.
		 */
        SMI502HWPtr p502Hw;
		p502Hw = pSmi->pHardware = (SMIHWPtr)XNFcalloc(sizeof(SMI502HWRec));
		if(pSmi->pHardware == NULL)
			LEAVE(NULL);
		memcpy(pSmi->pHardware,&sm502_hwrec,sizeof(SMI502HWRec));

        p502Hw->pRegSave = xnfcalloc(sizeof(SMI502_RegRec),1);
		if(p502Hw->pRegSave == NULL)
			LEAVE(NULL);
#if 0
		p502Hw->fonts = xalloc(KB(64));
		if(p502Hw->fonts == NULL)
			LEAVE(NULL);
#endif
		
		pSmi->pHardware->dual = 1;
		pSmi->pHardware->primary_screen_rec = pSmi;
		pSmi->screen = 0;
        ///////////////////////////
        //ilena:how???? copy from 750. not sure...
        #ifdef XSERVER_LIBPCIACCESS
		pSmi->pHardware->phyaddr_reg = pPci->regions[1].base_addr;
		pSmi->pHardware->physize_reg = 0x200000;
		pSmi->pHardware->phyaddr_mem = pPci->regions[0].base_addr;
		//pSmi->pHardware->physize_mem = pPci->regions[0].size;
		/*Get total video memory size from misc ccontrol register MMIO_base+0x0004 bit13 and bit12*/
        //pSmi->pHardware->physize_mem = ddk502_getFrameBufSize();

		pSmi->pHardware->devId = pPci->device_id;
	#else
		pSmi->pHardware->phyaddr_reg = pPci->memBase[1];
		pSmi->pHardware->physize_reg = 0x200000;
		pSmi->pHardware->phyaddr_mem = pPci->memBase[0];
		/*Get total video memory size from misc ccontrol register MMIO_base+0x0004 bit13 and bit12*/
		//pSmi->pHardware->physize_mem = pPci->size[0];
        //pSmi->pHardware->physize_mem = ddk502_getFrameBufSize();
		pSmi->pHardware->devId = pPci->device;
	#endif
	
	#ifdef XSERVER_LIBPCIACCESS
		pSmi->pHardware->revId = pPci->revision;
		pSmi->pHardware->pPci = pPci;
	#else
	    pSmi->pHardware->revId = pPci->chipRev;
        pSmi->pHardware->pPci = pPci;
	#endif
        /////////////////////////
        }else{
		/*
		 * pSmi->pHardware is not NULL, which means current entity already
		 * mallocated a SMIHWPtr structure,so we are in dualview 
		 * mode!
		 * */
		pSmi->pHardware->dual += 1;/*The total number of screen*/
		pSmi->screen = (pSmi->pHardware->dual)-1;/*The index of screen*/
	}
	LEAVE(pSmi);
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sm502_pcDeepmap
 *  Description:  
 * =====================================================================================
 */
void sm502_pcDeepmap(SMIHWPtr pHw)
{
	ENTER();

	ddk502_set_mmio(pHw->pReg,pHw->devId,0xC0);

	SMI502HWPtr p502Hw = (SMI502HWPtr)pHw;
	SMIPtr pSmi = HWPSMI(pHw);
	ScrnInfoPtr pScrn = SMIPSI(pSmi);
            pHw->DPRBase = pHw->pReg + 0x100000;
            pHw->VPRBase = pHw->pReg + 0x000000;
            pHw->CPRBase = pHw->pReg + 0x090000;
            pHw->DCRBase = pHw->pReg + 0x080000;
            pHw->SCRBase = pHw->pReg + 0x000000;
            pHw->IOBase = 0;
            pHw->DataPortBase = pHw->pReg + 0x110000;
            pHw->DataPortSize = 0x10000;
	LEAVE(TRUE);
}
void sm502_vgaAndpciInit(ScrnInfoPtr pScrn,int entityIndex)
{
        ENTER();

        LEAVE();

}

void sm502_entityInit(int entityIndex,pointer private)
{	
    ENTER();
   
    struct pci_device * pd;
    EntityInfoPtr pEnt;	
    //ilena: sth have been moved to entityinit_common.
    pd = xf86GetPciInfoForEntity(entityIndex);
    pEnt = xf86GetEntityInfo (entityIndex);	
    pci_device_probe(pd);// this line only used for 502.
    xfree(pEnt);

    ddk502_initHw();

    LEAVE();
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sm502_entityEnter
 *  Description:  save console's registers
 * =====================================================================================
 */
void sm502_entityEnter(int entIndex,pointer private)
{
    ENTER();
    ddk502_initHw();
	LEAVE();
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sm502_entityLeave
 *  Description:  restore console's registers
 * =====================================================================================
 */
void sm502_entityLeave(int entIndex,pointer private)
{
	ENTER();
	
	/* Restore the registers */
        //restore_reg_502((SMIHWPtr)private);
	//save_reg_502((SMIHWPtr)private);
	
	LEAVE();
}

void sm502_saveText(ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR (pScrn);

	ENTER();
	/* This functin called from entity init,
	Of course, it is the primary screen */
	if(xf86IsPrimaryPci(pSmi->pHardware->pPci))
	{	
		/* Save the vga fonts */
		//sm750_saveFonts(pSmi->pHardware);
		/* Save the registers */
		//save_reg_502(pSmi->pHardware);
	}
	LEAVE();
}

void sm502_getMapResult(ScrnInfoPtr pScrn)
{
   	SMIPtr pSmi = SMIPTR(pScrn);
	SMIHWPtr pHw = pSmi->pHardware;
	ENTER();

	/* set video memory size */
	pSmi->videoRAMBytes = pHw->physize_mem / (pSmi->pHardware->dual);
	pScrn->videoRam = (pSmi->videoRAMBytes)>>10;/* kbytes*/

	/* set pScrn physica address*/
#if 1
	pScrn->memPhysBase = pHw->phyaddr_mem;
	pScrn->memPhysBase += (pSmi->screen) * pSmi->videoRAMBytes;
#else
    pScrn->memPhysBase = 0;
#endif

	/* set OFFSET */// delete tmp by ilena
	pScrn->fbOffset = pSmi->FBOffset = pScrn->memPhysBase - pHw->phyaddr_mem;

    /* monk:below is wrong,pScrn->memPhysBase should not pretend the 
     * physica address of video memory,but the offset of the videomemory
     * of onscreen! So just set to 0 is okay !
     * */
    if (!pSmi->screen)
        //pSmi->fbMapOffset = 0x0;
        pSmi->FBOffset = 0x0;
    else
        //pSmi->fbMapOffset = pScrn->videoRam * 1024;
        pSmi->FBOffset = pScrn->videoRam * 1024;

	/* set virtual address */
	pSmi->pFB = pHw->pMem;
	pSmi->pFB += (pSmi->screen) * pSmi->videoRAMBytes;

    //pSmi->FBCursorOffset = pSmi->base.videoRAMBytes - 2048;
	pSmi->FBReserved = pSmi->videoRAMBytes - 4096;

#if 0 //ilena: why dont we use this?
    if (pSmi->IsLCD) {
        pSmi->lcd = 1;
    }
#endif

    if (!pSmi->width)
		pSmi->width = pScrn->displayWidth;
	if (!pSmi->height)
		pSmi->height = pScrn->virtualY;
	//ilena:seems moved from modeInit
	pSmi->Bpp = pScrn->bitsPerPixel / 8;
	pSmi->Stride = (pSmi->width * pSmi->Bpp + 15) & ~15;

    LEAVE(TRUE);

}

void sm502_handleOptions(ScrnInfoPtr pScrn)
{
    SMI502_Ptr pSmi;
    pSmi = SMI502_PTR (pScrn);
    MessageType from;// i'm not sure that we need this.
        if (pScrn->depth == 8)
        {
                pScrn->rgbBits = 8;
        }
    #if 1 // maybe will use it in handleoption func.
    /* 
       if no HWCursor option defined in xorg.conf ,then software cursor will 
           be used by default
     */
    pSmi->hwcursor = FALSE;
    if (xf86GetOptValBool (pSmi->base.Options, OPTION_HWCURSOR, &pSmi->hwcursor)){
        from = X_CONFIG;
    }
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,5,1,0,0)
    if(pScrn->scrnIndex != 0)
    {
        if(pScrn->depth != xf86Screens[0]->depth)
        {
            /*	for X server 1.51+ , use ugly hardware cursor for
             *	none-primary screen or system will hang kind of X server bug	
             */
            pSmi->hwcursor = TRUE;
        }				
    }
#endif	
    xf86DrvMsg (pScrn->scrnIndex, from, "Using %s Cursor\n",
            pSmi->hwcursor ? "Hardware" : "Software");

    if (xf86GetOptValInteger (pSmi->base.Options, OPTION_VIDEOKEY, &pSmi->videoKey))
    {
        xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: Video key set to "
                "0x%08X\n", pSmi->videoKey);
    }
    else
    {
        /* should not use pScrn->offset now,because screenInit will 
         * modify their value,the videoKey will be set in ScreenInit after
         * the pScrn->offset modified 
         * */
    }

    if (xf86ReturnOptValBool (pSmi->base.Options, OPTION_BYTESWAP, FALSE))
    {
        pSmi->ByteSwap = TRUE;
        xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: ByteSwap enabled.\n");
    }
    else
    {
        pSmi->ByteSwap = FALSE;
    }
#endif

}

void sm502_adjustFrame(ScrnInfoPtr pScrn, int offset)
//void SMI502_AdjustFrame (int scrnIndex, int x, int y, int flags)
{
    return 0;// ilena:how to re-write it?
/*    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    SMI502_Ptr pSmi = SMI502_PTR (pScrn);
    CARD32 Base;

    if (pSmi->ShowCache && y)
    {
        y += pScrn->virtualY - 1;
    }

    xf86DrvMsg("", X_INFO, "pSmi->FBOffset is 0x%x, x is %d, y is %d\n", 
            pSmi->FBOffset, x, y);
    Base = pSmi->FBOffset + (x + y * pScrn->displayWidth) * pSmi->Bpp;

    if (!pSmi->IsSecondary)//ilena: to dual
    {
        WRITE_DCR (pSmi, DCR0C, Base);
    }
    else
    {
        WRITE_DCR (pSmi, DCR204, Base);
    }*/
}

void sm502_closeAllScreen(SMIHWPtr pHw)
{
	ENTER();
	SMI502HWPtr	p502Hw = (SMI502HWPtr)pHw;

	if(p502Hw->pRegSave)
	{
		xf86Msg(X_INFO,"Close Screen Free saved reg\n");
		xfree(p502Hw->pRegSave);
		p502Hw->pRegSave = NULL;
	}
#if 0
	if(p502Hw->fonts)
	{
		xf86Msg(X_INFO,"Close Screen Free saved fonts\n");
		xfree(p502Hw->fonts);
		p502Hw->fonts = NULL;
	}
#endif
	xfree(p502Hw);
	LEAVE();
}

ModeStatus sm502_validMode(ScrnInfoPtr pScrn,DisplayModePtr pMode)
{
        ENTER();
        LEAVE (MODE_OK);
}

void sm502_setMode(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    ENTER();
    SMIPtr pSmi = SMIPTR (pScrn);
    unsigned int tmpData, tmpReg;
    unsigned long dataLength;
    double refresh;
    logicalMode_t LogicalMode;
    LogicalMode.x= mode->HDisplay;
    LogicalMode.y =mode->VDisplay;
    LogicalMode.bpp= pScrn->depth;
    
    LogicalMode.baseAddress = pSmi->FBOffset;
    LogicalMode.pitch=(pSmi->width * pSmi->Bpp + 15) & ~15;/*Dont use pSmi->Stride, it had been changed in accel.c*/
    LogicalMode.virtual= 0; 
    LogicalMode.xLCD=pSmi->width;
    LogicalMode.yLCD=pSmi->height;
	//ilena: adjust vertical refresh.
    refresh = mode->VRefresh;
    if(!mode->VRefresh)
    {
        if (mode->HTotal > 0 && mode->VTotal > 0) {
        	    refresh = mode->Clock * 1000.0 / mode->HTotal / mode->VTotal;
        	if (mode->Flags & V_INTERLACE)
        	    refresh *= 2.0;
        	if (mode->Flags & V_DBLSCAN)
        	    refresh /= 2.0;
        	if (mode->VScan > 1)
        	    refresh /= (float)(mode->VScan);
        }else
            refresh = 60;
    }	
#if SMI_RANDR
	if(pSmi->lcdWidth * pSmi->lcdHeight != 0){
	    /* expansion mode, only 60 hz is valid */
        LogicalMode.hz = 60;
	}else{	    
	    /* no expansion */	    
	    /*LogicalMode.hz  = (uint32_t)(((mode->VRefresh!=0) ? mode->VRefresh : adjusted_mode->VRefresh) + 0.5);	*/
		/*Above sentence is more compatible, but hard code is a simple select
		Because the adjusted_mode->VRefresh = 60.0038414 */
		//LogicalMode.hz = 60;
		LogicalMode.hz  = (uint32_t)(refresh+ 0.5);
    }	
	if (pSmi->dispCtrl == PANEL)
    {
        LogicalMode.dispCtrl=PANEL_CTRL;
        ddk502_setModeTiming(&LogicalMode);	
        tmpReg = DCR00;
    }
	else if (pSmi->dispCtrl == CRT)
    {
        LogicalMode.dispCtrl=CRT_CTRL;
        ddk502_setModeTiming(&LogicalMode);
        tmpReg = DCR200;
    }
#else
    if (!pSmi->screen)
    {
        LogicalMode.hz = 60;
        LogicalMode.dispCtrl=PANEL_CTRL;
        ddk502_setModeTiming(&LogicalMode);	
        tmpReg = DCR00;
    }
    else
    {
        LogicalMode.hz = (int) (refresh + 0.5);
        LogicalMode.dispCtrl=CRT_CTRL;
        ddk502_setModeTiming(&LogicalMode);
        tmpReg = DCR200;
    }
#endif
    /*ilena: open 2D engine CLOCK Control. enable it so that 2d could work*/
	unsigned long dwVal = ddk502_PEEK32(POWER_MODE0_GATE);
	dwVal = FIELD_SET(dwVal, POWER_MODE0_GATE, 2D, ENABLE);
	ddk502_POKE32(POWER_MODE0_GATE,dwVal);

    /* Jason 2010/09/21
       Set registers according to modeline sync polarity */
       SMIHWPtr pHw = pSmi->pHardware;
    if (mode->Flags & V_PHSYNC) {
        tmpData = READ_DCR(pHw, tmpReg);
        WRITE_DCR(pHw, tmpReg, (tmpData & 0xFFFFEFFF));
    } 
    if (mode->Flags & V_NHSYNC){
        tmpData = READ_DCR(pHw, tmpReg);
        WRITE_DCR(pHw, tmpReg, (tmpData | 0x00001000));
    } 
    if (mode->Flags & V_PVSYNC){
        tmpData = READ_DCR(pHw, tmpReg);
        WRITE_DCR(pHw, tmpReg, (tmpData & 0xFFFFDFFF));
    } 
    if (mode->Flags & V_NVSYNC) {
        tmpData = READ_DCR(pHw, tmpReg);
        WRITE_DCR(pHw, tmpReg, (tmpData | 0x00002000));
    } 

    tmpData = READ_SCR(pHw, SYSTEM_CTRL);
    WRITE_SCR(pHw, SYSTEM_CTRL, (tmpData | 0x20000000));

    LEAVE ();
}

void sm502_setDisplay(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
	int pitch;
	int bpp = pScrn->bitsPerPixel;
	SMIPtr pSmi = SMIPTR (pScrn);

	display_t channel;
	
#if SMI_RANDR
	if (pSmi->dispCtrl == PANEL)
		channel = PANEL;
	else if (pSmi->dispCtrl == CRT)
		channel = CRT;
#else	
	if(pSmi->IsPrimary)
		channel = PANEL;
	else
		channel = CRT;
#endif
    /* 
     * Pitch value calculation in Bytes.
     * Usually, it is (screen width) * (byte per pixel).
     * However, there are cases that screen width is not 16 pixel aligned, which is
     * a requirement for some OS and the hardware itself.
     * For standard 4:3 resolutions: 320, 640, 800, 1024 and 1280, they are all
     * 16 pixel aligned and pitch is simply (screen width) * (byte per pixel).
     *   
     * However, 1366 resolution, for example, has to be adjusted for 16 pixel aligned.
     */
    /* "+ 15) & ~15)" This calculation has no effect on 640, 800, 1024 and 1280. */
    pitch = ((pSmi->width+ 15) & ~15) * (bpp / 8);
	
	set_display_502(pSmi->pHardware, channel, bpp, pitch);  
}


static int SMI502_InternalScreenInit (int scrnIndex, ScreenPtr pScreen)
{
#if 0
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SMI502_Ptr pSmi = SMI502_PTR (pScrn);
    int width, height, displayWidth;
    int bytesPerPixel = pScrn->bitsPerPixel / 8;
    int xDpi, yDpi;
    int ret;

    ENTER();

    if (pSmi->rotate && pSmi->rotate != SMI_ROTATE_UD)
    {
        width = pScrn->virtualY;
        height = pScrn->virtualX;
        xDpi = pScrn->yDpi;
        yDpi = pScrn->xDpi;
        displayWidth = ((width * bytesPerPixel + 15) & ~15) / bytesPerPixel;
    }
    else
    {
        width = pScrn->virtualX;
        height = pScrn->virtualY;
        xDpi = pScrn->xDpi;
        yDpi = pScrn->yDpi;
        displayWidth = pScrn->displayWidth;
    }
    if (pSmi->shadowFB)
    {
        pSmi->ShadowWidth = width;
        pSmi->ShadowHeight = height;
        pSmi->ShadowWidthBytes = ((width + 15) & ~15) * bytesPerPixel;
        if (bytesPerPixel == 3)
        {
            if(pSmi->rotate == SMI_ROTATE_CW || pSmi->rotate == SMI_ROTATE_CCW)
            {
                pSmi->ShadowPitch = ((height * 3) << 16) |
                    pSmi->ShadowWidthBytes;
            }
            else
            {
                pSmi->ShadowPitch = ((width * 3) << 16) | 
                    pSmi->ShadowWidthBytes;
            }
        }
        else
        {
            if(pSmi->rotate == SMI_ROTATE_CW || pSmi->rotate == SMI_ROTATE_CCW)
            {
                pSmi->ShadowPitch = (((height + 15) & ~15)<< 16) |
                    (pSmi->ShadowWidthBytes / bytesPerPixel);
            }
            else
            {
                pSmi->ShadowPitch = (((width + 15) & ~15)<< 16) |
                    (pSmi->ShadowWidthBytes / bytesPerPixel);	
            }
        }
        pSmi->saveBufferSize = pSmi->ShadowWidthBytes * pSmi->ShadowHeight;
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
                "line %d: pSmi->FBReserved is 0x%x\n", 
                __LINE__, pSmi->base.FBReserved);
        pSmi->base.FBReserved -= pSmi->saveBufferSize;
        pSmi->base.FBReserved &= ~0x15;
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
                "line %d: pSmi->FBReserved is 0x%x\n", 
                __LINE__, pSmi->base.FBReserved);
        WRITE_VPR (pSmi, 0x0C, (pSmi->base.FBOffset = pSmi->base.FBReserved) >> 3);
        xf86DrvMsg("", X_INFO, 
                "pSmi->SCRBase is 0x%x, DCRBase is 0x%x, \
                FBOffset is 0x%x\n", 
                pSmi->SCRBase, pSmi->DCRBase, pSmi->base.FBOffset);
        if (!pSmi->base.screen) {
            WRITE_DCR (pSmi, DCR0C, pSmi->base.FBOffset);
        } else {
            WRITE_DCR (pSmi, DCR204, pSmi->base.FBOffset);
        }
        pScrn->fbOffset = pSmi->base.FBOffset + pSmi->base.FBOffset;//fbMapoffset
        xf86DrvMsg (pScrn->scrnIndex, X_INFO, "Shadow: width=%d height=%d "
                "offset=0x%08X pitch=0x%08X\n", pSmi->ShadowWidth,
                pSmi->ShadowHeight, pSmi->base.FBOffset, pSmi->ShadowPitch);
        xf86DrvMsg("", X_INFO, 
                "line %d: Internalxxx: fbOffset = 0x%x\n", 
                __LINE__, pScrn->fbOffset);
    }
    else {
        xf86DrvMsg("", X_INFO, "line %d: Internalxxx: fbOffset = 0x%x, \
                FBOffset is 0x%x\n", 
                __LINE__, pScrn->fbOffset, pSmi->base.FBOffset);
    }
    /*
     * Call the framebuffer layer's ScreenInit function, and fill in other
     * pScreen fields.
     */
    DEBUG ("\tInitializing FB @ 0x%08X for %dx%d (%d)\n",
            pSmi->base.pFB, width, height, displayWidth);
    switch (pScrn->bitsPerPixel)
    {
        /* #ifdef USE_FB*/
        case 8:
        case 16:
        case 24:
        case 32:
            {// by ilena: i dont think it should be keeped
                ret = fbScreenInit (pScreen, pSmi->base.pFB, width, height, xDpi,
                        yDpi, displayWidth, pScrn->bitsPerPixel);
            }
            break;
        default:
            xf86DrvMsg (scrnIndex, X_ERROR, "Internal error: invalid bpp (%d) "
                    "in SMI_InternalScreenInit\n", pScrn->bitsPerPixel);
            LEAVE (FALSE);
    }
    if(pScrn->bitsPerPixel == 8)
    {
        /* Initialize Palette entries 0 and 1, they don't seem to get hit */
        if (!pSmi->base.screen)
        {
            WRITE_DCR (pSmi, DCR400 + 0, 0x00000000);	/* CRT Palette       */
            WRITE_DCR (pSmi, DCR400 + 4, 0x00FFFFFF);	/* CRT Palette       */
        }
        else
        {
            WRITE_DCR (pSmi, DCR800 + 0, 0x00000000);	/* Panel Palette */
            WRITE_DCR (pSmi, DCR800 + 4, 0x00FFFFFF);	/* Panel Palette */
        }
    }
    LEAVE (ret);
#endif
	return 0;
}

void sm502_LoadPalette (ScrnInfoPtr pScrn, int numColors, int *indicies,
        LOCO * colors, VisualPtr pVisual)
{
    ENTER();
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    int i;
    int iRGB;

    /* Enable both the CRT and LCD DAC RAM paths, 
       so both palettes are updated */
    for (i = 0; i < numColors; i++)
    {
        DEBUG ("pal[%d] = %d %d %d\n", indicies[i],
                colors[indicies[i]].red, colors[indicies[i]].green,
                colors[indicies[i]].blue);

        iRGB = (colors[indicies[i]].red << 16) |
            (colors[indicies[i]].green << 8) | (colors[indicies[i]].blue);

        if (pSmi->screen)
        {
            /* CRT Palette   */
            WRITE_DCR (pHw, DCR400 + (4 * (indicies[i])), iRGB);	
        }
        else
            /* Panel Palette */
            WRITE_DCR (pHw, DCR800 + (4 * (indicies[i])), iRGB);	
    }
    LEAVE();
}

int sm502_totalFB(SMIHWPtr pHw)
{
	ENTER();
	
	LEAVE( ddk502_getFrameBufSize());
}
#if SMI_RANDR
void sm502_I2CInit(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);
	SMIHWPtr pHw = pSmi->pHardware;
    int cnt = pSmi->DualView?2:1;
    int index;
    I2CBusPtr ptr[2] = {NULL,NULL};
	static char * sm502_name[] = {"I2C Bus PanelPath","I2C Bus CrtPath"};	
    ENTER();
    
	pfn_I2CPutBits_502[0] = i2c_putbits_panel_502;
	pfn_I2CPutBits_502[1] = i2c_putbits_crt_502;
	pfn_I2CGetBits_502[0] = i2c_getbits_panel_502;
	pfn_I2CGetBits_502[1] = i2c_getbits_crt_502;

    index= 0 ;        
    while(index < cnt)
    {       
        if(ptr[index] == NULL )
        {
        	I2CBusPtr I2CPtr = xf86CreateI2CBusRec();
        	if (I2CPtr == NULL)
        	    return FALSE;

			I2CPtr->scrnIndex  = pScrn->scrnIndex;			
			I2CPtr->BusName  = sm502_name[index];
			I2CPtr->I2CPutBits = pfn_I2CPutBits_502[index];
			I2CPtr->I2CGetBits = pfn_I2CGetBits_502[index];			

						
        	I2CPtr->DriverPrivate.ptr = (void *)xalloc(sizeof(int));

        	if (!xf86I2CBusInit(I2CPtr)){				
        	    xfree(I2CPtr->DriverPrivate.ptr);
        	    xf86DestroyI2CBusRec(I2CPtr, TRUE, TRUE);
        	    LEAVE(FALSE);
        	}        	
     
        	ptr[index] = I2CPtr;
			/* 	for sm712: 0 means CRT HEAD i2c bus
				for sm750: 0 means PANEL HEAD i2c bus, 
				note that head is not pll
			*/
        	*((int *)I2CPtr->DriverPrivate.ptr) = index;
        }
        index++;
    }

    pSmi->I2C_primary = ptr[0];
    pSmi->I2C_secondary = ptr[1];

	init_i2c_502(pHw);
	
    LEAVE(TRUE);
}
#endif
