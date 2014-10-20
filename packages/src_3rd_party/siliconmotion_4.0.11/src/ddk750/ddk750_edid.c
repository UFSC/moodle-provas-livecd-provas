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
#include <math.h>
#include "../smi_common.h"
#include "ddk750_edid.h"
#include "ddk750_hwi2c.h"
#include "ddk750_swi2c.h"
#include "ddk750_chip.h"

/* Enable this one to print the VDIF timing when debug is enabled. */
//#define ENABLE_DEBUG_PRINT_VDIF

/****************************************************************
 * Configuration setting
 ****************************************************************/
/* I2C Address of each Monitor. Currently, there is only one i2c bus and
   both display devices will responds to 0xA0 address with analog CRT as the first priority
   The second version of evaluation board will separate the i2c bus, so that each i2c bus
   corresponds to one display devices.
   The new monitor devices (at least the tested DVI monitor) also corresponds to 0xA2, 
   which is temporarily used in this DDK. */
#define EDID_DEVICE_I2C_ADDRESS             0xA0

/* GPIO used for the I2C on the PANEL_PATH size  */
#define EDID_PANEL_I2C_SCL                  DEFAULT_I2C_SCL
#define EDID_PANEL_I2C_SDA                  DEFAULT_I2C_SDA

/* GPIO used for the I2C on the CRT_PATH size.
   These GPIO pins only available in the Evaluation Board version 2.2.
   Need to find out which pins are used for the CRT_PATH i2c. */
#define EDID_CRT_I2C_SCL                    DEFAULT_I2C_SCL
#define EDID_CRT_I2C_SDA                    DEFAULT_I2C_SDA

#define TOTAL_EDID_REGISTERS                128

typedef struct _est_timing_mode_t
{
    uint32_t x;        /* Mode Width */
    uint32_t y;        /* Mode Height */
    uint32_t hz;       /* Refresh Rate */
    unsigned char source;   /* Source:  0 - VESA
                                        1 - IBM
                                        2 - Apple
                             */
}
est_timing_mode_t;

/* These values only applies to EDID Version 1 */
static est_timing_mode_t establishTiming[3][8] =
{
    /* Established Timing 1 */
    {   
        { 800,  600, 60, 0},
        { 800,  600, 56, 0},
        { 640,  480, 75, 0},
        { 640,  480, 72, 0},
        { 640,  480, 67, 2},
        { 640,  480, 60, 1},
        { 720,  400, 88, 1},
        { 720,  400, 70, 1},
    },
    {
        {1280, 1024, 75, 0},
        {1024,  768, 75, 0},
        {1024,  768, 70, 0},
        {1024,  768, 60, 0},
        {1024,  768, 87, 1},
        { 832,  624, 75, 0},
        { 800,  600, 75, 0},
        { 800,  600, 72, 0},
    },
    {
        {   0,    0,  0, 0},
        {   0,    0,  0, 0},
        {   0,    0,  0, 0},
        {   0,    0,  0, 0},
        {   0,    0,  0, 0},
        {   0,    0,  0, 0},
        {   0,    0,  0, 0},
        {1152,  870, 75, 2},
    }
};

static void printVdif(
    vdif_t *pVDIF
)
{
#ifdef DDKDEBUG

#ifndef ENABLE_DEBUG_PRINT_VDIF
    DDKDEBUGENABLE(0);
#endif
    
    DDKDEBUGPRINT((DISPLAY_LEVEL, "pixelClock = %d\n", pVDIF->pixelClock));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "characterWidth = %d\n", pVDIF->characterWidth));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "scanType = %s\n", (pVDIF->scanType == VDIF_INTERLACED) ? "Interlaced" : "Progressive"));
    
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalFrequency = %d\n", pVDIF->horizontalFrequency));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalTotal = %d\n", pVDIF->horizontalTotal));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalActive = %d\n", pVDIF->horizontalActive));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalBlankStart = %d\n", pVDIF->horizontalBlankStart));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalBlankTime = %d\n", pVDIF->horizontalBlankTime));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalSyncStart = %d\n", pVDIF->horizontalSyncStart));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalRightBorder = %d\n", pVDIF->horizontalRightBorder));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalFrontPorch = %d\n", pVDIF->horizontalFrontPorch));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalSyncWidth = %d\n", pVDIF->horizontalSyncWidth));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalBackPorch = %d\n", pVDIF->horizontalBackPorch));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalLeftBorder = %d\n", pVDIF->horizontalLeftBorder));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "horizontalSyncPolarity = %s\n", 
        (pVDIF->horizontalSyncPolarity == VDIF_SYNC_NEGATIVE) ? "Negative" : "Positive"));
    
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalFrequency = %d\n", pVDIF->verticalFrequency));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalTotal = %d\n", pVDIF->verticalTotal));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalActive = %d\n", pVDIF->verticalActive));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalBlankStart = %d\n", pVDIF->verticalBlankStart));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalBlankTime = %d\n", pVDIF->verticalBlankTime));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalSyncStart = %d\n", pVDIF->verticalSyncStart));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalBottomBorder = %d\n", pVDIF->verticalBottomBorder));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalFrontPorch = %d\n", pVDIF->verticalFrontPorch));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalSyncHeight = %d\n", pVDIF->verticalSyncHeight));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalBackPorch = %d\n", pVDIF->verticalBackPorch));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalTopBorder = %d\n", pVDIF->verticalTopBorder));
    DDKDEBUGPRINT((DISPLAY_LEVEL, "verticalSyncPolarity = %s\n", 
        (pVDIF->verticalSyncPolarity == VDIF_SYNC_NEGATIVE) ? "Negative" : "Positive"));

#ifndef ENABLE_DEBUG_PRINT_VDIF
    DDKDEBUGENABLE(1);
#endif
#endif
}

/*
 *  edidGetVersion
 *      This function gets the EDID version
 *
 *  Input:
 *      pEDIDBuffer - Buffer that contains the EDID structure of the monitor
 *      pRevision   - Revision of the EDIE (if exist)
 *
 *  Output:
 *      Revision number of the given EDID buffer.
 */
unsigned char edidGetVersion(
    unsigned char *pEDIDBuffer,
    unsigned char *pRevision
)
{
    unsigned char version;
    
    if (pEDIDBuffer != (unsigned char *)0)
    {
        /* Check the header */
        if ((pEDIDBuffer[0] == 0x00) && (pEDIDBuffer[1] == 0xFF) && (pEDIDBuffer[2] == 0xFF) &&
            (pEDIDBuffer[3] == 0xFF) && (pEDIDBuffer[4] == 0xFF) && (pEDIDBuffer[5] == 0xFF) &&
            (pEDIDBuffer[6] == 0xFF) && (pEDIDBuffer[7] == 0x00))
        {
            /* 
             * EDID Structure Version 1.
             */
        
            /* Read the version field from the buffer. It should be 1 */
            version  = pEDIDBuffer[18];
        
            if (version == 1)
            {
                /* Copy the revision first */
                if (pRevision != (unsigned char *)0)
                    *pRevision = pEDIDBuffer[19];
                    
                return version;
            }
        }
        else
        {
            /* 
             * EDID Structure Version 2 
             */
             
            /* Read the version and revision field from the buffer. */
            version = pEDIDBuffer[0];
        
            if ((version >> 4) == 2)
            {
                /* Copy the revision */
                if (pRevision != (unsigned char *)0)
                    *pRevision = version & 0x0F;
                
                return (version >> 4);
            }
        }
    }
    
    return 0;    
}

