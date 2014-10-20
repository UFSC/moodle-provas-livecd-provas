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
#include	"smi_750le_driver.h"
#include 	"smi_750le_hw.h"
#include	"../smi_dbg.h"
#if SMI_RANDR
void (*pfn_I2CPutBits_750le)(I2CBusPtr,int,int) = NULL;
void (*pfn_I2CGetBits_750le)(I2CBusPtr,int*,int*) = NULL;
#endif

mode_parameter_t mode_table_750le[] = {
    /*----------------------------------------------------------------------------------------
     * H.	H.    H.     H.   H.        V.   V.    V.    V.   V.        Pixel     H.     V.
     * tot.	disp. sync   sync sync      tot. disp. sync  sync sinc      clock     freq.  freq.
     *      end   start  wdth polarity       end   start hght polarity
     *---------------------------------------------------------------------------------------*/
    /* 800 x 600 */
    {1056, 800, 840, 128, POSITIVE, 628, 600, 601, 4, POSITIVE,
        40000000, 37879, 60,  POSITIVE},
    /* 1024 x 768 */
    {1344, 1024, 1048, 136, NEGATIVE, 806, 768, 771, 6, NEGATIVE,
        65000000, 48363, 60,  NEGATIVE},
    /* 1280 x 960 */
    {1800, 1280, 1376, 112, POSITIVE, 1000, 960, 961, 3, POSITIVE,
        108000000, 60000, 60, NEGATIVE},
    /* 1280 x 1024 */
    /* without "negative" the tail, 1280x1024 mode will show inverted color */
    {1712, 1280, 1360, 136, NEGATIVE, 1060, 1024, 1025, 3, POSITIVE,
        108883200, 63600, 60, NEGATIVE}, 

    /* 1280 x 768 */
    {1678, 1280, 1350, 136, POSITIVE, 795, 768, 769, 3, POSITIVE,
        80000000, 47676, 60,  NEGATIVE},
    /* 1280 x 720 */
    {1664, 1280, 1336, 136, POSITIVE, 746, 720, 721, 3, POSITIVE,
        74481000, 44760, 60, NEGATIVE},
    /* 1152 x 864 */
    {1475, 1152, 1208, 96, POSITIVE, 888, 864, 866, 3, POSITIVE,
        78600000, 53288, 60,  NEGATIVE},
    /* End of table. */
    {0, 0, 0, 0, NEGATIVE, 0, 0, 0, 0, NEGATIVE, 0, 0, 0,  NEGATIVE},
};

/* Global Vars */

static const SMI750leHWRec sm750le_hwrec = {
	.base = {
		.pcDeepMap = sm750le_pcDeepmap,/* actually,NULL value can be just ignored,gcc will know it*/
		.pcEntityInit = sm750le_entityInit,
		.pcEntityEnter = sm750le_entityEnter,
		.pcEntityLeave = sm750le_entityLeave,	
		.pcCloseAllScreen = sm750le_closeAllScreen,
		.pcFBSize = sm750le_totalFB,
		.pcInitHardware = NULL,
	},

};

static const SMI750leRec sm750le_rec = {
	.base = {
		.minHeight = 128,
		.psGetMapResult = sm750le_getMapResult,
		.psVgaPciInit = sm750le_vgaAndpciInit,
		.psHandleOptions = sm750le_handleOptions,
		.psValidMode = sm750le_validMode,
		.psSetMode = sm750le_setMode,
		.psAdjustFrame = sm750le_adjustFrame,	
		.psLoadPalette = sm750le_LoadPalette,
		.psSetDisplay = sm750le_setDisplay,
		.psSaveText = sm750le_saveText,
	#if SMI_RANDR
		.psCrtcPreInit = SMI750le_CrtcPreInit,
		.psOutputPreInit = SMI750le_OutputPreInit,
		.psI2CInit = sm750le_I2CInit,
	#endif	
		},
};

SMIPtr sm750le_genSMI(pointer priv,int entityIndex)
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

    pSmi = (SMIPtr)XNFcalloc(sizeof(SMI750leRec));
    memset(pSmi,0, sizeof(SMI750leRec));
	if(pSmi == NULL)
		return pSmi;

	memcpy(pSmi,&sm750le_rec,sizeof(SMI750leRec));
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
        SMI750leHWPtr p750leHw;
		p750leHw = pSmi->pHardware = (SMIHWPtr)XNFcalloc(sizeof(SMI750leHWRec));
		if(pSmi->pHardware == NULL)
			LEAVE(NULL);
		memcpy(pSmi->pHardware,&sm750le_hwrec,sizeof(SMI750leHWRec));

        p750leHw->pRegSave = xnfcalloc(sizeof(SMI750leRegRec),1);
		if(p750leHw->pRegSave == NULL)
			LEAVE(NULL);
