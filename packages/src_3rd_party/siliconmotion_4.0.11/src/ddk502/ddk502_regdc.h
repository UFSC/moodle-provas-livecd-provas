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

/* Panel Graphics Control */

#define PANEL_DISPLAY_CTRL                              0x080000
#define PANEL_DISPLAY_CTRL_RESERVED_1_MASK              31:28
#define PANEL_DISPLAY_CTRL_RESERVED_1_MASK_DISABLE      0
#define PANEL_DISPLAY_CTRL_RESERVED_1_MASK_ENABLE       0xF
#define PANEL_DISPLAY_CTRL_FPEN                         27:27
#define PANEL_DISPLAY_CTRL_FPEN_LOW                     0
#define PANEL_DISPLAY_CTRL_FPEN_HIGH                    1
#define PANEL_DISPLAY_CTRL_VBIASEN                      26:26
#define PANEL_DISPLAY_CTRL_VBIASEN_LOW                  0
#define PANEL_DISPLAY_CTRL_VBIASEN_HIGH                 1
#define PANEL_DISPLAY_CTRL_DATA                         25:25
#define PANEL_DISPLAY_CTRL_DATA_DISABLE                 0
#define PANEL_DISPLAY_CTRL_DATA_ENABLE                  1
#define PANEL_DISPLAY_CTRL_FPVDDEN                      24:24
#define PANEL_DISPLAY_CTRL_FPVDDEN_LOW                  0
#define PANEL_DISPLAY_CTRL_FPVDDEN_HIGH                 1
#define PANEL_DISPLAY_CTRL_PATTERN                      23:23
#define PANEL_DISPLAY_CTRL_PATTERN_4                    0
#define PANEL_DISPLAY_CTRL_PATTERN_8                    1
#define PANEL_DISPLAY_CTRL_TFT                          22:21
#define PANEL_DISPLAY_CTRL_TFT_24                       0
#define PANEL_DISPLAY_CTRL_TFT_9                        1
#define PANEL_DISPLAY_CTRL_TFT_12                       2
#define PANEL_DISPLAY_CTRL_DITHER                       20:20
#define PANEL_DISPLAY_CTRL_DITHER_DISABLE               0
#define PANEL_DISPLAY_CTRL_DITHER_ENABLE                1
#define PANEL_DISPLAY_CTRL_LCD                          19:18
#define PANEL_DISPLAY_CTRL_LCD_TFT                      0
#define PANEL_DISPLAY_CTRL_LCD_STN_8                    2
#define PANEL_DISPLAY_CTRL_LCD_STN_12                   3
#define PANEL_DISPLAY_CTRL_FIFO                         17:16
#define PANEL_DISPLAY_CTRL_FIFO_1                       0
#define PANEL_DISPLAY_CTRL_FIFO_3                       1
#define PANEL_DISPLAY_CTRL_FIFO_7                       2
#define PANEL_DISPLAY_CTRL_FIFO_11                      3
#define PANEL_DISPLAY_CTRL_8BIT_TV                      15:15
#define PANEL_DISPLAY_CTRL_8BIT_TV_DISABLE              0
#define PANEL_DISPLAY_CTRL_8BIT_TV_ENABLE               1
#define PANEL_DISPLAY_CTRL_CLOCK_PHASE                  14:14
#define PANEL_DISPLAY_CTRL_CLOCK_PHASE_ACTIVE_HIGH      0
#define PANEL_DISPLAY_CTRL_CLOCK_PHASE_ACTIVE_LOW       1
#define PANEL_DISPLAY_CTRL_VSYNC_PHASE                  13:13
#define PANEL_DISPLAY_CTRL_VSYNC_PHASE_ACTIVE_HIGH      0
#define PANEL_DISPLAY_CTRL_VSYNC_PHASE_ACTIVE_LOW       1
#define PANEL_DISPLAY_CTRL_HSYNC_PHASE                  12:12
#define PANEL_DISPLAY_CTRL_HSYNC_PHASE_ACTIVE_HIGH      0
#define PANEL_DISPLAY_CTRL_HSYNC_PHASE_ACTIVE_LOW       1
#define PANEL_DISPLAY_CTRL_RESERVED_2_MASK              11:11
#define PANEL_DISPLAY_CTRL_RESERVED_2_MASK_DISABLE      0
#define PANEL_DISPLAY_CTRL_RESERVED_2_MASK_ENABLE       1
#define PANEL_DISPLAY_CTRL_CAPTURE_TIMING               10:10
#define PANEL_DISPLAY_CTRL_CAPTURE_TIMING_DISABLE       0
#define PANEL_DISPLAY_CTRL_CAPTURE_TIMING_ENABLE        1
#define PANEL_DISPLAY_CTRL_COLOR_KEY                    9:9
#define PANEL_DISPLAY_CTRL_COLOR_KEY_DISABLE            0
#define PANEL_DISPLAY_CTRL_COLOR_KEY_ENABLE             1
#define PANEL_DISPLAY_CTRL_TIMING                       8:8
#define PANEL_DISPLAY_CTRL_TIMING_DISABLE               0
#define PANEL_DISPLAY_CTRL_TIMING_ENABLE                1
#define PANEL_DISPLAY_CTRL_VERTICAL_PAN_DIR             7:7
#define PANEL_DISPLAY_CTRL_VERTICAL_PAN_DIR_DOWN        0
#define PANEL_DISPLAY_CTRL_VERTICAL_PAN_DIR_UP          1
#define PANEL_DISPLAY_CTRL_VERTICAL_PAN                 6:6
#define PANEL_DISPLAY_CTRL_VERTICAL_PAN_DISABLE         0
#define PANEL_DISPLAY_CTRL_VERTICAL_PAN_ENABLE          1
#define PANEL_DISPLAY_CTRL_HORIZONTAL_PAN_DIR           5:5
#define PANEL_DISPLAY_CTRL_HORIZONTAL_PAN_DIR_RIGHT     0
#define PANEL_DISPLAY_CTRL_HORIZONTAL_PAN_DIR_LEFT      1
#define PANEL_DISPLAY_CTRL_HORIZONTAL_PAN               4:4
#define PANEL_DISPLAY_CTRL_HORIZONTAL_PAN_DISABLE       0
#define PANEL_DISPLAY_CTRL_HORIZONTAL_PAN_ENABLE        1
#define PANEL_DISPLAY_CTRL_GAMMA                        3:3
#define PANEL_DISPLAY_CTRL_GAMMA_DISABLE                0
#define PANEL_DISPLAY_CTRL_GAMMA_ENABLE                 1
#define PANEL_DISPLAY_CTRL_PLANE                        2:2
#define PANEL_DISPLAY_CTRL_PLANE_DISABLE                0
#define PANEL_DISPLAY_CTRL_PLANE_ENABLE                 1
#define PANEL_DISPLAY_CTRL_FORMAT                       1:0
#define PANEL_DISPLAY_CTRL_FORMAT_8                     0
#define PANEL_DISPLAY_CTRL_FORMAT_16                    1
#define PANEL_DISPLAY_CTRL_FORMAT_32                    2

