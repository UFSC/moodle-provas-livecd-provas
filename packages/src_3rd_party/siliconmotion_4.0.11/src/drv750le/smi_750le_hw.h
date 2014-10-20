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

#ifndef  SMI_750LE_HW_INC
#define  SMI_750LE_HW_INC

/* Driver data structure; this should contain all needed info for a mode */
typedef struct
{    
    Bool    modeInit;
    CARD16 mode;
    
    /* registers for save */
    uint32_t de_stretch_format, de_masks, de_window_width, de_control;
    uint32_t secondary_display_ctrl, secondary_fb_address, secondary_fb_width;
    uint32_t secondary_horizontal_total, secondary_horizontal_sync;
    uint32_t secondary_vertical_total, secondary_vertical_sync;
    uint32_t secondary_auto_centering_tl, secondary_auto_centering_br;
    uint32_t secondary_scale, secondary_hwc_address, secondary_hwc_location;
    uint32_t secondary_hwc_color_12, secondary_hwc_color_3;
    uint32_t display_control_750le;

    /* palette ram */
    uint32_t secondary_palette_ram[32];

    /* ext vga */
    uint8_t vga_conf;

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
} SMI750leRegRec, *SMI750leRegPtr;

#define DEFAULT_SM750LE_CHIP_CLOCK  	333

void save_reg_750le(SMIHWPtr);
void restore_reg_750le(SMIHWPtr);
int get_total_fb_750le();
void save_reg_vga_750le(ScrnInfoPtr pScrn);
#if SMI_RANDR
void i2c_putbits_panel_750le(I2CBusPtr bus,int clock,int data);
void i2c_getbits_panel_750le(I2CBusPtr bus,int* clock,int* data);
#endif
#endif   /* ----- #ifndef SMI_750LE_HW_INC  ----- */
