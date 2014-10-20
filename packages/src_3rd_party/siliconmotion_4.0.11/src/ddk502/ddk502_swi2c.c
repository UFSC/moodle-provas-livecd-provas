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

#include "../smi_common.h"
#include "ddk502_voyager.h"
#include "ddk502_hardware.h"
#include "ddk502_power.h"
#include "ddk502_swi2c.h"
#include "ddk502_help.h"
#include "ddk502_reggpio.h"

/*******************************************************************
 * I2C Software Master Driver:   
 * ===========================
 * Each i2c cycle is split into 4 sections. Each of these section marks
 * a point in time where the SCL or SDA may be changed. 
 * 
 * 1 Cycle == |  Section I. |  Section 2. |  Section 3. |  Section 4. |
 *            +-------------+-------------+-------------+-------------+
 *            | SCL set LOW |SCL no change| SCL set HIGH|SCL no change|
 *                 
 *                                          ____________ _____________
 * SCL == XXXX _____________ ____________ /
 *                 
 * I.e. the SCL may only be changed in section 1. and section 3. while
 * the SDA may only be changed in section 2. and section 4. The table
 * below gives the changes for these 2 lines in the varios sections.
 * 
 * Section changes Table:        
 * ======================
 * blank = no change, L = set bit LOW, H = set bit HIGH
 *                       
 *                                | 1.| 2.| 3.| 4.|      
 *                 ---------------+---+---+---+---+      
 *                 Tx Start   SDA |   | H |   | L |      
 *                            SCL | L |   | H |   |      
 *                 ---------------+---+---+---+---+                
 *                 Tx Stop    SDA |   | L |   | H |      
 *                            SCL | L |   | H |   |      
 *                 ---------------+---+---+---+---+                
 *                 Tx bit H   SDA |   | H |   |   |      
 *                            SCL | L |   | H |   |      
 *                 ---------------+---+---+---+---+                
 *                 Tx bit L   SDA |   | L |   |   |      
 *                            SCL | L |   | H |   |      
 *                 ---------------+---+---+---+---+                
 *                                  
 ******************************************************************/

/* GPIO pins used for this I2C. It ranges from 0 to 63. */
static unsigned char g_i2cClockGPIO = DEFAULT_I2C_SCL;
static unsigned char g_i2cDataGPIO = DEFAULT_I2C_SDA;

/*
 *  Below is the variable declaration for the GPIO pin register usage
 *  for the i2c Clock and i2c Data.
 *
 *  Note:
 *      Notice that the GPIO usage for the i2c clock and i2c Data are
 *      separated. This is to make this code flexible enough when 
 *      two separate GPIO pins for the clock and data are located
 *      in two different GPIO register set (worst case).
 */

/* i2c Clock GPIO Register usage */
static unsigned long g_i2cClkGPIOMuxReg = GPIO_MUX_HIGH;
static unsigned long g_i2cClkGPIODataReg = GPIO_DATA_HIGH;
static unsigned long g_i2cClkGPIODataDirReg = GPIO_DATA_DIRECTION_HIGH;

/* i2c Data GPIO Register usage */
static unsigned long g_i2cDataGPIOMuxReg = GPIO_MUX_HIGH;
static unsigned long g_i2cDataGPIODataReg = GPIO_DATA_HIGH;
static unsigned long g_i2cDataGPIODataDirReg = GPIO_DATA_DIRECTION_HIGH;

/*******************************************************************************
    swI2CWait
        This function puts a delay between command

    Parameters:
        None

    Return Value:
        None
 *******************************************************************************/        
static void swI2CWait(void)
{
    int i, Temp;

    for(i=0; i<600; i++)
    {
        Temp = i;
        Temp += i;
    }
}

/*******************************************************************************
    swI2CSCL
        This function set/reset the SCL GPIO pin

    Parameters:
        value	- Bit value to set to the SCL or SDA (0 = low, 1 = high)

    Return Value:
        None
 *******************************************************************************/ 
