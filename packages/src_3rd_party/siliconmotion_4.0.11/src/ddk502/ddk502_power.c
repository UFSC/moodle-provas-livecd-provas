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
#include "ddk502_help.h"

/* Semaphore Counter for Bus Master Enable Bit */
unsigned long g_ulBusMasterSemaphoreCounter[MAX_SMI_DEVICE] = { 0, 0, 0, 0};
unsigned long g_ulPCISlaveBurstWriteSemaphoreCounter[MAX_SMI_DEVICE] = { 0, 0, 0, 0};
unsigned long g_ulPCISlaveBurstReadSemaphoreCounter[MAX_SMI_DEVICE] = { 0, 0, 0, 0};
unsigned long g_ulHostSemaphoreCounter[MAX_SMI_DEVICE] = { 0, 0, 0, 0};

/* Program new power mode */
void setPower(unsigned long Gate, unsigned long Clock)
{
    unsigned long gate_reg, clock_reg;
    unsigned long control_value;

    /* Get current power mode */
    control_value = FIELD_GET(peekRegisterDWord(POWER_MODE_CTRL),
			      POWER_MODE_CTRL,
			      MODE);

    /* Switch from mode 1 or sleep to mode 0 */
    gate_reg = POWER_MODE0_GATE;
    clock_reg = POWER_MODE0_CLOCK;
    control_value = FIELD_SET(control_value, POWER_MODE_CTRL, MODE, MODE0);

    /* Program new power mode */
    pokeRegisterDWord(gate_reg, Gate);
    pokeRegisterDWord(clock_reg, Clock);
    pokeRegisterDWord(POWER_MODE_CTRL, control_value);

    /* When returning from sleep, wait until finished */
    while (FIELD_GET(peekRegisterDWord(POWER_MODE_CTRL), POWER_MODE_CTRL,
	     SLEEP_STATUS) == POWER_MODE_CTRL_SLEEP_STATUS_ACTIVE) ;
}

/* Set DPMS state */
_X_EXPORT void ddk502_setDPMS(DPMS_t state)
{
    unsigned long value;

    value = peekRegisterDWord(SYSTEM_CTRL);
    switch (state)
    {
       case DPMS_ON:
	    value = FIELD_SET(value, SYSTEM_CTRL, DPMS, VPHP);
	    break;

       case DPMS_STANDBY:
	    value = FIELD_SET(value, SYSTEM_CTRL, DPMS, VPHN);
	    break;

       case DPMS_SUSPEND:
	    value = FIELD_SET(value, SYSTEM_CTRL, DPMS, VNHP);
	    break;

       case DPMS_OFF:
	    value = FIELD_SET(value, SYSTEM_CTRL, DPMS, VNHN);
	    break;
    }

    pokeRegisterDWord(SYSTEM_CTRL, value);
}

/*
 * This function gets the power mode, one of three modes: 0, 1 or Sleep.
 * On hardware reset, power mode 0 is default.
 */
unsigned long getPowerMode()
{
    return (FIELD_GET(peekRegisterDWord(POWER_MODE_CTRL), POWER_MODE_CTRL, MODE));
}

/*
 * SM50x can operate in one of three modes: 0, 1 or Sleep.
 * On hardware reset, power mode 0 is default.
 */
void setPowerMode(unsigned long powerMode)
{
    unsigned long control_value = 0;

    switch (powerMode)
    {
       case POWER_MODE_CTRL_MODE_MODE0:
	    control_value = FIELD_SET(control_value, POWER_MODE_CTRL, MODE, MODE0);
	    break;

       case POWER_MODE_CTRL_MODE_MODE1:
	    control_value = FIELD_SET(control_value, POWER_MODE_CTRL, MODE, MODE1);
	    break;

       case POWER_MODE_CTRL_MODE_SLEEP:
	    control_value = FIELD_SET(control_value, POWER_MODE_CTRL, MODE, SLEEP);
	    break;

       default:
	    break;
    }

    /* Program new power mode. */
    pokeRegisterDWord(POWER_MODE_CTRL, control_value);

    /* When returning from sleep, wait until finished. */
    while (FIELD_GET(peekRegisterDWord(POWER_MODE_CTRL), POWER_MODE_CTRL,
	 SLEEP_STATUS) == POWER_MODE_CTRL_SLEEP_STATUS_ACTIVE) ;
}

