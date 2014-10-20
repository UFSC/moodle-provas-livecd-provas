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

#include "../smi_common.h"
#include "../smi_driver.h"
#include "../ddk750/ddk750.h"
#include "../ddk750/ddk750_display.h"
#include "smi_750le_hw.h"
#include "smi_750le_driver.h"
#include "../smi_dbg.h"

/*Set the bpp and date channel and pitch for panel*/
void set_display_750le( int bpp, int pitch, int ulBaseAddress,int HDisplay, int VDisplay )
{
        ENTER();
        int SetValue;
        disp_output_t output;

        SetValue = PEEK32 (CRT_DISPLAY_CTRL);
		
        SetValue = (bpp == 8 ? 
            FIELD_SET(SetValue, CRT_DISPLAY_CTRL, FORMAT, 8)
            :FIELD_SET(SetValue, CRT_DISPLAY_CTRL, FORMAT, 16));
	
        POKE32(CRT_DISPLAY_CTRL, SetValue | 
            FIELD_SET(0, CRT_DISPLAY_CTRL, SELECT, CRT));

        POKE32( CRT_FB_ADDRESS, 
            FIELD_SET(0, CRT_FB_ADDRESS, STATUS, PENDING) |
            FIELD_SET(0, CRT_FB_ADDRESS, EXT, LOCAL) |
            FIELD_VALUE(0, CRT_FB_ADDRESS, ADDRESS, ulBaseAddress));

	POKE32(CRT_FB_WIDTH,
           FIELD_VALUE(0, CRT_FB_WIDTH, WIDTH, pitch)
           | FIELD_VALUE(0, CRT_FB_WIDTH, OFFSET, pitch));
	
    LEAVE();
}

void restore_reg_750le(SMIHWPtr pHw)
{
    ENTER();
    int 	i, j;
    SMI750leHWPtr p750leHw = (SMI750leHWPtr)pHw;
    SMI750leRegPtr save = p750leHw->pRegSave;
    ScrnInfoPtr pScrn;
    SMIPtr pSmi = HWPSMI(pHw);
    pScrn = SMIPSI(pSmi);
    vgaHWPtr hwp = VGAHWPTR (pScrn);
    vgaRegPtr vgaSavePtr = &hwp->SavedReg;
    SMI750lePtr p750Rec = (SMI750lePtr)pSmi;
    int idx;

    /* restore mmio registers */
    POKE32( SECONDARY_FB_ADDRESS,  save->secondary_fb_address);
    POKE32(SECONDARY_FB_WIDTH, save->secondary_fb_width);
    POKE32( SECONDARY_HORIZONTAL_TOTAL,  save->secondary_horizontal_total);
    POKE32(SECONDARY_HORIZONTAL_SYNC, save->secondary_horizontal_sync);
    POKE32( SECONDARY_VERTICAL_TOTAL,  save->secondary_vertical_total);
    POKE32( SECONDARY_VERTICAL_SYNC, save->secondary_vertical_sync);
    POKE32(SECONDARY_SCALE, save->secondary_scale);
    POKE32( SECONDARY_HWC_ADDRESS, save->secondary_hwc_address);
    POKE32( SECONDARY_HWC_LOCATION, save->secondary_hwc_location);
    POKE32( SECONDARY_HWC_COLOR_12, save->secondary_hwc_color_12);
    POKE32( SECONDARY_HWC_COLOR_3, save->secondary_hwc_color_3);
    POKE32( DE_STRETCH_FORMAT, save->de_stretch_format);
    POKE32( DE_MASKS, save->de_masks);
    POKE32( DE_WINDOW_WIDTH, save->de_window_width);
    POKE32( SECONDARY_AUTO_CENTERING_TL,  save->secondary_auto_centering_tl);
    POKE32( SECONDARY_AUTO_CENTERING_BR,  save->secondary_auto_centering_br);
    POKE32( SECONDARY_DISPLAY_CTRL, save->secondary_display_ctrl);
    POKE32( DISPLAY_CONTROL_750LE, save->display_control_750le);
    POKE32( DE_CONTROL, save->de_control);

    /* restore palette ram */
    for(idx = 0; idx < 32; idx++) {
        POKE32( SECONDARY_PALETTE_RAM + idx * 4,
            save->secondary_palette_ram[idx]);
    }

    /* restore vga */
    outb(p750Rec->PIOBase + 0x3D4, 0x88);
    outb(p750Rec->PIOBase + 0x3D5, save->vga_conf);
    vgaHWRestore(pScrn, vgaSavePtr, VGA_SR_ALL);

    LEAVE();	
}
void save_reg_vga_750le(ScrnInfoPtr pScrn)
{
        ENTER();
        vgaHWPtr hwp = VGAHWPTR (pScrn);
        vgaRegPtr vgaSavePtr = &hwp->SavedReg;
        vgaHWSave(pScrn, vgaSavePtr, VGA_SR_ALL);
        LEAVE();
}

