

#ifndef PN_AR_H
#define PN_AR_H


/***************************************************************************/
/* constants */
/***************************************************************************/
#define AR_CAR_SINGLE                           0x0001
#define AR_SAR                                  0x0006

#define PN_AR_PROP_STATE                        (7 << 0)
#define PN_AR_PROP_SUPERVISOR_TO_ALLOW          (1 << 3)
#define PN_AR_PROP_PARAM_SERVER                 (1 << 4)
#define PN_AR_PROP_DEVICE_ACCESS                (1 << 8)
#define PN_AR_PROP_COMPANION_AR                 (2 << 9)
#define PN_AR_PROP_ACK_COMPANION_AR             (1 << 1)
#define PN_AR_PROP_STARTUP_MODE                 (1 << 30)
#define PN_AR_PROP_PULL_MOD_ALARM_ALLOW         (1 << 31)


/***************************************************************************/
/* structures */
/***************************************************************************/

/**< ARServerBlock structure */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T header;                      /**< header */
    Unsigned16 length;                          /**< station name len */
} PN_CC_PACKED PN_AR_SERVER_BLK_T;


/***************************************************************************/
/* prototypes */
/***************************************************************************/
IOD_STATUS_T AR_processArBlock(
    const AR_BLOCK_REQ_T *pArBlockReq,
    AR_BLOCK_RES_T *pArBlockRes,
    ERROR_STATUS_T *pErrorResp,
    Unsigned16 *pRespLen, AR_T **ppAR
);

PN_ALARM_EP_T * AR_getAlarmEP(
    Unsigned32 api,
    Unsigned16 slotNr,
    Unsigned16 subSlotNr
);

AR_T * AR_getAR(
    Unsigned32 api,                             /* API number */
    Unsigned16 slotNr,                          /* SubSlot number */
    Unsigned16 subslotNr                        /* Slot number */
);

IOD_STATUS_T AR_init(
    void
);

void AR_deleteAR(
    AR_T *pAR,
    Unsigned8 errCode2,
    int noAlarm
);

void AR_deleteAll(
    Unsigned8 errCode2
);

PN_BOOL_T IOD_getControllerAR(
    void
);

AR_T * AR_findAR(
    const Unsigned8 *
);

PN_BOOL_T PN_arCheckActive(
    void
);

IOD_STATUS_T PN_arGetById(
    Integer32 id,                               /**< AR id */
    AR_T **ppAr                                 /**< AR ptr ref */
);

IOD_STATUS_T PN_arGetEpByType(
    AR_T *pAr,                                  /**< AR ptr */
    Unsigned8 type,                             /**< EP type */
    IO_EP_T **ppEp                              /**< EP ptr ref */
);


#endif /* PN_AR_H */
