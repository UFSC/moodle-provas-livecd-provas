
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


#ifndef  SMI_502_INC
#define  SMI_502_INC
#include "../smi_common.h"
#include "smi_502_hw.h"
#include	"ddk502/ddk502_mode.h"

SMIPtr sm502_genSMI(pointer priv,int);

#define REG_SYS_HEAD_502    0x0
#define REG_SYS_TAIL_502    0x74
#define REG_PNL_HEAD_502    0x80000
#define REG_PNL_TAIL_502    0x80034
#define REG_VID_HEAD_502    0x80040
#define REG_VID_TAIL_502    0x80068
#define REG_PCUR_HEAD_502    0x800f0
#define REG_PCUR_TAIL_502    0x800fc
#define REG_CRT_HEAD_502    0x80200
#define REG_CRT_TAIL_502    0x80224
#define REG_CCUR_HEAD_502   0x80230
#define REG_CCUR_TAIL_502   0x8023c

/* Maximum hardware cursor dimensions */
#define SMI501_MAX_CURSOR	64
#define SMI501_CURSOR_SIZE	1024

#define SMI502_CURSOR_ALPHA_PLANE	0
#if SMI502_CURSOR_ALPHA_PLANE
/* Stored in either 4:4:4:4 or 5:6:5 format */
# define SMI501_ARGB_CURSOR_SIZE					\
    (SMI501_MAX_CURSOR * SMI501_MAX_CURSOR * 2)
#else    	
#define SMI501_ARGB_CURSOR_SIZE 		0
#endif

/* Driver data structure; this should contain all needed info for a mode */
typedef struct
{    
    Bool    modeInit;
    CARD16	mode;

    CARD32 System[(REG_SYS_TAIL_502 - REG_SYS_HEAD_502)/4+1];    //0x00 -- 0x74
    CARD32 PanelControl[(REG_PNL_TAIL_502 - REG_PNL_HEAD_502)/4 +1];    //0x80000 -- 0x80034
    CARD32 VIDEOControl[(REG_VID_TAIL_502 - REG_VID_HEAD_502)/4+1];     //0x80040 -- 0x80068
    CARD32 PanelCursorControl[(REG_PCUR_TAIL_502 - REG_PCUR_HEAD_502)/4+1];  
    CARD32 CRTControl[(REG_CRT_TAIL_502 - REG_CRT_HEAD_502)/4 + 1];     //0x80200 -- 0x802284
    CARD32 CRTCursorControl[(REG_CCUR_TAIL_502 - REG_CCUR_HEAD_502)/4+1];
  
    /* entity information */
    Bool DualHead;
    ScrnInfoPtr pSecondaryScrn;
    ScrnInfoPtr pPrimaryScrn;
    int		lastInstance;

    /* shared resource */
    int mmio_require;
    volatile unsigned char * MMIOBase;		/* Base of MMIO */
    int MapSize;	/* how many mmio should map and unmap */
    int total_videoRam;			/* memory count in bytes */
}SMI502_RegRec,*SMI502_RegPtr;


typedef struct {
	/* put common structure here */
	SMIHWRec base;
	/* put chip dependent stuffs here*/
	SMI502_RegPtr pRegSave;
	/* vga stuffs */
	//char * fonts;
}SMI502HWRec,*SMI502HWPtr;



typedef struct {
	/* we best put base in header of structure*/
	SMIRec base;

    /* chip dependent stuffs of this pSmi*/
	CARD32			AccelCmd;	/* Value for DPR0C */

    CARD32			ScissorsLeft;	/* Left/top of current
                                                   scissors */
    CARD32			ScissorsRight;	/* Right/bottom of current
                                                   scissors */
    Bool			ClipTurnedOn;	/* Clipping was turned on by
                                                   the previous command */
    xf86CursorInfoPtr	        CursorInfoRec;	/* HW Cursor info */

    /* XAA */
    int			        MapSize;	/* Size of mapped memory */
    /* below resource should get directly from entity but no mmap */
    unsigned char *		MapBase;	/* Base of mapped io */
    /*be monk*/

    CARD32			FBCursorOffset;	/* Cursor storage location */

    Bool			PrimaryVidMapped;	/* Flag indicating if
                        vgaHWMapMem was used successfully for this screen */
    int			        GEResetCnt;	/* Limit the number of errors
                                                   printed using a counter */
    Bool			NoPCIRetry;	/* Disable PCI retries */
    Bool			hwcursor;	/* hardware cursor enabled */
    Bool			ShowCache;	/* Debugging option */
    Bool			edid_enable;

//    XAAInfoRecPtr		AccelInfoRec;	/* XAA info Rec */

    /* Panel information */
    Bool			lcd;		/* LCD active, 1=TFT, 2=DSTN */

    I2CBusPtr		        I2C;		/* Pointer into I2C module */
    I2CBusPtr		        I2C_primary;	
    I2CBusPtr		        I2C_secondary;	
   
    /* Shadow frame buffer (rotation) */
    Bool			shadowFB;	/* Flag if shadow buffer is
                                                   used */
    Bool			clone_mode;	/* Flag if clone mode is
                                                   used */
    int	        		rotate;		/* Rotation flags */
    Bool	        	RandRRotation;
    int		        	ShadowPitch;	/* Pitch of shadow buffer */
    int		        	ShadowWidthBytes;	/* Width of shadow
                                                           buffer in bytes */
    int			        ShadowWidth;	/* Width of shadow buffer in
                                                   pixels */
    int		        	ShadowHeight;	/* Height of shadow buffer in
                                                   pixels */
    CARD32			saveBufferSize;	/* #670 - FB save buffer size */
    void *			pSaveBuffer;	/* #670 - FB save buffer */
    CARD32			savedFBOffset;	/* #670 - Saved FBOffset value */
    CARD32			savedFBReserved;	/* #670 - Saved
                                                           FBReserved value */
    CARD8 *			paletteBuffer;	/* #920 - Palette save buffer */
    /* Polylines - #671 */
    //ValidateGCProcPtr	        ValidatePolylines;	
    /* Org.
                                     ValidatePolylines   function */
    Bool			polyLines;	/* Our polylines patch is
                                                   active */
    void (*PointerMoved)(int index, int x, int y);
    int			        videoKey;	/* Video chroma key */
    Bool			ByteSwap;	/* Byte swap for ZV port */
    Bool			interlaced;	/* True: Interlaced Video */

    Bool	            	IsLCD;
    Bool			IsCRT;
    /*For CSC Video*/
	CARD32      fb_priCursorOffset;
    CARD32      fb_secCursorOffset; 
    CARD32      fb_argbCursorOffset;
}SMI502_Rec,*SMI502_Ptr;

