

#ifndef PN_FRAMES_H
#define PN_FRAMES_H


/* public constant definitions
---------------------------------------------------------------------------*/

/* Data Directions in Frames (U16) */
#define DATADIR_INPUT  0x0001
#define DATADIR_OUTPUT 0x0002

/* public data types
---------------------------------------------------------------------------*/
/* Block Header */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16          blockType;
    Unsigned16          blockLength;
    Unsigned8           blockVersionHigh;
    Unsigned8           blockVersionLow;
} PN_CC_PACKED BLOCK_HEADER_T;

/* Connect frame blocks
---------------------------------------------------------------------------*/

/* ?? */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T      blockHeader;
    Unsigned16          padding1;
    Unsigned8           arUUID[16];
    Unsigned16          sessionKey;
    Unsigned16          padding2;
    Unsigned16          controlCommand;
    Unsigned16          controlBlockProperties;
} PN_CC_PACKED CONTROL_BLOCK_CONNECT_T;

typedef CONTROL_BLOCK_CONNECT_T RELEASE_BLOCK_T;

/* Profinet IO Service Request PDU */
/*typedef struct {*/

/*} SERV_REQ_PDU_T;*/

/* Application relationship block request */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T      blockHeader;
    Unsigned16          type;
    Unsigned8           uuid[16];
    Unsigned16          sessionKey;
    Unsigned8           cmInitiatorMac[6];
    Unsigned8           cmInitiatorObjectUUID[16];
    Unsigned32          properties;
    Unsigned16          cmInitActToutFact; /* CM Initiator Activity Timeout Factor */
    Unsigned16          cmInitUDPRTPort;        /* CM Initiator UDP RT port */
    Unsigned16          stationNameLengh;       /* Station name length */
    Unsigned8           cmInitStationName[1];   /* CM Initiator Statio name*/
} PN_CC_PACKED AR_BLOCK_REQ_T;

/* Application relationship block response */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T      blockHeader;
    Unsigned16          type;
    Unsigned8           uuid[16];
    Unsigned16          sessionKey;
    Unsigned8           cmResponderMac[6];
    Unsigned16          cmRespUDPRTPort;        /* CM Responder UDP RT port */
} PN_CC_PACKED AR_BLOCK_RES_T;

/* IO CR Block Request - Body and List elements
---------------------------------------------------------------------------*/

/* Communication relationship block request - IO Data Object */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16          slotNr;                 /* Slot number */
    Unsigned16          subSlotNr;              /* SubSlot number */
    Unsigned16          frameOffset;            /* IO Data Object frame offset */
} PN_CC_PACKED IOCRREQ_IOD_OBJ_T;

/* Communication relationship block request - IO Consumer Status */
typedef IOCRREQ_IOD_OBJ_T IOCRREQ_IOCS_T;

/* Communication relationship block request - API */
typedef PN_CC_PACKED_PRE struct {
    Unsigned32          apiNr;                  /* AP identifier number */
    Unsigned16          numberOfIODObj;         /* Number of IO Data Objects */
    Unsigned8           ioBlocks[FLA_LENGTH];   /* IO Data Objects and IOCS Blocks */
} PN_CC_PACKED IOCRREQ_API_T;

/* Communication relationship block request - body with BlockHeader */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T      blockHeader;
    Unsigned16          iocrType;               /* IO CR type */
    Unsigned16          reference;              /* IO CR preference */
    Unsigned16          lt;                     /* Length/Type field */
    Unsigned32          properties;             /* IO CR Properties */
    Unsigned16          dataLength;             /* C_SDU length */
    Unsigned16          frameID;                /* Frame ID */
    Unsigned16          sendClockFactor;        /* Send Clock Factor */
    Unsigned16          reductionRatio;         /* Reduction Ratio */
    Unsigned16          phase;                  /* Phase */
    Unsigned16          sequence;               /* Sequence */
    Unsigned32          frameSendOffset;        /* Frame send offset */
    Unsigned16          watchdogFactor;         /* Watchdog factor */
    Unsigned16          dataHoldFactor;         /* Data hold factor */
    Unsigned16          tagHeader;              /* IO CR Tag Header */
    Unsigned8           multicastMac[6];        /* Multicast MAC address */
    Unsigned16          numberOfAPIs;           /* Number of APIs */
    Unsigned8           apiBlocks[FLA_LENGTH];  /* API blocks */
} PN_CC_PACKED IOCR_BLOCK_REQ_T;

