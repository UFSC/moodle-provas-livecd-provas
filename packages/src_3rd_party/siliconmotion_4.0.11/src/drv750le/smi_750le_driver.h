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


#ifndef SMI_750LE_INC
#define SMI_750LE_INC
/* Includes Start */


#include "../smi_common.h"
#include "smi_750le_hw.h"
#include	"../ddk750/ddk750.h"
#include	"../ddk750/ddk750_chip.h"
#include	"../ddk750/ddk750_mode.h"
#if 1

#include <X11/extensions/dpmsconst.h>

#else

#if (XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)) || \
	(XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(4,3,0,0,0))
#define DPMS_SERVER
#include  <X11/extensions/dpms.h>
#else
#include <X11/extensions/dpmsconst.h>
#endif

#endif
/* Includes End */

enum region_type {
    REGION_MEM,
    REGION_IO 
};

#ifndef XSERVER_LIBPCIACCESS
/* region addr: xfree86 uses different fields for memory regions and I/O ports */
#define PCI_REGION_BASE(_pcidev, _b, _type)             \
    (((_type) == REGION_MEM) ? (_pcidev)->memBase[(_b)] \
                             : (_pcidev)->ioBase[(_b)])
#else
#define PCI_REGION_BASE(_pcidev, _b, _type) ((_pcidev)->regions[(_b)].base_addr)
#endif

typedef struct {
	/* put common structure here */
	SMIHWRec base;
	/* put chip dependent stuffs here*/
	SMI750leRegPtr pRegSave;
	/* vga stuffs */
	char * fonts;
}SMI750leHWRec,*SMI750leHWPtr;

typedef struct {
	/* we best put base in header of structure*/
	SMIRec base;
        IOADDRESS		        PIOBase;	/* Base of I/O ports */	
}SMI750leRec,*SMI750lePtr;

/* Structures End*/

/* Function Prototypes Start*/

mode_parameter_t * sm750le_findMode(mode_parameter_t * mode_table, INT width, 
        INT height, INT refresh_rate);
void sm750le_adjustMode(mode_parameter_t *vesaMode, mode_parameter_t *mode);
void sm750le_LoadPalette (ScrnInfoPtr pScrn, int numColors, 
        int *indicies, LOCO * colors, VisualPtr pVisual);

Bool SMI750LE_EnterVT (int scrnIndex, int flags);

SMIPtr sm750le_genSMI(pointer priv,int);
void sm750le_vgaAndpciInit(ScrnInfoPtr pScrn,int entityIndex);
void sm750le_entityInit(int,pointer);
void sm750le_entityEnter(int,pointer);
void sm750le_entityLeave(int,pointer);
void sm750le_getMapResult(ScrnInfoPtr);
void sm750le_handleOptions(ScrnInfoPtr);
ModeStatus sm750le_validMode(ScrnInfoPtr,DisplayModePtr );
void sm750le_setMode(ScrnInfoPtr,DisplayModePtr);
void sm750le_adjustFrame(ScrnInfoPtr,int);
void sm750le_setDisplay(ScrnInfoPtr pScrn, DisplayModePtr mode);
void sm750le_pcDeepmap(SMIHWPtr pHw);
void sm750le_saveText(ScrnInfoPtr pScrn);
static void sm750le_saveFonts(SMIHWPtr pHw);
static void sm750le_restoreFonts(SMIHWPtr pHw);
void sm750le_closeAllScreen(SMIHWPtr pHw);
int sm750le_totalFB(SMIHWPtr pHw);

extern unsigned int ddk750_getVMSize();
extern int ddk750_initHw(initchip_param_t * pInitParam);
extern void ddk750_setLogicalDispOut(disp_output_t output);
extern int ddk750_initDVIDisp();
extern int PEEK32(addr) ;
extern void POKE32(addr,data);
extern void ddk750_set_mmio(volatile unsigned char * addr,unsigned short devId,unsigned long revId);
extern int ddk750_setModeTiming(mode_parameter_t * parm,clock_type_t clock);
extern void ddk750_setDPMS(DPMS_t state);
extern Bool SMI750le_CrtcPreInit(ScrnInfoPtr pScrn);
extern Bool SMI750le_OutputPreInit(ScrnInfoPtr pScrn);
void sm750le_I2CInit(ScrnInfoPtr pScrn);
void set_display_750le(int bpp,int pitch,int ulBaseAddress,int HDisplay,int VDisplay);
/* Function Prototypes End*/


#ifndef _SMI_FUNCS_ONLY
/* Registers Start */

// Display Controller Registers
/* Secondary Graphics Control */
/* SM750le*/
#define GPIO_DATA_SM750LE                               0x020018
#define GPIO_DATA_SM750LE_1                             1:1
#define GPIO_DATA_SM750LE_0                             0:0
/* SM750le*/
#define GPIO_DATA_DIRECTION_SM750LE                     0x02001C
#define GPIO_DATA_DIRECTION_SM750LE_1                   1:1
#define GPIO_DATA_DIRECTION_SM750LE_1_INPUT             0
#define GPIO_DATA_DIRECTION_SM750LE_1_OUTPUT            1
#define GPIO_DATA_DIRECTION_SM750LE_0                   0:0
#define GPIO_DATA_DIRECTION_SM750LE_0_INPUT             0
#define GPIO_DATA_DIRECTION_SM750LE_0_OUTPUT            1



#define SECONDARY_DISPLAY_CTRL                              0x080200

