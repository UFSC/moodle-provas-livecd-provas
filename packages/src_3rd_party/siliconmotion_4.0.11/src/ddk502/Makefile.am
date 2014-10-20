#Copyright (c) 2012 by Silicon Motion, Inc. (SMI)
#
#Permission is hereby granted, free of charge, to any person obtaining a copy 
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights to 
#use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies 
#of the Software, and to permit persons to whom the Software is furnished to 
#do so, subject to the following conditions:

#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
#EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
#NONINFRINGEMENT.  IN NO EVENT SHALL MILL CHEN, MONK LIU, ALEX YAO, 
#SUNNY YANG, ILENA ZHOU, MANDY WANG OR COPYRIGHT 
#HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
#WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
#FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR 
#OTHER DEALINGS IN THE SOFTWARE.

AM_CFLAGS = @XORG_CFLAGS@
smiddk502_drv_la_LTLIBRARIES = smiddk502_drv.la
smiddk502_drv_la_LDFLAGS = -module -avoid-version -lpci
smiddk502_drv_ladir = @moduledir@/drivers

smiddk502_drv_la_SOURCES = \
	./ddk502_voyager.h \
	./ddk502_chip.h \
	./ddk502_display.h \
	./ddk502_mode.h \
	./ddk502_clock.h \
	./ddk502_regsc.h \
	./ddk502_regdc.h \
	./ddk502_regzv.h \
	./ddk502_regdma.h \
	./ddk502_reggpio.h \
	./ddk502_hardware.h \
	./ddk502_power.h \
	./ddk502_ddkdebug.h \
	./ddk502_swi2c.h \
	./ddk502_swi2c.c \
	./ddk502_chip.c \
	./ddk502_display.c \
	./ddk502_power.c \
	./ddk502_hardware.c \
	./ddk502_clock.c \
	./ddk502_ddkdebug.c \
	./ddk502_mode.c \
	./ddk502_os.c \
	./ddk502_os.h \
	./ddk502_linux.c \
	./ddk502_help.c \
	./ddk502_help.h \
	./version.h \
	./502ddk_module.c 

	

