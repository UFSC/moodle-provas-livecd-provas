/*
Copyright (C) 1994-1999 The XFree86 Project, Inc.  All Rights Reserved.
Copyright (C) 2000 Silicon Motion, Inc.  All Rights Reserved. 
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

Except as contained in this notice, the names of the XFree86 Project and
Silicon Motion shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization from the XFree86 Project and silicon Motion.
*/

#include "miline.h"
#include "xaalocal.h"
#include "xaarop.h"
#include "servermd.h"
#include "smi_common.h"
#include "smi_accel.h"
#include "smi_dbg.h"
#include "drv750le/smi_750le_driver.h"
extern int entity_priv_index[MAX_ENTITIES];

void wait_for_not_busy_502(SMIPtr);
void wait_for_not_busy_712(SMIPtr);
void wait_for_not_busy_750(SMIPtr);

static void SMI_RestoreAccelState (ScrnInfoPtr pScrn);
static void SMI_SetupForScreenToScreenCopy (ScrnInfoPtr, int, int, int, unsigned int, int);
static void SMI_SubsequentScreenToScreenCopy (ScrnInfoPtr, int, int, int, int, int, int);
static void SMI_SetupForSolidFill (ScrnInfoPtr, int, int, unsigned);
static void SMI_SubsequentSolidFillRect (ScrnInfoPtr, int, int, int, int);
static void SMI_SubsequentSolidHorVertLine (ScrnInfoPtr, int, int, int, int);
static void SMI_SetupForCPUToScreenColorExpandFill (ScrnInfoPtr, int, int, int, unsigned int);
static void SMI_SubsequentCPUToScreenColorExpandFill (ScrnInfoPtr, int, int, int, int, int);
static void SMI_SetupForMono8x8PatternFill (ScrnInfoPtr, int, int, int, int, int, unsigned int);
static void SMI_SubsequentMono8x8PatternFillRect (ScrnInfoPtr, int, int, int, int, int, int);
static void SMI_SetupForColor8x8PatternFill (ScrnInfoPtr, int, int, int, unsigned int, int);
static void SMI_SubsequentColor8x8PatternFillRect (ScrnInfoPtr, int, int, int, int, int, int);
#if SMI_USE_IMAGE_WRITES
	static void SMI_SetupForImageWrite (ScrnInfoPtr, int, unsigned int, int, int, int);
	static void SMI_SubsequentImageWriteRect (ScrnInfoPtr, int, int, int, int, int);
#endif
static void SMI_SetClippingRectangle (ScrnInfoPtr, int, int, int, int);
static void SMI_DisableClipping (ScrnInfoPtr);
static void SMI_ValidatePolylines (GCPtr, unsigned long, DrawablePtr);

static void SMI_Polylines (DrawablePtr, GCPtr, int, int, DDXPointPtr);

Bool SMI_AccelInit (ScreenPtr pScreen)
{
  return;
  #if 1
    XAAInfoRecPtr infoPtr;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SMIPtr pSmi = SMIPTR (pScrn);
    /*BoxRec AvailFBArea; */
    Bool ret;
    /*int numLines, maxLines; */

	ENTER();

 //   pSmi->AccelInfoRec = infoPtr = XAACreateInfoRec ();
    if (infoPtr == NULL)
    {
        LEAVE(FALSE);
    }

    infoPtr->Flags = PIXMAP_CACHE | LINEAR_FRAMEBUFFER | OFFSCREEN_PIXMAPS;

    infoPtr->Sync = SMI_AccelSync;

    /*boyod */

    if (xf86IsEntityShared (pScrn->entityList[0]))
    {
        /*if there are more than one devices sharing this entity, we
          have to assign this call back, otherwise the XAA will be
          disabled */
        if (pSmi->pHardware->dual > 1)
            infoPtr->RestoreAccelState = SMI_RestoreAccelState;
    }
    /*boyod*/


    /* Screen to screen copies */

    infoPtr->ScreenToScreenCopyFlags = NO_PLANEMASK
        | ONLY_TWO_BITBLT_DIRECTIONS;
    infoPtr->SetupForScreenToScreenCopy = SMI_SetupForScreenToScreenCopy;
    infoPtr->SubsequentScreenToScreenCopy = SMI_SubsequentScreenToScreenCopy;
    if (pScrn->bitsPerPixel == 24)
    {
        infoPtr->ScreenToScreenCopyFlags |= NO_TRANSPARENCY;
    }

    /* Solid Fills */
    infoPtr->SolidFillFlags = NO_PLANEMASK;
    infoPtr->SetupForSolidFill = SMI_SetupForSolidFill;
    infoPtr->SubsequentSolidFillRect = SMI_SubsequentSolidFillRect;

    /* Solid Lines */
    infoPtr->SolidLineFlags = NO_PLANEMASK;
    infoPtr->SetupForSolidLine = SMI_SetupForSolidFill;
    infoPtr->SubsequentSolidHorVertLine = SMI_SubsequentSolidHorVertLine;

    /* Color Expansion Fills */
    infoPtr->CPUToScreenColorExpandFillFlags = ROP_NEEDS_SOURCE
        | NO_PLANEMASK
#if X_BYTE_ORDER == BIG_ENDIAN
		| BIT_ORDER_IN_BYTE_LSBFIRST
#else
        | BIT_ORDER_IN_BYTE_MSBFIRST
#endif
        | LEFT_EDGE_CLIPPING | CPU_TRANSFER_PAD_DWORD | SCANLINE_PAD_DWORD;
    infoPtr->ColorExpandBase = pSmi->pHardware->DataPortBase;
    infoPtr->ColorExpandRange = pSmi->pHardware->DataPortSize;

    infoPtr->SetupForCPUToScreenColorExpandFill =
        SMI_SetupForCPUToScreenColorExpandFill;
    infoPtr->SubsequentCPUToScreenColorExpandFill =
        SMI_SubsequentCPUToScreenColorExpandFill;

    /* 8x8 Mono Pattern Fills */
    infoPtr->Mono8x8PatternFillFlags = NO_PLANEMASK |
		HARDWARE_PATTERN_PROGRAMMED_BITS | 
		HARDWARE_PATTERN_SCREEN_ORIGIN	|
#if X_BYTE_ORDER == BIG_ENDIAN
		BIT_ORDER_IN_BYTE_LSBFIRST |
#else
		BIT_ORDER_IN_BYTE_MSBFIRST |
#endif
		0;
    infoPtr->SetupForMono8x8PatternFill = 
		SMI_SetupForMono8x8PatternFill;
    infoPtr->SubsequentMono8x8PatternFillRect =
        SMI_SubsequentMono8x8PatternFillRect;
#if 1

    /* 8x8 Color Pattern Fills */
    if (!SMI_LYNX3D_SERIES (pSmi->pHardware->Chipset) || (pScrn->bitsPerPixel != 24))
    {
        infoPtr->Color8x8PatternFillFlags = NO_PLANEMASK
            | HARDWARE_PATTERN_SCREEN_ORIGIN;
        infoPtr->SetupForColor8x8PatternFill = SMI_SetupForColor8x8PatternFill;
        infoPtr->SubsequentColor8x8PatternFillRect =
            SMI_SubsequentColor8x8PatternFillRect;
    }

#endif

#if SMI_USE_IMAGE_WRITES
    /* Image Writes */
    infoPtr->ImageWriteFlags = ROP_NEEDS_SOURCE
        | NO_PLANEMASK | CPU_TRANSFER_PAD_DWORD | SCANLINE_PAD_DWORD;
    infoPtr->ImageWriteBase = pSmi->pHardware->DataPortBase;
    infoPtr->ImageWriteRange = pSmi->pHardware->DataPortSize;
    infoPtr->SetupForImageWrite = SMI_SetupForImageWrite;
    infoPtr->SubsequentImageWriteRect = SMI_SubsequentImageWriteRect;
#endif

    /* Clipping */
    infoPtr->ClippingFlags = HARDWARE_CLIP_SCREEN_TO_SCREEN_COPY
        | HARDWARE_CLIP_MONO_8x8_FILL
        | HARDWARE_CLIP_COLOR_8x8_FILL
        | HARDWARE_CLIP_SOLID_FILL
        | HARDWARE_CLIP_SOLID_LINE | HARDWARE_CLIP_DASHED_LINE;
    infoPtr->SetClippingRectangle = SMI_SetClippingRectangle;
    infoPtr->DisableClipping = SMI_DisableClipping;

    /* Pixmap Cache */
    if (pScrn->bitsPerPixel >= 24)
    {
        infoPtr->CachePixelGranularity = 16;
    }
    else
    {
        infoPtr->CachePixelGranularity = 128 / pScrn->bitsPerPixel;
    }

    /* Offscreen Pixmaps */
    infoPtr->maxOffPixWidth = 4096;
    infoPtr->maxOffPixHeight = 4096;
    if (pScrn->bitsPerPixel == 24)
        infoPtr->maxOffPixWidth = 4096 / 3;

	
	if(SMI_NEWLYNX(pSmi->pHardware->devId))
	{
		if(pSmi->pHardware->revId == 0xfe)
		{
			SMI750LE_EngineReset(pScrn);
			SMI750LE_deInit(pScrn);
			pSmi->Accelwaitfornotbusy = SMI750LE_WaitForNotBusy;
		}
		else
			pSmi->Accelwaitfornotbusy = wait_for_not_busy_750;
	}
	else if(pSmi->pHardware->Chipset == SMI_MSOC)
		pSmi->Accelwaitfornotbusy = wait_for_not_busy_502;
	else
		pSmi->Accelwaitfornotbusy = wait_for_not_busy_712;
	if(pSmi->pHardware->revId != 0xfe)
        SMI_EngineReset (pScrn);
    ret = XAAInit (pScreen, infoPtr);

#if 0
    if (ret && pSmi->shadowFB)	/* #671 */
    {
        pSmi->ValidatePolylines = infoPtr->ValidatePolylines;
        infoPtr->ValidatePolylines = SMI_ValidatePolylines;
    }
#endif
    LEAVE(ret);
#endif
}


