

#ifndef PN_RPC_TYPES_H
#define PN_RPC_TYPES_H

#include <pn_types_indep.h>
#include <pn_net.h>

#ifdef RPC_FRAG_SUPPORT
#define RPC_MAX_REQ_LEN        (2*1024)
#define RPC_MAX_FRAG_NUM       4
#else
#define RPC_MAX_REQ_LEN        (1524)
#endif

/* EPM defines */
#define PN_RPC_EPM_HANDLE_LEN  16
#define PN_RPC_EPM_SES_TIMEOUT 3000


/* RPC Header */
typedef PN_CC_PACKED_PRE struct
{
    Unsigned8 rpc_vers;         /* RPC protocol major version (4 LSB only)*/
    Unsigned8 ptype;            /* Packet type (5 LSB only)*/
    Unsigned8 flags1;           /* Packet flags */
    Unsigned8 flags2;           /* Packet flags */
    Unsigned8 drep[3];          /* Data representation format label */
    Unsigned8 serial_hi;        /* High byte of serial number */
    UUID_T object;              /* Object identifier */
    UUID_T if_id;               /* Interface identifier */
    UUID_T act_id;              /* Activity identifier */
    Unsigned32 server_boot;     /* Server boot time */
    Unsigned32 if_vers;         /* Interface version */
    Unsigned32 seqnum;          /* Sequence number */
    Unsigned16 opnum;           /* Operation number */
    Unsigned16 ihint;           /* Interface hint */
    Unsigned16 ahint;           /* Activity hint */
    Unsigned16 len;             /* Length of packet body */
    Unsigned16 fragnum;         /* Fragment number */
    Unsigned8 auth_proto;       /* Authentication protocol identifier */
    Unsigned8 serial_lo;        /* Low byte of serial number */
} PN_CC_PACKED PN_RPC_HDR_T;


typedef PN_CC_PACKED_PRE struct
{
    Unsigned8    ver;              /* fack pocket body version */
    Unsigned8    pad1;
    Unsigned16   window_size;      /* sender's receive window size (in pkts) */
    Unsigned32   max_tsdu;         /* largest local TPDU size */
    Unsigned32   max_frag_size;    /* largest TPDU not fragmented */
    Unsigned16   serial_num;       /* serial # of packets that include this fack */
    Unsigned16   selack_len;       /* number of elements in the selack array */
    Unsigned32   selack;           /* variable number of 32 bit selective ack bit masks */
} PN_CC_PACKED RPC_FACK_BODY_T;

typedef PN_CC_PACKED_PRE struct
{
    Unsigned8   st[4];    /* status code */
} PN_CC_PACKED RPC_FAULT_BODY_T;


/**< Network Data Representation Header */
typedef PN_CC_PACKED_PRE struct
{
    Unsigned32 amax;        /* args maximum */
    Unsigned32 alen;        /* args length */
    Unsigned32 mcnt;        /* maximum count (same value as amax) */
    Unsigned32 offset;      /* offset */
    Unsigned32 acnt;        /* actual count (same value as alen) */
} PN_CC_PACKED PN_NDR_HDR_T;


typedef PN_CC_PACKED_PRE struct
{
    Unsigned8  request;
    Unsigned8  btype;
    Unsigned16 blen;    /* block length */
    Unsigned8  verh;    /* version high */
    Unsigned8  verl;    /* version low */
    Unsigned16 sq;      /* sequence */
    UUID_T     aruuid;  /* config tool */
    Unsigned32 ap;      /* API */
    Unsigned16 sl;      /* slot */
    Unsigned16 ssl;     /* subslot */
    Unsigned16 padding;
    Unsigned16 idx;     /* index */
    Unsigned32 len;     /* data length */
    Unsigned8  padding2[24];
} PN_CC_PACKED WRITE_REQ_T;

