

#ifndef PN_ALARM_H
#define PN_ALARM_H


/*****************************************************************************/
/* defines/constants */
/*****************************************************************************/

/* alarm types */
#define PN_ALARM_TYPE_DIAG                      0x0001
#define PN_ALARM_TYPE_PROCESS                   0x0002
#define PN_ALARM_TYPE_PULL                      0x0003
#define PN_ALARM_TYPE_PLUG                      0x0004
#define PN_ALARM_TYPE_RETURN_OF_SUBMODULE       0x000B
#define PN_ALARM_TYPE_DIAG_DISP                 0x000C

/* Alarm PROFINET IDs (Table FrameID range 8) */
#define PN_ALARM_ID_MAX                         2
#define PN_ALARM_PRIO_HIGH                      0
#define PN_ALARM_PRIO_LOW                       1
#define PN_ALARM_PRIO_MAX                       2
#define PN_ALARM_FRAME_ID_HIGH                  0xFC01
#define PN_ALARM_FRAME_ID_LOW                   0xFE01

/* alarm sequence initializers */
#define PN_ALARM_APMX_SEQ_MASK                  0x7fff
#define PN_ALARM_APMX_SEQ_INIT                  0xffff
#define PN_ALARM_ALPMX_SEQ_MASK                 0x07FF
#define PN_ALARM_ALPMX_SEQ_INIT                 0x0000
#define PN_ALARM_ALPMX_SEQ_MAX                  0x0800
#define PN_ALARM_ALPMX_SEQ_DIFF_MIN             0x0001
#define PN_ALARM_ALPMX_SEQ_DIFF_MAX             0x0780

/* user struct ids */
#define PN_ALARM_DIAG_STRUCT_ID                 0x8000

#define PN_ALARM_BLOCK_TYPE                     0x0103
#define PN_ALARM_BLOCK_LEN                      22
#define PN_ALARM_TYPE                           0x0001

#define PN_ALARM_TRTYPE(a)                      ((0x02 & ((const Unsigned8 *) &(a)->alarmCrProperties_be32)[3]) >> 1)
#define PN_ALARM_PRIOH(a)                       (PN_be16toh_p(&(a)->alarmCrTagHeaderHigh_be16) >> 13)
#define PN_ALARM_PRIOL(a)                       (PN_be16toh_p(&(a)->alarmCrTagHeaderLow_be16) >> 13)

#define PN_ALARM_HDR_SIZE                       12

/* PDUType.Type */
#define PN_ALARM_PDU_TYPE_MASK                  0x0f
#define PN_ALARM_PDU_TYPE_DATA                  1
#define PN_ALARM_PDU_TYPE_NAK                   2
#define PN_ALARM_PDU_TYPE_ACK                   3
#define PN_ALARM_PDU_TYPE_ERR                   4

/* PDUType.Version */
#define PN_ALARM_PDU_VER                        (0x01 << 4)
#define PN_ALARM_PDU_VER_MASK                   (0x0f << 4)

/* AddFlags */
#define PN_ALARM_ADDFLAGS_WINDOWSIZE            0x01
#define PN_ALARM_ADDFLAGS_WINDOWSIZE_MASK       0x0f
#define PN_ALARM_ADDFLAGS_TACK_IMM_MASK         (1 << 4)
#define PN_ALARM_ADDFLAGS_TACK_IMM_FALSE        (0 << 4)
#define PN_ALARM_ADDFLAGS_TACK_IMM_TRUE         (1 << 4)

/* SendSeqNum */
#define PN_ALARM_SENDSEQNUM_INIT                0xfffe

/* AckSeqNum */
#define PN_ALARM_ACKSEQNUM_INIT                 0xfffe

/* VarPartLen */
#define PN_ALARM_VARPARTLEN_ERR_RTA             4


/*****************************************************************************/
/* data types */
/*****************************************************************************/

/**< Alarm CR Block Request */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T blockHeader;                 /**< BlockHeader */
    Unsigned16 alarmCrType_be16;                /**< AlarmCRType */
    Unsigned16 lt_be16;                         /**< Length/Type */
    Unsigned32 alarmCrProperties_be32;          /**< AlarmCRProperties */
    Unsigned16 rtaTimeoutFactor_be16;           /**< RTATimeoutFactor */
    Unsigned16 rtaRetries_be16;                 /**< RTARetries */
    Unsigned16 localAlarmReference_be16;        /**< LocalAlarmReference */
    Unsigned16 maxAlarmDataLength_be16;         /**< MaxAlarmDataLength */
    Unsigned16 alarmCrTagHeaderHigh_be16;       /**< AlarmCRTagHeaderHigh */
    Unsigned16 alarmCrTagHeaderLow_be16;        /**< AlarmCRTagHeaderLow */
} PN_CC_PACKED PN_ALARM_CR_BLOCK_REQ_T;


