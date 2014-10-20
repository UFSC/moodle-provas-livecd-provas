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
#include "ddk502_display.h"
#include "ddk502_mode.h"
#include "ddk502_power.h"

/*
 * Use panel vertical sync as time delay function.
 * Input: Number of vertical sync to wait.
 */
void panelWaitVerticalSync(unsigned long vsync_count)
{
    unsigned long status;

    /* Do not wait when the display control is already off. This will prevent
       the software to wait forever. */
    if (FIELD_GET(peekRegisterDWord(PANEL_DISPLAY_CTRL), PANEL_DISPLAY_CTRL, TIMING) ==
        PANEL_DISPLAY_CTRL_TIMING_DISABLE)
    {
        return;
    }
    
    while (vsync_count-- > 0)
    {
        /* Wait for end of vsync. */
        do
        {
            status = FIELD_GET(peekRegisterDWord(CMD_INTPR_STATUS),
                               CMD_INTPR_STATUS,
                               PANEL_SYNC);
        }
        while (status == CMD_INTPR_STATUS_PANEL_SYNC_ACTIVE);

        /* Wait for start of vsync. */
        do
        {
            status = FIELD_GET(peekRegisterDWord(CMD_INTPR_STATUS),
                               CMD_INTPR_STATUS, 
                               PANEL_SYNC);
        }
        while (status == CMD_INTPR_STATUS_PANEL_SYNC_INACTIVE);
    }
}

/*
 * Use crt vertical sync as time delay function.
 * Input: Number of vertical sync to wait.
 */
void crtWaitVerticalSync(unsigned long vsync_count)
{
    unsigned long status;
    
    /* Do not wait when the display control is already off. This will prevent
       the software to wait forever. */
    if (FIELD_GET(peekRegisterDWord(CRT_DISPLAY_CTRL), CRT_DISPLAY_CTRL, TIMING) ==
        CRT_DISPLAY_CTRL_TIMING_DISABLE)
    {
        return;
    }

    while (vsync_count-- > 0)
    {
        /* Wait for end of vsync. */
        do
        {
            status = FIELD_GET(peekRegisterDWord(CMD_INTPR_STATUS),
                               CMD_INTPR_STATUS,
                               CRT_SYNC);
        }
        while (status == CMD_INTPR_STATUS_CRT_SYNC_ACTIVE);

        /* Wait for start of vsync. */
        do
        {
            status = FIELD_GET(peekRegisterDWord(CMD_INTPR_STATUS),
                               CMD_INTPR_STATUS, 
                               CRT_SYNC);
        }
        while (status == CMD_INTPR_STATUS_CRT_SYNC_INACTIVE);
    }
}

/*
 * Use vertical sync as time delay function.
 * Input:
 *          dispControl - Display Control (either panel or crt) 
 *          vsync_count - Number of vertical sync to wait.
 *
 * Note:
 *      This function is waiting for the next vertical sync.
 */
void waitNextVerticalSync(disp_control_t dispControl, unsigned long vsync_count)
{
    if (dispControl == CRT_CTRL)
        crtWaitVerticalSync(vsync_count);
    else
        panelWaitVerticalSync(vsync_count);
}

/*
 * Use panel vertical sync line as time delay function.
 * This function does not wait for the next VSync. Instead, it will wait
 * until the current line reaches the Vertical Sync line.
 *
 * Input: display control (PANEL_CTRL or CRT_CTRL)
 */
void waitVSyncLine(disp_control_t dispControl)
{
    unsigned long value;
    mode_parameter_t modeParam;
    
    /* Get the current mode parameter of the specific display control */
    modeParam = getCurrentModeParam(dispControl);
    
    do
    {
        if (dispControl == CRT_CTRL)
            value = FIELD_GET(peekRegisterDWord(CRT_CURRENT_LINE), CRT_CURRENT_LINE, LINE) + 1;
        else            
            value = FIELD_GET(peekRegisterDWord(PANEL_CURRENT_LINE), PANEL_CURRENT_LINE, LINE) + 1;
    }
    while (value < modeParam.vertical_sync_start);
}

/*
 * This functions uses software sequence to turn on/off the panel.
 *
 */
