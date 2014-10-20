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
#ifndef  SMI_DRIVER_INC
#define  SMI_DRIVER_INC

#define SM5XX 	1
#define SM7XX	2
#define SM7X2	4
/* Init flags and values used in init_crt structure */
#define DISP_CRT_TVP                       0x00000100    /* TV clock phase select */
#define DISP_CRT_TVP_HIGH                  0x00000000
#define DISP_CRT_TVP_LOW                   0x00008000

#define DISP_CRT_CP                        0x00000200    /* CRT clock phase select */
#define DISP_CRT_CP_HIGH                   0x00000000
#define DISP_CRT_CP_LOW                    0x00004000

#define DISP_CRT_BLANK                     0x00000400    /* CRT data blanking */
#define DISP_CRT_BLANK_OFF                 0x00000000
#define DISP_CRT_BLANK_ON                  0x00000400

#define DISP_CRT_FORMAT                    0x00000800    /* CRT graphics plane format */
#define DISP_CRT_FORMAT_8                  0x00000000
#define DISP_CRT_FORMAT_16                 0x00000001
#define DISP_CRT_FORMAT_32                 0x00000002

#define DISP_MODE_8_BPP			           0		     /* 8 bits per pixel i8RGB                        */
#define DISP_MODE_16_BPP		           1		     /* 16 bits per pixel RGB565                      */
#define DISP_MODE_32_BPP		           2		     /* 32 bits per pixel RGB888                      */
#define DISP_MODE_YUV			           3		     /* 16 bits per pixel YUV422                      */
#define DISP_MODE_ALPHA_8		           4		     /* 8 bits per pixel a4i4RGB                      */
#define DISP_MODE_ALPHA_16		           5		     /* 16 bits per pixel a4RGB444                    */

#define DISP_PAN_LEFT			           0		     /* Pan left                                      */
#define DISP_PAN_RIGHT			           1		     /* Pan right                                     */
#define DISP_PAN_UP				           2		     /* Pan upwards                                   */
#define DISP_PAN_DOWN		               3		     /* Pan downwards                                 */
                                                                                                           
#define DISP_DPMS_QUERY			           -1		     /* Query DPMS value                              */
#define DISP_DPMS_ON			           0		     /* DPMS on                                       */
#define DISP_DPMS_STANDBY	               1		     /* DPMS standby                                  */
#define DISP_DPMS_SUSPEND		           2		     /* DPMS suspend                                  */
#define DISP_DPMS_OFF			           3		     /* DPMS off                                      */

#define DISP_DELAY_DEFAULT		           0		     /* Default delay                                 */

#define DISP_HVTOTAL_UNKNOWN               -1            /* Used in panelSetTiming, crtSetTiming if       */
                                                         /* nHTotal, nVTotal not specified by user        */
#define DISP_HVTOTAL_SCALEFACTOR           1.25          /* Used in panelSetTiming, crtSetTiming if       */
                                                         /* nHTotal, nVTotal not specified by user        */

#define VGX_SIGNAL_PANEL_VSYNC	           100		     /* Panel VSYNC                                   */
#define VGX_SIGNAL_PANEL_PAN               101		     /* Panel auto panning complete                   */
#define VGX_SIGNAL_CRT_VSYNC               102		     /* CRT VSYNC                                     */

#define VSYNCTIMEOUT                       10000

#define ALPHA_MODE_PER_PIXEL               0             /* Use per-pixel alpha values                    */
#define ALPHA_MODE_ALPHA                   1             /* Use alpha value specified in Alpha bitfield   */
#define ALPHA_COLOR_LUT_SIZE               16            /* Number of colors in alpha/video alpha palette */
                                                                                                           
#define HWC_ON_SCREEN                      0             /* Cursor is within screen top/left boundary     */
#define HWC_OFF_SCREEN                     1             /* Cursor is outside screen top/left boundary    */
#define HWC_NUM_COLORS                     3             /* Number of cursor colors                       */

#define RGB565_R_MASK                      0xF8          /* Mask for red color                            */
#define RGB565_G_MASK                      0xFC          /* Mask for green color                          */
#define RGB565_B_MASK                      0xF8          /* Mask for blue color                           */

#define RGB565_R_SHIFT                     8             /* Number of bits to shift for red color         */
#define RGB565_G_SHIFT                     3             /* Number of bits to shift for green color       */
#define RGB565_B_SHIFT                     3             /* Number of bits to shift for blue color        */

/* Some modules needed by special video card*/
#define MODULE_INT10	1
#define MODULE_VBE		2
#define MODULE_VGA		4

#endif   /* ----- #ifndef SMI_DRIVER_INC  ----- */
