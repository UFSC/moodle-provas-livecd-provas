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
#ifndef  SMI_ACCEL_INC
#define  SMI_ACCEL_INC
#include "ddk750/ddk750_reg.h"
#include "drv750le/smi_750le_driver.h"

/* Drawing Engine Control Registers */
#define DPR10 0x10	/* DPR10: Source Row Pitch */
#define DPR12 0x12	/* DPR12: Destination Row Pitch */
#define DPR3C 0x3C	/* DPR3C: XY Addressing Destination & Source Window Widths */

/* 2D Engine commands */
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


#define ENGINE_IDLE_501()												\
     ((READ_SCR(pHw, SCR00) & 0x00080000) == 0) 						 /*For SM502*/	

#define FIFO_EMPTY_501()												\
     ((READ_SCR(pHw, SCR00) & 0x00100000) != 0) 						 /*For SM502*/	

#define ENGINE_IDLE_712()												\
     ((VGAIN8_INDEX_712(pSmi->pHardware, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x16) & 0x08) == 0) 	 /*For SM712 and SM722*/	

#define FIFO_EMPTY_712()												\
     ((VGAIN8_INDEX_712(pSmi->pHardware, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x16) & 0x10) != 0) 	 /*For SM712 and SM722*/

#define ENGINE_IDLE_750()												\
     ((READ_SCR(pHw, SCR00) & 0x00400000) == 0) 						 /*For SM718 and SM750 and SM750LE*/

#define FIFO_EMPTY_750()												\
     ((READ_SCR(pSmi->pHardware, SCR00) & 0x00800000) != 0) 			 /*For SM718 and SM750 and SM750LE*/
/*Wait until "v" queue entries are free */
/*Note, we shouldn't wait the "Drawing Engine FIFO Empty Status" 
	when the driver is running on server. 
  If we wait the status, the system will hang.*/
#ifdef SMI_CHECK_2D_FIFO
#define	WaitQueue(v)							   						\
do								   										\
{								   										\
    if(pHw->revId == 0xfe) 												\	
		SMI750LE_WaitForNotBusy(pSmi); 									\
    else																\
	{                   												\
	    if (pSmi->NoPCIRetry)				  	   						\
	    {						  	   									\
	        int loop = MAXLOOP; mem_barrier();		   					\
			if(pHw->Chipset == SMI_MSOCE || pHw->Chipset == SMI_SM718)  \
	        {						              						\
				ddk750_set_mmio(pHw->pReg,pHw->devId,pHw->revId);  		\
	            while (!FIFO_EMPTY_750())		  						\
	            {														\
	                if (loop-- == 0) break;		   						\
	            }														\
	        }															\
	        else if (pHw->Chipset == SMI_LYNXEMplus || pHw->Chipset == SMI_LYNX3DM)	\
	        {															\
	            while (!FIFO_EMPTY_712())			  					\
	            {														\
	                if (loop-- == 0) break;			  					\
	            }														\
	        } 															\
	        else														\
	        {															\
	            while (!FIFO_EMPTY_501())			  					\
	            {														\
	                if (loop-- == 0) break;			  					\
	            }														\
	        } 															\
	        if (loop <= 0)  SMI_GEReset(pScrn, 1, __LINE__, __FILE__);	\
	    }																\	
	}						   											\
} while (0) 
#else
#define	WaitQueue(v)			\
do								\
{								\
} while (0)			
#endif

/* Wait until GP is idle */
#define WaitIdle()								  						\
    do									   								\
{									   									\
    int loop = MAXLOOP; mem_barrier();                                 	\
	if(SMI_NEWLYNX(pHw->devId)) 										\
    {																	\
        ddk750_set_mmio(pHw->pReg,pHw->devId,pHw->revId);  				\
        while (!ENGINE_IDLE_750())				  						\
        {																\
            if (loop-- == 0) break;				  						\
        }                                                    			\
    }																	\
    else if (SMI_OLDLYNX(pHw->devId)) 									\
    {																	\
        while (!ENGINE_IDLE_712())				  						\
        {																\
            if (loop-- == 0) break;				  						\
        }																\
    }   																\
    else 																\
    {																	\
        while (!ENGINE_IDLE_501())				  						\
        {																\
            if (loop-- == 0) break;				  						\
        }																\
    }   																\
    if (loop <= 0) SMI_GEReset(pScrn, 1, __LINE__, __FILE__);           \
}while (0)                                                                                                



/* Wait until GP is idle and queue is empty */
#define	WaitIdleEmpty()			\
do								\
{								\
    WaitQueue(MAXFIFO);			\
    WaitIdle();					\
}								\
while (0)

Bool SMI_AccelInit(ScreenPtr pScrn);
void SMI_AccelSync(ScrnInfoPtr pScrn);
void SMI_EngineReset (ScrnInfoPtr pScrn);
int SMI750LE_deSetClipping(SMIPtr pSmi, uint8_t enable, uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
static int SMI750LE_deSetTransparency(SMIPtr pSmi, uint8_t enable, uint8_t tSelect, uint8_t tMatch, uint32_t color);
void SMI750LE_deInit(ScrnInfoPtr pScrn);
void SMI750LE_EngineReset (ScrnInfoPtr pScrn);
int SMI750LE_WaitForNotBusy(SMIPtr pSmi);


#endif   /* ----- #ifndef SMI_ACCEL_INC  ----- */