/*
 *  edidGetProductInfo
 *      This function gets the vendor and product information.
 *
 *  Input:
 *      pEDIDBuffer         - Buffer that contains the EDID structure of the monitor [in]
 *      pManufacturerName   - Pointer to a 3 byte length variable to store the manufacturer name [out]
 *      pProductCode        - Pointer to a variable to store the product code [out]
 *      pSerialNumber       - Pointer to a variable to store the serial number [out]
 *      pWeekOfManufacture  - Pointer to a variable to store the week of manufacture [out]
 *      pYearOfManufacture  - Pointer to a variable to store the year of manufacture 
 *                            or model year (if WeekOfManufacture is 0xff) [out]
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetProductInfo(
    unsigned char *pEDIDBuffer,
    char *pManufacturerName,
    unsigned short *pProductCode,
    uint32_t *pSerialNumber,
    unsigned char *pWeekOfManufacture,
    unsigned short *pYearOfManufacture
)
{
    unsigned char version, revision;
    unsigned short manufactureID;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        if (pManufacturerName != (char *)0)
        {
            /* Swap the byte */
            manufactureID = (pEDIDStructure->manufacturerID >> 8) + (pEDIDStructure->manufacturerID << 8);
            pManufacturerName[0] = ((manufactureID >> 10) & 0x001F) + 'A' - 1;
            pManufacturerName[1] = ((manufactureID >> 5) & 0x001F) + 'A' - 1;
            pManufacturerName[2] = (manufactureID & 0x001F) + 'A' - 1;
            pManufacturerName[3] = '\0';
        }
        
        if (pProductCode != (unsigned short *)0)
            *pProductCode = pEDIDStructure->productCode;
            
        /* Only EDID structure version 1.1 and 1.2 supports this. EDID 1.3 uses
           detail timing descriptor to store the serial number in ASCII. */
        if (pSerialNumber != (uint32_t *)0)
            *pSerialNumber = pEDIDStructure->serialNumber;
        
        /*
         * Rev 1.3: - A value of 0 means that week of manufacture is not specified
         *          - A value in the range of 1 to 54 (0x01 - 0x36) means the week of manufacture
         *          - Any values greater than 54 is invalid.
         *
         * Rev 1.4: - A value of 0 means that week of manufacture is not specified
         *          - A value in the range of 1 to 54 (0x01 - 0x36) means the week of manufacture
         *          - A value of 0xFF means that Year of Manufacture contains the model year
         *            instead of year of Manufacture.
         *          - Other values means invalid
         */
        if (pWeekOfManufacture != (unsigned char *)0)
            *pWeekOfManufacture = pEDIDStructure->weekOfManufacture;
            
        /* The value must be greater than 3 and less than or equal to the current
           year minus 1990.
           A value of 3 or less would indicated that the display was manufactured 
           before the EDID standard was defined.
           A value greater than (current year - 1990) would indicate that the display
           has not yet been manufactured.
         */
        if (pYearOfManufacture != (unsigned short *)0)
            *pYearOfManufacture = (unsigned short) pEDIDStructure->yearOfManufacture + 1990;
        
        return 0;
    }

    return (-1);
}

/*
 *  edidCheckMonitorInputSignal
 *      This function checks whether the monitor is expected analog/digital 
 *      input signal.
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      0   - Analog
 *      1   - Digital
 */
unsigned char edidCheckMonitorInputSignal(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version;
    unsigned short index;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
        return (unsigned char)((edid_version_1_t *)pEDIDBuffer)->videoInputDefinition.analogSignal.inputSignal;
    
    return 0;
}

/*
 *  edidGetAnalogSignalInfo
 *      This function gets the analog video input signal information
 *
 *  Input:
 *      pEDIDBuffer             - Buffer that contains the EDID structure of the monitor
 *      pRefWhiteAboveBlank     - Pointer to a variable to store the reference white above blank
 *                                value. The value is in milliVolt.
 *      pSyncLevelBelowBlank    - Pointer to a variable to store the Sync tip level below blank
 *                                The value is also in milliVolt
 *      pBlank2BlackSetup       - Pointer to a variable to store the Blank to black setup or
 *                                pedestal per appropriate Signal Level Standard flag. 
 *                                1 means that the display expect the setup.
 *      pSeparateSyncSupport    - Pointer to a variable to store the flag to indicate that the
 *                                monitor supports separate sync.
 *      pCompositeSyncSupport   - Pointer to a variable to store a flag to indicate that the
 *                                monitor supports composite sync.
 *      pSyncOnGreenSupport     - Pointer to a variable to store a flag to indicate that
 *                                the monitor supports sync on green video.
 *      pVSyncSerrationRequired - Pointer to a variable to store a flag to indicate that serration
 *                                of the VSync pulse is required when composite sync or
 *                                sync-on-green video is used.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetAnalogSignalInfo(
    unsigned char *pEDIDBuffer,
    unsigned short *pRefWhiteAboveBlank,
    unsigned short *pSyncLevelBelowBlank,
    unsigned char *pBlank2BlackSetup,
    unsigned char *pSeparateSyncSupport,
    unsigned char *pCompositeSyncSupport,
    unsigned char *pSyncOnGreenSupport,
    unsigned char *pVSyncSerrationRequired
)
{
    unsigned char version, revision;
    unsigned short whiteReference, syncLevel;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        /* Check if the input signal is analog */
        if (pEDIDStructure->videoInputDefinition.analogSignal.inputSignal != 0)
            return (-1);
        
        switch (pEDIDStructure->videoInputDefinition.analogSignal.signalLevelStd)
        {
            case 0:
                whiteReference = 700;
                syncLevel = 300;
                break;
            case 1:
                whiteReference = 714;
                syncLevel = 286;
                break;
            case 2:
                whiteReference = 1000;
                syncLevel = 400;
                break;
            case 3:
                whiteReference = 700;
                syncLevel = 0;
                break;
        }
        
        if (pRefWhiteAboveBlank != (unsigned short *)0)
            *pRefWhiteAboveBlank = whiteReference; 
        
        if (pSyncLevelBelowBlank != (unsigned short *)0)
            *pSyncLevelBelowBlank = syncLevel;
        
        if (pBlank2BlackSetup != (unsigned char *)0)
            *pBlank2BlackSetup = (unsigned char)
                                  pEDIDStructure->videoInputDefinition.analogSignal.blank2Black;
        
        if (pSeparateSyncSupport != (unsigned char *)0)
            *pSeparateSyncSupport = (unsigned char)
                                     pEDIDStructure->videoInputDefinition.analogSignal.separateSyncSupport;
        
        if (pCompositeSyncSupport != (unsigned char *)0)
            *pCompositeSyncSupport = (unsigned char)
                                      pEDIDStructure->videoInputDefinition.analogSignal.compositeSyncSupport;
        
        if (pSyncOnGreenSupport != (unsigned char *)0)
            *pSyncOnGreenSupport = (unsigned char)
                                    pEDIDStructure->videoInputDefinition.analogSignal.syncOnGreenSupport;
        
        if (pVSyncSerrationRequired != (unsigned char *)0)
            *pVSyncSerrationRequired = (unsigned char)
                                        pEDIDStructure->videoInputDefinition.analogSignal.vsyncSerration;
                                        
        return 0;
    }
    else
    {
        /* EDID Structure 2 */
    }
    
    return (-1);
}

/*
 *  edidGetDigitalSignalInfo
 *      This function gets the digital video input signal information.
 *      Only applies to EDID 1.3 and above.
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *      pDFP1xSupport   - Pointer to a variable to store the flag to indicate that
 *                        the mointor interface is signal compatible with VESA
 *                        DFP 1.x TMDS CRGB, 1 pixel/clock, up to 8 bits / color
 *                        MSB aligned, DE active high
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetDigitalSignalInfo(
    unsigned char *pEDIDBuffer,
    unsigned char *pDFP1xSupport
)
{
    unsigned char version, revision;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, &revision);
    
    if ((version == 1) && (revision == 3))
    {
        edid_version_1_t *pEDIDStructure;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        /* Check if the input signal is digital */
        if (pEDIDStructure->videoInputDefinition.digitalSignal.inputSignal != 1)
            return (-1);
            
        if (pDFP1xSupport != (unsigned char *)0)
            *pDFP1xSupport = pEDIDStructure->videoInputDefinition.digitalSignal.dfp1Support;
        
        return 0;
    }
    
    return (-1);
}