void sm502_pcDeepmap(SMIHWPtr pHw);
void sm502_entityInit(int,pointer);
void sm502_entityEnter(int,pointer);
void sm502_entityLeave(int,pointer);
void sm502_getMapResult(ScrnInfoPtr);
void sm502_vgaAndpciInit(ScrnInfoPtr pScrn,int entityIndex);
void sm502_handleOptions(ScrnInfoPtr);
ModeStatus sm502_validMode(ScrnInfoPtr,DisplayModePtr);
void sm502_setMode(ScrnInfoPtr,DisplayModePtr);
void sm502_adjustFrame(ScrnInfoPtr,int);
void sm502_setDisplay(ScrnInfoPtr pScrn, DisplayModePtr mode);
void sm502_pcDeepmap(SMIHWPtr pHw);
void sm502_saveText(ScrnInfoPtr pScrn);
void sm502_LoadPalette (ScrnInfoPtr , int , int *, LOCO * , VisualPtr );
void sm502_closeAllScreen(SMIHWPtr pHw);
int sm502_totalFB();
extern int ddk502_getFrameBufSize();
extern long ddk502_initHw();
extern unsigned long ddk502_PEEK32(addr);
extern void ddk502_POKE32(addr,data);
extern void ddk502_POKEfb32(addr,data);
extern void ddk502_set_mmio(volatile unsigned char * addr,unsigned short devId,unsigned long revId);
extern void ddk502_set_fb(volatile unsigned char * addr,unsigned short devId,char revId);
extern long ddk502_setModeTiming(logicalMode_t *pLogicalMode);
extern Bool SMI502_CrtcPreInit(ScrnInfoPtr pScrn);
extern Bool SMI502_OutputPreInit(ScrnInfoPtr pScrn);
extern void setLogicalDispOutput(disp_output_t output);
void sm502_I2CInit(ScrnInfoPtr pScrn);

#ifndef _SMI_FUNCS_ONLY
#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,7,0,0,0)
#include <X11/extensions/dpmsconst.h>
#else
#define DPMS_SERVER
#include  <X11/extensions/dpms.h>
#endif

/* Definitions Start*/
#define VSYNCTIMEOUT                       10000
/* Definitions End*/

/* Macros Start*/
#define SMI502_PTR(p) ((SMI502_Ptr)((p)->driverPrivate))
/* Macros End*/
#endif

/* Function Prototypes Start*/

/* smi502_shadow.c */
void SMI502_PointerMoved(int index, int x, int y);
void SMI502_RefreshArea(ScrnInfoPtr pScrn, int num, BoxPtr pbox);

/* Function Prototypes End*/

#ifndef _SMI_FUNCS_ONLY
/* Registers Start */

#define CURRENT_POWER_CLOCK_P1XCLK               31:31
#define CURRENT_POWER_CLOCK_P1XCLK_ENABLE               1
#define CURRENT_POWER_CLOCK_P1XCLK_DISABLE              0
#define CURRENT_POWER_CLOCK_PLLCLK_SELECT               30:30
#define CURRENT_POWER_CLOCK_PLLCLK_SELECT_ENABLE               1
#define CURRENT_POWER_CLOCK_PLLCLK_SELECT_DISABLE              0
#define CURRENT_POWER_CLOCK_V1XCLK		               21:21
#define CURRENT_POWER_CLOCK_V1XCLK_ENABLE               1
#define CURRENT_POWER_CLOCK_V1XCLK_DISABLE              0
#define POWER_MODE0_CLOCK_PLL3_P1XCLK			31:31
#define POWER_MODE0_CLOCK_PLL3_P1XCLK_ENABLE		1
#define POWER_MODE0_CLOCK_PLL3_P1XCLK_DISABLE		0
#define POWER_MODE0_CLOCK_PLL3				30:30
#define POWER_MODE0_CLOCK_PLL3_ENABLE			0
#define POWER_MODE0_CLOCK_PLL3_DISABLE			1

