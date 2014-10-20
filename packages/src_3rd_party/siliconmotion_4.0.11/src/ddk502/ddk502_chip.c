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
#include "ddk502_voyager.h"
#include "ddk502_hardware.h"
#include "ddk502_power.h"
#include "ddk502_clock.h"
#include "ddk502_chip.h"
#include "ddk502_help.h"
#include "../smi_common.h"

#define MB(x) (x*0x100000) /* Don't use this macro if x is fraction number */

static logical_chip_type_t gLogicalChipType = SM_UNKNOWN;

/*
 * This function returns frame buffer memory size in Byte units.
 */
//_X_EXPORT unsigned long getFrameBufSize()
_X_EXPORT int ddk502_getFrameBufSize()
{
    //unsigned long sizeSymbol, memSize;
    unsigned long sizeSymbol;
    int memSize;

    sizeSymbol = FIELD_GET(peekRegisterDWord(DRAM_CTRL), DRAM_CTRL, SIZE);

    switch(sizeSymbol)
    {
        /* Default is set to the lowest memory setting (requested by driver). */
        default:
        case DRAM_CTRL_SIZE_2:  memSize = MB(2);  break; /* 2  Mega byte */
        case DRAM_CTRL_SIZE_4:  memSize = MB(4);  break; /* 4  Mega byte */
        case DRAM_CTRL_SIZE_8:  memSize = MB(8);  break; /* 8  Mega byte */
        case DRAM_CTRL_SIZE_16: memSize = MB(16); break; /* 16 Mega byte */
        case DRAM_CTRL_SIZE_32: memSize = MB(32); break; /* 32 Mega byte */
        case DRAM_CTRL_SIZE_64: memSize = MB(64); break; /* 64 Mega byte */
#if 0
        default:                memSize = MB(0);  break; /* 0  Mege byte */
#endif
    }

    return memSize;
}

/*
 * This function gets the Frame buffer location.
 *
 * Return:
 *      0   - Embedded
 *      1   - External
 */
unsigned char getFrameBufLocation()
{
    /* Check if the memory is external memory */
    if (FIELD_GET(peekRegisterDWord(DRAM_CTRL), DRAM_CTRL, EMBEDDED) == DRAM_CTRL_EMBEDDED_DISABLE)
        return 1;

    /* Embedded Memory */
    return 0;
}

extern unsigned long  revId502;
extern unsigned short devId502;

/*
 * This function returns the logical chip type defined in chip.h
 * It is one of the following: SM501, SM502, SM107, SM718, SM 750 or
 * SM_UNKNOWN.
 */
logical_chip_type_t getChipType()
{
    unsigned long physicalID, physicalRev, memSize;
    logical_chip_type_t chip;
/*
    physicalID =  FIELD_GET(peekRegisterDWord(DEVICE_ID), DEVICE_ID, DEVICE_ID);
    physicalRev = FIELD_GET(peekRegisterDWord(DEVICE_ID), DEVICE_ID, REVISION_ID);
    memSize = ddk502_getFrameBufSize();*/
    physicalID = devId502;//either 0x718 or 0x750
	physicalRev = revId502;

    if (physicalID == 0x0501)
    {
        if (physicalRev == 0xC0)
        {/*
            if (memSize == MB(4))
                chip = SM107;
            else*/
                chip = SM502;
        }
        else
        {
            chip = SM501;
        }
    }
    else if (physicalID == 0x0718)
    {
        chip = SM718;
    }
    else if (physicalID == 0x750)
    {
        chip = SM750;
    }
    else
    {
        chip = SM_UNKNOWN;
    }

    return chip;
}

/*
 * Return a char string name of the current chip.
 * It's convenient for application need to display the chip name.
 */
char * getChipTypeString()
{
    char * chipName;

    switch(getChipType())
    {
        case SM501:
            chipName = "SM501";
            break;
        case SM502:
            chipName = "SM502";
            break;
        case SM107:
            chipName = "SM107";
            break;
		case SM718:
            chipName = "SM718";
            break;
        case SM750:
            chipName = "SM750";
            break;
        default:
            chipName = "Unknown";
            break;
    }

    return chipName;
}

/*
 * Initialize a single chip and environment according to input parameters.
 *
 * Input: initchip_param_t structure.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 * Note:
 *      Caller needs to call the detectDevice and setCurrentDevice
 *      to set the device before calling this initChipParamEx.
 */