static int SMI750LE_deSetTransparency(SMIPtr pSmi,
    uint8_t enable, uint8_t tSelect, uint8_t tMatch, uint32_t color)
{
    if (SMI750LE_WaitForNotBusy(pSmi) != 0) {
        return -1;
    }

    if(enable) {
        POKE32(DE_COLOR_COMPARE_MASK, 0x00FFFFFF);
        POKE32( DE_COLOR_COMPARE, color);
    } else {
        POKE32(DE_COLOR_COMPARE_MASK, 0);
        POKE32( DE_COLOR_COMPARE, 0);
    }
    
    pSmi->AccelCmd &= 
        FIELD_CLEAR(DE_CONTROL, TRANSPARENCY) &
        FIELD_CLEAR(DE_CONTROL, TRANSPARENCY_MATCH) &
        FIELD_CLEAR(DE_CONTROL, TRANSPARENCY_SELECT);

    pSmi->AccelCmd |= 
        (enable ?
            FIELD_SET(0, DE_CONTROL, TRANSPARENCY, ENABLE) :
            FIELD_SET(0, DE_CONTROL, TRANSPARENCY, DISABLE)) |
        (tMatch ?
            FIELD_SET(0, DE_CONTROL, TRANSPARENCY_MATCH, TRANSPARENT) :
            FIELD_SET(0, DE_CONTROL, TRANSPARENCY_MATCH, OPAQUE)) |
        (tSelect ?
            FIELD_SET(0, DE_CONTROL, TRANSPARENCY_SELECT, DESTINATION) :
            FIELD_SET(0, DE_CONTROL, TRANSPARENCY_SELECT, SOURCE));
    return 0;
}

void SMI750LE_deInit(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR (pScrn);

	ENTER();
    SMI750LE_EngineReset (pScrn);
    POKE32( DE_MASKS, 0xFFFFFFFF);
    POKE32( DE_STRETCH_FORMAT,
        FIELD_SET(0, DE_STRETCH_FORMAT, PATTERN_XY, NORMAL) |
        FIELD_VALUE(0, DE_STRETCH_FORMAT, PATTERN_Y, 0) |
        FIELD_VALUE(0, DE_STRETCH_FORMAT, PATTERN_X, 0) |
        FIELD_SET(0, DE_STRETCH_FORMAT, ADDRESSING, XY) |
        (pScrn->bitsPerPixel == 8 ?
            FIELD_SET(0, DE_STRETCH_FORMAT, PIXEL_FORMAT, 8) :
            FIELD_SET(0, DE_STRETCH_FORMAT, PIXEL_FORMAT, 16)) |
        FIELD_VALUE(0, DE_STRETCH_FORMAT, SOURCE_HEIGHT, 3));

#if 1
	/* monk add*/
    POKE32(DE_PITCH, 
        FIELD_VALUE(0, DE_PITCH, DESTINATION, pScrn->displayWidth) |
        FIELD_VALUE(0, DE_PITCH, SOURCE, pScrn->displayWidth));
    POKE32(DE_WINDOW_WIDTH,
        FIELD_VALUE(0, DE_WINDOW_WIDTH, DESTINATION, pScrn->displayWidth) |
        FIELD_VALUE(0, DE_WINDOW_WIDTH, SOURCE, pScrn->displayWidth));
#endif

    /* Clipping and transparent are disable after INIT */
    SMI750LE_deSetClipping(pSmi, 0, 0, 0, 0, 0);
    SMI750LE_deSetTransparency(pSmi, 0, 0, 0, 0);
	LEAVE();
}