#if 1
		p750leHw->fonts = xalloc(KB(64));
		if(p750leHw->fonts == NULL)
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
		pSmi->pHardware->devId = pPci->device_id;
	#else
		pSmi->pHardware->phyaddr_reg = pPci->memBase[1];
		pSmi->pHardware->physize_reg = 0x200000;
		pSmi->pHardware->phyaddr_mem = pPci->memBase[0];
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
		//pSmi->pHardware->dual += 1;/*The total number of screen*/
		//pSmi->screen = (pSmi->pHardware->dual)-1;/*The index of screen*/
                                    pSmi->pHardware->dual = 1;
                                    pSmi->screen = 0 ;
	}
	LEAVE(pSmi);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sm750le_pcDeepmap
 *  Description:  
 * =====================================================================================
 */
void sm750le_pcDeepmap(SMIHWPtr pHw)
{
	ENTER();
	SMI750leHWPtr p750leHw = (SMI750leHWPtr)pHw;
	SMIPtr pSmi = HWPSMI(pHw);
	ScrnInfoPtr pScrn = SMIPSI(pSmi);

	ddk750_set_mmio(pHw->pReg,pHw->devId, pHw->revId);

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

void sm750le_vgaAndpciInit(ScrnInfoPtr pScrn,int entityIndex)
{
        ENTER();
    struct pci_device * pd;
    EntityInfoPtr pEnt;	
    //ilena: sth have been moved to entityinit_common.
    pd = xf86GetPciInfoForEntity(entityIndex);
    pEnt = xf86GetEntityInfo (entityIndex);	
#ifdef 	XSERVER_LIBPCIACCESS	
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,7,0,0,0)		
    if(pd && !xf86IsEntityPrimary(entityIndex))
	{
            xf86ErrorF("not a primary vga device !\n");
        }
	else
	{
        xf86ErrorF("sm750le is the primary vga device!\n");
    }

#endif
    pci_device_enable(pd);	
#endif
        xfree(pEnt);
        LEAVE();

}

