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
#include   "../smi_video.h"
#include	"smi_750_driver.h"
#include 	"smi_750_hw.h"
#include	"../smi_dbg.h"
#include "xf86Crtc.h"
#include "smi_crtc.h"

pll_value_t *pll_table;


void (*pfn_I2CPutBits_750[])(I2CBusPtr,int,int)=
{NULL,NULL};

void (*pfn_I2CGetBits_750[])(I2CBusPtr,int*,int*)=
{NULL,NULL};

/*-----------------------------------------------------------------------------
 *  they are just static datas,don't pass their address to X
 *  you can put down the member data you want into these const
 *  variable and make a copy of them to use in driver
 *  or dualview and multicard case will shut your driver off
 *
 *-----------------------------------------------------------------------------*/
static const SMI750HWRec sm750_hwrec = {
	.base = {
		.pcDeepMap = sm750_pcDeepmap,/* actually,NULL value can be just ignored,gcc will know it*/
		.pcEntityInit = sm750_entityInit,
		.pcEntityEnter = sm750_entityEnter,
		.pcEntityLeave = sm750_entityLeave,	
		.pcCloseAllScreen = sm750_closeAllScreen,
		.pcFBSize = sm750_totalFB,
		.pcInitHardware = NULL,
		.pcVideoReset = sm750_VideoReset,
	},

};


static const SMI750Rec sm750_rec = {
	.base = {
		.minHeight = 128,
		.psGetMapResult = sm750_getMapResult,
		.psVgaPciInit = sm750_vgaAndpciInit,
		.psHandleOptions = sm750_handleOptions,
		.psValidMode = sm750_validMode,
		.psSetMode = sm750_setMode,
		.psAdjustFrame = sm750_adjustFrame,
		.psLoadPalette = sm750_loadPalette,
		.psSetDisplay = sm750_setDisplay,
		.psSaveText = sm750_saveText,
	#if SMI_RANDR
		.psCrtcPreInit = SMI750_CrtcPreInit,
		.psOutputPreInit = SMI750_OutputPreInit,
		.psI2CInit = sm750_I2CInit,
	#endif
	},
};


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sm750_genSMI
 *  Description:  build up the entire private date for sm750 video card and initialize them.
 *	malloc an SMIPtr (named pSmi) and SMIHWPtr(named pHw) if in need
 *  setting confirmed member of pSmi and pHw (such as physical address,etc..)
 *  hook all known function pointer to pSmi and pHw
 *  in a sentence, put everything associated with SM750 this stage can confirm into the two structure !!!
 * =====================================================================================
 */
SMIPtr sm750_genSMI(pointer priv,int entityIndex)
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
	pSmi = (SMIPtr)XNFcalloc(sizeof(SMI750Rec));	
	memset(pSmi,0, sizeof(SMI750Rec));
	if(pSmi == NULL)
		return pSmi;
	memcpy(pSmi,&sm750_rec,sizeof(SMI750Rec));
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
		SMI750HWPtr p750Hw;
		p750Hw = pSmi->pHardware = (SMIHWPtr)XNFcalloc(sizeof(SMI750HWRec));
		if(pSmi->pHardware == NULL)
			LEAVE(NULL);
		memcpy(pSmi->pHardware,&sm750_hwrec,sizeof(SMI750HWRec));
		p750Hw->pRegSave = xnfcalloc(sizeof(SMI750RegRec),1);
		if(p750Hw->pRegSave == NULL)
			LEAVE(NULL);
		p750Hw->fonts = xalloc(KB(64));
		if(p750Hw->fonts == NULL)
			LEAVE(NULL);
		
		pSmi->pHardware->dual = 1;
		pSmi->pHardware->primary_screen_rec = pSmi;
		pSmi->screen = 0;
		
		/* Put what you already known into structure */
	#ifdef XSERVER_LIBPCIACCESS
		pSmi->pHardware->phyaddr_reg = pPci->regions[1].base_addr;
		pSmi->pHardware->physize_reg = 0x200000;
		pSmi->pHardware->phyaddr_mem = pPci->regions[0].base_addr;
		/*Get total video memory size from misc ccontrol register MMIO_base+0x0004 bit13 and bit12
		pSmi->pHardware->physize_mem = get_total_fb()*/;
		pSmi->pHardware->devId = pPci->device_id;
	#else
		pSmi->pHardware->phyaddr_reg = pPci->memBase[1];
		pSmi->pHardware->physize_reg = 0x200000;
		pSmi->pHardware->phyaddr_mem = pPci->memBase[0];
		/*Get total video memory size from misc ccontrol register MMIO_base+0x0004 bit13 and bit12
		pSmi->pHardware->physize_mem = get_total_fb()*/;
		pSmi->pHardware->devId = pPci->device;
	#endif
	
	#ifdef XSERVER_LIBPCIACCESS
		pSmi->pHardware->revId = pPci->revision;
		pSmi->pHardware->pPci = pPci;
	#else
	    //pSmi->pHardware->revId = pPci->chipRev; //ilena guess...
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
	
	/*Assign the private function for SM750*/

	LEAVE(pSmi);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sm750_pcDeepmap
 *  Description:  
 * =====================================================================================
 */
