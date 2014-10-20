/*
Copyright (C) 1994-1999 The XFree86 Project, Inc.  All Rights Reserved.
Copyright (C) 2000 Silicon Motion, Inc.  All Rights Reserved.
Copyright (C) 2008 Francisco Jerez. All Rights Reserved. 
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

Except as contained in this notice, the names of The XFree86 Project and
Silicon Motion shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization from The XFree86 Project or Silicon Motion.
*/

#include "smi_712_hw.h"
#include "smi_712_driver.h"

static int	saved_console_reg = -1;
__inline__ CARD8
VGAIN8_INDEX_712(SMIHWPtr pHw, int indexPort, int dataPort, CARD8 index)
{
    if (0)//pSmi->IOBase)
    {
        MMIO_OUT8(pHw->IOBase, indexPort, index);
        return(MMIO_IN8(pHw->IOBase, dataPort));
    }
    else
    {
        outb(pHw->PIOBase + indexPort, index);
        return(inb(pHw->PIOBase + dataPort));
    }
}
__inline__ void
VGAOUT8_INDEX_712(SMIHWPtr pHw, int indexPort, int dataPort, CARD8 index, CARD8 data)
{
    if (0)//pSmi->IOBase)
    {
        MMIO_OUT8(pHw->IOBase, indexPort, index);
        MMIO_OUT8(pHw->IOBase, dataPort, data);
    }
    else
    {
        outb(pHw->PIOBase+ indexPort, index);
        outb(pHw->PIOBase+ dataPort, data);
    }
}

__inline__ CARD8
VGAIN8_712(SMIHWPtr pHw, int port)    //ilena: need change for 718
{
    if (0)//pSmi->IOBase)
    {
        return(MMIO_IN8(pHw->IOBase, port));
    }
    else
    {
        return(inb(pHw->PIOBase + port));
    }
}

__inline__ void
VGAOUT8_712(SMIHWPtr pHw, int port, CARD8 data)
{
    if (0)//pSmi->IOBase)
    {
        MMIO_OUT8(pHw->IOBase, port, data);
    }
    else
    {
        outb(pHw->PIOBase, data);
    }
}
void enable_mmio_712(ScrnInfoPtr pScrn)
{
        CARD8 tmp;
        ENTER();
        SMIPtr pSmi = SMIPTR(pScrn);
        SMIHWPtr pHw = pSmi->pHardware;
        SMI712HWPtr p712Hw = (SMI712HWPtr)pHw;

        /* Enable linear mode */
        outb(pHw->PIOBase + VGA_SEQ_INDEX, SCR18);
        tmp = inb(pHw->PIOBase + VGA_SEQ_DATA);
        p712Hw->SR18Value = tmp;
        /* !!To enable MMIO the following must be set prior to setting SCR18[0]: SCR17[1] = 1!! */
        /* Before enable the SCR18[0], all MMIO cant access*/
        /* So, enable the SCR18[0] by IO access first! */
        outb(pHw->PIOBase + VGA_SEQ_DATA, tmp | 0x11);

        /* Enable 2D/3D Engine and Video Processor */
        outb(pHw->PIOBase + VGA_SEQ_INDEX, PDR21);
        tmp = inb(pHw->PIOBase + VGA_SEQ_DATA);
        p712Hw->SR21Value = tmp;
        outb(pHw->PIOBase + VGA_SEQ_DATA, tmp & ~0x03);
    LEAVE();
}

void
disable_mmio_712(ScrnInfoPtr pScrn)
{
        SMIPtr pSmi = SMIPTR(pScrn);
        SMI712HWPtr p712Hw = (SMI712HWPtr)(pSmi->pHardware);
        SMIHWPtr pHw = pSmi->pHardware;
        ENTER();

        /* Disable 2D/3D Engine and Video Processor */
        outb(pHw->PIOBase + VGA_SEQ_INDEX, PDR21);
        outb(pHw->PIOBase + VGA_SEQ_DATA, p712Hw->SR21Value);	/* PDR#521 */

        /* Disable linear mode */
        outb(pHw->PIOBase + VGA_SEQ_INDEX, SCR18);
        outb(pHw->PIOBase + VGA_SEQ_DATA, p712Hw->SR18Value);	/* PDR#521 */

        LEAVE();
}

