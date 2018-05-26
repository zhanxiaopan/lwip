

#ifndef PN_TYPES_INDEP_H
#define PN_TYPES_INDEP_H

/* include portable type definitions */
#include <pn_oal_types.h>


/**< timer specific defines */
#define PN_TIMER_INVALID                0
#define PN_TIMER_FUNC(f,p)              void f(void *p)
typedef Unsigned32 PN_TIMER_ID_T;
typedef void (*PN_TIMER_CB_T)(void*);

typedef enum {
    PN_TIMER_SINGLE = 0,                /**< single shot timer */
    PN_TIMER_PERIODIC = 1               /**< periodic timer */
} PN_TIMER_TYPE_T;


typedef enum {                          /**< timer priorities */
    PN_TIMER_LOW = 0,
    PN_TIMER_HIGH = 1
} PN_TIMER_PRIO_T;


/**< 64 bit timestamp */
typedef struct {
    Unsigned64 ts;                      /**< timestamp */
} PN_TIMESTAMP_T;


typedef enum {
    PN_NET_TX_LOW = 0,                  /**< low priority */
    PN_NET_TX_LLDP,                     /**< LLDP */
    PN_NET_TX_RT,                       /**< real-time data */
} PN_NET_TX_TYPE_T;


/**< boolean definition */
#ifdef PORT_GMBH_GOAL
#  define PN_FALSE GOAL_FALSE
#  define PN_TRUE GOAL_TRUE
#  define PN_BOOL_T GOAL_BOOL_T
#else
typedef enum {
    PN_FALSE = 0,
    PN_TRUE = 1
}
PN_BOOL_T;
#endif


/**< byte specific UUID definition */
typedef PN_CC_PACKED_PRE struct {
    Unsigned8 data1[4];
    Unsigned8 data2[2];
    Unsigned8 data3[2];
    Unsigned8 data4[8];
} PN_CC_PACKED UUID_T;


/**< type specific UUID definition */
typedef struct
{
    Unsigned32 data1;
    Unsigned16 data2;
    Unsigned16 data3;
    Unsigned8  data4[8];
}
UUID_TYPE_T;


/**< endian type */
typedef enum
{
    PN_ENDIAN_LITTLE = 0,
    PN_ENDIAN_BIG
}
PN_ENDIAN_T;


