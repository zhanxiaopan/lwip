

#ifndef PN_RPC_H
#define PN_RPC_H

#include <pn_includes.h>


/* public variables
---------------------------------------------------------------------------*/
extern UUID_T PN_RPC_interface_id_pnio;         /**< PNIO interface ID */
extern UUID_T PN_RPC_interface_id_epmv4;        /**< EPMv4 interface ID */


/* definitions
---------------------------------------------------------------------------*/
#ifndef CONFIG_RPC_NUM_SESSIONS
#  define CONFIG_RPC_NUM_SESSIONS 4
#endif

#ifdef CONFIG_RECORD_DATA_BUSY
#  define PN_RPC_MAX_PDU_LEN   (sizeof(PN_RPC_HDR_T) + PN_RPC_MAX_DATA_LEN)
#endif

/* max number of concurrent activities */
#define PN_RPC_NUM_SESSIONS    CONFIG_RPC_NUM_SESSIONS

#define RPC_GET_PKT_SIZE(l)    ((l) + sizeof(PN_RPC_HDR_T) + sizeof(PN_NDR_HDR_T))
#define RPC_GET_DATA_PTR(b)    ((Unsigned8*)((Unsigned8*)(b) + (sizeof(PN_RPC_HDR_T) + sizeof(PN_NDR_HDR_T))))

#define PN_RPC_VER             0x04
#define PN_RPC_IDEMPOT         0x20
#define PN_RPC_LITTLE_ENDIAN   0x10
#define PN_RPC_NOFACK          0x08
#define PN_RPC_FRAGM           0x04
#define PN_RPC_LASTFRAG        0x02
#define PN_RPC_IF_VER          0x00000001uL
#define PN_RPC_UUIDHINT_NO     0xFFFF
#define PN_RPC_AUTH_NO         0x00

#define PN_RPC_SRV_PORT        0x8894    /**< RPC port - 34964 (according to IANA)  */
#define PN_RPC_RSP_PORT        0xc000    /**< RPC responder port - 49152 (according to IANA) */
#define PN_RPC_EPM_SES_CNT     1         /**< count of RPC EPM sessions */
#define PN_RPC_EPM_PORT_CNT    2         /**< count of RPC EPM ports */

#define PN_RPC_HDR_OFF         ETH_IP_UDP_HDR
#define PN_RPC_SERVERBOOT_OFF  (PN_RPC_HDR_OFF + 56)
#define PN_RPC_IFVER_OFF       (PN_RPC_HDR_OFF + 60)
#define PN_RPC_SEQNO_OFF       (PN_RPC_HDR_OFF + 64)
#define PN_RPC_OPNO_OFF        (PN_RPC_HDR_OFF + 68)
#define PN_RPC_IHINT_OFF       (PN_RPC_HDR_OFF + 70)
#define PN_RPC_AHINT_OFF       (PN_RPC_HDR_OFF + 72)
#define PN_RPC_LEN_OFF         (PN_RPC_HDR_OFF + 74)
#define PN_RPC_FRAGNUM_OFF     (PN_RPC_HDR_OFF + 76)
#define PN_RPC_PDU_HANDLE_OFF  (36)

#define PN_RPC_SERV_ARR_SIZE   6         /* max number of supported operations  */
#define PN_RPC_MAX_DATA_LEN    1400u     /* max packet length (RPC body) - RPC header not included */
#define PN_RPC_MAX_FRAME_LEN   (PN_RPC_MAX_DATA_LEN + sizeof(PN_RPC_HDR_T) + sizeof(PN_NDR_HDR_T)) /* max lenth of RPC packet (with RPC header) */
#define PN_RPC_MAX_NUM_RETX    4         /* maximum number of fragment retransmissions */

/* RPC fault/reject codes */
#define PN_RPC_ST_UNSPEC       0x1c000009uL
#define PN_RPC_ST_RESPTOOBIG   0x1c010013uL
#define PN_RPC_ST_UNSUPPORTED  0x1c010017uL

#define PN_RPC_NDR_HDR_SIZE    (sizeof(PN_RPC_HDR_T) + sizeof(PN_NDR_HDR_T))

#define PN_RPC_NDR_OFF         (PN_RPC_HDR_OFF + sizeof(PN_RPC_HDR_T))
#define PN_RPC_ARGLEN_OFF      (PN_RPC_NDR_OFF + 4)
#define PN_RPC_DATA_OFF        (PN_RPC_HDR_OFF + sizeof(PN_RPC_HDR_T) + sizeof(PN_NDR_HDR_T))

#define PN_RPC_PCKT_TYPE_MASK  0x1F

#define PN_RPC_TIMEOUT_FRAG    500
#define PN_RPC_TIMEOUT_IDLE    3000

/* RPC states */
#define PN_RPC_ST_WFReq        0
#define PN_RPC_ST_WFFrg        1
#define PN_RPC_ST_WFRsp        3

