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
#include "../smi_dbg.h"
#include "../smi_common.h"
#include "../smi_crtc.h"
#include "../ddk750/ddk750.h"
#include "../smi_accel.h"
#if SMI_RANDR

static void SMI750LE_CrtcCommit(xf86CrtcPtr);
static void *SMI750LE_CrtcShadowAllocate(xf86CrtcPtr, int, int);
static void SMI750LE_CrtcAdjustFrame(xf86CrtcPtr, int, int);
static void SMI750LE_CrtcModeSet(xf86CrtcPtr, DisplayModePtr, DisplayModePtr, 
        int, int);
static Bool SMI750LE_CrtcLock(xf86CrtcPtr);
static void SMI750LE_CrtcLoadLUT(xf86CrtcPtr);
static Bool SMI750LE_CrtcConfigResize(ScrnInfoPtr, int, int);
static void SMI750LE_CrtcShadowDestroy(xf86CrtcPtr, PixmapPtr, void*);
static Bool SMI750LE_CrtcConfigResize(ScrnInfoPtr, int, int);
static void SMI750LE_CrtcLoadLUT(xf86CrtcPtr);
static void SMI750LE_CrtcVideoInit(xf86CrtcPtr);

static xf86CrtcConfigFuncsRec SMI750LE_CrtcConfigFuncs = {
    .resize = SMI750LE_CrtcConfigResize
};

static void
SMI750LE_CrtcCommit(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    SMIPtr pSmi =SMIPTR(pScrn);
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);
    ENTER();

    crtc->funcs->dpms(crtc,DPMSModeOn);
    LEAVE();
}

/*    if no enough memory for allcation, please return NULL pointer  */
static void *SMI750LE_CrtcShadowAllocate (xf86CrtcPtr crtc, int width, int height)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);
    int	offset, size;

    ENTER();

/*    size = ((width * pSmi->Bpp + 15) & ~15)  * height;
    offset = SMI750LE_AllocateMemory(pScrn, &crtcPriv->shadowArea, size);
    if(offset!= 0)
    {
        crtcPriv->shadow_pitch = ((width* pSmi->Bpp)+15)&~15;
        if (!crtcPriv->shadowArea)
        	LEAVE(NULL);
        LEAVE(pSmi->pFB + offset);
    }
*/
    LEAVE(NULL);
}

static void SMI750LE_CrtcAdjustFrame(xf86CrtcPtr crtc, int x, int y)
{
    ScrnInfoPtr pScrn=crtc->scrn;
    SMIPtr pSmi =SMIPTR(pScrn);
    xf86CrtcConfigPtr crtcConf = XF86_CRTC_CONFIG_PTR(pScrn);
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);
    ENTER();

    CARD32 Base;
    CARD32 Pitch;
    CARD32 regval;

    Pitch = pScrn->displayWidth * pSmi->Bpp;
    if(crtc->rotatedData){
        Base = (char*)crtc->rotatedData - (char*)pSmi->pFB;
        Pitch = crtcPriv->shadow_pitch;
    }else{
        Base = pSmi->FBOffset + 
            (x + y * pScrn->displayWidth) * pSmi->Bpp;	
    }

    /*	adjust base address and pitch 	*/
    POKE32(CRT_FB_ADDRESS,Base);
    //regval = PEEK32(SECONDARY_FB_WIDTH); 
    regval = FIELD_VALUE(0,CRT_FB_WIDTH,OFFSET,Pitch) |
                FIELD_VALUE(0, CRT_FB_WIDTH, WIDTH, Pitch);
    POKE32(CRT_FB_WIDTH,regval);
    LEAVE();	
}