#define PANEL_PAN_CTRL                                  0x080004
#define PANEL_PAN_CTRL_VERTICAL_PAN                     31:24
#define PANEL_PAN_CTRL_VERTICAL_VSYNC                   21:16
#define PANEL_PAN_CTRL_HORIZONTAL_PAN                   15:8
#define PANEL_PAN_CTRL_HORIZONTAL_VSYNC                 5:0

#define PANEL_COLOR_KEY                                 0x080008
#define PANEL_COLOR_KEY_MASK                            31:16
#define PANEL_COLOR_KEY_VALUE                           15:0

#define PANEL_FB_ADDRESS                                0x08000C
#define PANEL_FB_ADDRESS_STATUS                         31:31
#define PANEL_FB_ADDRESS_STATUS_CURRENT                 0
#define PANEL_FB_ADDRESS_STATUS_PENDING                 1
#define PANEL_FB_ADDRESS_EXT                            27:27
#define PANEL_FB_ADDRESS_EXT_LOCAL                      0
#define PANEL_FB_ADDRESS_EXT_EXTERNAL                   1
#define PANEL_FB_ADDRESS_CS                             26:26
#define PANEL_FB_ADDRESS_CS_0                           0
#define PANEL_FB_ADDRESS_CS_1                           1
#define PANEL_FB_ADDRESS_ADDRESS                        25:0

#define PANEL_FB_WIDTH                                  0x080010
#define PANEL_FB_WIDTH_WIDTH                            29:16
#define PANEL_FB_WIDTH_OFFSET                           13:0

#define PANEL_WINDOW_WIDTH                              0x080014
#define PANEL_WINDOW_WIDTH_WIDTH                        27:16
#define PANEL_WINDOW_WIDTH_X                            11:0

#define PANEL_WINDOW_HEIGHT                             0x080018
#define PANEL_WINDOW_HEIGHT_HEIGHT                      27:16
#define PANEL_WINDOW_HEIGHT_Y                           11:0

#define PANEL_PLANE_TL                                  0x08001C
#define PANEL_PLANE_TL_TOP                              26:16
#define PANEL_PLANE_TL_LEFT                             10:0

#define PANEL_PLANE_BR                                  0x080020
#define PANEL_PLANE_BR_BOTTOM                           26:16
#define PANEL_PLANE_BR_RIGHT                            10:0

#define PANEL_HORIZONTAL_TOTAL                          0x080024
#define PANEL_HORIZONTAL_TOTAL_TOTAL                    27:16
#define PANEL_HORIZONTAL_TOTAL_DISPLAY_END              11:0

#define PANEL_HORIZONTAL_SYNC                           0x080028
#define PANEL_HORIZONTAL_SYNC_WIDTH                     23:16
#define PANEL_HORIZONTAL_SYNC_START                     11:0

#define PANEL_VERTICAL_TOTAL                            0x08002C
#define PANEL_VERTICAL_TOTAL_TOTAL                      26:16
#define PANEL_VERTICAL_TOTAL_DISPLAY_END                10:0

#define PANEL_VERTICAL_SYNC                             0x080030
#define PANEL_VERTICAL_SYNC_HEIGHT                      21:16
#define PANEL_VERTICAL_SYNC_START                       10:0

#define PANEL_CURRENT_LINE                              0x080034
#define PANEL_CURRENT_LINE_LINE                         10:0

/* Video Control */