/*
 *  edidGetDisplaySize
 *      This function gets the display sizes in cm.
 *
 *  Input:
 *      pEDIDBuffer         - Buffer that contains the EDID structure of the monitor
 *      pMaxHorzImageSize   - Pointer to a variable to store the maximum horizontal 
 *                            image size to the nearest centimeter. A value of 0
 *                            indicates that the size is indeterminate size.
 *      pMaxVertImageSize   - Pointer to a variable to store the maximum vertical
 *                            image size to the nearest centimeter. A value of 0
 *                            indicates that the size is indeterminate size.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetDisplaySize(
    unsigned char *pEDIDBuffer,
    unsigned char *pMaxHorzImageSize,
    unsigned char *pMaxVertImageSize
)
{
    unsigned char version, revision;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        if (pMaxHorzImageSize != (unsigned char *)0)
            *pMaxHorzImageSize = pEDIDStructure->maxHorzImageSize;
        
        if (pMaxVertImageSize != (unsigned char *)0)
            *pMaxVertImageSize = pEDIDStructure->maxVertImageSize;
        
        return 0;
    }
    
    return (-1);
}

#if 0   /* Use the edidGetWhitePoint to get the Gamma */
/*
 *  edidGetGamma
 *      This function gets the Display Transfer Characteristic (Gamma).
 *
 *  Input:
 *      pEDIDBuffer         - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      Gamma value multiplied by 100. A value of 0xFFFF (-1) indicates that
 *      the gamma value is not defined.
 */
unsigned short edidGetGamma(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
        return (unsigned short)(((edid_version_1_t *)pEDIDBuffer)->displayTransferChar + 100);
    
    return (-1);
}
#endif

/*
 *  edidGetPowerManagementSupport
 *      This function gets the monitor's power management support.
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *      pStandBy        - Pointer to a variable to store the flag to indicate that
 *                        standby power mode is supported.
 *      pSuspend        - Pointer to a variable to store the flag to indicate that
 *                        suspend power mode is supported.
 *      pLowPower       - Pointer to a variable to store the flag to indicate that
 *                        the display consumes low power when it receives a timing
 *                        signal that is outside its declared active operating range.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetPowerManagementSupport(
    unsigned char *pEDIDBuffer,
    unsigned char *pStandBy,
    unsigned char *pSuspend,
    unsigned char *pLowPower
)
{
    unsigned char version;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        if (pStandBy != (unsigned char *)0)
            *pStandBy = (unsigned char) pEDIDStructure->featureSupport.standbySupport;
            
        if (pSuspend != (unsigned char *)0)
            *pSuspend = (unsigned char) pEDIDStructure->featureSupport.suspendSupport;
            
        if (pLowPower != (unsigned char *)0)
            *pLowPower = (unsigned char) pEDIDStructure->featureSupport.lowPowerSupport;
        
        return 0;
    }
    
    return (-1);
}

/*
 *  edidGetDisplayType
 *      This function gets the display type.
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      0   - Monochrome / grayscale display
 *      1   - RGB Color Display
 *      2   - Non-RGB multicolor display, e.g. R/G/Y
 *      3   - Undefined
 */
unsigned char edidGetDisplayType(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
        return (unsigned char)((edid_version_1_t *)pEDIDBuffer)->featureSupport.displayType;
    
    return (3);
}

/*
 *  edidChecksRGBUsage
 *      This function checks if the display is using the sRGB standard default
 *      color space as its primary color space. 
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      0   - Does not use sRGB as its primary color space
 *      1   - Use sRGB as its primary color space
 */
unsigned char edidChecksRGBUsage(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
        return (unsigned char)((edid_version_1_t *)pEDIDBuffer)->featureSupport.sRGBSupport;
    
    return (0);
}

/*
 *  edidIsPreferredTimingAvailable
 *      This function checks whether the preffered timing mode is available.
 *      Use of preferred timing mode is required by EDID structure version 1
 *      Revision 3 and higher. 
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      0   - Preferred Timing is not available
 *      1   - Preferred Timing is available
 */
unsigned char edidIsPreferredTimingAvailable(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version;
    
    if (pEDIDBuffer != (unsigned char *)0)
    {
        /* Get EDID Version and revision */
        version = edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
        if (version == 1)
            return (unsigned char)((edid_version_1_t *)pEDIDBuffer)->featureSupport.preferredTiming;
    }
        
    return (0);
}

/*
 *  edidIsDefaultGTFSupported
 *      This function checks whether the display supports timings based on the
 *      GTF standard using default GTF parameter values. 
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      0   - Default GTF is not supported
 *      1   - Default GTF is supported
 */
unsigned char edidIsDefaultGTFSupported(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version, revision;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
        return (unsigned char)((edid_version_1_t *)pEDIDBuffer)->featureSupport.defaultGTFSupport;
    
    return (0);
}

/*
 *  edidCalculateChromaticValue
 *      This function calculates the chromatic value. 
 *
 *  Input:
 *      colorBinaryValue    - Color Characteristic Binary Representation Value 
 *                            to be computed
 *
 *  Output:
 *      The chromatic value times a 1000.
 */
static unsigned short edidCalculateChromaticValue(
    unsigned short colorBinaryValue
)
{
    uint32_t index;
    uint32_t result;
    
    result = 0;
    for (index = 10; index > 0; index--)
    {
        /* Times 1000000 to make it accurate to the micro value. */
        result += roundedDiv((colorBinaryValue & 0x0001) * 1000000, twoToPowerOfx(index));
        colorBinaryValue >>= 1;
    }
    
    /* Make it accurate to 1000 place */
    return ((unsigned short)roundedDiv(result, 1000));
}

/*
 *  edidGetColorCharacteristic
 *      This function gets the chromaticity and white point values expressed as
 *      an integer value which represents the actual value times 1000.
 *
 *  Input:
 *      pEDIDBuffer - Buffer that contains the EDID structure of the monitor
 *      pRedX   - Pointer to a variable to store the Red X values
 *      pRedY   - Pointer to a variable to store the Red Y values
 *      pGreenX - Pointer to a variable to store the Green X values
 *      pGreenY - Pointer to a variable to store the Green Y values
 *      pBlueX  - Pointer to a variable to store the Blue X values
 *      pBlueY  - Pointer to a variable to store the Blue Y values
 *
 *  Note:
 *      To get the White color characteristic, use the edidGetWhitePoint
 */
void edidGetColorCharacteristic(
    unsigned char *pEDIDBuffer,
    unsigned short *pRedX,
    unsigned short *pRedY,
    unsigned short *pGreenX,
    unsigned short *pGreenY,
    unsigned short *pBlueX,
    unsigned short *pBlueY
)
{
    unsigned char version, revision;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        if (pRedX != (unsigned short *)0)
        {
            *pRedX = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->redX << 2) + 
                                                  (unsigned short)pEDIDStructure->redGreenLowBits.redXLowBits);
        }
            
        if (pRedY != (unsigned short *)0)
        {
            *pRedY = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->redY << 2) + 
                                                  (unsigned short)pEDIDStructure->redGreenLowBits.redYLowBits);
        }
            
        if (pGreenX != (unsigned short *)0)
        {
            *pGreenX = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->greenX << 2) + 
                                                    (unsigned short)pEDIDStructure->redGreenLowBits.greenXLowBits);
        }
            
        if (pGreenY != (unsigned short *)0)
        {
            *pGreenY = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->greenY << 2) + 
                                                    (unsigned short)pEDIDStructure->redGreenLowBits.greenYLowBits);
        }
            
        if (pBlueX != (unsigned short *)0)
        {
            *pBlueX = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->blueX << 2) +
                                                   (unsigned short)pEDIDStructure->blueWhiteLowBits.blueXLowBits);
        }
            
        if (pBlueY != (unsigned short *)0)
        {
            *pBlueY = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->blueY << 2) +
                                                   (unsigned short)pEDIDStructure->blueWhiteLowBits.blueYLowBits);
        }
    }
}