void set_display_712(SMIHWPtr pHw,display_t channel, ScrnInfoPtr pScrn,  uint32_t offset, int x, int y)
{
	ENTER();
	SMI712HWPtr p712Hw = (SMI712HWPtr)pHw;
        SMIPtr pSmi = SMIPTR(pScrn);
	SMI712Ptr pSmi712 = (SMI712Ptr)pSmi;
        SMI712RegPtr mode = p712Hw->pRegMode;
		
	CARD8 tmp;
	CARD32 Pitch,Width;
	CARD32 Base;


	/* Scratch Pad Register 1 */
	tmp = 0;
	tmp = VGAIN8_INDEX_712(pHw,VGA_SEQ_INDEX,VGA_SEQ_DATA,GPR70);
	XERR("MONK,GET 3C4:70 == %02x\n",tmp);
	tmp &= 0x0f;
	tmp |= 0x50;		
	VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, GPR70, tmp);
	XERR("MONK,after set ,GET 3C4:70 == %02x\n",VGAIN8_INDEX_712(pHw,VGA_SEQ_INDEX,VGA_SEQ_DATA,GPR70));

	tmp = 0;
	Pitch = (pScrn->displayWidth * pSmi->Bpp);	/* it == pixel unit divided with 8 */
	Width = (x * pSmi->Bpp);					/* as caculated like pitch */

	/* set the TFT color */
	tmp = VGAIN8_INDEX_712(pHw,VGA_SEQ_INDEX,VGA_SEQ_DATA,FPR30)&0x8f;
	VGAOUT8_INDEX_712(pHw,VGA_SEQ_INDEX,VGA_SEQ_DATA,FPR30,tmp | (pSmi->TFTColor<<4));

	switch (pScrn->bitsPerPixel)
	{
		case 8:
			WRITE_VPR(pHw, 0x00, 0x00000000);
			break;
		case 16:
			WRITE_VPR(pHw, 0x00, 0x00020000);
			break;
		case 24:
			WRITE_VPR(pHw, 0x00, 0x00040000);
			break;
		case 32:
			WRITE_VPR(pHw, 0x00, 0x00030000);
			break;
	}

	Base = offset + (pScrn->frameX0 + pScrn->frameY0* pScrn->displayWidth) * pSmi->Bpp;
	if(pHw->devId == SMI_LYNX3DM)
	{		
		Base = (Base + 15) & ~15;
		while ((Base % pSmi->Bpp) > 0)
			Base -= 16;
	}else
	{
		Base = (Base + 7) & ~7;
		while ((Base % pSmi->Bpp) > 0)
			Base -= 8;
	}
	Base >>= 3;
	Width >>= 3;
	Pitch >>= 3;

	if((pHw->dual > 1)&&(channel == PANEL))
	{
	    /* LCD channel pitch and base address  */
		CARD8 SR40,SR41,SR42,SR43,SR45;
		XERR("LCD and CRT double head");
	    /* FIFO1 read start address */
	    SR40 = Base & 0x000000FF;
	    SR41 = (Base & 0x0000FF00) >> 8;

	    /* FIFO2 read start address */
	    SR42 = Base & 0x000000FF;
	    SR43 = (Base & 0x0000FF00) >> 8;

	    /* FIFO1/2 read start address overflow */
	    if(pHw->Chipset == SMI_LYNX3DM)
    		SR45 = (Base & 0x000F0000) >> 16 | (Base & 0x000F0000) >> 16 << 4;
	    else
    		SR45 = (SR45 & 0xC0) |
		    (Base & 0x00070000) >> 16 | (Base & 0x00070000) >> 16 << 3;

	    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, FPR40, SR40);
	    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, FPR41, SR41);
	    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, FPR42, SR42);
	    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, FPR43, SR43);
	    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, FPR45, SR45);    

	    /* Offset  of LCD channel*/
	    SR45 = (SR45 & 0x3f)|((Pitch & 0x300 ) >> 2);
	    VGAOUT8_INDEX_712(pHw,VGA_SEQ_INDEX,VGA_SEQ_DATA,FPR44,Pitch & 0xff);
	    VGAOUT8_INDEX_712(pHw,VGA_SEQ_INDEX,VGA_SEQ_DATA,FPR45,SR45);

		/* For create dual view mode, it will be clone without following statement */
		VGAOUT8_INDEX_712(pHw,VGA_SEQ_INDEX,VGA_SEQ_DATA,PDR21,0x20);/**/
		VGAOUT8_INDEX_712(pHw,VGA_SEQ_INDEX,VGA_SEQ_DATA,FPR31,0xc3);/*0xC0 for clone mode*/
	}
	else if((channel == CRT)||(pHw->dual <= 1))
	{
		/* CRT and single head */	    
		XERR("CRT and single head");
		WRITE_VPR(pHw, 0x0C, Base);
		WRITE_VPR(pHw,0x10,(Width<<16)|(Pitch&0xffff));
	}
	if(pHw->Chipset == SMI_LYNX3DM)
	{
	    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, PDR22, 0x0);
	    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, FPR31, 0x2);
	}
	LEAVE();
}
int get_vga_mode_712(SMIHWPtr pHw)
{
	return 0;
//    return peek32(0x88);
}