/**< usage types */
typedef enum {
    PN_USAGE_UNKNOWN = 0,           /**< unknown usage */
    PN_USAGE_ALARM_ACK,             /**< alarm: ACK/NACK-RTA-PDU */
    PN_USAGE_ALARM_DATA,            /**< alarm: DATA-RTA-PDU */
    PN_USAGE_ALARM_ERR,             /**< alarm: ERR-RTA-PDU */
    PN_USAGE_ALARM_SUBMIT,          /**< alarm: submit new alarm */
    PN_USAGE_ART_ALARM,             /**< alarm: submit new alarm */
    PN_USAGE_ART_PDU,               /**< alarm: send PDU */
    PN_USAGE_CRT_EP,                /**< crtdata: endpoint */
    PN_USAGE_CRT_EP_INPUT,          /**< crtdata: input endpoint */
    PN_USAGE_CRT_EP_OUTPUT,         /**< crtdata: output endpoint */
    PN_USAGE_CRT_EP_CLEANUP,        /**< crtdata: endpoint cleanup */
    PN_USAGE_CRT_EP_TRANSMIT,       /**< crtdata: endpoint transfer */
    PN_USAGE_CONFIG_NVS,            /**< config: global NVS buffer */
    PN_USAGE_DCP_RESPBUF,           /**< dcp: response buffer lock */
    PN_USAGE_DCP_IDENTIFY_RESPONSE, /**< dcp: identify response buffer */
    PN_USAGE_DCP_GET_RESPONSE,      /**< dcp: get response buffer */
    PN_USAGE_DCP_SET_RESPONSE,      /**< dcp: set response buffer */
    PN_USAGE_DCP_HELLO_RESPONSE,    /**< dcp: hello response buffer */
    PN_USAGE_LLDP,                  /**< lldp: buffer */
    PN_USAGE_MEM_POOL,              /**< mem: buffer pool */
    PN_USAGE_MEM_RXBUF,             /**< mem: rx buffer pool */
    PN_USAGE_MEM_TXBUF,             /**< mem: tx buffer pool */
    PN_USAGE_NET_SLOTS,             /**< net: slots lock */
    PN_USAGE_RPC_LOCK,              /**< rpc: data structure lock */
    PN_USAGE_RPC_EPM_LOCK,          /**< rpc epm: session lock */
    PN_USAGE_TIMER,                 /**< timer: data structure */
    PN_USAGE_OAL_ETH,               /**< oal: ethernet handling */
    PN_USAGE_OAL_ETH_RECV,          /**< oal: ethernet receive buffer */
    PN_USAGE_OAL_ETH_SEND,          /**< oal: ethernet send buffer */
    PN_USAGE_OAL_ETH_SEND_OS,       /**< oal: OS frame in ethernet send */
    PN_USAGE_OAL_ETH_SEND_PNIO,     /**< oal: PROFINET frame in ethernet send */
    PN_USAGE_OAL_RAW_SEND,          /**< oal: raw frame send */
    PN_USAGE_OAL_RAW_RECV,          /**< oal: raw frame receive */
    PN_USAGE_OAL_UDP_RECV,          /**< oal: UDP frame receive */
    PN_USAGE_OAL_UDP_SEND,          /**< oal: UDP frame send */
    PN_USAGE_OAL_UDP_SLOTS,         /**< oal: UDP slots lock */
    PN_USAGE_TCPIP_SEND,            /**< net: UDP frame send */
    PN_USAGE_LOG,                   /**< log: log buffer lock */
    PN_USAGE_LOG_SYSLOG,            /**< log: syslog buffers */
    PN_USAGE_LMPM_CYCLIC,           /**< lmpm: cyclic data */
    PN_USAGE_SNMP_SEND,             /**< snmp: send buffer */
#ifdef CONFIG_UIP_TCP_DELAYED_ACK
    PN_USAGE_UIP,                   /**< uIP: network stack */
#endif
} PN_USAGE_T;


/* generic lock handle defines */
#define PN_LOCK_INFINITE                 0

#ifdef PORT_GMBH_GOAL
#  define PN_LOCK_BINARY  GOAL_LOCK_BINARY
#  define PN_LOCK_COUNT   GOAL_LOCK_COUNT
#  define PN_LOCK_TYPE_T  GOAL_LOCK_TYPE_T
#  define PN_LOCK_T       GOAL_LOCK_T
#else

/**< lock type */
typedef enum {
    PN_LOCK_BINARY,
    PN_LOCK_COUNT
} PN_LOCK_TYPE_T;

/**< lock data */
typedef struct {
    PN_BOOL_T active;           /**< lock active flag */
    PN_LOCK_TYPE_T type;        /**< lock type */
    union {
        OAL_LOCK_BIN_T bin;     /**< OAL binary lock handle */
        OAL_LOCK_CNT_T cnt;     /**< OAL counting lock handle */
    } handle;
    PN_USAGE_T usage;           /**< usage indicator */

#ifdef CONFIG_TARGET_RECURSIVE_MUTEX_SUPPORT
    OAL_THREAD_T owner;         /**< mutex owner */
    Unsigned32 ownerCnt;        /**< nested lock counter */
#endif
} PN_LOCK_T;
#endif


/* endianness convert defines
 *
 * PN_htobe16 - host to big endian, 16 bit
 * PN_htole16 - host to little endian, 16 bit
 * PN_be16toh - big endian to host, 16 bit
 * PN_le16toh - little endian to host, 16 bit
 *
 * PN_htobe32 - host to big endian, 32 bit
 * PN_htole32 - host to little endian, 32 bit
 * PN_be32toh - big endian to host, 32 bit
 * PN_le32toh - little endian to host, 32 bit
 */


