/**
 *  \file main.c
 *  \brief
 *  modified by TMS in Sept. 2017
 */

#include "bsp_config.h"
#include "system.h"
#include <stdio.h>

#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_BL
#include "reset.h"
#endif

int main(void) {
	//==
	//The following section is only avaliable
	//in the Bootloader project.
	//It redirect the program to 0x20000 if the 
	//board is reset by "resetLaunch()" with Watch-\
	//dog.
	//==
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
	//initialization of the system
	system_init();
	//major loop of the system
	while(1) system_loop();
}