void save_dpr_address(SMIHWPtr pHw)
{

}

/*
 * This function performs the inverse of the restore function: It saves all the
 * standard and extended registers that we are going to modify to set up a video
 * mode.
 */
void save_reg_712 (SMIHWPtr pHw)
{
        ENTER();
        ScrnInfoPtr pScrn;
        SMIPtr pSmi = HWPSMI(pHw);
        pScrn = SMIPSI(pSmi);
        CARD8 tmp;
                
        vgaHWPtr hwp = VGAHWPTR (pScrn);
        vgaRegPtr vgaSavePtr = &hwp->SavedReg;
        SMI712HWPtr p712Hw = (SMI712HWPtr)pHw;
        SMI712RegPtr save = p712Hw->pRegSave;

        vgaHWSetStdFuncs(hwp);
        vgaHWGetIOBase(hwp);
		
        int		i;
        CARD32	offset;
        int		vgaIOBase  = hwp->IOBase;
        int		vgaCRIndex = vgaIOBase + VGA_CRTC_INDEX_OFFSET;
        int		vgaCRData  = vgaIOBase + VGA_CRTC_DATA_OFFSET;

        /* Save the standard VGA registers */
        vgaHWSave(pScrn, vgaSavePtr, VGA_SR_ALL);
        save->smiDACMask = VGAIN8_712(p712Hw, VGA_DAC_MASK);
        VGAOUT8_712(pHw, VGA_DAC_READ_ADDR, 0);
        for (i = 0; i < 256; i++) {
                save->smiDacRegs[i][0] = VGAIN8_712(p712Hw, VGA_DAC_DATA);
                save->smiDacRegs[i][1] = VGAIN8_712(p712Hw, VGA_DAC_DATA);
                save->smiDacRegs[i][2] = VGAIN8_712(p712Hw, VGA_DAC_DATA);
        }

	/* Save Fonts */
        for (i = 0, offset = 2; i < 8192; i++, offset += 8)
    		save->smiFont[i] = *((unsigned char *)(pSmi->pHardware->pMem) + offset);

    /* Now we save all the extended registers we need. */
        save->SR17 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x17);
        save->SR18 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x18);

        save->SR20 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x20);
        save->SR21 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, PDR21);
        save->SR22 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, PDR22);
        save->SR23 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x23);
        save->SR24 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x24);

        save->SR31 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, FPR31);
        save->SR32 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, FPR32);

        save->SR66 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, CCR66);
        save->SR68 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x68);
        save->SR69 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x69);
        save->SR6A = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6A);
        save->SR6B = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6B);
        save->SR6C = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6C);
        save->SR6D = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6D);

        save->SR81 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x81);
        save->SRA0 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, FPRA0);

#if 1
    //if (pSmi->DualView)
	{
		/* dualhead stuff */
		save->SR40 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x40);
		save->SR41 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x41);
		save->SR42 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x42);
		save->SR43 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x43);
		save->SR44 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x44);
		save->SR45 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x45);
		save->SR48 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x48);
		save->SR49 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x49);
		save->SR4A = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x4A);
		save->SR4B = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x4B);
		save->SR4C = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x4C);

		save->SR50 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x50);
		save->SR51 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x51);
		save->SR52 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x52);
		save->SR53 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x53);
		save->SR54 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x54);
		save->SR55 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x55);
		save->SR56 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x56);
		save->SR57 = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x57);
		save->SR5A = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x5A);

		/* PLL2 stuff */
		save->SR6E = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6E);
		save->SR6F = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6F);
    }
