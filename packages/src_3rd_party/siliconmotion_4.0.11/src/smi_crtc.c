/*
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
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "smi_dbg.h"
#include "smi_crtc.h"
#include "smi_common.h"
Bool SMI_CrtcPreInit(ScrnInfoPtr pScrn);
#if 1
void SMI_CrtcDPMS(xf86CrtcPtr crtc, int	mode)
{
    ENTER();
    /* Nothing */
    LEAVE();
}
#endif

#if 1
Bool SMI_CrtcLock (xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    SMIPtr pSmi = SMIPTR (pScrn);
    ENTER();
    LEAVE(FALSE);
}
#endif

#if 1
void SMI_CrtcUnlock (xf86CrtcPtr crtc)
{
    ENTER();
    /* Nothing */
    LEAVE();
}
#endif

Bool SMI_CrtcModeFixup(xf86CrtcPtr crtc,
        DisplayModePtr mode,
        DisplayModePtr adjusted_mode)
{
    ENTER();
    /* mill add*/
    if(!mode->VRefresh)
		adjusted_mode->VRefresh = xf86ModeVRefresh(mode);
    LEAVE(TRUE);
}

void SMI_CrtcPrepare(xf86CrtcPtr crtc)
{
    ENTER();
    LEAVE();
}

static void
SMI_CrtcCommit(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);
    ENTER();

    crtc->funcs->dpms(crtc,DPMSModeOn);
    //if (IS_SM750(pSmi) && pSmi->ARGBCursor) 
 #if 0 
    if(pSmi->HwCursor)
    {
        if(crtc->scrn->pScreen != NULL)
            xf86_reload_cursors(crtc->scrn->pScreen);//x will crash if hardware cursor not registered
    }
#endif
    LEAVE();
}

void SMI_CrtcGammaSet(xf86CrtcPtr crtc, CARD16 *red, 
        CARD16 *green, CARD16 *blue, int size)
{
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);
    int i;
    ENTER();
    for(i=0; i<256; i++){
        crtcPriv->lut_r[i] = red[i * size >> 8];
        crtcPriv->lut_g[i] = green[i * size >> 8];
        crtcPriv->lut_b[i] = blue[i * size >> 8];
    }
    crtcPriv->load_lut(crtc);
    LEAVE();
}

/*    if no enough memory for allcation, please return NULL pointer  */
static void *
SMI_CrtcShadowAllocate (xf86CrtcPtr crtc, int width, int height)
{
    ScrnInfoPtr	 	 pScrn = crtc->scrn;
    SMIPtr	 	 pSmi = SMIPTR(pScrn);
    SMICrtcPrivatePtr	 crtcPriv = SMICRTC(crtc);
    int			 offset, size;

    ENTER();

    size = ((width * pSmi->Bpp + 15) & ~15)  * height;
    offset = SMI_AllocateMemory(pScrn, &crtcPriv->shadowArea, size);
    if(offset!= 0)
    {
        crtcPriv->shadow_pitch = ((width* pSmi->Bpp)+15)&~15;
        
        if (!crtcPriv->shadowArea)
        {
		LEAVE(NULL);
	}

        LEAVE(pSmi->pFB + offset);
    }
    LEAVE(NULL);
}

/* return 0 means no enough video memory for allcation */
int
SMI_AllocateMemory(ScrnInfoPtr pScrn, void **mem_struct, int size)
{
    ScreenPtr	pScreen = screenInfo.screens[pScrn->scrnIndex];
    SMIPtr	pSmi = SMIPTR(pScrn);
    int		offset = 0;
    return 0;
#if 0
    ENTER();
    
    	FBLinearPtr	linear = *mem_struct;

    	/*  XAA allocates in units of pixels at the screen bpp,
        	 *  so adjust size appropriately.
        	 */
    	size = (size + pSmi->Bpp - 1) / pSmi->Bpp;
    if (linear)
    	{
	    if (linear->size >= size)
    		LEAVE(linear->offset * pSmi->Bpp);

    	    if (xf86ResizeOffscreenLinear(linear, size))
    		LEAVE(linear->offset * pSmi->Bpp);

    	    xf86FreeOffscreenLinear(linear);
    	}
    	else
    	{
    	    int max_size;

    	    xf86QueryLargestOffscreenLinear(pScreen, &max_size, 16,
    					    PRIORITY_EXTREME);
    	    if (max_size < size)
   		LEAVE(0);   		

    	    xf86PurgeUnlockedOffscreenAreas(pScreen);
    	}

    	linear = xf86AllocateOffscreenLinear(pScreen, size, 16,
					     NULL, NULL, NULL);
    	if ((*mem_struct = linear) != NULL)
    	    offset = linear->offset * pSmi->Bpp;

    	DEBUG("offset = %p\n", offset);

    LEAVE(offset);
#endif    
}

PixmapPtr
SMI_CrtcShadowCreate (xf86CrtcPtr crtc, void *data, int width, int height)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    int aligned_pitch;
    ENTER();
    aligned_pitch = (width * (pScrn->bitsPerPixel >> 3) + 15) & ~15;
    LEAVE(GetScratchPixmapHeader(pScrn->pScreen,width,height,pScrn->depth,
				  pScrn->bitsPerPixel,aligned_pitch,data));
}

static void
SMI_CrtcShadowDestroy (xf86CrtcPtr crtc, PixmapPtr pPixmap, void *data)
{
    ScrnInfoPtr		pScrn = crtc->scrn;
    SMIPtr		pSmi = SMIPTR(pScrn);
    SMICrtcPrivatePtr	crtcPriv = SMICRTC(crtc);

    ENTER();
#if 0
    if (pSmi->useEXA && pPixmap)
	FreeScratchPixmapHeader(pPixmap);

    if (crtcPriv->shadowArea) {
	SMI_FreeMemory(pScrn, crtcPriv->shadowArea);
	crtcPriv->shadowArea = NULL;
    }
#endif
    LEAVE();
}

