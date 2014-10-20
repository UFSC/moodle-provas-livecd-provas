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

#include <fcntl.h>
#include <pci/pci.h>
#include <pci/types.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <asm/mtrr.h>
#include <sys/time.h>

#include "ddk502_voyager.h"
#include "ddk502_hardware.h"
//////
#include <stdint.h>

#include "ddk502_os.h"

#include "ddk502_clock.h"
#include "ddk502_mode.h"

#include <xf86str.h>

#include "xf86_OSproc.h"
#include <X11/X.h>
#include <X11/Xos.h>
#include <X11/Xproto.h>
#include "xf86i2c.h"

/* Global variable to save all the SMI devices */
static struct pci_dev *g_pCurrentDevice = (struct pci_dev *)0;
static struct pci_access *g_pAccess = (struct pci_access *)0;
static struct pci_filter g_filter;

/*
 * This function maps a physical address into logical address.
 * Return: NULL address pointer if fail
 *         A Logical address pointer if success.
 */
void *mapPhysicalAddress(
    void *phyAddr,            /* 32 bit physical address */
    unsigned long size        /* Memory size to be mapped */
)
{
    unsigned long address;
    int fileDescriptor;
        
    fileDescriptor = open("/dev/mem", O_RDWR);
    if (fileDescriptor == -1)
        return ((void *)0);
        
    address = (unsigned long) mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fileDescriptor, (unsigned long)phyAddr);

    if ((void *) address == MAP_FAILED)
		address = 0;

	close(fileDescriptor);
    
    return ((void *) address);
}

/*
 * This function unmaps a linear logical address obtained by mapPhysicalAddress.
 * Return:
 *      0   - Success
 *     -1   - Fail
 */
long unmapPhysicalAddress(
    void *linearAddr,				/* 32 bit linear address */
	unsigned long size
)
{
    return ((long)munmap(linearAddr, size));
}

/* Initialize the PCI */
long initPCI(
    unsigned short vendorId, 
    unsigned short deviceId, 
    unsigned short deviceNum
)
{
    unsigned short deviceIndex;
    
    if (g_pCurrentDevice != (struct pci_dev *)0)
        return 0;
    
    /* Get the pci_access structure */
    g_pAccess = pci_alloc();

    /* Initialize the PCI library */
    pci_init(g_pAccess);

    /* Set all options you want -- here we stick with the defaults */    
    pci_filter_init(g_pAccess, &g_filter);
  	g_filter.vendor = vendorId;
  	g_filter.device = deviceId;
  	
    /* Get the list of devices */
    pci_scan_bus(g_pAccess);
    for(g_pCurrentDevice = g_pAccess->devices, deviceIndex = 0; 
        g_pCurrentDevice; 
        g_pCurrentDevice = g_pCurrentDevice->next)
    {
        if (pci_filter_match(&g_filter, g_pCurrentDevice))
        {
            if (deviceIndex == deviceNum)
            {
                pci_fill_info(g_pCurrentDevice, PCI_FILL_IDENT | PCI_FILL_IRQ | PCI_FILL_BASES | PCI_FILL_ROM_BASE | PCI_FILL_SIZES);
                return 0;
            }
            
            /* Increment the device index */
            deviceIndex++;
        }
    }

    return (-1);
}

/*
 * This function reads a DWord value from the PCI configuration space
 * of a specific PCI device.
 *
 * Inputs are the Vendor and device ID of the device in question.
 * Return: a DWord value.
 */
unsigned long readPCIDword(
    unsigned short vendorId,    /* PCI vendor ID */
    unsigned short deviceId,    /* PCI device ID */
    unsigned short deviceNum,   /* If more than one device in system, device number are ordered as 0, 1, 2,... */
    unsigned short offset       /* Offset in configuration space to be read */
)
{
    if (initPCI(vendorId, deviceId, deviceNum) == 0)
    {
        return ((unsigned long) pci_read_long(g_pCurrentDevice, offset));
    }
    
    return 0;
}

/*
 * This function reads a Word value from the PCI configuration space
 * of a specific device.
 *
 * Inputs are the Vendor and device ID of the device in question.
 * Return: a WORD value.
 */
unsigned short readPCIWord(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum, /* If more than one device in system, device number are ordered as 0, 1, 2,... */
    unsigned short offset     /* Offset in configuration space to be read */
)
{
    if (initPCI(vendorId, deviceId, deviceNum) == 0)
    {
        return ((unsigned short) pci_read_word(g_pCurrentDevice, offset));
    }
    
    return 0;
}

/*
 * This function reads a byte value from the PCI configuration space
 * of a specific device.
 *
 * Inputs are the Vendor and device ID of the device in question.
 * Return: a BYTE value.
 */
unsigned char readPCIByte(
    unsigned short vendorId, /* PCI Vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum, /* If more than one device in system, device number are ordered as 0, 1, 2,... */
    unsigned short offset     /* Offset in configuration space to be read */
)
{
    if (initPCI(vendorId, deviceId, deviceNum) == 0)
    {
        return ((unsigned char) pci_read_byte(g_pCurrentDevice, offset));
    }
    
    return 0;
}

/*
 * This function writes a byte value to the PCI configuration space
 * of a specific device.
 *
 * Inputs are the Vendor and device ID of the device in question.
 * Return:  0 = Success.
 *         -1 = Fail.
 */
long writePCIByte(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum, /* If more than one device in system, device number are ordered as 0, 1, 2,... */
    unsigned short offset,      /* Offset in configuration space to be written */
    unsigned char value       /* To be written BYTE value */
)
{
    if (initPCI(vendorId, deviceId, deviceNum) == 0)
        return (pci_write_byte(g_pCurrentDevice, offset, value));
    
    return (-1);
}

