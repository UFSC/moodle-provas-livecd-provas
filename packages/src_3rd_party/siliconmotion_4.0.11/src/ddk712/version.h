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

#ifndef _VERSION_H_
#define _VERSION_H_

#define LIBRARY_VERSION 0x00000006
#define MAJOR_VERSION(v) (v >> 16)
#define MINOR_VERSION(v) (v & 0x0000ffff)
#if XORG_VERSION_CURRENT < 10706000
uint32_t getLibVersion(void)
{
	return(LIBRARY_VERSION);
}
#endif
#endif /* _VERSION_H_ */