#endif
	/* note the judgement ,monk */
	{
		/* Save common registers */
		save->CR30 = VGAIN8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x30);
		save->CR3A = VGAIN8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x3A);
		for (i = 0; i < 15; i++) {
		    save->CR90[i] = VGAIN8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x90 + i);
		}
		for (i = 0; i < 14; i++) {
		    save->CRA0[i] = VGAIN8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0xA0 + i);
		}

		/* Save primary registers */
		VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x9E, save->CR90[14] & ~0x20);

		save->CR33 = VGAIN8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x33);
		for (i = 0; i < 14; i++) {
		    save->CR40[i] = VGAIN8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x40 + i);
		}
		save->CR9F = VGAIN8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x9F);

		/* Save secondary registers */
		VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x9E, save->CR90[14] | 0x20);
		save->CR33_2 = VGAIN8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x33);
		for (i = 0; i < 14; i++) {
		    save->CR40_2[i] = VGAIN8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x40 + i);
		}
		save->CR9F_2 = VGAIN8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x9F);

		/* PDR#1069 */
		VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x9E, save->CR90[14]);
    }

        save->DPR10 = READ_DPR(pHw, 0x10);
        save->DPR1C = READ_DPR(pHw, 0x1C);
        save->DPR20 = READ_DPR(pHw, 0x20);
        save->DPR24 = READ_DPR(pHw, 0x24);
        save->DPR28 = READ_DPR(pHw, 0x28);
        save->DPR2C = READ_DPR(pHw, 0x2C);
        save->DPR30 = READ_DPR(pHw, 0x30);
        save->DPR3C = READ_DPR(pHw, 0x3C);
        save->DPR40 = READ_DPR(pHw, 0x40);
        save->DPR44 = READ_DPR(pHw, 0x44);

        save->VPR00 = READ_VPR(pHw, 0x00);
        save->VPR0C = READ_VPR(pHw, 0x0C);
        save->VPR10 = READ_VPR(pHw, 0x10);

        save->CPR00 = READ_CPR(pHw, 0x00);

	vgaHWCopyReg(&hwp->ModeReg, vgaSavePtr);

    LEAVE();
}

/*
 * This function is used to restore a video mode. It writes out all of the
 * standard VGA and extended registers needed to setup a video mode.
 */