_X_EXPORT void ddk502_setCurrentGate(unsigned long gate)
{
    unsigned long gate_reg, clock_reg;
    unsigned long control_value = 0;

    /* Get current power mode. */
    control_value = FIELD_GET(peekRegisterDWord(POWER_MODE_CTRL),
			      POWER_MODE_CTRL,
			      MODE);

    switch (control_value)
    {
       case POWER_MODE_CTRL_MODE_MODE0:
	    gate_reg = POWER_MODE0_GATE;
	    break;

       case POWER_MODE_CTRL_MODE_MODE1:
	    gate_reg = POWER_MODE1_GATE;
	    break;

       case POWER_MODE_CTRL_MODE_SLEEP:
	    
       default:
	    return; /* Nothing to set in sleep mode */
	}
    pokeRegisterDWord(gate_reg, gate);
}

void setCurrentClock(unsigned long clock)
{
    unsigned long clock_reg;
    unsigned long control_value = 0;

    /* Get current power mode. */
    control_value = FIELD_GET(peekRegisterDWord(POWER_MODE_CTRL),
			      POWER_MODE_CTRL,
			      MODE);

    switch (control_value)
    {
       case POWER_MODE_CTRL_MODE_MODE0:
	    clock_reg = POWER_MODE0_CLOCK;
	    break;

       case POWER_MODE_CTRL_MODE_MODE1:
	    clock_reg = POWER_MODE1_CLOCK;
	    break;

       case POWER_MODE_CTRL_MODE_SLEEP:
       default:
	    return; /* Nothing to set in sleep mode */
	}
    pokeRegisterDWord(clock_reg, clock);
}


/*
 * This function enable/disable Bus Master
 */
void enableBusMaster(unsigned long enable)
{
	unsigned long busMasterCounter, value;

    /* Enable Bus Master as necessary.*/
	busMasterCounter = g_ulBusMasterSemaphoreCounter[getCurrentDevice()];
	value = peekRegisterDWord(SYSTEM_CTRL);

    if (enable)
    {
        if (busMasterCounter == 0)
		{
			value = FIELD_SET(value, SYSTEM_CTRL, PCI_MASTER, START);
#if 1		/* Is it necessary to enable/disable the PCI Clock Run??? */
			value = FIELD_SET(value, SYSTEM_CTRL, PCI_CLOCK_RUN, ENABLE);
#endif
			/*enableHost(1);*/
            pokeRegisterDWord(SYSTEM_CTRL, value);
        }

        busMasterCounter++;
    }
    else
    {
        if (busMasterCounter > 0)
            busMasterCounter--;
        
        if (busMasterCounter == 0)
		{
			value = FIELD_SET(value, SYSTEM_CTRL, PCI_MASTER, STOP);
#if 1		/* Is it necessary to enable/disable PCI Clock Run??? */
			value = FIELD_SET(value, SYSTEM_CTRL, PCI_CLOCK_RUN, DISABLE); 
#endif
			/*enableHost(0);*/
            pokeRegisterDWord(SYSTEM_CTRL, value);
		}
    }

	g_ulBusMasterSemaphoreCounter[getCurrentDevice()] = busMasterCounter;
}

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
)
{
	unsigned long pciMasterBaseAddress;

	/* Set System Memory Address */
	pciMasterBaseAddress = FIELD_VALUE(0, PCI_MASTER_BASE, ADDRESS, physicalSystemMemAddress & 0xFFF00000);
    pokeRegisterDWord(PCI_MASTER_BASE, pciMasterBaseAddress);

	/* Send back the remaining address */
    return (physicalSystemMemAddress - pciMasterBaseAddress);
}

