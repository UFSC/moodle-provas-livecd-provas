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


#ifndef _SMI_CRTC_H
#define _SMI_CRTC_H

#include "xf86Crtc.h"



typedef struct {
    int index;  //crtc_0 or crtc_1
	int controller;    //primary controller or secondary controller
	
    /* Memory copy of the CRTC color palette */
    CARD16 lut_r[256],lut_g[256],lut_b[256];
    /* Allocated memory area used as shadow pixmap (for rotation) */
    void* shadowArea;
    int shadow_pitch;   /* pitch value used when in rotated mode */
	void (*set_backlight)(int x);
    /* Setup the CRTC registers to show the specified framebuffer location*/
    void (*adjust_frame)(xf86CrtcPtr crtc, int x, int y);
    /* Setup the CRTC framebuffer format. Called when the FB is
       resized to modify the screen stride */
    void (*video_init)(xf86CrtcPtr crtc);
    /* Load the LUT fields above to the hardware */
    void (*load_lut)(xf86CrtcPtr crtc);

#if SMI_CURSOR_ALPHA_PLANE
    Bool	argb_cursor;
#endif
} SMICrtcPrivateRec, *SMICrtcPrivatePtr;

typedef struct {
    int index;  // output_0 or output_1 
    int path;	//0 means PANEL path,1 means CRT path
    int head;	//exactly the output_head_t:TFT,DSTN,LVDS,VGA,etc....
    void (*set_backlight)(int x);	
} SMIOutputPrivateRec, *SMIOutputPrivatePtr;

#define SMICRTC(crtc) ((SMICrtcPrivatePtr)(crtc)->driver_private)
#define SMIOUTPUT(output)   ((SMIOutputPrivatePtr)(output)->driver_private)

/* smi_crtc.c */
/* Initialize the xf86CrtcFuncsRec with functions common to all the hardware */
Bool SMI_CrtcFuncsInit_base(xf86CrtcFuncsPtr* crtcFuncs, SMICrtcPrivatePtr* crtcPriv);
/* Create and initialize the display controllers. */
Bool SMI_CrtcPreInit(ScrnInfoPtr pScrn);

/* smi_output.c */
/* Initialize the xf86OutputFuncsRec with functions common to all the hardware */
Bool SMI_OutputFuncsInit_base(xf86OutputFuncsPtr* outputFuncs);
/* Create and initialize the video outputs. */
Bool SMI_OutputPreInit(ScrnInfoPtr pScrn);

/* Output detect dummy implementation that always returns connected. */
xf86OutputStatus SMI_OutputDetect_lcd(xf86OutputPtr output);
/* Output get_modes implementation that returns the LCD native mode */
DisplayModePtr SMI_OutputGetModes_native(xf86OutputPtr output);

Bool SMI_CrtcModeFixup(xf86CrtcPtr, DisplayModePtr, DisplayModePtr);

void SMI_CrtcGammaSet(xf86CrtcPtr, CARD16*, CARD16*, CARD16*, int);
PixmapPtr SMI_CrtcShadowCreate (xf86CrtcPtr, void*, int, int);
void SMI_CrtcDestroy (xf86CrtcPtr);
void SMI_CrtcDPMS(xf86CrtcPtr, int);
Bool SMI_CrtcLock (xf86CrtcPtr);
void SMI_CrtcUnlock (xf86CrtcPtr);
void SMI_CrtcPrepare(xf86CrtcPtr);



/* smi_video.c */
int SMI_AllocateMemory(ScrnInfoPtr pScrn, void **mem_struct, int size);


#endif