typedef PN_CC_PACKED_PRE struct
{
    Unsigned8  request;
    Unsigned8  btype;
    Unsigned16 blen;    /* block length */
    Unsigned8  verh;    /* version high */
    Unsigned8  verl;    /* version low */
    Unsigned16 sq;      /* sequence */
    UUID_T     aruuid;  /* config tool */
    Unsigned32 ap;      /* API */
    Unsigned16 sl;      /* slot */
    Unsigned16 ssl;     /* subslot */
    Unsigned16 padding;
    Unsigned16 idx;     /* index */
    Unsigned32 len;     /* data length */
    Unsigned16 adv1;    /* add value 1 */
    Unsigned16 adv2;    /* add value 2 */
    Unsigned8  padding2[20];
} PN_CC_PACKED WRITE_RES_T;

typedef PN_CC_PACKED_PRE struct
{
    Unsigned8  request;
    Unsigned8  btype;
    Unsigned16 blen;    /* block length */
    Unsigned8  verh;    /* version high */
    Unsigned8  verl;    /* version low */
    Unsigned16 sq;      /* sequence */
    UUID_T     aruuid;  /* config tool */
    Unsigned32 ap;      /* API */
    Unsigned16 sl;      /* slot */
    Unsigned16 ssl;     /* subslot */
    Unsigned16 padding;
    Unsigned16 idx;     /* index */
    Unsigned32 len;     /* data length */
    UUID_T     targetuuid;  /* config tool */
    Unsigned8  padding2[8];
} PN_CC_PACKED READ_REQ_T;

typedef PN_CC_PACKED_PRE struct
{
    Unsigned8  request;
    Unsigned8  btype;
    Unsigned16 blen;    /* block length */
    Unsigned8  verh;    /* version high */
    Unsigned8  verl;    /* version low */
    Unsigned16 sq;      /* sequence */
    UUID_T     aruuid;  /* config tool */
    Unsigned32 ap;      /* API */
    Unsigned16 sl;      /* slot */
    Unsigned16 ssl;     /* subslot */
    Unsigned16 padding;
    Unsigned16 idx;     /* index */
    Unsigned32 len;     /* data length */
    Unsigned16 adv1;    /* add value 1 */
    Unsigned16 adv2;    /* add value 2 */
    Unsigned8  padding2[20];
} PN_CC_PACKED READ_RES_T;


typedef PN_CC_PACKED_PRE struct {
#if (0 < ETH_IP_UDP_HDR)
    Unsigned8    eth_ip_udp[ETH_IP_UDP_HDR];
#endif
    PN_RPC_HDR_T hdr;                           /**< RPC header */
    PN_NDR_HDR_T ndr;                           /**< NDR header */
    Unsigned8    data[RPC_MAX_REQ_LEN];
} PN_CC_PACKED RPC_PDU_T;


/**< RPC session info structure */
typedef struct
{
    PN_BOOL_T      used;                        /**< used flag */
    PN_BOOL_T      locked;                      /**< locked flag */
    PN_BOOL_T      keep;                        /**< keep flag (no removal flag) */
    PN_TIMESTAMP_T timeout;                     /**< timeout */

    PN_ENDIAN_T    endian;                      /**< endianness */
    UUID_T         activityUUID;                /**< activity UUID */
    Unsigned32     responseAck;                 /**< response acknowledge */
    Unsigned32     maxCount;                    /**< max count */
    Unsigned32     sequenceNumber;              /**< sequence number */

    RPC_PDU_T      msgBuf;

    Unsigned16     serialNumber;                /**< serial number */
    Unsigned16     opNum;                       /**< operation number */
    Unsigned8      state;                       /**< session object state */

    int            netChan;                     /**< connection channel handle */
    OAL_UDP_ADDR_T netAddr;                     /**< remote address */

#ifdef RPC_FRAG_SUPPORT
    Unsigned32     requestAck;                  /**< request acknowledge */
    PN_TIMER_ID_T  tFAck;                       /**< resend timer */
    Unsigned16     reqLen;                      /**< request length */
    Unsigned16     fragOffset;                  /**< fragmentation data offset */
    Unsigned8      lastFragNum;                 /**< number of last fragment */
    Unsigned8      retxCount;                   /**< retransmission counter */
#endif
} RPC_SESSION_INFO_T;


#endif /* PN_RPC_TYPES_H */
