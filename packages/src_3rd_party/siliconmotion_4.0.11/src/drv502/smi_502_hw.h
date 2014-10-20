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

#ifndef  SMI_502_HW_INC
#define  SMI_502_HW_INC


//static VOID panelUseLCD (SMI502_Ptr pSmi, BOOL bEnable);
//static VOID panelUseCRT (SMI502_Ptr pSmi, BOOL bEnable);
//static void panel_wait_Vsync_502 (SMI502_Ptr pSmi, INT vsync_count);
//static VOID panel_power_sequence_502 (SMI502_Ptr pSmi, panel_state_t on_off, INT vsync_delay);
//static LONG find_clock_502 (SMI502_Ptr pSmi,LONG requested_clock, clock_select_t * clock, display_t display);
//static VOID set_DPMS_502 (SMI502_Ptr pSmi, DPMS_t state);
//static VOID set_power_502 (SMI502_Ptr pSmi, ULONG nGates, ULONG Clock, int control_value);

void save_reg_502(SMIHWPtr pHw);
long wait_for_not_busy_502(SMIPtr);
void set_display_502(SMIHWPtr pHw, display_t channel, int bpp, int pitch);
void init_i2c_502(SMIHWPtr pHw);

void i2c_putbits_panel_502(I2CBusPtr bus,int clock,int data);
void i2c_putbits_crt_502(I2CBusPtr bus,int clock,int data);
void i2c_getbits_panel_502(I2CBusPtr bus,int* clock,int* data);
void i2c_getbits_crt_502(I2CBusPtr bus,int* clock,int* data);
void set_backlight_502(int x);

extern void ddk502_I2CPutBits_panel(I2CBusPtr bus,int clock,int data);
extern void ddk502_I2CGetBits_panel(I2CBusPtr bus,int* clock,int* data);
extern void ddk502_I2CPutBits_crt(I2CBusPtr bus,int clock,int data);
extern void ddk502_I2CGetBits_crt(I2CBusPtr bus,int* clock,int* data);

#endif   /* ----- #ifndef SMI_502_HW_INC  ----- */

