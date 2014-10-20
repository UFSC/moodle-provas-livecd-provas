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
#include "../ddk750/ddk750_power.h"
#include "smi_750_hw.h"
#include "smi_750_driver.h"
#include "../smi_dbg.h"

/*
IF THE FUNCTION WHOSE NAME POSTFIXED WITH 718, IT IS ONLY FOR SM718
IF THE FUNCTION WHOSE NAME POSTFIXED WITH 750, IT IS FOR SM750 AND SM718
IF THE FUNCTION WHOSE NAME POSTFIXED WITH 750o, IT IS ONLY FOR SM750
*/
/*****************************
	For FB Display
 *****************************/
void enable_pci_burst_718(SMIHWPtr pHw)
{
	/* Enable PCI_BURST for SM718 */
	unsigned long tmp = PEEK32(SYSTEM_CTRL);
	ddk750_set_mmio(pHw->pReg,pHw->devId,0);
	POKE32(SYSTEM_CTRL, tmp | 0x20000000);
}

/*Set the bpp and date channel and pitch for panel*/
void set_display_750(SMIHWPtr pHw, display_t display, int bpp, int pitch, int ulBaseAddress,int HDisplay, int HTotal )
{
	int SetValue;
	disp_output_t output;

	ddk750_set_mmio(pHw->pReg,pHw->devId,0);

	if(display == PANEL){
	    SetValue = PEEK32 (PANEL_DISPLAY_CTRL);
		
		SetValue = (bpp == 8 ? 
	            FIELD_SET(SetValue, PANEL_DISPLAY_CTRL, FORMAT, 8)
	            : (bpp == 16 ? 
	            FIELD_SET(SetValue, PANEL_DISPLAY_CTRL, FORMAT, 16)
	            : FIELD_SET(SetValue, PANEL_DISPLAY_CTRL, FORMAT, 32)));
			
		POKE32(PANEL_DISPLAY_CTRL, SetValue);
	
#if SMI_RANDR == 0	
		output = do_LCD1_PRI;
		ddk750_setLogicalDispOut(output);
	
		if(pHw->dual==1)
		{
			/* For mirror, vga and dvi use the primary channel data in common*/
			output = do_LCD2_PRI;
			ddk750_setLogicalDispOut(output);
		}
		
		POKE32(PANEL_FB_ADDRESS,
	          FIELD_SET(0, PANEL_FB_ADDRESS, STATUS, PENDING)
	        | FIELD_SET(0, PANEL_FB_ADDRESS, EXT, LOCAL)
	        | FIELD_VALUE(0, PANEL_FB_ADDRESS, ADDRESS, ulBaseAddress));
	
		POKE32(PANEL_FB_WIDTH,
	           FIELD_VALUE(0, PANEL_FB_WIDTH, WIDTH, pitch)
	           | FIELD_VALUE(0, PANEL_FB_WIDTH, OFFSET, pitch));		
#endif	
	}
	else if (display == CRT){
	    SetValue = PEEK32 (CRT_DISPLAY_CTRL);
		
		SetValue = (bpp == 8 ? 
	            FIELD_SET(SetValue, CRT_DISPLAY_CTRL, FORMAT, 8)
	            : (bpp == 16 ? 
	            FIELD_SET(SetValue, CRT_DISPLAY_CTRL, FORMAT, 16)
	            : FIELD_SET(SetValue, CRT_DISPLAY_CTRL, FORMAT, 32)));
			
		POKE32(CRT_DISPLAY_CTRL, SetValue);
#if SMI_RANDR == 0	
		output = do_CRT_SEC;//do_LCD2_SEC;
		ddk750_setLogicalDispOut(output);
#endif		
		POKE32(CRT_FB_WIDTH,
	           FIELD_VALUE(0, CRT_FB_WIDTH, WIDTH,pitch)
	           | FIELD_VALUE(0, CRT_FB_WIDTH, OFFSET, pitch));

                POKE32(CRT_FB_ADDRESS,
                      FIELD_SET(0, CRT_FB_ADDRESS, STATUS, PENDING)
                    | FIELD_SET(0, CRT_FB_ADDRESS, EXT, LOCAL)
                    | FIELD_VALUE(0, CRT_FB_ADDRESS, ADDRESS, ulBaseAddress));// adjust the right screen's FB.
	}
}

void save_reg_750(SMIHWPtr pHw)
{
	int 	i, j;
	SMI750HWPtr p750Hw = (SMI750HWPtr)pHw;
	SMI750RegPtr save = p750Hw->pRegSave;

	ENTER();
	ddk750_set_mmio(pHw->pReg,pHw->devId,0);


	/* save mmio */
	for (i = REG_SYS_HEAD, j = 0; i <= REG_SYS_TAIL; i += 4, j++){
		save->System[j] = PEEK32(i);
	}

	for (i = REG_PNL_HEAD, j = 0; i <= REG_PNL_TAIL; i += 4, j++){
		save->PanelControl[j] = PEEK32(i);
	}

	for (i = REG_CRT_HEAD, j = 0; i<=REG_CRT_TAIL; i+= 4, j++) {
		save->CRTControl[j] = PEEK32(i);
	}

	for(i=REG_PCUR_HEAD,j = 0;i<=REG_PCUR_TAIL;i+=4,j++){
		save->PriCursorControl[j] = PEEK32(i);
	}

	for(i=REG_SCUR_HEAD,j = 0;i<=REG_SCUR_TAIL;i+=4,j++){
		save->SecCursorControl[j] = PEEK32(i);
	}	
		
	LEAVE();
}

