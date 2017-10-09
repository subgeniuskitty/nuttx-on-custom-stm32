#include <nuttx/config.h>
#include <stdio.h>
#include <arch/board/board.h>
#include "up_arch.h"

const char * stm32_getchipid(void) {
    static char cpuid[12];
    int i;

    for (i = 0; i < 12; i++) cpuid[i] = getreg8(0x1ffff7e8+i);

    return cpuid;
}

const char * stm32_getchipid_string(void) {
    static char cpuid[27];
    int i, c;

    for (i = 0, c = 0; i < 12; i++) {
        sprintf(&cpuid[c], "%02X", getreg8(0x1ffff7e8+11-i));
        c += 2;
        if (i % 4 == 3) cpuid[c++] = '-';
    }

    cpuid[26] = '\0';
    return cpuid;
}