/*
 * 	This function enable/disable PCI Slave Burst Write provided the CPU supports Write Combine.
 *
 *	Input:
 *			enable		- Enable/Disable the PCI Slave Burst Write (0 = disable, 1 = enable)
 */
void enablePCISlaveBurstWrite(
	unsigned long enable
)
{
	unsigned long pciSlaveBurstWriteCounter, value;

    /* Enable PCI Slave Burst Write */
	pciSlaveBurstWriteCounter = g_ulPCISlaveBurstWriteSemaphoreCounter[getCurrentDevice()];
	value = peekRegisterDWord(SYSTEM_CTRL);

    if (enable != 0)
    {
        if (pciSlaveBurstWriteCounter == 0)
		{
			/* Enable PCI Slave Burst Write. */
			value = FIELD_SET(peekRegisterDWord(SYSTEM_CTRL), SYSTEM_CTRL, PCI_BURST, ENABLE); 
			pokeRegisterDWord(SYSTEM_CTRL, value);
        }

        pciSlaveBurstWriteCounter++;
    }
    else
    {
        if (pciSlaveBurstWriteCounter > 0)
            pciSlaveBurstWriteCounter--;
        
        if (pciSlaveBurstWriteCounter == 0)
		{
			/* Disable PCI Slave Burst Write */
			value = FIELD_SET(peekRegisterDWord(SYSTEM_CTRL), SYSTEM_CTRL, PCI_BURST, DISABLE); 
			pokeRegisterDWord(SYSTEM_CTRL, value);
		}
    }

	g_ulPCISlaveBurstWriteSemaphoreCounter[getCurrentDevice()] = pciSlaveBurstWriteCounter;
}

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
)
{
	unsigned long pciSlaveBurstReadCounter, value;

    /* Currently, only SM718 needs to enable the Bus Master enable bit. 
       The Bus Master in SM750 is enabled by default, without programming any bits. */
	pciSlaveBurstReadCounter = g_ulPCISlaveBurstReadSemaphoreCounter[getCurrentDevice()];
	value = peekRegisterDWord(SYSTEM_CTRL);

    if (enable != 0)
    {
        if (pciSlaveBurstReadCounter == 0)
		{
			value = peekRegisterDWord(SYSTEM_CTRL);

			/* Enable PCI Slave Burst Read. */
			value = FIELD_SET(value, SYSTEM_CTRL, PCI_BURST_READ, ENABLE); 

			/* Set the Read Size */
			switch(burstReadSize)
			{
				case 1:
					value = FIELD_SET(value, SYSTEM_CTRL, PCI_SLAVE_BURST_READ_SIZE, 1);
					break;
				case 2:
					value = FIELD_SET(value, SYSTEM_CTRL, PCI_SLAVE_BURST_READ_SIZE, 2);
					break;
				case 4:
					value = FIELD_SET(value, SYSTEM_CTRL, PCI_SLAVE_BURST_READ_SIZE, 4);
					break;
				default:
				case 8:
					value = FIELD_SET(value, SYSTEM_CTRL, PCI_SLAVE_BURST_READ_SIZE, 8);
					break;
			}
			pokeRegisterDWord(SYSTEM_CTRL, value);
        }

        pciSlaveBurstReadCounter++;
    }
    else
    {
        if (pciSlaveBurstReadCounter > 0)
            pciSlaveBurstReadCounter--;
        
        if (pciSlaveBurstReadCounter == 0)
		{
			/* Disable PCI Slave Burst */
			value = FIELD_SET(peekRegisterDWord(SYSTEM_CTRL), SYSTEM_CTRL, PCI_BURST_READ, DISABLE); 
			pokeRegisterDWord(SYSTEM_CTRL, value);
		}
    }

	g_ulPCISlaveBurstReadSemaphoreCounter[getCurrentDevice()] = pciSlaveBurstReadCounter;
}