void SMI750LE_EngineReset (ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    uint32_t tmp;

    ENTER();

    tmp = PEEK32(DE_STATE1);
    tmp = FIELD_SET(tmp, DE_STATE1, DE_ABORT, ON);
    POKE32( DE_STATE1, tmp);

    tmp = PEEK32(DE_STATE1);
    tmp = FIELD_SET(tmp, DE_STATE1, DE_ABORT, OFF);
    POKE32( DE_STATE1, tmp);

    SMI_DisableClipping (pScrn);

    LEAVE();
}

int SMI750LE_WaitForNotBusy(SMIPtr pSmi)
{
    uint32_t dwVal, i;
    for (i = 0x100000; i > 0; i--)
    {
        dwVal = PEEK32( DE_STATE2);
        if ((FIELD_GET(dwVal, DE_STATE2, DE_STATUS) == 
                    DE_STATE2_DE_STATUS_IDLE) &&
                (FIELD_GET(dwVal, DE_STATE2, DE_FIFO) == 
                 DE_STATE2_DE_FIFO_EMPTY) &&
                (FIELD_GET(dwVal, DE_STATE2, DE_MEM_FIFO) == 
                 DE_STATE2_DE_MEM_FIFO_EMPTY))
        {
            return 0;
        }
    }		
    return -1;
}

void
SMI_GEReset (ScrnInfoPtr pScrn, int from_timeout, int line, char *file)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    CARD8 tmp;
    unsigned int iTempVal;
	ENTER();
	SMIHWPtr pHw = pSmi->pHardware;
	switch(pSmi->pHardware->devId){
		case SMI_502:
			ddk502_set_mmio(pHw->pReg,pHw->devId,0xC0);
			break;
		case SMI_712:
		case SMI_722:
			ddk712_set_mmio(pHw->pReg,pHw->devId);
			break;
		case SMI_750:
            ddk750_set_mmio(pHw->pReg,pHw->devId,pHw->revId );
            break;
	}

    if (from_timeout)
    {
        if (pSmi->GEResetCnt++ < 10 )
        {
            xf86DrvMsg (pScrn->scrnIndex, X_INFO,
                    "\tSMI_GEReset called from %s line %d\n", file, line);
        }
    }
    else
    {
        WaitIdleEmpty ();
    }


	if(!SMI_OLDLYNX(pHw->Chipset))/*For SM718 or SM750 or SM502*/
    {
        ErrorF ("Resetting Graphics Engine!!!\n");
        iTempVal = READ_SCR (pHw, SCR00) & ~0x00003000;
        WRITE_SCR (pHw, SCR00, (iTempVal | 0x00003000));
        WRITE_SCR (pHw, SCR00, iTempVal);
    }
    else/*For SM712 or SM722*/
    {
        tmp = VGAIN8_INDEX_712 (pSmi->pHardware, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x15);
        VGAOUT8_INDEX_712 (pSmi->pHardware, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x15, tmp | 0x30);
    }

    WaitIdleEmpty ();

	if(SMI_OLDLYNX(pHw->Chipset))/*For SM712 or SM722*/
    {
        VGAOUT8_INDEX_712 (pSmi->pHardware, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x15, tmp);
    }

    SMI_EngineReset (pScrn);
    LEAVE();
}


/* The sync function for the GE */
void SMI_AccelSync (ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR (pScrn);
	SMIHWPtr pHw = pSmi->pHardware;
	ENTER();
	/*Except SM750LE*/
    if(pHw->revId != 0xfe)
        WaitIdleEmpty();	
	
	pSmi->Accelwaitfornotbusy(pSmi);
	LEAVE();
}

void SMI_EngineReset (ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    CARD32 DEDataFormat = 0;
    int i;
    int xyAddress[] = { 320, 400, 512, 640, 800, 1024, 1280, 1600, 2048 };

	ENTER();

	pSmi->Stride = (pScrn->displayWidth * pSmi->Bpp + 15) & ~15;
	
    switch (pScrn->bitsPerPixel)
    {
        case 8:
            DEDataFormat = 0x00000000;
            break;

        case 16:
            pSmi->Stride >>= 1;
            DEDataFormat = 0x00100000;
            break;

        case 24:
            DEDataFormat = 0x00300000;
            break;

        case 32:
            pSmi->Stride >>= 2;
            DEDataFormat = 0x00200000;
            break;
    }
    /*
     * Remakrd by Belcon. Why need we do this?
     */
    for (i = 0; i < sizeof(xyAddress) / sizeof(xyAddress[0]); i++)
    {
        if (xyAddress[i] == pSmi->width)
        {
            DEDataFormat |= i << 16;
            break;
        }
    }

    WaitIdleEmpty ();
    WRITE_DPR (pHw, DPR10, (pSmi->Stride << 16) | pSmi->Stride);
    WRITE_DPR (pHw, 0x1C, DEDataFormat);
    WRITE_DPR (pHw, 0x24, 0xFFFFFFFF);
    WRITE_DPR (pHw, 0x28, 0xFFFFFFFF);
    WRITE_DPR (pHw, DPR3C, (pSmi->Stride << 16) | pSmi->Stride);

    if (!pSmi->IsSecondary)
    {
        WRITE_DPR (pHw, 0x40, 0);
        WRITE_DPR (pHw, 0x44, 0);
    }
    else
    {
		if(SMI_OLDLYNX(pSmi->pHardware->devId))
		{
	        WRITE_DPR (pHw, 0x40, pScrn->fbOffset/8);
	        WRITE_DPR (pHw, 0x44, pScrn->fbOffset/8);
		}
		else
		{
	        WRITE_DPR (pHw, 0x40, (pScrn->fbOffset / 16 << 4));
	        WRITE_DPR (pHw, 0x44, (pScrn->fbOffset / 16 << 4));
		}
    }

    SMI_DisableClipping (pScrn);

	LEAVE();

}

static void
SMI_RestoreAccelState (ScrnInfoPtr pScrn)
{
	ENTER();
    SMI_EngineReset (pScrn);
    LEAVE();
}

/****************************
 * Screen to Screen Copies
 ****************************/