static void SMI750LE_CrtcModeSet(xf86CrtcPtr crtc,
        DisplayModePtr xf86mode,
        DisplayModePtr adjusted_mode,
        int x, int y)
{
    ScrnInfoPtr pScrn;
    SMIPtr pSmi;
    SMICrtcPrivatePtr crtcPriv;

    ENTER();
    pScrn = crtc->scrn;
    pSmi =SMIPTR(pScrn);
    crtcPriv = SMICRTC(crtc);
#if 0
    pEntPriv = pSmi->entityPrivate;

    Mode.dispCtrl = CRT_CTRL;
    Mode.xLCD = Mode.yLCD = 0;	
    Mode.hz  = (uint32_t)(((xf86mode->VRefresh!=0) ? xf86mode->VRefresh : 
                adjusted_mode->VRefresh) + 0.5);	    
    Mode.x   = xf86mode->HDisplay;
    Mode.y   = xf86mode->VDisplay;  	
    Mode.bpp = pScrn->bitsPerPixel;		

    if(crtc->desiredRotation && crtc->rotatedData)
        Mode.baseAddress = ((FBLinearPtr)crtcPriv->shadowArea)->offset * pSmi->Bpp;
    else
        Mode.baseAddress = pSmi->FBOffset + pSmi->fbMapOffset;

    XMSG("Mode.[x,y,hz] = [%d,%d,%d]\n",Mode.x,Mode.y,Mode.hz);
    XMSG("member of DisplayModePtr \n");
    XMSG("DisplayModePtr->Clock = %d\n",xf86mode->Clock);
    XMSG("DisplayModePtr->HSync = %f\n",xf86mode->HSync);
    XMSG("DisplayModePtr->VRefresh = %f\n",xf86mode->VRefresh);


    XMSG("DisplayModePtr->HDisplay = %d\n",xf86mode->HDisplay);
    XMSG("DisplayModePtr->HSyncStart = %d\n",xf86mode->HSyncStart);
    XMSG("DisplayModePtr->HSyncEnd = %d\n",xf86mode->HSyncEnd);
    XMSG("DisplayModePtr->HTotal = %d\n",xf86mode->HTotal);
    XMSG("DisplayModePtr->HSkew = %d\n",xf86mode->HSkew);

    XMSG("DisplayModePtr->VDisplay = %d\n",xf86mode->VDisplay);
    XMSG("DisplayModePtr->VSyncStart = %d\n",xf86mode->VSyncStart);
    XMSG("DisplayModePtr->VSyncEnd = %d\n",xf86mode->VSyncEnd);
    XMSG("DisplayModePtr->VTotal = %d\n",xf86mode->VTotal);
    XMSG("DisplayModePtr->VScan = %d\n",xf86mode->VScan);

    if(setModeEx(&Mode) == -1)
    {        
        XMSG("Failed on setModeEx\n");
        /* the request mode can not be set by DDK ,directly set the mode  */
        if(xorg_setMode(&Mode,adjusted_mode))
        {
            XMSG("Failed on xorg_setMode\n");
        }        
    }

#if 0
    if(Mode.xLCD && Mode.yLCD){
        /* expansion needed ,we use setModeEx */
        if(setModeEx(&Mode) == -1)
            XERR("Opps,seems DDK setmode failed \n");

    }else{
        /* common mode set requirment ,just use common routine */
        if(xorg_setMode(&Mode,adjusted_mode))
            XERR("Opps,seems xorg_setMode failed\n");
    }
#endif
#endif
    pSmi->psSetMode(pScrn,xf86mode);
    pSmi->psSetDisplay(pScrn,pScrn->currentMode);
    SMI750LE_CrtcAdjustFrame(crtc, x, y); 
    LEAVE();
}


static Bool SMI750LE_CrtcLock (xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    SMIPtr pSmi =SMIPTR(pScrn);
    ENTER();
//    deWaitForNotBusy();
    LEAVE(FALSE);
}

static Bool SMI750LE_CrtcConfigResize(ScrnInfoPtr pScrn, int width, int height)
{
    SMIPtr pSmi =SMIPTR(pScrn);
    xf86CrtcConfigPtr crtcConf = XF86_CRTC_CONFIG_PTR(pScrn);
    int i, aligned_pitch;
    xf86CrtcPtr crtc;

    ENTER();
    aligned_pitch = (width*pSmi->Bpp + 15) & ~15;
    /* Modify the screen frame buffer address */	
    pScrn->fbOffset = pSmi->FBOffset;
    pScrn->pScreen->ModifyPixmapHeader(
            pScrn->pScreen->GetScreenPixmap(pScrn->pScreen),
            -1,-1,-1,-1,-1, pSmi->pFB + pSmi->FBOffset);
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,10,0,0,0)
    if(pScrn->pixmapPrivate.ptr)
        /* The pixmap devPrivate just set may be overwritten by 
           xf86EnableDisableFBAccess */
        pScrn->pixmapPrivate.ptr = pSmi->pFB + pSmi->FBOffset;
