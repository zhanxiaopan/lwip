

#ifndef PN_TYPES_H
#define PN_TYPES_H

/* OAL_BUFFER_T */
#ifndef PORT_GMBH_GOAL
#  include <pn_mem.h>
#endif
#include <pn_rpc_types.h>


/* public constant definitions
---------------------------------------------------------------------------*/
#define IPV4_ADDR_LEN                     4
#define MAC_ADDR_LEN                      6
#define IFACE_NAME_LEN                    16
#define NVS_NAME_LEN                      255

#define IOD_EPIO_IN                       1
#define IOD_EPIO_OUT                      2
#define IOD_EPALARM                       3

#define IOD_PNIO_PROTO                    1
#define IOD_IP_PROTO                      1

#define PN_LEN_STRTERM                    1

#define PN_LEN_HWREV                      sizeof(Unsigned16)
#define PN_LEN_SWREV_PREFIX               sizeof(Unsigned8)
#define PN_LEN_SWREV_ENH                  sizeof(Unsigned8)
#define PN_LEN_SWREV_BUGFIX               sizeof(Unsigned8)
#define PN_LEN_SWREV_INTCHG               sizeof(Unsigned8)
#define PN_LEN_SWREV_REVCNT               sizeof(Unsigned16)
#define PN_LEN_PROFILE_ID                 sizeof(Unsigned16)
#define PN_LEN_PROFILE_TYPE               sizeof(Unsigned16)
#define PN_LEN_ORDER_ID                   20
#define PN_LEN_SERIAL_NR                  16

#define PN_LEN_IM2_DASH                   1
#define PN_LEN_IM2_BLANK                  1
#define PN_LEN_IM2_COLON                  1
#define PN_LEN_IM2_YEAR                   4
#define PN_LEN_IM2_MONTH                  2
#define PN_LEN_IM2_DAY                    2
#define PN_LEN_IM2_HOUR                   2
#define PN_LEN_IM2_MINUTE                 2


/* Block types*/
#define IOD_BLOCK_ALARM_HIGH              0x0001
#define IOD_BLOCK_ALARM_LOW               0x0002
#define IOD_BLOCK_ACK_HIGH                0x8001
#define IOD_BLOCK_ACK_LOW                 0x8002

#define IOD_MAX_IO_DATA_LEN               1440

#ifndef NULL
#  define NULL                            ((void *) 0)
#endif

#define ARRAY_ELEMENTS(x)                 (sizeof(x) / sizeof(x[0]))

/**! Ethernet Frame Types */
#define ETH_P_PNIO                        0x8892
#define ETH_P_LLDP                        0x88CC
#define ETH_P_VLAN                        0x8100
#define ETH_P_IP                          0x0800
#define ETH_P_ARP                         0x0806


/** Max Callback Data Elements
 *
 * Highest value is used in pn_recdata:RD_processRecWrite
 */
#define IOD_CALLBACK_DATA_MAX             10


/* APDU_Status Defines */
#define PN_APDU_DS_STATE_PRIMARY                (1 << 0)
#define PN_APDU_DS_REDUNDANCY_BCK_NONE_PRIM     (1 << 1)
#define PN_APDU_DS_REDUNDANCY_PRIM_IS_BCK       (1 << 1)
#define PN_APDU_DS_DATAVALID_VALID              (1 << 2)
#define PN_APDU_DS_PROVIDERSTATE_RUN            (1 << 4)
#define PN_APDU_DS_STATION_PROBLEM_INDICATOR    (1 << 5)


/* public data types
---------------------------------------------------------------------------*/
struct AR_T;                                    /**< AR_T typedef prototype */
struct PN_ALARM_EP_T;                           /**< alarm endpoint */
struct PN_ALARM_NOTIFY_T;                       /**< alarm notification */

typedef Integer16 PN_DIAG_HANDLE_T;             /**< diagnosis handle */
typedef Unsigned8 MACADDR_T[MAC_ADDR_LEN];      /**< MAC address */
typedef Unsigned32 IPADDR_T;                    /**< IPv4 address */