static void swI2CSCL(unsigned char value)
{
    unsigned long ulGPIOData;
    unsigned long ulGPIODirection;

    ulGPIODirection = peekRegisterDWord(g_i2cClkGPIODataDirReg);
    if (value)      /* High */
    {
        /* Set direction as input. This will automatically pull the signal up. */
        ulGPIODirection &= ~(1 << g_i2cClockGPIO);	
        pokeRegisterDWord(g_i2cClkGPIODataDirReg, ulGPIODirection);
    }
    else            /* Low */
    {
        /* Set the signal down */
        ulGPIOData = peekRegisterDWord(g_i2cClkGPIODataReg);
        ulGPIOData &= ~(1 << g_i2cClockGPIO);
        pokeRegisterDWord(g_i2cClkGPIODataReg, ulGPIOData);

        /* Set direction as output */
        ulGPIODirection |= (1 << g_i2cClockGPIO);		
        pokeRegisterDWord(g_i2cClkGPIODataDirReg, ulGPIODirection);
    }
}

/*******************************************************************************
    swI2CSDA
        This function set/reset the SDA GPIO pin

    Parameters:
        value	- Bit value to set to the SCL or SDA (0 = low, 1 = high)

    Return Value:
        None
 *******************************************************************************/
static void swI2CSDA(unsigned char value)
{
    unsigned long ulGPIOData;
    unsigned long ulGPIODirection;

    ulGPIODirection = peekRegisterDWord(g_i2cDataGPIODataDirReg);
    if (value)      /* High */
    {
        /* Set direction as input. This will automatically pull the signal up. */
        ulGPIODirection &= ~(1 << g_i2cDataGPIO);	
        pokeRegisterDWord(g_i2cDataGPIODataDirReg, ulGPIODirection);
    }
    else            /* Low */
    {
        /* Set the signal down */
        ulGPIOData = peekRegisterDWord(g_i2cDataGPIODataReg);
        ulGPIOData &= ~(1 << g_i2cDataGPIO);
        pokeRegisterDWord(g_i2cDataGPIODataReg, ulGPIOData);

        /* Set direction as output */
        ulGPIODirection |= (1 << g_i2cDataGPIO);		
        pokeRegisterDWord(g_i2cDataGPIODataDirReg, ulGPIODirection);
    }
}

/*******************************************************************************
    swI2CReadSDA
        This function read the data from the SDA GPIO pin

    Parameters:
        None

    Return Value:
        The SDA data bit sent by the Slave
 *******************************************************************************/
static unsigned char swI2CReadSDA()
{
    unsigned long ulGPIODirection;
    unsigned long ulGPIOData;

    /* Make sure that the direction is input (High) */
    ulGPIODirection = peekRegisterDWord(g_i2cDataGPIODataDirReg);
    if ((ulGPIODirection & (1 << g_i2cDataGPIO)) != (~(1 << g_i2cDataGPIO)))
    {
        ulGPIODirection &= ~(1 << g_i2cDataGPIO);
        pokeRegisterDWord(g_i2cDataGPIODataDirReg, ulGPIODirection);
    }

    /* Now read the SDA line */
    ulGPIOData = peekRegisterDWord(g_i2cDataGPIODataReg);
    if (ulGPIOData & (1 << g_i2cDataGPIO)) 
        return 1;
    else 
        return 0;
}

static unsigned char swI2CReadSCL()
{
    uint32_t ulGPIODirection;
    uint32_t ulGPIOData;

    /* Make sure that the direction is input (High) */
    ulGPIODirection = peekRegisterDWord(g_i2cClkGPIODataDirReg);
    if ((ulGPIODirection & (1 << g_i2cClockGPIO)) != (~(1 << g_i2cClockGPIO)))
    {
        ulGPIODirection &= ~(1 << g_i2cClockGPIO);
        pokeRegisterDWord(g_i2cClkGPIODataDirReg, ulGPIODirection);
    }

    /* Now read the SDA line */
    ulGPIOData = peekRegisterDWord(g_i2cClkGPIODataReg);
    if (ulGPIOData & (1 << g_i2cClockGPIO)) 
        return 1;
    else 
        return 0;
}
#pragma optimize( "", off )

/*******************************************************************************
    swI2CAck
        This function sends ACK signal

    Parameters:
        None

    Return Value:
        None
 *******************************************************************************/
static void swI2CAck(void)
{
    return;  /* Single byte read is ok without it. */
}

/*******************************************************************************
    swI2CStart
        This function sends the start command to the slave device

    Parameters:
        None

    Return Value:
        None
 *******************************************************************************/
void swI2CStart(void)
{
    /* Start I2C */
    swI2CSDA(1);
    swI2CSCL(1);
    swI2CSDA(0);
}

