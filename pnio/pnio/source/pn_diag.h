

#ifndef PN_DIAG_H
#define PN_DIAG_H


/***************************************************************************/
/* constants */
/***************************************************************************/

/* diagnosis handle */
#define PN_DIAG_HANDLE_ERR                          -1

/* alarm specifiers */
#define PN_DIAG_ALARM_SPEC_CHAN_DIAG                (1 << 11)
#define PN_DIAG_ALARM_SPEC_MANU_SPEC_DIAG           (1 << 12)
#define PN_DIAG_ALARM_SPEC_SUBMOD_DIAG              (1 << 13)
#define PN_DIAG_ALARM_SPEC_AR_DIAG                  (1 << 15)

/* UserStructureIdentifier */
#define PN_DIAG_USI_CHAN                            0x8000
#define PN_DIAG_USI_CHAN_EXT                        0x8002
#define PN_DIAG_USI_CHAN_QUAL                       0x8003

/* diagnosis defines */
#define PN_DIAG_CHAN_NUM_SUBMODULE                  0x8000
#define PN_DIAG_CHAN_TYPE_DATA_TRANS_IMPOSSIBLE     0x8000
#define PN_DIAG_CHAN_TYPE_REMOTE_MISMATCH           0x8001

/* ChannelProperties.Type */
#define PN_DIAG_CHAN_PROP_TYPE_UNSET                0x00
#define PN_DIAG_CHAN_PROP_TYPE_01                   0x01
#define PN_DIAG_CHAN_PROP_TYPE_02                   0x02
#define PN_DIAG_CHAN_PROP_TYPE_04                   0x03
#define PN_DIAG_CHAN_PROP_TYPE_08                   0x04
#define PN_DIAG_CHAN_PROP_TYPE_16                   0x05
#define PN_DIAG_CHAN_PROP_TYPE_32                   0x06
#define PN_DIAG_CHAN_PROP_TYPE_64                   0x07

/* ChannelProperties.Accumulative */
#define PN_DIAG_CHAN_PROP_ACCUMULATIVE              (1 << 8)

/* ChannelProperties.Maintainance */
#define PN_DIAG_CHAN_PROP_MAINT_REQ                 (1 << 9)
#define PN_DIAG_CHAN_PROP_MAINT_DEM                 (1 << 10)
#define PN_DIAG_CHAN_PROP_MAINT_MASK                (3 << 9)

/* ChannelProperties.Specifier */
#define PN_DIAG_CHAN_PROP_SPEC_ALL_DISAPP           (0 << 11)
#define PN_DIAG_CHAN_PROP_SPEC_APPEARS              (1 << 11)
#define PN_DIAG_CHAN_PROP_SPEC_DISAPP               (2 << 11)
#define PN_DIAG_CHAN_PROP_SPEC_DISAPP_BUT_REM       (3 << 11)
#define PN_DIAG_CHAN_PROP_SPEC_MASK                 (3 << 11)

/* ChannelProperties.Direction */
#define PN_DIAG_CHAN_PROP_DIR_MANU                  (0 << 13)
#define PN_DIAG_CHAN_PROP_DIR_INPUT                 (1 << 13)
#define PN_DIAG_CHAN_PROP_DIR_OUTPUT                (2 << 13)
#define PN_DIAG_CHAN_PROP_DIR_INOUT                 (3 << 13)

/* Diagnosis: Data Transfer Impossible */
#define PN_DIAG_CHAN_EXT_TYPE_LINK_STATE_MISMATCH   0x8000
#define PN_DIAG_CHAN_EXT_TYPE_MAUTYPE_MISMATCH      0x8001

/* Diagnosis: Remote Mismatch */
#define PN_DIAG_CHAN_EXT_TYPE_STATION_NAME_MISMATCH 0x8000
#define PN_DIAG_CHAN_EXT_TYPE_PORT_NAME_MISMATCH    0x8001
#define PN_DIAG_CHAN_EXT_TYPE_NO_PEER_DETECTED      0x8005


/***************************************************************************/
/* structures */
/***************************************************************************/

/**< diagnosis structure */
typedef struct {

    PN_BOOL_T used;                             /**< usage indicator */
    Unsigned32 api;                             /**< API */
    Unsigned16 slot;                            /**< slot */
    Unsigned16 subSlot;                         /**< subslot */
    Unsigned16 usi;                             /**< user structure identifier */
    Unsigned16 chanNum;                         /**< channel number */
    Unsigned16 chanProp;                        /**< channel properties */
    Unsigned16 chanErrType;                     /**< channel error type */

    Unsigned16 extChanErrType;                  /**< ext chan err type */
    Unsigned32 extChanAddValue;                 /**< ext chan added value */

    Unsigned16 dataLen;                         /**< manufact spec data len */
    Unsigned8 data[CONFIG_MAX_DIAG_DATA_SIZE];  /**< manufact spec data */

} PN_DIAG_TYPE_T;


/* channel diagnosis data */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 chanNum;                         /**< channel number */
    Unsigned16 chanProp;                        /**< channel properties */
    Unsigned16 errorType;                       /**< error type */
} PN_CC_PACKED PN_CHANNEL_DIAGNOSIS_DATA_T;


