/**
 *  \file main.c
 *  \brief
 *  modified by TMS in Sept. 2017
 */

#include "bsp_config.h"
#include "system.h"
#include <stdio.h>
#include "aio_config.h"

#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_BL
#include "reset.h"
#endif

//static uint8_t BIN_MARK __attribute__((section (".identifier")))  __attribute__((used))= 30;

int main(void) {

	aio_readConfig();
	//==
	//The following section is only avaliable
	//in the Bootloader project.
    // - # Deprecated
	// - It redirects the program to 0x20000 if the
	// - uC is reset by "resetLaunch()" with Watch-\
	// - dog.
	//==
    //04.12.2017 changed by Tms:
    //in all-in-one version, redirect is trigger by
    //aio_bl_config value stored in EEMROM
    //

#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_BL
    //if(resetIsByWatchdog()) {
    if(aio_bl_config == AIO_BL_REDI) {
        //both bin should be ready before redirect
        if(aio_bin_exist_flag == AIO_BIN_BOTH)
        {
            //eips exist and redirect to eips
            if(aio_network_sel == AIO_NETWORK_EIPS)
                HWREG(0xE000ED08) = AIO_EIPS_VTABLE_ADDR; //eips vtalbe
            //pnio/pnioio exist and redirect to it
            else if ( aio_network_sel == AIO_NETWORK_PNIO
                    || aio_network_sel == AIO_NETWORK_PNIOIO
                    )
                HWREG(0xE000ED08) = AIO_PNIO_VTABLE_ADDR; //pnio vtalbe
            else
            {
                //bad state,
                //should redirect to either eips or pnio/pnioio
                printf("Bootloader configuration error!@0");
                while(1);
            }

            //the following code will be executed if
            //the uC is not trapped in while(1) above

            //stack pointer
            __asm("    ldr     r1, [r0]\n"
                  "    mov     sp, r1\n");
            //program counter
            __asm("    ldr     r0, [r0, #4]\n"
                  "    bx      r0\n");
        }
        //redirect but no bin exists
        else
        {
            //both bins should be ready before redirect
            printf("Bootloader configuration error!@1");
            while(1);
        }
    }

    /*
     * #FOR BOOTLOADER#
     * if aio_bl_config is not configured to
     * redirect state,
     * the code below will be reachable.
     */
#endif

	//initialization of the system
	system_init();
	//major loop of the system

	while(1) system_loop();
}