#define VIDEO_DISPLAY_CTRL                              0x080040
#define VIDEO_DISPLAY_CTRL_FIFO                         17:16
#define VIDEO_DISPLAY_CTRL_FIFO_1                       0
#define VIDEO_DISPLAY_CTRL_FIFO_3                       1
#define VIDEO_DISPLAY_CTRL_FIFO_7                       2
#define VIDEO_DISPLAY_CTRL_FIFO_11                      3
#define VIDEO_DISPLAY_CTRL_BUFFER                       15:15
#define VIDEO_DISPLAY_CTRL_BUFFER_0                     0
#define VIDEO_DISPLAY_CTRL_BUFFER_1                     1
#define VIDEO_DISPLAY_CTRL_CAPTURE                      14:14
#define VIDEO_DISPLAY_CTRL_CAPTURE_DISABLE              0
#define VIDEO_DISPLAY_CTRL_CAPTURE_ENABLE               1
#define VIDEO_DISPLAY_CTRL_DOUBLE_BUFFER                13:13
#define VIDEO_DISPLAY_CTRL_DOUBLE_BUFFER_DISABLE        0
#define VIDEO_DISPLAY_CTRL_DOUBLE_BUFFER_ENABLE         1
#define VIDEO_DISPLAY_CTRL_BYTE_SWAP                    12:12
#define VIDEO_DISPLAY_CTRL_BYTE_SWAP_DISABLE            0
#define VIDEO_DISPLAY_CTRL_BYTE_SWAP_ENABLE             1
#define VIDEO_DISPLAY_CTRL_VERTICAL_SCALE               11:11
#define VIDEO_DISPLAY_CTRL_VERTICAL_SCALE_NORMAL        0
#define VIDEO_DISPLAY_CTRL_VERTICAL_SCALE_HALF          1
#define VIDEO_DISPLAY_CTRL_HORIZONTAL_SCALE             10:10
#define VIDEO_DISPLAY_CTRL_HORIZONTAL_SCALE_NORMAL      0
#define VIDEO_DISPLAY_CTRL_HORIZONTAL_SCALE_HALF        1
#define VIDEO_DISPLAY_CTRL_VERTICAL_MODE                9:9
#define VIDEO_DISPLAY_CTRL_VERTICAL_MODE_REPLICATE      0
#define VIDEO_DISPLAY_CTRL_VERTICAL_MODE_INTERPOLATE    1
#define VIDEO_DISPLAY_CTRL_HORIZONTAL_MODE              8:8
#define VIDEO_DISPLAY_CTRL_HORIZONTAL_MODE_REPLICATE    0
#define VIDEO_DISPLAY_CTRL_HORIZONTAL_MODE_INTERPOLATE  1
#define VIDEO_DISPLAY_CTRL_PIXEL                        7:4
#define VIDEO_DISPLAY_CTRL_GAMMA                        3:3
#define VIDEO_DISPLAY_CTRL_GAMMA_DISABLE                0
#define VIDEO_DISPLAY_CTRL_GAMMA_ENABLE                 1
#define VIDEO_DISPLAY_CTRL_PLANE                        2:2
#define VIDEO_DISPLAY_CTRL_PLANE_DISABLE                0
#define VIDEO_DISPLAY_CTRL_PLANE_ENABLE                 1
#define VIDEO_DISPLAY_CTRL_FORMAT                       1:0
#define VIDEO_DISPLAY_CTRL_FORMAT_8                     0
#define VIDEO_DISPLAY_CTRL_FORMAT_16                    1
#define VIDEO_DISPLAY_CTRL_FORMAT_32                    2
#define VIDEO_DISPLAY_CTRL_FORMAT_YUV                   3

#define VIDEO_FB_0_ADDRESS                              0x080044
#define VIDEO_FB_0_ADDRESS_STATUS                       31:31
#define VIDEO_FB_0_ADDRESS_STATUS_CURRENT               0
#define VIDEO_FB_0_ADDRESS_STATUS_PENDING               1
#define VIDEO_FB_0_ADDRESS_EXT                          27:27
#define VIDEO_FB_0_ADDRESS_EXT_LOCAL                    0
#define VIDEO_FB_0_ADDRESS_EXT_EXTERNAL                 1
#define VIDEO_FB_0_ADDRESS_CS                           26:26
#define VIDEO_FB_0_ADDRESS_CS_0                         0
#define VIDEO_FB_0_ADDRESS_CS_1                         1
#define VIDEO_FB_0_ADDRESS_ADDRESS                      25:0

#define VIDEO_FB_WIDTH                                  0x080048
#define VIDEO_FB_WIDTH_WIDTH                            29:16
#define VIDEO_FB_WIDTH_OFFSET                           13:0

#define VIDEO_FB_0_LAST_ADDRESS                         0x08004C
#define VIDEO_FB_0_LAST_ADDRESS_EXT                     27:27
#define VIDEO_FB_0_LAST_ADDRESS_EXT_LOCAL               0
#define VIDEO_FB_0_LAST_ADDRESS_EXT_EXTERNAL            1
#define VIDEO_FB_0_LAST_ADDRESS_CS                      26:26
#define VIDEO_FB_0_LAST_ADDRESS_CS_0                    0
#define VIDEO_FB_0_LAST_ADDRESS_CS_1                    1
#define VIDEO_FB_0_LAST_ADDRESS_ADDRESS                 25:0

#define VIDEO_PLANE_TL                                  0x080050
#define VIDEO_PLANE_TL_TOP                              26:16
#define VIDEO_PLANE_TL_LEFT                             13:0

#define VIDEO_PLANE_BR                                  0x080054
#define VIDEO_PLANE_BR_BOTTOM                           26:16
#define VIDEO_PLANE_BR_RIGHT                            13:0

#define VIDEO_SCALE                                     0x080058
#define VIDEO_SCALE_VERTICAL_MODE                       31:31
#define VIDEO_SCALE_VERTICAL_MODE_EXPAND                0
#define VIDEO_SCALE_VERTICAL_MODE_SHRINK                1
#define VIDEO_SCALE_VERTICAL_SCALE                      27:16
#define VIDEO_SCALE_HORIZONTAL_MODE                     15:15
#define VIDEO_SCALE_HORIZONTAL_MODE_EXPAND              0
#define VIDEO_SCALE_HORIZONTAL_MODE_SHRINK              1
#define VIDEO_SCALE_HORIZONTAL_SCALE                    11:0

#define VIDEO_INITIAL_SCALE                             0x08005C
#define VIDEO_INITIAL_SCALE_FB_1                        27:16
#define VIDEO_INITIAL_SCALE_FB_0                        11:0

#define VIDEO_YUV_CONSTANTS                             0x080060
#define VIDEO_YUV_CONSTANTS_Y                           31:24
#define VIDEO_YUV_CONSTANTS_R                           23:16
#define VIDEO_YUV_CONSTANTS_G                           15:8
#define VIDEO_YUV_CONSTANTS_B                           7:0

