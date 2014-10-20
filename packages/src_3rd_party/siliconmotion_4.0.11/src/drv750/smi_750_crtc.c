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
#include "smi_750_hw.h"


#if SMI_RANDR
static uint g_HWCOffset[]={PANEL_HWC_ADDRESS,CRT_HWC_ADDRESS};
static Bool g_argb_saved = FALSE;

#if 0
static void init_argb_cursor(SMIPtr pSmi)
{    
    if(!pSmi->entityPrivate->alphaOkay)
    {   
        alphaInit(0,0,SMI750_MAX_CURSOR,SMI750_MAX_CURSOR,
                pSmi->fb_argbCursorOffset,0,SMI750_MAX_CURSOR * 2,
                ALPHA_FORMAT_ARGB_4444,0,0,0);
                
//memset(pSmi->FBBase + pSmi->fbMapOffset+ pSmi->FBReserved, 0,(SMI501_CURSOR_SIZE + SMI750_ARGB_CURSOR_SIZE)*2);
        pSmi->entityPrivate->alphaOkay = 1;        
    }  
}


/*
    750 Alpha layer is not flexiable as cursor layer
    cursor layer can handle negetive position
    alpha layer must do some stupid data copy to accomplish negetive potion effect ...    
                                                                                    Monk @ 2009-12-31
*/

static void SMI750_ARGB_SetCursorPosition(xf86CrtcPtr crtc, int x, int y)
{

#define ALPHA_BPP 2
#define PITCH (SMI750_MAX_CURSOR * ALPHA_BPP)   

    static Bool moved = FALSE;    
    int deltaX,deltaY;
    int X,Y,v,h;
    
    uint reg1,reg2;        
	ScrnInfoPtr pScrn = crtc->scrn;
	SMIPtr	pSmi = SMIPTR(pScrn);
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);
    deltaX = deltaY = 0;
    
    char * dst = pSmi->FBBase + pSmi->fbMapOffset + pSmi->fb_argbCursorOffset;
    char * src = pSmi->FBBase + pSmi->fbMapOffset + pSmi->fb_argbCursorOffset - SMI750_ARGB_CURSOR_SIZE;
    
#if 0    
    XMSG("X = %d, Y = %d \n",x,y);    
#endif
    
    if(x < 0){
        X = 0;deltaX = (-x) * ALPHA_BPP;
    }else{
        X = x;
    }
    
    if(y < 0){
        Y = 0;deltaY = (-y)*PITCH;
    }else{
        Y = y;
    }


    alphaEnableDisplay(0);
    
    //POKE32(ALPHA_FB_WIDTH,FIELD_VALUE(PEEK32(ALPHA_FB_WIDTH),ALPHA_FB_WIDTH,WIDTH,PITCH - deltaX));  

    if(x < 0)
    { 
        /*      see if we need backup original cursor data first,copy ^dst ^ to ^src^     */
        if(!g_argb_saved){
            memcpy(src,dst,SMI750_ARGB_CURSOR_SIZE);
            g_argb_saved = TRUE;
        }
        
        /*    only move alpha data could make it , because of alpha layer address 16 byte align requirement     */                    
        for(v = deltaY;v < (SMI750_MAX_CURSOR * PITCH);v += PITCH)
        {             
#if 0        
            for(h = 0;h<(PITCH - deltaX);h++)
                *(dst + v + h) = *(src + v + deltaX + h);
#else
            memcpy(dst+v,src+v+deltaX,PITCH-deltaX);
#endif
            moved = TRUE;
        }
    }
    else
    {
        if(moved == TRUE){
            /* need original cusor data come back */
            memcpy(dst,src,SMI750_ARGB_CURSOR_SIZE);
            moved = FALSE;
        }
    }    
    
    if(y < 0){
        POKE32(ALPHA_FB_ADDRESS,
        FIELD_VALUE(PEEK32(ALPHA_FB_ADDRESS),ALPHA_FB_ADDRESS,ADDRESS,pSmi->fb_argbCursorOffset + deltaY));
    }else{        
        POKE32(ALPHA_FB_ADDRESS,
        FIELD_VALUE(PEEK32(ALPHA_FB_ADDRESS),ALPHA_FB_ADDRESS,ADDRESS,pSmi->fb_argbCursorOffset));
    }
    
    reg1 = (X & 0xffff)|(Y << 16);
    reg2 = (x + SMI750_MAX_CURSOR -  1)|((y + SMI750_MAX_CURSOR - 1)<<16);
    
    POKE32(ALPHA_PLANE_TL,reg1);
    POKE32(ALPHA_PLANE_BR,reg2);
	alphaEnableDisplay(1);
