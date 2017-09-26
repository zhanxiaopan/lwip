/**
 *  \file new 1
 *  \brief Brief
 */

#include "bsp_config.h"
#include "system.h"
#include <stdio.h>

#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_BL
#include "reset.h"
#endif

int main(void) {
#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_BL
    if(resetIsByWatchdog()) {
        //vtalbe
        HWREG(0xE000ED08) = 0x20000;
        //stack pointer
        __asm("    ldr     r1, [r0]\n"
              "    mov     sp, r1\n");
        //program counter
        __asm("    ldr     r0, [r0, #4]\n"
              "    bx      r0\n");
    }
#endif
	system_init();
	while(1)
	{

		system_loop();
	}
}