_X_EXPORT void ddk502_swPanelPowerSequence(disp_state_t dispState, int vsync_delay)
{
    unsigned long panelControl = peekRegisterDWord(PANEL_DISPLAY_CTRL);

    if (dispState == DISP_ON)
    {
        /* Turn on FPVDDEN. */
        panelControl = FIELD_SET(panelControl,
                     PANEL_DISPLAY_CTRL, FPVDDEN, HIGH);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVerticalSync(vsync_delay);

        /* Turn on FPDATA. */
        panelControl = FIELD_SET(panelControl, 
                     PANEL_DISPLAY_CTRL, DATA, ENABLE);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVerticalSync(vsync_delay);

        /* Turn on FPVBIAS. */
        panelControl = FIELD_SET(panelControl, 
                     PANEL_DISPLAY_CTRL, VBIASEN, HIGH);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVerticalSync(vsync_delay);

        /* Turn on FPEN. */
        panelControl = FIELD_SET(panelControl, 
                     PANEL_DISPLAY_CTRL, FPEN, HIGH);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
    }

    else
    {
        /* Turn off FPEN. */
        panelControl = FIELD_SET(panelControl,
                                 PANEL_DISPLAY_CTRL, FPEN, LOW);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVerticalSync(vsync_delay);

        /* Turn off FPVBIASEN. */
        panelControl = FIELD_SET(panelControl, 
                                 PANEL_DISPLAY_CTRL, VBIASEN, LOW);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVerticalSync(vsync_delay);

        /* Turn off FPDATA. */
        panelControl = FIELD_SET(panelControl, 
                                 PANEL_DISPLAY_CTRL, DATA, DISABLE);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
        panelWaitVerticalSync(vsync_delay);

        /* Turn off FPVDDEN. */
        panelControl = FIELD_SET(panelControl, 
                                 PANEL_DISPLAY_CTRL, FPVDDEN, LOW);
        pokeRegisterDWord(PANEL_DISPLAY_CTRL, panelControl);
    }
}

/* 
 * This function turns on/off the DAC for CRT display control.
 * Input: On or off
 */
void setDAC(disp_state_t state)
{
    if (state == DISP_ON)
    {
        pokeRegisterDWord(MISC_CTRL, FIELD_SET(peekRegisterDWord(MISC_CTRL),
                                               MISC_CTRL,
                                               DAC_POWER,
                                               ENABLE));
    }
    else
    {
        pokeRegisterDWord(MISC_CTRL, FIELD_SET(peekRegisterDWord(MISC_CTRL),
                                               MISC_CTRL,
                                               DAC_POWER,
                                               DISABLE));
    }
}

/*
 * This function turns on/off the display control.
 * Currently, it for CRT and Panel controls only.
 * Input: Panel or CRT, or ...
 *        On or Off.
 *
 * This function manipulate the physical display channels 
 * and devices.
 *
 * Note:
 *      Turning on/off the timing and the plane requires programming sequence.
 *      The plane can not be changed without turning on the timing. However,
 *      changing the plane has no effect when the timing (clock) is off. Below,
 *      is the description of the timing and plane combination setting.
 *
 *      +-----------+-----------+-----------------------------------------------+
 *      |  Timing   |   Plane   |                    Description                |
 *      +-----------+-----------+-----------------------------------------------+
 *      |    ON     |    OFF    | no Display but clock is on (consume power)    |
 *      |    ON     |    ON     | normal display                                |
 *      |    OFF    |    OFF    | no display and no clock (power down)          |
 *      |    OFF    |    ON     | no display and no clock (same as power down)  |
 *      +-----------+-----------+-----------------------------------------------+
 */
void setDisplayControl(disp_control_t dispControl, disp_state_t dispState)
{
    unsigned long ulDisplayCtrlReg;

    if (dispControl == PANEL_CTRL)
    {
        ulDisplayCtrlReg = peekRegisterDWord(PANEL_DISPLAY_CTRL);

        /* Turn on/off the Panel display control */
        if (dispState == DISP_ON)
        {
            /* Timing should be enabled first before enabling the panel because changing at the
               same time does not guarantee that the plane will also enabled or disabled. 
             */
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, PANEL_DISPLAY_CTRL, TIMING, ENABLE);
            pokeRegisterDWord(PANEL_DISPLAY_CTRL, ulDisplayCtrlReg);
            
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, PANEL_DISPLAY_CTRL, PLANE, ENABLE);
        }
        else
        {
            /* When turning off, there is no rule on the programming sequence since whenever the
               clock is off, then it does not matter whether the plane is enabled or disabled 
             */
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, PANEL_DISPLAY_CTRL, PLANE, DISABLE);
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, PANEL_DISPLAY_CTRL, TIMING, DISABLE);
        }

        pokeRegisterDWord(PANEL_DISPLAY_CTRL, ulDisplayCtrlReg);
    }
    else /* CRT */
    {
        ulDisplayCtrlReg = peekRegisterDWord(CRT_DISPLAY_CTRL);

        if (dispState == DISP_ON)
        {
            /* Timing should be enabled first before enabling the panel because changing at the
               same time does not guarantee that the plane will also enabled or disabled. 
             */
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, CRT_DISPLAY_CTRL, TIMING, ENABLE);
            pokeRegisterDWord(CRT_DISPLAY_CTRL, ulDisplayCtrlReg);
            
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, CRT_DISPLAY_CTRL, PLANE, ENABLE);
        }
        else
        {
            /* When turning off, there is no rule on the programming sequence since whenever the
               clock is off, then it does not matter whether the plane is enabled or disabled 
             */
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, CRT_DISPLAY_CTRL, PLANE, DISABLE);
            ulDisplayCtrlReg = FIELD_SET(ulDisplayCtrlReg, CRT_DISPLAY_CTRL, TIMING, DISABLE);
        }
        pokeRegisterDWord(CRT_DISPLAY_CTRL, ulDisplayCtrlReg);
    }
}