//	memset(pSmi->FBBase + pSmi->FBReserved,0xff,(SMI501_CURSOR_SIZE + SMI750_ARGB_CURSOR_SIZE)*2);
#undef PITCH    
  
}

static void SMI750_ARGB_ShowCursor(xf86CrtcPtr crtc)
{
    alphaEnableDisplay(1);
//    sm750_do_ARGB_showCursor(1);
}


static void SMI750_ARGB_HideCursor(xf86CrtcPtr crtc)
{
    alphaEnableDisplay(0);
//    sm750_do_ARGB_showCursor(0);
}

static void SMI750_ARGB_LOADARGB(xf86CrtcPtr crtc,CARD32* src)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    uint add;  
    int i;
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);
	
	//static uint cnt = 0;
	//XERR("cnt = %08x \n",cnt++);    
    //ENTER();    

	
    if(crtcPriv->controller!=0){
        return;
    }
        
//    if(crtc->cursor_argb != TRUE)
//		crtc->cursor_argb = TRUE;   
	
        
#if 0
    /* copy cursor data to onscreen */
    {
        char * fb = pSmi->FBBase;
        int cnt,i,pitch;
        cnt = SMI750_MAX_CURSOR * 4;
        pitch = pScrn->displayWidth * pSmi->Bpp;
        fb+=pitch/2;
        for(i=0;i<SMI750_MAX_CURSOR;i++){
            memcpy(fb+i*pitch,(char*)src+i*cnt,cnt);
        }        
    }
#endif

    init_argb_cursor(pSmi);
#if 0    
    XMSG("sm750 alpha cursor:\n");
    XMSG("crtc->rotation = %d:\n",crtc->rotation);
    XMSG("crtc->desiredRotation = %d:\n",crtc->desiredRotation);
    if(crtc->randr_crtc!=NULL)
        XMSG("crtc->randr_crtc->rotation = %d:\n",crtc->randr_crtc->rotation);
#endif   
    
    
#if 0    
    for(i=0;i<(SMI501_ARGB_CURSOR_SIZE)/4;i+=4)
    	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
    	"MONK :src[%d] = %08x src[%d] = %08x, src[%d] = %08x, src[%d] = %08x\n",
         i,src[i],i+1,src[i+1],i+2,src[i+2],i+3,src[i+3]);	
#endif  
    uint argb;
    unsigned short * addr = (unsigned short *)(pSmi->FBBase + pSmi->fb_argbCursorOffset);
    unsigned short * addr_bk = (unsigned short *)((unsigned long)addr - SMI750_ARGB_CURSOR_SIZE);
#if 1   
    for(i=0;i<SMI750_ARGB_CURSOR_SIZE/4;i++)
    {
        argb = (src[i] & 0xf0000000) >> 16;
        argb |= (src[i] & 0xf00000) >> 12;
        argb |= (src[i] & 0xf000) >> 8;
        argb |= (src[i] & 0xf0) >> 4;
        addr[i] = (unsigned short)argb;
        addr_bk[i] = argb;
    }    
    
    add = pSmi->fb_argbCursorOffset;
    POKE32(ALPHA_FB_ADDRESS,add);

    /* tell argb_set_position routine that cursor data updated ,need redraw backup data */
    g_argb_saved = FALSE;
#endif    
    
    //LEAVE();
}