#define VIDEO_FB_1_ADDRESS                              0x080064
#define VIDEO_FB_1_ADDRESS_STATUS                       31:31
#define VIDEO_FB_1_ADDRESS_STATUS_CURRENT               0
#define VIDEO_FB_1_ADDRESS_STATUS_PENDING               1
#define VIDEO_FB_1_ADDRESS_EXT                          27:27
#define VIDEO_FB_1_ADDRESS_EXT_LOCAL                    0
#define VIDEO_FB_1_ADDRESS_EXT_EXTERNAL                 1
#define VIDEO_FB_1_ADDRESS_CS                           26:26
#define VIDEO_FB_1_ADDRESS_CS_0                         0
#define VIDEO_FB_1_ADDRESS_CS_1                         1
#define VIDEO_FB_1_ADDRESS_ADDRESS                      25:0

#define VIDEO_FB_1_LAST_ADDRESS                         0x080068
#define VIDEO_FB_1_LAST_ADDRESS_EXT                     27:27
#define VIDEO_FB_1_LAST_ADDRESS_EXT_LOCAL               0
#define VIDEO_FB_1_LAST_ADDRESS_EXT_EXTERNAL            1
#define VIDEO_FB_1_LAST_ADDRESS_CS                      26:26
#define VIDEO_FB_1_LAST_ADDRESS_CS_0                    0
#define VIDEO_FB_1_LAST_ADDRESS_CS_1                    1
#define VIDEO_FB_1_LAST_ADDRESS_ADDRESS                 25:0

/* Video Alpha Control */

#define VIDEO_ALPHA_DISPLAY_CTRL                        0x080080
#define VIDEO_ALPHA_DISPLAY_CTRL_SELECT                 28:28
#define VIDEO_ALPHA_DISPLAY_CTRL_SELECT_PER_PIXEL       0
#define VIDEO_ALPHA_DISPLAY_CTRL_SELECT_ALPHA           1
#define VIDEO_ALPHA_DISPLAY_CTRL_ALPHA                  27:24
#define VIDEO_ALPHA_DISPLAY_CTRL_FIFO                   17:16
#define VIDEO_ALPHA_DISPLAY_CTRL_FIFO_1                 0
#define VIDEO_ALPHA_DISPLAY_CTRL_FIFO_3                 1
#define VIDEO_ALPHA_DISPLAY_CTRL_FIFO_7                 2
#define VIDEO_ALPHA_DISPLAY_CTRL_FIFO_11                3
#define VIDEO_ALPHA_DISPLAY_CTRL_VERT_SCALE             11:11
#define VIDEO_ALPHA_DISPLAY_CTRL_VERT_SCALE_NORMAL      0
#define VIDEO_ALPHA_DISPLAY_CTRL_VERT_SCALE_HALF        1
#define VIDEO_ALPHA_DISPLAY_CTRL_HORZ_SCALE             10:10
#define VIDEO_ALPHA_DISPLAY_CTRL_HORZ_SCALE_NORMAL      0
#define VIDEO_ALPHA_DISPLAY_CTRL_HORZ_SCALE_HALF        1
#define VIDEO_ALPHA_DISPLAY_CTRL_VERT_MODE              9:9
#define VIDEO_ALPHA_DISPLAY_CTRL_VERT_MODE_REPLICATE    0
#define VIDEO_ALPHA_DISPLAY_CTRL_VERT_MODE_INTERPOLATE  1
#define VIDEO_ALPHA_DISPLAY_CTRL_HORZ_MODE              8:8
#define VIDEO_ALPHA_DISPLAY_CTRL_HORZ_MODE_REPLICATE    0
#define VIDEO_ALPHA_DISPLAY_CTRL_HORZ_MODE_INTERPOLATE  1
#define VIDEO_ALPHA_DISPLAY_CTRL_PIXEL                  7:4
#define VIDEO_ALPHA_DISPLAY_CTRL_CHROMA_KEY             3:3
#define VIDEO_ALPHA_DISPLAY_CTRL_CHROMA_KEY_DISABLE     0
#define VIDEO_ALPHA_DISPLAY_CTRL_CHROMA_KEY_ENABLE      1
#define VIDEO_ALPHA_DISPLAY_CTRL_PLANE                  2:2
#define VIDEO_ALPHA_DISPLAY_CTRL_PLANE_DISABLE          0
#define VIDEO_ALPHA_DISPLAY_CTRL_PLANE_ENABLE           1
#define VIDEO_ALPHA_DISPLAY_CTRL_FORMAT                 1:0
#define VIDEO_ALPHA_DISPLAY_CTRL_FORMAT_8               0
#define VIDEO_ALPHA_DISPLAY_CTRL_FORMAT_16              1
#define VIDEO_ALPHA_DISPLAY_CTRL_FORMAT_ALPHA_4_4       2
#define VIDEO_ALPHA_DISPLAY_CTRL_FORMAT_ALPHA_4_4_4_4   3

#define VIDEO_ALPHA_FB_ADDRESS                          0x080084
#define VIDEO_ALPHA_FB_ADDRESS_STATUS                   31:31
#define VIDEO_ALPHA_FB_ADDRESS_STATUS_CURRENT           0
#define VIDEO_ALPHA_FB_ADDRESS_STATUS_PENDING           1
#define VIDEO_ALPHA_FB_ADDRESS_EXT                      27:27
#define VIDEO_ALPHA_FB_ADDRESS_EXT_LOCAL                0
#define VIDEO_ALPHA_FB_ADDRESS_EXT_EXTERNAL             1
#define VIDEO_ALPHA_FB_ADDRESS_CS                       26:26
#define VIDEO_ALPHA_FB_ADDRESS_CS_0                     0
#define VIDEO_ALPHA_FB_ADDRESS_CS_1                     1
#define VIDEO_ALPHA_FB_ADDRESS_ADDRESS                  25:0