/**< Alarm CR Block Response */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T blockHeader;                 /**< BlockHeader */
    Unsigned16 alarmCrType_be16;                /**< AlarmCRType */
    Unsigned16 localAlarmReference_be16;        /**< LocalAlarmReference */
    Unsigned16 maxAlarmDataLength_be16;         /**< MaxAlarmDataLength */
} PN_CC_PACKED PN_ALARM_CR_BLOCK_RES_T;


/**< Alarm Frame */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 dstAlarmEndpoint_be16;           /**< DestinationServiceAccessPoint */
    Unsigned16 srcAlarmEndpoint_be16;           /**< SourceServiceAccessPoint */
    Unsigned8 pduType;                          /**< PDUType */
    Unsigned8 addFlags;                         /**< AddFlags */
    Unsigned16 sendSeqNum_be16;                 /**< SendSeqNum */
    Unsigned16 ackSeqNum_be16;                  /**< AckSeqNum */
    Unsigned16 varPartLen_be16;                 /**< VarPartLen */
} PN_CC_PACKED PN_ALARM_FRAME_T;


/**< AlarmNotification-PDU */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T blockHeader;                 /**< BlockHeader */
    Unsigned16 alarmType_be16;                  /**< AlarmType */
    Unsigned32 api_be32;                        /**< API */
    Unsigned16 slotNumber_be16;                 /**< SlotNumber */
    Unsigned16 subSlotNumber_be16;              /**< SubslotNumber */
    Unsigned32 moduleIdentNumber_be32;          /**< ModuleIdentNumber */
    Unsigned32 subModuleIdentNumber_be32;       /**< SubmoduleIdentNumber */
    Unsigned16 alarmSpecifier_be16;             /**< AlarmSpecifier */
    Unsigned16 userStructIdentifier_be16;       /**< UserStructureIdentifier */
} PN_CC_PACKED PN_ALARM_NOTIFY_PDU_T;


/**< AlarmAck-PDU */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T blockHeader;                 /**< BlockHeader */
    Unsigned16 alarmType_be16;                  /**< AlarmType */
    Unsigned32 api_be32;                        /**< API */
    Unsigned16 slotNumber_be16;                 /**< SlotNumber */
    Unsigned16 subSlotNumber_be16;              /**< SubslotNumber */
    Unsigned16 alarmSpecifier_be16;             /**< AlarmSpecifier */
    ERROR_STATUS_T pnioStatus;                  /**< PNIOStatus */
} PN_CC_PACKED PN_ALARM_NOTIFY_ACK_PDU_T;


/**< VendorDeviceErrorInfo */
typedef PN_CC_PACKED_PRE struct {
    ERROR_STATUS_T pnioStatus;                  /**< PNIOStatus */
    Unsigned16 vendorId_be16;                   /**< VendorID */
    Unsigned16 deviceId_be16;                   /**< DeviceID */
} PN_CC_PACKED PN_ALARM_VENDEVERRINF_T;


/**< AlarmNotification User Structure */
typedef struct PN_ALARM_NOTIFY_T {
    Unsigned16 alarmType;                       /**< AlarmType */
    Unsigned32 api;                             /**< API */
    Unsigned16 slot;                            /**< SlotNumber */
    Unsigned16 subSlot;                         /**< SubslotNumber */
    Unsigned32 modId;                           /**< ModuleIdentNumber */
    Unsigned32 subModId;                        /**< SubmoduleIdentNumber */
    Unsigned16 alarmSpec;                       /**< AlarmSpecifier */
    Unsigned16 seqNr;                           /**< SequenceNumber */

    Unsigned16 userStructId;                    /**< UserStructureIdentifier */
    Unsigned16 userDataLen;                     /**< UserData length */
    Unsigned8 *pUserData;                       /**< UserData */
} PN_ALARM_NOTIFY_T;


