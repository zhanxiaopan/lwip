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

//extern const uint32_t* g_pfnVectors;

//void myPrintf (void (*fPointer)(const char *_format, int), int val)
//{
//        fPointer("Hallo%d\n",val);
//}

int main(void) {
#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_BL
    if(resetIsByWatchdog()) {
        HWREG(0xE000ED08) = 0x20000;
        //
        // Load the stack pointer from the application's vector table.
        //
        __asm("    ldr     r1, [r0]\n"
              "    mov     sp, r1\n");

        //
        // Load the initial PC from the application's vector table and branch to
        // the application's entry point.
        //
        __asm("    ldr     r0, [r0, #4]\n"
              "    bx      r0\n");
        // Init the system for application.
    }
#endif
	system_init();
	//myPrintf(0x000156a1,12);
	//puts("Hola\n");
	// Loop the system.
	// so far no priority scheduling done yet.
	while(1)
	{

		system_loop();
	}
}