static void SMI750_SetCursorColors(xf86CrtcPtr crtc,int bg,int fg)
{
	//translate rgb888 color into rg 565	
	//use hwc color 3 as bg and color 2 as fg
	//X use 1 for fg and 0 for bg so set 750 hw color 2 with bg and color 3 with fg
#define RBIT 0xf80000
#define GBIT 0xfc00
#define BBIT 0xf8
#define COLOR_24_16(X) (((X & RBIT) >> 8)|((X & GBIT) >> 5)|((X & BBIT) >>3))



	ScrnInfoPtr pScrn = crtc->scrn;
	SMIPtr pSmi = SMIPTR(pScrn);
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);
    uint regadd,regval;
	uint hwcolor2,hwcolor3;
    
    ENTER();      
    hwcolor2 = g_HWCOffset[crtcPriv->controller] + 8;
	hwcolor3 = g_HWCOffset[crtcPriv->controller] + 0xc;
	
    //XMSG("bg = %08x,fg = %08x\n",bg,fg);

    regval = COLOR_24_16(bg)<<16;
	POKE32(hwcolor2,regval);
	
	regval = COLOR_24_16(fg);	
	POKE32(hwcolor3,regval);
	
    LEAVE();
#undef COLOR_24_16    
#undef RBIT
#undef GBIT
#undef BBIT

}

inline static void sm750_do_showCursor(SMIPtr pSmi,int channel,int onoff)
{  
    uint regadd;
    uint offset;    
    ENTER();
    
    regadd = g_HWCOffset[channel];        
    offset = (channel == 0)?pSmi->fb_priCursorOffset:pSmi->fb_secCursorOffset;
    if(onoff)
        POKE32(regadd,offset|0x80000000);
    else
        POKE32(regadd,0);

    LEAVE();    
}


/*
    750 cursor layer is smarter , it can handle negetive potion both for X and Y
    just set outside bit and make potion multiplus -1 will reach it 
                                            Monk @ 2009-12-31
*/
static void SMI750_SetCursorPosition(xf86CrtcPtr crtc,int x,int y)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);
    uint regadd,regvalue;   
    
    XMSG("Set %s position to %d,%d\n",crtcPriv->controller?"CRT":"PANEL",x,y);
    
    regadd = g_HWCOffset[crtcPriv->controller] + 4; 
    regvalue = FIELD_VALUE(0,PRIMARY_HWC_LOCATION,TOP,y<0?1:0)|
               FIELD_VALUE(0,PRIMARY_HWC_LOCATION,Y,y<0?-y:y)|
               FIELD_VALUE(0,PRIMARY_HWC_LOCATION,LEFT,x<0?1:0)|
               FIELD_VALUE(0,PRIMARY_HWC_LOCATION,X,x<0?-x:x);
    
    POKE32(regadd,regvalue); 
}


static void SMI750_CrtcShowCursor(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);
    sm750_do_showCursor(pSmi,crtcPriv->controller,1);
}


static void SMI750_CrtcHideCursor(xf86CrtcPtr crtc)
{	
    ScrnInfoPtr pScrn = crtc->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);
    sm750_do_showCursor(pSmi,crtcPriv->controller,0);        
}

static void SMI750_CrtcLoadCursorImage(xf86CrtcPtr crtc, CARD8* image)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);
    uint regadd,regvalue;
    
    ENTER();
		
    regadd = g_HWCOffset[crtcPriv->controller];        
    regvalue = (crtcPriv->controller == 0)?pSmi->fb_priCursorOffset:pSmi->fb_secCursorOffset;    
    POKE32(regadd,regvalue);    
    memcpy(pSmi->FBBase + regvalue,image,SMI501_CURSOR_SIZE);
    
    LEAVE();
}
#endif