/*******************************************************************
 * Time related functions
 * 
 * This implementation can be used for performance analysis or delay
 * function.
 *******************************************************************/

/* Tick count will be reset after midnight 24 hours. Therefore, when the tick count counter
   reset, it means that it has passed the 24 hours boundary. 
   A Timer tick occurs every 1,193,180 / 65536 (= ~18.20648) times per second. 
   The maximum tick count is calculated as follows: (24*3600*1000/54.9254) = ~1573040
 */
unsigned long getSystemTickCount()
{
    /* TODO: Check how to implement this in LINUX */
	return 0;
}

/* Get current time in milliseconds. */
unsigned long getCurrentTime()
{
	struct timeval currentTime;
	unsigned long milliseconds = 0;

	if (gettimeofday(&currentTime, (struct timezone *)0) == 0)
		milliseconds = (currentTime.tv_sec * 1000) + (currentTime.tv_usec / 1000);

	return milliseconds;
}

/*******************************************************************
 * Interrupt implementation support from the OS
 * 
 * This implementation is used for handling the SM50x interrupt.
 *******************************************************************/
short enableTimerInterrupt(
    unsigned char enable,
    void (*pfnHandler)()
)
{
    struct itimerval tick;
        
    /* Initialize struct */
    memset((void *)&tick, 0, sizeof(tick));

    if (enable != 0)
    {
        signal(SIGALRM, pfnHandler);
        
        /* Timeout to run function first time */
        tick.it_value.tv_sec = 0;               /* sec */
        tick.it_value.tv_usec = 5880/*16666*/;           /* micro sec. */

        /* Interval time to run function */
        tick.it_interval.tv_sec = 0;
        tick.it_interval.tv_usec = 5880/*16666*/;
    }

    /* Set timer, ITIMER_REAL : real-time to decrease timer, send SIGALRM when timeout */
    if (setitimer(ITIMER_REAL, &tick, NULL))
        return (-1);
        
    return (0);        
}

/* 
 * Register an interrupt handler (ISR) to the interrupt vector table associated
 * with the given irq number.
 *
 * Input:
 *          irqNumber   - IRQ Number
 *          pfnHandler  - Pointer to the ISR function
 *
 * Output:
 *           0  - Success
 *          -1  - Fail
 */
short registerInterrupt(
    unsigned char irqNumber, 
    void (*pfnHandler)()
)
{
    return enableTimerInterrupt(1, pfnHandler);
}

/* 
 * Unregister an interrupt handler from the interrupt vector table
 *
 * Input:
 *          irqNumber   - IRQ Number
 */
short unregisterInterrupt(
    unsigned char irqNumber
)
{
    return enableTimerInterrupt(0, (void *)0);
}

/* 
 * Signal the End Of Interrupt to the system and chain the interrupt
 * if necessary.
 *
 * Input:
 *          irqNumber   - IRQ Number
 */
void interruptEOI(
    unsigned char irqNumber
)
{
}

/*******************************************************************
 * CPU implementation support from the OS
 * 
 * This implementation is used to detect CPU and manipulate its 
 * registers.
 * It only applies to Pentium Pro.
 *******************************************************************/

/*
 *	registerWCMemoryRange
 *		This function registers the memory range as the memory write combine type (burst)
 *
 *	Input:
 *		phyAddr		- Physical Memory Address to be registered as WC memory type
 *		size		- The memory size to be registered.
 *
 *	Output:
 *		0	- Success
 *	   -1	- Fail
 *
 *  Note:
 *		1. This code is only works in Intel Pentium Pro and above CPU. Previous CPU does not
 *		   support Write Combine.
 *		2. Not all systems have their MTRR enabled by default. I believe it depends on the 
 *		   either the system BIOS enable it or not during boot up. In most of ASUS motherboard,
 *		   it seems that the system BIOS do a good job in setting up the MTRR when bootin up. 
 *		   However, in other motherboard, the MTRR seems to be disabled. In such case, it can
 *		   be enabled in Ring 0 privilege.
 */
long registerWCMemoryRange(
	void *phyAddr,            /* 32 bit physical address */
    unsigned long size
)
{
	int fileDescriptor;
	struct mtrr_sentry mtrrSentry;

	if ((fileDescriptor = open("/proc/mtrr", O_WRONLY, 0)) != (-1))
	{
		mtrrSentry.base = (unsigned long)phyAddr;
		mtrrSentry.size = size;
		mtrrSentry.type = MTRR_TYPE_WRCOMB;

		if (ioctl(fileDescriptor, MTRRIOC_ADD_ENTRY, &mtrrSentry) != (-1))
			return (0);
	}

	return (-1);
}

/*
 *	This function unregisters the memory range WC type
 */
long unregisterWCMemoryRange(
	void *phyAddr,            /* 32 bit physical address */
    unsigned long size
)
{
	int fileDescriptor;
	struct mtrr_sentry mtrrSentry;

	if ((fileDescriptor = open("/proc/mtrr", O_WRONLY, 0)) != (-1))
	{
		mtrrSentry.base = (unsigned long)phyAddr;
		mtrrSentry.size = size;
		mtrrSentry.type = MTRR_TYPE_UNCACHABLE;

		if (ioctl(fileDescriptor, MTRRIOC_DEL_ENTRY, &mtrrSentry) != (-1))
			return (0);
	}

	return (-1);
}


