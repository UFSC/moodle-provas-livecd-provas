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

#include "smi_712_driver.h"

static const SMI712HWRec sm712_hwrec = {
	.base = {
		.pcDeepMap = sm712_pcDeepmap,/* actually,NULL value can be just ignored,gcc will know it*/
		.pcEntityInit = sm712_entityInit,
		.pcEntityEnter = sm712_entityEnter,
		.pcEntityLeave = sm712_entityLeave,	
		.pcCloseAllScreen = sm712_closeAllScreen,
		.pcFBSize = sm712_totalFB,
		.pcInitHardware = sm712_hardwareInit,
	},

};


static const SMI712Rec sm712_rec = {
	.base = {
		.minHeight = 128,
		.psGetMapResult = sm712_getMapResult,
		.psVgaPciInit = sm712_vgaAndpciInit,
		.psHandleOptions = sm712_handleOptions,
		.psValidMode = sm712_validMode,
		.psSetMode = sm712_setMode,
		.psAdjustFrame = sm712_adjustFrame,
		.psLoadPalette = sm712_LoadPalette,
		.psSetDisplay = sm712_setDisplay,
		.psSaveText = sm712_saveText,
	#if SMI_RANDR
		.psCrtcPreInit = SMI712_CrtcPreInit,
		.psOutputPreInit = SMI712_OutputPreInit,
		.psI2CInit = sm712_I2CInit,
	#endif
	},
};
SMIPtr sm712_genSMI(pointer priv,int entityIndex)
{
    SMIPtr pSmi;
#ifdef XSERVER_LIBPCIACCESS
    struct pci_device * pPci;
#else
    pciVideoPtr pPci;
#endif

    ENTER();
#ifdef XSERVER_LIBPCIACCESS	
    /*what's the difference between 'pScrn->entityList[0]' and 'pEntInfo->index'??*/
    pPci = xf86GetPciInfoForEntity(entityIndex);
#endif
    /* pSmi always need create for every screen*/
    pSmi = (SMIPtr)XNFcalloc(sizeof(SMI712Rec));	
    memset(pSmi,0, sizeof(SMI712Rec));
    if(pSmi == NULL)
    	return pSmi;
    memcpy(pSmi,&sm712_rec,sizeof(SMI712Rec));
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
    	SMI712HWPtr p712Hw;
    	p712Hw = pSmi->pHardware = (SMIHWPtr)XNFcalloc(sizeof(SMI712HWRec));
    	if(pSmi->pHardware == NULL)
    		LEAVE(NULL);
    	memcpy(pSmi->pHardware,&sm712_hwrec,sizeof(SMI712HWRec));
    	p712Hw->pRegSave = xnfcalloc(sizeof(SMI712RegRec),1);
    	if(p712Hw->pRegSave == NULL)
    		LEAVE(NULL);
		p712Hw->pRegMode = xnfcalloc(sizeof(SMI712RegRec),1);
    	if(p712Hw->pRegMode== NULL)
    		LEAVE(NULL);

    	p712Hw->fonts = xalloc(KB(64));
    	if(p712Hw->fonts == NULL)
    		LEAVE(NULL);
            	
    	pSmi->pHardware->dual = 1;
    	pSmi->pHardware->primary_screen_rec = pSmi;
    	pSmi->screen = 0;
#ifdef XSERVER_LIBPCIACCESS
    	pSmi->pHardware->devId = pPci->device_id;
#else
    	pSmi->pHardware->devId = pPci->device;
#endif
    	if(SMI_712 == pSmi->pHardware->devId)
    	{
	    	/* Put what you already known into structure */
#ifdef XSERVER_LIBPCIACCESS
	    	pSmi->pHardware->phyaddr_reg = pPci->regions[0].base_addr + SM712_REG_OFFSET;
	    	pSmi->pHardware->physize_reg = SM712_REG_SIZE;
	    	pSmi->pHardware->phyaddr_mem = pPci->regions[0].base_addr;
#else
	    	pSmi->pHardware->phyaddr_reg = pPci->memBase[0] + SM712_REG_OFFSET;
	    	pSmi->pHardware->physize_reg = SM712_REG_SIZE;
	    	pSmi->pHardware->phyaddr_mem = pPci->memBase[0] ;
#endif
		}
		else/*if(SMI_722 == pSmi->pHardware->devId)*/
		{
	    	/* Put what you already known into structure */
#ifdef XSERVER_LIBPCIACCESS
	    	pSmi->pHardware->phyaddr_reg = pPci->regions[0].base_addr;
	    	pSmi->pHardware->physize_reg = SM722_REG_SIZE;
	    	pSmi->pHardware->phyaddr_mem = pPci->regions[0].base_addr + SM722_FB_OFFSET;
#else
	    	pSmi->pHardware->phyaddr_reg = pPci->memBase[0];
	    	pSmi->pHardware->physize_reg = SM722_REG_SIZE;
	    	pSmi->pHardware->phyaddr_mem = pPci->memBase[0] + SM722_FB_OFFSET;
#endif
		}

#ifdef XSERVER_LIBPCIACCESS
    	pSmi->pHardware->revId = pPci->revision;
    	pSmi->pHardware->pPci = pPci;
#endif
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

void sm712_pcDeepmap(SMIHWPtr pHw)
{
	vgaHWPtr hwp;
	SMI712HWPtr p712Hw = (SMI712HWPtr)pHw;
	SMIPtr pSmi = HWPSMI(pHw);
	ScrnInfoPtr pScrn = SMIPSI(pSmi);
	ENTER();
	
    ddk712_set_mmio(pHw->pReg,pHw->devId);
	switch (pHw->devId) 
    {
		case SMI_712:
			pHw->DPRBase = pHw->pReg + 0x008000;
			pHw->VPRBase = pHw->pReg + 0x00C000;
			pHw->CPRBase = pHw->pReg + 0x00E000;
			pHw->IOBase  = pHw->pReg + 0x300000;
			pHw->DataPortBase = pHw->pReg + 0x100000;
			pHw->DataPortSize = 0x200000;
			break;
		case SMI_722:
            		pHw->DPRBase = pHw->pReg + 0x000000;
            		pHw->VPRBase = pHw->pReg + 0x000800;
            		pHw->CPRBase = pHw->pReg + 0x001000;
            		pHw->IOBase  = pHw->pReg + 0x0C0000;
            		pHw->DataPortBase = pHw->pReg + 0x100000;
            		pHw->DataPortSize = 0x100000;
    		        break;
		default:
			xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "SM712 chip ID error.\n");        
			break;
	 }  
	   pHw->DCRBase = 0;
            pHw->SCRBase = 0;
    LEAVE(TRUE);
}
extern void enable_mmio_712(ScrnInfoPtr pScrn);
void sm712_vgaAndpciInit(ScrnInfoPtr pScrn,int entityIndex)
{
    ENTER();
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    enable_mmio_712(pScrn);
    LEAVE();
}
void sm712_hardwareInit(SMIHWPtr pHw)
{
	ENTER();
	init_parm_712 initParam;
	SMIPtr pSmi = HWPSMI(pHw);
	SMI712HWPtr p712Hw = (SMI712HWPtr)pHw;
	SMI712RegPtr mode = p712Hw->pRegMode;

	initParam.devid = pHw->devId;

	if(pSmi->pci_burst)
	initParam.pci_burst = 2;
	else
	initParam.pci_burst = 0;

	sm712_DetectMCLK(pSmi);
	/*130 mhz is the maximum for sm712 BA version*/
	initParam.memClock = pSmi->MCLK; //130*1000000;

	initParam.lcd = LCD712_TFT;
	/*In the current stage, it cant get the tft color*/
	initParam.lcd_color.tftColor = 0;
	initParam.lcd_color.dstnColor = DSTN_USE_JUMP;
	/* Disable DAC and LCD framebuffer r/w operation */
	/*add for mosaic of screen. by ilena*/
	mode->SR21 |= 0xB0;
#if 0 //use it later
	if (pSmi->lcd == 2) /* Panel is DSTN */
		mode->SR21 = 0x00;
#endif
	ddk712_hw_init(&initParam);
	LEAVE();
}