/* This function set the display path together with the HSync and VSync. */
void setCRTPath(disp_control_t dispControl)
{
    unsigned long crtControl;
    mode_parameter_t modeParam;
    
    /* Get the current mode parameter of the specific display control */
    modeParam = getCurrentModeParam(dispControl);
    
    crtControl = peekRegisterDWord(CRT_DISPLAY_CTRL);
    
    /* Adjust the VSync polarity */
    if (modeParam.vertical_sync_polarity == POS)
        crtControl = FIELD_SET(crtControl, CRT_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_HIGH);
    else
        crtControl = FIELD_SET(crtControl, CRT_DISPLAY_CTRL, VSYNC_PHASE, ACTIVE_LOW);
    
    /* Adjust the HSync polarity */
    if (modeParam.horizontal_sync_polarity == POS)
        crtControl = FIELD_SET(crtControl, CRT_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_HIGH);
    else
        crtControl = FIELD_SET(crtControl, CRT_DISPLAY_CTRL, HSYNC_PHASE, ACTIVE_LOW);
        
    /* Adjust the CRT Data path either to use Panel control or CRT control. */    
    if (dispControl == PANEL_CTRL)
        crtControl = FIELD_SET(crtControl, CRT_DISPLAY_CTRL, SELECT, PANEL);
    else
        crtControl = FIELD_SET(crtControl, CRT_DISPLAY_CTRL, SELECT, CRT);
    
    pokeRegisterDWord(CRT_DISPLAY_CTRL, crtControl);
}

/*
 * This function is an example showing how to set up
 * PANEL only, CRT only , SIMUL or DUAL with the following functions:
 *      setDisplayControl()
 *      swPanelPowerSequence()
 *      setDAC()
 *      setDPMS()
 * 
 * The output is called logical because it is independent of physical implementation.
 * For example, CRT only mode is not using the internal CRT control. It uses the
 * Panel Control with its output directed to CRT DAC. 
 */
_X_EXPORT void setLogicalDispOutput(disp_output_t output)
{
    unsigned long ulReg;

    switch (output)
    {
        case NO_DISPLAY:
        
            /* In here, all the display device has to be turned off first before the
               the display control. */
            ddk502_swPanelPowerSequence(DISP_OFF, 4);          /* Turn off Panel */

            setDAC(DISP_OFF);                           /* Turn off DAC */
            ddk502_setDPMS(DPMS_OFF);                          /* Turn off DPMS */
            
            setDisplayControl(PANEL_CTRL, DISP_OFF);    /* Turn off Panel Control */
            setDisplayControl(CRT_CTRL, DISP_OFF);      /* Turn off CRT control */

            break;
            
        case PANEL_ONLY:
            setDisplayControl(PANEL_CTRL, DISP_ON);     /* Turn on Panel Control */
            ddk502_swPanelPowerSequence(DISP_ON, 4);           /* Turn on Panel */

            setDisplayControl(CRT_CTRL, DISP_OFF);      /* Turn off CRT control */
            setDAC(DISP_OFF);                           /* Turn off DAC */
            ddk502_setDPMS(DPMS_OFF);                          /* Turn off DPMS */
            
            break;

        case CRT_ONLY:
            setDisplayControl(PANEL_CTRL, DISP_ON);     /* Turn on Panel Control */
            ddk502_swPanelPowerSequence(DISP_OFF, 4);          /* Turn off Panel */

            setDisplayControl(CRT_CTRL, DISP_OFF);      /* Turn off CRT control */
            setDAC(DISP_ON);                            /* Turn on DAC */
            ddk502_setDPMS(DPMS_ON);                           /* Turn on DPMS to drive CRT */
            
            /* Set up CRT control to use data from panel control */
            setCRTPath(PANEL_CTRL);

            break;

        case PANEL_CRT_SIMUL: /* Panel and CRT same content */
            setDisplayControl(PANEL_CTRL, DISP_ON);     /* Turn on Panel Control */
            ddk502_swPanelPowerSequence(DISP_ON, 4);           /* Turn on Panel */

            setDisplayControl(CRT_CTRL, DISP_OFF);      /* Turn off CRT control */
            setDAC(DISP_ON);                            /* Turn on DAC */
            ddk502_setDPMS(DPMS_ON);                           /* Turn on DPMS to drive CRT and TV */

            /* Set up CRT control to use data from panel control */
            setCRTPath(PANEL_CTRL);
            
            break;

        case PANEL_CRT_DUAL: /* Panel and CRT different content */
            setDisplayControl(PANEL_CTRL, DISP_ON);     /* Turn on panel control */
            ddk502_swPanelPowerSequence(DISP_ON, 4);           /* Turn on Panel */

            /* If previous state is SIMUL or CRT only, just be sure the
               data direction bit of CRT channel is correct
            */
            setCRTPath(CRT_CTRL);

            setDisplayControl(CRT_CTRL, DISP_ON);       /* Turn on CRT control */
            setDAC(DISP_ON);                            /* Turn on DAC */
            ddk502_setDPMS(DPMS_ON);                           /* Turn on DPMS to drive CRT */
            break;            
     }
}
