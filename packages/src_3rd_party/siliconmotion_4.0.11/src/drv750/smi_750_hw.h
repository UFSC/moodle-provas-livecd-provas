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
#ifndef  SMI_750_HW_INC
#define  SMI_750_HW_INC

//simple save --wish to add the function to DDK
#define REG_SYS_HEAD    0x0
#define REG_SYS_TAIL    0x88
#define REG_PNL_HEAD    0x80000
#define REG_PNL_TAIL    0x80034
#define REG_CRT_HEAD    0x80200
#define REG_CRT_TAIL    0x80228
#define REG_ALPH_HEAD   0x80100
#define REG_ALPH_TAIL   0x80114
#define REG_PCUR_HEAD   0x800f0
#define REG_PCUR_TAIL   0x800fc
#define REG_SCUR_HEAD   0x80230
#define REG_SCUR_TAIL   0x80240

typedef struct
{
    CARD32 System[(REG_SYS_TAIL - REG_SYS_HEAD)/4+1];    //0x00 -- 0x88
    CARD32 PanelControl[(REG_PNL_TAIL - REG_PNL_HEAD)/4 +1];    //0x80000 -- 0x80034
    CARD32 CRTControl[(REG_CRT_TAIL - REG_CRT_HEAD)/4 + 1];     //0x80200 -- 0x80228
    CARD32 AlphaControl[(REG_ALPH_TAIL - REG_ALPH_HEAD)/4+1];     //0x80100 -- 0x80114
    CARD32 PriCursorControl[(REG_PCUR_TAIL - REG_PCUR_HEAD)/4+1];  
    CARD32 SecCursorControl[(REG_SCUR_TAIL - REG_SCUR_HEAD)/4+1];
    
} SMI750RegRec, *SMI750RegPtr;

#define PANEL_DATA  0
#define CRT_DATA    1
#define VGA_DATA    2

/* Memory Clock Default Value. It can be calculated by dividing 
   the chip clock by certain valid values, which are: 1, 2, 3, and 4 */
#define DEFAULT_MEMORY_CLK          290


void enable_pci_burst_718(SMIHWPtr);
void save_reg_750(SMIHWPtr);
void restore_reg_750(SMIHWPtr);
int get_vga_mode_750(SMIHWPtr);
void wait_for_not_busy_750(SMIPtr);
void i2c_putbits_panel_750(I2CBusPtr bus,int clock,int data);
void i2c_putbits_crt_750(I2CBusPtr bus,int clock,int data);
void i2c_getbits_panel_750(I2CBusPtr bus,int* clock,int* data);
void i2c_getbits_crt_750(I2CBusPtr bus,int* clock,int* data);

void set_backlight_750(int x);

extern void ddk750_I2CPutBits_panel(I2CBusPtr bus,int clock,int data);
extern void ddk750_I2CGetBits_panel(I2CBusPtr bus,int* clock,int* data);
extern void ddk750_I2CPutBits_crt(I2CBusPtr bus,int clock,int data);
extern void ddk750_I2CGetBits_crt(I2CBusPtr bus,int* clock,int* data);
void init_i2c_750(SMIHWPtr pHw);
#endif   /* ----- #ifndef SMI_750_HW_INC  ----- */

