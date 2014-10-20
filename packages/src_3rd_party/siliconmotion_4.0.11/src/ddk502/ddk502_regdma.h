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

#define DMA_0_SDRAM                                     0x0D0000
#define DMA_0_SDRAM_EXT                                 27:27
#define DMA_0_SDRAM_EXT_LOCAL                           0
#define DMA_0_SDRAM_EXT_EXTERNAL                        1
#define DMA_0_SDRAM_CS                                  26:26
#define DMA_0_SDRAM_CS_0                                0
#define DMA_0_SDRAM_CS_1                                1
#define DMA_0_SDRAM_ADDRESS                             25:0

#define DMA_0_SRAM                                      0x0D0004
#define DMA_0_SRAM_ADDRESS                              15:0

#define DMA_0_SIZE_CONTROL                              0x0D0008
#define DMA_0_SIZE_CONTROL_STATUS                       31:31
#define DMA_0_SIZE_CONTROL_STATUS_IDLE                  0
#define DMA_0_SIZE_CONTROL_STATUS_ACTIVE                1
#define DMA_0_SIZE_CONTROL_DIR                          30:30
#define DMA_0_SIZE_CONTROL_DIR_TO_SRAM                  0
#define DMA_0_SIZE_CONTROL_DIR_FROM_SRAM                1
#define DMA_0_SIZE_CONTROL_SIZE                         15:0

#define DMA_1_SOURCE                                    0x0D0010
#define DMA_1_SOURCE_ADDRESS_EXT                        27:27
#define DMA_1_SOURCE_ADDRESS_EXT_LOCAL                  0
#define DMA_1_SOURCE_ADDRESS_EXT_EXTERNAL               1
#define DMA_1_SOURCE_ADDRESS_CS                         26:26
#define DMA_1_SOURCE_ADDRESS_CS_0                       0
#define DMA_1_SOURCE_ADDRESS_CS_1                       1
#define DMA_1_SOURCE_ADDRESS                            25:0

#define DMA_1_DESTINATION                               0x0D0014
#define DMA_1_DESTINATION_ADDRESS_EXT                   27:27
#define DMA_1_DESTINATION_ADDRESS_EXT_LOCAL             0
#define DMA_1_DESTINATION_ADDRESS_EXT_EXTERNAL          1
#define DMA_1_DESTINATION_ADDRESS_CS                    26:26
#define DMA_1_DESTINATION_ADDRESS_CS_0                  0
#define DMA_1_DESTINATION_ADDRESS_CS_1                  1
#define DMA_1_DESTINATION_ADDRESS                       25:0

#define DMA_1_SIZE_CONTROL                              0x0D0018
#define DMA_1_SIZE_CONTROL_STATUS                       31:31
#define DMA_1_SIZE_CONTROL_STATUS_IDLE                  0
#define DMA_1_SIZE_CONTROL_STATUS_ACTIVE                1
#define DMA_1_SIZE_CONTROL_SIZE                         23:0

#define DMA_ABORT_INTERRUPT                             0x0D0020
#define DMA_ABORT_INTERRUPT_ABORT_1                     5:5
#define DMA_ABORT_INTERRUPT_ABORT_1_ENABLE              0
#define DMA_ABORT_INTERRUPT_ABORT_1_ABORT               1
#define DMA_ABORT_INTERRUPT_ABORT_0                     4:4
#define DMA_ABORT_INTERRUPT_ABORT_0_ENABLE              0
#define DMA_ABORT_INTERRUPT_ABORT_0_ABORT               1
#define DMA_ABORT_INTERRUPT_INT_1                       1:1
#define DMA_ABORT_INTERRUPT_INT_1_CLEAR                 0
#define DMA_ABORT_INTERRUPT_INT_1_FINISHED              1
#define DMA_ABORT_INTERRUPT_INT_0                       0:0
#define DMA_ABORT_INTERRUPT_INT_0_CLEAR                 0
#define DMA_ABORT_INTERRUPT_INT_0_FINISHED              1