#define PN_RPC_CANCEL_ACK_LEN  9

/* EPM defines */
#define PN_RPC_EPM_TYPE_EPMV4           0
#define PN_RPC_EPM_TYPE_PNIO            1
#define PN_RPC_EPM_TOWER_REF            3
#define PN_RPC_EPM_ENTRY_NO             1
#define PN_RPC_EPM_ENTRY_MAX            1
#define PN_RPC_EPM_ANNO_OFS             0
#define PN_RPC_EPM_FLOOR_MAX            5
#define PN_RPC_EPM_FLOOR_RPCID          0x0d
#define PN_RPC_EPM_FLOOR_RPCPROTOID     0x0a
#define PN_RPC_EPM_FLOOR_RPCSRVUDPPORT  0x08
#define PN_RPC_EPM_FLOOR_RPCHOSTADDR    0x09
#define PN_RPC_EPM_STATUS_OKAY          0x00000000
#define PN_RPC_EPM_STATUS_EP_NOT_REG    0x16C9A0D6


/* device annotation
---------------------------------------------------------------------------*/

/* 4.10.2.2.4 - Coding of the field SWRevisionPrefix
 *
 * V - for an officially released version
 * R - for Revision
 * P - for Prototype
 * U - for Under Test (Field Test)
 * T - for Test Device
 */
#define PN_RPC_SWREV_OFFICIAL               'V'
#define PN_RPC_SWREV_REVISION               'R'
#define PN_RPC_SWREV_PROTOTYPE              'P'
#define PN_RPC_SWREV_UNDERTEST              'U'
#define PN_RPC_SWREV_TESTDEV                'T'


/* fixed string length for annotation */
#define PN_RPC_LEN_ANNO                     64
#define PN_RPC_LEN_DEVTYPE                  25
#define PN_RPC_LEN_ORDERID                  20
#define PN_RPC_LEN_HWREV                    5
#define PN_RPC_LEN_SWREV_PREFIX             1
#define PN_RPC_LEN_SWREV_ENH                3
#define PN_RPC_LEN_SWREV_BUGFIX             3
#define PN_RPC_LEN_SWREV_INTCHG             3


/* public data types
---------------------------------------------------------------------------*/
typedef enum {
    PN_RPC_REQUEST        = 0x00,
    PN_RPC_PING           = 0x01,
    PN_RPC_RESPONSE       = 0x02,
    PN_RPC_FAULT          = 0x03,
    PN_RPC_WORKING        = 0x04,
    PN_RPC_NOCALL         = 0x05,
    PN_RPC_REJECT         = 0x06,
    PN_RPC_ACK            = 0x07,
    PN_RPC_CANCEL         = 0x08,
    PN_RPC_FACK           = 0x09,
    PN_RPC_CANCELACK      = 0x0A
} PN_RPC_PCKT_TYPE;

typedef enum {
    PN_RPC_CONNECT        = 0x00,
    PN_RPC_RELEASE        = 0x01,
    PN_RPC_READ           = 0x02,
    PN_RPC_EPMLOOKUP      = 0x02,
    PN_RPC_WRITE          = 0x03,
    PN_RPC_CONTROL        = 0x04,
    PN_RPC_EPMFREE        = 0x04,
    PN_RPC_READIMPLICIT   = 0x05
} PN_RPC_OPERATION_T;

typedef IOD_STATUS_T (*FPTR_RPCSERV_T)( Unsigned8 serviceNum,
                    RPC_SESSION_INFO_T *pSesInfo,
                    const Unsigned8  *pInData,
                    Unsigned16  inDataLen,
                    Unsigned8  *pOutData,
                    Unsigned16 *pOutDataLen,
                    ERROR_STATUS_T *pErrCode);

typedef struct
{
    PN_RPC_OPERATION_T  serviceId;
    FPTR_RPCSERV_T  pftrCB;
} RPC_SERVICEINFO_T;


/**< RPC EPM UDP port info */
typedef struct {
    int used;                                   /**< usage indicator */
    int type;                                   /**< UDP port type (EPMv4, PNIO) */
    Unsigned16 port;                            /**< UDP port */
} PN_RPC_EPM_UDP_T;


/**< RPC EPM session info */
typedef struct {
    PN_BOOL_T used;                             /**< usage indicator */
    Unsigned8 handle[PN_RPC_EPM_HANDLE_LEN];    /**< EPM handle */
    PN_TIMESTAMP_T exp;                         /**< expiration timestamp */
    unsigned int portIdx;                       /**< port index */
} PN_RPC_EPM_SES_T;