void SMI_CrtcDestroy (xf86CrtcPtr crtc)
{
    ENTER();
    xfree(SMICRTC(crtc));
    //xfree(crtc->funcs);
    LEAVE();
}

static Bool SMI_CrtcConfigResize(ScrnInfoPtr pScrn, int width, int height)
{
    SMIPtr pSmi = SMIPTR(pScrn);
    xf86CrtcConfigPtr crtcConf = XF86_CRTC_CONFIG_PTR(pScrn);
    int i;
    xf86CrtcPtr crtc;

    ENTER();
	/*if mode is not changed,don't turn off backlight*/
    if (pScrn->virtualX == width && pScrn->virtualY == height)
	    return TRUE;

    int aligned_pitch = (width*pSmi->Bpp + 15) & ~15;
    /* Allocate another offscreen area and use it as screen, if it really has to be resized */
#if 0
    if(!pSmi->NoAccel && pSmi->useEXA &&
            ( !pSmi->fbArea || width != pScrn->virtualX || height != pScrn->virtualY ))
    {


        ExaOffscreenArea* fbArea = exaOffscreenAlloc(pScrn->pScreen, aligned_pitch*height, 16, TRUE, NULL, NULL);
        if(!fbArea)
        {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                    "SMI_CrtcConfigResize: Not enough memory to resize the framebuffer\n");
            LEAVE(FALSE);
        }

        if(pSmi->fbArea)
            exaOffscreenFree(pScrn->pScreen, pSmi->fbArea);

        pSmi->fbArea = fbArea;
        pSmi->FBOffset = fbArea->offset;
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,"MONK : fbArea->base_offset = \n",fbArea->base_offset);
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,"MONK : fbArea->size = \n",fbArea->size);

    }
#endif
    /* Modify the screen frame buffer address */	
    pScrn->fbOffset = pSmi->FBOffset ;
    pScrn->pScreen->ModifyPixmapHeader(pScrn->pScreen->GetScreenPixmap(pScrn->pScreen),
            -1,-1,-1,-1,-1, pSmi->pFB + pSmi->FBOffset);
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,10,0,0,0)
    if(pScrn->pixmapPrivate.ptr)
        /* The pixmap devPrivate just set may be overwritten by  xf86EnableDisableFBAccess */
        pScrn->pixmapPrivate.ptr = pSmi->pFB + pSmi->FBOffset;
#endif

    /* Modify the screen pitch */
    pScrn->displayWidth = aligned_pitch / pSmi->Bpp;
    pScrn->pScreen->ModifyPixmapHeader(pScrn->pScreen->GetScreenPixmap(pScrn->pScreen),
            -1, -1, -1, -1, aligned_pitch, NULL);


    /* Modify the screen dimensions */
    pScrn->virtualX = width;
    pScrn->virtualY = height;
    pScrn->pScreen->ModifyPixmapHeader(pScrn->pScreen->GetScreenPixmap(pScrn->pScreen),
            width, height, -1, -1, 0, NULL);


    /*adjust_frame causes garbage in the process of switch mode(only low to high).
     * Disable PLANE(PNL) and OFF DPMS(CRT),
     * restore them in smi_OutputCommit and smi_OutputModeValid*/
    /* Setup each crtc video processor */
    for(i=0;i<crtcConf->num_crtc;i++){
        crtc = crtcConf->crtc[i];
	SMICRTC(crtc)->set_backlight(0);
        SMICRTC(crtc)->video_init(crtc);
        SMICRTC(crtc)->adjust_frame(crtc,crtc->x,crtc->y);	
    }

    LEAVE(TRUE);
}

Bool
SMI_CrtcFuncsInit_base(xf86CrtcFuncsPtr* crtcFuncs, SMICrtcPrivatePtr* crtcPriv){

	ENTER();
    *crtcFuncs = xnfcalloc(sizeof(xf86CrtcFuncsRec), 1);
    *crtcPriv = xnfcalloc(sizeof(SMICrtcPrivateRec), 1);

	if(*crtcFuncs == NULL || *crtcPriv == NULL)
	{
		xfree(*crtcFuncs);
		xfree(*crtcPriv);
		LEAVE(FALSE);
	}

    (*crtcFuncs)->dpms = SMI_CrtcDPMS;//SMI_CrtcDPMS;
    (*crtcFuncs)->lock = SMI_CrtcLock;
    (*crtcFuncs)->unlock = SMI_CrtcUnlock;
    (*crtcFuncs)->mode_fixup = SMI_CrtcModeFixup;
    (*crtcFuncs)->prepare = SMI_CrtcPrepare;
    (*crtcFuncs)->commit = SMI_CrtcCommit;
    (*crtcFuncs)->gamma_set = SMI_CrtcGammaSet;
    (*crtcFuncs)->shadow_allocate = SMI_CrtcShadowAllocate;
    (*crtcFuncs)->shadow_create = SMI_CrtcShadowCreate;
    (*crtcFuncs)->shadow_destroy = SMI_CrtcShadowDestroy;
    (*crtcFuncs)->destroy = SMI_CrtcDestroy;

	LEAVE(TRUE);
}

static xf86CrtcConfigFuncsRec SMI_CrtcConfigFuncs = {
    .resize = SMI_CrtcConfigResize
};

Bool SMI_CrtcPreInit(ScrnInfoPtr pScrn)
{
    ENTER();

    xf86CrtcConfigInit(pScrn,&SMI_CrtcConfigFuncs);
    xf86CrtcSetSizeRange(pScrn,128,128,4096,4096);
    LEAVE(TRUE);
}