/* extended channel diagnosis data */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 chanNum;                         /**< channel number */
    Unsigned16 chanProp;                        /**< channel properties */
    Unsigned16 errorType;                       /**< error type */
    Unsigned16 extChannelErrorType;             /**< ext chan error type */
    Unsigned32 extChannelAddValue;              /**< ext chan added value */
} PN_CC_PACKED PN_DIAG_CHAN_EXT_T;


/***************************************************************************/
/* prototypes */
/***************************************************************************/

IOD_STATUS_T PN_diagInit(
    void
);

IOD_STATUS_T PN_diagSendProcessAlarm(
    Unsigned32 api,                             /**< API number */
    Unsigned16 slot,                            /**< slot number */
    Unsigned16 subSlot,                         /**< subslot number */
    Unsigned16 userStructIdent,                 /**< user structure ID */
    Unsigned16 dataLen,                         /**< data length */
    Unsigned8 *dataPtr                          /**< data pointer */
);

IOD_STATUS_T PN_diagSendModuleAlarmHelper(
    Unsigned32 api,                             /**< API number */
    Unsigned16 slot,                            /**< slot number */
    Unsigned16 subSlot,                         /**< subslot number */
    Unsigned8 alarmType,                        /**< alarm type */
    Unsigned16 *seqNr                           /**< sequence number */
);

IOD_STATUS_T PN_diagChannelDiagNew(
    PN_CHANNEL_DIAGNOSIS_DATA_T *data,          /**< diagnosis data */
    PN_DIAG_HANDLE_T handle,                    /**< diagnosis buffer handle */
    unsigned int *pLen                          /**< length ptr */
);

IOD_STATUS_T PN_diagExtChannelDiagNew(
    PN_DIAG_CHAN_EXT_T *data,                   /**< diagnosis data */
    PN_DIAG_HANDLE_T handle,                    /**< diagnosis buffer handle */
    unsigned int *pLen                          /**< length ptr */
);

PN_DIAG_HANDLE_T PN_diagChanDiagAdd(
    Unsigned32 api,                             /**< API number */
    Unsigned16 slot,                            /**< slot number */
    Unsigned16 subSlot,                         /**< subslot number */
    Unsigned16 channel,                         /**< channel number */
    Unsigned16 errorNumber,                     /**< error number */
    PN_BOOL_T maintenanceRequired,              /**< maintainance required flag */
    PN_BOOL_T maintenanceDemanded,              /**< maintainance demanded flag */
    PN_BOOL_T submitAlarm                       /**< submit alarm flag */
);

IOD_STATUS_T PN_diagChanDiagRemove(
    PN_DIAG_HANDLE_T handle,                    /**< diagnosis handle */
    PN_BOOL_T submitAlarm                       /**< submit alarm flag */
);

PN_DIAG_HANDLE_T PN_diagExtChanDiagAdd(
    Unsigned32 api,                             /**< API number */
    Unsigned16 slot,                            /**< slot number */
    Unsigned16 subSlot,                         /**< subslot number */
    Unsigned16 channel,                         /**< channel number */
    Unsigned16 errorNumber,                     /**< error number */
    Unsigned16 extChanErrType,                  /**< extended channel error type */
    Unsigned32 extChanAddValue,                 /**< extended channel error value */
    PN_BOOL_T maintenanceRequired,              /**< maintainance required flag */
    PN_BOOL_T maintenanceDemanded,              /**< maintainance demanded flag */
    PN_BOOL_T submitAlarm                       /**< submit alarm flag */
);

PN_DIAG_HANDLE_T PN_diagGenDiagAdd(
    Unsigned32 api,                             /**< API number */
    Unsigned16 slot,                            /**< slot number */
    Unsigned16 subSlot,                         /**< subslot number */
    Unsigned16 channel,                         /**< channel number */
    Unsigned16 userStructIdent,                 /**< user structure ID */
    Unsigned16 dataLen,                         /**< data length */
    Unsigned8 *dataPtr,                         /**< data pointer */
    PN_BOOL_T maintenanceRequired,              /**< maintainance required flag */
    PN_BOOL_T maintenanceDemanded,              /**< maintainance demanded flag */
    PN_BOOL_T submitAlarm                       /**< submit alarm flag */
);

PN_DIAG_HANDLE_T PN_diagGetFirstEntry(
    void
);

PN_DIAG_HANDLE_T PN_diagGetNextEntry(
    PN_DIAG_HANDLE_T diagHandle                 /**< diagnosis handle to start from */
);

PN_DIAG_TYPE_T * PN_diagGetDiagBufPtr(
    PN_DIAG_HANDLE_T handle                     /**< diagnosis handle */
);

void PN_diagClearSubSlot(
    const Unsigned32 api,                       /**< API */
    const Unsigned16 slot,                      /**< Slot */
    const Unsigned16 subSlot                    /**< Subslot */
);

IOD_STATUS_T PN_diagPdPortData(
    void
);

IOD_STATUS_T PN_diagAvail(
    const Unsigned32 api,                       /**< API */
    const Unsigned16 slot,                      /**< slot */
    const Unsigned16 subSlot,                   /**< subslot */
    Unsigned16 *pState                          /**< submodule state */
);


#endif /* PN_DIAG_H */
