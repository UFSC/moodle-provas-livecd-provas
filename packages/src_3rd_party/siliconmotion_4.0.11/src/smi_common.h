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
authorization from the XFree86 Project and Silicon Motion.
*/

#ifndef  SMI_COMMON_INC
#define  SMI_COMMON_INC

#ifdef HAVE_CONFIG_H
#include	"config.h"
#endif
#include	"stdint.h"
#include	"xf86.h"
#include	"xf86_OSproc.h"
#ifndef  XSERVER_LIBPCIACCESS
#warning "no libpciaccess flag"
#include	"xf86_ansic.h"
#else      /* -----  not XSERVER_LIBPCIACCESS  ----- */

#endif     /* -----  not XSERVER_LIBPCIACCESS  ----- */

#include	"xf86PciInfo.h"
#include	"xf86Pci.h"
#include	"xf86Cursor.h"
#include	"vgaHW.h"
#include	"compiler.h"
#include	"mipointer.h"
#include	"micmap.h"
#include	"fb.h"
//#include	"xaa.h"
#include	"xf86cmap.h"
#include	"xf86i2c.h"
#include	"xf86int10.h"
#include	"vbe.h"
#include	"xf86xv.h"
#include	<X11/extensions/Xv.h>
#ifdef  RANDR
#include	<X11/extensions/randr.h>
#endif
#include "xf86fbman.h"
//#include "xf86Rename.h"
#define SMI_DEBUG 0

#ifndef XORG_VERSION_CURRENT
#warning "building xfree86"
#ifdef XF86_VERSION_CURRENT
#define XORG_VERSION_CURRENT XF86_VERSION_CURRENT
#endif
#endif



#ifndef XORG_VERSION_NUMERIC
#define XORG_VERSION_NUMERIC(major,minor,patch,snap,dummy) \
		(((major)*10000000) + ((minor)*100000) + ((patch)*1000) +snap)
#endif

/* xorgVersion.h contained in xorg6.9 and new xserver 
 *  * so xserver 1.5,1.6,1.7,1.8 and
 *   * xorg6.8,6.9 should contain this header file
 *    * but xfree86 should ignore it
 *     * */
#if (XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(2,0,0,0,0)) || \
	(XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,8,0,0,0))
#include "xorgVersion.h"
#endif
#include "compat-api.h"

/* xorg-server.h contained only in new xserver
 *  * so it gonna not contained by xfree86 and xorg6.9
 *   * */
#if (XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(4,3,0,0,0))
#include "xorg-server.h"
#endif


#if (XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)) || \
	(XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(4,3,0,0,0)) 
#include "xf86Resources.h"
#include "xf86RAC.h"
#endif


#define INT     int
#define LONG    int
#define DWORD   unsigned int
#define ULONG   unsigned int
#define FLONG   unsigned int
#define VOID    void
#define PUCHAR  unsigned char *
#define UCHAR   unsigned char

#define MAXLOOP 0x100000	/* timeout value for engine waits */

#define PCI_VENDOR_SMI 0x126f
#define PCI_CHIP_SMI712 0x0712
#define PCI_CHIP_SMI720 0x0720
#define PCI_CHIP_SMI501 0x0501
#define PCI_CHIP_SMI750 0x0750
#define PCI_CHIP_SMI718 0x0718

#define PCI_SMI_VENDOR_ID 		PCI_VENDOR_SMI
#define SMI_712 				PCI_CHIP_SMI712
#define SMI_722 				PCI_CHIP_SMI720
#define SMI_502 				PCI_CHIP_SMI501
#define SMI_750           		PCI_CHIP_SMI750
#define SMI_718           		PCI_CHIP_SMI718

