/**
 * This project is used to erase the EEPROM and restore
 * the initial status of the board.
 *
 * Simply just burn it into the board and wait for a few
 * second. Then you can burn your actually application with
 * no previous configuration left.
 */

#include <stdint.h>
#include <stdbool.h>
#include "eeprom.h"
#include <sysctl.h>

int main(void) {
	SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0));
	EEPROMMassErase();
	while(1);
}