/**< AlarmAck User Structure */
typedef struct PN_ALARM_NOTIFY_ACK_T {
    Unsigned16 alarmType;                       /**< AlarmType */
    Unsigned32 api;                             /**< API */
    Unsigned16 slot;                            /**< SlotNumber */
    Unsigned16 subSlot;                         /**< SubslotNumber */
    Unsigned16 alarmSpec;                       /**< AlarmSpecifier */
    Unsigned16 seqNr;                           /**< SequenceNumber */
    ERROR_STATUS_T pnioStatus;                  /**< PNIOStatus */
} PN_ALARM_NOTIFY_ACK_T;


/**< Acyclic Protocol Machine Sender States */
typedef enum {
    PN_ALARM_APMS_STATE_CLOSED = 0,              /**< uninitialized */
    PN_ALARM_APMS_STATE_OPEN,                    /**< wait for internal request */
    PN_ALARM_APMS_STATE_WTACK,                   /**< wait for transport ACK */
} PN_ALARM_APMS_STATE_T;


/**< Acyclic Protocol Machine Receiver States */
typedef enum {
    PN_ALARM_APMR_STATE_CLOSED = 0,              /**< uninitialized */
    PN_ALARM_APMR_STATE_OPEN,                    /**< wait for external request */
    PN_ALARM_APMR_STATE_WCNF,                    /**< wait for ACK confirmation */
} PN_ALARM_APMR_STATE_T;


/**< Alarm Protocol Machine Initiator States */
typedef enum {
    PN_ALARM_ALPMI_STATE_WSTART = 0,             /**< uninitialized */
    PN_ALARM_ALPMI_STATE_WALARM,                 /**< wait for local alarm notify request */
    PN_ALARM_ALPMI_STATE_WACK,                   /**< wait for alarm ACK / gen notify confirm */
} PN_ALARM_ALPMI_STATE_T;


/**< Alarm Protocol Machine Responder States */
typedef enum {
    PN_ALARM_ALPMR_STATE_WSTART = 0,             /**< uninitialized */
    PN_ALARM_ALPMR_STATE_WNOTIFY,                /**< wait for remote alarm notify request */
    PN_ALARM_ALPMR_STATE_WUSERACK,               /**< wait for alarm ACK from application */
    PN_ALARM_ALPMR_STATE_WTACK,                  /**< wait for transport ACK confirmation */
} PN_ALARM_ALPMR_STATE_T;


/**< Acyclic Protocol Machine Sender/Receiver */
typedef struct {
    struct PN_ALARM_EP_T *pEp;                  /**< alarm endpoint ptr */

    PN_ALARM_APMS_STATE_T apmsState;            /**< APMS state */
    PN_ALARM_APMR_STATE_T apmrState;            /**< APMR state */

    PN_TIMER_ID_T apmsTimer;                    /**< APMS timer ID */
    Unsigned16 rtaRetries;                      /**< RTARetries counter */

    OAL_BUFFER_T *pDataBuf;                     /**< DATA-RTA-PDU buffer */
    OAL_BUFFER_T *pAckBuf;                      /**< ACK/NAK-RTA-PDU buffer */

    Unsigned16 sendSeqCount;                    /**< Send_Seq_Count */
    Unsigned16 sendSeqCountO;                   /**< Send_Seq_CountO */

    Unsigned16 expectedSeqCount;                /**< Expected_Seq_Count */
    Unsigned16 expectedSeqCountO;               /**< Expected_Seq_CountO */
} PN_ALARM_APMX_T;


/**< Alarm Protocol Machine Initiator/Responder */
typedef struct {
    PN_ALARM_ALPMI_STATE_T alpmiState;          /**< ALPMI state */
    PN_ALARM_ALPMR_STATE_T alpmrState;          /**< ALPMR state */

    PN_ALARM_NOTIFY_T alarmNotify;              /**< AlarmNotification Storage */

    Unsigned16 alpmiSequenceNumber;             /**< ALPMI SequenceNumber */
    Unsigned16 alpmiSequenceNumberCur;          /**< ALPMI SequenceNumber Current */

    Unsigned16 alpmrSequenceNumber;             /**< ALPMR SequenceNumber */
    PN_BOOL_T alpmrSequenceNumberCheck;         /**< ALPMR SequenceNumber Check Flag */
} PN_ALARM_ALPMX_T;