/* Chip tags */
#define PCI_SMI_VENDOR_ID	PCI_VENDOR_SMI
#define SMI_UNKNOWN			0
#define SMI_LYNX			PCI_CHIP_SMI910
#define SMI_LYNXE			PCI_CHIP_SMI810
#define SMI_LYNX3D			PCI_CHIP_SMI820
#define SMI_LYNXEM			PCI_CHIP_SMI710
#define SMI_LYNXEMplus		PCI_CHIP_SMI712
#define SMI_LYNX3DM			PCI_CHIP_SMI720
#define SMI_COUGAR3DR       PCI_CHIP_SMI731
#define SMI_MSOC			PCI_CHIP_SMI501
#define SMI_MSOCE           PCI_CHIP_SMI750
#define SMI_SM718           PCI_CHIP_SMI718

#define SMI_VGA_SERIES(chip)  ((chip & 0xff00) == 0x0700)
#define SMI_NEWLYNX(chip) (chip == 0x0750 || chip == 0x0718)
#define SMI_LYNX_SERIES(chip)	((chip & 0xF0F0) == 0x0010)
#define SMI_LYNX3D_SERIES(chip)	((chip & 0xF0F0) == 0x0020)
#define SMI_COUGAR_SERIES(chip)	((chip & 0xF0F0) == 0x0030)
#define SMI_LYNXEM_SERIES(chip) ((chip & 0xFFF0) == 0x0710 && (chip != 0x718))
/*#define SMI_LYNXM_SERIES(chip)  ((chip & 0xff00) == 0x700 && !SMI_NEWLYNX(chip))*/
#define SMI_LYNXM_SERIES(chip) ((chip & 0xff00) == 0x700 && chip != 0x718 && chip != 0x750)
#define SMI_OLDLYNX(chip) ((chip & 0xF00) == 0x0700 && chip != 0x718 && chip != 0x750)
#define SMI_MSOC_SERIES(chip)   ((chip & 0xFF00) == 0x0500)


#define KB(X) ((X)<<10)
#define MB(X) ((X)<<20)

/*If define this flag, the driver will check  
  the "Drawing Engine FIFO Empty Status" when drawing.
  Note:
  1. This status is removed from the newest SM712 chip.
  2. We shouldn't check this status when running on the server, 
  it will hang the system.
 */
#define SMI_CHECK_2D_FIFO 		1
#define MAX_ENTITIES			16
#define MAX_ENT_IDX 			16
#define SMI_USE_IMAGE_WRITES	0
#define SMI_USE_VIDEO			1
#define SMI_USE_CAPTURE			0
/* Definitions End*/
/* Macros Start*/
#define XMSG(...) xf86DrvMsg(pScrn->scrnIndex, X_NOTICE,": "__VA_ARGS__)
#define XERR(...) xf86DrvMsg(pScrn->scrnIndex, X_ERROR,": "__VA_ARGS__)
#define XCONF(...) xf86DrvMsg(pScrn->scrnIndex, X_CONFIG,": "__VA_ARGS__)
#define XINF(...) xf86DrvMsg(pScrn->scrnIndex, X_INFO,": "__VA_ARGS__)


/* Internal macros  */
#define _F_START(f)             (0 ? f)
#define _F_END(f)               (1 ? f)
#define _F_SIZE(f)              (1 + _F_END(f) - _F_START(f))
#define _F_MASK(f)              (((1 << _F_SIZE(f)) - 1) << _F_START(f))
#define _F_NORMALIZE(v, f)      (((v) & _F_MASK(f)) >> _F_START(f))
#define _F_DENORMALIZE(v, f)    (((v) << _F_START(f)) & _F_MASK(f))

