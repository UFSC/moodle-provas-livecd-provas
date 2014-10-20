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

#ifndef _POWER_H_
#define _POWER_H_

typedef enum _DPMS_t
{
    DPMS_ON,
    DPMS_STANDBY,
    DPMS_SUSPEND,
    DPMS_OFF
}
DPMS_t;

/* 
 * This function sets the current power state 
 */
void setPower(unsigned long Gate, unsigned long Clock);

/*
 * This function sets the DPMS state 
 */
void ddk502_setDPMS(DPMS_t state);

/* 
 * This function gets the current power mode 
 */
unsigned long getPowerMode();

/* 
 * This function sets the current power mode
 */
void setPowerMode(unsigned long powerMode);

/* 
 * This function sets current gate 
 */
void ddk502_setCurrentGate(unsigned long gate);

/* 
 * This function sets the current clock 
 */
void setCurrentClock(unsigned long clock);

/*
 * This function enable/disable Bus Master
 */
void enableBusMaster(unsigned long enable);

/*
 * 	This function enable/disable PCI Slave Burst Write provided the CPU supports Write Combine.
 *
 *	Input:
 *			enable		- Enable/Disable the PCI Slave Burst Write (0 = disable, 1 = enable)
 */
void enablePCISlaveBurstWrite(
	unsigned long enable
);

/*
 * 	This function enable/disable PCI Slave Burst Read provided the CPU supports it.
 *
 *	Input:
 *			enable			- Enable/Disable the PCI Slave Burst Read (0 = disable, 1 = enable)
 *			burstReadSize	- Burst Read Size in 32-words (valid values are 1, 2, 4, and 8)
 */
void enablePCISlaveBurstRead(
	unsigned long enable,
	unsigned long burstReadSize
);

/* 
 *	setPCIMasterBaseAddress
 *		This function set the PCI Master Base Address (used by bus master or DMA).
 *
 *	Input:	
 *		physicalSystemMemAddress	- System physical memory address which PCI
 *									  Master Base Address to be set to.
 *
 *	Output:
 *		The memory address to be set in the register.  
 */
unsigned long setPCIMasterBaseAddress(
	unsigned long physicalSystemMemAddress
);

/* 
 * This function enable/disable the 2D engine.
 */
void enable2DEngine(unsigned long enable);

/* 
 * This function enable/disable the ZV Port 
 */
void enableZVPort(unsigned long enable);

/*
 * This function enable/disable the 8051 gate
 */
void enable8051(unsigned long enable);

/*
 * This function enable/disable the AC97 or I2S gate
 */
void enableAC97_I2S(unsigned long enable);

/* 
 * This function enable/disable the Host Interface, Command Interpreter, and DMA Engine
 */
void enableHost(unsigned long enable);

/* 
 * This function enable/disable the DMA Engine
 */
void enableDMA(unsigned long enable);

#endif /* _POWER_H_ */