void sm750le_entityInit(int entityIndex,pointer private)
{	
    ENTER();
   
    initchip_param_t initParam;

    initParam.powerMode = 0;  /* Default to power mode 0 */
	initParam.chipClock = DEFAULT_SM750LE_CHIP_CLOCK;
    initParam.memClock = DEFAULT_SM750LE_CHIP_CLOCK; /* Default memory clock to 144MHz */
    initParam.masterClock = initParam.chipClock/3; /* Default master clock to half of mem clock */
    initParam.setAllEngOff = 0;
	initParam.resetMemory = 0;

     ddk750_initHw(&initParam);
    LEAVE();
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sm750le_entityEnter
 *  Description:  save console's registers
 * =====================================================================================
 */
void sm750le_entityEnter(int entIndex,pointer private)
{
    ENTER();
    //sm750le_entityInit(entIndex, private);

#if 1
        initchip_param_t initParam;

    initParam.powerMode = 0;  /* Default to power mode 0 */
    initParam.chipClock = DEFAULT_SM750LE_CHIP_CLOCK;
    initParam.memClock = DEFAULT_SM750LE_CHIP_CLOCK; /* Default memory clock to 144MHz */
    initParam.masterClock = initParam.chipClock/3; /* Default master clock to half of mem clock */
    initParam.setAllEngOff = 0;
    initParam.resetMemory = 0;

     ddk750_initHw(&initParam);
 #endif
    LEAVE();
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  sm750le_entityLeave
 *  Description:  restore console's registers
 * =====================================================================================
 */
void sm750le_entityLeave(int entIndex,pointer private)
{
	ENTER();
	
	/* Restore the registers */
	restore_reg_750le((SMIHWPtr)private);
	
	/* Restore the vga fonts */
	sm750le_restoreFonts((SMIHWPtr)private);
	
	LEAVE();
}

void sm750le_saveText(ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR (pScrn);

	ENTER();
	/* This functin called from entity init,
	Of course, it is the primary screen */
	if(xf86IsPrimaryPci(pSmi->pHardware->pPci))
	{	
		/* Save the vga fonts */
		sm750le_saveFonts(pSmi->pHardware);
		/* Save the registers */
		save_reg_750le(pSmi->pHardware);
	}
	LEAVE();
}

/* Save vga fonts */
static void sm750le_saveFonts(SMIHWPtr pHw)
{	
	SMI750leHWPtr	p750leHw = (SMI750leHWPtr)pHw;
	ENTER();

	memcpy((char *)p750leHw->fonts,(char *)pHw->pMem + KB(0),KB(64));

	/* not use legency method to access vga fonts , or multi-card will cry */
 	LEAVE();
}

static void sm750le_restoreFonts(SMIHWPtr pHw)
{
	SMI750leHWPtr	p750leHw = (SMI750leHWPtr)pHw;
	ENTER();
	
	if(p750leHw->fonts)
		memcpy((char *)pHw->pMem + KB(0),(char *)p750leHw->fonts,KB(64));
	
	/* not use legency method or multi-card cries */
	LEAVE();
}

void sm750le_getMapResult(ScrnInfoPtr pScrn)
{
	vgaHWPtr hwp;
   	SMIPtr pSmi = SMIPTR(pScrn);
                SMI750lePtr p750leRec = (SMI750lePtr)pSmi;
	SMIHWPtr pHw = pSmi->pHardware;
	ENTER();
        //    pRegPtr->mmio_require++;

	/* set video memory size */
	pSmi->videoRAMBytes = pHw->physize_mem / (pSmi->pHardware->dual);
	pScrn->videoRam = (pSmi->videoRAMBytes)>>10;/* kbytes*/

	/* set pScrn physica address*/
#ifndef	XSERVER_LIBPCIACCESS	
    pScrn->memPhysBase = pHw->pPci->memBase[0];
#else
    pScrn->memPhysBase = PCI_REGION_BASE(pHw->pPci, 0, REGION_MEM); 
#endif

	/* set OFFSET */
	pScrn->fbOffset = pSmi->FBOffset = pScrn->memPhysBase - pHw->phyaddr_mem;

	/* set virtual address */
	pSmi->pFB = pHw->pMem;//?
	pSmi->pFB += (pSmi->screen) * pSmi->videoRAMBytes;//?

	pSmi->FBReserved = pSmi->videoRAMBytes - 4096;

	//ilena:seems moved from modeInit
	pSmi->Bpp = pScrn->bitsPerPixel / 8;
	pSmi->width = pScrn->virtualX;
	pSmi->height = pScrn->virtualY;
	pSmi->Stride = (pSmi->width * pSmi->Bpp + 15) & ~15;
#if 1
	if (pSmi->screen){
	    vgaHWGetHWRec(pScrn);
	}
    hwp = VGAHWPTR (pScrn);

#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,12,0,0,0)    
    p750leRec->PIOBase= hwp->PIOOffset;
#else
    p750leRec->PIOBase = 0;//X donot use the PIOOffset anymore.
#endif
    /* vga fun was not init when no ioport platform */
    /* vgaHWGetIOBase(hwp); */ 
    /* Map the VGA memory when the primary video */
    if (xf86IsPrimaryPci (pHw->pPci))
    {
        hwp->MapSize = 0x10000;
        if (!vgaHWMapMem (pScrn))
        {
            LEAVE(FALSE);
        }
    }
#endif

    LEAVE(TRUE);

}

void sm750le_handleOptions(ScrnInfoPtr pScrn)
{
	ENTER();
        if (pScrn->depth == 8)
        {
        	pScrn->rgbBits = 8;
        }
	LEAVE();
}

ModeStatus sm750le_validMode(ScrnInfoPtr pScrn,DisplayModePtr mode)
{
	ENTER();
    float refresh;
    int tmpRfrsh;
    mode_parameter_t * findMode;

    refresh = (mode->VRefresh > 0) ? mode->VRefresh
        : mode->Clock * 1000.0 / mode->VTotal / mode->HTotal;
    xf86Msg ( X_INFO, " Mode: %dx%d %d-bpp, %fHz\n",
            mode->HDisplay, mode->VDisplay, pScrn->bitsPerPixel, refresh);
    tmpRfrsh = (int)(refresh + 0.5);

    if(tmpRfrsh != 60) 
	{
		/* monk:change range withen 59 and 61 or we will failed in startx*/
		XERR("only support 60hz\n");
        LEAVE (MODE_HSYNC);
    }

    findMode = sm750le_findMode (mode_table_750le,mode->HDisplay,mode->VDisplay,60);
    if (findMode == NULL)	
    {
		XERR("mod not supported X=%x, Y=%x\n", mode->HDisplay, mode->VDisplay);
        LEAVE (MODE_NOMODE);
    }

    xf86Msg ( X_INFO, " Mode: %dx%d %d-bpp, %fHz supported!\n",
            mode->HDisplay, mode->VDisplay, pScrn->bitsPerPixel, refresh);
    LEAVE (MODE_OK);

}


void sm750le_setMode(ScrnInfoPtr pScrn, DisplayModePtr pMode)
{
    ENTER();
    SMIPtr pSmi = SMIPTR(pScrn);
    SMI750lePtr p750Rec = (SMI750lePtr)pSmi;
    mode_parameter_t parm;
    mode_parameter_t * vesaMode;
    clock_type_t clock;

    clock = SECONDARY_PLL;
    save_reg_vga_750le(pScrn);

    vesaMode = sm750le_findMode (mode_table_750le, pMode->HDisplay, pMode->VDisplay,60);
    if (vesaMode != NULL)
    {
            sm750le_adjustMode (vesaMode, &parm);
    }
    else{
        LEAVE(FALSE);
    }

    outb(p750Rec->PIOBase + 0x3D4, 0x88);
    outb(p750Rec->PIOBase + 0x3D5, 0x06);
    outb(p750Rec->PIOBase + 0x3d4,0x88);
    char tmp = inb(p750Rec->PIOBase + 0x3d5);
    DEBUG("monk: 3d4 88 is %02x\n",tmp);
    ddk750_setModeTiming( &parm, clock);

    LEAVE(TRUE);
}

void sm750le_adjustMode (mode_parameter_t * vesaMode, mode_parameter_t * mode)
{
    unsigned long blank_width, sync_start, sync_width;
    *mode = *vesaMode;
    if ((vesaMode->horizontal_sync_start - vesaMode->horizontal_display_end) 
            > 24) {
        return;
    }
    blank_width = vesaMode->horizontal_total - vesaMode->horizontal_display_end;
    sync_start = roundDiv((vesaMode->horizontal_sync_start - 
        vesaMode->horizontal_display_end) * 100, blank_width);
    sync_width = roundDiv(vesaMode->horizontal_sync_width * 100, blank_width);
    mode->horizontal_total = roundDiv(mode->pixel_clock, 
            vesaMode->horizontal_frequency);
    blank_width = mode->horizontal_total - mode->horizontal_display_end;
    mode->horizontal_sync_start = mode->horizontal_display_end +
        roundDiv(blank_width * sync_start, 100);
    mode->horizontal_sync_width = roundDiv(mode->horizontal_frequency, 
            mode->vertical_total);
    return;
}

void sm750le_setDisplay(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
    ENTER();

    SMIPtr pSmi = SMIPTR(pScrn);
    CARD32 Base;
    int pitch;
    int bpp = pScrn->bitsPerPixel;
    int x = pScrn->frameX0;
    int y = pScrn->frameY0;

    pitch = ((pSmi->width+ 15) & ~15) * (bpp / 8);
    xf86DrvMsg("", X_INFO, "pSmi->FBOffset is 0x%x, x is %d, y is %d\n", 
            pSmi->FBOffset, x, y);
    Base = pSmi->FBOffset + (x + y * pScrn->displayWidth) * pSmi->Bpp;

    set_display_750le( bpp, pitch, Base, mode->HDisplay, mode->VDisplay);

    LEAVE();
}

void sm750le_adjustFrame(ScrnInfoPtr pScrn,int offset)
{
	ENTER();
	LEAVE();
}

void sm750le_closeAllScreen(SMIHWPtr pHw)
{

    ENTER();
    SMI750leHWPtr p750leHw = (SMI750leHWPtr)pHw;

    if(p750leHw->pRegSave)
    {
        xf86Msg(X_INFO,"Close Screen Free saved reg\n");
        xfree(p750leHw->pRegSave);
        p750leHw->pRegSave = NULL;
    }

    xfree(p750leHw);

    LEAVE();
}

int sm750le_totalFB(SMIHWPtr pHw)
{
	unsigned int total_memory;
	ENTER();

	total_memory = ddk750_getVMSize();
	
	LEAVE(total_memory);
}
extern SMI750LE_ModeInit(ScrnInfoPtr pScn,DisplayModePtr mode);
Bool SMI750LE_EnterVT (int scrnIndex, int flags)
{
    ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
    SMIPtr pSmi = SMIPTR (pScrn);
    Bool ret;
    ENTER();

    //SMI750LE_Restore(pScrn);
    swI2CInit(0,1);

	/* save routine called at preInit,so no need save anymore*/

    ret = SMI750LE_ModeInit (pScrn, pScrn->currentMode); 

    if (!pSmi->NoAccel){	
#if 1
	/* using 750le_deInit will make 750le die,root cause unknown */
	//    SMI750LE_EngineReset (pScrn);//delete tmp by ilena
	    POKE32(DE_MASKS, 0xFFFFFFFF);
	    POKE32( DE_STRETCH_FORMAT,
			    FIELD_SET(0, DE_STRETCH_FORMAT, PATTERN_XY, NORMAL) |
			    FIELD_VALUE(0, DE_STRETCH_FORMAT, PATTERN_Y, 0) |
			    FIELD_VALUE(0, DE_STRETCH_FORMAT, PATTERN_X, 0) |
			    FIELD_SET(0, DE_STRETCH_FORMAT, ADDRESSING, XY) |
			    (pScrn->bitsPerPixel == 8 ?
			     FIELD_SET(0, DE_STRETCH_FORMAT, PIXEL_FORMAT, 8) :
			     FIELD_SET(0, DE_STRETCH_FORMAT, PIXEL_FORMAT, 16)) |
			    FIELD_VALUE(0, DE_STRETCH_FORMAT, SOURCE_HEIGHT, 3));
#else
	    SMI750LE_deInit(pScrn); 
#endif
    }

    LEAVE(ret);
}

mode_parameter_t * sm750le_findMode (mode_parameter_t * mode_table, INT width, 
        INT height, INT refresh_rate)
{
    /* Walk the entire mode table. */
    while (mode_table->pixel_clock != 0)
    {
        /* If this mode matches the requested mode, return it! */
        if ((mode_table->horizontal_display_end == width)
                && (mode_table->vertical_display_end == height)
                && (mode_table->vertical_frequency == refresh_rate))
        {
            return (mode_table);
        }

        /* Next entry in the mode table. */
        mode_table++;
    }

    /* No mode found. */
    return (NULL);
}

void sm750le_LoadPalette (ScrnInfoPtr pScrn, int numColors, 
        int *indicies, LOCO * colors, VisualPtr pVisual)
{
        ENTER();
    SMIPtr pSmi = SMIPTR (pScrn);
    int i;
    int iRGB;

    for (i = 0; i < numColors; i++)
    {
#if 0        
        DEBUG ("pal[%d] = %d %d %d\n", indicies[i],
                colors[indicies[i]].red, colors[indicies[i]].green,
                colors[indicies[i]].blue);
#endif
        iRGB = (colors[indicies[i]].red << 16) |
            (colors[indicies[i]].green << 8) | (colors[indicies[i]].blue);

            /* CRT Palette */
        POKE32( SECONDARY_PALETTE_RAM + (4 * indicies[i]),
                iRGB);
    }
    LEAVE();
}
#if SMI_RANDR
void sm750le_I2CInit(ScrnInfoPtr pScrn)
{
	SMIPtr pSmi = SMIPTR(pScrn);
	SMIHWPtr pHw = pSmi->pHardware;
	I2CBusPtr ptr = NULL;
	static char * sm750le_name = "I2C Bus PanelPath";	
	ENTER();

	pfn_I2CPutBits_750le = i2c_putbits_panel_750le;
	pfn_I2CGetBits_750le = i2c_getbits_panel_750le;

	    if(ptr == NULL )
	    {
	    	I2CBusPtr I2CPtr = xf86CreateI2CBusRec();
	    	if (I2CPtr == NULL)
	    	    return FALSE;

			I2CPtr->scrnIndex  = pScrn->scrnIndex;
			I2CPtr->BusName  = sm750le_name;
			I2CPtr->I2CPutBits = pfn_I2CPutBits_750le;
			I2CPtr->I2CGetBits = pfn_I2CGetBits_750le;				
			
			        	
	    	I2CPtr->DriverPrivate.ptr = (void *)xalloc(sizeof(int));

	    	if (!xf86I2CBusInit(I2CPtr)){				
	    	    xfree(I2CPtr->DriverPrivate.ptr);
	    	    xf86DestroyI2CBusRec(I2CPtr, TRUE, TRUE);
	    	    LEAVE(FALSE);
	    	}        		 
	    	ptr = I2CPtr;
	    	*((int *)I2CPtr->DriverPrivate.ptr) = 0;
	    }

	pSmi->I2C_primary = ptr;

	LEAVE(TRUE);
}
#endif
/*
Bool
SMI750le_OutputPreInit(ScrnInfoPtr pScrn)
{
	ENTER();
	LEAVE();
}
*/