/* SM750 and SM718 definition */
#define SECONDARY_DISPLAY_CTRL_RESERVED_1_MASK              31:27
#define SECONDARY_DISPLAY_CTRL_RESERVED_1_MASK_DISABLE      0
#define SECONDARY_DISPLAY_CTRL_RESERVED_1_MASK_ENABLE       0x1F
/* SM750LE definition */
#define SECONDARY_DISPLAY_CTRL_DPMS                         31:30
#define SECONDARY_DISPLAY_CTRL_DPMS_0                       0
#define SECONDARY_DISPLAY_CTRL_DPMS_1                       1
#define SECONDARY_DISPLAY_CTRL_DPMS_2                       2
#define SECONDARY_DISPLAY_CTRL_DPMS_3                       3
#define SECONDARY_DISPLAY_CTRL_CLK                          29:27
#define SECONDARY_DISPLAY_CTRL_CLK_PLL25                    0
#define SECONDARY_DISPLAY_CTRL_CLK_PLL41                    1
#define SECONDARY_DISPLAY_CTRL_CLK_PLL62                    2
#define SECONDARY_DISPLAY_CTRL_CLK_PLL65                    3
#define SECONDARY_DISPLAY_CTRL_CLK_PLL74                    4
#define SECONDARY_DISPLAY_CTRL_CLK_PLL80                    5
#define SECONDARY_DISPLAY_CTRL_CLK_PLL108                   6
#define SECONDARY_DISPLAY_CTRL_CLK_RESERVED                 7
#define SECONDARY_DISPLAY_CTRL_SHIFT_VGA_DAC                26:26
#define SECONDARY_DISPLAY_CTRL_SHIFT_VGA_DAC_DISABLE        1
#define SECONDARY_DISPLAY_CTRL_SHIFT_VGA_DAC_ENABLE         0

/* SM750 and SM718 definition */
#define SECONDARY_DISPLAY_CTRL_RESERVED_2_MASK              25:24
#define SECONDARY_DISPLAY_CTRL_RESERVED_2_MASK_DISABLE      0
#define SECONDARY_DISPLAY_CTRL_RESERVED_2_MASK_ENABLE       3

/* SM750LE definition */
#define SECONDARY_DISPLAY_CTRL_CRTSELECT                    25:25
#define SECONDARY_DISPLAY_CTRL_CRTSELECT_VGA                0
#define SECONDARY_DISPLAY_CTRL_CRTSELECT_CRT                1
#define SECONDARY_DISPLAY_CTRL_RGBBIT                       24:24
#define SECONDARY_DISPLAY_CTRL_RGBBIT_24BIT                 0
#define SECONDARY_DISPLAY_CTRL_RGBBIT_12BIT                 1

#define SECONDARY_DISPLAY_CTRL_LOCK_TIMING                  23:23
#define SECONDARY_DISPLAY_CTRL_LOCK_TIMING_DISABLE          0
#define SECONDARY_DISPLAY_CTRL_LOCK_TIMING_ENABLE           1
#define SECONDARY_DISPLAY_CTRL_EXPANSION                    22:22
#define SECONDARY_DISPLAY_CTRL_EXPANSION_DISABLE            0
#define SECONDARY_DISPLAY_CTRL_EXPANSION_ENABLE             1
#define SECONDARY_DISPLAY_CTRL_VERTICAL_MODE                21:21
#define SECONDARY_DISPLAY_CTRL_VERTICAL_MODE_REPLICATE      0
#define SECONDARY_DISPLAY_CTRL_VERTICAL_MODE_INTERPOLATE    1
#define SECONDARY_DISPLAY_CTRL_HORIZONTAL_MODE              20:20
#define SECONDARY_DISPLAY_CTRL_HORIZONTAL_MODE_REPLICATE    0
#define SECONDARY_DISPLAY_CTRL_HORIZONTAL_MODE_INTERPOLATE  1
#define SECONDARY_DISPLAY_CTRL_SELECT                       19:18
#define SECONDARY_DISPLAY_CTRL_SELECT_PRIMARY               0
#define SECONDARY_DISPLAY_CTRL_SELECT_VGA                   1
#define SECONDARY_DISPLAY_CTRL_SELECT_SECONDARY             2
#define SECONDARY_DISPLAY_CTRL_FIFO                         17:16
#define SECONDARY_DISPLAY_CTRL_FIFO_1                       0
#define SECONDARY_DISPLAY_CTRL_FIFO_3                       1
#define SECONDARY_DISPLAY_CTRL_FIFO_7                       2
#define SECONDARY_DISPLAY_CTRL_FIFO_11                      3
#define SECONDARY_DISPLAY_CTRL_RESERVED_3_MASK              15:15
#define SECONDARY_DISPLAY_CTRL_RESERVED_3_MASK_DISABLE      0
#define SECONDARY_DISPLAY_CTRL_RESERVED_3_MASK_ENABLE       1
#define SECONDARY_DISPLAY_CTRL_CLOCK_PHASE                  14:14
#define SECONDARY_DISPLAY_CTRL_CLOCK_PHASE_ACTIVE_HIGH      0
#define SECONDARY_DISPLAY_CTRL_CLOCK_PHASE_ACTIVE_LOW       1
#define SECONDARY_DISPLAY_CTRL_VSYNC_PHASE                  13:13
#define SECONDARY_DISPLAY_CTRL_VSYNC_PHASE_ACTIVE_HIGH      0
#define SECONDARY_DISPLAY_CTRL_VSYNC_PHASE_ACTIVE_LOW       1
#define SECONDARY_DISPLAY_CTRL_HSYNC_PHASE                  12:12
#define SECONDARY_DISPLAY_CTRL_HSYNC_PHASE_ACTIVE_HIGH      0
#define SECONDARY_DISPLAY_CTRL_HSYNC_PHASE_ACTIVE_LOW       1
#define SECONDARY_DISPLAY_CTRL_BLANK                        10:10
#define SECONDARY_DISPLAY_CTRL_BLANK_OFF                    0
#define SECONDARY_DISPLAY_CTRL_BLANK_ON                     1
#define SECONDARY_DISPLAY_CTRL_RESERVED_4_MASK              9:9
#define SECONDARY_DISPLAY_CTRL_RESERVED_4_MASK_DISABLE      0
#define SECONDARY_DISPLAY_CTRL_RESERVED_4_MASK_ENABLE       1
#define SECONDARY_DISPLAY_CTRL_TIMING                       8:8
#define SECONDARY_DISPLAY_CTRL_TIMING_DISABLE               0
#define SECONDARY_DISPLAY_CTRL_TIMING_ENABLE                1
#define SECONDARY_DISPLAY_CTRL_PIXEL                        7:4
#define SECONDARY_DISPLAY_CTRL_GAMMA                        3:3
#define SECONDARY_DISPLAY_CTRL_GAMMA_DISABLE                0
#define SECONDARY_DISPLAY_CTRL_GAMMA_ENABLE                 1
#define SECONDARY_DISPLAY_CTRL_PLANE                        2:2
#define SECONDARY_DISPLAY_CTRL_PLANE_DISABLE                0
#define SECONDARY_DISPLAY_CTRL_PLANE_ENABLE                 1
#define SECONDARY_DISPLAY_CTRL_FORMAT                       1:0
#define SECONDARY_DISPLAY_CTRL_FORMAT_8                     0
#define SECONDARY_DISPLAY_CTRL_FORMAT_16                    1
#define SECONDARY_DISPLAY_CTRL_FORMAT_32                    2
#define SECONDARY_DISPLAY_CTRL_RESERVED_BITS_MASK           0xFF000200

