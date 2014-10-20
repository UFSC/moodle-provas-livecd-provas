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

#ifndef _OS_H_
#define _OS_H_

typedef enum _intel_cpu_id_t
{
	INTEL_CPU_ID_8086_8088 = 0,
	INTEL_CPU_ID_186_188,
	INTEL_CPU_ID_286,
	INTEL_CPU_ID_386,
	INTEL_CPU_ID_486,
	INTEL_CPU_ID_586
} 
intel_cpu_id_t;

/*
 * Important:
 * How to implement the functions here is OS, bus and CPU dependent.
 * Please refer to OS.C as an implementation example for WATCOM DOS extender.
 *
 * By keeping the same interface here, rest of the codes in this DDK are
 * pretty much portable for different compilers.
 */

/*
 * This function reads a DWord value from the PCI configuration space
 * of a specific PCI device.
 *
 * Inputs are the Vendor and device ID of the device in question.
 * Return: a DWord value.
 */
unsigned long readPCIDword(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum, /* If more than one device in system, device number are ordered as 0, 1, 2,... */
    unsigned short offset     /* Offset in configuration space to be read */
);

/*
 * This function writes a dword value to the PCI configuration space
 * of a specific device.
 *
 * Inputs are the Vendor and device ID of the device in question.
 * Return:  0 = Success.
 *         -1 = Fail.
 */
long writePCIDword(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum, /* If more than one device in system, device number are ordered as 0, 1, 2,... */
    unsigned short offset,   /* Offset in configuration space to be written */
    unsigned long value     /* To be written BYTE value */
);

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
);

/*
 * This function writes a word value to the PCI configuration space
 * of a specific device.
 *
 * Inputs are the Vendor and device ID of the device in question.
 * Return:  0 = Success.
 *         -1 = Fail.
 */
long writePCIWord(
    unsigned short vendorId, /* PCI vendor ID */
    unsigned short deviceId, /* PCI device ID */
    unsigned short deviceNum, /* If more than one device in system, device number are ordered as 0, 1, 2,... */
    unsigned short offset,   /* Offset in configuration space to be written */
    unsigned short value     /* To be written BYTE value */
);

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
);

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
);

/*
 * This function maps a physical address into logical address.
 * Return: NULL address pointer if fail
 *         A Logical address pointer if success.
 */
void *mapPhysicalAddress(
    void *phyAddr,            /* 32 bit physical address */
    unsigned long size        /* Memory size to be mapped */
);

/*
 * This function unmaps a linear logical address obtained by mapPhysicalAddress.
 * Return:
 *      0   - Success
 *     -1   - Fail
 */
long unmapPhysicalAddress(
    void *linearAddr,         /* 32 bit linear address */
	unsigned long size
);

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
#define MAX_TICKCOUNT                       1573040
unsigned long getSystemTickCount();
 
/* Get current time in milliseconds. */
#define MAX_TIME_VALUE                      (24*3600*1000)
unsigned long getCurrentTime();

/*******************************************************************
 * Interrupt implementation support from the OS
 * 
 * This implementation is used for handling the interrupt.
 *******************************************************************/

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
#ifdef WDOSE     
    void (interrupt far *pfnHandler)()
#else
    void (*pfnHandler)()
#endif
);

/* 
 * Unregister an interrupt handler from the interrupt vector table
 *
 * Input:
 *          irqNumber   - IRQ Number
 */
short unregisterInterrupt(
    unsigned char irqNumber
);

/* 
 * Signal the End Of Interrupt to the system and chain the interrupt
 * if necessary.
 *
 * Input:
 *          irqNumber   - IRQ Number
 */
void interruptEOI(
    unsigned char irqNumber
);

/*******************************************************************
 * Timer Interrupt implementation
 * 
 * This implementation is used for handling the SM50x interrupt.
 *******************************************************************/

/*
 * Register an interrupt handler function to an interrupt status.
 * The interrupt is happens every 55ms, or about 18.2 ticks per second.
 */ 
short registerTimerHandler(
    void (*handler)(void),
    unsigned long totalTicks        /* Total number of ticks to wait */
);

/*
 * Un-register a registered interrupt handler
 */
short unregisterTimerHandler(
    void (*handler)(void)
);

/*******************************************************************
 * COM Port implementation
 * 
 * This implementation is used by Debug module to send any 
 * debugging messages to other system through serial port.
 *******************************************************************/
typedef enum _baud_rate_t
{
    COM_2400 = 0,
    COM_4800,
    COM_9600,
    COM_19200,
    COM_38400,
    COM_57600,
    COM_115200
} 
baud_rate_t;

typedef enum _data_size_t
{
    DATA_SIZE_8,
    DATA_SIZE_7
}
data_size_t;

typedef enum _parity_t
{
    PARITY_NONE = 0,
    PARITY_ODD,
    PARITY_EVEN,
    PARITY_SPACE
}
parity_t;

typedef enum _stop_bit_t
{
    STOP_BIT_1,
    STOP_BIT_2
}
stop_bit_t;

typedef enum _flow_ctrl_t
{
    FLOW_CONTROL_NONE = 0,
    FLOW_CONTROL_HW,
    FLOW_CONTROL_SW
}
flow_ctrl_t;

/* 
 * Initialize the serial port.
 *
 * Parameters:
 *      comPortIndex    - Serial Port Index
 *      baudRate        - The communication speed to be set
 *      dataSize        - Number of bits per characters
 *      parity          - Error checking
 *      stopBit         - Number of bits used as the end of each character
 *      flowCtrl        - Serial port Flow Control (handshake)
 *
 * Returns:
 *       0  - Success
 *      -1  - Fail
 */
long comInit(
    unsigned char comPortIndex,
    baud_rate_t baudRate,
    data_size_t dataSize,
    parity_t parity,
    stop_bit_t stopBit,
    flow_ctrl_t flowCtrl
);

/* 
 * Send data out to through the serial port.
 *
 * Parameters:
 *      pszPrintBuffer  - Pointer to a buffer which data to be sent out
 *      length          - Number of characters to be sent out.
 *
 * Returns:
 *      Number of characters that are actually sent out.
 */
unsigned long comWrite(
    char* pszPrintBuffer,
    unsigned long length
);

/* 
 * Close the serial communication port.
 */
void comClose();

/*******************************************************************
 * CPU implementation support from the OS
 * 
 * This implementation is used to supports Write Combine.
 * It only applies to Pentium Pro.
 *******************************************************************/

/*
 *	detectCPU
 *		This function detects the CPU ID
 *
 *	Output:
 *		INTEL_CPU_ID_8086_8088 		- 8086/8088 Intel CPU
 *		INTEL_CPU_ID_186_188		- 186 Intel CPU
 *		INTEL_CPU_ID_286			- 286 Intel CPU
 *		INTEL_CPU_ID_386			- 386 Intel CPU
 *		INTEL_CPU_ID_486			- 486 Intel CPU
 *		INTEL_CPU_ID_586			- 586 Intel CPU (Pentium)
 */
intel_cpu_id_t detectIntelCPU();

/*
 *	This function enable the write combine (burst)
 */
long registerWCMemoryRange(
	void *phyAddr,            /* 32 bit physical address */
    unsigned long size
);

/*
 *	This function enable the write combine (burst)
 */
long unregisterWCMemoryRange(
	void *phyAddr,            /* 32 bit physical address */
    unsigned long size
);

#endif /* _OS_H_ */