/** Swap a 16 bit value */
#define PN_bswap16(x)   ((Unsigned16)(                                   \
                        (((Unsigned16)(x) & (Unsigned16)0x00ffU) << 8) | \
                        (((Unsigned16)(x) & (Unsigned16)0xff00U) >> 8)))


/** Read a 16 bit value as little endian byte-wise from a pointer destination
 *
 * ua - (assume) unaligned
 */
static CONFIG_INLINE Unsigned16 PN_get_le16_ua(const Unsigned16 PN_CC_PACKED_PRE *p16)
{
    return ((Unsigned16)(
            ((Unsigned16)(((const Unsigned8 *) p16)[0])) |
            ((Unsigned16)(((const Unsigned8 *) p16)[1] << 8))));
}


/** Read a 16 bit value as big endian byte-wise from a pointer destination
 *
 * ua - (assume) unaligned
 */
static CONFIG_INLINE Unsigned16 PN_get_be16_ua(const Unsigned16 PN_CC_PACKED_PRE *p16)
{
    return ((Unsigned16)(
            ((Unsigned16)(((const Unsigned8 *) p16)[0]) << 8) |
            ((Unsigned16)(((const Unsigned8 *) p16)[1]))));
}


/** Write a 16 bit value as little endian byte-wise to pointer destination
 *
 * ua - (assume) unaligned
 */
static CONFIG_INLINE void PN_set_le16_ua(Unsigned16 PN_CC_PACKED_PRE *p16, Unsigned16 x)
{
    ((Unsigned8 *) p16)[0] = (Unsigned8) (((Unsigned16)(x) & (Unsigned16)0x00ffU));
    ((Unsigned8 *) p16)[1] = (Unsigned8) (((Unsigned16)(x) & (Unsigned16)0xff00U) >> 8);
}


/** Write a 16 bit value as big endian byte-wise to pointer destination
 *
 * ua - (assume) unaligned
 */
static CONFIG_INLINE void PN_set_be16_ua(Unsigned16 PN_CC_PACKED_PRE *p16, Unsigned16 x)
{
    ((Unsigned8 *) p16)[0] = (Unsigned8) (((Unsigned16)(x) & (Unsigned16)0xff00U) >> 8);
    ((Unsigned8 *) p16)[1] = (Unsigned8) (((Unsigned16)(x) & (Unsigned16)0x00ffU));
}


/** Read a 24 bit value as big endian byte-wise from a pointer destination
 *
 * ua - (assume) unaligned
 */
static CONFIG_INLINE Unsigned32 PN_get_be24_ua(const Unsigned32 PN_CC_PACKED_PRE *p32)
{
    return ((Unsigned32)(
            ((Unsigned32)(((const Unsigned8 *) p32)[0]) << 16) |
            ((Unsigned32)(((const Unsigned8 *) p32)[1]) <<  8) |
            ((Unsigned32)(((const Unsigned8 *) p32)[2]))));
}


/** Swap a 32 bit value */
#define PN_bswap32(x)   ((Unsigned32)(                                          \
                        (((Unsigned32)(x) & (Unsigned32)0x000000ffUL) << 24) |  \
                        (((Unsigned32)(x) & (Unsigned32)0x0000ff00UL) <<  8) |  \
                        (((Unsigned32)(x) & (Unsigned32)0x00ff0000UL) >>  8) |  \
                        (((Unsigned32)(x) & (Unsigned32)0xff000000UL) >> 24)))


/** Read a 32 bit value as little endian byte-wise from a pointer destination
 *
 * ua - (assume) unaligned
 */