/**< Ethernet Header */
typedef PN_CC_PACKED_PRE struct {
    MACADDR_T destMac;                          /**< destination MAC */
    MACADDR_T srcMac;                           /**< source MAC */
    Unsigned16 type_be16;                       /**< EtherType / VLAN TPID */
} PN_CC_PACKED PN_NET_ETH_HDR_T;


/**< VLAN Header */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 tci_be16;                        /**< TCI (PCP, DEI, VID) */
    Unsigned16 type_be16;                       /**< EtherType */
} PN_CC_PACKED PN_NET_VLAN_HDR_T;


/**< PROFINET Header */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 frameId_be16;                    /**< PROFINET FrameID */
} PN_CC_PACKED PN_NET_PNIO_HDR_T;


/**< CREP */
typedef struct {
    Unsigned16 crep;                            /**< CREP */
    MACADDR_T remoteMac;                        /**< remote MAC */
    IPADDR_T remoteIP;                          /**< remote IP */
    Unsigned16 vlanID;                          /**< VLAN ID */
    Unsigned16 frameID;                         /**< frame ID */
    Unsigned8 type;                             /**< type */
    Unsigned8 state;                            /**< state */
} IOD_CREP_T;


/*! IO Data Object type */
typedef enum {
    IO_CR_TYPE_NONE,            /*! No IO CR active */
    IO_CR_TYPE_INPUT,           /*! IO Data Object */
    IO_CR_TYPE_OUTPUT,          /*! IO consumer status */
    IO_CR_TYPE_END              /*! -enum end marker- */
} IO_CR_TYPE_T;


/**< APDU_Status */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 cycleCounter_be16;               /**< CycleCounter */
    Unsigned8 dataStatus;                       /**< DataStatus */
    Unsigned8 transferStatus;                   /**< TransferStatus */
} PN_CC_PACKED PN_APDU_STATUS_T;


typedef struct
{
    struct AR_T     *pAR;               /**< pointer to AR_T structure */
    PN_TIMER_ID_T   tid;                /**< timer ID */
    PN_LOCK_T       mutex;              /**< lock */

    PN_APDU_STATUS_T apduStatus;        /**< APDUStatus */
    unsigned int    apduStatusOfs;      /**< APDUStatus offset */
    Unsigned16      cycleCounter;       /**< CycleCounter */

    IOD_CREP_T      base;               /**< communication relation */
    Unsigned32      frameSendOff;       /**< frame send offset */
    Unsigned16      dataLen;            /**< data length */
    Unsigned16      redRatio;           /**< reduction ratio */
    Unsigned16      phase;              /**< phase */
    Unsigned16      sendClkFactor;      /**< send clock factor */
    Unsigned16      dataHoldFactor;     /**< data hold factor */
    Unsigned32      timerVal;           /**< timer value */

    OAL_BUFFER_T    *pBuf;              /**< pointer to memory buffer */
    QUEUE_T         *pQueue;            /**< CRT specific memory queue */
} IO_EP_T;


typedef enum {
    IOD_EP_CLOSED = 0,
    IOD_EP_OPEN,
    IOD_EP_WACK1,
    IOD_EP_WACK2,
    IOD_EP_READY,
    IOD_EP_ACTIVE,
    IOD_EP_TIMEOUT
} IOD_EP_STATE_T;


typedef enum {
    RESP_FILLED,
    RESP_EMPTY,
    RESP_ERROR
} RESP_RET_T;

/*! Data directions */
typedef enum {
    PN_NO_IO,       /*! no direction */
    PN_INPUT,       /*! input */
    PN_OUTPUT,      /*! output */
    PN_IO,          /*! input & output */
    PN_DATA_DIR_END /*! -enum end marker- */
} DATA_DIR_T;