/* CRT Cursor Control */

#define CRT_PALETTE_RAM                                 0x080400
#define PANEL_PALETTE_RAM                               0x080800
#define VIDEO_PALETTE_RAM                               0x080C00

#define SYSTEM_PLL3_CLOCK				0x000074
#define SYSTEM_PLL3_CLOCK_M				7:0
#define SYSTEM_PLL3_CLOCK_N				14:8
#define SYSTEM_PLL3_CLOCK_DIVIDE			15:15
#define SYSTEM_PLL3_CLOCK_DIVIDE_1			0
#define SYSTEM_PLL3_CLOCK_DIVIDE_2			1
#define SYSTEM_PLL3_CLOCK_INPUT				16:16
#define SYSTEM_PLL3_CLOCK_INPUT_CRYSTAL			0
#define SYSTEM_PLL3_CLOCK_INPUT_TEST			1
#define SYSTEM_PLL3_CLOCK_POWER				17:17
#define SYSTEM_PLL3_CLOCK_POWER_OFF			0
#define SYSTEM_PLL3_CLOCK_POWER_ON			1

#define CURRENT_POWER_PLLCLOCK                          0x000074
#define CURRENT_POWER_PLLCLOCK_TEST_OUTPUT            	20:20
#define CURRENT_POWER_PLLCLOCK_TEST_OUTPUT_ENABLE	1
#define CURRENT_POWER_PLLCLOCK_TEST_OUTPUT_DISABLE     	0
#define CURRENT_POWER_PLLCLOCK_TESTMODE              	19:18
#define CURRENT_POWER_PLLCLOCK_TESTMODE_ENABLE          1
#define CURRENT_POWER_PLLCLOCK_TESTMODE_DISABLE         0
#define CURRENT_POWER_PLLCLOCK_POWER              	17:17
#define CURRENT_POWER_PLLCLOCK_POWER_DOWN              	0
#define CURRENT_POWER_PLLCLOCK_POWER_ON              	1
#define CURRENT_POWER_PLLCLOCK_INPUT_SELECT             16:16
#define CURRENT_POWER_PLLCLOCK_INPUT_SELECT_TEST        1
#define CURRENT_POWER_PLLCLOCK_INPUT_SELECT_CRYSTAL     0
#define CURRENT_POWER_PLLCLOCK_DIVIDEBY2                15:15
#define CURRENT_POWER_PLLCLOCK_DIVIDE_N                 14:8
#define CURRENT_POWER_PLLCLOCK_MULTIPLE_M               7:0

/* Registers End */

/* SM501 Video Alpha Control */
#define DCR80						0x0080
#define DCR84						0x0084
#define DCR88						0x0088
#define DCR8C						0x008C
#define DCR90						0x0090
#define DCR94						0x0094
#define DCR98						0x0098
#define DCR9C						0x009C
#define DCRA0						0x00A0
#define DCRA4						0x00A4

/* SM501 Panel Cursor Control */
#define DCRF0						0x00F0
#define DCRF4						0x00F4
#define DCRF8						0x00F8
#define DCRFC						0x00FC

/* SM 501 Alpha Control */
#define DCR100						0x0100
#define DCR104						0x0104
#define DCR108						0x0108
#define DCR10C						0x010C
#define DCR110						0x0110
#define DCR114						0x0114
#define DCR118						0x0118

/* SM 501 CRT Graphics Control */
#define DCR200						0x0200
#define   DCR200_CRT_BLANK            0x00000400
#define   DCR200_CRT_GRAPHICS_PLN_FMT 0x00000003
#define     CRT_GRAPHICS_PLN_FMT_8      0x00
#define     CRT_GRAPHICS_PLN_FMT_16     0x01
#define     CRT_GRAPHICS_PLN_FMT_32     0x10
#define DCR204						0x0204
#define DCR208						0x0208
#define DCR20C						0x020C
#define DCR210						0x0210
#define DCR214						0x0214
#define DCR218						0x0218
#define DCR21C						0x021C
#define DCR220						0x0220
#define DCR224						0x0224

/* SM 501 CRT Cursor Control */
#define DCR230						0x0230
#define DCR234						0x0234
#define DCR238						0x0238
#define DCR23C						0x023C

/* SM 501 Palette Ram */
#define DCR400						0x0400      /* Panel */
#define DCR800						0x0800      /* Video */
#define DCRC00						0x0C00      /* CRT   */


#endif
#endif   /* ----- #ifndef SMI_502_INC  ----- */