static CONFIG_INLINE Unsigned32 PN_get_le32_ua(const Unsigned32 PN_CC_PACKED_PRE *p32)
{
    return ((Unsigned32)(
            ((Unsigned32)(((const Unsigned8 *) p32)[0]))       |
            ((Unsigned32)(((const Unsigned8 *) p32)[1]) <<  8) |
            ((Unsigned32)(((const Unsigned8 *) p32)[2]) << 16) |
            ((Unsigned32)(((const Unsigned8 *) p32)[3]) << 24)));
}


/** Read a 32 bit value as big endian byte-wise from a pointer destination
 *
 * ua - (assume) unaligned
 */
static CONFIG_INLINE Unsigned32 PN_get_be32_ua(const Unsigned32 PN_CC_PACKED_PRE *p32)
{
    return ((Unsigned32)(
            ((Unsigned32)(((const Unsigned8 *) p32)[0]) << 24) |
            ((Unsigned32)(((const Unsigned8 *) p32)[1]) << 16) |
            ((Unsigned32)(((const Unsigned8 *) p32)[2]) <<  8) |
            ((Unsigned32)(((const Unsigned8 *) p32)[3]))));
}


/** Write a 32 bit value as little endian byte-wise to pointer destination
 *
 * ua - (assume) unaligned
 */
static CONFIG_INLINE void PN_set_le32_ua(Unsigned32 PN_CC_PACKED_PRE *p32, Unsigned32 x)
{
    ((Unsigned8 *) p32)[0] = (Unsigned8) (((Unsigned32)(x) & (Unsigned32)0x000000ffU));
    ((Unsigned8 *) p32)[1] = (Unsigned8) (((Unsigned32)(x) & (Unsigned32)0x0000ff00U) >>  8);
    ((Unsigned8 *) p32)[2] = (Unsigned8) (((Unsigned32)(x) & (Unsigned32)0x00ff0000U) >> 16);
    ((Unsigned8 *) p32)[3] = (Unsigned8) (((Unsigned32)(x) & (Unsigned32)0xff000000U) >> 24);
}


/** Write a 32 bit value as big endian byte-wise to pointer destination
 *
 * ua - (assume) unaligned
 */
static CONFIG_INLINE void PN_set_be32_ua(Unsigned32 PN_CC_PACKED_PRE *p32, Unsigned32 x)
{
    ((Unsigned8 *) p32)[0] = (Unsigned8) (((Unsigned32)(x) & (Unsigned32)0xff000000U) >> 24);
    ((Unsigned8 *) p32)[1] = (Unsigned8) (((Unsigned32)(x) & (Unsigned32)0x00ff0000U) >> 16);
    ((Unsigned8 *) p32)[2] = (Unsigned8) (((Unsigned32)(x) & (Unsigned32)0x0000ff00U) >>  8);
    ((Unsigned8 *) p32)[3] = (Unsigned8) (((Unsigned32)(x) & (Unsigned32)0x000000ffU));
}


/** Swap a 64 bit value */
#define PN_bswap64(x)   ((Unsigned64)(                                          \
                        (((Unsigned64)(x) & (Unsigned64)0x00000000000000ffULL) << 56) |  \
                        (((Unsigned64)(x) & (Unsigned64)0x000000000000ff00ULL) << 40) |  \
                        (((Unsigned64)(x) & (Unsigned64)0x0000000000ff0000ULL) >> 24) |  \
                        (((Unsigned64)(x) & (Unsigned64)0x00000000ff000000ULL) <<  8) |  \
                        (((Unsigned64)(x) & (Unsigned64)0x000000ff00000000ULL) >>  8) |  \
                        (((Unsigned64)(x) & (Unsigned64)0x0000ff0000000000ULL) >> 24) |  \
                        (((Unsigned64)(x) & (Unsigned64)0x00ff000000000000ULL) >> 40) |  \
                        (((Unsigned64)(x) & (Unsigned64)0xff00000000000000ULL) >> 56)))


/** Read a 64 bit value as little endian byte-wise from a pointer destination
 *
 * ua - (assume) unaligned
 */