#define SECONDARY_FB_ADDRESS                            0x080204
#define SECONDARY_FB_ADDRESS_STATUS                     31:31
#define SECONDARY_FB_ADDRESS_STATUS_CURRENT             0
#define SECONDARY_FB_ADDRESS_STATUS_PENDING             1
#define SECONDARY_FB_ADDRESS_EXT                        27:27
#define SECONDARY_FB_ADDRESS_EXT_LOCAL                  0
#define SECONDARY_FB_ADDRESS_EXT_EXTERNAL               1
#define SECONDARY_FB_ADDRESS_ADDRESS                    25:0

#define SECONDARY_FB_WIDTH                              0x080208
#define SECONDARY_FB_WIDTH_WIDTH                        29:16
#define SECONDARY_FB_WIDTH_OFFSET                       13:0

#define SECONDARY_HORIZONTAL_TOTAL                      0x08020C
#define SECONDARY_HORIZONTAL_TOTAL_TOTAL                27:16
#define SECONDARY_HORIZONTAL_TOTAL_DISPLAY_END          11:0

#define SECONDARY_HORIZONTAL_SYNC                       0x080210
#define SECONDARY_HORIZONTAL_SYNC_WIDTH                 23:16
#define SECONDARY_HORIZONTAL_SYNC_START                 11:0

#define SECONDARY_VERTICAL_TOTAL                        0x080214
#define SECONDARY_VERTICAL_TOTAL_TOTAL                  26:16
#define SECONDARY_VERTICAL_TOTAL_DISPLAY_END            10:0

#define SECONDARY_VERTICAL_SYNC                         0x080218
#define SECONDARY_VERTICAL_SYNC_HEIGHT                  21:16
#define SECONDARY_VERTICAL_SYNC_START                   10:0

#define SECONDARY_SIGNATURE_ANALYZER                    0x08021C
#define SECONDARY_SIGNATURE_ANALYZER_STATUS             31:16
#define SECONDARY_SIGNATURE_ANALYZER_ENABLE             3:3
#define SECONDARY_SIGNATURE_ANALYZER_ENABLE_DISABLE     0
#define SECONDARY_SIGNATURE_ANALYZER_ENABLE_ENABLE      1
#define SECONDARY_SIGNATURE_ANALYZER_RESET              2:2
#define SECONDARY_SIGNATURE_ANALYZER_RESET_NORMAL       0
#define SECONDARY_SIGNATURE_ANALYZER_RESET_RESET        1
#define SECONDARY_SIGNATURE_ANALYZER_SOURCE             1:0
#define SECONDARY_SIGNATURE_ANALYZER_SOURCE_RED         0
#define SECONDARY_SIGNATURE_ANALYZER_SOURCE_GREEN       1
#define SECONDARY_SIGNATURE_ANALYZER_SOURCE_BLUE        2

#define SECONDARY_CURRENT_LINE                          0x080220
#define SECONDARY_CURRENT_LINE_LINE                     10:0

#define SECONDARY_MONITOR_DETECT                        0x080224
#define SECONDARY_MONITOR_DETECT_VALUE                  25:25
#define SECONDARY_MONITOR_DETECT_VALUE_DISABLE          0
#define SECONDARY_MONITOR_DETECT_VALUE_ENABLE           1
#define SECONDARY_MONITOR_DETECT_ENABLE                 24:24
#define SECONDARY_MONITOR_DETECT_ENABLE_DISABLE         0
#define SECONDARY_MONITOR_DETECT_ENABLE_ENABLE          1
#define SECONDARY_MONITOR_DETECT_RED                    23:16
#define SECONDARY_MONITOR_DETECT_GREEN                  15:8
#define SECONDARY_MONITOR_DETECT_BLUE                   7:0