/* IO CR Block Response */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T      blockHeader;
    Unsigned16          type;                   /* IO CR type */
    Unsigned16          reference;              /* IO CR reference */
    Unsigned16          frameID;
} PN_CC_PACKED IOCR_BLOCK_RES_T;


/* Error Response definitions
---------------------------------------------------------------------------*/

/* 4.10.2.3.6 Coding of the field PNIOStatus */

/* Everything OK */
#define ERR_CODE_OK                                          0x00
#define ERR_DECODE_OK                                        0x00
#define ERR_CODE1_OK                                         0x00
#define ERR_CODE2_OK                                         0x00

/* 6.2.5.2 Coding of the field ErrorCode */
#define ERR_CODE_RESERVED                                    0x00
#define ERR_CODE_PNIO                                        0x81
#define ERR_CODE_RTA_ERROR                                   0xCF    /* RTA Error */
#define ERR_CODE_ALARM_ACK                                   0xDA    /* Alarm ACK */
#define ERR_CODE_IOD_CONNECT_RES                             0xDB    /* IODConnectRes */
#define ERR_CODE_IOD_RELEASE_RES                             0xDC    /* IODReleaseRes */
#define ERR_CODE_IODX_CONTROL_RES                            0xDD    /* IOD-, IOX-, ControlRes */
#define ERR_CODE_IOD_READ_RES                                0xDE    /* IODReadRes */
#define ERR_CODE_IOD_WRITE_RES                               0xDF    /* IODWriteRes */

/* 6.2.5.3 Coding of the field ErrorDecode */
#define ERR_DECODE_RESERVED                                  0x00
#define ERR_DECODE_PNIORW                                    0x80    /* context with user error codes */
#define ERR_DECODE_PNIO                                      0x81    /* context with other services */

/* 6.2.5.4 Coding of the field ErrorCode1 and ErrorCode2 */
#define ERR_CODE1_RESERVED                                   0
#define ERR_CODE2_RESERVED                                   0
#define ERR_CODE2_PARAM_BLOCK_TYPE                           0
#define ERR_CODE2_PARAM_BLOCK_LENGTH                         1
#define ERR_CODE2_PARAM_BLOCK_VER_HIGH                       2
#define ERR_CODE2_PARAM_BLOCK_VER_LOW                        3

/* ErrorCode1: 1 - Connect Parameter Error: Faulty ARBlockReq */
#define ERR_CODE1_FAU_AR_BLOCK_REQ                           1

/* ErrorCode1: 2 - Connect Parameter Error: Faulty IOCRBlockReq */
#define ERR_CODE1_FAU_IOCR_BLOCK_REQ                         2
#define ERR_CODE2_SENDCLOCKFACTOR_ERROR                      10
#define ERR_CODE2_API_CONFIG_WRONG                           20
#define ERR_CODE2_SLOT_CONFIG_WRONG                          22
#define ERR_CODE2_SUBSLOT_CONFIG_WRONG                       23
#define ERR_CODE2_WRONG_IO_DATA_OBJECT_FRAME_OFFSET          24
#define ERR_CODE2_WRONG_NUMBER_OF_IOCS                       25
#define ERR_CODE2_SLOT_CONFIG_WRONG_2                        26
#define ERR_CODE2_SUBSLOT_CONFIG_WRONG_2                     27