static CONFIG_INLINE Unsigned64 PN_get_le64_ua(const Unsigned64 PN_CC_PACKED_PRE *p64)
{
    return ((Unsigned64)(
            ((Unsigned64)(((const Unsigned8 *) p64)[0]))       |
            ((Unsigned64)(((const Unsigned8 *) p64)[1]) <<  8) |
            ((Unsigned64)(((const Unsigned8 *) p64)[2]) << 16) |
            ((Unsigned64)(((const Unsigned8 *) p64)[3]) << 24) |
            ((Unsigned64)(((const Unsigned8 *) p64)[4]) << 32) |
            ((Unsigned64)(((const Unsigned8 *) p64)[5]) << 40) |
            ((Unsigned64)(((const Unsigned8 *) p64)[6]) << 48) |
            ((Unsigned64)(((const Unsigned8 *) p64)[7]) << 56)));
}


/** Read a 64 bit value as big endian byte-wise from a pointer destination
 *
 * ua - (assume) unaligned
 */
static CONFIG_INLINE Unsigned64 PN_get_be64_ua(const Unsigned64 PN_CC_PACKED_PRE *p64)
{
    return ((Unsigned64)(
            ((Unsigned64)(((const Unsigned8 *) p64)[0]) << 56) |
            ((Unsigned64)(((const Unsigned8 *) p64)[1]) << 48) |
            ((Unsigned64)(((const Unsigned8 *) p64)[2]) << 40) |
            ((Unsigned64)(((const Unsigned8 *) p64)[3]) << 32) |
            ((Unsigned64)(((const Unsigned8 *) p64)[4]) << 24) |
            ((Unsigned64)(((const Unsigned8 *) p64)[5]) << 16) |
            ((Unsigned64)(((const Unsigned8 *) p64)[6]) <<  8) |
            ((Unsigned64)(((const Unsigned8 *) p64)[7]))));
}


/** Write a 64 bit value as little endian byte-wise to pointer destination
 *
 * ua - (assume) unaligned
 */
static CONFIG_INLINE void PN_set_le64_ua(Unsigned64 PN_CC_PACKED_PRE *p64, Unsigned64 x)
{
    ((Unsigned8 *) p64)[0] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0x00000000000000ffULL));
    ((Unsigned8 *) p64)[1] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0x000000000000ff00ULL) >>  8);
    ((Unsigned8 *) p64)[2] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0x0000000000ff0000ULL) >> 16);
    ((Unsigned8 *) p64)[3] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0x00000000ff000000ULL) >> 24);
    ((Unsigned8 *) p64)[4] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0x000000ff00000000ULL) >> 32);
    ((Unsigned8 *) p64)[5] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0x0000ff0000000000ULL) >> 40);
    ((Unsigned8 *) p64)[6] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0x00ff000000000000ULL) >> 48);
    ((Unsigned8 *) p64)[7] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0xff00000000000000ULL) >> 56);
}


/** Write a 64 bit value as big endian byte-wise to pointer destination
 *
 * ua - (assume) unaligned
 */
static CONFIG_INLINE void PN_set_be64_ua(Unsigned64 PN_CC_PACKED_PRE *p64, Unsigned64 x)
{
    ((Unsigned8 *) p64)[0] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0xff00000000000000ULL) >> 56);
    ((Unsigned8 *) p64)[1] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0x00ff000000000000ULL) >> 48);
    ((Unsigned8 *) p64)[2] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0x0000ff0000000000ULL) >> 40);
    ((Unsigned8 *) p64)[3] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0x000000ff00000000ULL) >> 32);
    ((Unsigned8 *) p64)[4] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0x00000000ff000000ULL) >> 24);
    ((Unsigned8 *) p64)[5] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0x0000000000ff0000ULL) >> 16);
    ((Unsigned8 *) p64)[6] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0x000000000000ff00ULL) >>  8);
    ((Unsigned8 *) p64)[7] = (Unsigned8) (((Unsigned64)(x) & (Unsigned64)0x00000000000000ffULL));
}