#define VIDEO_ALPHA_FB_WIDTH                            0x080088
#define VIDEO_ALPHA_FB_WIDTH_WIDTH                      29:16
#define VIDEO_ALPHA_FB_WIDTH_OFFSET                     13:0

#define VIDEO_ALPHA_FB_LAST_ADDRESS                     0x08008C
#define VIDEO_ALPHA_FB_LAST_ADDRESS_EXT                 27:27
#define VIDEO_ALPHA_FB_LAST_ADDRESS_EXT_LOCAL           0
#define VIDEO_ALPHA_FB_LAST_ADDRESS_EXT_EXTERNAL        1
#define VIDEO_ALPHA_FB_LAST_ADDRESS_CS                  26:26
#define VIDEO_ALPHA_FB_LAST_ADDRESS_CS_0                0
#define VIDEO_ALPHA_FB_LAST_ADDRESS_CS_1                1
#define VIDEO_ALPHA_FB_LAST_ADDRESS_ADDRESS             25:0

#define VIDEO_ALPHA_PLANE_TL                            0x080090
#define VIDEO_ALPHA_PLANE_TL_TOP                        26:16
#define VIDEO_ALPHA_PLANE_TL_LEFT                       10:0

#define VIDEO_ALPHA_PLANE_BR                            0x080094
#define VIDEO_ALPHA_PLANE_BR_BOTTOM                     26:16
#define VIDEO_ALPHA_PLANE_BR_RIGHT                      10:0

#define VIDEO_ALPHA_SCALE                               0x080098
#define VIDEO_ALPHA_SCALE_VERTICAL_MODE                 31:31
#define VIDEO_ALPHA_SCALE_VERTICAL_MODE_EXPAND          0
#define VIDEO_ALPHA_SCALE_VERTICAL_MODE_SHRINK          1
#define VIDEO_ALPHA_SCALE_VERTICAL_SCALE                27:16
#define VIDEO_ALPHA_SCALE_HORIZONTAL_MODE               15:15
#define VIDEO_ALPHA_SCALE_HORIZONTAL_MODE_EXPAND        0
#define VIDEO_ALPHA_SCALE_HORIZONTAL_MODE_SHRINK        1
#define VIDEO_ALPHA_SCALE_HORIZONTAL_SCALE              11:0

#define VIDEO_ALPHA_INITIAL_SCALE                       0x08009C
#define VIDEO_ALPHA_INITIAL_SCALE_FB                    11:0

#define VIDEO_ALPHA_CHROMA_KEY                          0x0800A0
#define VIDEO_ALPHA_CHROMA_KEY_MASK                     31:16
#define VIDEO_ALPHA_CHROMA_KEY_VALUE                    15:0

#define VIDEO_ALPHA_COLOR_LOOKUP_01                     0x0800A4
#define VIDEO_ALPHA_COLOR_LOOKUP_01_1                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_01_1_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_01_1_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_01_1_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_01_0                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_01_0_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_01_0_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_01_0_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_23                     0x0800A8
#define VIDEO_ALPHA_COLOR_LOOKUP_23_3                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_23_3_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_23_3_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_23_3_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_23_2                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_23_2_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_23_2_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_23_2_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_45                     0x0800AC
#define VIDEO_ALPHA_COLOR_LOOKUP_45_5                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_45_5_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_45_5_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_45_5_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_45_4                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_45_4_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_45_4_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_45_4_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_67                     0x0800B0
#define VIDEO_ALPHA_COLOR_LOOKUP_67_7                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_67_7_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_67_7_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_67_7_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_67_6                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_67_6_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_67_6_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_67_6_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_89                     0x0800B4
#define VIDEO_ALPHA_COLOR_LOOKUP_89_9                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_89_9_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_89_9_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_89_9_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_89_8                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_89_8_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_89_8_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_89_8_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_AB                     0x0800B8
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_B                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_B_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_B_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_B_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_A                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_A_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_A_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_AB_A_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_CD                     0x0800BC
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_D                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_D_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_D_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_D_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_C                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_C_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_C_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_CD_C_BLUE              4:0

#define VIDEO_ALPHA_COLOR_LOOKUP_EF                     0x0800C0
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_F                   31:16
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_F_RED               31:27
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_F_GREEN             26:21
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_F_BLUE              20:16
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_E                   15:0
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_E_RED               15:11
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_E_GREEN             10:5
#define VIDEO_ALPHA_COLOR_LOOKUP_EF_E_BLUE              4:0

/* Panel Cursor Control */

#define PANEL_HWC_ADDRESS                               0x0800F0
#define PANEL_HWC_ADDRESS_ENABLE                        31:31
#define PANEL_HWC_ADDRESS_ENABLE_DISABLE                0
#define PANEL_HWC_ADDRESS_ENABLE_ENABLE                 1
#define PANEL_HWC_ADDRESS_EXT                           27:27
#define PANEL_HWC_ADDRESS_EXT_LOCAL                     0
#define PANEL_HWC_ADDRESS_EXT_EXTERNAL                  1
#define PANEL_HWC_ADDRESS_CS                            26:26
#define PANEL_HWC_ADDRESS_CS_0                          0
#define PANEL_HWC_ADDRESS_CS_1                          1
#define PANEL_HWC_ADDRESS_ADDRESS                       25:0

#define PANEL_HWC_LOCATION                              0x0800F4
#define PANEL_HWC_LOCATION_TOP                          27:27
#define PANEL_HWC_LOCATION_TOP_INSIDE                   0
#define PANEL_HWC_LOCATION_TOP_OUTSIDE                  1
#define PANEL_HWC_LOCATION_Y                            26:16
#define PANEL_HWC_LOCATION_LEFT                         11:11
#define PANEL_HWC_LOCATION_LEFT_INSIDE                  0
#define PANEL_HWC_LOCATION_LEFT_OUTSIDE                 1
#define PANEL_HWC_LOCATION_X                            10:0