static void
SMI_SetupForScreenToScreenCopy (ScrnInfoPtr pScrn, int xdir, int ydir,
        int rop, unsigned int planemask, int trans)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
	CARD32 stride = pScrn->displayWidth | (pScrn->displayWidth<<16);
	ENTER();

	// #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
    // #ifdef XORG_VERSION_CURRENT
#ifdef XORG_VERSION_NUMERIC
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
    pSmi->AccelCmd = XAAGetCopyROP (rop) | SMI_BITBLT | SMI_START_ENGINE;
#else
    pSmi->AccelCmd = XAACopyROP[rop] | SMI_BITBLT | SMI_START_ENGINE;
#endif
#else
#if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
    pSmi->AccelCmd = XAAGetCopyROP (rop) | SMI_BITBLT | SMI_START_ENGINE;
#else
    pSmi->AccelCmd = XAACopyROP[rop] | SMI_BITBLT | SMI_START_ENGINE;
#endif
#endif

    if ((xdir == -1) || (ydir == -1))
    {
        pSmi->AccelCmd |= FIELD_SET(0, DE_CONTROL, DIRECTION, RIGHT_TO_LEFT);
    } else {
        pSmi->AccelCmd |= FIELD_SET(0, DE_CONTROL, DIRECTION, LEFT_TO_RIGHT);
    }


    if (trans != -1)
    {
        pSmi->AccelCmd |= SMI_TRANSPARENT_SRC | SMI_TRANSPARENT_PXL;
        WaitQueue (1);
        WRITE_DPR (pHw, 0x20, trans);

    }

    if (pSmi->ClipTurnedOn)
    {
        WaitQueue (1);
        WRITE_DPR (pHw, 0x2C, pSmi->ScissorsLeft);
        pSmi->ClipTurnedOn = FALSE;
    }
	/*Maybe the stride will change, so set the stride for every 2D function*/
    WRITE_DPR (pHw, DPR10, stride);
    WRITE_DPR (pHw, DPR3C, stride);
	LEAVE();

}


static void
SMI_SubsequentScreenToScreenCopy (ScrnInfoPtr pScrn, int x1, int y1, int x2,
        int y2, int w, int h)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
	ENTER();

    DEBUG ("x1=%d y1=%d x2=%d y2=%d w=%d h=%d\n", x1, y1, x2, y2, w,h);

    if (pSmi->AccelCmd & SMI_RIGHT_TO_LEFT)
    {
        x1 += w - 1;
        y1 += h - 1;
        x2 += w - 1;
        y2 += h - 1;
    }

    if (pScrn->bitsPerPixel == 24)
    {
        x1 *= 3;
        x2 *= 3;
        w *= 3;

        if (pSmi->AccelCmd & SMI_RIGHT_TO_LEFT)
        {
            x1 += 2;
            x2 += 2;
        }
    }

    WaitQueue (6);
    if (!pSmi->IsSecondary)
    {
        WRITE_DPR (pHw, 0x40, 0);
        WRITE_DPR (pHw, 0x44, 0);
    }
    else
    {
		if(SMI_OLDLYNX(pSmi->pHardware->devId))
		{
	        WRITE_DPR (pHw, 0x40, pScrn->fbOffset/8);
	        WRITE_DPR (pHw, 0x44, pScrn->fbOffset/8);
		}
		else
		{
	        WRITE_DPR (pHw, 0x40, (pScrn->fbOffset / 16 << 4));
	        WRITE_DPR (pHw, 0x44, (pScrn->fbOffset / 16 << 4));
		}
    }

    WRITE_DPR (pHw, 0x00, (x1 << 16) | (y1 & 0xFFFF));
    WRITE_DPR (pHw, 0x04, (x2 << 16) | (y2 & 0xFFFF));
    WRITE_DPR (pHw, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR (pHw, 0x0C, pSmi->AccelCmd);

	LEAVE();

}

/***************/
/* Solid Fills */
/***************/

static void
SMI_SetupForSolidFill (ScrnInfoPtr pScrn, int color, int rop,
        unsigned int planemask)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    CARD32 stride = pScrn->displayWidth | (pScrn->displayWidth<<16);
	ENTER();
	
    DEBUG ("color=%08X rop=%02X\n", color, rop);

    // #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
    // #ifdef XORG_VERSION_CURRENT
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
    pSmi->AccelCmd = XAAGetPatternROP (rop) | SMI_BITBLT | SMI_START_ENGINE;
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
    pSmi->AccelCmd = XAAGetPatternROP (rop) | SMI_BITBLT | SMI_START_ENGINE;
#else
    pSmi->AccelCmd = XAAPatternROP[rop] | SMI_BITBLT | SMI_START_ENGINE;
#endif

    if (pSmi->ClipTurnedOn)
    {
        WaitQueue (4);
        WRITE_DPR (pHw, 0x2C, pSmi->ScissorsLeft);

        pSmi->ClipTurnedOn = FALSE;
    }
    else
    {
        WaitQueue (3);
    }
#if X_BYTE_ORDER == BIG_ENDIAN
	POKE32(0x100014,color);
#else
    WRITE_DPR (pHw, 0x14, color);
#endif
    WRITE_DPR (pHw, 0x34, 0xFFFFFFFF);
    WRITE_DPR (pHw, 0x38, 0xFFFFFFFF);
	/*Maybe the stride will change, so set the stride for every 2D function*/
    WRITE_DPR (pHw, DPR10, stride);
    WRITE_DPR (pHw, DPR3C, stride);
	
	LEAVE();

}