#if defined(CONFIG_TARGET_LITTLE_ENDIAN) && defined(CONFIG_TARGET_BIG_ENDIAN)
#  error "Target endianness cannot be both: CONFIG_TARGET_LITTLE_ENDIAN and CONFIG_TARGET_BIG_ENDIAN"
#endif

#ifdef CONFIG_TARGET_LITTLE_ENDIAN

#  define PN_htole16(x)             (x)
#  define PN_htole16_p(p, x)        PN_set_le16_ua(p, x)
#  define PN_htole32(x)             (x)
#  define PN_htole32_p(p, x)        PN_set_le32_ua(p, x)
#  define PN_htole64(x)             (x)
#  define PN_htole64_p(p, x)        PN_set_le64_ua(p, x)

#  define PN_htobe16(x)             PN_bswap16(x)
#  define PN_htobe16_p(p, x)        PN_set_be16_ua(p, x)
#  define PN_htobe32(x)             PN_bswap32(x)
#  define PN_htobe32_p(p, x)        PN_set_be32_ua(p, x)
#  define PN_htobe64(x)             PN_bswap64(x)
#  define PN_htobe64_p(p, x)        PN_set_be64_ua(p, x)

#  define PN_le16toh(x)             (x)
#  define PN_le16toh_p(p)           PN_get_le16_ua(p)
#  define PN_le32toh(x)             (x)
#  define PN_le32toh_p(p)           PN_get_le32_ua(p)
#  define PN_le64toh(x)             (x)
#  define PN_le64toh_p(p)           PN_get_le64_ua(p)

#  define PN_be16toh(x)             PN_bswap16(x)
#  define PN_be16toh_p(p)           PN_get_be16_ua(p)
#  define PN_be24toh_p(p)           PN_get_be24_ua(p)
#  define PN_be32toh(x)             PN_bswap32(x)
#  define PN_be32toh_p(p)           PN_get_be32_ua(p)
#  define PN_be64toh(x)             PN_bswap64(x)
#  define PN_be64toh_p(p)           PN_get_be64_ua(p)

#elif defined(CONFIG_TARGET_BIG_ENDIAN)

#  define PN_htole16(x)             PN_bswap16(x)
#  define PN_htole16_p(p, x)        PN_set_le16_ua(p, x)
#  define PN_htole32(x)             PN_bswap32(x)
#  define PN_htole32_p(p, x)        PN_set_le32_ua(p, x)
#  define PN_htole64(x)             PN_bswap64(x)
#  define PN_htole64_p(p, x)        PN_set_le64_ua(p, x)

#  define PN_htobe16(x)             (x)
#  define PN_htobe16_p(p, x)        PN_set_be16_ua(p, x)
#  define PN_htobe32(x)             (x)
#  define PN_htobe32_p(p, x)        PN_set_be32_ua(p, x)
#  define PN_htobe64(x)             (x)
#  define PN_htobe64_p(p, x)        PN_set_be64_ua(p, x)

#  define PN_le16toh(x)             PN_bswap16(x)
#  define PN_le16toh_p(p)           PN_get_le16_ua(p)
#  define PN_le32toh(x)             PN_bswap32(x)
#  define PN_le32toh_p(p)           PN_get_le32_ua(p)
#  define PN_le64toh(x)             PN_bswap64(x)
#  define PN_le64toh_p(p)           PN_get_le64_ua(p)

#  define PN_be16toh(x)             (x)
#  define PN_be16toh_p(p)           PN_get_be16_ua(p)
#  define PN_be24toh_p(p)           PN_get_be24_ua(p)
#  define PN_be32toh(x)             (x)
#  define PN_be32toh_p(p)           PN_get_be32_ua(p)
#  define PN_be64toh(x)             (x)
#  define PN_be64toh_p(p)           PN_get_be64_ua(p)

