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

#include "ddk502_hardware.h"

#include "ddk502_ddkdebug.h"
#include "ddk502_help.h"
#include "ddk502_os.h"

/* Table of all the device ID's supported by this library */
static unsigned short gwDeviceIdTable[] = {
0x0501,
0};

/* Use Voyager PCI ID as default, but it may be changed by
   detectDevice() if it is called */
static unsigned short gwDeviceId = 0x0501;

/* Total number of devices with the same ID in the system.
   Use 1 as default.
*/
static unsigned short gwNumOfDev = 1;

/* Current device in use.
   If there is only ONE device, it's always device 0.
   If gwNumDev > 1, user need to set device 0, device 1, device 2, etc..
   The purpose is using it control multiple devices of the same ID, if any.
*/
static unsigned short gwCurDev = 0;

static unsigned char *frameBufBase[MAX_SMI_DEVICE] = {0, 0, 0, 0};
static unsigned char *mmioBase[MAX_SMI_DEVICE] = {0, 0, 0, 0};
static unsigned char *frameBufPhysicalBase[MAX_SMI_DEVICE] = {0, 0, 0, 0};

/* Get revision ID from the given Vendor ID, device ID, and device Number */
unsigned char getRevisionId(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum /* If more than one device in system, device number are ordered as 0, 1, 2,... */
)
{
    return 0;//readPCIByte(vendorId, deviceId, deviceNum, 0x08);
}

/*
 * Detect if a specific device exist in the system.
 * If exist, enable it on the PCI bus.
 * Return: 0 if device found
 *         -1 if no device.
 */
long enableDevice(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum /* If more than one device in system, device number are ordered as 0, 1, 2,... */
)
{
	unsigned short value;
    unsigned char control;

    /* Detect SMI device */

    return 0;
}

/*
 * Get a pointer to the base of Memory Mapped IO space.
 *
 * Return: A pointer to base of MMIO.
 *         NULL pointer if fail.
 */
void *getMmioBase( 
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum /* If more than one device in system, device number are ordered as 0, 1, 2,... */
)
{
    unsigned long physicalAddr;

    if (enableDevice(vendorId, deviceId, deviceNum) == -1)
        return (void *)0;

    /* Get MMIO physical address */
//    physicalAddr = readPCIDword(vendorId, deviceId, deviceNum, 0x14);
        
    DDKDEBUGPRINT((INIT_LEVEL, "Mmio Physical Addr: %08x\n", physicalAddr));
    if (physicalAddr == (-1))
       return (void *)0;
    
    /* Map it into the address space. */
    return NULL;
    // return mapPhysicalAddress((void*)physicalAddr, SM50X_PCI_ALLOC_MMIO_SIZE);
   // return (x);
}

/* 
 * Get a pointer to the base of physical video memory. This can be used for DMA transfer.
 * In DOS, the physical and the logical address most likely are the same.
 * Return: A pointer to base of physical video memory.
 *         NULL pointer if fail.
 */
void *getPhysicalMemoryBase(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum /* If more than one device in system, device number are ordered as 0, 1, 2,... */
)
{
    unsigned long physicalAddr;

    if (enableDevice(vendorId, deviceId, deviceNum) == -1)
        return (void *)0;    
    
    /*
     * Access frame buffer through the PCI linear address.  
     */
     
    /* Get frame buffer physical address */
//    physicalAddr = readPCIDword(vendorId, deviceId, deviceNum, 0x10);

    /*
     * In a memory bar (Frame buffer), bit 3 to bit 0 are used as the base
     * address information, therefore, we should mask this out and set them
     * to zero.
     */
    physicalAddr &= ~0x0000000F;

    DDKDEBUGPRINT((INIT_LEVEL, "Memory Physical Addr: %08x\n", physicalAddr));
    if (physicalAddr == (-1))
       return (void *)0;

    return ((void *)physicalAddr);
}

/* 
 * Get a pointer to the base of video memory (or frame buffer).
 *
 * Return: A pointer to base of video memory.
 *         NULL pointer if fail.
 */