void
SMI_SubsequentSolidFillRect (ScrnInfoPtr pScrn, int x, int y, int w, int h)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
	ENTER();

    DEBUG ("x=%d y=%d w=%d h=%d\n", x, y, w, h);

    if (pScrn->bitsPerPixel == 24)
    {
        x *= 3;
        w *= 3;
    }

	if(!SMI_OLDLYNX(pSmi->pHardware->devId))/*For SM718 or SM750 or SM502*/
    {
        /* Clip to prevent negative screen coordinates */
        if (x < 0)
            x = 0;

        if (y < 0)
            y = 0;
    }

    WaitQueue (5);
    if (!pSmi->IsSecondary)
    {
        WRITE_DPR (pHw, 0x40, 0);
        WRITE_DPR (pHw, 0x44, 0);
    }
    else
    {
		if(SMI_OLDLYNX(pSmi->pHardware->devId))
		{
	        WRITE_DPR (pHw, 0x40, pScrn->fbOffset/8);
	        WRITE_DPR (pHw, 0x44, pScrn->fbOffset/8);
		}
		else
		{
	        WRITE_DPR (pHw, 0x40, (pScrn->fbOffset / 16 << 4));
	        WRITE_DPR (pHw, 0x44, (pScrn->fbOffset / 16 << 4));
		}
    }
    WRITE_DPR (pHw, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR (pHw, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR (pHw, 0x0C, pSmi->AccelCmd);

	LEAVE();
}

/***************/
/* Solid Lines */
/***************/

static void
SMI_SubsequentSolidHorVertLine (ScrnInfoPtr pScrn, int x, int y, int len,
        int dir)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    int w, h;

	ENTER();

//    DEBUG ("x=%d y=%d len=%d dir=%d\n", x, y, len, dir);

    if (dir == DEGREES_0)
    {
        w = len;
        h = 1;
    }
    else
    {
        w = 1;
        h = len;
    }

    if (pScrn->bitsPerPixel == 24)
    {
        x *= 3;
        w *= 3;
    }

    WaitQueue (5);
    if (!pSmi->IsSecondary)
    {
        WRITE_DPR (pHw, 0x40, 0);
        WRITE_DPR (pHw, 0x44, 0);
    }
    else
    {
		if(SMI_OLDLYNX(pSmi->pHardware->devId))
		{
	        WRITE_DPR (pHw, 0x40, pScrn->fbOffset/8);
	        WRITE_DPR (pHw, 0x44, pScrn->fbOffset/8);
		}
		else
		{
	        WRITE_DPR (pHw, 0x40, (pScrn->fbOffset / 16 << 4));
	        WRITE_DPR (pHw, 0x44, (pScrn->fbOffset / 16 << 4));
		}
    }
    WRITE_DPR (pHw, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR (pHw, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR (pHw, 0x0C, pSmi->AccelCmd);

	LEAVE();
}

/******************************************************************************/
/*							  Color Expansion Fills							  */
/******************************************************************************/

static void
SMI_SetupForCPUToScreenColorExpandFill (ScrnInfoPtr pScrn, int fg, int bg,
        int rop, unsigned int planemask)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    CARD32 stride = pScrn->displayWidth | (pScrn->displayWidth<<16);
	ENTER();
    DEBUG ("fg=%08X bg=%08X rop=%02X\n", fg, bg, rop);

    // #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
    // #ifdef XORG_VERSION_CURRENT
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
    pSmi->AccelCmd = XAAGetCopyROP (rop)
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
        pSmi->AccelCmd = XAAGetCopyROP (rop)
#else
        pSmi->AccelCmd = XAACopyROP[rop]
#endif
        | SMI_HOSTBLT_WRITE | SMI_SRC_MONOCHROME | SMI_START_ENGINE;
    if (bg == -1)
    {
        pSmi->AccelCmd |= SMI_TRANSPARENT_SRC;

        WaitQueue (3);
#if X_BYTE_ORDER == BIG_ENDIAN
		POKE32(0x100014,fg);
		POKE32(0x100018,~fg);
		POKE32(0x100020,fg);
#else
        WRITE_DPR (pHw, 0x14, fg);
        WRITE_DPR (pHw, 0x18, ~fg);
        WRITE_DPR (pHw, 0x20, fg);
#endif
    }
    else
    {
#if X_BYTE_ORDER == BIG_ENDIAN
		POKE32(0x100014,fg);
		POKE32(0x100018,bg);
#else
        WaitQueue (2);
        WRITE_DPR (pHw, 0x14, fg);
        WRITE_DPR (pHw, 0x18, bg);
#endif
    }
	/*Maybe the stride will change, so set the stride for every 2D function*/
    WRITE_DPR (pHw, DPR10, stride);
    WRITE_DPR (pHw, DPR3C, stride);
	LEAVE();
}

void
SMI_SubsequentCPUToScreenColorExpandFill (ScrnInfoPtr pScrn, int x, int y,
        int w, int h, int skipleft)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
	ENTER();

//    DEBUG ("x=%d y=%d w=%d h=%d skipleft=%d\n", x, y, w, h,skipleft);

    if (pScrn->bitsPerPixel == 24)
    {
        x *= 3;
        w *= 3;
        skipleft *= 3;
    }
    if (skipleft)
    {
        WaitQueue (5);
        WRITE_DPR (pHw, 0x2C, (pSmi->ScissorsLeft & 0xFFFF0000)
                | (x + skipleft) | 0x2000);
        pSmi->ClipTurnedOn = TRUE;
    }
    else
    {
        if (pSmi->ClipTurnedOn)
        {
            WaitQueue (7);
            WRITE_DPR (pHw, 0x2C, pSmi->ScissorsLeft);
            pSmi->ClipTurnedOn = FALSE;
        }
        else
        {
            WaitQueue (6);
        }
    }

    if (!pSmi->IsSecondary)
    {
        WRITE_DPR (pHw, 0x40, 0);
        WRITE_DPR (pHw, 0x44, 0);
    }
    else
    {
		if(SMI_OLDLYNX(pSmi->pHardware->devId))
		{
	        WRITE_DPR (pHw, 0x40, pScrn->fbOffset/8);
	        WRITE_DPR (pHw, 0x44, pScrn->fbOffset/8);
		}
		else
		{
	        WRITE_DPR (pHw, 0x40, (pScrn->fbOffset / 16 << 4));
	        WRITE_DPR (pHw, 0x44, (pScrn->fbOffset / 16 << 4));
		}
    }
    WRITE_DPR (pHw, 0x00, 0);
    WRITE_DPR (pHw, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR (pHw, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR (pHw, 0x0C, pSmi->AccelCmd);
	LEAVE();
}

/******************************************************************************/
/*							 8x8 Mono Pattern Fills							  */
/******************************************************************************/

static void
SMI_SetupForMono8x8PatternFill (ScrnInfoPtr pScrn, int patx, int paty, int fg,
        int bg, int rop, unsigned int planemask)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
    CARD32 stride = pScrn->displayWidth | (pScrn->displayWidth<<16);
	ENTER();

    DEBUG ("patx=%08X paty=%08X fg=%08X bg=%08X rop=%02X\n", patx,
                paty, fg, bg, rop);

    // #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
    // #ifdef XORG_VERSION_CURRENT
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
    pSmi->AccelCmd = XAAGetPatternROP (rop) | SMI_BITBLT | SMI_START_ENGINE;
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
    pSmi->AccelCmd = XAAGetPatternROP (rop) | SMI_BITBLT | SMI_START_ENGINE;
#else
    pSmi->AccelCmd = XAAPatternROP[rop] | SMI_BITBLT | SMI_START_ENGINE;
#endif

    if (pSmi->ClipTurnedOn)
    {
        WaitQueue (1);
        WRITE_DPR (pHw, 0x2C, pSmi->ScissorsLeft);
        pSmi->ClipTurnedOn = FALSE;
    }

    WaitQueue (2);
    if (!pSmi->IsSecondary)
    {
        WRITE_DPR (pHw, 0x40, 0);
        WRITE_DPR (pHw, 0x44, 0);
    }
    else
    {
		if(SMI_OLDLYNX(pSmi->pHardware->devId))
		{
	        WRITE_DPR (pHw, 0x40, pScrn->fbOffset/8);
	        WRITE_DPR (pHw, 0x44, pScrn->fbOffset/8);
		}
		else
		{
	        WRITE_DPR (pHw, 0x40, (pScrn->fbOffset / 16 << 4));
	        WRITE_DPR (pHw, 0x44, (pScrn->fbOffset / 16 << 4));
		}
    }


    if (bg == -1)
    {
        WaitQueue (5);
#if X_BYTE_ORDER == BIG_ENDIAN
		POKE32(0x10014,fg);
		POKE32(0x10018,~fg);
		POKE32(0x10020,fg);
#else
        WRITE_DPR (pHw, 0x14, fg);
        WRITE_DPR (pHw, 0x18, ~fg);
        WRITE_DPR (pHw, 0x20, fg);
#endif
        WRITE_DPR (pHw, 0x34, patx);
        WRITE_DPR (pHw, 0x38, paty);
    }
    else
    {
        WaitQueue (4);
#if X_BYTE_ORDER == BIG_ENDIAN
		POKE32(0x10014,fg);
		POKE32(0x10018,bg);
#else
        WRITE_DPR (pHw, 0x14, fg);
        WRITE_DPR (pHw, 0x18, bg);
#endif
        WRITE_DPR (pHw, 0x34, patx);
        WRITE_DPR (pHw, 0x38, paty);
    }
	/*Maybe the stride will change, so set the stride for every 2D function*/
    WRITE_DPR (pHw, DPR10, stride);
    WRITE_DPR (pHw, DPR3C, stride);
	LEAVE();
}

static void
SMI_SubsequentMono8x8PatternFillRect (ScrnInfoPtr pScrn, int patx, int paty,
        int x, int y, int w, int h)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
	ENTER();

//    DEBUG ("x=%d y=%d w=%d h=%d\n", x, y, w, h);

    if (pScrn->bitsPerPixel == 24)
    {
        x *= 3;
        w *= 3;
    }

    WaitQueue (5);

    if (!pSmi->IsSecondary)
    {
        WRITE_DPR (pHw, 0x40, 0);
        WRITE_DPR (pHw, 0x44, 0);
    }
    else
    {
		if(SMI_OLDLYNX(pSmi->pHardware->devId))
		{
	        WRITE_DPR (pHw, 0x40, pScrn->fbOffset/8);
	        WRITE_DPR (pHw, 0x44, pScrn->fbOffset/8);
		}
		else
		{
	        WRITE_DPR (pHw, 0x40, (pScrn->fbOffset / 16 << 4));
	        WRITE_DPR (pHw, 0x44, (pScrn->fbOffset / 16 << 4));
		}
    }
    WRITE_DPR (pHw, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR (pHw, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR (pHw, 0x0C, pSmi->AccelCmd);
	LEAVE();

}

/******************************************************************************/
/*							 8x8 Color Pattern Fills						  */
/******************************************************************************/

static void
SMI_SetupForColor8x8PatternFill (ScrnInfoPtr pScrn, int patx, int paty,
        int rop, unsigned int planemask,
        int trans_color)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
	CARD32 stride = pScrn->displayWidth | (pScrn->displayWidth<<16);
	ENTER();

    DEBUG ("patx=%d paty=%d rop=%02X trans_color=%08X\n", patx, paty,
                rop, trans_color);

    // #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
    // #ifdef XORG_VERSION_CURRENT
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
    pSmi->AccelCmd = XAAGetPatternROP (rop)
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
        pSmi->AccelCmd = XAAGetPatternROP (rop) 
#else
        pSmi->AccelCmd = XAAPatternROP[rop]
#endif
        | SMI_BITBLT | SMI_COLOR_PATTERN | SMI_START_ENGINE;

    if (pScrn->bitsPerPixel <= 16)
    {
        /* PDR#950 */
        CARD8 *pattern = pHw->pMem
            + (patx + paty * pSmi->Stride) * pSmi->Bpp;

        WaitIdleEmpty ();
        WRITE_DPR (pHw, 0x0C, SMI_BITBLT | SMI_COLOR_PATTERN);
        memcpy (pHw->DataPortBase, pattern, 8 * pSmi->Bpp * 8);
    }
    else
    {
        if (pScrn->bitsPerPixel == 24)
            patx *= 3;

        WaitQueue (1);
        WRITE_DPR (pHw, 0x00, (patx << 16) | (paty & 0xFFFF));
    }

    WaitQueue (2);
    if (!pSmi->IsSecondary)
    {
        WRITE_DPR (pHw, 0x40, 0);
        WRITE_DPR (pHw, 0x44, 0);
    }
    else
    {
		if(SMI_OLDLYNX(pSmi->pHardware->devId))
		{
	        WRITE_DPR (pHw, 0x40, pScrn->fbOffset/8);
	        WRITE_DPR (pHw, 0x44, pScrn->fbOffset/8);
		}
		else
		{
	        WRITE_DPR (pHw, 0x40, (pScrn->fbOffset / 16 << 4));
	        WRITE_DPR (pHw, 0x44, (pScrn->fbOffset / 16 << 4));
		}
    }

    if (trans_color == -1)
    {
        pSmi->AccelCmd |= SMI_TRANSPARENT_SRC | SMI_TRANSPARENT_PXL;

        WaitQueue (1);
        WRITE_DPR (pHw, 0x20, trans_color);
    }

    if (pSmi->ClipTurnedOn)
    {
        WaitQueue (1);
        WRITE_DPR (pHw, 0x2C, pSmi->ScissorsLeft);
        pSmi->ClipTurnedOn = FALSE;
    }
	/*Maybe the stride will change, so set the stride for every 2D function*/
    WRITE_DPR (pHw, DPR10, stride);
    WRITE_DPR (pHw, DPR3C, stride);
	LEAVE();

}

static void
SMI_SubsequentColor8x8PatternFillRect (ScrnInfoPtr pScrn, int patx, int paty,
        int x, int y, int w, int h)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
	ENTER();

//    DEBUG ("x=%d y=%d w=%d h=%d\n", x, y, w, h);

    if (pScrn->bitsPerPixel == 24)
    {
        x *= 3;
        w *= 3;
    }

    WaitQueue (5);
    if (!pSmi->IsSecondary)
    {
        WRITE_DPR (pHw, 0x40, 0);
        WRITE_DPR (pHw, 0x44, 0);
    }
    else
    {
		if(SMI_OLDLYNX(pSmi->pHardware->devId))
		{
	        WRITE_DPR (pHw, 0x40, pScrn->fbOffset/8);
	        WRITE_DPR (pHw, 0x44, pScrn->fbOffset/8);
		}
		else
		{
	        WRITE_DPR (pHw, 0x40, (pScrn->fbOffset / 16 << 4));
	        WRITE_DPR (pHw, 0x44, (pScrn->fbOffset / 16 << 4));
		}
    }
    WRITE_DPR (pHw, 0x04, (x << 16) | (y & 0xFFFF));
    WRITE_DPR (pHw, 0x08, (w << 16) | (h & 0xFFFF));	/* PDR#950 */
    WRITE_DPR (pHw, 0x0C, pSmi->AccelCmd);

	LEAVE();

}