void restore_reg_712(SMIHWPtr pHw)
{
        ENTER();
        ScrnInfoPtr pScrn;
        SMIPtr pSmi = HWPSMI(pHw);
        pScrn = SMIPSI(pSmi);
                
        vgaHWPtr hwp = VGAHWPTR (pScrn);
        vgaRegPtr vgaSavePtr = &hwp->SavedReg;
        SMI712HWPtr p712Hw = (SMI712HWPtr)pHw;
        SMI712RegPtr restore = p712Hw->pRegSave;
		
        int		i;
        CARD8	tmp;
        CARD32	offset;
        int		vgaIOBase  = hwp->IOBase;
        int		vgaCRIndex = vgaIOBase + VGA_CRTC_INDEX_OFFSET;
        int		vgaCRData  = vgaIOBase + VGA_CRTC_DATA_OFFSET;

        vgaHWProtect(pScrn, TRUE);

        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x17, restore->SR17);
        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x18, restore->SR18);

        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x20, restore->SR20);
        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, PDR21, restore->SR21);
        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, PDR22, restore->SR22);
        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x23, restore->SR23);
        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x24, restore->SR24);

        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, FPR31, restore->SR31);
        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, FPR32, restore->SR32);

        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, CCR66, restore->SR66);
        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x68, restore->SR68);
        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x69, restore->SR69);
        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6A, restore->SR6A);
        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6B, restore->SR6B);
        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6C, restore->SR6C);
        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6D, restore->SR6D);

        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x81, restore->SR81);
        VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0xA0, restore->SRA0);

	{
		/* Restore the standard VGA registers */
		vgaHWRestore(pScrn, vgaSavePtr, VGA_SR_ALL);
		
		if (restore->smiDACMask)
		{
		        VGAOUT8_712(pHw, VGA_DAC_MASK, restore->smiDACMask);
		}
		else
		{
		        VGAOUT8_712(pHw, VGA_DAC_MASK, 0xFF);
		}
		VGAOUT8_712(pHw, VGA_DAC_WRITE_ADDR, 0);
		for (i = 0; i < 256; i++) {
                        VGAOUT8_712(pHw, VGA_DAC_DATA, restore->smiDacRegs[i][0]);
                        VGAOUT8_712(pHw, VGA_DAC_DATA, restore->smiDacRegs[i][1]);
                        VGAOUT8_712(pHw, VGA_DAC_DATA, restore->smiDacRegs[i][2]);
		}
		for (i = 0, offset = 2; i < 8192; i++, offset += 8) {
            		    *((unsigned char *)(pSmi->pHardware->pMem) + offset) = restore->smiFont[i];
		}
		    /* Restore secondary registers */
		    VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x9E,
				  restore->CR90[14] | 0x20);

		    VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x33, restore->CR33_2);
		    for (i = 0; i < 14; i++) {
			VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x40 + i,
				      restore->CR40_2[i]);
		    }
		    VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x9F, restore->CR9F_2);

		    /* Restore primary registers */
		    VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x9E,
				  restore->CR90[14] & ~0x20);

		    VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x33, restore->CR33);
		    for (i = 0; i < 14; i++) {
			VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x40 + i,
				      restore->CR40[i]);
		    }
		    VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x9F, restore->CR9F);

		    /* Restore common registers */
		    VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x30, restore->CR30);
		    VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x3A, restore->CR3A);

		    for (i = 0; i < 15; i++)
			VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0x90 + i,
				      restore->CR90[i]);

		    for (i = 0; i < 14; i++)
			VGAOUT8_INDEX_712(pHw, vgaCRIndex, vgaCRData, 0xA0 + i,
				      restore->CRA0[i]);
	
	//	if (pSmi->DualView) {
		{    /* dualhead stuff */
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x40, restore->SR40);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x41, restore->SR41);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x42, restore->SR42);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x43, restore->SR43);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x44, restore->SR44);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x45, restore->SR45);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x48, restore->SR48);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x49, restore->SR49);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x4A, restore->SR4A);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x4B, restore->SR4B);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x4C, restore->SR4C);

		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x50, restore->SR50);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x51, restore->SR51);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x52, restore->SR52);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x53, restore->SR53);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x54, restore->SR54);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x55, restore->SR55);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x56, restore->SR56);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x57, restore->SR57);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x5A, restore->SR5A);

		    /* PLL2 stuff */
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6E, restore->SR6E);
		    VGAOUT8_INDEX_712(pHw, VGA_SEQ_INDEX, VGA_SEQ_DATA, 0x6F, restore->SR6F);
		}
    }

    /* Reset the graphics engine */
    WRITE_DPR(pHw, 0x10, restore->DPR10);
    WRITE_DPR(pHw, 0x1C, restore->DPR1C);
    WRITE_DPR(pHw, 0x20, restore->DPR20);
    WRITE_DPR(pHw, 0x24, restore->DPR24);
    WRITE_DPR(pHw, 0x28, restore->DPR28);
    WRITE_DPR(pHw, 0x2C, restore->DPR2C);
    WRITE_DPR(pHw, 0x30, restore->DPR30);
    WRITE_DPR(pHw, 0x3C, restore->DPR3C);
    WRITE_DPR(pHw, 0x40, restore->DPR40);
    WRITE_DPR(pHw, 0x44, restore->DPR44);

    /* write video controller regs */
    WRITE_VPR(pHw, 0x00, restore->VPR00);
    WRITE_VPR(pHw, 0x0C, restore->VPR0C);
    WRITE_VPR(pHw, 0x10, restore->VPR10);

    WRITE_CPR(pHw, 0x00, restore->CPR00);
	
    vgaHWProtect(pScrn, FALSE);

    LEAVE();
}

extern SMI_GEReset(ScrnInfoPtr pScrn,int from_timeout,int line,char *file);
void wait_for_not_busy_712(SMIPtr pSmi)
{
        ScrnInfoPtr pScrn;
        pScrn = SMIPSI(pSmi);
        SMIHWPtr pHw = pSmi->pHardware;
 
    do
    {								
    	int	loop = MAXLOOP;									
    	mem_barrier();
    									
	    int	status;			
	    for (status = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX,VGA_SEQ_DATA, 0x16);
                loop && (status & 0x18) != 0x10;
                status = VGAIN8_INDEX_712(pHw, VGA_SEQ_INDEX,VGA_SEQ_DATA, 0x16), loop--);
									
    	if (loop <= 0)							
    	    SMI_GEReset(pScrn, 1, __LINE__, __FILE__);
    }while (0); 

}
void set_backlight_712(int x)
{
	ENTER();
	LEAVE();
}