/* ErrorCode1: 3 - Connect Parameter Error: Faulty ExpectedSubmoduleBlockReq */
#define ERR_CODE1_FAU_EXP_SUBMODULE_BLOCK_REQ                3
#define ERR_CODE2_NR_OF_API_IS_ZERO                          4
#define ERR_CODE2_API_UNSUPPORTED                            5
#define ERR_CODE2_SLOT_NR_WRONG                              6
#define ERR_CODE2_MOD_IDENT_NR_IS_ZERO                       7
#define ERR_CODE2_MOD_PROP_RESERVED_NOT_ZERO                 8
#define ERR_CODE2_NR_OF_SUB_MOD_IS_ZERO                      9
#define ERR_CODE2_SUB_SLOT_NR_WRONG                          10
#define ERR_CODE2_SUBMODULE_PROPERTIES                       12
#define ERR_CODE2_SUBMODULE_DATA_LENGTH                      14
#define ERR_CODE2_LENGTH_IOPS                                15
#define ERR_CODE2_LENGTH_IOCS                                16

/* ErrorCode1: 4 - Connect Parameter Error: Faulty AlarmCRBlockReq */
#define ERR_CODE1_FAU_ALARM_CR_BLOCK_REQ                     4
#define ERR_CODE2_ALARM_CR_TYPE                              4
#define ERR_CODE2_TRANSPORT_TYPE                             5
#define ERR_CODE2_RTA_TIMEOUT_FACTOR                         7
#define ERR_CODE2_RTA_RETRIES                                8
#define ERR_CODE2_MAX_ALARM_DATA_LENGTH                      10
#define ERR_CODE2_ALARM_CR_PRIO_HIGH                         11
#define ERR_CODE2_ALARM_CR_PRIO_LOW                          12

/* ErrorCode1: 8 - IODWriteReqHeader 鈥�request check */
#define ERR_PNIO_C1_RW_REC_PARAM_ERR                         8
#define ERR_PNIO_C2_AR_UUID_ERR                              5
#define ERR_PNIO_C2_REC_DATA_LEN                             11
#define ERR_PNIO_C2_TARGET_AR_UUID_ERR                       12

#define ERR_CODE1_RESERVED_B2                                0xB2
#define ERR_CODE1_RESERVED_B4                                0xB4
#define ERR_CODE1_RESERVED_B6                                0xB6
#define ERR_CODE1_PNIORW_ACCESS                              0xB0
#define ERR_CODE1_PNIORW_ACCESS_INVAL_IDX                    (ERR_CODE1_PNIORW_ACCESS | 0x00)
#define ERR_CODE1_PNIORW_ACCESS_INVAL_RANGE                  (ERR_CODE1_PNIORW_ACCESS | 0x07)

#define ERR_CODE2_API                                        6
#define ERR_CODE2_SLOT                                       7
#define ERR_CODE2_SUBSLOT                                    8
#define ERR_CODE2_PADDING                                    9
#define ERR_CODE2_INDEX                                      10

/* ErrorCode1: 20 - IODControl Parameter Error: Faulty ControlBlockConnect */
#define ERR_CODE1_FAU_CTRL_BLOCK_CONNECT                     20

/* ErrorCode1: 60 - AlarmAck Error Codes */
#define ERR_CODE1_ALARM_ACK                                  60
#define ERR_CODE2_ALARM_TYPE_NOT_SUPPORTED                   0

/* ErrorCode1: 64 - CMRPC (successor of the RMPM) */
#define ERR_CODE1_CMRPC                                      64
#define ERR_CODE2_OUT_OF_AR_RESOURCES                        4
#define ERR_CODE2_AR_UUID_UNKNOWN                            5
#define ERR_CODE2_OUT_OF_RESOURCES                           7