#else
#  error "Target endianness not set: CONFIG_TARGET_LITTLE_ENDIAN or CONFIG_TARGET_BIG_ENDIAN"
#endif


/** Convert a 16-bit value with a given endianness to host-endianness
 *
 * @param [in] pVal value pointer
 * @param [in] endian endianness
 *
 * @returns 16-bit value in host-format
 */

#define PN_en16toh_p(pVal, endian)  ((PN_ENDIAN_BIG == endian) ?    \
                                      PN_get_be16_ua(pVal) :        \
                                      PN_get_le16_ua(pVal))


/** Convert a 32-bit value with a given endianness to host-endianness
 *
 * @param [in] pVal value pointer
 * @param [in] endian endianness
 *
 * @returns 32-bit value in host-format
 */
#define PN_en32toh_p(pVal, endian)  ((PN_ENDIAN_BIG == endian) ?    \
                                      PN_get_be32_ua(pVal) :        \
                                      PN_get_le32_ua(pVal))


/** Convert a 64-bit value with a given endianness to host-endianness
 *
 * @param [in] pVal value pointer
 * @param [in] endian endianness
 *
 * @returns 64-bit value in host-format
 */
#define PN_en64toh_p(pVal, endian)  ((PN_ENDIAN_BIG == endian) ?    \
                                      PN_get_be64_ua(pVal) :        \
                                      PN_get_le64_ua(pVal))


/** Convert a 16-bit value from host-endianness to a given endianness
 *
 * @param [out] pVal value pointer
 * @param [in] valHost host-value
 * @param [in] endian endianness
 */
#define PN_htoen16_p(pVal, valHost, endian)                         \
                            do {                                    \
                                if (PN_ENDIAN_BIG == endian)        \
                                    PN_set_be16_ua(pVal, valHost);  \
                                else                                \
                                    PN_set_le16_ua(pVal, valHost);  \
                            } while(0)


/** Convert a 32-bit value from host-endianness to a given endianness
 *
 * @param [out] pVal value pointer
 * @param [in] valHost host-value
 * @param [in] endian endianness
 */
#define PN_htoen32_p(pVal, valHost, endian)                         \
                            do {                                    \
                                if (PN_ENDIAN_BIG == endian)        \
                                    PN_set_be32_ua(pVal, valHost);  \
                                else                                \
                                    PN_set_le32_ua(pVal, valHost);  \
                            } while(0)


/** Convert a 64-bit value from host-endianness to a given endianness
 *
 * @param [out] pVal value pointer
 * @param [in] valHost host-value
 * @param [in] endian endianness
 */
#define PN_htoen64_p(pVal, valHost, endian)                         \
                            do {                                    \
                                if (PN_ENDIAN_BIG == endian)        \
                                    PN_set_be64_ua(pVal, valHost);  \
                                else                                \
                                    PN_set_le64_ua(pVal, valHost);  \
                            } while(0)


/* align the position at 4 bytes and fill the gap with zeros */
#define PN_align32(ptr, pos)        do {                                             \
                                        Unsigned8 _cnt = (4 - (pos & 0x03)) & 0x03;   \
                                        for (; _cnt; _cnt--, ptr[pos++] = 0);          \
                                    } while (0)


/* logic to align memory pointer at OAL_MEM_ALIGN defines */
#if (OAL_MEM_ALIGN_CPU > OAL_MEM_ALIGN_NET)
#  define PN_ALIGN_PAD OAL_MEM_ALIGN_CPU
#else
#  define PN_ALIGN_PAD OAL_MEM_ALIGN_NET
#endif

#if (PN_ALIGN_PAD < OAL_MEM_ALIGN_CPU) || (PN_ALIGN_PAD < OAL_MEM_ALIGN_NET)
#  error "PN_ALIGN_PAD < OAL_MEM_ALIGN_*"
#endif