void sm712_entityInit(int entIndex,pointer private)
{
	ENTER();
	SMIHWPtr pHw = (SMIHWPtr)private;
	SMIPtr pSmi = HWPSMI(pHw);
	ddk712_set_mmio(pHw->pReg,pHw->devId);
	LEAVE();
}

void sm712_DetectMCLK(SMIPtr pSmi )
{
    double		real;

	pSmi->MCLK = 130*1000000;/*130 mhz is the maximum for sm712 BA version*/
   /* MCLK from user settings */
    if (xf86GetOptValFreq(pSmi->Options, OPTION_MCLK, OPTUNITS_MHZ, &real)) 
    {
		if ((int)real <= 120) 
            pSmi->MCLK = (int)(real * 1000.0);
		else
            xf86Msg(X_INFO, "Memory Clock %1.3f MHz larger than limit of 120 MHz\n", real);
        }
  xf86Msg( X_INFO, "MCLK = %1.3f\n", pSmi->MCLK  / 1000.0);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sm712_entityEnter
 *  Description:  save console's registers
 * =====================================================================================
 */
void sm712_entityEnter(int entIndex,pointer private)
{
        ENTER();
        init_parm_712 initParam;
        SMIHWPtr pHw = (SMIHWPtr)private;
        SMIPtr pSmi = HWPSMI(pHw);
        ScrnInfoPtr	pScrn = SMIPSI(pSmi);
        initParam.devid = pHw->devId;

        if(pSmi->pci_burst)
			initParam.pci_burst = 2;
        else
			initParam.pci_burst = 1;

        sm712_DetectMCLK(pSmi );
        /*130 mhz is the maximum for sm712 BA version*/
        initParam.memClock = pSmi->MCLK; //130*1000000;

        initParam.lcd = LCD712_TFT;
        /*In the current stage, it cant get the tft color*/
        initParam.lcd_color.tftColor = 0;	
        enable_mmio_712(pScrn);
        ddk712_hw_init(&initParam);
        LEAVE();
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sm712_entityLeave
 *  Description:  restore console's registers
 * =====================================================================================
 */
extern void disable_mmio_712(ScrnInfoPtr pScrn);
void sm712_entityLeave(int entIndex,pointer private)
{
    SMIHWPtr pHw = (SMIHWPtr)private;
    ScrnInfoPtr pScrn;
    SMIPtr pSmi = HWPSMI(pHw);
    pScrn = SMIPSI(pSmi);
	ENTER();
	if(xf86IsPrimaryPci(pHw->pPci))
	{
		/* Restore the registers */
		restore_reg_712((SMIHWPtr)private);
		/* Restore the vga fonts */
		sm712_restoreFonts((SMIHWPtr)private);

        //   vgaHWUnmapMem(pScrn);
        disable_mmio_712(pScrn);
	}
	LEAVE();
}
void sm712_saveText(ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR (pScrn);
	SMIHWPtr pHw = pSmi->pHardware;
	ENTER();
	/* This functin called from entity init,
	Of course, it is the primary device */
	if(xf86IsPrimaryPci(pSmi->pHardware->pPci))
	{	
		/* Save the vga fonts */
		sm712_saveFonts(pSmi->pHardware);
		/* Save the registers */
		save_reg_712(pSmi->pHardware);
	}
	LEAVE();
}

void sm712_getMapResult(ScrnInfoPtr pScrn)
{
    ENTER();
    vgaHWPtr hwp;
    SMIPtr pSmi = SMIPTR(pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    SMI712HWPtr p712Hw = (SMI712HWPtr)pHw;

	/* set video memory size */
	pSmi->videoRAMBytes = pHw->physize_mem / (pSmi->pHardware->dual);
	pScrn->videoRam = (pSmi->videoRAMBytes)>>10;/* kbytes*/

	/* set pScrn physica address*/
	pScrn->memPhysBase = pHw->phyaddr_mem;
	pScrn->memPhysBase += (pSmi->screen) * pSmi->videoRAMBytes;

	/* set OFFSET */
	pScrn->fbOffset = pSmi->FBOffset = pScrn->memPhysBase - pHw->phyaddr_mem;

	/* set virtual address */
	pSmi->pFB = pHw->pMem;
	pSmi->pFB += (pSmi->screen) * pSmi->videoRAMBytes ;

	//pSmi->FBReserved = pSmi->videoRAMBytes - 2048;

	if (!pSmi->width)
		pSmi->width = pScrn->displayWidth;
	if (!pSmi->height)
		pSmi->height = pScrn->virtualY;
	
	pSmi->Bpp = pScrn->bitsPerPixel / 8;
	pSmi->Stride = (pSmi->width * pSmi->Bpp + 15) & ~15;
        /* set up the fifo reserved space */       
        if (VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x30) & 0x01)
        {    /*  if DSTN panel selected */
            /* #1074 */ 
            CARD32 fifoOffset = 0;
            fifoOffset |= VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA,
                    0x46) << 3;
            fifoOffset |= VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA,
                    0x47) << 11;
            fifoOffset |= (VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA,
                        0x49) & 0x1C) << 17;
            pSmi->FBReserved = fifoOffset;	/* PDR#1074 */
        }
        else    /*  if TFT panel selected */
        {
            /*    sm712 only handle 4 mega video memory which means 
                            hardware cursor is not suitable for this chip of new arch x server driver  */       
            pSmi->FBReserved = pSmi->videoRAMBytes;
        }
	//SMI_DisableVideo (pScrn);

	hwp = VGAHWPTR (pScrn);

      if (!pSmi->screen)
      {
            if (pHw->IOBase != NULL)
                vgaHWSetMmioFuncs(hwp, (char*)pHw->pReg, pHw->IOBase - (CARD8*)pHw->pReg);
            
            vgaHWGetIOBase(hwp);//tmp by ilena

            /* Map the VGA memory when the primary video */
            if (xf86IsPrimaryPci(pHw->pPci))
            {
                hwp->MapSize = 0x10000;
                if (!vgaHWMapMem(pScrn))
                    LEAVE(FALSE);
            }
            /*	    vgaCRReg   = vgaIOBase + VGA_CRTC_DATA_OFFSET;  */
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,12,0,0,0)    
            pHw->PIOBase= hwp->PIOOffset;
