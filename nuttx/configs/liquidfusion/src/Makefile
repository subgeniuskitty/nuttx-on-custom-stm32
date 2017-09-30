-include $(TOPDIR)/Make.defs

ASRCS =
CSRCS = stm32_boot.c stm32_userleds.c

ifeq ($(CONFIG_LIB_BOARDCTL),y)
CSRCS += stm32_appinit.c
endif

include $(TOPDIR)/configs/Board.mk