/*
 *  edidGetWhitePoint
 *      This function gets the white point.
 *      To get the default white point, set the index to 0. For multiple white point,
 *      call this function multiple times to check if more than 1 white point is supported.
 *
 *  Input:
 *      pEDIDBuffer - Buffer that contains the EDID structure of the monitor
 *      pWhitePointIndex    - Pointer to a variable that contains the white point index 
 *                            to be retrieved.
 *      pWhiteX             - Pointer to a variable to store the White X value
 *      pWhiteY             - Pointer to a variable to store the White Y value
 *      pWhiteGamma         - Pointer to a variable to store the White Gamma value
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetWhitePoint(
    unsigned char *pEDIDBuffer,
    unsigned char *pWhitePointIndex,
    unsigned short *pWhiteX,
    unsigned short *pWhiteY,
    unsigned short *pWhiteGamma
)
{
    unsigned char version, revision, index, tableIndex;
    
    if (pWhitePointIndex == (unsigned char *)0)
        return (-1);
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, (unsigned char *)0);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        monitor_desc_t *pMonitorDescriptor;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        /* Get the index to a temporary variable and increment the index for the
           next loop. */
        index = *pWhitePointIndex;
        (*pWhitePointIndex)++;
        
        if (index == 0)
        {
            if (pWhiteX != (unsigned short *)0)
            {
                *pWhiteX = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->whiteX << 2) +
                                                        (unsigned short)pEDIDStructure->blueWhiteLowBits.whiteXLowBits);
            }
            
            if (pWhiteY != (unsigned short *)0)
            {
                *pWhiteY = edidCalculateChromaticValue(((unsigned short)pEDIDStructure->whiteY << 2) +
                                                        (unsigned short)pEDIDStructure->blueWhiteLowBits.whiteYLowBits);
            }
            
            if (pWhiteGamma != (unsigned short *)0)
                *pWhiteGamma = pEDIDStructure->displayTransferChar + 100;
                
            return 0;
        }
        else
        {
            for (tableIndex = 0; tableIndex < 4; tableIndex++)
            {
                pMonitorDescriptor = &pEDIDStructure->miscInformation.monitorDesc[tableIndex];
                if ((pMonitorDescriptor->flag1 == 0) && (pMonitorDescriptor->flag2 == 0) &&
                    (pMonitorDescriptor->dataTypeTag == 0xFB) && 
                    (pMonitorDescriptor->descriptor.colorPoint.white[index-1].whitePointIndex != 0))
                {
                    if (pWhiteX != (unsigned short *)0)
                    {
                        *pWhiteX = edidCalculateChromaticValue(((unsigned short)pMonitorDescriptor->descriptor.colorPoint.white[index-1].whiteX << 2) +
                                                                (unsigned short)pMonitorDescriptor->descriptor.colorPoint.white[index-1].whiteLowBits.whiteXLowBits);
                    }
                    
                    if (pWhiteY != (unsigned short *)0)
                    {
                        *pWhiteY = edidCalculateChromaticValue(((unsigned short)pMonitorDescriptor->descriptor.colorPoint.white[index-1].whiteY << 2) +
                                                                (unsigned short)pMonitorDescriptor->descriptor.colorPoint.white[index-1].whiteLowBits.whiteYLowBits);
                    }
                    
                    if (pWhiteGamma != (unsigned short *)0)
                        *pWhiteGamma = pMonitorDescriptor->descriptor.colorPoint.white[index-1].gamma + 100;
                    
                    return 0;
                }
            }
        }
    }
    
    return (-1);
}

/*
 *  edidCalculateChecksum
 *      This function adds all one-byte value of the EDID buffer. 
 *      The total should be equal to 0x00
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      Total of one-byte values. It should equal to 0x00. A value other than
 *      0x00 indicates the EDID buffer is not valid.
 */
static unsigned char edidCalculateChecksum(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version, revision, checksum;
    unsigned short index;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, &revision);
    
    checksum = 0;
    if (version == 1)
    {
        for (index = 0; index < 128; index++)
            checksum += pEDIDBuffer[index];
    }
    
    return checksum;
}

/*
 *  edidGetExtension
 *      This function gets the number of (optional) EDID extension blocks to follow
 *      the given EDID buffer.
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *
 *  Output:
 *      Total number of EDID Extension to follow the given EDID buffer.
 */
unsigned char edidGetExtension(
    unsigned char *pEDIDBuffer
)
{
    unsigned char version, revision;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
        return ((edid_version_1_t *)pEDIDBuffer)->extFlag;
    
    return 0;
}

#define EDID_TOTAL_RETRY_COUNTER            4
/*
 *  edidReadMonitor
 *      This function reads the EDID structure from the attached monitor
 *
 *  Input:
 *      displayPath - Display device which EDID to be read from.
 *      pEDIDBuffer - Buffer that contains the EDID structure of the monitor
 *      bufferSize  - The EDID Buffer size index (usually 128-bytes)
 *      edidExtNo   - Extension Index of the EDID Structure to be read
 *      sclGpio     - GPIO pin used as the I2C Clock (SCL)
 *      sdaGpio     - GPIO pin used as the I2C Data (SDA)
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
_X_EXPORT int32_t ddk750_edidReadMonitorEx(
    disp_path_t displayPath,
    unsigned char *pEDIDBuffer,
    uint32_t bufferSize,
    unsigned char edidExtNo,
    unsigned char sclGpio,
    unsigned char sdaGpio
)
{
    unsigned char value, retry, edidVersion, edidRevision;
    unsigned char edidBuffer[256];
    uint32_t offset;
    
    /* Initialize the i2c bus */
#if USE_HW_I2C == 1
	hwI2CInit(1);//use fast mode
#else
    swI2CInit(sclGpio, sdaGpio);
#endif
    for (retry = 0; retry < EDID_TOTAL_RETRY_COUNTER; retry++)
    {
            
        /* Read the EDID from the monitor. */
        for (offset = 0; offset < TOTAL_EDID_REGISTERS; offset++)
#if USE_HW_I2C == 1
			edidBuffer[offset] = hwI2CReadReg(EDID_DEVICE_I2C_ADDRESS, (unsigned char)offset);
#else
            edidBuffer[offset] = swI2CReadReg(EDID_DEVICE_I2C_ADDRESS, (unsigned char)offset);
#endif        
        /* Check if the EDID is valid. */
        edidVersion = edidGetVersion((unsigned char *)&edidBuffer, (unsigned char *)&edidRevision);
        if (edidVersion != 0)
            break;
    }
    
    /* 
     *  The monitor might not be DDC2B compliance. Therefore, need to use DDC1 protocol,
     *  which uses the Vertical Sync to clock in the EDID data.
     *  Currently this function return error. DDC1 protocol can be added later.  
     */
    if (retry == EDID_TOTAL_RETRY_COUNTER)
    {
        /* DDC1 uses the SDA line to transmit 9 bit data per byte. The last bit is
         * only an acknowledge flag, which could be high or low. However, SCL line
         * is not used. Instead the data is clock-in using vertical sync.
         */
        return (-1);
    }
    
    /* Copy the data to the given buffer */
    if (pEDIDBuffer != (unsigned char *)0)
    {
        for (offset = 0; offset < bufferSize; offset++)
            pEDIDBuffer[offset] = edidBuffer[offset];
    }
    
