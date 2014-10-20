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
#ifndef  SMI_VER_INC
#define  SMI_VER_INC

#define SMI_VER_MAJOR 4
#define SMI_VER_MINOR 0
#define SMI_VER_PATCH 11
#define VERSION_ATTACH_TMP(a,b,c)  #a"."#b"."#c
#define VERSION_ATTACH_TMP2(a,b,c) VERSION_ATTACH_TMP(a,b,c)
#define SILICONMOTION_VERSION_NAME VERSION_ATTACH_TMP2(SMI_VER_MAJOR,SMI_VER_MINOR,SMI_VER_PATCH)

#define SILICONMOTION_DRIVER_VERSION ((SMI_VER_MAJOR << 24) | \
		(SMI_VER_MINOR << 16) | \
		(SMI_VER_PATCH))

#define SILICONMOTION_NAME          "Silicon Motion"
#define SILICONMOTION_DRIVER_NAME   "siliconmotion"

#define SILICONMOTION_VERSION_NAME	VERSION_ATTACH_TMP2(SMI_VER_MAJOR,SMI_VER_MINOR,SMI_VER_PATCH)


#endif