#define SECONDARY_SCALE                                 0x080228
#define SECONDARY_SCALE_VERTICAL_MODE                   31:31
#define SECONDARY_SCALE_VERTICAL_MODE_EXPAND            0
#define SECONDARY_SCALE_VERTICAL_MODE_SHRINK            1
#define SECONDARY_SCALE_VERTICAL_SCALE                  27:16
#define SECONDARY_SCALE_HORIZONTAL_MODE                 15:15
#define SECONDARY_SCALE_HORIZONTAL_MODE_EXPAND          0
#define SECONDARY_SCALE_HORIZONTAL_MODE_SHRINK          1
#define SECONDARY_SCALE_HORIZONTAL_SCALE                11:0

/* Secondary Cursor Control */

#define SECONDARY_HWC_ADDRESS                           0x080230
#define SECONDARY_HWC_ADDRESS_ENABLE                    31:31
#define SECONDARY_HWC_ADDRESS_ENABLE_DISABLE            0
#define SECONDARY_HWC_ADDRESS_ENABLE_ENABLE             1
#define SECONDARY_HWC_ADDRESS_EXT                       27:27
#define SECONDARY_HWC_ADDRESS_EXT_LOCAL                 0
#define SECONDARY_HWC_ADDRESS_EXT_EXTERNAL              1
#define SECONDARY_HWC_ADDRESS_ADDRESS                   25:0

#define SECONDARY_HWC_LOCATION                          0x080234
#define SECONDARY_HWC_LOCATION_TOP                      27:27
#define SECONDARY_HWC_LOCATION_TOP_INSIDE               0
#define SECONDARY_HWC_LOCATION_TOP_OUTSIDE              1
#define SECONDARY_HWC_LOCATION_Y                        26:16
#define SECONDARY_HWC_LOCATION_LEFT                     11:11
#define SECONDARY_HWC_LOCATION_LEFT_INSIDE              0
#define SECONDARY_HWC_LOCATION_LEFT_OUTSIDE             1
#define SECONDARY_HWC_LOCATION_X                        10:0

#define SECONDARY_HWC_COLOR_12                          0x080238
#define SECONDARY_HWC_COLOR_12_2_RGB565                 31:16
#define SECONDARY_HWC_COLOR_12_1_RGB565                 15:0

#define SECONDARY_HWC_COLOR_3                           0x08023C
#define SECONDARY_HWC_COLOR_3_RGB565                    15:0

/* This vertical expansion below start at 0x080240 ~ 0x080264 */
#define SECONDARY_VERTICAL_EXPANSION                    0x080240
#define SECONDARY_VERTICAL_EXPANSION_CENTERING_VALUE    31:24 
#define SECONDARY_VERTICAL_EXPANSION_COMPARE_VALUE      23:16
#define SECONDARY_VERTICAL_EXPANSION_LINE_BUFFER        15:12
#define SECONDARY_VERTICAL_EXPANSION_SCALE_FACTOR       11:0

/* This horizontal expansion below start at 0x080268 ~ 0x08027C */
#define SECONDARY_HORIZONTAL_EXPANSION                  0x080268
#define SECONDARY_HORIZONTAL_EXPANSION_CENTERING_VALUE  31:24 
#define SECONDARY_HORIZONTAL_EXPANSION_COMPARE_VALUE    23:16
#define SECONDARY_HORIZONTAL_EXPANSION_SCALE_FACTOR     11:0

/* Auto Centering */
#define SECONDARY_AUTO_CENTERING_TL                     0x080280
#define SECONDARY_AUTO_CENTERING_TL_TOP                 26:16
#define SECONDARY_AUTO_CENTERING_TL_LEFT                10:0

#define SECONDARY_AUTO_CENTERING_BR                     0x080284
#define SECONDARY_AUTO_CENTERING_BR_BOTTOM              26:16
#define SECONDARY_AUTO_CENTERING_BR_RIGHT               10:0

/* SM750LE new register to control panel output */
#define DISPLAY_CONTROL_750LE                           0x80288
#define DISPLAY_CONTROL_750LE_RESERVED                  31:5
#define DISPLAY_CONTROL_750LE_PANEL                     4:4
#define DISPLAY_CONTROL_750LE_PANEL_NORMAL              0
#define DISPLAY_CONTROL_750LE_PANEL_TRISTATE            1
#define DISPLAY_CONTROL_750LE_EN                        3:3
#define DISPLAY_CONTROL_750LE_EN_LOW                    0
#define DISPLAY_CONTROL_750LE_EN_HIGH                   1
#define DISPLAY_CONTROL_750LE_BIAS                      2:2
#define DISPLAY_CONTROL_750LE_BIAS_LOW                  0
#define DISPLAY_CONTROL_750LE_BIAS_HIGH                 1
#define DISPLAY_CONTROL_750LE_DATA                      1:1
#define DISPLAY_CONTROL_750LE_DATA_DISABLE              0
#define DISPLAY_CONTROL_750LE_DATA_ENABLE               1
#define DISPLAY_CONTROL_750LE_VDD                       0:0 
#define DISPLAY_CONTROL_750LE_VDD_LOW                   0
#define DISPLAY_CONTROL_750LE_VDD_HIGH                  1

/* SM750LE new register for display interrtup control */
#define RAW_INT_750LE                                    0x080290
#define RAW_INT_750LE_RESERVED1                          31:3
#define RAW_INT_750LE_SECONDARY_VSYNC                    2:2
#define RAW_INT_750LE_SECONDARY_VSYNC_INACTIVE           0
#define RAW_INT_750LE_SECONDARY_VSYNC_ACTIVE             1
#define RAW_INT_750LE_SECONDARY_VSYNC_CLEAR              1
#define RAW_INT_750LE_PRIMARY_VSYNC                      1:1
#define RAW_INT_750LE_PRIMARY_VSYNC_INACTIVE             0
#define RAW_INT_750LE_PRIMARY_VSYNC_ACTIVE               1
#define RAW_INT_750LE_PRIMARY_VSYNC_CLEAR                1
#define RAW_INT_750LE_VGA_VSYNC                          0:0
#define RAW_INT_750LE_VGA_VSYNC_INACTIVE                 0
#define RAW_INT_750LE_VGA_VSYNC_ACTIVE                   1
#define RAW_INT_750LE_VGA_VSYNC_CLEAR                    1