#if 0 /*def DDKDEBUG*/
    for (offset = 0; offset < TOTAL_EDID_REGISTERS; offset++)
    {
        if ((offset % 16) == 0)
        {
            if (offset != 0)
                DDKDEBUGPRINT((0/*DISPLAY_LEVEL*/, "\n"));
            DDKDEBUGPRINT((0/*DISPLAY_LEVEL*/, "%02x:\t", offset));
        }
        DDKDEBUGPRINT((0/*DISPLAY_LEVEL*/, "%02x  ", pEDIDBuffer[offset]));
    }
    DDKDEBUGPRINT((0/*DISPLAY_LEVEL*/, "\n"));
#endif

    return 0;
}

/*
 *  edidReadMonitor
 *      This function reads the EDID structure from the attached monitor
 *
 *  Input:
 *      displayPath - Display device which EDID to be read from.
 *      pEDIDBuffer - Buffer that contains the EDID structure of the monitor
 *      bufferSize  - The EDID Buffer size index (usually 128-bytes)
 *      edidExtNo   - Extension Index of the EDID Structure to be read
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
_X_EXPORT int32_t edidReadMonitor(
    disp_path_t displayPath,
    unsigned char *pEDIDBuffer,
    uint32_t bufferSize,
    unsigned char edidExtNo
)
{
    return ddk750_edidReadMonitorEx(displayPath, pEDIDBuffer, bufferSize, edidExtNo, DEFAULT_I2C_SCL, DEFAULT_I2C_SDA);
}


_X_EXPORT int32_t edidReadMonitorEx_software(
    disp_path_t displayPath,
    unsigned char *pEDIDBuffer,
    uint32_t bufferSize,
    unsigned char edidExtNo,
    unsigned char sclGpio,
    unsigned char sdaGpio
)
{
    unsigned char value, retry, edidVersion, edidRevision;
    unsigned char edidBuffer[256];
    uint32_t offset;

    /* Initialize the i2c bus */
    swI2CInit(sclGpio, sdaGpio);
    for (retry = 0; retry < EDID_TOTAL_RETRY_COUNTER; retry++)
    {
		
        /* Read the EDID from the monitor. */
        for (offset = 0; offset < TOTAL_EDID_REGISTERS; offset++){
            edidBuffer[offset] = swI2CReadReg(EDID_DEVICE_I2C_ADDRESS, (unsigned char)offset);
		}
            
        /* Check if the EDID is valid. */
        edidVersion = edidGetVersion((unsigned char *)&edidBuffer, (unsigned char *)&edidRevision);
		if (edidVersion != 0)
	    	break;
    }
    
    /* 
     *  The monitor might not be DDC2B compliance. Therefore, need to use DDC1 protocol,
     *  which uses the Vertical Sync to clock in the EDID data.
     *  Currently this function return error. DDC1 protocol can be added later.  
     */
    if (retry == EDID_TOTAL_RETRY_COUNTER)
    {
        /* DDC1 uses the SDA line to transmit 9 bit data per byte. The last bit is
         * only an acknowledge flag, which could be high or low. However, SCL line
         * is not used. Instead the data is clock-in using vertical sync.
         */
        return (-1);
    }
    
    /* Copy the data to the given buffer */
    if (pEDIDBuffer != (unsigned char *)0)
    {
        for (offset = 0; offset < bufferSize; offset++)
            pEDIDBuffer[offset] = edidBuffer[offset];
    }
    return 0;
}


/*
 *  edidGetEstablishedTiming
 *      This function gets the established timing list from the given EDID buffer,
 *      table, and timing index.
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor (In)
 *      pEstTableIndex  - Pointer to the Established Timing Table index  (In/Out)
 *      pIndex          - Pointer to the Establihsed Timing Index (In/Out)
 *      pWidth          - Pointer to a variable that to store the horizontal active / width
 *                        value of the retrieved timing (Out)
 *      pHeight         - Pointer to a variable to store the vertical active / height
 *                        value of the retrieved timing (Out)
 *      pRefreshRate    - Pointer to a variable to store the vertical frequency value
 *                        of the retrieved timing (out)
 *      pSource         - Pointer to a variable to store the standard timing source:
 *                          0 - VESA
 *                          1 - IBM
 *                          2 - Apple
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetEstablishedTiming(
    unsigned char *pEDIDBuffer,
    /*unsigned char *pEstTableIndex,*/
    unsigned char *pIndex,
    uint32_t *pWidth,
    uint32_t *pHeight,
    uint32_t *pRefreshRate,
    unsigned char *pSource
)
{
    unsigned char version, revision;
    unsigned char tableIndex, index;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        while (1)
        {
            /* Get index */
            index = *pIndex;
            
            if (index > 16)
                break;
            
            /* Search Established Table index 0 when the index is less than 8 */
            tableIndex = index / 8;
            
            /* Exit the function when it has reached the last table. */
            if (tableIndex > 2)
                break;
            
            /* Increment the index value and update the index accordingly */
            (*pIndex)++;
            index %= 8;
            
            /* Check */
            if ((pEDIDStructure->estTiming[tableIndex] & (1 << index)) != 0)
            {
                if (pWidth != (uint32_t *)0)
                    *pWidth = establishTiming[tableIndex][index].x;
        
                if (pHeight != (uint32_t *)0)
                    *pHeight = establishTiming[tableIndex][index].y;
    
                if (pRefreshRate != (uint32_t *)0)
                    *pRefreshRate = establishTiming[tableIndex][index].hz;
    
                if (pSource != (unsigned char *)0)
                    *pSource = establishTiming[tableIndex][index].source;
                    
                /* Return success */
                return 0;
            }
        }
    }
    else
    {
        /* EDID Structure Version 2.0. */
    }
    
    return (-1);
}

/*
 *  edidCalculateStdTiming
 *      This function calculates the width, height, and vertical frequency values
 *      from the given Standard Timing structure. This function only applies to
 *      EDID structure version 1. It will give the wrong result when used with
 *      EDID version 2.
 *
 *  Input:
 *      pStdTiming      - Pointer to a standard timing structure that contains the
 *                        standard timing value to be calculated (In)
 *      edid1Revision   - Revision of the EDID 1 (In)
 *      pWidth          - Pointer to a variable that to store the horizontal active / width
 *                        value of the retrieved timing (Out)
 *      pHeight         - Pointer to a variable to store the vertical active / height
 *                        value of the retrieved timing (Out)
 *      pRefreshRate    - Pointer to a variable to store the vertical frequency value
 *                        of the retrieved timing (out)
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
static int32_t edidCalculateStdTiming(
    standard_timing_t *pStdTiming,
    unsigned char edid1Revision,
    uint32_t *pWidth,
    uint32_t *pHeight,
    uint32_t *pRefreshRate
)
{
    uint32_t x, y;
     
    /* Calculate the standard timing into x and y mode dimension */
    if (pStdTiming->horzActive != 0x01)
    {
        /* Calculate the X and Y */
        x = (pStdTiming->horzActive + 31) * 8;
        switch (pStdTiming->stdTimingInfo.aspectRatio)
        {
            case 0:
                if (edid1Revision != 3)
                    y = x;                  /* 1:1 aspect ratio (prior revision 1.3) */
                else
                    y = x * 10 / 16;        /* 16:10 aspect ratio (revision 1.3) */
                break;
            case 1:
                y = x * 3 / 4;              /* 4:3 aspect ratio */
                break;
            case 2:
                y = x * 4 / 5;              /* 5:4 aspect ratio */
                break;
            case 3:
                y = x * 9 / 16;             /* 16:9 aspect ratio */
                break;
        }

        if (pWidth != (uint32_t *)0)
            *pWidth = x;

        if (pHeight != (uint32_t *)0)
            *pHeight = y;

        if (pRefreshRate != (uint32_t *)0)
            *pRefreshRate = pStdTiming->stdTimingInfo.refreshRate + 60;
    
        return 0;
    }
    
    return (-1);
}

