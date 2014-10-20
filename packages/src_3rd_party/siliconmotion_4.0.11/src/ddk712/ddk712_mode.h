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

#ifndef DDK712_MODE_H__
#define DDK712_MODE_H__

typedef struct{
	unsigned short h_res;
	unsigned short v_res;
	char vsync;
	char svr[14];/* shadow vga register :svr 40 => svr 4d*/
	char ccr[2];/* pixel pll:ccr6c,6d */
}SM712CrtTiming;

typedef struct{
	unsigned short h_res;
	unsigned short v_res;
	char vsync;
	char fpr[14];/* fpr50 ==> fpr57,fpr5a*/
	char ccr[2];/* ccr6e.ccr6f*/
}SM712PnlTiming;


void ddk712_setModeTiming(int channel,int x,int y,int hz);
#endif