void *getMemoryBase(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum /* If more than one device in system, device number are ordered as 0, 1, 2,... */
)
{
    void *physicalAddr;

    physicalAddr = getPhysicalMemoryBase(vendorId, deviceId, deviceNum);
    if (physicalAddr == (void *)0)
       return (void *)0;

    /* Map it into the address space */
return NULL;
//    return (mapPhysicalAddress(physicalAddr, SM50X_PCI_ALLOC_MEMORY_SIZE));
}

/*
 * Get the physical address of an offset location in frame buffer.
 * In DOS, the physical and the logical address most likely are the same.
 * Return: A valid address if success.
 *         NULL address if fail.
 */
void *getPhysicalAddress(unsigned long offset /* Offset from base of physical frame buffer */)
{
    if (frameBufPhysicalBase[gwCurDev] == (unsigned char *)0)
    {
        frameBufPhysicalBase[gwCurDev] = (unsigned char *)getPhysicalMemoryBase(SMI_PCI_VENDOR_ID, gwDeviceId, gwCurDev);
        DDKDEBUGPRINT((INIT_LEVEL, "Device: %x, Memory Physical Base Addr: %08x\n", gwCurDev, frameBufPhysicalBase[gwCurDev]));
        if (frameBufPhysicalBase[gwCurDev] == 0)
        {
            return((void *)0);
        }
    }
    
    return(frameBufPhysicalBase[gwCurDev] + offset);
}

/*
 * Get the logical address of an offset location in frame buffer.
 * Return: A valid address if success.
 *         NULL address if fail.
 */
void *getAddress(unsigned long offset /*Offset from base of frame buffer*/)
{
    if (frameBufBase[gwCurDev] == (unsigned char *)0)
    {
        frameBufBase[gwCurDev] = (unsigned char *)getMemoryBase(SMI_PCI_VENDOR_ID, gwDeviceId, gwCurDev);
        DDKDEBUGPRINT((INIT_LEVEL, "Device: %x, Memory Base Addr: %08x\n", gwCurDev, frameBufBase[gwCurDev]));
        if (frameBufBase[gwCurDev] == 0)
        {
            return((void *)0);
        }
    }

    return(frameBufBase[gwCurDev] + offset);
}

/*
 * Get the logical address of an offset location in MMIO space.
 * Return: A valid address if success.
 *         NULL address if fail.
 */
void *getMemoryMappedAddress(unsigned long offset /*Offset from base of MMIO*/)
{
    if (mmioBase[gwCurDev] == (unsigned char *)0)
    {
        mmioBase[gwCurDev] = (unsigned char *)getMmioBase(SMI_PCI_VENDOR_ID, gwDeviceId, gwCurDev);
        if (mmioBase[gwCurDev] == 0)
        {
            return((void *)0);
        }
    }

    return(mmioBase[gwCurDev] + offset);
}

/*
 * Get the IRQ number of the current device.
 */
unsigned char getIRQ()
{
return 0;
//    return (readPCIByte(SMI_PCI_VENDOR_ID, gwDeviceId, gwCurDev, 0x3C));
}

/*
 * This function detects what SMI chips is in the system.
 *
 * Return: A non-zero device ID if SMI chip is detected.
 *         Zero means NO SMI chip is detected.
 */
unsigned short detectDevices()
{
    unsigned char i, j;
    
    /* Clean up the MMIO and Frame Buffer structure first. 
       Otherwise, calling this function twice, will result in different gwDeviceID. 
     */
    for (i = 0; i < MAX_SMI_DEVICE; i++)
    {
        frameBufBase[i] = 0;
        mmioBase[i] = 0;
        frameBufPhysicalBase[i] = 0;
    }

    /* Walk through the device ID table */
    for (i=0; gwDeviceIdTable[i] != 0; i++)
    {
        /* For a specific device, find out how many exist.
          This library supports a max of 4 devices of the same ID.
        */
        gwDeviceId = gwDeviceIdTable[i];
        gwNumOfDev = 0;

        for (j=0; j<MAX_SMI_DEVICE; j++)
        {
            gwCurDev = j;
            if (getMemoryMappedAddress(0) == (void *)0)
                break;

            if (getAddress(0) == (void *)0)
                break;

            gwNumOfDev++;
        }

        /* If a device is detected, don't need to try the next */
        if (gwNumOfDev > 0) break;
    }

    gwCurDev = 0; /* Always defaults to device 0 after initialization */

    /* Return the ID of detected device */
    return gwDeviceIdTable[i];
}