void restore_reg_750(SMIHWPtr pHw)
{
	int 	i, j;
	SMI750HWPtr p750Hw = (SMI750HWPtr)pHw;
	SMI750RegPtr save = p750Hw->pRegSave;

	ENTER();
	ddk750_set_mmio(pHw->pReg,pHw->devId,0);

	/* restore mmio */
	for(i = REG_SYS_HEAD,j = 0 ;i<= REG_SYS_TAIL ;i+=4,j++)
	{
		POKE32(i,save->System[j]);
	}

	for(i = REG_PNL_HEAD, j = 0 ;i<= REG_PNL_TAIL ;i+=4,j++)
	{
		POKE32(i,save->PanelControl[j]);
	}

	for(i = REG_CRT_HEAD,j = 0 ;i<= REG_CRT_TAIL ;i+=4,j++)
	{
		POKE32(i,save->CRTControl[j]);
	}

	for(i = REG_PCUR_HEAD,j = 0 ;i<= REG_PCUR_TAIL ;i+=4,j++)
	{		 
		POKE32(i,save->PriCursorControl[j]);
	}


	for(i = REG_SCUR_HEAD,j = 0 ;i<= REG_SCUR_TAIL ;i+=4,j++)
	{		
		POKE32(i,save->SecCursorControl[j]);
	}		

	/* The followding is to restore fonts */ 	
	
	LEAVE();	
}

int get_vga_mode_750(SMIHWPtr pHw)
{
	ddk750_set_mmio(pHw->pReg,pHw->devId,0);
	return FIELD_GET(PEEK32(VGA_CONFIGURATION),VGA_CONFIGURATION,MODE);
}

/*****************************
	For Accelerate
 *****************************/

void wait_for_not_busy_750(SMIPtr pSmi)
{
	SMIHWPtr pHw = pSmi->pHardware;
    DWORD dwVal;
    int i;
	ddk750_set_mmio(pHw->pReg,pHw->devId,0);

    for (i = 0x1000000; i > 0; i--)
    {
        dwVal = PEEK32(SYSTEM_CTRL);
        if ((FIELD_GET(dwVal, SYSTEM_CTRL, DE_STATUS) == SYSTEM_CTRL_DE_STATUS_IDLE) &&
                (FIELD_GET(dwVal, SYSTEM_CTRL, DE_FIFO) == SYSTEM_CTRL_DE_FIFO_EMPTY) &&
                (FIELD_GET(dwVal, SYSTEM_CTRL, CSC_STATUS) == SYSTEM_CTRL_CSC_STATUS_IDLE) &&
                (FIELD_GET(dwVal, SYSTEM_CTRL, DE_MEM_FIFO ) == SYSTEM_CTRL_DE_MEM_FIFO_EMPTY) &&
                TRUE
           )
        {
            break;
        }
    }		
}

void init_i2c_750(SMIHWPtr pHw)
{
	ddk750_set_mmio(pHw->pReg,pHw->devId,0);
	/* set mux for the i2c scl/sda*/
	int val = PEEK32(GPIO_MUX);
	
	val = FIELD_SET(val,GPIO_MUX,31,GPIO);
	val = FIELD_SET(val,GPIO_MUX,30,GPIO);
	val = FIELD_SET(val,GPIO_MUX,18,GPIO);
	val = FIELD_SET(val,GPIO_MUX,17,GPIO);			
	POKE32(GPIO_MUX,val);

	/* enable GPIO gate*/
	enableGPIO(1);
}

void i2c_putbits_panel_750(I2CBusPtr bus,int clock,int data)
{
    ddk750_I2CPutBits_panel(bus, clock, data);
}

void i2c_putbits_crt_750(I2CBusPtr bus,int clock,int data)
{
	ddk750_I2CPutBits_crt(bus, clock, data);
}

void i2c_getbits_panel_750(I2CBusPtr bus,int* clock,int* data)
{
	ddk750_I2CGetBits_panel(bus, clock, data);
}

void i2c_getbits_crt_750(I2CBusPtr bus,int* clock,int* data)
{
	ddk750_I2CGetBits_crt(bus, clock, data);
}

void set_backlight_750(int x)
{
	ENTER();
	if (x == 1) {
		/*Enable PNL PLANE */
		POKE32(PANEL_DISPLAY_CTRL,FIELD_SET(PEEK32(PANEL_DISPLAY_CTRL),PANEL_DISPLAY_CTRL, PLANE,ENABLE));
		/*Turn on CRT PLANE */
		POKE32(CRT_DISPLAY_CTRL,FIELD_SET(PEEK32(CRT_DISPLAY_CTRL),CRT_DISPLAY_CTRL, PLANE,ENABLE));
	}
	else
	{	
		POKE32(PANEL_DISPLAY_CTRL,FIELD_SET(PEEK32(PANEL_DISPLAY_CTRL),PANEL_DISPLAY_CTRL,PLANE,DISABLE));
		POKE32(CRT_DISPLAY_CTRL,FIELD_SET(PEEK32(CRT_DISPLAY_CTRL),CRT_DISPLAY_CTRL,PLANE,DISABLE));
	}
	LEAVE();
}