/* 
 * This function enable/disable the 2D engine.
 */
void enable2DEngine(unsigned long enable)
{
    unsigned long gate;

    gate = peekRegisterDWord(CURRENT_POWER_GATE);
    if (enable)
    {
        gate = FIELD_SET(gate, CURRENT_POWER_GATE, 2D,  ENABLE);
        gate = FIELD_SET(gate, CURRENT_POWER_GATE, CSC, ENABLE);
    }
    else
    {
        gate = FIELD_SET(gate, CURRENT_POWER_GATE, 2D,  DISABLE);
        gate = FIELD_SET(gate, CURRENT_POWER_GATE, CSC, DISABLE);
    }

    ddk502_setCurrentGate(gate);
}

/* 
 * This function enable/disable the ZV Port.
 */
void enableZVPort(
    unsigned long enable
)
{
    unsigned long gate;
    
    /* Enable ZV Port Gate */
    gate = peekRegisterDWord(CURRENT_POWER_GATE);
    if (enable)
    {
        gate = FIELD_SET(gate, CURRENT_POWER_GATE, ZVPORT, ENABLE);
        gate = FIELD_SET(gate, CURRENT_POWER_GATE, GPIO_PWM_I2C, ENABLE);        
    }
    else
    {
        /* Disable ZV Port Gate. There is no way to know whether the GPIO pins are being used
           or not. Therefore, do not disable the GPIO gate. */
        gate = FIELD_SET(gate, CURRENT_POWER_GATE, ZVPORT, DISABLE);
    }
    
    ddk502_setCurrentGate(gate);
}

/*
 * This function enable/disable the 8051 gate
 */
void enable8051(
    unsigned long enable
)
{
    unsigned long gate;
 
    gate = peekRegisterDWord(CURRENT_POWER_GATE);
    if (enable)
    {
        /* Enable 8051 Gate */ 
        gate = FIELD_SET(gate, CURRENT_POWER_GATE, 8051, ENABLE);
    }
    else
    {
        /* Disable 8051 Gate */ 
        gate = FIELD_SET(gate, CURRENT_POWER_GATE, 8051, DISABLE);
    }
    
    ddk502_setCurrentGate(gate);
}

/*
 * This function enable/disable the AC97 or I2S gate
 */
void enableAC97_I2S(
    unsigned long enable
)
{
    unsigned long gate;
 
    gate = peekRegisterDWord(CURRENT_POWER_GATE);
    if (enable)
    {
        /* Enable AC97/I2S Gate */ 
        gate = FIELD_SET(gate, CURRENT_POWER_GATE, AC97_I2S, ENABLE);
    }
    else
    {
        /* Disable AC97/I2S Gate */ 
        gate = FIELD_SET(gate, CURRENT_POWER_GATE, AC97_I2S, DISABLE);
    }
    
    ddk502_setCurrentGate(gate);
}

/* 
 * This function enable/disable the Host Interface, Command Interpreter, and DMA Engine
 */
void enableHost(unsigned long enable)
{
	unsigned long hostCounter, gate;

	hostCounter = g_ulHostSemaphoreCounter[getCurrentDevice()];

	gate = peekRegisterDWord(CURRENT_POWER_GATE);
    if (enable)
    {
        if (hostCounter == 0)
			gate = FIELD_SET(gate, CURRENT_POWER_GATE, HOST, ENABLE);

        hostCounter++;
    }
    else
    {
        if (hostCounter > 0)
            hostCounter--;
        
        if (hostCounter == 0)
			gate = FIELD_SET(gate, CURRENT_POWER_GATE, HOST, DISABLE);
    }
	ddk502_setCurrentGate(gate);

	g_ulHostSemaphoreCounter[getCurrentDevice()] = hostCounter;
}

/* 
 * This function enable/disable the Host Interface, Command Interpreter, and DMA Engine
 */
void enableDMA(unsigned long enable)
{
	enableHost(enable);
}