void save_reg_750le(SMIHWPtr pHw)
{
    ENTER();
    SMI750leHWPtr p750leHw = (SMI750leHWPtr)pHw;
    SMI750leRegPtr save = p750leHw->pRegSave;
    ScrnInfoPtr pScrn;
    SMIPtr pSmi = HWPSMI(pHw);
    pScrn = SMIPSI(pSmi);
    SMI750lePtr p750Rec = (SMI750lePtr)pSmi;
    int idx;
    
    /* save mmio registers */
    save->de_stretch_format = PEEK32( DE_STRETCH_FORMAT);
    save->de_masks = PEEK32( DE_MASKS);
    save->de_window_width = PEEK32( DE_WINDOW_WIDTH);
    save->de_control = PEEK32(DE_CONTROL);
    save->secondary_display_ctrl = PEEK32(SECONDARY_DISPLAY_CTRL);
    save->secondary_fb_address = PEEK32(SECONDARY_FB_ADDRESS);
    save->secondary_fb_width = PEEK32(SECONDARY_FB_WIDTH);
    save->secondary_horizontal_total = PEEK32(SECONDARY_HORIZONTAL_TOTAL);
    save->secondary_horizontal_sync = PEEK32(SECONDARY_HORIZONTAL_SYNC);
    save->secondary_vertical_total = PEEK32(SECONDARY_VERTICAL_TOTAL);
    save->secondary_vertical_sync = PEEK32(SECONDARY_VERTICAL_SYNC);
    save->secondary_scale = PEEK32(SECONDARY_SCALE);
    save->secondary_hwc_address = PEEK32(SECONDARY_HWC_ADDRESS);
    save->secondary_hwc_location = PEEK32(SECONDARY_HWC_LOCATION);
    save->secondary_hwc_color_12 = PEEK32(SECONDARY_HWC_COLOR_12);
    save->secondary_hwc_color_3 = PEEK32(SECONDARY_HWC_COLOR_3);
    save->display_control_750le = PEEK32(DISPLAY_CONTROL_750LE);
    save->secondary_auto_centering_tl = PEEK32(SECONDARY_AUTO_CENTERING_TL);
    save->secondary_auto_centering_br = PEEK32(SECONDARY_AUTO_CENTERING_BR);
    
    /* save palette ram */
    for(idx = 0; idx < 32; idx++) {
        save->secondary_palette_ram[idx] = PEEK32(
                SECONDARY_PALETTE_RAM + 4 * idx);
    }

    /* save vga */
    outb(p750Rec->PIOBase + 0x3D4, 0x88);
    save->vga_conf = inb(p750Rec->PIOBase + 0x3D5);

    LEAVE();
    
}
#if SMI_RANDR

void i2c_putbits_panel_750le(I2CBusPtr bus,int clock,int data)
{
	ddk750_I2CPutBits_panel(bus, clock, data);
}

void i2c_getbits_panel_750le(I2CBusPtr bus,int* clock,int* data)
{
	ddk750_I2CGetBits_panel(bus, clock, data);
}
#endif