/**< RPC EPM response */
typedef PN_CC_PACKED_PRE struct {
    Unsigned32 handleAttr;                      /**< RPCEntryHandleAttribute */
    UUID_T handleUUID;                          /**< RPCEntryHandleUUID */
    Unsigned32 noOfEntries;                     /**< RPCNumberOfEntries */
    Unsigned32 maxEntries;                      /**< RPCMaxEntries */
    Unsigned32 entriesOfs;                      /**< RPCEntriesOffset */
    Unsigned32 entriesCnt;                      /**< RPCEntriesCount */
} PN_CC_PACKED PN_RPC_EPM_RES_T;


/**< RPC EPM response entry */
typedef PN_CC_PACKED_PRE struct {
    UUID_T objUUID;                             /**< RPCObjectUUID */
    Unsigned32 towerRef;                        /**< RPCTowerReference */
    Unsigned32 annoOfs;                         /**< RPCAnnotationOffset */
    Unsigned32 annoLen;                         /**< RPCAnnotationLength */
    char anno[PN_RPC_LEN_ANNO];                 /**< RPCAnnotation */
    Unsigned32 towerLen;                        /**< RPCTowerLength */
    Unsigned32 towerOctStrLen;                  /**< RPCTowerOctetStringLength */
    Unsigned16 floorCnt_le;                     /**< RPCFloorCount */
} PN_CC_PACKED PN_RPC_EPM_RES_ENTRY_T;


/**< RPC EPM Floor 1/2 LHS */
typedef PN_CC_PACKED_PRE struct {
    Unsigned8 rpcId;                            /**< RPCID */
    UUID_T rpcIfaceUUID_le;                     /**< RPCInterfaceUUID */
    Unsigned16 rpcIfaceVerMaj_le;               /**< RPCInterfaceVersionMajor */
} PN_CC_PACKED PN_RPC_EPM_FLOOR12_LHS_T;


/**< RPC EPM Floor 1/2 RHS */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 rpcIfaceVerMin_le;               /**< RPCInterfaceVersionMinor */
} PN_CC_PACKED PN_RPC_EPM_FLOOR12_RHS_T;


/**< RPC EPM Floor 3 LHS */
typedef PN_CC_PACKED_PRE struct {
    Unsigned8 rpcProtoId;                       /**< RPCProtocolIdentifier */
} PN_CC_PACKED PN_RPC_EPM_FLOOR3_LHS_T;


/**< RPC EPM Floor 3 RHS */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 rpcIfaceVerMin_le;               /**< RPCInterfaceVersionMinor */
} PN_CC_PACKED PN_RPC_EPM_FLOOR3_RHS_T;


/**< RPC EPM Floor 4 LHS */
typedef PN_CC_PACKED_PRE struct {
    Unsigned8 rpcSrvUdpPort;                    /**< RPCServerUDPPort */
} PN_CC_PACKED PN_RPC_EPM_FLOOR4_LHS_T;


/**< RPC EPM Floor 4 RHS */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 rpcPortNum;                      /**< RPCPortNumber */
} PN_CC_PACKED PN_RPC_EPM_FLOOR4_RHS_T;


/**< RPC EPM Floor 5 LHS */
typedef PN_CC_PACKED_PRE struct {
    Unsigned8 rpcHostAddr;                      /**< RPCHostAddress */
} PN_CC_PACKED PN_RPC_EPM_FLOOR5_LHS_T;


/**< RPC EPM Floor 5 RHS */
typedef PN_CC_PACKED_PRE struct {
    Unsigned32 rpcIpAddr;                       /**< RPCIPAddress */
} PN_CC_PACKED PN_RPC_EPM_FLOOR5_RHS_T;


/* list of public functions
---------------------------------------------------------------------------*/
IOD_STATUS_T RPC_init(const RPC_SERVICEINFO_T* pServices, Unsigned8 iLength);
void RPC_shutdown(void);

IOD_STATUS_T RPC_call(RPC_SESSION_INFO_T *pSesInfo, const UUID_T *pObjectID, Unsigned8 servNum,
                      const Unsigned8 *pData, Unsigned16 iDataLen);

void RPC_setAppReadyData(AR_T *, Unsigned16, Unsigned16);
void RPC_checkAutoAnswers(void);
void PN_rpcLoop(PN_TIMESTAMP_T ts);

#ifdef CONFIG_RECORD_DATA_BUSY
IOD_STATUS_T RPC_sendResponse(RPC_SESSION_INFO_T *pSesInfo, Unsigned16 respLen);
#endif /* CONFIG_RECORD_DATA_BUSY */

void PN_rpcEpmInit(void);
IOD_STATUS_T PN_rpcEpmReg(Unsigned8 type, Unsigned16 port);
Unsigned16 PN_rpcEpmReq(RPC_SESSION_INFO_T *pSesInfo, const RPC_PDU_T *pRequest);
Unsigned16 PN_rpcEpmReqFree(RPC_SESSION_INFO_T *pSesInfo, const RPC_PDU_T *pRequest);

void PN_rpcSesCtrlKeep(RPC_SESSION_INFO_T *pSession, PN_BOOL_T flag);


#endif /* PN_RPC_H */