/*******************************************************************************
    swI2CStop
        This function sends the stop command to the slave device

    Parameters:
        None

    Return value:
        None
 *******************************************************************************/
void swI2CStop()
{
    /* Stop the I2C */
    swI2CSCL(1);
    swI2CSDA(0);
    swI2CSDA(1);
}


/*******************************************************************************
    swI2CWriteByte
        This function writes one byte to the slave device

    Parameters:
        data    - Data to be write to the slave device

    Return Value:
        0   - Fail to write byte
        1   - Success
 *******************************************************************************/
unsigned char swI2CWriteByte(unsigned char data) 
{
    unsigned char value = data;
    int i;

    /* Sending the data bit by bit */
    for (i=0; i<8; i++)
    {
        /* Set SCL to low */
        swI2CSCL(0);

        /* Send data bit */
        if ((value & 0x80) != 0)
            swI2CSDA(1);
        else
            swI2CSDA(0);

        swI2CWait();

        /* Toggle clk line to one */
        swI2CSCL(1);

        /* Shift byte to be sent */
        value = value << 1;
    }

    /* Set the SCL Low and SDA High (prepare to get input) */
    swI2CSCL(0);
    swI2CSDA(1);

    /* Set the SCL High for ack */
    swI2CWait();
    swI2CSCL(1);

    /* Read SDA, until SDA==0 */
    for(i=0; i<0xff; i++) 
    {
        swI2CWait();
        swI2CWait();
        if (!swI2CReadSDA())
            break;
    }

    /* Set the SCL Low and SDA High */
    swI2CSCL(0);
    swI2CSDA(1);

    return (i<0xff);
}

/*******************************************************************************
    swI2CWriteByte
        This function writes one byte to the slave device

    Parameters:
        ack	- Flag to indicate either to send the acknowledge
              message to the slave device or not

    Return Value:
        One byte data read from the Slave device
 *******************************************************************************/
unsigned char swI2CReadByte(unsigned char ack)
{
    int i;
    unsigned char data = 0;

    for(i=7; i>=0; i--)
    {
        /* Set the SCL to Low and SDA to High (Input) */
        swI2CSCL(0);
        swI2CSDA(1);
        swI2CWait();

        /* Set the SCL High */
        swI2CSCL(1);
        swI2CWait();

        /* Read data bits from SDA */
        data |= (swI2CReadSDA() << i);
    }

    if (ack)
        swI2CAck();

    /* Set the SCL Low and SDA High */
    swI2CSCL(0);
    swI2CSDA(1);

    return data;
}
#pragma optimize( "", on )


/*******************************************************************************
    ddk502_swI2CInit
        This function initializes the i2c attributes and bus

    Parameters:
        i2cClkGPIO  - The GPIO pin to be used as i2c SCL
        i2cDataGPIO - The GPIO pin to be used as i2c SDA

    Return Value:
        0   - Fail to initialize the i2c
        1   - Success
 *******************************************************************************/
