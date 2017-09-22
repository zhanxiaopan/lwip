/** @file
 *
 * @brief
 * PROFINET Generic Interrupt Locking Implementation
 *
 * @details
 * This module implements locking mechanisms via interrupt disabling and
 * enabling. The target implementation must provide the two macros
 * CONFIG_TARGET_LOCK_INT_ENABLE and CONFIG_TARGET_LOCK_INT_DISABLE.
 *
 * @copyright
 * Copyright 2010-2016 port GmbH Halle/Saale.
 * This software is protected Intellectual Property and may only be used
 * according to the license agreement.
 */
#include <pn_includes.h>
#include "driverlib/rom_map.h"
#include "driverlib/interrupt.h"

/* Lock Handling Defines
 *
 * CONFIG_TARGET_LOCK_INT_ENABLE - command to enable interrupts
 * CONFIG_TARGET_LOCK_INT_DISABLE - command to disable interrupts
 */
#define CONFIG_TARGET_LOCK_INT_ENABLE MAP_IntMasterEnable()
#define CONFIG_TARGET_LOCK_INT_DISABLE MAP_IntMasterDisable()

#if !defined(CONFIG_TARGET_LOCK_INT_ENABLE) || !defined(CONFIG_TARGET_LOCK_INT_DISABLE)
#  error "Error, don't compile this file without providing interrupt handling macros."
#endif


/* variables
---------------------------------------------------------------------------*/
static volatile Unsigned32 g_lockCnt = 0;                   /**< global lock counter */

#ifdef CONFIG_DEBUG_CONTEXT_LOCKING
volatile PN_OAL_CONTEXT_T gLockContext = PN_OAL_CTX_NORMAL; /**< global setable lock context */
static volatile PN_OAL_CONTEXT_T g_lockCtx;                 /**< context of last taken lock */
#endif


/** Create a Binary or Counting Lock
 *
 * A binary lock represents a mutex, where a counting lock represents a
 * semaphore. When a counting lock is created, the inital an maximal value can
 * be set. For example if 3 instances of a ressource are available and 2 should
 * be marked as already in use, set the valInit to 2 and the valMax to 3.
 *
 * It is not necessary to set pLock->type to lockType, because this will be
 * done in pn_lock.c.
 *
 * @return IOD_OK - success
 * @return other - fail
 */
IOD_STATUS_T OAL_lockCreate(
    PN_LOCK_TYPE_T lockType,        /**< PN_LOCK_BINARY or PN_LOCK_COUNT */
    PN_LOCK_T *pLock,               /**< pointer to store the lock data */
    Unsigned32 valInit,             /**< initial lock value */
    Unsigned32 valMax               /**< maximal lock value */
)
{
    UNUSEDARG(lockType);
    UNUSEDARG(pLock);
    UNUSEDARG(valInit);
    UNUSEDARG(valMax);

    return IOD_OK;
}


/** Aquire a Lock
 *
 * Enter a binary or counting lock.
 * If this is not possible, the function will sleep until the timeout (in ms)
 * is reached. Setting the timeout to zero, will result in an infinite wait.
 *
 * Make sure the lock can be called recursively by the same thread (or when in
 * IRQ context, by the currently running IRQ). This is for example necessary,
 * because the timer handler locks the timer structure while calling the timer
 * callback and the timer callback must be able to create new or stop active
 * timers.
 *
 * @returns IOD_OK - success
 * @returns IOD_LOCK_GET_TIMEOUT_FAIL - timeout reached
 * @returns other - fail
 */
IOD_STATUS_T OAL_lockGet(
    PN_LOCK_T *pLock,               /**< lock data */
    Unsigned32 timeout              /**< lock timeout */
)
{
    UNUSEDARG(pLock);
    UNUSEDARG(timeout);

    /* timeout is not supported on this implementation */
    PN_HALT_IF_FALSE(0 == timeout);

    CONFIG_TARGET_LOCK_INT_DISABLE;

#ifdef CONFIG_DEBUG_CONTEXT_LOCKING
    if (g_lockCnt) {
        if (g_lockCtx != gLockContext) {
            OAL_halt(0);
        }
    } else {
        g_lockCtx = gLockContext;
    }
#endif

    g_lockCnt++;

    return IOD_OK;
}


/** Release a Lock
 *
 * Leave a binary or counting lock.
 *
 * @returns IOD_OK - success
 * @returns other - fail
 */
IOD_STATUS_T OAL_lockPut(
    PN_LOCK_T *pLock                /**< lock data */
)
{
    UNUSEDARG(pLock);

#ifdef CONFIG_DEBUG_CONTEXT_LOCKING
    if (g_lockCtx != gLockContext) {
        OAL_halt(0);
    }
#endif

    if (g_lockCnt > 0) {
        g_lockCnt--;
    }

    if (g_lockCnt == 0) {
        CONFIG_TARGET_LOCK_INT_ENABLE;
    }

    return IOD_OK;
}


/** Free Lock Resources
 *
 * Cleans up the locks resources.
 *
 * @returns IOD_OK - success
 * @returns other - fail
 */
IOD_STATUS_T OAL_lockDelete(
    PN_LOCK_T *pLock                /**< lock data */
)
{
    UNUSEDARG(pLock);
    return IOD_OK;
}