/*! Frame transmit notification events */
typedef enum {
    PN_FRAME_TX,        /*! PNIO frame was submitted for the transmission */
    PN_FRAME_RX         /*! PNIO frame was received */
} FRAME_TX_EVENT_T;

/*! AR state machine states */
typedef enum {
    CMDEV_W_CIND = 0,   /*! initial state (free),waiting for connect request */
    CMDEV_W_PIND,       /*! ConnectReq received, processing WriteReq, waiting for ControlReq ParamEnd */
    CMDEV_W_PIND_IOACT, /*! ConnectReq and 1st WriteReq received -> IOEP active processing WriteReq, waiting for ParamEnd */
    CMDEV_W_ARDY,       /*! ParamEnd received, waiting for ApplReady */
    CMDEV_W_RDYC,       /*! ApplReady sent, waiting for Response */
    CMDEV_DATA          /*! ApplReadyRes received, New Cyclic Data Indicated */
} CMDEV_STATE_T;

/*! Communication Relationship type */
typedef enum {
    IO_CR,      /*! IO Communication relationship */
    ALARM_CR    /*! Alarm Communication relationship */
} CR_TYPE_T;

/*! Application relationship state */
typedef enum  {
    CR_UNUSED,          /*! CR free to use */
    CR_CONNECTED        /*! CR used */
} CR_STATE_T;

/*! Application Relationship search key */
typedef enum {
    AR_UUID,    /*! UUID of application relationship */
    AR_EPPTR    /*! pointer to the AR endpoint */
} AR_KEY_T;

/*! Application Relationship */
typedef struct AR_T {
    Integer32       id;                                     /*! internal AR id */
    CMDEV_STATE_T   state;                                  /*! CMDEV state machine state */
    Unsigned16      type;                                   /*! normal/supervisor */
    Unsigned8       arUUID[AR_UUID_SIZE];                   /*! AR identifier from IO Controller */
    Unsigned16      sessionKey;                             /*! session key from IO Controller */
    Unsigned8       cmInitiatorMAC[MAC_ADDR_LEN];           /*! MAC from IO Controller */
    UUID_T          cmInitObjUUID;                          /*! CM Initiator Object UUID */
    Unsigned16      cmInitActToutFact;                      /*! Timeout factor (100ms base)*/
    Unsigned16      initiatorUDPport;                       /*! UDP port from IO Controller */
    char            cmInitiatorName[AR_INIT_NAME_SIZE];     /*! name of IO Controller */
    Unsigned16      nameLength;                             /*! length of initiator name */
    struct PN_ALARM_EP_T *pAlarmEP;                         /*! AR related Alarm Endpoint */
    RPC_SESSION_INFO_T *rpcSessionId;                       /*! RPC Session ID */
    Unsigned16      ioEPCount;                              /*! count of IO endpoints */
    IO_EP_T         *pIoEP[CONFIG_MAX_CR_COUNT];            /*! List of communication relationships */
    PN_TIMER_ID_T   timerID;                                /*! ID for Timeout timer */
    Unsigned32      properties;                             /*! AR properties */
} AR_T;

/*! submodule definition */
typedef struct {
    Unsigned32      identNumber;            /*! submodule identification number */
    DATA_DIR_T      dataDirection;          /*! data direction (in, out, in/out) */
    Unsigned16      inDataLength;           /*! in data length */
    Unsigned16      outDataLength;          /*! out data length */
} SUBMOD_T;

/*! List of SubModules */
typedef struct {
    Unsigned32 nrOfElements;                    /*! number of elements */
    SUBMOD_T   subMod[CONFIG_MAX_SUBMOD_COUNT]; /*! array of submodules */
} SUBMOD_LIST_T;

/*! module definition */
typedef struct {
    Unsigned32      identNumber;                /*! module identification number */
    SUBMOD_LIST_T   *pSubModList;               /*! Pointer to Submodule list */
} MODULE_T;