#if SMI_USE_IMAGE_WRITES
/******************************************************************************/
/*								  Image Writes								  */
/******************************************************************************/

static void
SMI_SetupForImageWrite (ScrnInfoPtr pScrn, int rop, unsigned int planemask,
        int trans_color, int bpp, int depth)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
	ENTER();

    DEBUG ("rop=%02X trans_color=%08X bpp=%d depth=%d\n", rop,
                trans_color, bpp, depth);

    // #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
    // #ifdef XORG_VERSION_CURRENT
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
    pSmi->AccelCmd = XAAGetCopyROP (rop) | SMI_HOSTBLT_WRITE | SMI_START_ENGINE;
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
    pSmi->AccelCmd = XAAGetCopyROP (rop) | SMI_HOSTBLT_WRITE | SMI_START_ENGINE;
#else
    pSmi->AccelCmd = XAACopyROP[rop] | SMI_HOSTBLT_WRITE | SMI_START_ENGINE;
#endif

    if (trans_color != -1)
    {
        pSmi->AccelCmd |= SMI_TRANSPARENT_SRC | SMI_TRANSPARENT_PXL;

        WaitQueue (1);
        WRITE_DPR (pHw, 0x20, trans_color);
    }

	LEAVE();

}

static void
SMI_SubsequentImageWriteRect (ScrnInfoPtr pScrn, int x, int y, int w, int h,
        int skipleft)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
	ENTER();

