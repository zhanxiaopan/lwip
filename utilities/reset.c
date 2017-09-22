/*
 * reset.c
 *
 *  Created on: Sep 22, 2017
 *      Author: CNTHXIE
 */

#include "reset.h"
#include <stdint.h>
#include "hw_memmap.h"
#include "sysctl.h"
#include "watchdog.h"

void resetInit()
{

    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
    //
    // Wait for the Watchdog 0 module to be ready.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_WDOG0));
    //
    // Check to see if the registers are locked, and if so, unlock them.
    //
    if(WatchdogLockState(WATCHDOG0_BASE) == true)
    {
    WatchdogUnlock(WATCHDOG0_BASE);
    }
    //
    // Initialize the watchdog timer.
    //
    WatchdogReloadSet(WATCHDOG0_BASE, 0x00000F);
    //
    // Enable the reset.
    //
    WatchdogResetEnable(WATCHDOG0_BASE);
}

void resetLaunch() {
    //
    // Enable the watchdog timer.
    //
    WatchdogEnable(WATCHDOG0_BASE);
    //
}

bool resetIsByWatchdog() {
    uint32_t cause = SysCtlResetCauseGet();
    SysCtlResetCauseClear(cause);
    bool is = (cause == SYSCTL_CAUSE_WDOG0);
    return is;
}