_X_EXPORT unsigned char ddk502_swI2CInit(unsigned char i2cClkGPIO, unsigned char i2cDataGPIO)
{
    int i;
    unsigned long value;
    unsigned long gate;
    
    /* Return 0 if the GPIO pins to be used is out of range. The range is only from [0..63] */
    if ((i2cClkGPIO > 63) || (i2cDataGPIO > 63))
        return 0;
    
    /* Initialize the GPIO pin for the i2c Clock Register */
    if (i2cClkGPIO < 32)
    {
        g_i2cClkGPIOMuxReg = GPIO_MUX_LOW;   
        g_i2cClkGPIODataReg = GPIO_DATA_LOW;    
        g_i2cClkGPIODataDirReg = GPIO_DATA_DIRECTION_LOW;
        
        /* Initialize the Clock GPIO Offset */
        g_i2cClockGPIO = i2cClkGPIO;
         
    }
    else
    {
        g_i2cClkGPIOMuxReg = GPIO_MUX_HIGH;
        g_i2cClkGPIODataReg = GPIO_DATA_HIGH;
        g_i2cClkGPIODataDirReg = GPIO_DATA_DIRECTION_HIGH;
        
        /* Initialize the Clock GPIO Offset */
        g_i2cClockGPIO = i2cClkGPIO - 32;
    }
    
    /* Initialize the GPIO pin for the i2c Data Register */
    if (i2cDataGPIO < 32)
    {
        g_i2cDataGPIOMuxReg = GPIO_MUX_LOW;    
        g_i2cDataGPIODataReg = GPIO_DATA_LOW;    
        g_i2cDataGPIODataDirReg = GPIO_DATA_DIRECTION_LOW;
        
        /* Initialize the Data GPIO Offset */
        g_i2cDataGPIO = i2cDataGPIO;
    }
    else
    {
        g_i2cDataGPIOMuxReg = GPIO_MUX_HIGH;
        g_i2cDataGPIODataReg = GPIO_DATA_HIGH;
        g_i2cDataGPIODataDirReg = GPIO_DATA_DIRECTION_HIGH;
        
        /* Initialize the Data GPIO Offset */
        g_i2cDataGPIO = i2cDataGPIO - 32;
    }

    /* Enable the GPIO pins for the i2c Clock and Data (GPIO MUX) */
    pokeRegisterDWord(g_i2cClkGPIOMuxReg, 
                      peekRegisterDWord(g_i2cClkGPIOMuxReg) & ~(1 << g_i2cClockGPIO));
    pokeRegisterDWord(g_i2cDataGPIOMuxReg, 
                      peekRegisterDWord(g_i2cDataGPIOMuxReg) & ~(1 << g_i2cDataGPIO));

    /* Enable GPIO power */
    gate = peekRegisterDWord(CURRENT_POWER_GATE);
    gate = FIELD_SET(gate, CURRENT_POWER_GATE, GPIO_PWM_I2C, ENABLE);
    ddk502_setCurrentGate(gate);

    /* Clear the i2c lines. */
    for(i=0; i<9; i++) 
        swI2CStop();

    return 1;
}
/*******************************************************************************
    ddk502_swI2CReadReg
    This function reads the slave device's register

    Parameters:
    deviceAddress   - i2c Slave device address which register
                      to be read from
    registerIndex   - Slave device's register to be read

    Return Value:
        Register value
 *******************************************************************************/
_X_EXPORT unsigned char ddk502_swI2CReadReg(unsigned char deviceAddress, unsigned char registerIndex)
{
    unsigned char data;

    /* Send the Start signal */
    swI2CStart();

    /* Send the device address */
    swI2CWriteByte(deviceAddress);                                                  

    /* Send the register index */
    swI2CWriteByte(registerIndex);               

    /* Get the bus again and get the data from the device read address */
    swI2CStart();
    swI2CWriteByte(deviceAddress + 1);
    data = swI2CReadByte(1);

    /* Stop swI2C and release the bus */
    swI2CStop();

    return data;
}

/*******************************************************************************
    swI2CWriteReg
        This function write a value to the slave device's register

    Parameters:
        deviceAddress   - i2c Slave device address which register
                          to be written
        registerIndex   - Slave device's register to be written
        data            - Data to be written to the register

    Return Value:
        0   - Fail
        1   - Success
 *******************************************************************************/
unsigned char swI2CWriteReg(unsigned char deviceAddress, unsigned char registerIndex, unsigned char data)
{
    unsigned char bReturn = 0;

    /* Send the Start signal */
    swI2CStart();

    /* Send the device address and read the data. All should return success
       in order for the writing processed to be successful
     */
    if (swI2CWriteByte(deviceAddress) && 
        swI2CWriteByte(registerIndex) &&
        swI2CWriteByte(data))
    {
        bReturn = 1;
    }
    
    /* Stop i2c and release the bus */
    swI2CStop();

    return bReturn;
}
int __wait(void)
{
	int i=0,j=0;
	while(i++<1000){
		j+=i;
	}
	return j;
}

_X_EXPORT void ddk502_I2CPutBits_panel(I2CBusPtr bus,int clock,int data)
{	
	swI2CSCL(clock);
	swI2CSDA(data);	
	__wait();
}

_X_EXPORT void ddk502_I2CGetBits_panel(I2CBusPtr bus,int* clock,int* data)
{
	*data = swI2CReadSDA();	
	*clock = swI2CReadSCL();
	__wait();
}

_X_EXPORT void ddk502_I2CPutBits_crt(I2CBusPtr bus,int clock,int data)
{	
	swI2CSCL(clock);
	swI2CSDA(data);	
	__wait();
}

_X_EXPORT void ddk502_I2CGetBits_crt(I2CBusPtr bus,int* clock,int* data)
{		
	*data = swI2CReadSDA();	
	*clock = swI2CReadSCL();
	__wait();
}

