

#ifndef PN_LOCK_H
#define PN_LOCK_H


/*****************************************************************************/
/* Prototypes */
/*****************************************************************************/

void PN_lockCreate(
    PN_LOCK_TYPE_T lockType,        /**< PN_LOCK_BINARY or PN_LOCK_COUNT */
    PN_LOCK_T *pLock,               /**< pointer to store the lock data */
    Unsigned32 valInit,             /**< initial lock value */
    Unsigned32 valMax,              /**< maximal lock value */
    PN_USAGE_T usage                /**< usage indicator */
);

IOD_STATUS_T PN_lockGet(
    PN_LOCK_T *pLock,               /**< lock data */
    Unsigned32 timeout              /**< lock timeout */
);

void PN_lockPut(
    PN_LOCK_T *pLock                /**< lock data */
);

void PN_lockDelete(
    PN_LOCK_T *pLockId              /**< lock ID */
);

IOD_STATUS_T PN_lockGetIfTrue(
    PN_LOCK_T *pLock,               /**< lock */
    Unsigned32 timeout,             /**< lock timeout */
    volatile PN_BOOL_T *val         /**< value to check */
);

IOD_STATUS_T PN_lockGetIfFalse(
    PN_LOCK_T *pLock,                           /**< lock */
    Unsigned32 timeout,                         /**< lock timeout */
    volatile PN_BOOL_T *val                     /**< value to check */
);


#endif /* PN_LOCK_H */
