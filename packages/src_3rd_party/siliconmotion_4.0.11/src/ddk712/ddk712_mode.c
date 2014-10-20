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

#include "ddk712_mode.h"
#include "ddk712_help.h"
#include "ddk712_reg.h"

const SM712CrtTiming sm712_crt_modedb[] = {
/* 640x480 */
{
    640,480,60,
    {0x5F, 0x4F, 0x00, 0x54, 0x00, 0x0B, 0xDF, 0x00,
    0xEA, 0x0C, 0x2E, 0x00, 0x4F, 0xDF}, {0x07, 0x82},
},
{
    640,480,75,
    {0x64, 0x4F, 0x00, 0x52, 0x1A, 0xF2, 0xDF, 0x00,
    0xE0, 0x03, 0x0F, 0xC0, 0x4F, 0xDF}, {0x16, 0x85},
},
{
    640,480,85,
    {0x63, 0x4F, 0x00, 0x57, 0x1E, 0xFB, 0xDF, 0x00,
    0xE0, 0x03, 0x0F, 0xC0, 0x4F, 0xDF}, {0x88, 0x9B},
},
/* 800x480 */
{
    800,480,60,
    {0x6B, 0x63, 0x00, 0x69, 0x1B, 0xF2, 0xDF, 0x00,
    0xE2, 0xE4, 0x1F, 0xC0, 0x63, 0xDF}, {0x2C, 0x17},
},
{
    800,480,75,
    {0x6B, 0x63, 0x00, 0x69, 0x1B, 0xF2, 0xDF, 0x00,
    0xE2, 0xE4, 0x1F, 0xC0, 0x63, 0xDF}, {0x2C, 0x17},
},
{
    800,480,85,
    {0x6B, 0x63, 0x00, 0x69, 0x1B, 0xF2, 0xDF, 0x00,
    0xE2, 0xE4, 0x1F, 0xC0, 0x63, 0xDF}, {0x2C, 0x17},
},
/* 800x600 */
{
    800,600,60,
    {0x7F, 0x63, 0x00, 0x69, 0x18, 0x72, 0x57, 0x00,
    0x58, 0x0C, 0xE0, 0x20, 0x63, 0x57}, {0x1C, 0x85},
},
{
    800,600,75,
    {0x7F, 0x63, 0x00, 0x66, 0x10, 0x6F, 0x57, 0x00,
    0x58, 0x0B, 0xE0, 0x20, 0x63, 0x57}, {0x4C, 0x8B},
},
{
    800,600,85,
    {0x7E, 0x63, 0x00, 0x68, 0x10, 0x75, 0x57, 0x00,
    0x58, 0x0B, 0xE0, 0x20, 0x63, 0x57},{ 0x37, 0x87},
},
#if 0
/* 1024x600 */
{
    1024,600,60,
    0xA3, 0x7F, 0x00, 0x82, 0x0B, 0x6F, 0x57, 0x00,
    0x5C, 0x0F, 0xE0, 0xE0, 0x7F, 0x57, 0x16, 0x07,
},
#endif
/* 1024x768 */

{
    1024,768,60,
    {0xA3, 0x7F, 0x00, 0x86, 0x15, 0x24, 0xFF, 0x00,
    0x01, 0x07, 0xE5, 0x20, 0x7F, 0xFF}, {0x52, 0x89},
},
{
    1024,768,75,
    {0x9F, 0x7F, 0x00, 0x82, 0x0E, 0x1E, 0xFF, 0x00,
    0x00, 0x03, 0xE5, 0x20, 0x7F, 0xFF}, {0x0B, 0x02},
},
{
    1024,768,85,
    {0xA7, 0x7F, 0x00, 0x86, 0x12, 0x26, 0xFF, 0x00,
    0x00, 0x03, 0xE5, 0x20, 0x7F, 0xFF}, {0x70, 0x11},
},

/* 1280x1024 */
{
    1280,1024,60,
    {0xCE, 0x9F, 0x00, 0xA7, 0x15, 0x28, 0xFF, 0x00,
    0x00, 0x03, 0x4A, 0x20, 0x9F, 0xFF},{ 0x53, 0x0B},
},
{
    1280,1024,75,
    {0xCE, 0x9F, 0x00, 0xA2, 0x14, 0x28, 0xFF, 0x00,
//           0x00, 0x03, 0x4A, 0x20, 0x9F, 0xFF, 0x13, 0x02,
    0x00, 0x03, 0x4A, 0x20, 0x9F, 0xFF}, {0x42, 0x07},           
},
{
    1280,1024,85,
    {0xD3, 0x9F, 0x00, 0xA8, 0x1C, 0x2E, 0xFF, 0x00,
//           0x00, 0x03, 0x4A, 0x20, 0x9F, 0xFF, 0x16, 0x42,
    0x00, 0x03, 0x4A, 0x20, 0x9F, 0xFF}, {0x0b, 0x01},           
},

};

