/**
 *  \file new 1
 *  \brief Brief
 */

#include "bsp_config.h"
#include "system.h"
#include <stdio.h>


int main(void) {
	// Init the system for application.
	system_init();

	// Loop the system.
	// so far no priority scheduling done yet.
	while(1)
	{

		system_loop();
	}
}