#define INT_STATUS_750LE                                 0x080294
#define INT_STATUS_750LE_RESERVED1                       31:3
#define INT_STATUS_750LE_SECONDARY_VSYNC                 2:2
#define INT_STATUS_750LE_SECONDARY_VSYNC_INACTIVE        0
#define INT_STATUS_750LE_SECONDARY_VSYNC_ACTIVE          1
#define INT_STATUS_750LE_PRIMARY_VSYNC                   1:1
#define INT_STATUS_750LE_PRIMARY_VSYNC_INACTIVE          0
#define INT_STATUS_750LE_PRIMARY_VSYNC_ACTIVE            1
#define INT_STATUS_750LE_VGA_VSYNC                       0:0
#define INT_STATUS_750LE_VGA_VSYNC_INACTIVE              0
#define INT_STATUS_750LE_VGA_VSYNC_ACTIVE                1

#define INT_MASK_750LE                                   0x080298
#define INT_MASK_750LE_RESERVED1                         31:3
#define INT_MASK_750LE_SECONDARY_VSYNC                   2:2
#define INT_MASK_750LE_SECONDARY_VSYNC_DISABLE           0
#define INT_MASK_750LE_SECONDARY_VSYNC_ENABLE            1
#define INT_MASK_750LE_PRIMARY_VSYNC                     1:1
#define INT_MASK_750LE_PRIMARY_VSYNC_DISABLE             0
#define INT_MASK_750LE_PRIMARY_VSYNC_ENABLE              1
#define INT_MASK_750LE_VGA_VSYNC                         0:0
#define INT_MASK_750LE_VGA_VSYNC_DISABLE                 0
#define INT_MASK_750LE_VGA_VSYNC_ENABLE                  1

/* Palette RAM */

/* Panel Pallete register starts at 0x080400 ~ 0x0807FC */
#define PRIMARY_PALETTE_RAM                             0x080400

/* Panel Pallete register starts at 0x080C00 ~ 0x080FFC */
#define SECONDARY_PALETTE_RAM                           0x080C00

//Drawing Engine Registers
/* 2D registers. */

#define DE_SOURCE                                       0x100000
#define DE_SOURCE_WRAP                                  31:31
#define DE_SOURCE_WRAP_DISABLE                          0
#define DE_SOURCE_WRAP_ENABLE                           1

/* 
 * The following definitions are used in different setting 
 */

/* Use these definitions in XY addressing mode or linear addressing mode. */
#define DE_SOURCE_X_K1                                  27:16
#define DE_SOURCE_Y_K2                                  11:0

/* Use this definition in host write mode for mono. The Y_K2 is not used
   in host write mode. */
#define DE_SOURCE_X_K1_MONO                             20:16

/* Use these definitions in Bresenham line drawing mode. */
#define DE_SOURCE_X_K1_LINE                             29:16
#define DE_SOURCE_Y_K2_LINE                             13:0

#define DE_DESTINATION                                  0x100004
#define DE_DESTINATION_WRAP                             31:31
#define DE_DESTINATION_WRAP_DISABLE                     0
#define DE_DESTINATION_WRAP_ENABLE                      1
#if 1
    #define DE_DESTINATION_X                            27:16
    #define DE_DESTINATION_Y                            11:0
#else
    #define DE_DESTINATION_X                            28:16
    #define DE_DESTINATION_Y                            15:0
#endif

#define DE_DIMENSION                                    0x100008
#define DE_DIMENSION_X                                  28:16
#define DE_DIMENSION_Y_ET                               15:0

