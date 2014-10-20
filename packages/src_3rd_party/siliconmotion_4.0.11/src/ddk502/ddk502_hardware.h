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

#ifndef _HARDWARE_H_
#define _HARDWARE_H_

/* Silicon Motion PCI vendor ID */
#define SMI_PCI_VENDOR_ID 0x126F

/* Maximum number of devices with the same ID supported by this library. */
#define MAX_SMI_DEVICE                  4

/* Size of the SM50X MMIO and memory */
#define SM50X_PCI_ALLOC_MMIO_SIZE       (2*1024*1024)
#define SM50X_PCI_ALLOC_MEMORY_SIZE     (64*1024*1024)

/*
 * Get the physical address of an offset location in frame buffer.
 * In DOS, the physical and the logical address most likely are the same.
 * Return: A valid address if success.
 *         NULL address if fail.
 */
void *getPhysicalAddress(unsigned long offset /* Offset from base of physical frame buffer */);

/*
 * Get the logical address of an offset location in frame buffer.
 * Return: A valid address if success.
 *         NULL address if fail.
 */
void *getAddress(unsigned long offset /*Offset from base of frame buffer*/);

/*
 * Get the logical address of an offset location in MMIO space.
 * Return: A valid address if success.
 *         NULL address if fail.
 */
void *getMemoryMappedAddress(unsigned long offset /*Offset from base of MMIO*/);

/*
 * This function detects what SMI chips is in the system.
 *
 * Return: A non-zero device ID if SMI chip is detected.
 *         Zero means NO SMI chip is detected.
 */
unsigned short detectDevices(void);

/*
 * How many devices of the same ID are there.
 */
unsigned short getNumOfDevices(void);

/* 
 * This function gets the IRQ line for the device in use
 */
unsigned char getIRQ(void);

/*
 * This function sets up the current accessible device, if more
 * than one of the same ID exist in the system.
 *
 * Note:
 * Single device application don't need to call this function.
 * This function is to control multiple devices.
 */
long setCurrentDevice(unsigned short dev);
unsigned short getCurrentDevice(void);

/* Video Memory read/write functions */
unsigned char peekByte(unsigned long offset);
unsigned short peekWord(unsigned long offset);
unsigned long peekDWord(unsigned long offset);
void poke_4_Byte(unsigned long offset, unsigned char *buf);
void pokeByte(unsigned long offset, unsigned char value);
void pokeWord(unsigned long offset, unsigned short value);
void pokeDWord(unsigned long offset, unsigned long value);

/* MMIO read/write functions */
unsigned char peekRegisterByte(unsigned long offset);
unsigned short peekRegisterWord(unsigned long offset);
unsigned long peekRegisterDWord(unsigned long offset);
void pokeRegisterByte(unsigned long offset, unsigned char value);
void pokeRegisterWord(unsigned long offset, unsigned short value);
void pokeRegisterDWord(unsigned long offset, unsigned long value);
unsigned char getIRQ();
#endif /* _HARDWARE_H_ */