#else
            pHw->PIOBase = 0;//X donot use the PIOOffset anymore.
#endif
    }
    LEAVE();
}

void sm712_handleOptions(ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR(pScrn);	
	ENTER();
	
    if (pScrn->depth == 8)
    {
    	pScrn->rgbBits = 6;//why sm712/sm722 couldn't support 8bpp depth?
    }           

    LEAVE();
}

ModeStatus sm712_validMode(ScrnInfoPtr pScrn,DisplayModePtr pMode)
{
	ENTER();
	LEAVE(MODE_OK);
}

void sm712_setMode(ScrnInfoPtr pScrn,DisplayModePtr pMode)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    ENTER();
    save_dpr_address(pHw);

    if(pSmi->IsCSCVideo == TRUE)
    {
        pSmi->IsCSCVideo = FALSE; // do not use CSC Video for 712/722 chips.
        xf86Msg(X_INFO, "SM712/722 do not support CSC Video.\n");
    }
    int channel, width, height, hz;
    channel = width = height = hz = 0;
    if (pSmi->screen == 0)// i'm not sure. by ilena.
            channel = 1;
    else if (pSmi->screen == 1)
            channel = 0;
    width = pMode->HDisplay;
    height = pMode->VDisplay,
    hz = pMode->VRefresh;

    ddk712_setModeTiming(channel, width, height, hz);
    LEAVE();
}
extern set_display_712(SMIHWPtr pHw,display_t channel,ScrnInfoPtr pScrn,uint32_t offset,int x,int y);
void sm712_setDisplay(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    ENTER();
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    display_t channel;
    if(!pHw)
    		return;
    if(pSmi->screen)
        channel = PANEL;
    else
        channel = CRT;
    set_display_712(pHw, channel, pScrn, pSmi->FBOffset, mode->HDisplay, mode->VDisplay);

    LEAVE();
}

