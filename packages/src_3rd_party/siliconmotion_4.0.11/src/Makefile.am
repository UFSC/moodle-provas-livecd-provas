#  Copyright 2005 Adam Jackson.
#  Copyright (c) 2012 by Silicon Motion, Inc. (SMI)
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  on the rights to use, copy, modify, merge, publish, distribute, sub
#  license, and/or sell copies of the Software, and to permit persons to whom
#  the Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice (including the next
#  paragraph) shall be included in all copies or substantial portions of the
#  Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
#  ADAM JACKSON BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
#  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
#  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


# this is obnoxious:
# -module lets us name the module exactly how we want
# -avoid-version prevents gratuitous .0.0.0 version numbers on the end
#
#
# -ladir passes a dummy rpath to libtool so the thing will actually link
# TODO: -nostdlib/-Bstatic/-lgcc platform magic, not installing the .a, etc.
AM_CFLAGS = @XORG_CFLAGS@
siliconmotion_drv_la_LTLIBRARIES = siliconmotion_drv.la
siliconmotion_drv_la_LDFLAGS = -module -avoid-version
siliconmotion_drv_ladir = @moduledir@/drivers

SUBDIRS = ddk750 ddk712 ddk502 

siliconmotion_drv_la_SOURCES = \
        smi_common.h \
        smi_dbg.h \
        smi.h \
        smi_output.c \
        smi_crtc.c \
        smi_ver.h \
        smi_driver.h \
        smi_video.h \
        smi_common.c \
        smi_driver.c \
        smi_video.c \
        ./drv750/smi_750_driver.h \
        ./drv750/smi_750_hw.h \ 
        ./drv750/smi_750_output.h \
        ./drv750/smi_750_crtc.h \
        ./drv502/smi_502_driver.h \
        ./drv502/smi_502_hw.h \
        ./drv750/smi_750_output.c \
        ./drv750/smi_750_crtc.c \
        ./drv750/smi_750_hw.c \
        ./drv750/smi_750_driver.c \
        ./drv750le/smi_750le_driver.h \
        ./drv750le/smi_750le_hw.h \
        ./drv750le/smi_750le_output.c \
        ./drv750le/smi_750le_crtc.c \
        ./drv750le/smi_750le_driver.c \
        ./drv750le/smi_750le_hw.c \
        ./drv712/smi_712_driver.h \
        ./drv712/smi_712_hw.h \
        ./drv712/smi_712_output.c \
        ./drv712/smi_712_crtc.c \
        ./drv712/smi_712_hw.c \
        ./drv712/smi_712_driver.c \
        ./drv502/smi_502_crtc.c \
        ./drv502/smi_502_output.c \
        ./drv502/smi_502_driver.c \
        ./drv502/smi_502_hw.c 
