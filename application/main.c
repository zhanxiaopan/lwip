/**
 *  \file new 1
 *  \brief Brief
 */

#include "bsp_config.h"
#include "system.h"
#include <stdio.h>


int main(void) {
    puts("Sys_Begin");
	// Init the system for application.
	system_init();
	puts("init_finished");

	// Loop the system.
	// so far no priority scheduling done yet.
	while(1)
	{

		system_loop();
	}
}