/*
 * How many devices of the same ID are there.
 */
unsigned short getNumOfDevices()
{
    return gwNumOfDev;
}
 
/*
 * This function sets up the current accessible device, if more
 * than one of the same ID exist in the system.
 *
 * Note:
 * Single device application don't need to call this function.
 * This function is to control multiple devices.
 */
long setCurrentDevice(unsigned short dev)
{
    /* Error check */
    if ( dev >= gwNumOfDev)
        return -1;

    gwCurDev = dev;
    return 0;
}

/*
 * This function gets the current accessible device index.
 */
unsigned short getCurrentDevice()
{
    return gwCurDev;
}

unsigned char peekByte(unsigned long offset)
{
    void *pAddr;

    if ((pAddr = getAddress(offset)) == (void *)0)
        return(0xFF);

    return *(volatile unsigned char *)(pAddr);
}

unsigned short peekWord(unsigned long offset)
{
    void *pAddr;

    if ((pAddr = getAddress(offset)) == (void *)0)
        return(0xFFFF);

    return *(volatile unsigned short *)(pAddr);
}

unsigned long peekDWord(unsigned long offset)
{
#if 0
    void *pAddr;

    if ((pAddr = getAddress(offset)) == (void *)0)
        return(0xFFFFFFFF);

    return *(volatile unsigned long *)(pAddr);
#else
    return ddk502_PEEK32(offset);
#endif
}

void poke_4_Byte(unsigned long offset, unsigned char *buf)
{
    unsigned long tData;
    void *pAddr;

    tData = ((unsigned long)buf[3]<<24) + ((unsigned long)buf[2]<<16) + ((unsigned int)buf[1]<<8) + buf[0];

    if ((pAddr = getAddress(offset)) != (void *)0)
        *(volatile unsigned long *)(pAddr) = tData;
}
 
void pokeByte(unsigned long offset, unsigned char value)
{
    void *pAddr;

    if ((pAddr = getAddress(offset)) != (void *)0)
        *(volatile unsigned char *)(pAddr) = value;
}

void pokeWord(unsigned long offset, unsigned short value)
{
    void *pAddr;

    if ((pAddr = getAddress(offset)) != (void *)0)
        *(volatile unsigned short *)(pAddr) = value;
}

void pokeDWord(unsigned long offset, unsigned long value)
{
#if 0
    void *pAddr;

    if ((pAddr = getAddress(offset)) != (void *)0)
        *(volatile unsigned long *)(pAddr) = value;
#else
    ddk502_POKE32(offset, value);
    return;
#endif
}

/* REGISTER SPACE */

unsigned char peekRegisterByte(unsigned long offset)
{
    void *pAddr;

    if ((pAddr = getMemoryMappedAddress(offset)) == (void *)0)
        return (0xFF);

   return *(volatile unsigned char *)(pAddr);
}

unsigned short peekRegisterWord(unsigned long offset)
{
    void *pAddr;

    if ((pAddr = getMemoryMappedAddress(offset)) == (void *)0)
        return (0xFFFF);

   return *(volatile unsigned short *)(pAddr);
}

unsigned long peekRegisterDWord(unsigned long offset)
{
#if 0
    void *pAddr;

    if ((pAddr = getMemoryMappedAddress(offset)) == (void *)0)
        return (0xFFFFFFFF);

   return *(volatile unsigned long *)(pAddr);
#else
    return ddk502_PEEK32(offset);
#endif
}

void pokeRegisterByte(unsigned long offset, unsigned char value)
{
    void *pAddr;

    if ((pAddr = getMemoryMappedAddress(offset)) != (void *)0)
        *(volatile unsigned char *)(pAddr) = value;
}

void pokeRegisterWord(unsigned long offset, unsigned short value)
{
    void *pAddr;

    if ((pAddr = getMemoryMappedAddress(offset)) != (void *)0)
        *(volatile unsigned short *)(pAddr) = value;
}

void pokeRegisterDWord(unsigned long offset, unsigned long value)
{
#if 0
    void *pAddr;

    if ((pAddr = getMemoryMappedAddress(offset)) != (void *)0)
        *(volatile unsigned long *)(pAddr) = value;
#else
    return ddk502_POKE32(offset, value);
#endif
}