#if ((2 != OAL_MEM_ALIGN_CPU) && (4 != OAL_MEM_ALIGN_CPU) && (8 != OAL_MEM_ALIGN_CPU) &&\
    (16 != OAL_MEM_ALIGN_CPU)) ||                                                       \
    ((2 != OAL_MEM_ALIGN_NET) && (4 != OAL_MEM_ALIGN_NET) && (8 != OAL_MEM_ALIGN_NET) &&\
    (16 != OAL_MEM_ALIGN_NET))
#  error "OAL_MEM_ALIGN_CPU or OAL_MEM_ALIGN_NET: only 2, 4, 8 and 16 bit alignment supported"
#endif


#define PN_align_2(ptr, ptrcast)                                                        \
    ((3 == (ptrcast ptr & 3)) ? 3 : (2 - (ptrcast ptr & 3)))

#define PN_align_4(ptr, ptrcast)                                                        \
    ((ptrcast ptr & 3) ? (4 - (ptrcast ptr & 3)) : 0)

#define PN_align_8(ptr, ptrcast)                                                        \
    ((ptrcast ptr & 7) ? (8 - (ptrcast ptr & 7)) : 0)

#define PN_align_16(ptr, ptrcast)                                                       \
    ((ptrcast ptr & 15) ? (16 - (ptrcast ptr & 15)) : 0)

#define PN_alignPtr(align, ptr)                                                         \
    (void *) ((PtrCast) ptr + (                                                         \
        (2 == align) ? PN_align_2(ptr, (PtrCast)) : (                                   \
        (4 == align) ? PN_align_4(ptr, (PtrCast)) : (                                   \
        (8 == align) ? PN_align_8(ptr, (PtrCast)) :                                     \
                       PN_align_16(ptr, (PtrCast))))))

#define PN_alignInt(align, val)                                                         \
    (val + ((2 == align) ? PN_align_2(val, ) : (                                        \
            (4 == align) ? PN_align_4(val, ) : (                                        \
            (8 == align) ? PN_align_8(val, ) :                                          \
                           PN_align_16(val, )))))


/* define maximum values */
#define PN_MAX_I16 0x7fff
#define PN_MAX_U16 0xffff
#define PN_MAX_U32 0xffffffff


/* sane convert functions for larger to smaller widths */
#define PN_INT2U16(x)   (((0 < (x)) && (PN_MAX_I16 >= (x))) ? (Unsigned16) (x) : 0)
#define PN_UINT2I16(x)  (((0 < (x)) && (PN_MAX_I16 >= (x))) ? (Integer16)  (x) : 0)
#define PN_UINT2U16(x)  (((0 < (x)) && (PN_MAX_U16 >= (x))) ? (Unsigned16) (x) : 0)
#define PN_UINT2U32(x)  (((0 < (x)) && (PN_MAX_U32 >= (x))) ? (Unsigned32) (x) : 0)


/* set and clear bit macros
 * explanation: the C standard converts unsigned char operands to integers,
 * thats why the casting is necessary - otherwise for example IAR will give you
 * the warning:
 *     Remark[Pa091]: operator operates on value promoted to int (with possibly
 *     unexpected result)
 */
#define PN_bitSet(x, y)     (x | (1 << y))
#define PN_bitClear(x, y)   (x & ~(1 << y))
#define PN_maskSet(x, y)    (x | y)
#define PN_maskClear(x, y)  (x & ~(unsigned int) y)


/* verify type widths at compile time */
CASSERT(1 == sizeof(Integer8));
CASSERT(1 == sizeof(Unsigned8));
CASSERT(2 == sizeof(Integer16));
CASSERT(2 == sizeof(Unsigned16));
CASSERT(4 == sizeof(Integer32));
CASSERT(4 == sizeof(Unsigned32));
CASSERT(8 == sizeof(Integer64));
CASSERT(8 == sizeof(Unsigned64));


#endif