#define PANEL_HWC_COLOR_12                              0x0800F8
#define PANEL_HWC_COLOR_12_2_RGB565                     31:16
#define PANEL_HWC_COLOR_12_1_RGB565                     15:0

#define PANEL_HWC_COLOR_3                               0x0800FC
#define PANEL_HWC_COLOR_3_RGB565                        15:0

/* Old Definitions +++ */
#define PANEL_HWC_COLOR_01                              0x0800F8
#define PANEL_HWC_COLOR_01_1_RED                        31:27
#define PANEL_HWC_COLOR_01_1_GREEN                      26:21
#define PANEL_HWC_COLOR_01_1_BLUE                       20:16
#define PANEL_HWC_COLOR_01_0_RED                        15:11
#define PANEL_HWC_COLOR_01_0_GREEN                      10:5
#define PANEL_HWC_COLOR_01_0_BLUE                       4:0

#define PANEL_HWC_COLOR_2                               0x0800FC
#define PANEL_HWC_COLOR_2_RED                           15:11
#define PANEL_HWC_COLOR_2_GREEN                         10:5
#define PANEL_HWC_COLOR_2_BLUE                          4:0
/* Old Definitions --- */

/* Alpha Control */

#define ALPHA_DISPLAY_CTRL                              0x080100
#define ALPHA_DISPLAY_CTRL_SELECT                       28:28
#define ALPHA_DISPLAY_CTRL_SELECT_PER_PIXEL             0
#define ALPHA_DISPLAY_CTRL_SELECT_ALPHA                 1
#define ALPHA_DISPLAY_CTRL_ALPHA                        27:24
#define ALPHA_DISPLAY_CTRL_FIFO                         17:16
#define ALPHA_DISPLAY_CTRL_FIFO_1                       0
#define ALPHA_DISPLAY_CTRL_FIFO_3                       1
#define ALPHA_DISPLAY_CTRL_FIFO_7                       2
#define ALPHA_DISPLAY_CTRL_FIFO_11                      3
#define ALPHA_DISPLAY_CTRL_PIXEL                        7:4
#define ALPHA_DISPLAY_CTRL_CHROMA_KEY                   3:3
#define ALPHA_DISPLAY_CTRL_CHROMA_KEY_DISABLE           0
#define ALPHA_DISPLAY_CTRL_CHROMA_KEY_ENABLE            1
#define ALPHA_DISPLAY_CTRL_PLANE                        2:2
#define ALPHA_DISPLAY_CTRL_PLANE_DISABLE                0
#define ALPHA_DISPLAY_CTRL_PLANE_ENABLE                 1
#define ALPHA_DISPLAY_CTRL_FORMAT                       1:0
#define ALPHA_DISPLAY_CTRL_FORMAT_16                    1
#define ALPHA_DISPLAY_CTRL_FORMAT_ALPHA_4_4             2
#define ALPHA_DISPLAY_CTRL_FORMAT_ALPHA_4_4_4_4         3

#define ALPHA_FB_ADDRESS                                0x080104
#define ALPHA_FB_ADDRESS_STATUS                         31:31
#define ALPHA_FB_ADDRESS_STATUS_CURRENT                 0
#define ALPHA_FB_ADDRESS_STATUS_PENDING                 1
#define ALPHA_FB_ADDRESS_EXT                            27:27
#define ALPHA_FB_ADDRESS_EXT_LOCAL                      0
#define ALPHA_FB_ADDRESS_EXT_EXTERNAL                   1
#define ALPHA_FB_ADDRESS_CS                             26:26
#define ALPHA_FB_ADDRESS_CS_0                           0
#define ALPHA_FB_ADDRESS_CS_1                           1
#define ALPHA_FB_ADDRESS_ADDRESS                        25:0

#define ALPHA_FB_WIDTH                                  0x080108
#define ALPHA_FB_WIDTH_WIDTH                            29:16
#define ALPHA_FB_WIDTH_OFFSET                           13:0

#define ALPHA_PLANE_TL                                  0x08010C
#define ALPHA_PLANE_TL_TOP                              26:16
#define ALPHA_PLANE_TL_LEFT                             10:0

#define ALPHA_PLANE_BR                                  0x080110
#define ALPHA_PLANE_BR_BOTTOM                           26:16
#define ALPHA_PLANE_BR_RIGHT                            10:0

#define ALPHA_CHROMA_KEY                                0x080114
#define ALPHA_CHROMA_KEY_MASK                           31:16
#define ALPHA_CHROMA_KEY_VALUE                          15:0