//    DEBUG ("x=%d y=%d w=%d h=%d skipleft=%d\n", x, y, w, h,skipleft);

    if (pScrn->bitsPerPixel == 24)
    {
        x *= 3;
        w *= 3;
        skipleft *= 3;
    }

    if (skipleft)
    {
        WaitQueue (5);
        WRITE_DPR (pHw, 0x2C, (pSmi->ScissorsLeft & 0xFFFF0000) |
                (x + skipleft) | 0x2000);
        pSmi->ClipTurnedOn = TRUE;
    }
    else
    {
        if (pSmi->ClipTurnedOn)
        {
            WaitQueue (7);
            WRITE_DPR (pHw, 0x2C, pSmi->ScissorsLeft);
            pSmi->ClipTurnedOn = FALSE;
        }
        else
        {
            WaitQueue (6);
        }
    }

    if (!pSmi->IsSecondary)
    {
        WRITE_DPR (pHw, 0x40, 0);
        WRITE_DPR (pHw, 0x44, 0);
    }
    else
    {
		if(SMI_OLDLYNX(pSmi->pHardware->devId))
		{
	        WRITE_DPR (pHw, 0x40, pScrn->fbOffset/8);
	        WRITE_DPR (pHw, 0x44, pScrn->fbOffset/8);
		}
		else
		{
	        WRITE_DPR (pHw, 0x40, (pScrn->fbOffset / 16 << 4));
	        WRITE_DPR (pHw, 0x44, (pScrn->fbOffset / 16 << 4));
		}
    }

    WRITE_DPR (pHw, 0x00, 0);
    WRITE_DPR (pHw, 0x04, (x << 16) | (y * 0xFFFF));
    WRITE_DPR (pHw, 0x08, (w << 16) | (h & 0xFFFF));
    WRITE_DPR (pHw, 0x0C, pSmi->AccelCmd);

	LEAVE();

}
#endif

/******************************************************************************/
/*									Clipping								  */
/******************************************************************************/

static void
SMI_SetClippingRectangle (ScrnInfoPtr pScrn, int left, int top, int right,
        int bottom)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
	ENTER();

    DEBUG ("left=%d top=%d right=%d bottom=%d\n", left, top, right,
                bottom);

    /* CZ 26.10.2001: this code prevents offscreen pixmaps being drawn ???
       left   = max(left, 0);
       top    = max(top, 0);
       right  = min(right, pSmi->width);
       bottom = min(bottom, pSmi->height);
     */

    if (pScrn->bitsPerPixel == 24)
    {
        left *= 3;
        right *= 3;
    }


    pSmi->ScissorsLeft = (top << 16) | (left & 0xFFFF) | 0x2000;
    pSmi->ScissorsRight = (bottom << 16) | (right & 0xFFFF);

    pSmi->ClipTurnedOn = FALSE;

    WaitQueue (2);
    WRITE_DPR (pHw, 0x2C, pSmi->ScissorsLeft);

	if(SMI_OLDLYNX(pHw->devId))/*For SM712 and SM722*/
    {
        WRITE_DPR (pHw, 0x30, pSmi->ScissorsRight);
    }
    else
    {
        WRITE_DPR (pHw, 0x30, (((bottom + 1) << 16) | ((right + 1) & 0xFFFF)));
    }

	LEAVE();

}

static void
SMI_DisableClipping (ScrnInfoPtr pScrn)
{
    SMIPtr pSmi = SMIPTR (pScrn);
    SMIHWPtr pHw = pSmi->pHardware;
	ENTER();
    pSmi->ScissorsLeft = 0;
    if (pScrn->bitsPerPixel == 24)
        pSmi->ScissorsRight = (pSmi->height << 16) | (pSmi->width * 3);
    else
        pSmi->ScissorsRight = (pSmi->height << 16) | pSmi->width;

    pSmi->ClipTurnedOn = FALSE;

    WaitQueue (2);
    WRITE_DPR (pHw, 0x2C, pSmi->ScissorsLeft);
    WRITE_DPR (pHw, 0x30, pSmi->ScissorsRight);

	LEAVE();

}