#define DE_CONTROL                                      0x10000C
#define DE_CONTROL_STATUS                               31:31
#define DE_CONTROL_STATUS_STOP                          0
#define DE_CONTROL_STATUS_START                         1
#define DE_CONTROL_PATTERN                              30:30
#define DE_CONTROL_PATTERN_MONO                         0
#define DE_CONTROL_PATTERN_COLOR                        1
#define DE_CONTROL_UPDATE_DESTINATION_X                 29:29
#define DE_CONTROL_UPDATE_DESTINATION_X_DISABLE         0
#define DE_CONTROL_UPDATE_DESTINATION_X_ENABLE          1
#define DE_CONTROL_QUICK_START                          28:28
#define DE_CONTROL_QUICK_START_DISABLE                  0
#define DE_CONTROL_QUICK_START_ENABLE                   1
#define DE_CONTROL_DIRECTION                            27:27
#define DE_CONTROL_DIRECTION_LEFT_TO_RIGHT              0
#define DE_CONTROL_DIRECTION_RIGHT_TO_LEFT              1
#define DE_CONTROL_MAJOR                                26:26
#define DE_CONTROL_MAJOR_X                              0
#define DE_CONTROL_MAJOR_Y                              1
#define DE_CONTROL_STEP_X                               25:25
#define DE_CONTROL_STEP_X_POSITIVE                      0
#define DE_CONTROL_STEP_X_NEGATIVE                      1
#define DE_CONTROL_STEP_Y                               24:24
#define DE_CONTROL_STEP_Y_POSITIVE                      0
#define DE_CONTROL_STEP_Y_NEGATIVE                      1
#define DE_CONTROL_STRETCH                              23:23
#define DE_CONTROL_STRETCH_DISABLE                      0
#define DE_CONTROL_STRETCH_ENABLE                       1
#define DE_CONTROL_HOST                                 22:22
#define DE_CONTROL_HOST_COLOR                           0
#define DE_CONTROL_HOST_MONO                            1
#define DE_CONTROL_LAST_PIXEL                           21:21
#define DE_CONTROL_LAST_PIXEL_OFF                       0
#define DE_CONTROL_LAST_PIXEL_ON                        1
#define DE_CONTROL_COMMAND                              20:16
#define DE_CONTROL_COMMAND_BITBLT                       0
#define DE_CONTROL_COMMAND_RECTANGLE_FILL               1
#define DE_CONTROL_COMMAND_DE_TILE                      2
#define DE_CONTROL_COMMAND_TRAPEZOID_FILL               3
#define DE_CONTROL_COMMAND_ALPHA_BLEND                  4
#define DE_CONTROL_COMMAND_RLE_STRIP                    5
#define DE_CONTROL_COMMAND_SHORT_STROKE                 6
#define DE_CONTROL_COMMAND_LINE_DRAW                    7
#define DE_CONTROL_COMMAND_HOST_WRITE                   8
#define DE_CONTROL_COMMAND_HOST_READ                    9
#define DE_CONTROL_COMMAND_HOST_WRITE_BOTTOM_UP         10
#define DE_CONTROL_COMMAND_ROTATE                       11
#define DE_CONTROL_COMMAND_FONT                         12
#define DE_CONTROL_COMMAND_TEXTURE_LOAD                 15
#define DE_CONTROL_ROP_SELECT                           15:15
#define DE_CONTROL_ROP_SELECT_ROP3                      0
#define DE_CONTROL_ROP_SELECT_ROP2                      1
#define DE_CONTROL_ROP2_SOURCE                          14:14
#define DE_CONTROL_ROP2_SOURCE_BITMAP                   0
#define DE_CONTROL_ROP2_SOURCE_PATTERN                  1
#define DE_CONTROL_MONO_DATA                            13:12
#define DE_CONTROL_MONO_DATA_NOT_PACKED                 0
#define DE_CONTROL_MONO_DATA_8_PACKED                   1
#define DE_CONTROL_MONO_DATA_16_PACKED                  2
#define DE_CONTROL_MONO_DATA_32_PACKED                  3
#define DE_CONTROL_REPEAT_ROTATE                        11:11
#define DE_CONTROL_REPEAT_ROTATE_DISABLE                0
#define DE_CONTROL_REPEAT_ROTATE_ENABLE                 1
#define DE_CONTROL_TRANSPARENCY_MATCH                   10:10
#define DE_CONTROL_TRANSPARENCY_MATCH_OPAQUE            0
#define DE_CONTROL_TRANSPARENCY_MATCH_TRANSPARENT       1
#define DE_CONTROL_TRANSPARENCY_SELECT                  9:9
#define DE_CONTROL_TRANSPARENCY_SELECT_SOURCE           0
#define DE_CONTROL_TRANSPARENCY_SELECT_DESTINATION      1
#define DE_CONTROL_TRANSPARENCY                         8:8
#define DE_CONTROL_TRANSPARENCY_DISABLE                 0
#define DE_CONTROL_TRANSPARENCY_ENABLE                  1
#define DE_CONTROL_ROP                                  7:0

/* Pseudo fields. */

#define DE_CONTROL_SHORT_STROKE_DIR                     27:24
#define DE_CONTROL_SHORT_STROKE_DIR_225                 0
#define DE_CONTROL_SHORT_STROKE_DIR_135                 1
#define DE_CONTROL_SHORT_STROKE_DIR_315                 2
#define DE_CONTROL_SHORT_STROKE_DIR_45                  3
#define DE_CONTROL_SHORT_STROKE_DIR_270                 4
#define DE_CONTROL_SHORT_STROKE_DIR_90                  5
#define DE_CONTROL_SHORT_STROKE_DIR_180                 8
#define DE_CONTROL_SHORT_STROKE_DIR_0                   10
#define DE_CONTROL_ROTATION                             25:24
#define DE_CONTROL_ROTATION_0                           0
#define DE_CONTROL_ROTATION_270                         1
#define DE_CONTROL_ROTATION_90                          2
#define DE_CONTROL_ROTATION_180                         3

#define DE_PITCH                                        0x100010
#define DE_PITCH_DESTINATION                            28:16
#define DE_PITCH_SOURCE                                 12:0

#define DE_FOREGROUND                                   0x100014
#define DE_FOREGROUND_COLOR                             31:0

#define DE_BACKGROUND                                   0x100018
#define DE_BACKGROUND_COLOR                             31:0

#define DE_STRETCH_FORMAT                               0x10001C
#define DE_STRETCH_FORMAT_PATTERN_XY                    30:30
#define DE_STRETCH_FORMAT_PATTERN_XY_NORMAL             0
#define DE_STRETCH_FORMAT_PATTERN_XY_OVERWRITE          1
#define DE_STRETCH_FORMAT_PATTERN_Y                     29:27
#define DE_STRETCH_FORMAT_PATTERN_X                     25:23
#define DE_STRETCH_FORMAT_PIXEL_FORMAT                  21:20
#define DE_STRETCH_FORMAT_PIXEL_FORMAT_8                0
#define DE_STRETCH_FORMAT_PIXEL_FORMAT_16               1
#define DE_STRETCH_FORMAT_PIXEL_FORMAT_32               2
#define DE_STRETCH_FORMAT_ADDRESSING                    19:16
#define DE_STRETCH_FORMAT_ADDRESSING_XY                 0
#define DE_STRETCH_FORMAT_ADDRESSING_LINEAR             15
#define DE_STRETCH_FORMAT_SOURCE_HEIGHT                 11:0

