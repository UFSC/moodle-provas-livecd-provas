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
#ifndef SMI_DBG_INC
#define SMI_DBG_INC

#if 0
#undef  SMI_DBG
#else
#define  SMI_DBG
#endif
extern int smi_indent;

#ifdef SMI_DBG
#define VERBLEV 1
#define ENTER() xf86ErrorFVerb(VERBLEV, "%*c %s \n", \
				smi_indent++, '>', __FUNCTION__)

#define LEAVE(...) \
	do{ \
		xf86ErrorFVerb(VERBLEV, "%*c %s \n", \
					--smi_indent, '<', __FUNCTION__); \
		return __VA_ARGS__; \					
	}while(0)
#define DEBUG(...) xf86Msg(X_NOTICE,__VA_ARGS__)
#define ERROR(...) xf86Msg(X_ERROR,__VA_ARGS__)
#else
#define VERBLEV	4
#define ENTER()
#define LEAVE(...) return __VA_ARGS__
#define DEBUG(...)
#endif
#endif   /* ----- #ifndef SMI_DBG_INC  ----- */