int SMI750LE_deSetClipping(SMIPtr pSmi,
    uint8_t enable, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2)
{
    if(SMI750LE_WaitForNotBusy(pSmi) != 0) {
        return -1;
    }
    PEEK32( DE_CLIP_TL,
        FIELD_VALUE(0, DE_CLIP_TL, TOP, y1) |
        (enable ?
            FIELD_SET(0, DE_CLIP_TL, STATUS, ENABLE) :
            FIELD_SET(0, DE_CLIP_TL, STATUS, DISABLE)) |
        FIELD_SET(0, DE_CLIP_TL, INHIBIT, OUTSIDE) |
        FIELD_VALUE(0, DE_CLIP_TL, LEFT, x1));
    POKE32( DE_CLIP_BR,
        FIELD_VALUE(0, DE_CLIP_BR, BOTTOM, y2) |
        FIELD_VALUE(0, DE_CLIP_BR, RIGHT, x2));
    return 0;
}
/******************************************************************************/
/*									Polylines							 #671 */
/******************************************************************************/

/*

   In order to speed up the "logout" screen in rotated modes, we need to intercept
   the Polylines function. Normally, the polylines are drawn and the shadowFB is
   then sending a request of the bounding rectangle of those poylines. This should
   be okay, if it weren't for the fact that the Gnome logout screen is drawing
   polylines in rectangles and this asks for a rotation of the entire rectangle.
   This is very slow.

   To circumvent this slowness, we intercept the ValidatePolylines function and
   override the default "Fallback" Polylines with our own Polylines function. Our
   Polylines function first draws the polylines through the original Fallback
   function and then rotates the lines, line by line. We then set a flag and
   return control to the shadowFB which will try to rotate the bounding rectangle.
   However, the flag has been set and the RefreshArea function does nothing but
   clear the flag so the next Refresh that comes in shoiuld be handled correctly.

   All this code improves the speed quite a bit.

 */

#define IS_VISIBLE(pWin) \
    ( \
      pScrn->vtSema \
      && (((WindowPtr) pWin)->visibility != VisibilityFullyObscured) \
    )

#define TRIM_BOX(box, pGC) \
{ \
    BoxPtr extents = &pGC->pCompositeClip->extents; \
    if (box.x1 < extents->x1) box.x1 = extents->x1; \
    if (box.y1 < extents->y1) box.y1 = extents->y1; \
    if (box.x2 > extents->x2) box.x2 = extents->x2; \
    if (box.y2 > extents->y2) box.y2 = extents->y2; \
}

#define TRANSLATE_BOX(box, pDraw) \
{ \
    box.x1 += pDraw->x; \
    box.y1 += pDraw->y; \
    box.x2 += pDraw->x; \
    box.y2 += pDraw->y; \
}

#define BOX_NOT_EMPTY(box) \
    ((box.x2 > box.x1) && (box.y2 > box.y1))

static void
SMI_ValidatePolylines (GCPtr pGC, unsigned long changes, DrawablePtr pDraw)
{
    XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_GC (pGC);
    SMIPtr pSmi = SMIPTR (infoRec->pScrn);
	ENTER();

	/* In computer graphics a polygonal chain is called a polyline and is often used to approximate curved paths. */
    pSmi->ValidatePolylines (pGC, changes, pDraw);
    // #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
    // #ifdef XORG_VERSION_CURRENT
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
    if (pGC->ops->Polylines == XAAGetFallbackOps ()->Polylines)
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
        if (pGC->ops->Polylines == XAAGetFallbackOps ()->Polylines)
#else
            if (pGC->ops->Polylines == XAAFallbackOps.Polylines)
#endif
            {
                /* Override the Polylines function with our own Polylines function. */
                pGC->ops->Polylines = SMI_Polylines;
            }
    LEAVE();
}

static void
SMI_Polylines (DrawablePtr pDraw, GCPtr pGC, int mode, int npt,
        DDXPointPtr pptInit)
{
    XAAInfoRecPtr infoRec = GET_XAAINFORECPTR_FROM_GC (pGC);
    ScrnInfoPtr pScrn = infoRec->pScrn;
    SMIPtr pSmi = SMIPTR (pScrn);
	ENTER();


    /* Call the original Polylines function. */
    // #if XF86_VERSION_CURRENT >= XF86_VERSION_NUMERIC(6,9,0,0,0)
    // #ifdef XORG_VERSION_CURRENT
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
    pGC->ops->Polylines = XAAGetFallbackOps ()->Polylines;
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
    pGC->ops->Polylines = XAAGetFallbackOps ()->Polylines;
#else
    pGC->ops->Polylines = XAAFallbackOps.Polylines;
#endif
    (*pGC->ops->Polylines) (pDraw, pGC, mode, npt, pptInit);
    pGC->ops->Polylines = SMI_Polylines;

    if (IS_VISIBLE (pDraw) && npt)
    {
        /* Allocate a temporary buffer for all segments of the polyline. */
        BoxPtr pBox = xnfcalloc (sizeof (BoxRec), npt);
        int extra = pGC->lineWidth >> 1, box;

        if (npt > 1)
        {
            /* Adjust the extra space required per polyline segment. */
            if (pGC->joinStyle == JoinMiter)
            {
                extra = 6 * pGC->lineWidth;
            }
            else if (pGC->capStyle == CapProjecting)
            {
                extra = pGC->lineWidth;
            }
        }

        for (box = 0; --npt;)
        {
            /* Setup the bounding box for one polyline segment. */
            pBox[box].x1 = pptInit->x;
            pBox[box].y1 = pptInit->y;
            pptInit++;
            pBox[box].x2 = pptInit->x;
            pBox[box].y2 = pptInit->y;
            if (mode == CoordModePrevious)
            {
                pBox[box].x2 += pBox[box].x1;
                pBox[box].y2 += pBox[box].y1;
            }

            /* Sort coordinates. */
            if (pBox[box].x1 > pBox[box].x2)
            {
                int tmp = pBox[box].x1;
                pBox[box].x1 = pBox[box].x2;
                pBox[box].x2 = tmp;
            }
            if (pBox[box].y1 > pBox[box].y2)
            {
                int tmp = pBox[box].y1;
                pBox[box].y1 = pBox[box].y2;
                pBox[box].y2 = tmp;
            }

            /* Add extra space required for each polyline segment. */
            pBox[box].x1 -= extra;
            pBox[box].y1 -= extra;
            pBox[box].x2 += extra + 1;
            pBox[box].y2 += extra + 1;

            /* See if we need to draw this polyline segment. */
            TRANSLATE_BOX (pBox[box], pDraw);
            TRIM_BOX (pBox[box], pGC);
            if (BOX_NOT_EMPTY (pBox[box]))
            {
                box++;
            }
        }

        if (box)
			SMI_RefreshArea (pScrn, box, pBox);

        /* Free the temporary buffer. */
        xfree (pBox);
    }
    pSmi->polyLines = TRUE;
	LEAVE();
}