#endif
    /* Modify the screen pitch */
    pScrn->displayWidth = aligned_pitch / pSmi->Bpp;
    pScrn->pScreen->ModifyPixmapHeader(
            pScrn->pScreen->GetScreenPixmap(pScrn->pScreen),
            -1, -1, -1, -1, aligned_pitch, NULL);
    /* Modify the screen dimensions */
    pScrn->virtualX = width;
    pScrn->virtualY = height;
    pScrn->pScreen->ModifyPixmapHeader(
            pScrn->pScreen->GetScreenPixmap(pScrn->pScreen),
            width, height, -1, -1, 0, NULL);
    /* Setup each crtc video processor */
    for(i=0;i<crtcConf->num_crtc;i++){
        crtc = crtcConf->crtc[i];
        SMICRTC(crtc)->video_init(crtc);
        SMICRTC(crtc)->adjust_frame(crtc,crtc->x,crtc->y);	
    }
    LEAVE(TRUE);
}

static void SMI750LE_CrtcShadowDestroy(xf86CrtcPtr crtc, 
        PixmapPtr pPixmap, void *data)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);

    ENTER();
/*
        if (crtcPriv->shadowArea) {
	SMI750LE_FreeMemory(pScrn, crtcPriv->shadowArea);
	crtcPriv->shadowArea = NULL;
    }
*/
  LEAVE();
}

static void SMI750LE_CrtcLoadLUT(xf86CrtcPtr crtc)
{
#if 0
    ScrnInfoPtr pScrn = crtc->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    xf86CrtcConfigPtr crtcConf = XF86_CRTC_CONFIG_PTR(pScrn);
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);
#endif
    ENTER();
    LEAVE();
}

static void SMI750LE_CrtcVideoInit(xf86CrtcPtr crtc)
{
#if 0
    ScrnInfoPtr pScrn=crtc->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
#endif
    ENTER();
    LEAVE();
}

Bool SMI750le_CrtcPreInit(ScrnInfoPtr pScrn)
{
    SMIPtr pSmi =SMIPTR(pScrn);
    xf86CrtcPtr crtc;
    xf86CrtcFuncsPtr crtcFuncs;
    SMICrtcPrivatePtr crtcPriv;
	SMIHWPtr pHw = pSmi->pHardware;
	
    ENTER();

    xf86CrtcConfigInit(pScrn, &SMI750LE_CrtcConfigFuncs);
    xf86CrtcSetSizeRange(pScrn,128,128,4096,4096);
    int idx = 0;
    int c = (pSmi->DualView?2:1);

    while(idx < c)
    {		
        crtcFuncs = xnfcalloc(sizeof(xf86CrtcFuncsRec), 1);
        crtcPriv = xnfcalloc(sizeof(SMICrtcPrivateRec), 1);
        crtcFuncs->dpms = SMI_CrtcDPMS;
        crtcFuncs->lock = SMI_CrtcLock;
        crtcFuncs->unlock = SMI_CrtcUnlock;
        crtcFuncs->mode_fixup = SMI_CrtcModeFixup;
        crtcFuncs->prepare = SMI_CrtcPrepare;
        crtcFuncs->mode_fixup = SMI_CrtcModeFixup;
        crtcFuncs->commit = SMI750LE_CrtcCommit;
        crtcFuncs->gamma_set = SMI_CrtcGammaSet;
        crtcFuncs->shadow_allocate = SMI750LE_CrtcShadowAllocate;
        crtcFuncs->shadow_create = SMI_CrtcShadowCreate;
        crtcFuncs->shadow_destroy = SMI750LE_CrtcShadowDestroy;//SMI_CrtcShadowDestroy;
        crtcFuncs->destroy = SMI_CrtcDestroy;
        crtcFuncs->mode_set = SMI750LE_CrtcModeSet;
        crtcFuncs->lock = SMI750LE_CrtcLock;		
        crtcPriv->adjust_frame = SMI750LE_CrtcAdjustFrame;
        crtcPriv->video_init = SMI750LE_CrtcVideoInit;
        crtcPriv->load_lut = SMI750LE_CrtcLoadLUT;
        crtcPriv->index = idx;
        crtcPriv->controller = idx;
        if (! (crtc = xf86CrtcCreate(pScrn, crtcFuncs)))
            LEAVE(FALSE);
        crtc->driver_private = crtcPriv;
        idx++;	
    }
    LEAVE(TRUE);
}

#endif /*#if SMI_RANDR*/