/* Global macros  */
#define FIELD_GET(x, reg, field) \
    ( \
	      _F_NORMALIZE((x), reg ## _ ## field) \
		      )

#define FIELD_SET(x, reg, field, value) \
    ( \
	      (x & ~_F_MASK(reg ## _ ## field)) \
		        | _F_DENORMALIZE(reg ## _ ## field ## _ ## value, reg ## _ ## field) \
				    )

#define FIELD_VALUE(x, reg, field, value) \
    ( \
	      (x & ~_F_MASK(reg ## _ ## field)) \
		        | _F_DENORMALIZE(value, reg ## _ ## field) \
				    )

#define FIELD_CLEAR(reg, field) \
    ( \
	      ~ _F_MASK(reg ## _ ## field) \
		      )

#define FIELD_START(field) (0 ? field)
#define FIELD_END(field) (1 ? field)
#define FIELD_SIZE(field) (1 + FIELD_END(field) - FIELD_START(field))
#define FIELD_MASK(field) (((1 << (FIELD_SIZE(field)-1)) | ((1 << \
                    (FIELD_SIZE(field)-1)) - 1)) << FIELD_START(field))

#define FIELD_NORMALIZE(reg, field) (((reg) & FIELD_MASK(field)) >> \
        FIELD_START(field))

#define FIELD_DENORMALIZE(field, value) (((value) << FIELD_START(field)) & \
        FIELD_MASK(field))

#define FIELD_INIT(reg, field, value)   FIELD_DENORMALIZE(reg ## _ ## field, \
        reg ## _ ## field ##\
		        _ ## value)

#define FIELD_INIT_VAL(reg, field, value) \
    (FIELD_DENORMALIZE(reg ## _ ## field, value))

#define FIELD_VAL_SET(x, r, f, v)       x = x & ~FIELD_MASK(r ## _ ## f) \
                                            | FIELD_DENORMALIZE(r ## _ ## f,\
												r ## _ ## f ## _ ## v)


/* Display type constants to use with setMode function and others. */
typedef enum _display_t
{
	PANEL = 0,
	CRT = 1,
	VGA = 2
}
display_t;

/* Panel On/Off constants to use with panelPowerSequence. */
typedef enum _panel_state_t
{
	PANEL_OFF,
	PANEL_ON
}
panel_state_t;

/* Polarity constants. */
typedef enum _polarity_t
{
	POSITIVE,
	NEGATIVE
}
polarity_t;

typedef enum _channel_t
{
    PRIMARY_CTRL = 0,
    SECONDARY_CTRL = 1,
}channel_t;

/* RGB color structure. */
typedef struct {
	UCHAR cBlue;
	UCHAR cGreen;
	UCHAR cRed;
	UCHAR cFiller;
} RGB;

typedef enum _TFT_color_t
{
    SMI_TFT_9BIT=0,
    SMI_TFT_12BIT,
    SMI_TFT_18BIT,
    SMI_TFT_24BIT,
    SMI_TFT_36BIT
}
TFT_color_t;

typedef enum _SMI_DPMS_t
{
	SMI_DPMS_ON,
	SMI_DPMS_STANDBY,
	SMI_DPMS_SUSPEND,
	SMI_DPMS_OFF
}
SMI_DPMS_t;

/* Format of mode table record */
typedef struct _mode_table_t
{
	/* Horizontal timing */
	int horizontal_total;
	int horizontal_display_end;
	int horizontal_sync_start;
	int horizontal_sync_width;
	polarity_t horizontal_sync_polarity;

	/* Vertical timing. */
	int vertical_total;
	int vertical_display_end;
	int vertical_sync_start;
	int vertical_sync_height;
	polarity_t vertical_sync_polarity;

	/* Refresh timing. */
	int pixel_clock;
	int horizontal_frequency;
	int vertical_frequency;

	/* Programe PLL3 */
	int M;
	int N;
	int bit15;
	int bit31;
}
mode_table_t, *pmode_table_t;

/* Clock value structure. */
typedef struct clock_select_t
{
	long mclk;
	long test_clock;
	int divider;
	int shift;

	long multipleM;
	int dividerN;
	short divby2;

}
clock_select_t, *pclock_select_t;

/* Registers necessary to set mode. */
typedef struct _reg_table_t
{
	DWORD clock;
	DWORD control;
	DWORD fb_width;
	DWORD horizontal_total;
	DWORD horizontal_sync;
	DWORD vertical_total;
	DWORD vertical_sync;
	DWORD width;
	DWORD height;
	display_t display;
}
reg_table_t, *preg_table_t;


/* 
 * driver defined PROC 
 */
typedef void (*SmiHwProc)(pointer pHw);
typedef void (*SmiProc)(ScrnInfoPtr pScrn);
typedef int (*SmiRetProc)(ScrnInfoPtr pScrn);
typedef ModeStatus (*SmiModeProc)(ScrnInfoPtr pScrn,DisplayModePtr mode);
typedef void (*SmiValueProc)(ScrnInfoPtr pScrn,int value);	
typedef void (*SmiPaletteProc)(ScrnInfoPtr pScrn, int numColors, int *indicies, LOCO * colors, VisualPtr pVisual);
typedef int (*SmiGetProc)(pointer pHw);
typedef void (*SmiVideoProc)(ScrnInfoPtr	pScrn);

/*-----------------------------------------------------------------------------
 *  all regular and common hardware related stuffs put into here
 *  one SMIHWPtr may be referenced by two SMIPtr (dualview case)
 *  
 *  SMIHWRec will be embedded in chip specific SMIxxxHWRec
 *  such as:
 *  typedef struct{
 *  	SMIHWRec baseHwRec;
 *  	...
 *  	...
 *  }SMI750hwRec,*SMI750hwPtr;
 *
 *  EntityProc is a funtion protoype defined in xf86str.h
 *
 *-----------------------------------------------------------------------------*/
typedef struct
{
	/* Chip info, set using PCI above */
	int			Chipset;	
	int			ChipRev;
	
	/* hardware attributes */
#ifdef XSERVER_LIBPCIACCESS
	pciaddr_t 	phyaddr_reg;/* don't forget 712 get another mmio(for vgaIO),so we use reg to define 
							the physica register address */
	pciaddr_t 	physize_reg;
	pciaddr_t 	phyaddr_mem;
	pciaddr_t 	physize_mem;
	
	struct pci_device *pPci;
#else
	ADDRESS 	phyaddr_reg;
	uint32_t 	physize_reg;
	ADDRESS 	phyaddr_mem;
	uint32_t 	physize_mem;

	pciVideoPtr pPci;
#endif

    IOADDRESS	PIOBase;	/* Base of I/O ports */

    CARD8 *		IOBase;		/* Base of MMIO VGA ports */
    CARD8 *		DPRBase;	/* Base of DPR registers */
    CARD8 *		VPRBase;	/* Base of VPR registers */
    CARD8 *		CPRBase;	/* Base of CPR registers */
    CARD8 *		FPRBase;    /* Base of FPR registers - for 0730 chipset */
    CARD8 *		DCRBase;    /* Base of DCR registers - for 0501 chipset */
    CARD8 *		SCRBase;    /* Base of SCR registers - for 0501 chipset */	
    CARD8 *		DataPortBase;	/* Base of data port */
    int			DataPortSize;	/* Size of data port */
	
	volatile void * pReg;/* mapped registers virtual address */
	void * 			pMem; /* mapped videomemory virtual address */
	int 			dual;
	uint16_t 		devId;/*This member save 'pPci->device'*/
	uint8_t 		revId;
	EntityInfoPtr	pEnt_info;

	void *  		primary_screen_rec;

	SmiHwProc 	pcDeepMap;/* can be zero pointer */	
	EntityProc 	pcEntityInit;/* all chip init code here */
	EntityProc 	pcEntityEnter;/* save console's registers, etc.. */
	EntityProc 	pcEntityLeave;/* restore console's registers,etc...*/
	SmiHwProc 	pcCloseAllScreen;
	SmiGetProc 	pcFBSize;
	SmiHwProc   pcInitHardware;
    SmiVideoProc pcVideoReset;
}SMIHWRec,*SMIHWPtr;

/*-----------------------------------------------------------------------------
 *  all regular and common stuffs put into here
 *
 *  SMIRec will be embedded in chip specific SMIxxxRec 
 *  such as
 *  typedef struct{
 *  	SMIRec baseRec;
 *  	...
 *  	...
 *  }SMI750Rec,*SMI750Ptr;
 *-----------------------------------------------------------------------------*/
typedef struct
{
	/* currrent screen briefs */
	int 			width;
	int 			height;
	int 			Bpp;
	int				Stride;		/* Stride of frame buffer,in bytes */
	Bool			IsSecondary;/* second Screen */
	Bool			IsPrimary;  /* first Screen */
	Bool			IsLCD;
	Bool			IsCRT;
	ScrnInfoPtr		screen_info;

	/*frame buffer special*/
/****************************
pMem: 		the beginning of the entire video memory address
pFB: 		the fb start address of current screen
			it will equal with 'pMem' when SIMUL (Not Dual View)
FBOffset:	the interval between 'pFB' to 'pMem'
			it will equal with 'videoRAMBytes' x 'screen index'
****************************/
	uint32_t 		FBOffset;	/*FBOffset: offset of videomemory of this screen,usually zero
	while 4mb(for sm502) if dualview*/
	void * 			pFB;			/*start address of frame buffer of current screen*/
	int 			videoRAMBytes;	/* frame buffer size for one screen.
	full video memory for SIMUL case,and half for dualview*/
	uint32_t 		FBReserved;/* An offset in framebuffer for reserved memory in frame buffer for XAA to use*/

	int screen;/* 0 means primary channel (DVI) and 1 secondary (VGA)*/
	/*x will malloc ScrnInfoRec for every screen.
	In Probe, it will assign an index for every SMIRec.
	When driver get the index as 0, it will handle primary channel 
	When driver get the index as 1, it will handle secondary channel */
	
	/*clock*/
	int				MCLK;		/* Memory Clock  */
	/* DPMS */
    int				CurrentDPMS;	/* Current DPMS state */
	/* members about Options */
	OptionInfoPtr 	Options;
	Bool			NoAccel;	/* Disable Acceleration */
	Bool			NoPCIRetry;	/* Disable PCI retries */
	int 			LoadModules;
	int 			pci_burst;	/* Enable PCI burst mode for reads? */
	
	/* limitations */
	ClockRangePtr 	clockRanges;
	int				lcdWidth;	/* LCD width */
    int				lcdHeight;	/* LCD height */
	int * 			linePitches;
	int 			minPitch;	/* min line pitch (width) */
	int 			maxPitch;	/* max line pitch (width) */
	int 			pitchInc;	/* bits of granularity for line pitch */
	/* (width) above */
	int 			minHeight;	/* min virtual height */
	int 			maxHeight;	/* max virtual height */
	int				TFTColor;/* for rec tft color*/

	/* references */
	SMIHWPtr 		pHardware;	/* two SMIPtr can link to one share*//*by ilena: 502 does not use this option*/
	xf86Int10InfoPtr pInt10; 	/* Pointer to INT10 module */
	vbeInfoPtr 		pVbe;		/*by ilena: 502 does not use this option*/
	
	/* functions */
	CloseScreenProcPtr CloseScreen; /* pointer used to save wrapped closescreen func*/
	/* Accel Additions */
//	XAAInfoRecPtr	AccelInfoRec;	/* XAA info Rec */
	CARD32			AccelCmd;		/* Value for DPR0C */
	CARD32			ScissorsLeft;	/* Left/top of current scissors */
	CARD32			ScissorsRight;	/* Right/bottom of current scissors */
	Bool			ClipTurnedOn;	/* Clipping was turned on by the previous command */
	int				GEResetCnt;		/* Limit the number of errors printed using a counter */
	SmiHwProc		Accelwaitfornotbusy;
		
	/* Polylines*/
//	ValidateGCProcPtr	ValidatePolylines;	/* Org. ValidatePolylines function */
	Bool			polyLines;		/* Our polylines patch is active */
	/* routine belong to per-screen */

    Bool			HwCursor;	/* hardware cursor enabled */
	Bool			ARGBCursor;	/*	ARGB cursor enabled only when DualView == false && hwcursor == true	*/

	int				videoKey;	/* Video chroma key */
	Bool			interlaced;	/* True: Interlaced Video */

	/* XvExtension */
	XF86VideoAdaptorPtr	ptrAdaptor;	/* Pointer to VideoAdapter
						   structure */
	void (*BlockHandler)(int i, pointer blockData, pointer pTimeout,
						 pointer pReadMask);	
	Bool 			IsCSCVideo;

        /* though each entity only map once, but each screen need
	 * to take map result like framebuffer virtual address (plus an offset,if dual)
	 * into its SMIRec,and copy some value like phyaddr_mem and videoRAMKBytes
	 * into pScrn structure.....
	 * so we need this *psGetMapResult*
	 * */   /*by ilena: 502 does not use these option, need to add*/
	SmiProc 		psGetMapResult;
	SmiValueProc	psVgaPciInit;
	SmiProc 		psHandleOptions;
	SmiModeProc 	psValidMode;
	SmiModeProc 	psSetMode;
	SmiValueProc 	psAdjustFrame; 
	SmiPaletteProc 	psLoadPalette;
	SmiModeProc 	psSetDisplay;
	SmiProc 		psSaveText;
#if SMI_RANDR
	SmiRetProc		psCrtcPreInit;
	SmiRetProc		psOutputPreInit;
	SmiProc			psI2CInit;
	I2CBusPtr		I2C_primary;		/* Pointer into I2C_primary module ,eg sm712 crt iic */
	I2CBusPtr       I2C_secondary;      /* Pointer to I2C secondary modules,eg sm712 panel iic*/
	Bool			DualView;
	display_t		dispCtrl;
#endif
}SMIRec,*SMIPtr;


#define SMIPTR(p) ((SMIPtr)((p)->driverPrivate))
#define	HWPSMI(p) ((SMIPtr)((p)->primary_screen_rec))
#define	SMIPSI(p) ((ScrnInfoPtr)((p)->screen_info))

LONG roundDiv (LONG num, LONG denom);

/*-----------------------------------------------------------------------------
 *  basic token,used to compose specific options
 *-----------------------------------------------------------------------------*/
typedef enum
{
	OPTION_PCI_BURST,
	OPTION_PCI_RETRY,
	OPTION_NOACCEL, //750le use
	OPTION_PANELWIDTH,
	OPTION_PANELHEIGHT,
	OPTION_CSCVIDEO,
	OPTION_EDID,
	OPTION_BAS_CNT,
	/* below is the specific options for 502*/
    OPTION_HWCURSOR, //750le use
    OPTION_VIDEOKEY,
    OPTION_BYTESWAP,
    OPTION_INTERLACED,
	/* for 718,750 */

	/* for 712,722*/
    OPTION_TFT_TYPE,
    OPTION_XLCD,
    OPTION_YLCD,
    OPTION_MCLK,
    OPTION_DUALVIEW,
} SMIOpts;


#define WRITE_DPR(pHw, dpr, data)	MMIO_OUT32(pHw->DPRBase, dpr, data); 
#define READ_DPR(pHw, dpr)			MMIO_IN32(pHw->DPRBase, dpr)
#define WRITE_VPR(pHw, vpr, data)	MMIO_OUT32(pHw->VPRBase, vpr, data); 
#define READ_VPR(pHw, vpr)			MMIO_IN32(pHw->VPRBase, vpr)
#define WRITE_CPR(pHw, cpr, data)	MMIO_OUT32(pHw->CPRBase, cpr, data);
#define READ_CPR(pHw, cpr)			MMIO_IN32(pHw->CPRBase, cpr)
#define WRITE_FPR(pHw, fpr, data)	MMIO_OUT32(pHw->FPRBase, fpr, data); 
#define READ_FPR(pHw, fpr)			MMIO_IN32(pHw->FPRBase, fpr)
#define WRITE_DCR(pHw, dcr, data)	MMIO_OUT32(pHw->DCRBase, dcr, data);
#define READ_DCR(pHw, dcr)			MMIO_IN32(pHw->DCRBase, dcr)
#define WRITE_SCR(pHw, scr, data)	MMIO_OUT32(pHw->SCRBase, scr, data); 
#define READ_SCR(pHw, scr)			MMIO_IN32(pHw->SCRBase, scr)

/* Needs refinement */

#define SMI_TRANSPARENT_SRC		0x00000100
#define SMI_TRANSPARENT_DEST	0x00000300

#define SMI_OPAQUE_PXL			0x00000000
#define SMI_TRANSPARENT_PXL		0x00000400

#define SMI_MONO_PACK_8			0x00001000
#define SMI_MONO_PACK_16		0x00002000
#define SMI_MONO_PACK_32		0x00003000

#define SMI_ROP2_SRC			0x00008000
#define SMI_ROP2_PAT			0x0000C000
#define SMI_ROP3				0x00000000

#define SMI_BITBLT				0x00000000
#define SMI_RECT_FILL			0x00010000
#define SMI_TRAPEZOID_FILL		0x00030000
#define SMI_SHORT_STROKE    	0x00060000
#define SMI_BRESENHAM_LINE		0x00070000
#define SMI_HOSTBLT_WRITE		0x00080000
#define SMI_HOSTBLT_READ		0x00090000
#define SMI_ROTATE_BLT			0x000B0000

#define SMI_SRC_COLOR			0x00000000
#define SMI_SRC_MONOCHROME		0x00400000

#define SMI_GRAPHICS_STRETCH	0x00800000

#define SMI_ROTATE_ZERO			0x0
#define SMI_ROTATE_CW			0x01000000
#define SMI_ROTATE_CCW			0x02000000
#define SMI_ROTATE_UD			0x03000000

#define SMI_MAJOR_X				0x00000000
#define SMI_MAJOR_Y				0x04000000

#define SMI_LEFT_TO_RIGHT		0x00000000
#define SMI_RIGHT_TO_LEFT		0x08000000

#define SMI_COLOR_PATTERN		0x40000000
#define SMI_MONO_PATTERN		0x00000000

#define SMI_QUICK_START			0x10000000
#define SMI_START_ENGINE		0x80000000


#define RGB8_PSEUDO      (-1)
#define RGB16_565         0
#define RGB16_555         1
#define RGB32_888         2

/* SM501 Panel Graphics Control */
#define DCR00						0x0000
#define DCR04						0x0004
#define DCR08						0x0008
#define DCR0C						0x000C
#define DCR10						0x0010
#define DCR14						0x0014
#define DCR18						0x0018
#define DCR1C						0x001C
#define DCR20						0x0020
#define DCR24						0x0024
#define DCR28						0x0028
#define DCR2c						0x002c
#define DCR30						0x0030
#define DCR34						0x0034

/* SM 501 Video Control */
#define DCR40						0x0040
#define DCR44						0x0044
#define DCR48						0x0048
#define DCR4C						0x004C
#define DCR50						0x0050
#define DCR54						0x0054
#define DCR58						0x0058
#define DCR5C						0x005C
#define DCR60						0x0060
#define DCR64						0x0064
#define DCR68						0x0068

#define SCR00						0x0000
#define SCR04						0x0004
#define SCR08						0x0008
#define SCR0C						0x000C
#define SCR10						0x0010
#define   SCR10_LOCAL_MEM_SIZE        0x0000E000
#define   SCR10_LOCAL_MEM_SIZE_SHIFT  13
#define SCR14						0x0014
#define SCR18						0x0018
#define SCR1C						0x001C
#define SCR20						0x0020
#define SCR24						0x0024
#define SCR28						0x0028
#define SCR2C						0x002C
#define SCR30						0x0030
#define SCR34						0x0034
#define SCR38						0x0038
#define SCR3C						0x003C
#define SCR40						0x0040
#define SCR44						0x0044
#define SCR48						0x0048
#define SCR4C						0x004C
#define SCR50						0x0050
#define SCR54						0x0054
#define SCR58						0x0058
#define SCR5C						0x005C
#define SCR60						0x0060
#define SCR64						0x0064
#define SCR68						0x0068
#define SCR6C						0x006C
#endif   /* ----- #ifndef SMI_COMMON_INC  ----- */
