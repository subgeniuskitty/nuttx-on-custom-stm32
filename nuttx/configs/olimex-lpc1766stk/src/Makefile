############################################################################
# configs/olimex-lpc1766stk/src/Makefile
#
#   Copyright (C) 2010, 2012 Gregory Nutt. All rights reserved.
#   Author: Gregory Nutt <gnutt@nuttx.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name NuttX nor the names of its contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
############################################################################

-include $(TOPDIR)/Make.defs

ASRCS =
CSRCS = lpc17_boot.c lpc17_leds.c lpc17_ssp.c

ifeq ($(CONFIG_LIB_BOARDCTL),y)
CSRCS += lpc17_appinit.c
endif

ifeq ($(CONFIG_USBMSC),y)
CSRCS += lpc17_usbmsc.c
endif

ifeq ($(CONFIG_NX_LCDDRIVER),y)
CSRCS += lpc17_lcd.c
endif

ifeq ($(CONFIG_ARCH_BUTTONS),y)
CSRCS += lpc17_buttons.c
endif

ifeq ($(CONFIG_CAN),y)
CSRCS += lpc17_can.c
endif

ifeq ($(CONFIG_USBHOST_HIDMOUSE),y)
CSRCS += lpc17_hidmouse.c
endif

include $(TOPDIR)/configs/Board.mk