/**< Alarm Endpoint */
typedef struct PN_ALARM_EP_T {
    PN_BOOL_T used;                             /**< used flag */
    PN_LOCK_T mtx;                              /**< mutex */
    AR_T *pAr;                                  /**< AR */

    OAL_BUFFER_T *pErrBuf;                      /**< persistent ERR-RTA-PDU buffer */

    Unsigned16 localAlarmEndpoint;              /**< Source-ServiceAccessPoint = AlarmEndpoint */
    Unsigned16 remoteAlarmEndpoint;             /**< Destination-ServiceAccessPoint = AlarmEndpoint */
    MACADDR_T remoteMac;                        /**< remote MAC address */

    Unsigned32 alarmCrProperties;               /**< AlarmCRProperties */
    Unsigned16 rtaTimeoutFactor_ms;             /**< RTATimeoutFactor in ms */
    Unsigned16 rtaRetries;                      /**< RTARetries */
    Unsigned16 maxAlarmDataLength;              /**< MaxAlarmDataLength */
    Unsigned16 alarmCrTagHeaderHigh;            /**< AlarmCRTagHeaderHigh */
    Unsigned16 alarmCrTagHeaderLow;             /**< AlarmCRTagHeaderLow */

    PN_ALARM_APMX_T apmx[PN_ALARM_ID_MAX];      /**< APMS/APMR state */
    PN_ALARM_ALPMX_T alpmx[PN_ALARM_ID_MAX];    /**< ALPMI/ALPMR state */
} PN_ALARM_EP_T;


/**< Acyclic Protocol Machine Frame State */
typedef struct {
    PN_ALARM_EP_T *pEp;                         /**< endpoint ptr */
    unsigned int prio;                          /**< priority index */
    Unsigned16 pduType;                         /**< PDUType.Type */
    Unsigned16 varPartLen;                      /**< PDU.VarPartLen */
} PN_ALARM_APM_STATE_T;


/*****************************************************************************/
/* prototypes */
/*****************************************************************************/

IOD_STATUS_T PN_alarmInit(
    void
);

IOD_STATUS_T PN_alarmOpenEp(
    AR_T *pAR,                                  /**< ptr to AR */
    const Unsigned8 *ctrlMacAddr,               /**< ptr to remote MAC address */
    const PN_ALARM_CR_BLOCK_REQ_T *pAlarmReqBlock, /**< ptr to alarm request block */
    Unsigned16 *pResLen,                        /**< ptr to store response length */
    PN_ALARM_CR_BLOCK_RES_T *pAlarmResBlock,    /**< ptr to store response */
    ERROR_STATUS_T *pErrStatus                  /**< ptr to store error code */
);

IOD_STATUS_T PN_alarmCloseEp(
    PN_ALARM_EP_T *pEp                          /**< ptr to alarm endpoint */
);

void PN_alarmRecv(
    Unsigned8 *pData,                           /**< data ptr */
    unsigned int dataLen,                       /**< data length */
    Unsigned16 pnioId,                          /**< Profinet Frame ID */
    unsigned char *pRemoteMac                   /**< remote MAC address */
);

IOD_STATUS_T PN_alarmErrSend(
    PN_ALARM_EP_T *pEp,                         /**< alarm endpoint */
    Unsigned8 errCode2,                         /**< ErrorCode2 */
    char *pVenData,                             /**< vendor data */
    unsigned int venDataLen                     /**< vendor data length */
);

IOD_STATUS_T PN_alarmApmsSend(
    PN_ALARM_EP_T *pEp,                         /**< alarm endpoint */
    unsigned int prio,                          /**< alarm priority */
    Unsigned8 pduType,                          /**< PDU type */
    Unsigned8 tack,                             /**< TACK */
    char *pVarPartData,                         /**< alarm data */
    Unsigned16 varPartLen,                      /**< alarm data length */
    char *pDataAdd,                             /**< additional alarm data */
    Unsigned16 dataAddLen                       /**< additional alarm data length */
);

IOD_STATUS_T PN_alarmSendNotify(
    AR_T *pAr,                                  /**< AR */
    unsigned int prio,                          /**< priority */
    const PN_ALARM_NOTIFY_T *pAlarmNotify,      /**< notify structure */
    Unsigned16 *pAlarmSeqNr                     /**< sequence nr ref */
);

IOD_STATUS_T PN_alarmSendNotifyAck(
    AR_T *pAr,                                  /**< AR pointer */
    unsigned int prio,                          /**< alarm priority */
    struct PN_ALARM_NOTIFY_T *pNotify,          /**< AlarmNotification data */
    ERROR_STATUS_T *pPnioStatus                 /**< PNIOStatus */
);


#endif /* PN_ALARM_H */