/* ErrorCode1: 253 - Used by RTA for protocol error - RTA_ERR_CLS_PROTOCOL */
#define ERR_CODE1_RTA_ERR_CLS_PROTOCOL                       253
#define ERR_CODE2_INSTANCE_CLOSED                            2       /* Instance closed - RTA_ERR_ABORT */
#define ERR_CODE2_AR_CONSUMER_DHT_WDT_EXPIRED                5       /* AR consumer DHT/WDT expired - RTA_ERR_ABORT */
#define ERR_CODE2_AR_CMI_TIMEOUT                             6
#define ERR_CODE2_AR_ABORT                                   8
#define ERR_CODE2_AR_ALARM_SEND_CONFIRM                      8
#define ERR_CODE2_AR_RELEASE_IND_RECEIVED                    15
#define ERR_CODE2_AR_PROTOCOL_VIOLATION                      18
#define ERR_CODE2_DCP_STATION_NAME_CHANGED                   31
#define ERR_CODE2_DCP_RESET_TO_FACTORY_SETTINGS              32

/* ErrorDecode PNIORW: Macros */
#define ERR_PNIORW_EC1(_class, _code) ((_class << 4) | _code)

/* ErrorDecode PNIORW: ErrorCode1 Class
 * Spec: Coding of ErrorCode1 with ErrorDecode PNIORW
 */
#define ERR_PNIORW_EC1_CLASS_APP                             10
#define ERR_PNIORW_EC1_CLASS_ACC                             11
#define ERR_PNIORW_EC1_CLASS_RES                             12

/* ErrorDecode PNIORW: ErrorCode1 Code - Application
 * Spec: Coding of ErrorCode1 with ErrorDecode PNIORW
 */
#define ERR_PNIORW_EC1_APP_READ_ERR                          0
#define ERR_PNIORW_EC1_APP_WRITE_ERR                         1
#define ERR_PNIORW_EC1_APP_MOD_FAIL                          2
#define ERR_PNIORW_EC1_APP_BUSY                              7
#define ERR_PNIORW_EC1_APP_VER_CONFLICT                      8
#define ERR_PNIORW_EC1_APP_UNSUP_FEAT                        9
#define ERR_PNIORW_EC1_ACC_INVAL_IDX                         0
#define ERR_PNIORW_EC1_ACC_WRITE_LEN_ERR                     1
#define ERR_PNIORW_EC1_ACC_INVAL_SLOT_SUBSLOT                2
#define ERR_PNIORW_EC1_ACC_TYPE_CONFLICT                     3
#define ERR_PNIORW_EC1_ACC_INVAL_AREA_API                    4
#define ERR_PNIORW_EC1_ACC_STATE_CONFLICT                    5
#define ERR_PNIORW_EC1_ACC_ACCESS_DENIED                     6
#define ERR_PNIORW_EC1_ACC_INVAL_RANGE                       7
#define ERR_PNIORW_EC1_ACC_INVAL_PARAM                       8
#define ERR_PNIORW_EC1_ACC_INVAL_TYPE                        9
#define ERR_PNIORW_EC1_ACC_BACKUP                            10
#define ERR_PNIORW_EC1_RES_READ_CONSTRAIN_CONFLICT           0
#define ERR_PNIORW_EC1_RES_WRITE_CONSTRAIN_CONFLICT          1
#define ERR_PNIORW_EC1_RES_RESOURCE_BUSY                     2
#define ERR_PNIORW_EC1_RES_RESOURCE_UNAVAIL                  3


/* Expected Submodules Block - Body and List elements 
---------------------------------------------------------------------------*/

/* Submodule properties */
#define SUBMOD_PROP_TYPE_IO 0x0003

/* Expected Submodule Block Request - Submodule Data Description */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16          dataDirection;
    Unsigned16          dataLength;
    Unsigned8           lengthIOCS;
    Unsigned8           lengthIOPS;
} PN_CC_PACKED EXPSUB_DATADESC_T;

/* Expected Submodule Block Request - Submodules */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16          subSlotNr;
    Unsigned32          subModIdentNr;
    Unsigned16          subModProp;
} PN_CC_PACKED EXPSUB_SUBMOD_T;