/*! List of Modules */
typedef struct {
    Unsigned32      nrOfElements;               /*! number of elements in array */
    MODULE_T        module[CONFIG_MAX_MODULE_COUNT]; /*! array of Modules */
} MODULE_LIST_T;

/*! state of slot */
typedef enum {
     SLOT_NO_MODULE,                            /*! No Module is plugged in slot */
     SLOT_WRONG_MODULE,                         /*! Wrong module ident number */
     SLOT_PROPER_MODULE,                        /*! Module OK, Submodule locked, wrong or missing */
     SLOT_SUBSTITUTE,                           /*! wrong but compatible module */
     SLOT_GOOD,                                 /*! correct module */
     SLOT_STATE_END                             /*! -enum end marker- */
} SLOT_STATE_T;

/*! state of subSlot */
typedef enum {
     SUB_NO_SUBMODULE,          /*! No SubModule is plugged in slot */
     SUB_WRONG_SUBMODULE,       /*! Wrong Submodule ident number */
     SUB_LOCKED_BY_IO_CONTR,    /*! SubSlot locked by other IO Controller */
     SUB_APPL_READY_PENDING,    /*! parameter fault or appl. not operating */
     SUB_SUBSTITUTE,            /*! wrong but compatible SubModule */
     SUB_GOOD,                  /*! SubModule OK */
     SUB_STATE_END              /*! -enum end marker- */
} SUBSLOT_STATE_T;

/*! Expected Module/Submodule Indicator */
typedef enum {
    IND_MOD,                            /*! indicator was called for module */
    IND_SUBMOD                          /*! indicator was called for submodule */
} IND_EXPMOD_T;

/*! IO Data Object */
typedef struct {
    IO_CR_TYPE_T        ioCrType;       /*! Type of IO CR (None/Input/Output) */
    IO_EP_T             *pIOEP;         /*! Cyclic data endpoint */
    Unsigned16          frameOffset;    /*! frame offset in cycle data buffer */
} IO_DATA_OBJ_T;


/*! IO CS/PS */
typedef struct {
    Unsigned8           val;            /**< value */
    Unsigned16          ofs;            /**< offset */
} PN_IO_XS_T;


/**< Subslot */
typedef struct {
    Unsigned16          number;                 /**< Subslot number */
    SUBMOD_T            *pSubMod;               /**< Pointer to plugged SubModule */
    SUBSLOT_STATE_T     state;                  /**< used/Unused */
    AR_T                *pAR;                   /**< pointer to AR which uses subSlot */
    IO_DATA_OBJ_T       outDataObj;             /**< IO Data Object */
    IO_DATA_OBJ_T       inDataObj;              /**< IO Data Object */

    PN_IO_XS_T          inIocs;                 /**< input IOCS */
    PN_IO_XS_T          inIops;                 /**< input IOPS */
    PN_IO_XS_T          outIocs;                /**< output IOCS */
    PN_IO_XS_T          outIops;                /**< output IOPS */
} SUBSLOT_T;


/**< Slot */
typedef struct {
    Unsigned16          number;                 /**< slot number */
    MODULE_T            *pModule;               /**< pointer to plugged module */
    SLOT_STATE_T        state;                  /**< slot state used/unused */
    AR_T                *pAR;                   /**< pointer to AR which uses slot */

    Unsigned32          subSlotCnt;             /**< subslot count */
    Unsigned32          subSlotMax;             /**< subslot count max */
    SUBSLOT_T           **ppSubSlots;           /**< subslot array pointer */
} SLOT_T;


/**< Application process */
typedef struct {
    Unsigned32          api;            /**< API number */
    Unsigned32          slotCnt;        /**< slot count */
    Unsigned32          slotMax;        /**< slot count max */
    SLOT_T              **ppSlots;      /**< slot array pointer */
} AP_T;


/**< Application process list */
typedef struct {
    Unsigned32          apCnt;          /**< AP count */
    Unsigned32          apMax;          /**< AP count max */
    AP_T                **ppAP;         /**< AP array pointer */
} AP_LIST_T;