#define ALPHA_COLOR_LOOKUP_01                           0x080118
#define ALPHA_COLOR_LOOKUP_01_1                         31:16
#define ALPHA_COLOR_LOOKUP_01_1_RED                     31:27
#define ALPHA_COLOR_LOOKUP_01_1_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_01_1_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_01_0                         15:0
#define ALPHA_COLOR_LOOKUP_01_0_RED                     15:11
#define ALPHA_COLOR_LOOKUP_01_0_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_01_0_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_23                           0x08011C
#define ALPHA_COLOR_LOOKUP_23_3                         31:16
#define ALPHA_COLOR_LOOKUP_23_3_RED                     31:27
#define ALPHA_COLOR_LOOKUP_23_3_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_23_3_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_23_2                         15:0
#define ALPHA_COLOR_LOOKUP_23_2_RED                     15:11
#define ALPHA_COLOR_LOOKUP_23_2_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_23_2_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_45                           0x080120
#define ALPHA_COLOR_LOOKUP_45_5                         31:16
#define ALPHA_COLOR_LOOKUP_45_5_RED                     31:27
#define ALPHA_COLOR_LOOKUP_45_5_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_45_5_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_45_4                         15:0
#define ALPHA_COLOR_LOOKUP_45_4_RED                     15:11
#define ALPHA_COLOR_LOOKUP_45_4_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_45_4_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_67                           0x080124
#define ALPHA_COLOR_LOOKUP_67_7                         31:16
#define ALPHA_COLOR_LOOKUP_67_7_RED                     31:27
#define ALPHA_COLOR_LOOKUP_67_7_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_67_7_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_67_6                         15:0
#define ALPHA_COLOR_LOOKUP_67_6_RED                     15:11
#define ALPHA_COLOR_LOOKUP_67_6_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_67_6_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_89                           0x080128
#define ALPHA_COLOR_LOOKUP_89_9                         31:16
#define ALPHA_COLOR_LOOKUP_89_9_RED                     31:27
#define ALPHA_COLOR_LOOKUP_89_9_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_89_9_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_89_8                         15:0
#define ALPHA_COLOR_LOOKUP_89_8_RED                     15:11
#define ALPHA_COLOR_LOOKUP_89_8_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_89_8_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_AB                           0x08012C
#define ALPHA_COLOR_LOOKUP_AB_B                         31:16
#define ALPHA_COLOR_LOOKUP_AB_B_RED                     31:27
#define ALPHA_COLOR_LOOKUP_AB_B_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_AB_B_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_AB_A                         15:0
#define ALPHA_COLOR_LOOKUP_AB_A_RED                     15:11
#define ALPHA_COLOR_LOOKUP_AB_A_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_AB_A_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_CD                           0x080130
#define ALPHA_COLOR_LOOKUP_CD_D                         31:16
#define ALPHA_COLOR_LOOKUP_CD_D_RED                     31:27
#define ALPHA_COLOR_LOOKUP_CD_D_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_CD_D_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_CD_C                         15:0
#define ALPHA_COLOR_LOOKUP_CD_C_RED                     15:11
#define ALPHA_COLOR_LOOKUP_CD_C_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_CD_C_BLUE                    4:0

#define ALPHA_COLOR_LOOKUP_EF                           0x080134
#define ALPHA_COLOR_LOOKUP_EF_F                         31:16
#define ALPHA_COLOR_LOOKUP_EF_F_RED                     31:27
#define ALPHA_COLOR_LOOKUP_EF_F_GREEN                   26:21
#define ALPHA_COLOR_LOOKUP_EF_F_BLUE                    20:16
#define ALPHA_COLOR_LOOKUP_EF_E                         15:0
#define ALPHA_COLOR_LOOKUP_EF_E_RED                     15:11
#define ALPHA_COLOR_LOOKUP_EF_E_GREEN                   10:5
#define ALPHA_COLOR_LOOKUP_EF_E_BLUE                    4:0

/* CRT Graphics Control */

#define CRT_DISPLAY_CTRL                                0x080200
#define CRT_DISPLAY_CTRL_FIFO                           17:16
#define CRT_DISPLAY_CTRL_FIFO_1                         0
#define CRT_DISPLAY_CTRL_FIFO_3                         1
#define CRT_DISPLAY_CTRL_FIFO_7                         2
#define CRT_DISPLAY_CTRL_FIFO_11                        3
#define CRT_DISPLAY_CTRL_TV_PHASE                       15:15
#define CRT_DISPLAY_CTRL_TV_PHASE_ACTIVE_HIGH           0
#define CRT_DISPLAY_CTRL_TV_PHASE_ACTIVE_LOW            1
#define CRT_DISPLAY_CTRL_CLOCK_PHASE                    14:14
#define CRT_DISPLAY_CTRL_CLOCK_PHASE_ACTIVE_HIGH        0
#define CRT_DISPLAY_CTRL_CLOCK_PHASE_ACTIVE_LOW         1
#define CRT_DISPLAY_CTRL_VSYNC_PHASE                    13:13
#define CRT_DISPLAY_CTRL_VSYNC_PHASE_ACTIVE_HIGH        0
#define CRT_DISPLAY_CTRL_VSYNC_PHASE_ACTIVE_LOW         1
#define CRT_DISPLAY_CTRL_HSYNC_PHASE                    12:12
#define CRT_DISPLAY_CTRL_HSYNC_PHASE_ACTIVE_HIGH        0
#define CRT_DISPLAY_CTRL_HSYNC_PHASE_ACTIVE_LOW         1
#define CRT_DISPLAY_CTRL_BLANK                          10:10
#define CRT_DISPLAY_CTRL_BLANK_OFF                      0
#define CRT_DISPLAY_CTRL_BLANK_ON                       1
#define CRT_DISPLAY_CTRL_SELECT                         9:9
#define CRT_DISPLAY_CTRL_SELECT_PANEL                   0
#define CRT_DISPLAY_CTRL_SELECT_CRT                     1
#define CRT_DISPLAY_CTRL_TIMING                         8:8
#define CRT_DISPLAY_CTRL_TIMING_DISABLE                 0
#define CRT_DISPLAY_CTRL_TIMING_ENABLE                  1
#define CRT_DISPLAY_CTRL_PIXEL                          7:4
#define CRT_DISPLAY_CTRL_GAMMA                          3:3
#define CRT_DISPLAY_CTRL_GAMMA_DISABLE                  0
#define CRT_DISPLAY_CTRL_GAMMA_ENABLE                   1
#define CRT_DISPLAY_CTRL_PLANE                          2:2
#define CRT_DISPLAY_CTRL_PLANE_DISABLE                  0
#define CRT_DISPLAY_CTRL_PLANE_ENABLE                   1
#define CRT_DISPLAY_CTRL_FORMAT                         1:0
#define CRT_DISPLAY_CTRL_FORMAT_8                       0
#define CRT_DISPLAY_CTRL_FORMAT_16                      1
#define CRT_DISPLAY_CTRL_FORMAT_32                      2