const SM712PnlTiming sm712_pnl_modedb[] = {
#if 0
{

    640,480,60,
    0x04, 0x24, 0x63, 0x4F, 0x52, 0x0C, 0xDF, 0xE9,
    0x00, 0x03, 0x59, 0x00, 0x4F, 0xDF, 0x03, 0x02,
},
	
{
    640,480,75,
    0x04, 0x24, 0x63, 0x4F, 0x52, 0x0C, 0xDF, 0xE9,
    0x00, 0x03, 0x59, 0xC0, 0x4F, 0xDF, 0x16, 0x85,
},
{
    640,480,85,
    0x04, 0x24, 0x63, 0x4F, 0x52, 0x0C, 0xDF, 0xE9,
    0x00, 0x03, 0x59, 0xC0, 0x4F, 0xDF, 0x88, 0x9B,
},
#endif
{
    800,480,60,
    {0x02, 0x24, 0x7B, 0x63, 0x67, 0xF3, 0xDF, 0xE2,
    0x00, 0x03, 0x41, 0xC0, 0x63, 0xDF}, {0x2C, 0x17},
},
#if 0
{
    800,480,75,
    0x6B, 0x63, 0x00, 0x69, 0x1B, 0xF2, 0xDF, 0x00,
    0xE2, 0xE4, 0x1F, 0xC0, 0x63, 0xDF, 0x2C, 0x17,
},
{
    800,480,85,
    0x6B, 0x63, 0x00, 0x69, 0x1B, 0xF2, 0xDF, 0x00,
    0xE2, 0xE4, 0x1F, 0xC0, 0x63, 0xDF, 0x2C, 0x17,
},
#endif
{
    800,600,60,
    {0x04, 0x48, 0x83, 0x63, 0x69, 0x73, 0x57, 0x58,
    0x00, 0x03, 0x7B, 0x20, 0x63, 0x57}, {0x0E, 0x05},
},
#if 0
{
    800,600,75,
    0x7F, 0x63, 0x00, 0x66, 0x10, 0x6F, 0x57, 0x00,
    0x58, 0x0B, 0xE0, 0x20, 0x63, 0x57, 0x4C, 0x8B,
},
{
    800,600,85,
    0x7E, 0x63, 0x00, 0x68, 0x10, 0x75, 0x57, 0x00,
    0x58, 0x0B, 0xE0, 0x20, 0x63, 0x57, 0x37, 0x87,
},
#endif
#if 0
{
    1024,600,60,
    0x04, 0x48, 0x95, 0x7F, 0x86, 0x70, 0x57, 0x5B,
    0x00, 0x60, 0x1c, 0x22, 0x7F, 0x57, 0x16, 0x07,
},
#endif
{
    1024,768,60,
    {0x06, 0x68, 0xA7, 0x7F, 0x83, 0x25, 0xFF, 0x02,
    0x00, 0x62, 0x85, 0x20, 0x7F, 0xFF}, {0x29, 0x09},
},
#if 0
{
    1024,768,75,
    0x9F, 0x7F, 0x00, 0x82, 0x0E, 0x1E, 0xFF, 0x00,
    0x00, 0x03, 0xE5, 0x20, 0x7F, 0xFF, 0x0B, 0x02,
},
{
    1024,768,85,
    0xA7, 0x7F, 0x00, 0x86, 0x12, 0x26, 0xFF, 0x00,
    0x00, 0x03, 0xE5, 0x20, 0x7F, 0xFF, 0x70, 0x11,
},
#endif
{
    1280,1024,60,
    {0x08, 0x8C, 0xD5, 0x9F, 0xAB, 0x26, 0xFF, 0x00,
    0x00, 0x03, 0x7E, 0x20, 0x9F, 0xFF}, {0x53, 0x0B},
},
#if 0
{
    1280,1024,75,
    0xCE, 0x9F, 0x00, 0xA2, 0x14, 0x28, 0xFF, 0x00,
    0x00, 0x03, 0x4A, 0x20, 0x9F, 0xFF, 0x42, 0x07,           
},
{
    1280,1024,85,
    0xD3, 0x9F, 0x00, 0xA8, 0x1C, 0x2E, 0xFF, 0x00,
    0x00, 0x03, 0x4A, 0x20, 0x9F, 0xFF, 0x0b, 0x01,           
},
#endif
};

#define CNT_CRT sizeof(sm712_crt_modedb)/sizeof(sm712_crt_modedb[0])
#define CNT_PNL sizeof(sm712_pnl_modedb)/sizeof(sm712_pnl_modedb[0])

_X_EXPORT void ddk712_setModeTiming(int channel,int x,int y,int hz)
{
	int i;
	int index;
	index = 0;
	if(channel == 0){
		while(index < CNT_CRT){
			if(sm712_crt_modedb[index].h_res == x &&
				sm712_crt_modedb[index].v_res == y &&
				sm712_crt_modedb[index].vsync == hz)
				break;
			index++;
		}
		if(index == CNT_CRT){
			/* no mode found in table */
			return;
		}

		/* program svr */
		for(i=0;i<14;i++){
			poke_svr(0x40+i,sm712_crt_modedb[index].svr[i]);
		}
		
		/* work around crt */
		if(y>=1024){
			poke_crt(0x30,peek_crt(0x30)|9);
		}else{
			poke_crt(0x30,peek_crt(0x30)&~9);
		}

		/* programe ccr for pixel clock */
		poke_ccr(0x6c,sm712_crt_modedb[index].ccr[0]);
		poke_ccr(0x6d,sm712_crt_modedb[index].ccr[1]);

	}else{
		while(index < CNT_PNL){
			if(sm712_pnl_modedb[index].h_res == x &&
				sm712_pnl_modedb[index].v_res == y &&
				sm712_pnl_modedb[index].vsync == hz)
				break;
			index++;				
		}
		if(index == CNT_PNL){
			return;
		}

		/* program FPR */
		for(i=0;i<8;i++){
			poke_fpr(0x50+i,sm712_pnl_modedb[index].fpr[i]);
		}

		/* program fpr5a*/
		poke_fpr(0x5a,sm712_pnl_modedb[index].fpr[0xa]);

		/* program ccr for pixel clock*/
		poke_ccr(0x6e,sm712_pnl_modedb[index].ccr[0]);
		poke_ccr(0x6f,sm712_pnl_modedb[index].ccr[1]);

	}
}
