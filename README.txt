NuttX on Liquid Fusion
======================

### Overview

These notes cover my initial bringup of NuttX 7.22 on Liquid Fusion rev. 2. The OS boots and executes userspace applications successfully.

### Build Environment

Unpack source tarballs to nuttx/ and apps/ in the same folder.

Install KConfig
  apt-get install gperf flex bison libncurses5-dev
  Download nuttx-tools repository
  cd kconfig-frontends
  ./configure --enable-mconf --disable-nconf --disable-gconf --disable-qconf
  make
  make install
  Add /usr/local/lib to $LD_LIBRARY_PATH else kconfig-conf can't find a library
    In ~/.profile ==> export LD_LIBRARY_PATH="/usr/local/lib"

Commands
  nuttx/tools/configure.sh liquidfusion/liquidfusion
  make oldconfig (to refresh config)
  make menuconfig
  make
  make distclean
  make apps_distclean

### Debugging

Launch OpenOCD as root
  openocd -f /usr/share/openocd/scripts/board/st_nucleo_f103rb.cfg -s /usr/share/openocd/scripts/

Launch and connect GDB -> OpenOCD
  arm-none-eabi-gdb <binary>
  target remote localhost:3333
  monitor reset halt
  load
  step

### Todo

Turns out we can't use HSE or LSI for RTC. See arch/arm/src/stm32/stm32_rtcounter.c around line 122-ish. I can input a clocking waveform directly to the LSE pins. See STM32F10x reference manual on page 129 of 1133. This requires setting LSEBYP in the RCC_BDCR register. I used 32.768 kHz frequency, 0.75 V amplitude and 0.75 V offset on waveform generator. I edited arch/arm/src/stm32/stm32_lse.c around line 104 to set LSEBYP bit in BDCR register and added an external BNC.

  modifyreg16(STM32_RCC_BDCR, RCC_BDCR_LSEON, 0);
  modifyreg16(STM32_RCC_BDCR, 0, RCC_BDCR_LSEBYP);
  modifyreg16(STM32_RCC_BDCR, 0, RCC_BDCR_LSEON);