/*
 *  edidGetStandardTiming
 *      This function gets the standard timing from the given EDID buffer and
 *      calculates the width, height, and vertical frequency from that timing.
 *
 *  Input:
 *      pEDIDBuffer     - Buffer that contains the EDID structure of the monitor
 *      pStdTimingIndex - Pointer to a standard timing index to be retrieved
 *      pWidth          - Pointer to a variable that to store the horizontal active / width
 *                        value of the retrieved timing (Out)
 *      pHeight         - Pointer to a variable to store the vertical active / height
 *                        value of the retrieved timing (Out)
 *      pRefreshRate    - Pointer to a variable to store the vertical frequency value
 *                        of the retrieved timing (out)
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetStandardTiming(
    unsigned char *pEDIDBuffer,
    unsigned char *pStdTimingIndex,
    uint32_t *pWidth,
    uint32_t *pHeight,
    uint32_t *pRefreshRate
)
{
    unsigned char version, revision, timingIndex, tableIndex;
    uint32_t x, y, aspectRatio;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        monitor_desc_t *pMonitorDescriptor;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        while (1)
        {
            /* There are only 8 standard timing entries */
            if (*pStdTimingIndex > 7)
                break;
            
            /* Get the table index first before incrementing the index. */
            timingIndex = *pStdTimingIndex;
            
            /* Increment the standard timing index */
            (*pStdTimingIndex)++;
            
            if (timingIndex < 8)
            {
                /*
                 *  Search the first Standard Timing Identifier table
                 */
                 
                /* Calculate the standard timing into x and y mode dimension */
                if (edidCalculateStdTiming(&pEDIDStructure->stdTiming[timingIndex], 
                                       revision, pWidth, pHeight, pRefreshRate) == 0)
                {
                    return 0;
                }
            }
            else
            {
                /*
                 *  Search Standard Timing Identifier Table in the detailed Timing block. 
                 */
                
                /* 
                 * Each Detailed Timing Identifier can contains 6 entries of Standard Timing
                 * Identifier. Based on this value, we can get the Detailed Timing Table Index
                 * that contains the requested standard timing.
                 */
                timingIndex = timingIndex - 8;
                for (tableIndex = 0; tableIndex < 4; tableIndex++)
                {
                    /* Get detailed info */
                    pMonitorDescriptor = &pEDIDStructure->miscInformation.monitorDesc[tableIndex];
                    if ((pMonitorDescriptor->flag1 == 0) && (pMonitorDescriptor->flag2 == 0) &&
                        (pMonitorDescriptor->dataTypeTag == 0xFA))
                    {
                        if (timingIndex >= 6)
                        {
                            timingIndex-=6;
                            continue;
                        }
                        else
                        {
                            if (edidCalculateStdTiming(&pMonitorDescriptor->descriptor.stdTimingExt.stdTiming[timingIndex], 
                                                   revision, pWidth, pHeight, pRefreshRate) == 0)
                            {
                                return 0;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        /* EDID Structure version 2 */
    }
    
    return (-1);
}

/*
 *  edidGetDetailedTiming
 *      This function gets the detailed timing from the given EDID buffer.
 *
 *  Input:
 *      pEDIDBuffer             - Buffer that contains the EDID structure of the monitor
 *      pDetailedTimingIndex    - Pointer to a detailed timing index to be retrieved
 *      pModeParameter          - Pointer to a mode_parameter_t structure that will be
 *                                filled with the detailed timing.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetDetailedTiming(
    unsigned char *pEDIDBuffer,
    unsigned char *pDetailedTimingIndex,
    vdif_t *pVDIF
)
{
    unsigned char version, revision, tableIndex;
    uint32_t x, y, aspectRatio;
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        detailed_timing_t *pDetailedTiming;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        while (1)
        {
            if (*pDetailedTimingIndex > 3)
                break;
            
            /* Get the Detail Timing entry index */
            tableIndex = *pDetailedTimingIndex;
            
            /* Increment the index */
            (*pDetailedTimingIndex)++;
                
            /* Get detailed info */
            pDetailedTiming = &pEDIDStructure->miscInformation.detailTiming[tableIndex];
            if ((pDetailedTiming->pixelClock != 0) && (pVDIF != (vdif_t *)0))
            {
                /* Translate the Detail timing to VDIF format. */
                pVDIF->pixelClock = (uint32_t)pDetailedTiming->pixelClock * 10000;
                pVDIF->characterWidth = 8;
                pVDIF->scanType = (pDetailedTiming->flags.interlaced == 0) ? VDIF_NONINTERLACED : VDIF_INTERLACED;

                pVDIF->horizontalActive = 
                    ((uint32_t)pDetailedTiming->horzActiveBlanking.horzActiveMSB << 8) +
                     (uint32_t)pDetailedTiming->horzActive;
                pVDIF->horizontalBlankStart = pVDIF->horizontalActive;
                pVDIF->horizontalBlankTime = 
                    ((uint32_t)pDetailedTiming->horzActiveBlanking.horzBlankingMSB << 8) +
                     (uint32_t)pDetailedTiming->horzBlanking;
                pVDIF->horizontalTotal = pVDIF->horizontalActive + pVDIF->horizontalBlankTime;
                pVDIF->horizontalFrontPorch = 
                    ((uint32_t)pDetailedTiming->syncAuxInfo.horzSyncOffset << 8) + 
                     (uint32_t)pDetailedTiming->horzSyncOffset;
                pVDIF->horizontalSyncStart = pVDIF->horizontalBlankStart + pVDIF->horizontalFrontPorch;
                pVDIF->horizontalSyncWidth = 
                    ((uint32_t)pDetailedTiming->syncAuxInfo.horzSyncWidth << 8) +
                     (uint32_t)pDetailedTiming->horzSyncPulseWidth;                     
                pVDIF->horizontalBackPorch = 
                    pVDIF->horizontalBlankTime - (pVDIF->horizontalFrontPorch + pVDIF->horizontalSyncWidth);
                pVDIF->horizontalFrequency = roundedDiv(pVDIF->pixelClock, pVDIF->horizontalTotal);
                pVDIF->horizontalLeftBorder = 0;
                pVDIF->horizontalRightBorder = 0;
        
                pVDIF->verticalActive = 
                    ((uint32_t)pDetailedTiming->vertActiveBlanking.vertActiveMSB << 8) +
                     (uint32_t)pDetailedTiming->vertActive;
                pVDIF->verticalBlankStart = pVDIF->verticalActive;
                pVDIF->verticalBlankTime =
                    ((uint32_t)pDetailedTiming->vertActiveBlanking.vertBlankingMSB << 8) +
                     (uint32_t)pDetailedTiming->vertBlanking;
                pVDIF->verticalTotal = pVDIF->verticalActive + pVDIF->verticalBlankTime;
                pVDIF->verticalFrontPorch = 
                    ((uint32_t)pDetailedTiming->syncAuxInfo.vertSyncOffset << 8) +
                     (uint32_t)pDetailedTiming->verticalSyncInfo.syncOffset;
                pVDIF->verticalSyncStart = pVDIF->verticalBlankStart + pVDIF->verticalFrontPorch;
                pVDIF->verticalSyncHeight =
                    ((uint32_t)pDetailedTiming->syncAuxInfo.vertSyncWidth  << 8) +
                     (uint32_t)pDetailedTiming->verticalSyncInfo.syncWidth;
                pVDIF->verticalBackPorch =
                    pVDIF->verticalBlankTime - (pVDIF->verticalFrontPorch + pVDIF->verticalSyncHeight);
                pVDIF->verticalFrequency =
                    roundedDiv(pVDIF->pixelClock, (pVDIF->horizontalTotal * pVDIF->verticalTotal));
                pVDIF->verticalTopBorder = 0;
                pVDIF->verticalBottomBorder = 0;
                
                if (pDetailedTiming->flags.connectionType == 3)
                {
                    pVDIF->verticalSyncPolarity = 
                        (pDetailedTiming->flags.vertSyncFlag == 1) ? VDIF_SYNC_POSITIVE : VDIF_SYNC_NEGATIVE;
                    pVDIF->horizontalSyncPolarity = 
                        (pDetailedTiming->flags.horzSyncFlag == 1) ? VDIF_SYNC_POSITIVE : VDIF_SYNC_NEGATIVE;
                }
                else
                {
                    pVDIF->verticalSyncPolarity = VDIF_SYNC_NEGATIVE;
                    pVDIF->horizontalSyncPolarity = VDIF_SYNC_NEGATIVE;
                }
                
                /* For debugging purpose. */
                printVdif(pVDIF);
                
                return 0;
            }
        }
    }
    
    return (-1);
}