#define DE_COLOR_COMPARE                                0x100020
#define DE_COLOR_COMPARE_COLOR                          23:0

#define DE_COLOR_COMPARE_MASK                           0x100024
#define DE_COLOR_COMPARE_MASK_MASKS                     23:0

#define DE_MASKS                                        0x100028
#define DE_MASKS_BYTE_MASK                              31:16
#define DE_MASKS_BIT_MASK                               15:0

#define DE_CLIP_TL                                      0x10002C
#define DE_CLIP_TL_TOP                                  31:16
#define DE_CLIP_TL_STATUS                               13:13
#define DE_CLIP_TL_STATUS_DISABLE                       0
#define DE_CLIP_TL_STATUS_ENABLE                        1
#define DE_CLIP_TL_INHIBIT                              12:12
#define DE_CLIP_TL_INHIBIT_OUTSIDE                      0
#define DE_CLIP_TL_INHIBIT_INSIDE                       1
#define DE_CLIP_TL_LEFT                                 11:0

#define DE_CLIP_BR                                      0x100030
#define DE_CLIP_BR_BOTTOM                               31:16
#define DE_CLIP_BR_RIGHT                                12:0

#define DE_MONO_PATTERN_LOW                             0x100034
#define DE_MONO_PATTERN_LOW_PATTERN                     31:0

#define DE_MONO_PATTERN_HIGH                            0x100038
#define DE_MONO_PATTERN_HIGH_PATTERN                    31:0

#define DE_WINDOW_WIDTH                                 0x10003C
#define DE_WINDOW_WIDTH_DESTINATION                     28:16
#define DE_WINDOW_WIDTH_SOURCE                          12:0

#define DE_WINDOW_SOURCE_BASE                           0x100040
#define DE_WINDOW_SOURCE_BASE_EXT                       27:27
#define DE_WINDOW_SOURCE_BASE_EXT_LOCAL                 0
#define DE_WINDOW_SOURCE_BASE_EXT_EXTERNAL              1
#define DE_WINDOW_SOURCE_BASE_CS                        26:26
#define DE_WINDOW_SOURCE_BASE_CS_0                      0
#define DE_WINDOW_SOURCE_BASE_CS_1                      1
#define DE_WINDOW_SOURCE_BASE_ADDRESS                   25:0

#define DE_WINDOW_DESTINATION_BASE                      0x100044
#define DE_WINDOW_DESTINATION_BASE_EXT                  27:27
#define DE_WINDOW_DESTINATION_BASE_EXT_LOCAL            0
#define DE_WINDOW_DESTINATION_BASE_EXT_EXTERNAL         1
#define DE_WINDOW_DESTINATION_BASE_CS                   26:26
#define DE_WINDOW_DESTINATION_BASE_CS_0                 0
#define DE_WINDOW_DESTINATION_BASE_CS_1                 1
#define DE_WINDOW_DESTINATION_BASE_ADDRESS              25:0

#define DE_ALPHA                                        0x100048
#define DE_ALPHA_VALUE                                  7:0

#define DE_WRAP                                         0x10004C
#define DE_WRAP_X                                       31:16
#define DE_WRAP_Y                                       15:0

#define DE_STATUS                                       0x100050
#define DE_STATUS_CSC                                   1:1
#define DE_STATUS_CSC_CLEAR                             0
#define DE_STATUS_CSC_NOT_ACTIVE                        0
#define DE_STATUS_CSC_ACTIVE                            1
#define DE_STATUS_2D                                    0:0
#define DE_STATUS_2D_CLEAR                              0
#define DE_STATUS_2D_NOT_ACTIVE                         0
#define DE_STATUS_2D_ACTIVE                             1

/* New register for SM750LE */
#define DE_STATE1                                        0x100054
#define DE_STATE1_DE_ABORT                               0:0
#define DE_STATE1_DE_ABORT_OFF                           0
#define DE_STATE1_DE_ABORT_ON                            1

#define DE_STATE2                                        0x100058
#define DE_STATE2_DE_FIFO                                3:3
#define DE_STATE2_DE_FIFO_NOTEMPTY                       0
#define DE_STATE2_DE_FIFO_EMPTY                          1
#define DE_STATE2_DE_STATUS                              2:2
#define DE_STATE2_DE_STATUS_IDLE                         0
#define DE_STATE2_DE_STATUS_BUSY                         1
#define DE_STATE2_DE_MEM_FIFO                            1:1
#define DE_STATE2_DE_MEM_FIFO_NOTEMPTY                   0
#define DE_STATE2_DE_MEM_FIFO_EMPTY                      1
#define DE_STATE2_DE_RESERVED                            0:0

/* Color Space Conversion registers. */

#define CSC_Y_SOURCE_BASE                               0x1000C8
#define CSC_Y_SOURCE_BASE_EXT                           27:27
#define CSC_Y_SOURCE_BASE_EXT_LOCAL                     0
#define CSC_Y_SOURCE_BASE_EXT_EXTERNAL                  1
#define CSC_Y_SOURCE_BASE_CS                            26:26
#define CSC_Y_SOURCE_BASE_CS_0                          0
#define CSC_Y_SOURCE_BASE_CS_1                          1
#define CSC_Y_SOURCE_BASE_ADDRESS                       25:0