/* Expected Submodule Block Request - Related API */
typedef PN_CC_PACKED_PRE struct {
    Unsigned32          api;
    Unsigned16          slotNr;
    Unsigned32          modIdentNr;
    Unsigned16          modProperties;
    Unsigned16          nrOfSubMod;
} PN_CC_PACKED EXPSUB_API_T;

/* Expected Submodule Block Request - Body with BlockHeader */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T      blockHeader;
    Unsigned16          nrOfAPIs;
} PN_CC_PACKED EXP_SUBMOD_BLOCK_T;

/* Module Diff Block - Body and List elements 
---------------------------------------------------------------------------*/

/* Module Diff Block Response - SubModules */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16          subSlotNr;
    Unsigned32          subModIdentNr;
    Unsigned16          subModState;
} PN_CC_PACKED MODDIFF_SUBMOD_T;

/* Module Diff Block Response - Modules */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16          slotNr;
    Unsigned32          modIdentNr;
    Unsigned16          modState;
    Unsigned16          nrOfSubMod;
    MODDIFF_SUBMOD_T    subMod[FLA_LENGTH];
} PN_CC_PACKED MODDIFF_MOD_T;

/* Module Diff Block Response - API */
typedef PN_CC_PACKED_PRE struct {
    Unsigned32          api;
    Unsigned16          nrOfModules;
    MODDIFF_MOD_T       modules[FLA_LENGTH];
} PN_CC_PACKED MODDIFF_API_T;

/* Module Diff Block Response - Body with Block Header */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T      blockHeader;
    Unsigned16          nrOfAPIs;
    MODDIFF_API_T       api[FLA_LENGTH];
} PN_CC_PACKED MOD_DIFF_BLOCK_T;

/* IOD/IOX Block Request */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T      blockHeader;
    Unsigned16          reserved1;
    Unsigned8           arUUID[16];
    Unsigned16          sessionKey;
    Unsigned16          reserved2;
    Unsigned16          controlCmd;             /* Control command */
    Unsigned16          cntrlBlockProp; /* Control block properties */
} PN_CC_PACKED IO__BLOCK_RE__T;

typedef IO__BLOCK_RE__T IOX_BLOCK_REQ_T;
typedef IO__BLOCK_RE__T IOD_BLOCK_REQ_T;
typedef IO__BLOCK_RE__T IOX_BLOCK_RES_T;
typedef IO__BLOCK_RE__T IOD_BLOCK_RES_T;

/**
 * Expected Ident Data for 1 subslot 4/4 - SubSlot
 */
typedef PN_CC_PACKED_PRE struct {
        Unsigned16 subSlotNr;                   /* Subslot Number */
        Unsigned32 subModuleIdentNr;            /* SubModule Ident Number */
} PN_CC_PACKED EXP_IDENT_DATA_SUBSLOT_T;

/**
 * Expected Ident Data for 1 subslot 3/4 - Slot
 */
typedef PN_CC_PACKED_PRE struct {
        Unsigned16 slotNr;                      /* Slot number */
        Unsigned32 moduleIdentNr;               /* Module Ident Number */
        Unsigned16 nrOfSubSlots;                /* Number of sub slots */
        EXP_IDENT_DATA_SUBSLOT_T expIdentDataSubslot;
} PN_CC_PACKED EXP_IDENT_DATA_SLOT_T;

/**
 * Expected Ident Data for one subslot/slot/AR
 */
typedef PN_CC_PACKED_PRE struct {
        BLOCK_HEADER_T header;          /* block header*/
        Unsigned16 nrOfApis;            /* Number Of APIs */
        Unsigned32 api;                 /* API */
        Unsigned16 nrOfSlots;           /* number of slots */
        EXP_IDENT_DATA_SLOT_T expIdentDataSlot;
} PN_CC_PACKED EXP_IDENT_DATA_T;


/* IOD/IOX Block Response */

/* list of public functions
---------------------------------------------------------------------------*/

/* list of public variables
---------------------------------------------------------------------------*/

#endif