#define CRT_FB_ADDRESS                                  0x080204
#define CRT_FB_ADDRESS_STATUS                           31:31
#define CRT_FB_ADDRESS_STATUS_CURRENT                   0
#define CRT_FB_ADDRESS_STATUS_PENDING                   1
#define CRT_FB_ADDRESS_EXT                              27:27
#define CRT_FB_ADDRESS_EXT_LOCAL                        0
#define CRT_FB_ADDRESS_EXT_EXTERNAL                     1
#define CRT_FB_ADDRESS_CS                               26:26
#define CRT_FB_ADDRESS_CS_0                             0
#define CRT_FB_ADDRESS_CS_1                             1
#define CRT_FB_ADDRESS_ADDRESS                          25:0

#define CRT_FB_WIDTH                                    0x080208
#define CRT_FB_WIDTH_WIDTH                              29:16
#define CRT_FB_WIDTH_OFFSET                             13:0

#define CRT_HORIZONTAL_TOTAL                            0x08020C
#define CRT_HORIZONTAL_TOTAL_TOTAL                      27:16
#define CRT_HORIZONTAL_TOTAL_DISPLAY_END                11:0

#define CRT_HORIZONTAL_SYNC                             0x080210
#define CRT_HORIZONTAL_SYNC_WIDTH                       23:16
#define CRT_HORIZONTAL_SYNC_START                       11:0

#define CRT_VERTICAL_TOTAL                              0x080214
#define CRT_VERTICAL_TOTAL_TOTAL                        26:16
#define CRT_VERTICAL_TOTAL_DISPLAY_END                  10:0

#define CRT_VERTICAL_SYNC                               0x080218
#define CRT_VERTICAL_SYNC_HEIGHT                        21:16
#define CRT_VERTICAL_SYNC_START                         10:0

#define CRT_SIGNATURE_ANALYZER                          0x08021C
#define CRT_SIGNATURE_ANALYZER_STATUS                   31:16
#define CRT_SIGNATURE_ANALYZER_ENABLE                   3:3
#define CRT_SIGNATURE_ANALYZER_ENABLE_DISABLE           0
#define CRT_SIGNATURE_ANALYZER_ENABLE_ENABLE            1
#define CRT_SIGNATURE_ANALYZER_RESET                    2:2
#define CRT_SIGNATURE_ANALYZER_RESET_NORMAL             0
#define CRT_SIGNATURE_ANALYZER_RESET_RESET              1
#define CRT_SIGNATURE_ANALYZER_SOURCE                   1:0
#define CRT_SIGNATURE_ANALYZER_SOURCE_RED               0
#define CRT_SIGNATURE_ANALYZER_SOURCE_GREEN             1
#define CRT_SIGNATURE_ANALYZER_SOURCE_BLUE              2

#define CRT_CURRENT_LINE                                0x080220
#define CRT_CURRENT_LINE_LINE                           10:0

#define CRT_MONITOR_DETECT                              0x080224
#define CRT_MONITOR_DETECT_ENABLE                       24:24
#define CRT_MONITOR_DETECT_ENABLE_DISABLE               0
#define CRT_MONITOR_DETECT_ENABLE_ENABLE                1
#define CRT_MONITOR_DETECT_RED                          23:16
#define CRT_MONITOR_DETECT_GREEN                        15:8
#define CRT_MONITOR_DETECT_BLUE                         7:0

/* CRT Cursor Control */

#define CRT_HWC_ADDRESS                                 0x080230
#define CRT_HWC_ADDRESS_ENABLE                          31:31
#define CRT_HWC_ADDRESS_ENABLE_DISABLE                  0
#define CRT_HWC_ADDRESS_ENABLE_ENABLE                   1
#define CRT_HWC_ADDRESS_EXT                             27:27
#define CRT_HWC_ADDRESS_EXT_LOCAL                       0
#define CRT_HWC_ADDRESS_EXT_EXTERNAL                    1
#define CRT_HWC_ADDRESS_CS                              26:26
#define CRT_HWC_ADDRESS_CS_0                            0
#define CRT_HWC_ADDRESS_CS_1                            1
#define CRT_HWC_ADDRESS_ADDRESS                         25:0

#define CRT_HWC_LOCATION                                0x080234
#define CRT_HWC_LOCATION_TOP                            27:27
#define CRT_HWC_LOCATION_TOP_INSIDE                     0
#define CRT_HWC_LOCATION_TOP_OUTSIDE                    1
#define CRT_HWC_LOCATION_Y                              26:16
#define CRT_HWC_LOCATION_LEFT                           11:11
#define CRT_HWC_LOCATION_LEFT_INSIDE                    0
#define CRT_HWC_LOCATION_LEFT_OUTSIDE                   1
#define CRT_HWC_LOCATION_X                              10:0

#define CRT_HWC_COLOR_12                                0x080238
#define CRT_HWC_COLOR_12_2_RGB565                       31:16
#define CRT_HWC_COLOR_12_1_RGB565                       15:0

#define CRT_HWC_COLOR_3                                 0x08023C
#define CRT_HWC_COLOR_3_RGB565                          15:0

/* Old Definitions +++ */
#define CRT_HWC_COLOR_01                                0x080238
#define CRT_HWC_COLOR_01_1_RED                          31:27
#define CRT_HWC_COLOR_01_1_GREEN                        26:21
#define CRT_HWC_COLOR_01_1_BLUE                         20:16
#define CRT_HWC_COLOR_01_0_RED                          15:11
#define CRT_HWC_COLOR_01_0_GREEN                        10:5
#define CRT_HWC_COLOR_01_0_BLUE                         4:0

#define CRT_HWC_COLOR_2                                 0x08023C
#define CRT_HWC_COLOR_2_RED                             15:11
#define CRT_HWC_COLOR_2_GREEN                           10:5
#define CRT_HWC_COLOR_2_BLUE                            4:0
/* Old Definitions --- */

/* Palette RAM */

#define CRT_PALETTE_RAM                                 0x080C00
#define PANEL_PALETTE_RAM                               0x080400
#define VIDEO_PALETTE_RAM                               0x080800