#if 1
static void
SMI750_CrtcAdjustFrame(xf86CrtcPtr crtc, int x, int y)
{
	ScrnInfoPtr pScrn=crtc->scrn;
	SMIPtr pSmi = SMIPTR(pScrn);
	SMIHWPtr pHw = pSmi->pHardware;
	xf86CrtcConfigPtr crtcConf = XF86_CRTC_CONFIG_PTR(pScrn);
	SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);
	ENTER();

	CARD32 Base;
	CARD32 Pitch;
	CARD32 regval;

	Pitch = pScrn->displayWidth * pSmi->Bpp;
	xf86Msg(X_INFO,"Crtc Adjust Frame:Pitch: %x\n", Pitch);
	
    if(crtc->rotatedData){
		Base = (char*)crtc->rotatedData - (char*)pSmi->pFB;
		Pitch = crtcPriv->shadow_pitch;
	}else{
		Base =  pSmi->FBOffset + (x + y * pScrn->displayWidth) * pSmi->Bpp;	
	}
	
	

	/*	adjust base address and pitch 	*/
	if(crtcPriv->controller == 0){
		WRITE_SCR(pHw,PANEL_FB_ADDRESS,Base);
		regval = FIELD_VALUE(0,PANEL_FB_WIDTH,OFFSET,Pitch)|
				FIELD_VALUE(0,PANEL_FB_WIDTH,WIDTH,Pitch);
		WRITE_SCR(pHw,PANEL_FB_WIDTH,regval);		
	}else{
		WRITE_SCR(pHw,CRT_FB_ADDRESS,Base);
		regval = READ_SCR(pHw,CRT_FB_WIDTH);
		regval = FIELD_VALUE(regval,CRT_FB_WIDTH,OFFSET,Pitch);
		WRITE_SCR(pHw,CRT_FB_WIDTH,regval);
	}

#if 0
    if(crtc->rotatedData)	
    {
        RRCrtcPtr rcrtc = crtc->randr_crtc;  
        xf86DrvMsg(pScrn->scrnIndex,X_ERROR,"MONK : In %s",__func__);
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "MONK : rotatedData != NULL \n");
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "crtc->desiredRotation= %d \n",crtc->desiredRotation);
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "crtc->x,y == %d,%d \n",crtc->x,crtc->y);
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "crtc->desiredX,desiredY == %d,%d \n",
                                                crtc->desiredX,crtc->desiredY);
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "crtc->randr_crtc->x,y == %d,%d \n",rcrtc->x,rcrtc->y);           
	}
	
#endif        
	LEAVE();	
}
#else
static void
SMI750_CrtcAdjustFrame(xf86CrtcPtr crtc, int x, int y)
{
    ScrnInfoPtr pScrn=crtc->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    xf86CrtcConfigPtr crtcConf = XF86_CRTC_CONFIG_PTR(pScrn);
   // MSOCRegPtr mode = pSmi->mode;
    CARD32 Base;

    ENTER();

    if(crtc->rotatedData)
	Base = (char*)crtc->rotatedData - (char*)pSmi->FBBase;
    else
	Base = pSmi->FBOffset + (x + y * pScrn->displayWidth) * pSmi->Bpp;

    Base = (Base + 15) & ~15;

    if (crtc == crtcConf->crtc[0]) {
	mode->panel_fb_address.f.address = Base >> 4;
	mode->panel_fb_address.f.pending = 1;
	WRITE_SCR(pSmi, PANEL_FB_ADDRESS, mode->panel_fb_address.value);
    }
    else {
	mode->crt_display_ctl.f.pixel = ((x * pSmi->Bpp) & 15) / pSmi->Bpp;
	WRITE_SCR(pSmi, CRT_DISPLAY_CTL, mode->crt_display_ctl.value);
	mode->crt_fb_address.f.address = Base >> 4;
	mode->crt_fb_address.f.mselect = 0;
	mode->crt_fb_address.f.pending = 1;
	WRITE_SCR(pSmi, CRT_FB_ADDRESS, mode->crt_fb_address.value);
    }

    LEAVE();
}
#endif