/*
 *  edidGetMonitorSerialNumber
 *      This function gets the monitor serial number from the EDID structure.
 *      Only EDID version 1 and revision 1 or above supports this feature.
 *
 *  Input:
 *      pEDIDBuffer             - Buffer that contains the EDID structure of the monitor
 *      pMonitorSerialNumber    - Pointer to a buffer to store the serial number 
 *                                retrieved from the EDID
 *      bufferSize              - The size of the buffer to store the serial number.
 *                                The maximum size required is 13 bytes.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetMonitorSerialNumber(
    unsigned char *pEDIDBuffer,
    char *pMonitorSerialNumber,
    unsigned char bufferSize
)
{
    unsigned char version, revision, tableIndex, charIndex;
    
    /* If no pointer is given or the buffer size is set to 0, then return fail. */
    if ((pMonitorSerialNumber == (char *)0) || (bufferSize == 0))
        return (-1);
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, &revision);
    
    if ((version == 1) && (revision > 0))
    {
        edid_version_1_t *pEDIDStructure;
        monitor_desc_t *pMonitorDescriptor;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        for (tableIndex = 0; tableIndex < 4; tableIndex++)
        {
            pMonitorDescriptor = &pEDIDStructure->miscInformation.monitorDesc[tableIndex];
            if ((pMonitorDescriptor->flag1 == 0) && (pMonitorDescriptor->flag2 == 0) &&
                (pMonitorDescriptor->dataTypeTag == 0xFF))
            {
                bufferSize = (bufferSize > 13) ? 13 : bufferSize;
                for (charIndex = 0; charIndex < 13; charIndex++)
                {
                    if (pMonitorDescriptor->descriptor.serialNo[charIndex] == 0x0A)
                    {
                        pMonitorSerialNumber[charIndex] = '\0';
                        break;
                    }
                        
                    pMonitorSerialNumber[charIndex] = pMonitorDescriptor->descriptor.serialNo[charIndex];
                }
                
                return 0;
            }
        }
    }
    
    /* Serial Number is not found. */
    return (-1);
}

/*
 *  edidGetDataString
 *      This function gets the data string from the EDID 
 *      Only EDID version 1 and revision 1 or above supports this feature.
 *
 *  Input:
 *      pEDIDBuffer             - Buffer that contains the EDID structure of the monitor
 *      pMonitorSerialNumber    - Pointer to a buffer to store the data string 
 *                                retrieved from the EDID
 *      bufferSize              - The size of the buffer to store the data string
 *                                The maximum size required is 13 bytes.
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetDataString(
    unsigned char *pEDIDBuffer,
    char *pDataString,
    unsigned char bufferSize
)
{
    unsigned char version, revision, tableIndex, charIndex;
    
    /* If no pointer is given or the buffer size is set to 0, then return fail. */
    if ((pDataString == (char *)0) || (bufferSize == 0))
        return (-1);
    
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, &revision);
    
    if ((version == 1) && (revision > 0))
    {
        edid_version_1_t *pEDIDStructure;
        monitor_desc_t *pMonitorDescriptor;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        for (tableIndex = 0; tableIndex < 4; tableIndex++)
        {
            pMonitorDescriptor = &pEDIDStructure->miscInformation.monitorDesc[tableIndex];
            if ((pMonitorDescriptor->flag1 == 0) && (pMonitorDescriptor->flag2 == 0) &&
                (pMonitorDescriptor->dataTypeTag == 0xFE))
            {
                bufferSize = (bufferSize > 13) ? 13 : bufferSize;
                for (charIndex = 0; charIndex < 13; charIndex++)
                {
                    if (pMonitorDescriptor->descriptor.dataString[charIndex] == 0x0A)
                    {
                        pDataString[charIndex] = '\0';
                        break;
                    }
                        
                    pDataString[charIndex] = pMonitorDescriptor->descriptor.dataString[charIndex];
                }
                
                return 0;
            }
        }
    }
    
    /* Data String is not found. */
    return (-1);
}

/*
 *  edidGetMonitorRangeLimit
 *      This function gets the monitor range limits from the EDID structure.
 *      Only EDID version 1 revision 1 or above supports this feature.
 *      This is a required field in EDID Version 1.3
 *
 *  Input:
 *      pEDIDBuffer         - Buffer that contains the EDID structure of the monitor
 *      pMinVerticalRate    - Pointer to a variable to store the Minimum Vertical Rate (Hz)
 *      pMaxVerticalRate    - Pointer to a variable to store the Maximum Vertical Rate (Hz)
 *      pMinHorzFreq        - Pointer to a variable to store the Minimum Horz. Freq (kHz)
 *      pMaxHorzFreq        - Pointer to a variable to store the Maximum Horz. Freq (kHz)
 *      pMaxPixelClock      - Pointer to a variable to store the Maximum Pixel Clock (Hz)
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetMonitorRangeLimit(
    unsigned char *pEDIDBuffer,
    unsigned char *pMinVerticalRate,
    unsigned char *pMaxVerticalRate,
    unsigned char *pMinHorzFreq,
    unsigned char *pMaxHorzFreq,
    uint32_t *pMaxPixelClock
)
{
    unsigned char version, revision, tableIndex, charIndex;
    
    if (pEDIDBuffer == (unsigned char *)0)
        return (-1);
            
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, &revision);
    
    if ((version == 1) && (revision > 0))
    {
        edid_version_1_t *pEDIDStructure;
        monitor_desc_t *pMonitorDescriptor;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        
        for (tableIndex = 0; tableIndex < 4; tableIndex++)
        {
            pMonitorDescriptor = &pEDIDStructure->miscInformation.monitorDesc[tableIndex];
            if ((pMonitorDescriptor->flag1 == 0) && (pMonitorDescriptor->flag2 == 0) &&
                (pMonitorDescriptor->dataTypeTag == 0xFD) && (pMonitorDescriptor->flag3 == 0))
            {
                if (pMinVerticalRate != (unsigned char *)0)
                    *pMinVerticalRate = pMonitorDescriptor->descriptor.monitorRange.minVertRate;

                if (pMaxVerticalRate != (unsigned char *)0)
                    *pMaxVerticalRate = pMonitorDescriptor->descriptor.monitorRange.maxVertRate;
                    
                if (pMinHorzFreq != (unsigned char *)0)
                    *pMinHorzFreq = pMonitorDescriptor->descriptor.monitorRange.minHorzFrequency;
                    
                if (pMaxHorzFreq != (unsigned char *)0)
                    *pMaxHorzFreq = pMonitorDescriptor->descriptor.monitorRange.maxHorzFrequency;
                    
                if (pMaxPixelClock != (uint32_t *)0)
                    *pMaxPixelClock = (uint32_t) pMonitorDescriptor->descriptor.monitorRange.maxPixelClock * 10 * 1000000;
                    
                return 0;
            }
        }
    }
    
    /* Data String is not found. */
    return (-1);
}

