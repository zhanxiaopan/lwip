

#ifndef PN_MEM_H
#define PN_MEM_H

#include <pn_includes.h>

#ifndef CONFIG_LOGGING_TARGET_SYSLOG_BUF
#  define CONFIG_LOGGING_TARGET_SYSLOG_BUF 0
#endif


#define OAL_NUM_SBUF            (                                   \
                                8 +                                 \
                                CONFIG_LOGGING_TARGET_SYSLOG_BUF    \
                                )


/* network buffer calculation */
#define OAL_NUM_NET             (                               \
                                8 +                             \
                                CONFIG_TARGET_ETH_PORT_COUNT +  \
                                CONFIG_TARGET_ETH_BUF_TX +      \
                                CONFIG_TARGET_ETH_BUF_RX        \
                                )

#ifdef CONFIG_UIP_TCP_DELAYED_ACK
#  define OAL_NUM_TCP           16
#endif /* CONFIG_UIP_TCP_DELAYED_ACK */


/* pn_crtdata: CD_verifyIOCRBlock tracking space */
#define MEM_SIZE_TRACK          20

#define MEM_SIZE_SBUF           (250 + MEM_SIZE_TRACK)
#define MEM_SIZE_NET            (ETH_MTU_LEN + MEM_SIZE_TRACK)


#ifdef CONFIG_UIP_TCP_DELAYED_ACK
#  include "uip.h"
#  include "uipopt.h"
#  if !defined(UIP_TCP_MSS)
#    error "UIP_TCP_MSS not defined"
#  elif !defined(ETH_HDR_LEN)
#    error "ETH_HDR_LEN not defined"
#  elif !defined(VLAN_TAG_LEN)
#    error "VLAN_TAG_LEN not defined"
#  elif !defined(UIP_TCPIP_HLEN)
#    error "UIP_TCPIP_HLEN not defined"
#  endif

#  define MEM_SIZE_TCPBUF       (UIP_TCP_MSS+ETH_HDR_LEN+VLAN_TAG_LEN+UIP_TCPIP_HLEN+MEM_SIZE_TRACK)
#endif /* CONFIG_UIP_TCP_DELAYED_ACK */


/* Wait-For-Release Queue Element Count */
#define PN_MEM_WAIT_FOR_RELEASE_MAX             5


/* Memory Flags */
#define PN_MEM_FLAG_NONE                        0
#define PN_MEM_FLAG_TX                          (1 << 0)    /**< transmit flag */
#define PN_MEM_FLAG_NO_RELEASE                  (1 << 1)    /**< no-release flag */
#define PN_MEM_FLAG_VLAN                        (1 << 2)    /**< VLAN flag */
#define PN_MEM_FLAG_KEEP_CONTENT                (1 << 3)    /**< keep content after sending */
#define PN_MEM_FLAG_USED                        (1 << 4)    /**< buffer currently in use */


/* Queue IDs */
typedef enum {
    MM_QID_SMALL = 0,               /**< small buffer queue */
    MM_QID_NET,                     /**< large/net buffer queue */

#ifdef CONFIG_UIP_TCP_DELAYED_ACK
    MM_QID_TCP,                     /**< TCP delayed ACK queue */
#endif

    MM_QID_MAX,                     /**< highest usable queue ID index */
    MM_QID_NONE                     /**< unqueued marker */
} MM_QID_T;


/**< OAL_BUFFER_T prototype */
struct OAL_BUFFER_T;


/**< Queue / Ringbuffer Structure */
typedef struct
{
    PN_LOCK_T           mutex;      /**< queue lock */
    Unsigned16          rdIdx;      /**< read index */
    Unsigned16          wrIdx;      /**< write index */
    Unsigned16          cnt;        /**< element count */
    Unsigned16          size;       /**< queue size */
    Unsigned8           flagEmpty;  /**< empty flag */
    Unsigned8           flagFull;   /**< full flag */

    struct OAL_BUFFER_T **pBufs;    /**< queue buffers */
} QUEUE_T;