/* Error Status struct */
typedef PN_CC_PACKED_PRE struct {
    Unsigned8 errorCode;
    Unsigned8 errorDecode;
    Unsigned8 errorCode1;
    Unsigned8 errorCode2;
} PN_CC_PACKED ERROR_STATUS_T;


/*! Profinet Stack Version Information */
typedef PN_CC_PACKED_PRE struct {
    Unsigned32 verid1;      /*! id to find version in binary */
    Unsigned32 verid2;      /*! id to find version in binary */
    Unsigned16 version;     /*! major version */
    Unsigned16 subver;      /*! minor version */
    Unsigned16 patch;       /*! patch set */
    char special[32];       /*! special tag */
    char rev[32];           /*! VCS revision */
} PN_CC_PACKED PNIO_VERSION_T;

/**! Device Configuration Generation Flag */
typedef enum {
    MANU_GEN = 0,           /*! manual generation of device tree */
    AUTO_GEN = 1            /*! create needed structure elements automatically */
} DEV_GEN_FLAG_T;


/**! Callback Data Content Structure */
typedef union {
    Integer32 i32;          /*! 32-bit signed */

    Unsigned8 *pu8;         /*! 8-bit unsigned pointer */
    Unsigned16 *pu16;       /*! 16-bit unsigned pointer */
    Unsigned32 *pu32;       /*! 32-bit unsigned pointer */
    const Unsigned8 *pcu8;  /*! const 8-bit unsigned pointer */

    Unsigned16 u16;         /*! 16-bit unsigned */
    Unsigned32 u32;         /*! 32-bit unsigned */

    PN_BOOL_T boolVal;      /*! true or false */

    AR_T *pAR;              /*! application relation pointer */
    IO_CR_TYPE_T ioCrType;  /*! IO CR type */
    const IO_EP_T *pIoEp;   /*! IO endpoint pointer */
    const IOD_CREP_T *pIoCrEp;  /*! IO CR endpoint pointer */
    IND_EXPMOD_T expMod;    /*! expected module selection */
    FRAME_TX_EVENT_T frTxEv;/*! frame TX event */
    ERROR_STATUS_T *pErrStat;   /*! error status pointer */
    struct PN_ALARM_NOTIFY_T *pAlarmNotify;     /*! alarm notification */
    struct PN_ALARM_NOTIFY_ACK_T *pAlarmNotifyAck; /*! alarm notification */
} IOD_CALLBACK_DATA_T;


/**! Callback Data Structure */
typedef struct {
    IOD_CALLBACK_DATA_T data[IOD_CALLBACK_DATA_MAX];    /*! callback data */
} IOD_CALLBACK_T;


/**< Callback IDs */
typedef enum {
    IOD_CB_ALARM_ACK_TIMEOUT,                   /**< alarm ACK timeout (APMS) */
    IOD_CB_ALARM_NOTIFY_ACK,                    /**< alarm notification ACK (ALPMI) */
    IOD_CB_ALARM_NOTIFY,                        /**< alarm notification (ALPMR) */
    IOD_CB_APPL_READY,                          /**< application ready indication */
    IOD_CB_BLINK,                               /**< blink indication */
    IOD_CB_CONNECT_FINISH,                      /**< connect finish indicator */
    IOD_CB_CONNECT_REQUEST,                     /**< connect request indicator */
    IOD_CB_CONNECT_REQUEST_EXP_START,           /**< expected submodule block start indicator */
    IOD_CB_END_OF_PARAM,                        /**< end of param indication */
    IOD_CB_END_OF_PARAM_PLUG,                   /**< plug end of param indication */
    IOD_CB_EXP_SUBMOD,                          /**< expected module/submodule indication */
    IOD_CB_FACTORY_RESET,                       /**< factory reset indication */
    IOD_CB_FRAME_TRANSMIT,                      /**< frame transmission indicator */
    IOD_CB_IO_DATA_TIMEOUT,                     /**< IO data timeout indication */
    IOD_CB_NET_IP_SET,                          /**< IP configuration update */
    IOD_CB_NEW_AR,                              /**< new AR indication */
    IOD_CB_NEW_IO_CR,                           /**< new IO CR indication */
    IOD_CB_NEW_IO_DATA,                         /**< new IO data indication */
    IOD_CB_NEW_IO_DATA_OBJ,                     /**< new IO data object indication */
    IOD_CB_PLUG_READY,                          /**< plug ready indication */
    IOD_CB_READ_RECORD,                         /**< read record indication */
    IOD_CB_RELEASE_AR,                          /**< release AR indication */
    IOD_CB_RESET_TO_FACTORY,                    /**< reset to factory indication */
    IOD_CB_STATION_NAME,                        /**< name of station changed */
    IOD_CB_WRITE_RECORD,                        /**< write record indication */
} IOD_CALLBACK_ID_T;