/*
 *  edidGetSecondaryTimingSupport
 *      This function gets the secondary GTF timing support.
 *      Only EDID version 1 and revision 1 or above supports this feature.
 *
 *  Input:
 *      pEDIDBuffer             - Buffer that contains the EDID structure of the monitor
 *      pStartFrequency         - Pointer to a variable to store the start frequency of 
 *                                the secondary GTF
 *      pOffset                 - Pointer to a variable to store the Offset (C) value of
 *                                the secondary GTF
 *      pGradient               - Pointer to a variable to store the Gradient (M) value of
 *                                the secondary GTF
 *      pScalingFactor          - Pointer to a variable to store the Scaling Factor (K)
 *                                value of the secondary GTF
 *      pScalingFactorWeight    - Pointer to a variable to store the Scaling Factore Weight (J)
 *                                value of the secondary GTF
 *
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetSecondaryTimingSupport(
    unsigned char *pEDIDBuffer,
    unsigned short *pStartFrequency,
    unsigned char *pOffset,
    unsigned short *pGradient,
    unsigned char *pScalingFactor,
    unsigned char *pScalingFactorWeight
)
{
    unsigned char version, revision, tableIndex, charIndex;
            
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, &revision);
    
    if ((version == 1) && (revision > 0) && (pEDIDBuffer != (unsigned char *)0))
    {
        edid_version_1_t *pEDIDStructure;
        monitor_desc_t *pMonitorDescriptor;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        for (tableIndex = 0; tableIndex < 4; tableIndex++)
        {
            pMonitorDescriptor = &pEDIDStructure->miscInformation.monitorDesc[tableIndex];
            if ((pMonitorDescriptor->flag1 == 0) && (pMonitorDescriptor->flag2 == 0) &&
                (pMonitorDescriptor->dataTypeTag == 0xFD) && 
                (pMonitorDescriptor->descriptor.monitorRange.secondaryTimingFlag == 0x02))
            {
                if (pStartFrequency != (unsigned short *)0)
                    *pStartFrequency = (unsigned short)
                        pMonitorDescriptor->descriptor.monitorRange.secondaryTimingInfo.cmkjParam.startFrequency * 2 * 1000;

                if (pOffset != (unsigned char *)0)
                    *pOffset = pMonitorDescriptor->descriptor.monitorRange.secondaryTimingInfo.cmkjParam.cParam/2;
                    
                if (pGradient != (unsigned short *)0)
                    *pGradient = pMonitorDescriptor->descriptor.monitorRange.secondaryTimingInfo.cmkjParam.mParam;
                    
                if (pScalingFactor != (unsigned char *)0)
                    *pScalingFactor = pMonitorDescriptor->descriptor.monitorRange.secondaryTimingInfo.cmkjParam.kParam;
                    
                if (pScalingFactorWeight != (unsigned char *)0)
                    *pScalingFactorWeight = pMonitorDescriptor->descriptor.monitorRange.secondaryTimingInfo.cmkjParam.jParam / 2;
                    
                return 0;
            }
        }
    }
    
    /* Data String is not found. */
    return (-1);
}

/*
 *  edidGetMonitorName
 *      This function gets the monitor name from the EDID structure.
 *      This is a required field in EDID Version 1.3
 *
 *  Input:
 *      pEDIDBuffer             - Buffer that contains the EDID structure of the monitor
 *      pMonitorName            - Pointer to a buffer to store the monitor name 
 *                                retrieved from the EDID
 *      bufferSize              - The size of the buffer to store the monitor name
 *                                The maximum size required is 13 bytes.
 * 
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetMonitorName(
    unsigned char *pEDIDBuffer,
    char *pMonitorName,
    unsigned char bufferSize
)
{
    unsigned char version, revision, tableIndex, charIndex;
    
    /* If no pointer is given or the buffer size is set to 0, then return fail. */
    if ((pMonitorName == (char *)0) || (bufferSize == 0))
        return (-1);
        
    /* Get EDID Version and revision */
    version = edidGetVersion(pEDIDBuffer, &revision);
    
    if (version == 1)
    {
        edid_version_1_t *pEDIDStructure;
        monitor_desc_t *pMonitorDescriptor;
        
        /* EDID Structure Version 1. */
        pEDIDStructure = (edid_version_1_t *)pEDIDBuffer;
        for (tableIndex = 0; tableIndex < 4; tableIndex++)
        {
            pMonitorDescriptor = &pEDIDStructure->miscInformation.monitorDesc[tableIndex];
            if ((pMonitorDescriptor->flag1 == 0) && (pMonitorDescriptor->flag2 == 0) &&
                (pMonitorDescriptor->dataTypeTag == 0xFC) && (pMonitorDescriptor->flag3 == 0))
            {
                bufferSize = (bufferSize > 13) ? 13 : bufferSize;
                for (charIndex = 0; charIndex < 13; charIndex++)
                {
                    if (pMonitorDescriptor->descriptor.monitorName[charIndex] == 0x0A)
                    {
                        pMonitorName[charIndex] = '\0';
                        break;
                    }
                        
                    pMonitorName[charIndex] = pMonitorDescriptor->descriptor.monitorName[charIndex];
                }
                
                return 0;
            }
        }
    }
    
    /* Data String is not found. */
    return (-1);
}

/*
 *  edidGetPreferredTiming
 *      This function gets the preferred/native timing of the monitor
 *
 *  Input:
 *      pEDIDBuffer         - Buffer that contains the EDID structure of the monitor
 *      pWidth              - Pointer to an uint32_t buffer to store the width 
 *                            of the preferred (native) timing.
 *      pHeight             - Pointer to an uint32_t buffer to store the height
 *                            of the preferred (native) timing.
 *      pVerticalFrequency  - Pointer to an uint32_t buffer to store the refresh
 *                            rate of the preferred (native) timing.
 * 
 *  Output:
 *      0   - Success
 *     -1   - Fail
 */
int32_t edidGetPreferredTiming(
    unsigned char *pEDIDBuffer,
    uint32_t *pWidth,
    uint32_t *pHeight,
    uint32_t *pVerticalFrequency
)
{
    unsigned char index = 0;
    vdif_t vdifBuffer;

/* Disable this checking since some old monitor does have the Detailed Timing although the preferred timing flag is 0. */
#if 0
    /* Check if preferred timing is available */
    if (edidIsPreferredTimingAvailable(pEDIDBuffer) == 1)
#endif
    {
        /* The preferred (native) timing is available, so get the timing. It is located
           at the first index of detailed timing.
         */
        if (edidGetDetailedTiming(pEDIDBuffer, &index, &vdifBuffer) == 0)
        {
            if (pWidth != (uint32_t *)0)
                *pWidth = vdifBuffer.horizontalActive;
                
            if (pHeight != (uint32_t *)0)
                *pHeight = vdifBuffer.verticalActive;
                
            if (pVerticalFrequency != (uint32_t *)0)
                *pVerticalFrequency = vdifBuffer.verticalFrequency;
                
            return 0;
        }
    }
    
    return (-1);
}