#define CSC_CONSTANTS                                   0x1000CC
#define CSC_CONSTANTS_Y                                 31:24
#define CSC_CONSTANTS_R                                 23:16
#define CSC_CONSTANTS_G                                 15:8
#define CSC_CONSTANTS_B                                 7:0

#define CSC_Y_SOURCE_X                                  0x1000D0
#define CSC_Y_SOURCE_X_INTEGER                          26:16
#define CSC_Y_SOURCE_X_FRACTION                         15:3

#define CSC_Y_SOURCE_Y                                  0x1000D4
#define CSC_Y_SOURCE_Y_INTEGER                          27:16
#define CSC_Y_SOURCE_Y_FRACTION                         15:3

#define CSC_U_SOURCE_BASE                               0x1000D8
#define CSC_U_SOURCE_BASE_EXT                           27:27
#define CSC_U_SOURCE_BASE_EXT_LOCAL                     0
#define CSC_U_SOURCE_BASE_EXT_EXTERNAL                  1
#define CSC_U_SOURCE_BASE_CS                            26:26
#define CSC_U_SOURCE_BASE_CS_0                          0
#define CSC_U_SOURCE_BASE_CS_1                          1
#define CSC_U_SOURCE_BASE_ADDRESS                       25:0

#define CSC_V_SOURCE_BASE                               0x1000DC
#define CSC_V_SOURCE_BASE_EXT                           27:27
#define CSC_V_SOURCE_BASE_EXT_LOCAL                     0
#define CSC_V_SOURCE_BASE_EXT_EXTERNAL                  1
#define CSC_V_SOURCE_BASE_CS                            26:26
#define CSC_V_SOURCE_BASE_CS_0                          0
#define CSC_V_SOURCE_BASE_CS_1                          1
#define CSC_V_SOURCE_BASE_ADDRESS                       25:0

#define CSC_SOURCE_DIMENSION                            0x1000E0
#define CSC_SOURCE_DIMENSION_X                          31:16
#define CSC_SOURCE_DIMENSION_Y                          15:0

#define CSC_SOURCE_PITCH                                0x1000E4
#define CSC_SOURCE_PITCH_Y                              31:16
#define CSC_SOURCE_PITCH_UV                             15:0

#define CSC_DESTINATION                                 0x1000E8
#define CSC_DESTINATION_WRAP                            31:31
#define CSC_DESTINATION_WRAP_DISABLE                    0
#define CSC_DESTINATION_WRAP_ENABLE                     1
#define CSC_DESTINATION_X                               27:16
#define CSC_DESTINATION_Y                               11:0

#define CSC_DESTINATION_DIMENSION                       0x1000EC
#define CSC_DESTINATION_DIMENSION_X                     31:16
#define CSC_DESTINATION_DIMENSION_Y                     15:0

#define CSC_DESTINATION_PITCH                           0x1000F0
#define CSC_DESTINATION_PITCH_X                         31:16
#define CSC_DESTINATION_PITCH_Y                         15:0

#define CSC_SCALE_FACTOR                                0x1000F4
#define CSC_SCALE_FACTOR_HORIZONTAL                     31:16
#define CSC_SCALE_FACTOR_VERTICAL                       15:0

#define CSC_DESTINATION_BASE                            0x1000F8
#define CSC_DESTINATION_BASE_EXT                        27:27
#define CSC_DESTINATION_BASE_EXT_LOCAL                  0
#define CSC_DESTINATION_BASE_EXT_EXTERNAL               1
#define CSC_DESTINATION_BASE_CS                         26:26
#define CSC_DESTINATION_BASE_CS_0                       0
#define CSC_DESTINATION_BASE_CS_1                       1
#define CSC_DESTINATION_BASE_ADDRESS                    25:0

#define CSC_CONTROL                                     0x1000FC
#define CSC_CONTROL_STATUS                              31:31
#define CSC_CONTROL_STATUS_STOP                         0
#define CSC_CONTROL_STATUS_START                        1
#define CSC_CONTROL_SOURCE_FORMAT                       30:28
#define CSC_CONTROL_SOURCE_FORMAT_YUV422                0
#define CSC_CONTROL_SOURCE_FORMAT_YUV420I               1
#define CSC_CONTROL_SOURCE_FORMAT_YUV420                2
#define CSC_CONTROL_SOURCE_FORMAT_YVU9                  3
#define CSC_CONTROL_SOURCE_FORMAT_IYU1                  4
#define CSC_CONTROL_SOURCE_FORMAT_IYU2                  5
#define CSC_CONTROL_SOURCE_FORMAT_RGB565                6
#define CSC_CONTROL_SOURCE_FORMAT_RGB8888               7
#define CSC_CONTROL_DESTINATION_FORMAT                  27:26
#define CSC_CONTROL_DESTINATION_FORMAT_RGB565           0
#define CSC_CONTROL_DESTINATION_FORMAT_RGB8888          1
#define CSC_CONTROL_HORIZONTAL_FILTER                   25:25
#define CSC_CONTROL_HORIZONTAL_FILTER_DISABLE           0
#define CSC_CONTROL_HORIZONTAL_FILTER_ENABLE            1
#define CSC_CONTROL_VERTICAL_FILTER                     24:24
#define CSC_CONTROL_VERTICAL_FILTER_DISABLE             0
#define CSC_CONTROL_VERTICAL_FILTER_ENABLE              1
#define CSC_CONTROL_BYTE_ORDER                          23:23
#define CSC_CONTROL_BYTE_ORDER_YUYV                     0
#define CSC_CONTROL_BYTE_ORDER_UYVY                     1

#define DE_DATA_PORT                                    0x110000
/* Registers End */
#endif
#endif   /* ----- #ifndef SMI_750LE_INC  ----- */