static void
SMI750_CrtcModeSet(xf86CrtcPtr crtc,
		       DisplayModePtr xf86mode,
		       DisplayModePtr adjusted_mode,
		       int x, int y)
{
   	ScrnInfoPtr pScrn;
   	SMIPtr pSmi;
   	SMICrtcPrivatePtr crtcPriv;
	pScrn = crtc->scrn;
	pSmi = SMIPTR(pScrn);
	crtcPriv = SMICRTC(crtc);
	
   	ENTER();
	
	pSmi->dispCtrl = PANEL;
	if(crtcPriv->controller == 1)
   		pSmi->dispCtrl = CRT;
	
#if 0
   	logicalMode_t Mode;


	
	pScrn = crtc->scrn;
	pSmi = SMIPTR(pScrn);
	crtcPriv = SMICRTC(crtc);
//	pEntPriv = pSmi->entityPrivate;

	Mode.xLCD = Mode.yLCD = 0;	
	if(crtcPriv->controller == 1)
   	{
   		pSmi->dispCtrl = SECONDARY_CTRL;
		Mode.xLCD = pSmi->lcdWidth;
		Mode.yLCD = pSmi->lcdHeight;
	}	


	if(Mode.xLCD * Mode.yLCD != 0){
	    /* expansion mode, only 60 hz is valid */
	    Mode.hz = 60;
	}else{	    
	    /* no expansion */	    
	    Mode.hz  = (uint32_t)(((xf86mode->VRefresh!=0) ? xf86mode->VRefresh : adjusted_mode->VRefresh) + 0.5);	    
    }	
	
   	Mode.x   = xf86mode->HDisplay;
   	Mode.y   = xf86mode->VDisplay;  	
   	Mode.bpp = pScrn->bitsPerPixel;		

   	if(crtc->desiredRotation && crtc->rotatedData)
   		Mode.baseAddress = ((FBLinearPtr)crtcPriv->shadowArea)->offset * pSmi->Bpp;
   	else
   		Mode.baseAddress = pSmi->FBOffset ;
#endif
#if 0
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "MONK : In %s \n",__func__);	
    xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "MONK : mode.dispCtrl = %d \n",Mode.dispCtrl);
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "MONK : CRTC->desiredRotation= %d \n",crtc->desiredRotation);		
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "MONK : mode.LCD = %dx%d \n",Mode.xLCD,Mode.yLCD);
	
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "MONK : pScrn->virtual[X,Y] == %d,%d \n",pScrn->virtualX,pScrn->virtualY);
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "MONK : pScrn->displayWidth == %d\n",pScrn->displayWidth);
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "MONK : position x,y== %d,%d \n",x,y);	
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "MONK : Mode = [%d x %d]@%d hz\n",Mode.x,Mode.y,Mode.hz);
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "MONK : xf86mode->VRefresh = %f\n",xf86mode->VRefresh);
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "MONK : adjusted_mode->VRefresh = %f\n",adjusted_mode->VRefresh);
#endif

#if 0
    /* if mode.hz == 71~76, we set it to 75 */
    if(Mode.hz > 70 && Mode.hz < 77)
        Mode.hz = 75;
#else

	/*XMSG("Mode.[x,y,hz] = [%d,%d,%d]\n",Mode.x,Mode.y,Mode.hz);
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
    XMSG("DisplayModePtr->VScan = %d\n",xf86mode->VScan);*/
	pSmi->psSetMode(pScrn,xf86mode);
	pSmi->psSetDisplay(pScrn,pScrn->currentMode);
#if 0
#ifndef SMI_EDID_ENTIRE
//    if(setModeEx(&Mode) == -1)
if(sm750_setMode(&Mode) == -1)
    {        
    	XMSG("Failed on setModeEx\n");
        /* the request mode can not be set by DDK ,directly set the mode  */
        if(xorg_setMode(&Mode,adjusted_mode))
//        {
            XMSG("Failed on xorg_setMode\n");
  //      }        
    }
#else
	if(Mode.xLCD && Mode.yLCD){
		/* expansion needed ,we use setModeEx */
//		if(setModeEx(&Mode) == -1)
	if(sm750_setMode(&Mode) == -1)
			XERR("Opps,seems DDK setmode failed \n");
		
	}else{
		/* common mode set requirment ,just use common routine */
		if(xorg_setMode(&Mode,adjusted_mode))
	//		XERR("Opps,seems xorg_setMode failed\n");
	}