/**< Queue Description Structure */
typedef struct {
    QUEUE_T *pQueue;                /**< queue pointer */
    Unsigned16 elemCnt;             /**< count of elements */
    Unsigned16 elemSize;            /**< size per element */
} QUEUE_DESC_T;


/**< Buffer Release Callback */
typedef IOD_STATUS_T (*MM_releaseCb)(struct OAL_BUFFER_T *, void *);


/**< Buffer Metadata Structure */
typedef struct OAL_BUFFER_T
{
    PN_LOCK_T     mtx;                      /**< buffer mutex */

    Unsigned16    bufSize;                  /**< buffer max size */
    Unsigned16    dataLen;                  /**< real data length */
    Unsigned32    flags;                    /**< flags */
    Unsigned8    *ptrData;                  /**< data pointer */
    QUEUE_T      *pOrigQueue;               /**< originator queue */
    QUEUE_T      *pQueue;                   /**< return queue pointer */
#ifdef CONFIG_TARGET_MEM_RELEASE_FUNC
    void         *pOalBuf;                  /**< OAL buffer describer */
#endif
#ifdef PORT_GMBH_GOAL
    GOAL_BUFFER_T *pGoal;                   /**< GOAL buffer */
#endif
    Unsigned32    usage;                    /**< usage describer */
    Unsigned32    netPort;                  /**< network port */
    Unsigned16    etherType;                /**< EtherType */

    MM_releaseCb  relCb;                    /**< release callback */
    void         *pArg;                     /**< release callback argument */

#ifdef CONFIG_DEBUG_BUFFER_TRACE
    Unsigned32    traceId;                  /**< trace ID */
#endif
} OAL_BUFFER_T;


/* Function Prototypes */
IOD_STATUS_T MM_init(void);
IOD_STATUS_T OAL_memInitQueue(QUEUE_T **, Unsigned16, Unsigned16, Unsigned16);
IOD_STATUS_T OAL_getBuffer(OAL_BUFFER_T **, unsigned int, PN_USAGE_T);
IOD_STATUS_T OAL_getBufferById(OAL_BUFFER_T **, MM_QID_T, PN_USAGE_T);
IOD_STATUS_T OAL_getBufferFromQueue(OAL_BUFFER_T **, QUEUE_T *, PN_USAGE_T);
IOD_STATUS_T OAL_releaseBuffer(OAL_BUFFER_T **);
IOD_STATUS_T OAL_releaseBufferToOrigQueue(OAL_BUFFER_T **);
IOD_STATUS_T OAL_releaseBufferToNewQueue(QUEUE_T *, OAL_BUFFER_T **);
IOD_STATUS_T OAL_QGetElemFromQueue(QUEUE_T *pQueue, void **pMem);
IOD_STATUS_T OAL_QAddElemToQueue(QUEUE_T *, void *);
unsigned int MM_isQueueFull(QUEUE_T *);
unsigned int MM_isQueueEmpty(QUEUE_T *);
void OAL_setBufferReleaseCallback(OAL_BUFFER_T *, MM_releaseCb, void *);
void PN_memDataLenSet(OAL_BUFFER_T *pBuf, Unsigned16 dataLen);
void PN_memNetPortSet(OAL_BUFFER_T *pBuf, Unsigned32 netPort);
void PN_memFlagsClear(OAL_BUFFER_T *pBuf, Unsigned32 flags);
void PN_memFlagsSet(OAL_BUFFER_T *pBuf, Unsigned32 flag);
PN_BOOL_T PN_memFlagsGet(OAL_BUFFER_T *pBuf, Unsigned32 flags);


#ifdef CONFIG_DEBUG_BUFFER_TRACE
void OAL_setBufferTraceId(OAL_BUFFER_T *, Unsigned32);
#else
#  define OAL_setBufferTraceId(x, y)
#endif

#endif /* PN_MEM_H */