/**! Callback Function Define */
typedef IOD_STATUS_T (*PNA_CALLBACK_FUNC)(IOD_CALLBACK_ID_T, IOD_CALLBACK_T *);


/**! module specific logging IDs - used as array idx in pn_log.c */
typedef enum {
    PN_LOG_ID_ALARM,
    PN_LOG_ID_APPLICATION,
    PN_LOG_ID_AR,
    PN_LOG_ID_CONFIG,
    PN_LOG_ID_CONTEXT,
    PN_LOG_ID_CRTDATA,
    PN_LOG_ID_CTRLPROTO,
    PN_LOG_ID_DCP,
    PN_LOG_ID_DEVICE,
    PN_LOG_ID_DIAG,
    PN_LOG_ID_IODATA,
    PN_LOG_ID_LLDP,
    PN_LOG_ID_LMPM,
    PN_LOG_ID_LOCK,
    PN_LOG_ID_LOG,
    PN_LOG_ID_MEM,
    PN_LOG_ID_NET,
    PN_LOG_ID_OAL,
    PN_LOG_ID_PDEV,
    PN_LOG_ID_RECDATA,
    PN_LOG_ID_RECDIAG,
    PN_LOG_ID_RPC,
    PN_LOG_ID_SNMP,
    PN_LOG_ID_STAT,
    PN_LOG_ID_TCPIP,
    PN_LOG_ID_TIMER,
    PN_LOG_ID_UTILS,
} PN_LOG_ID_T;


/* list of public variables
---------------------------------------------------------------------------*/
extern UUID_T            null_id; /**< UUID with all elements set to zero */


/* Priority defines for PROFINET stack threads
 * (If not defined in OAL.)
 *
 * These defines need to be adapted to the target platform. When using a
 * platform which doesn't support priorities or threads, just leave them
 * defined as they are or set them to one.
 *
 * Please set the priority as the threads are listed here. First listed thread
 * should have the highest priority in the system.
 *
 * OAL_THREAD_PRIO_TIMER   - timer thread (also sends out RT frames)
 * OAL_THREAD_PRIO_RT_RX   - LMPM realtime frame receive thread
 * OAL_THREAD_PRIO_RPC_RX  - RPC server thread
 * OAL_THREAD_PRIO_RPC_TX  - RPC responder thread
 * OAL_THREAD_PRIO_LLDP_RX - LLDP receive thread for neighborhood detection
 * OAL_THREAD_PRIO_MAIN    - main thread (main loop, user application)
 *
 * We only compare one priority and assume that if this is not set, all aren't
 * set.
 */
#ifndef OAL_THREAD_PRIO_TIMER
#  define OAL_THREAD_PRIO_TIMER   1
#  define OAL_THREAD_PRIO_RT_RX   1
#  define OAL_THREAD_PRIO_RPC_RX  1
#  define OAL_THREAD_PRIO_RPC_TX  1
#  define OAL_THREAD_PRIO_LLDP_RX 1
#  define OAL_THREAD_PRIO_MAIN    1
#endif


#endif
