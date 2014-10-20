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

#include "ddk502/ddk502_power.h"
#include "ddk502/ddk502_regdc.h"
#include "ddk502/ddk502_regdma.h"
#include "ddk502/ddk502_regsc.h"
#include "ddk502/ddk502_regzv.h"
#include "../smi_common.h"
#include "../smi_driver.h"
#include "smi_502_driver.h"
#include "smi_502_hw.h"
#include "../smi_dbg.h"


void set_display_502(SMIHWPtr pHw, display_t channel, int bpp, int pitch)
{
    
	int SetValue;
	disp_output_t output;	
   	SetValue = ddk502_PEEK32 (PANEL_DISPLAY_CTRL);

	if (channel == PANEL)
	{
	    SetValue |= (bpp == 8? 
	        FIELD_SET (0, PANEL_DISPLAY_CTRL, FORMAT, 8)
	        : (bpp == 16? 
	        FIELD_SET (0, PANEL_DISPLAY_CTRL, FORMAT, 16)
		    : FIELD_SET (0, PANEL_DISPLAY_CTRL, FORMAT, 32)));

	    SetValue |= (FIELD_SET (SetValue, CRT_DISPLAY_CTRL, SELECT, PANEL));//???why crt display ctrl>???
	    ddk502_POKE32(PANEL_DISPLAY_CTRL, SetValue);
		
#if SMI_RANDR == 0
	    ddk502_POKE32 (PANEL_FB_WIDTH,
	            FIELD_VALUE (0, PANEL_FB_WIDTH, WIDTH, pitch) |
	            FIELD_VALUE (0, PANEL_FB_WIDTH, OFFSET, pitch));
#endif	
		if(bpp == 8)
	         {
			ddk502_POKE32(PANEL_PALETTE_RAM, 0x00000000);	/* PANEL Palette       */
			ddk502_POKE32(PANEL_PALETTE_RAM,+ 4, 0x00FFFFFF);	/* PANEL Palette       */
	         }
            	if(pHw->dual ==1)
            	{
            		/* For mirror, vga and dvi use the primary channel data in common*/
            		output = PANEL_CRT_SIMUL;
                           setLogicalDispOutput( output);
            	}
            	else{
            		output = PANEL_CRT_DUAL;
                           setLogicalDispOutput( output);
            	}
	}
	else if (channel == CRT)
	{

	    SetValue |= (bpp == 8?
		        FIELD_SET (0, CRT_DISPLAY_CTRL, FORMAT, 8)
		        : (bpp == 16?
			    FIELD_SET (0, CRT_DISPLAY_CTRL, FORMAT, 16)
			    : FIELD_SET (0, CRT_DISPLAY_CTRL, FORMAT, 32)));
	    SetValue |= (FIELD_SET(SetValue,CRT_DISPLAY_CTRL,SELECT,CRT));//??ilena not sure
	    ddk502_POKE32(CRT_DISPLAY_CTRL, SetValue);
		if(bpp == 8)
		{
			ddk502_POKE32(CRT_PALETTE_RAM, 0x00000000);	/* CRT Palette       */
			ddk502_POKE32(CRT_PALETTE_RAM,+ 4, 0x00FFFFFF);	/* CRT Palette       */
		}
	}

}


long wait_for_not_busy_502(SMIPtr pSmi)
{
    unsigned long i = 0x1000000;
    while (i--)
    {
        unsigned long dwVal = ddk502_PEEK32(CMD_INTPR_STATUS);//PEEK_32(CMD_INTPR_STATUS);
        if ((FIELD_GET(dwVal, CMD_INTPR_STATUS, 2D_ENGINE)      == CMD_INTPR_STATUS_2D_ENGINE_IDLE) &&
            (FIELD_GET(dwVal, CMD_INTPR_STATUS, 2D_FIFO)        == CMD_INTPR_STATUS_2D_FIFO_EMPTY) &&
            (FIELD_GET(dwVal, CMD_INTPR_STATUS, 2D_SETUP)       == CMD_INTPR_STATUS_2D_SETUP_IDLE) &&
            (FIELD_GET(dwVal, CMD_INTPR_STATUS, CSC_STATUS)     == CMD_INTPR_STATUS_CSC_STATUS_IDLE) &&
            (FIELD_GET(dwVal, CMD_INTPR_STATUS, 2D_MEMORY_FIFO) == CMD_INTPR_STATUS_2D_MEMORY_FIFO_EMPTY) &&
            (FIELD_GET(dwVal, CMD_INTPR_STATUS, COMMAND_FIFO)   == CMD_INTPR_STATUS_COMMAND_FIFO_EMPTY))
        {
            return 0; /* Return because engine idle */
        }
    }

    return -1; /* Return because time out */
}
static void enable_GPIO_502(uint32_t enable)
{
    uint32_t gate;

    /* Enable GPIO Gate */
    gate = ddk502_PEEK32(CURRENT_POWER_GATE);
    if (enable)
        gate = FIELD_SET(gate, CURRENT_POWER_GATE, GPIO_PWM_I2C, DISABLE);        
    else
        gate = FIELD_SET(gate, CURRENT_POWER_GATE, GPIO_PWM_I2C, ENABLE);
    
    ddk502_setCurrentGate(gate);
}

void init_i2c_502(SMIHWPtr pHw)
{
	/* set mux for the i2c scl/sda*/
	int val = ddk502_PEEK32(GPIO_MUX_HIGH);
	val = FIELD_SET(val,GPIO_MUX_HIGH,63,GPIO);
	val = FIELD_SET(val,GPIO_MUX_HIGH,62,GPIO);
	val = FIELD_SET(val,GPIO_MUX_HIGH,46,GPIO);
	val = FIELD_SET(val,GPIO_MUX_HIGH,47,GPIO);			
	ddk502_POKE32(GPIO_MUX_HIGH,val);
	
	enable_GPIO_502(1);
	
	/* enable GPIO gate*/
}

void i2c_putbits_panel_502(I2CBusPtr bus,int clock,int data)
{
    ddk502_I2CPutBits_panel(bus, clock, data);
}

void i2c_putbits_crt_502(I2CBusPtr bus,int clock,int data)
{
	ddk502_I2CPutBits_crt(bus, clock, data);
}

void i2c_getbits_panel_502(I2CBusPtr bus,int* clock,int* data)
{
	ddk502_I2CGetBits_panel(bus, clock, data);
}

void i2c_getbits_crt_502(I2CBusPtr bus,int* clock,int* data)
{
	ddk502_I2CGetBits_crt(bus, clock, data);
}
void set_backlight_502(int x)
{
	ENTER();
	if (x == 1) {
		/*Enable PNL PLANE */
		ddk502_POKE32(PANEL_DISPLAY_CTRL,FIELD_SET(ddk502_PEEK32(PANEL_DISPLAY_CTRL),PANEL_DISPLAY_CTRL,PLANE,ENABLE));
		/*Enable CRT PLANE */
		ddk502_POKE32(CRT_DISPLAY_CTRL,FIELD_SET(ddk502_PEEK32(CRT_DISPLAY_CTRL),CRT_DISPLAY_CTRL,PLANE,ENABLE));
	}
	else
	{	
		ddk502_POKE32(PANEL_DISPLAY_CTRL,FIELD_SET(ddk502_PEEK32(PANEL_DISPLAY_CTRL),PANEL_DISPLAY_CTRL,PLANE,DISABLE));
		ddk502_POKE32(CRT_DISPLAY_CTRL,FIELD_SET(ddk502_PEEK32(CRT_DISPLAY_CTRL),CRT_DISPLAY_CTRL,PLANE,DISABLE));
	}
	LEAVE();
}

