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

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "xf86Module.h"

typedef enum _disp_control_t
{
    PANEL_CTRL = 0,
    CRT_CTRL   = 1,
    /* Think of defining other display control here: Video, alpha, etc. */
}
disp_control_t;

typedef enum _disp_state_t
{
    DISP_OFF = 0,
    DISP_ON  = 1,
}
disp_state_t;

typedef enum _disp_output_t
{
    NO_DISPLAY,             /* All display off. */
    PANEL_ONLY,
    CRT_ONLY,
    PANEL_CRT_SIMUL,        /* Both Panel and CRT displaying the same content. */
    PANEL_CRT_DUAL,         /* Panel and CRT displaying different contents. */
}
disp_output_t;

/*
 * This functions sets the CRT Path.
 */
void setCRTPath(disp_control_t dispControl);

/*
 * This functions uses software sequence to turn on/off the panel.
 */
void ddk502_swPanelPowerSequence(disp_state_t dispState, int vsync_delay);

/* 
 * This function turns on/off the DAC for CRT display control.
 * Input: On or off
 */
void setDAC(disp_state_t state);

/*
 * This function turns on/off the display control.
 * Currently, it for CRT and Panel controls only.
 * Input: Panel or CRT, or ...
 *        On or Off.
 */
void setDisplayControl(disp_control_t dispControl, disp_state_t dispState);

/*
 * This function is an example showing how to set up
 * PANEL only, CRT only , SIMUL or DUAL with the following functions:
 *      setDisplayControl()
 *      swPanelPowerSequence()
 *      setDAC()
 *      setDPMS()
 */
_X_EXPORT void setLogicalDispOutput(disp_output_t output);

/*
 * Use vertical sync as time delay function.
 * Input:
 *          dispControl - Display Control (PANEL_CTRL or CRT_CTRL) 
 *          vsync_count - Number of vertical sync to wait.
 *
 * Note:
 *      This function is waiting for the next vertical sync.
 */
void waitNextVerticalSync(disp_control_t dispControl, unsigned long vsync_count);

/*
 * Use panel vertical sync line as time delay function.
 * This function does not wait for the next VSync. Instead, it will wait
 * until the current line reaches the Vertical Sync line.
 *
 * Input: display control (PANEL_CTRL or CRT_CTRL)
 */
void waitVSyncLine(disp_control_t dispControl);

#endif /* _DISPLAY_H_ */