#endif
#endif
#endif

   SMI750_CrtcAdjustFrame(crtc, x, y); 
 
    LEAVE();
}


static Bool
SMI750_CrtcLock (xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);

    ENTER();

 //   deWaitForNotBusy();

    LEAVE(FALSE);
}



static void
SMI750_CrtcLoadLUT(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn = crtc->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    xf86CrtcConfigPtr crtcConf = XF86_CRTC_CONFIG_PTR(pScrn);
    SMICrtcPrivatePtr crtcPriv = SMICRTC(crtc);

    ENTER();


    LEAVE();
}


/*
 * Implementation
 */
static void
SMI750_CrtcVideoInit_lcd(xf86CrtcPtr crtc)
{
    ScrnInfoPtr pScrn=crtc->scrn;
    SMIPtr pSmi = SMIPTR(pScrn);
    ENTER();
	


    LEAVE();
}
Bool SMI750_CrtcPreInit(ScrnInfoPtr pScrn)
{
	SMIPtr	pSmi = SMIPTR(pScrn);
	xf86CrtcPtr crtc;
	xf86CrtcFuncsPtr crtcFuncs;
	SMICrtcPrivatePtr crtcPriv;

	ENTER();

	int idx = 0;
	int c = (pSmi->DualView?2:1);
	Bool expansion = (pSmi->lcdWidth >0 && pSmi->lcdHeight > 0);
	SMI_CrtcPreInit(pScrn);
	while(idx < c)
	{		
		crtcFuncs = NULL;
		crtcPriv = NULL;
		if(! SMI_CrtcFuncsInit_base(&crtcFuncs, &crtcPriv))
			 LEAVE(FALSE);

	    crtcFuncs->mode_set		= SMI750_CrtcModeSet;
	    crtcFuncs->lock		= SMI750_CrtcLock;		
	    crtcPriv->set_backlight	= set_backlight_750;
	    crtcPriv->adjust_frame	= SMI750_CrtcAdjustFrame;
	    crtcPriv->video_init	= SMI750_CrtcVideoInit_lcd;
	    crtcPriv->load_lut		= SMI750_CrtcLoadLUT;
	    crtcPriv->index = idx;
	    
#if 0	    
	    if(expansion && pSmi->DualView)
    		crtcPriv->controller = (idx+1)&1;
		else if(expansion)
		    crtcPriv->controller = 1;
	    else
    	    crtcPriv->controller = idx;   
#else
        if(expansion){  
        /*  if expansion enabled, than primary channel will be put into crtc_1 and secondary be crtc_0
                    This limitation need be take care 
              */
            crtcPriv->controller = (idx+1)&1;
        }else{
            crtcPriv->controller = idx;
        }        
#endif
#if 0        
	if(pSmi->ARGBCursor && !pSmi->DualView)
        {           
            crtcFuncs->set_cursor_position = SMI750_ARGB_SetCursorPosition;
            crtcFuncs->show_cursor = SMI750_ARGB_ShowCursor;
            crtcFuncs->hide_cursor = SMI750_ARGB_HideCursor;
            crtcFuncs->load_cursor_argb = SMI750_ARGB_LOADARGB;             
            //init_argb_cursor(pSmi);
        }
	    else if (pSmi->HwCursor)
		{	
			crtcFuncs->set_cursor_colors = SMI750_SetCursorColors;
			crtcFuncs->set_cursor_position = SMI750_SetCursorPosition;                
			crtcFuncs->show_cursor = SMI750_CrtcShowCursor;
			crtcFuncs->hide_cursor = SMI750_CrtcHideCursor;						
			crtcFuncs->load_cursor_image = SMI750_CrtcLoadCursorImage;
		}
#endif
	    if (! (crtc = xf86CrtcCreate(pScrn, crtcFuncs)))
		{	LEAVE(FALSE);

		}
		crtc->driver_private = crtcPriv;
		idx++;	
	}
    LEAVE(TRUE);
}

#endif /*#if SMI_RANDR*/