void sm712_adjustFrame(ScrnInfoPtr pScrn,int offset)
{
    ENTER();
    LEAVE();
}

void sm712_closeAllScreen(SMIHWPtr pHw)
{
    ENTER();
    LEAVE();
}

/* Save vga fonts */
void sm712_saveFonts(SMIHWPtr pHw)
{	
    SMI712HWPtr	p712Hw = (SMI712HWPtr)pHw;

    ENTER();

    /* leave if in graphic mode */
    //if(get_vga_mode_712(pHw) & 0x2)

    memcpy((char *)p712Hw->fonts,(char *)pHw->pMem + KB(0),KB(64));

    /* not use legency method to access vga fonts , or multi-card will cry */

    LEAVE();
}

void sm712_restoreFonts(SMIHWPtr pHw)
{
    SMI712HWPtr	p712Hw = (SMI712HWPtr)pHw;

    ENTER();
    if(p712Hw->fonts)
    	memcpy((char *)pHw->pMem + KB(0),(char *)p712Hw->fonts,KB(64));

    /* not use legency method or multi-card cries */

    LEAVE();
}
/*
	1. SM712 only support 2M/4M video buffer.
	2. Whatever the video buffer size, SM712 registers is always 4M 
	2.2 and start from the video memory + 4M
	3. Video memory and registers is in a continuous space
	
*/
int sm712_totalFB(SMIHWPtr pHw)
{
    ENTER();
    unsigned int total_memory;			
    unsigned int *ptr;
    char tmp;
    int err;

    switch(pHw->devId)
	{
        case SMI_712:
            total_memory = MB(4);   
            break;
        case SMI_722:
            total_memory = MB(8);
            break;
        default:
            break;
	}
    LEAVE(total_memory);
}
void sm712_LoadPalette (ScrnInfoPtr pScrn, int numColors, 
        int *indicies, LOCO * colors, VisualPtr pVisual)
{
    ENTER();

    SMIPtr pSmi = SMIPTR (pScrn);
    int i;
    int iRGB;
    SMIHWPtr pHw = pSmi->pHardware;

        /* Enable both the CRT and LCD DAC RAM paths, so both palettes are updated */
        if (pHw->devId == SMI_LYNX3DM) 
        {
        	CARD8 ccr66;

        	ccr66 = VGAIN8_INDEX_712 (pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, CCR66);
        	ccr66 &= 0x0f;
        	VGAOUT8_INDEX_712 (pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, CCR66, ccr66);
        }

        for (i = 0; i < numColors; i++)
        {
   /*     	DEBUG ("pal[%d] = %d %d %d\n", indicies[i],
        			colors[indicies[i]].red, colors[indicies[i]].green,
        			colors[indicies[i]].blue);*/
        	VGAOUT8_712 (pHw, VGA_DAC_WRITE_ADDR, indicies[i]);
        	VGAOUT8_712 (pHw, VGA_DAC_DATA, colors[indicies[i]].red);
        	VGAOUT8_712 (pHw, VGA_DAC_DATA, colors[indicies[i]].green);
        	VGAOUT8_712 (pHw, VGA_DAC_DATA, colors[indicies[i]].blue);
        }

    LEAVE();
}
#if SMI_RANDR
void sm712_I2CInit(ScrnInfoPtr pScrn)
{

    SMIPtr pSmi = SMIPTR(pScrn);
    int cnt = pSmi->DualView?2:1;
    int index;
    I2CBusPtr ptr[2] = {NULL,NULL};
    static char * sm712_name[] = {"I2C Bus crt","I2C Bus lcd"};
	static char * sm750_name[] = {"I2C Bus PanelPath","I2C Bus CrtPath"};	
    ENTER();
    
    index= 0 ;        
    while(index < cnt)
    {       
        if(ptr[index] == NULL )
        {
        	I2CBusPtr I2CPtr = xf86CreateI2CBusRec();
        	if (I2CPtr == NULL)
        	    return FALSE;

		I2CPtr->scrnIndex  = pScrn->scrnIndex;
	       	I2CPtr->BusName  = sm712_name[index];
	       	I2CPtr->I2CPutBits = sm712_I2CPutBits;
	       	I2CPtr->I2CGetBits = sm712_I2CGetBits;			
        	I2CPtr->DriverPrivate.ptr = (void *)xalloc(sizeof(int));

        	if (!xf86I2CBusInit(I2CPtr)){				
        	    xfree(I2CPtr->DriverPrivate.ptr);
//error:
        	    xf86DestroyI2CBusRec(I2CPtr, TRUE, TRUE);
        	    LEAVE(FALSE);
        	}        	
        	
		ptr[index] = I2CPtr;
			/* 	for sm712: 0 means CRT HEAD i2c bus
				note that head is not pll
			*/
        	*((int *)I2CPtr->DriverPrivate.ptr) = index;
        }
        index++;
    }
    pSmi->I2C_primary = ptr[0];
    pSmi->I2C_secondary = ptr[1];
}
void sm712_I2CPutBits(I2CBusPtr b, int clock,  int data)
{
    SMIPtr pSmi = SMIPTR(xf86Screens[b->scrnIndex]);
    unsigned char reg = 0x30;    
    int crtcIndex = *(int *)b->DriverPrivate.ptr;

    if (clock) reg |= 0x01;
    if (data)  reg |= 0x02;

    if(crtcIndex == 0 ){
        VGAOUT8_INDEX_712(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x72, reg);
    }else{
        VGAOUT8_INDEX_712(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x73, reg);    
    }     

    
#ifdef SMI_RECORD
    if(crtcIndex == 0){
    record_append("P",1);
    record_append(clock?"1":"0",1);
    record_append(data?"1":"0",1);
    record_append("\n",1);
    }
#endif    
}

void sm712_I2CGetBits(I2CBusPtr b, int *clock, int *data)
{
    SMIPtr pSmi = SMIPTR(xf86Screens[b->scrnIndex]);
    unsigned char reg;
    int crtcIndex = *(int *)b->DriverPrivate.ptr;
    
    if(crtcIndex == 0 ){
        reg = VGAIN8_INDEX_712(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x72);        
    }else{
        reg = VGAIN8_INDEX_712(pSmi, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x73);
    }

    *clock = reg & 0x04;
    *data  = reg & 0x08;

#ifdef SMI_RECORD
    if(crtcIndex == 0){
    record_append("G",1);
    record_append(*clock?"1":"0",1);
    record_append(*data?"1":"0",1);
    record_append("\n",1);
    }
#endif    
} 
#endif