void sm750_pcDeepmap(SMIHWPtr pHw)
{
	ENTER();	
	SMI750HWPtr p750Hw = (SMI750HWPtr)pHw;
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

void sm750_vgaAndpciInit(ScrnInfoPtr pScrn,int entityIndex)
{
	ENTER();
#ifdef 	XSERVER_LIBPCIACCESS	
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,7,0,0,0)		
	struct pci_device * pd;
	pd = xf86GetPciInfoForEntity(entityIndex);	
	if(pd && !xf86IsEntityPrimary(entityIndex)){
		/* give up vga legency resource decodes for none-primary vga cards 			  
		   Note: pci_device_vgaarb_*** function start be exist since libpciaccess v0.11.0 (ubuntu10.04). for v0.10.* and below (ubuntu9.10),
		   these function will throw you error when startx (but no error occured in compiling)
		/* I cannot get libpciaccess version number, so use X server version is a work around */
		pci_device_vgaarb_set_target(pd);
		pci_device_vgaarb_decodes(0);
		/*	monk:
			Weird, with only above line, it not work.
			it works well together with below line.
			root cause:

			Libpciaccess will not use new decode flag passing to its immediate kernel sys call 
			but use original decode flag, and save new decode flag to its struct member after kernel sys call.
			That's make kernel not sync with the latest request .Maybe it's a bug of libpciaccess
			So I used a tricky method to fool around the libpciacess in order to make kernel vgaarb 
			consider this card already disclaimed for its vga legency resource decodings:

			Call again "pci_device_vgaarb_decodes" with meaningless  decode flag but not the same as last one
			it will make libpciaccess using last saved decode flag (it is 0) as the parameter for kernel  sys call
		 */
		pci_device_vgaarb_decodes(0x10);		
	}else{
		xf86ErrorF("This should never happen!\n");
		LEAVE();
	}
#endif
#endif	
	
#ifdef  XSERVER_LIBPCIACCESS
	/*Enable pci device will open pci config space io/mem bits in theory ( according to kernel source code: pci-sysfs.c )
	Butter... seems 718 and 750 only be enabled for mem bits
	*/
	pci_device_enable(pd);
#else
	/* TODO: how to enable pci device with out libpciaccess ? */
#endif
	LEAVE();
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sm750_entityInit
 *  Description:  init hardware
 * =====================================================================================
 */
void sm750_entityInit(int entIndex,pointer private)
{
    initchip_param_t initParam;
	SMIHWPtr pHw = (SMIHWPtr)private;
	
	ENTER();

    initParam.powerMode = 0;  /* Default to power mode 0 */
	initParam.chipClock = DEFAULT_MEMORY_CLK;
    initParam.memClock = DEFAULT_MEMORY_CLK; /* Default memory clock to 144MHz */
    initParam.masterClock = initParam.chipClock/3; /* Default master clock to half of mem clock */
    initParam.setAllEngOff = 0; /* Using 2D now*/
	initParam.resetMemory = 1;
	
	ddk750_set_mmio(pHw->pReg,pHw->devId,0);
    ddk750_initHw(&initParam);
	
	LEAVE();
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sm750_entityEnter
 *  Description:  save console's registers
 * =====================================================================================
 */
void sm750_entityEnter(int entIndex,pointer private)
{
	ENTER();
	
	sm750_entityInit(entIndex, private);

	LEAVE();
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sm750_entityLeave
 *  Description:  restore console's registers
 * =====================================================================================
 */
void sm750_entityLeave(int entIndex,pointer private)
{
	SMIHWPtr pHw = (SMIHWPtr)private;
	ENTER();
	if(xf86IsPrimaryPci(pHw->pPci))
	{
		/* Restore the registers */
		restore_reg_750((SMIHWPtr)private);
		/* Restore the vga fonts */
		sm750_restoreFonts((SMIHWPtr)private);
	}
	LEAVE();
}
void sm750_saveText(ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR (pScrn);

	ENTER();
	/* This functin called from entity init,
	Of course, it is the primary device */
	if(xf86IsPrimaryPci(pSmi->pHardware->pPci))
	{	
		/* Save the vga fonts */
		sm750_saveFonts(pSmi->pHardware);
		/* Save the registers */
		save_reg_750(pSmi->pHardware);
	}
	LEAVE();
}
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sm750_getMapResult
 *  Description:  calculate the frame buffer for the current screen
 * =====================================================================================
 */
void sm750_getMapResult(ScrnInfoPtr pScrn)
{
	vgaHWPtr hwp;
	SMIPtr pSmi = SMIPTR(pScrn);
	SMIHWPtr pHw = pSmi->pHardware;
	ENTER();

	/* set video memory size */
	pSmi->videoRAMBytes = pHw->physize_mem / (pSmi->pHardware->dual);
	pScrn->videoRam = (pSmi->videoRAMBytes)>>10;/* kbytes*/

	/* set pScrn physica address*/
	pScrn->memPhysBase = pHw->phyaddr_mem;
	pScrn->memPhysBase += (pSmi->screen) * pSmi->videoRAMBytes;
    xf86Msg(X_INFO, "ilena, screen[%d]\n", pSmi->screen);
	/* set OFFSET */
	pScrn->fbOffset = pSmi->FBOffset = pScrn->memPhysBase - pHw->phyaddr_mem;

	/* set virtual address */
	pSmi->pFB = pHw->pMem;
	pSmi->pFB += (pSmi->screen) * pSmi->videoRAMBytes;

	pSmi->FBReserved = pSmi->videoRAMBytes - 4096;

	if (!pSmi->width)
		pSmi->width = pScrn->displayWidth;
	if (!pSmi->height)
		pSmi->height = pScrn->virtualY;
	
	pSmi->Bpp = pScrn->bitsPerPixel / 8;
	pSmi->Stride = (pSmi->width * pSmi->Bpp + 15) & ~15;

	LEAVE();
}

void sm750_handleOptions(ScrnInfoPtr pScrn)
{
	ENTER();
        if (pScrn->depth == 8)
        {
        	pScrn->rgbBits = 8;
        }
	LEAVE();
}

ModeStatus sm750_validMode(ScrnInfoPtr pScrn,DisplayModePtr pMode)
{
	ENTER();
	LEAVE(MODE_OK);
}

void sm750_setMode(ScrnInfoPtr pScrn,DisplayModePtr pMode)
{
	SMIPtr pSmi = SMIPTR (pScrn);
	SMIHWPtr pHw = pSmi->pHardware;
	I2CBusPtr bus;
	
	ENTER();
	
	mode_parameter_t parm;
	clock_type_t clock;
	
    if(pSmi->pci_burst && (pSmi->pHardware->pPci == SMI_718)) 
		enable_pci_burst_718(pHw);

#if SMI_RANDR
	if (pSmi->dispCtrl == PANEL)
		clock = PRIMARY_PLL;
	else if (pSmi->dispCtrl == CRT)
		clock = SECONDARY_PLL;
#else

	if (pSmi->screen == 0)
		clock = PRIMARY_PLL;
	else if (pSmi->screen == 1)
		clock = SECONDARY_PLL;
#endif
	/* Horizontal timing. */
    parm.horizontal_total = pMode->HTotal;
    parm.horizontal_display_end = pMode->HDisplay;
    parm.horizontal_sync_start = pMode->HSyncStart;
    parm.horizontal_sync_width = pMode->HSyncEnd - pMode->HSyncStart;
    parm.horizontal_sync_polarity = (pMode->Flags & V_PHSYNC)?POS:NEG;

    /* Vertical timing. */
    parm.vertical_total = pMode->VTotal;
    parm.vertical_display_end = pMode->VDisplay;
    parm.vertical_sync_start = pMode->VSyncStart;
    parm.vertical_sync_height = pMode->VSyncEnd - pMode->VSyncStart;;
    parm.vertical_sync_polarity = (pMode->Flags & V_PVSYNC)?POS:NEG;

    /* Refresh timing. */
    parm.pixel_clock = pMode->Clock*1000; 
    parm.horizontal_frequency = roundDiv(parm.pixel_clock,
                                        parm.horizontal_total);
    parm.vertical_frequency = roundDiv(parm.horizontal_frequency,
                                      parm.vertical_total);
    
    /* Clock Phase. This clock phase only applies to Panel. 
    parm.clock_phase_polarity = pMode->;*/
    xf86Msg(X_INFO,"Set Mode H: %x; V: %x; clock: %x\n",
    parm.horizontal_total, parm.vertical_total, clock);
    
    ddk750_set_mmio(pHw->pReg,pHw->devId,0);
	
    ddk750_setModeTiming(&parm, clock);
	
    
	
    LEAVE();
}

void sm750_setDisplay(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
	SMIPtr pSmi = SMIPTR (pScrn);
	SMIHWPtr pHw = pSmi->pHardware;
	int bpp = pScrn->bitsPerPixel;
	int pitch;
	
	ENTER();

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
     * However, 1366 resolution or 1368, for example, has to be adjusted for 16 pixel aligned.
     */
    /* "+ 15) & ~15)" This calculation has no effect on 640, 800, 1024 and 1280. */
	pitch = (pSmi->width * (bpp / 8) + 15) & ~15;	/* when bpp=16 or 32,1368 * (bpp/8) is 16 pixel aligned*/
	xf86Msg(X_INFO,"sm750:set Display:pitch: %x\n", pitch);
	
	set_display_750(pHw, channel, bpp, pitch, pSmi->FBOffset, mode->HDisplay, mode->HTotal);
	LEAVE();
}

void sm750_adjustFrame(ScrnInfoPtr pScrn,int offset)
{
	ENTER();
	LEAVE();
}

void sm750_closeAllScreen(SMIHWPtr pHw)
{
	ENTER();
	SMI750HWPtr	p750Hw;
	p750Hw = (SMI750HWPtr)pHw;

	if(p750Hw->pRegSave)
	{
		xf86Msg(X_INFO,"Close Screen Free saved reg\n");
		xfree(p750Hw->pRegSave);
		p750Hw->pRegSave = NULL;
	}

	if(p750Hw->fonts)
	{
		xf86Msg(X_INFO,"Close Screen Free saved fonts\n");
		xfree(p750Hw->fonts);
		p750Hw->fonts = NULL;
	}
	xfree(p750Hw);
	LEAVE();
}

/* Save vga fonts */
static void sm750_saveFonts(SMIHWPtr pHw)
{	
	SMI750HWPtr	p750Hw = (SMI750HWPtr)pHw;

	ENTER();
	
	/* leave if in graphic mode */
	if(get_vga_mode_750(pHw) & 0x2)
		LEAVE();

	memcpy((char *)p750Hw->fonts,(char *)pHw->pMem + KB(0),KB(64));
	
	/* not use legency method to access vga fonts , or multi-card will cry */

 	LEAVE();
}

static void sm750_restoreFonts(SMIHWPtr pHw)
{
	SMI750HWPtr	p750Hw = (SMI750HWPtr)pHw;

	ENTER();
	
	if(p750Hw->fonts)
		memcpy((char *)pHw->pMem + KB(0),(char *)p750Hw->fonts,KB(64));
	
	/* not use legency method or multi-card cries */

	LEAVE();
}

int sm750_totalFB(SMIHWPtr pHw)
{
	unsigned int total_memory;
	ENTER();
	
	ddk750_set_mmio(pHw->pReg,pHw->devId,0);
	total_memory = ddk750_getVMSize();
	
	LEAVE(total_memory);
}

void sm750_loadPalette (ScrnInfoPtr pScrn, int numColors, 
        int *indicies, LOCO * colors, VisualPtr pVisual)
{
	ENTER();
#if SMI_RANDR

    	xf86CrtcConfigPtr crtcConf = XF86_CRTC_CONFIG_PTR(pScrn);
    	int crtc_idx,i,j;

   	 if(pScrn->bitsPerPixel == 16){
        /* Expand the RGB 565 palette into the 256-elements LUT */

        for(crtc_idx=0; crtc_idx<crtcConf->num_crtc; crtc_idx++){
            SMICrtcPrivatePtr crtcPriv = SMICRTC(crtcConf->crtc[crtc_idx]);

            for(i=0; i<numColors; i++){
                int idx = indicies[i];

                if(idx<32){
                    for(j=0; j<8; j++){
                        crtcPriv->lut_r[idx*8 + j] = colors[idx].red << 8;
                        crtcPriv->lut_b[idx*8 + j] = colors[idx].blue << 8;
                    }
                }

                for(j=0; j<4; j++)
                    crtcPriv->lut_g[idx*4 + j] = colors[idx].green << 8;
            }

            crtcPriv->load_lut(crtcConf->crtc[crtc_idx]);
        }
    }else{
        for(crtc_idx=0; crtc_idx<crtcConf->num_crtc; crtc_idx++){
            SMICrtcPrivatePtr crtcPriv = SMICRTC(crtcConf->crtc[crtc_idx]);

            for(i = 0; i < numColors; i++) {
                int idx = indicies[i];

                crtcPriv->lut_r[idx] = colors[idx].red << 8;
                crtcPriv->lut_g[idx] = colors[idx].green << 8;
                crtcPriv->lut_b[idx] = colors[idx].blue << 8;
            }

            crtcPriv->load_lut(crtcConf->crtc[crtc_idx]);
        }
    }

#endif       
        LEAVE();
}

#if SMI_RANDR
void sm750_I2CInit(ScrnInfoPtr pScrn)
{    
	SMIPtr pSmi = SMIPTR(pScrn);
	SMIHWPtr pHw = pSmi->pHardware;
	int cnt = pSmi->DualView?2:1;
	int index;
	I2CBusPtr ptr[2] = {NULL,NULL};
	static char * sm750_name[] = {"I2C Bus PanelPath","I2C Bus CrtPath"};	
	ENTER();

	pfn_I2CPutBits_750[0] = i2c_putbits_panel_750;
	pfn_I2CPutBits_750[1] = i2c_putbits_crt_750;
	pfn_I2CGetBits_750[0] = i2c_getbits_panel_750;
	pfn_I2CGetBits_750[1] = i2c_getbits_crt_750;

	index= 0 ;        
	while(index < cnt)
	{       
	    if(ptr[index] == NULL )
	    {
	    	I2CBusPtr I2CPtr = xf86CreateI2CBusRec();
	    	if (I2CPtr == NULL)
	    	    return FALSE;

			I2CPtr->scrnIndex  = pScrn->scrnIndex;
			I2CPtr->BusName  = sm750_name[index];
			I2CPtr->I2CPutBits = pfn_I2CPutBits_750[index];
			I2CPtr->I2CGetBits = pfn_I2CGetBits_750[index];				
			
			        	
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

	init_i2c_750(pHw);

	LEAVE(TRUE);
}	
#endif

void sm750_VideoReset(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    	SMIHWPtr pHw = pSmi->pHardware;
    SMI_PortPtr pPort = (SMI_PortPtr) pSmi->ptrAdaptor->pPortPrivates[0].ptr;
    int r, g, b;

	ENTER();

    switch (pScrn->depth)
    {
        case 8:
            WRITE_DCR(pHw, DCR08, pPort->Attribute[XV_COLORKEY] & 0x00FF);
            break;
        case 15:
        case 16:
            WRITE_DCR(pHw, DCR08, pPort->Attribute[XV_COLORKEY] & 0xFFFF);
            break;
        default:
            r = (pPort->Attribute[XV_COLORKEY] & pScrn->mask.red) >> pScrn->offset.red;
            g = (pPort->Attribute[XV_COLORKEY] & pScrn->mask.green) >> pScrn->offset.green;
            b = (pPort->Attribute[XV_COLORKEY] & pScrn->mask.blue) >> pScrn->offset.blue;
            WRITE_DCR(pHw, DCR08, ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
            break;
    }

	LEAVE();
}