long initChipParamEx(initchip_param_t * pInitParam)
{
    unsigned long ulReg;
    
    /* Check if we know this chip */
    if ((gLogicalChipType = getChipType()) == SM_UNKNOWN)
        return (-1);
    
    /* For SM107, local memory bank size has to change from hardware
       default of 4 banks to 2 banks
    */
    if (gLogicalChipType == SM107)
    {
        ulReg = peekRegisterDWord(DRAM_CTRL);
        ulReg = FIELD_SET(ulReg, DRAM_CTRL, BANKS, 2);

        /* ulReg = FIELD_SET(ulReg, DRAM_CTRL, BLOCK_WRITE_PRECHARGE, 4); */
        /* ulReg = FIELD_SET(ulReg, DRAM_CTRL, WRITE_PRECHARGE, 1); */

        pokeRegisterDWord(DRAM_CTRL, ulReg);
    }

    /* Set power mode.
       Check parameter validity first.
       If calling function didn't set it up properly or set to some
       weird value, always default to to 0.
     */
    if (pInitParam->powerMode > 1)
        pInitParam->powerMode = 0;
    setPowerMode(pInitParam->powerMode);

    /* Set up memory clock. */
    setMemoryClock(pInitParam->memClock);

    /* Set up master clock */
    setMasterClock(pInitParam->masterClock);

    if (pInitParam->setAllEngOff == 1)
    {
        enable2DEngine(0);

        /* Disable Overlay, if a former application left it on */
        ulReg = peekRegisterDWord(VIDEO_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, VIDEO_DISPLAY_CTRL, PLANE, DISABLE); 
        pokeRegisterDWord(VIDEO_DISPLAY_CTRL, ulReg);

        /* Disable video alpha, if a former application left it on */
        ulReg = peekRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, VIDEO_ALPHA_DISPLAY_CTRL, PLANE, DISABLE); 
        pokeRegisterDWord(VIDEO_ALPHA_DISPLAY_CTRL, ulReg);

        /* Disable alpha plane, if a former application left it on */
        ulReg = peekRegisterDWord(ALPHA_DISPLAY_CTRL);
        ulReg = FIELD_SET(ulReg, ALPHA_DISPLAY_CTRL, PLANE, DISABLE); 
        pokeRegisterDWord(ALPHA_DISPLAY_CTRL, ulReg);

        /* Disable LCD hardware cursor, if a former application left it on */
        ulReg = peekRegisterDWord(PANEL_HWC_ADDRESS);
        ulReg = FIELD_SET(ulReg, PANEL_HWC_ADDRESS, ENABLE, DISABLE); 
        pokeRegisterDWord(PANEL_HWC_ADDRESS, ulReg);

        /* Disable CRT hardware cursor, if a former application left it on */
        ulReg = peekRegisterDWord(CRT_HWC_ADDRESS);
        ulReg = FIELD_SET(ulReg, CRT_HWC_ADDRESS, ENABLE, DISABLE); 
        pokeRegisterDWord(CRT_HWC_ADDRESS, ulReg);

        /* Disable ZV Port 0 */
        ulReg = peekRegisterDWord(ZV0_CAPTURE_CTRL);
        ulReg = FIELD_SET(ulReg, ZV0_CAPTURE_CTRL, CAP, DISABLE); 
        pokeRegisterDWord(ZV0_CAPTURE_CTRL, ulReg);

        /* Disable ZV Port 1 */
        ulReg = peekRegisterDWord(ZV1_CAPTURE_CTRL);
        ulReg = FIELD_SET(ulReg, ZV1_CAPTURE_CTRL, CAP, DISABLE); 
        pokeRegisterDWord(ZV1_CAPTURE_CTRL, ulReg);
    
        /* Disable ZV Port Power */
        enableZVPort(0);
    
        /* Disable both DMA Channel 0 and 1 */
        ulReg = peekRegisterDWord(DMA_ABORT_INTERRUPT);
        ulReg = FIELD_SET(ulReg, DMA_ABORT_INTERRUPT, ABORT_0, ABORT) |
                FIELD_SET(0, DMA_ABORT_INTERRUPT, ABORT_1, ABORT);
        pokeRegisterDWord(DMA_ABORT_INTERRUPT, ulReg);
    }
	else
	{
        enable2DEngine(1);
	}

    /* We can add more initialization as needed. */
    
    return 0;
}

/*
 * Initialize every chip and environment according to input parameters.
 *
 * Input: initchip_param_t structure.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 */
long initChipParam(initchip_param_t * pInitParam)
{
    unsigned short deviceId;
    unsigned char deviceIndex;
    long prevDevice, result = 0;

    /* Check if any SMI VGX family chip exist and alive */
    deviceId = detectDevices();
    if (deviceId == 0)
        return (-1);

    /* Save the currently set device. */
    prevDevice = getCurrentDevice();   
    
    /* Go through every device and do the initialization. */
    for (deviceIndex = 0; deviceIndex < (unsigned char)getNumOfDevices(); deviceIndex++)
    {
        /* Set the current adapter */
        setCurrentDevice(deviceIndex);
    
        /* Initialize a single chip. */
        if (initChipParamEx(pInitParam) != 0)
            result = -1;
    }
    
    /* Restore the previously set device. */
    setCurrentDevice(prevDevice);
        
    return result;
}

/*
 * Initialize a single chip with default parameters.
 *
 * Input: none.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 * Note:
 *      Caller needs to call the detectDevice and setCurrentDevice
 *      to set the device before calling this initChipEx.
 */
//long initChipEx()
_X_EXPORT long ddk502_initHw()
{
    initchip_param_t initParam;

    initParam.powerMode = 0;  /* Default to power mode 0 */
    initParam.memClock = MEMORY_DEFAULT; /* Default memory clock to 144MHz */
    initParam.masterClock = MEMORY_DEFAULT/2; /* Default master clock to half of mem clock */
    initParam.setAllEngOff = 0;

    return(initChipParamEx(&initParam));
}

/*
 * Initialize all available chips with default parameters.
 *
 * Input: none.
 *
 * Return: 0 (or NO_ERROR) if successful.
 *        -1 if fail.
 *
 * Note:
 *      This function initializes all the adapters at once.
 *      To initialize a selected adapters, please use initChipEx
 */
#if 0
_X_EXPORT long ddk502_initHw()
{
    initchip_param_t initParam;

    initParam.powerMode = 0;  /* Default to power mode 0 */
    initParam.memClock = MEMORY_DEFAULT; /* Default memory clock to 144MHz */
    initParam.masterClock = MEMORY_DEFAULT/2; /* Default master clock to half of mem clock */
    initParam.setAllEngOff = 1;

    return(initChipParam(&initParam));
}
#endif
