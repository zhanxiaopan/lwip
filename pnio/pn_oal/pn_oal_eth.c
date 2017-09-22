/** @file
 *
 * @brief
 * PROFINET Generic Ethernet Frame Handling
 *
 * @details
 * This module implements generic handling functions for Ethernet frames.
 *
 * @copyright
 * Copyright 2010-2016 port GmbH Halle/Saale.
 * This software is protected Intellectual Property and may only be used
 * according to the licence agreement.
 */
#include <pn_includes.h>
#include <pn_oal_eth.h>

#include "oal_lwip.h"


PN_LOG_SET_ID(PN_LOG_ID_OAL)

extern struct netif g_netif;

/* global variables */
static PN_LOCK_T gEthSendLock;                  /**< send lock */
static PN_BOOL_T gEthInitFlag = PN_FALSE;       /**< init flag */

static QUEUE_T *gEthRxLowQueue;                 /**< Eth RX low prio memory descriptors */
static QUEUE_T *gEthTxLowQueue;                 /**< Eth TX low prio memory descriptors */
static QUEUE_T *gEthTxHighQueue;                /**< Eth TX high prio memory descriptors */
#ifdef CONFIG_LOGGING_TARGET_SYSLOG
static QUEUE_T *gSyslogTxQueue;                 /**< Syslog TX queue memory descriptors */
#endif


/* prototypes */
#ifdef CONFIG_LOGGING_TARGET_SYSLOG
static IOD_STATUS_T OAL_msgSyslogInit(void);
#endif

static OAL_BUFFER_T *pRxBufPNCyclic;
static OAL_BUFFER_T *pRxBufPNGeneral;
static OAL_BUFFER_T *pRxBufLLDP;

static struct pbuf *pTxBuffer;


void OAL_ethBufferInit()
{
    IOD_STATUS_T res;                               /**< result */
    res = OAL_getBufferById(&pRxBufPNCyclic, MM_QID_NET, PN_USAGE_OAL_ETH_RECV);
    OAL_ASSERT(IOD_OK == res);
    res = OAL_getBufferById(&pRxBufPNGeneral, MM_QID_NET, PN_USAGE_OAL_ETH_RECV);
    OAL_ASSERT(IOD_OK == res);
    res = OAL_getBufferById(&pRxBufLLDP, MM_QID_NET, PN_USAGE_OAL_ETH_RECV);
    OAL_ASSERT(IOD_OK == res);

    return;
}


/** Ethernet Handling Initialization
 *
 * This function initializes the Ethernet send lock and the receive buffer.
 */
IOD_STATUS_T OAL_ethInit(
    unsigned char *macAddr,                  /**< MAC address */
    uint32_t ip,
    uint32_t nm,
    uint32_t gw
)
{
    IOD_STATUS_T retVal = IOD_OK;           /**< return value */

    /* create send lock */
    PN_lockCreate(PN_LOCK_BINARY, &gEthSendLock, 0, 1, PN_USAGE_OAL_ETH_SEND);

    /* initialize RX/TX queues */
    retVal = OAL_memInitQueue(&gEthRxLowQueue, ETH_RX_LOW_QUEUE_NUM, 0, 0);
    if (IOD_OK != retVal) {
        return retVal;
    }

    retVal = OAL_memInitQueue(&gEthTxLowQueue, ETH_TX_LOW_QUEUE_NUM, 0, 0);
    if (IOD_OK != retVal) {
        return retVal;
    }

    retVal = OAL_memInitQueue(&gEthTxHighQueue, ETH_TX_HIGH_QUEUE_NUM, 0, 0);
    if (IOD_OK != retVal) {
        return retVal;
    }

#ifdef CONFIG_LOGGING_TARGET_SYSLOG
    /* initialize syslog */
    retVal = OAL_msgSyslogInit();
    if (IOD_OK != retVal) {
        return retVal;
    }
#endif

    OAL_ethBufferInit();

    /* enable Ethernet */
    OAL_setupLwIP(ip, nm, gw);

    /* initialize udp channel */
    retVal = OAL_udpInit();
    if (IOD_OK != retVal) {
        return retVal;
    }

    /* set init done flag */
    gEthInitFlag = PN_TRUE;

    return IOD_OK;
}


/** Ethernet Handling Shutdown
 *
 * This function shuts the Ethernet handling down.
 */
void OAL_ethShutdown(
    void
)
{
    gEthInitFlag = PN_FALSE;
    PN_logInfo("OAL Ethernet shutdown");
}


/** Generic Ethernet Receive Callback for unspecific Frames
 *
 * This function is called by the target when an Ethernet frame was received.
 */
void OAL_ethRecv(
    Unsigned32 portIdx,         /**< Ethernet port index */
    OAL_BUFFER_T **ppBuf        /**< OS Ethernet buffer */
)
{
    OAL_BUFFER_T *pBuf = NULL;  /**< RX queue buffer */
    OAL_BUFFER_T *pBufTmp;      /**< temporary exchange buffer */

    /* check if Ethernet handling is initialized */
    if (PN_FALSE == gEthInitFlag) {
        return;
    }
    printf("OAL_ethRecv\n");
    /* check port index range */
    if (CONFIG_TARGET_ETH_PORT_COUNT <= portIdx) {
        PN_logErr("port index out of range: %"FMT_u32, portIdx);
        OAL_halt(portIdx);
    }

    /* check that buffer contains data */
    OAL_ASSERT(0 != (*ppBuf)->dataLen);

    /* update buffer pointer, length and OAL type */
    (*ppBuf)->netPort = portIdx;

    /* if the buffer can be handled by the cyclic handler, it'll be swapped */
    if (IOD_OK == LM_handleCyclic(ppBuf)) {
        return;
    }

    /* non-cyclic frame: check if the RX queue isn't full */
    if (MM_isQueueFull(gEthRxLowQueue)) {
        return;
    }

    /* allocate a new buffer for the non-RT traffic */
    if (IOD_OK != OAL_getBufferById(&pBuf, MM_QID_NET, PN_USAGE_OAL_ETH_RECV)) {
        PN_statInc(PN_STAT_OAL_RX_OOM);
        return;
    }

    /* exchange OAL buffer and RX queue buffer */
    pBufTmp = *ppBuf;
    *ppBuf = pBuf;
    pBuf = pBufTmp;

    OAL_ASSERT(PN_memFlagsGet(pBuf, PN_MEM_FLAG_USED));

    /* add current buffer to RX queue */
    if (IOD_OK != OAL_QAddElemToQueue(gEthRxLowQueue, pBuf)) {
        OAL_releaseBuffer(&pBuf);
    }
}


/** Generic Ethernet Send Function for PROFINET Buffers
 *
 * This function is called when an Ethernet frame that is embedded in a
 * PROFINET buffer is ready to send.
 */
IOD_STATUS_T OAL_ethSendAndRelease(
    void **ppOAL,                       /**< pointer to PROFINET buffer */
    PN_NET_TX_TYPE_T type               /**< frame channel type */
)
{
    OAL_BUFFER_T *cur = (OAL_BUFFER_T *) *ppOAL;

    /* check if Ethernet handling is initialized */
    if (PN_FALSE == gEthInitFlag) {

        /* free buffer */
        PN_netEthSendDone((OAL_BUFFER_T **) ppOAL, PN_FALSE);

        return IOD_NET_FRAME_SEND_FAILED;
    }

//    /* enqueue frame */
//    if (PN_NET_TX_RT == type) {
//        pTxQueue = gEthTxHighQueue;
//    } else {
//        pTxQueue = gEthTxLowQueue;
//    }
//
//    /* make sure frame isn't already released */
//    OAL_ASSERT(PN_memFlagsGet((OAL_BUFFER_T *) *ppOAL, PN_MEM_FLAG_USED));
//
//    res = OAL_QAddElemToQueue(pTxQueue, (OAL_BUFFER_T *) *ppOAL);
//    if (IOD_OK == res)
//    {
//        res = OAL_ethSendTrigger();
//        *ppOAL = NULL;
//    }
//    else
//    {
//        PN_netEthSendDone((OAL_BUFFER_T **) ppOAL, PN_FALSE);
//    }

    if (cur)
    {
        // allocate a buffer
        pTxBuffer = pbuf_alloc(PBUF_RAW, ETH_MTU_LEN, PBUF_RAM);
        // copy the data
        memcpy((void *)(pTxBuffer->payload), (void *)(cur->ptrData), cur->dataLen);
        pTxBuffer->len = cur->dataLen;
        // release the buffer
        PN_memFlagsClear(cur, PN_MEM_FLAG_TX);
        OAL_releaseBuffer(&cur);
        *ppOAL = NULL;
        //
        g_netif.linkoutput(&g_netif, pTxBuffer);
        //
        pbuf_free(pTxBuffer);
        pTxBuffer = NULL;
    }

    return IOD_OK;
}


/** Generic Ethernet Send Trigger Function
 *
 * This function is called when an Ethernet frame when a frame transfer was
 * finished or a new frame was put into the TX queue.
 *
 * It frees the current buffer and checks if the next buffer is available.
 *
 * Info: Make sure OAL_ethSendTrigger is not called recursively when
 *       TARGET_send is called. The normal invocation is secured by
 *       gEthSendLock but this won't work for TARGET_send (same context).
 */
IOD_STATUS_T OAL_ethSendTrigger(
    void
)
{
    OAL_BUFFER_T *cur = NULL;           /**< current frame indicator */
    IOD_STATUS_T res = IOD_OK;

    /* check if Ethernet handling is initialized */
    if (PN_FALSE == gEthInitFlag) {
        return IOD_NET_FRAME_SEND_FAILED;
    }

    /* This function can be called from the timer (send cyclic or other data),
     * the Ethernet interrupt (data send ready) or the application (send
     * various data). To prevent this function from manipulating the current
     * send buffer in parallel, the function entry is locked.
     */
    res = PN_lockGet(&gEthSendLock, PN_LOCK_INFINITE);
    if (IOD_OK != res) {
        return res;
    }

    /* check TX status */

    /* check if a high priority frame is pending */
    if (IOD_OK != OAL_QGetElemFromQueue(gEthTxHighQueue, (void **) &cur))
    {
        /* otherwise, check for low priority frames */
        OAL_QGetElemFromQueue(gEthTxLowQueue, (void **) &cur);
    }

    /* if a frame was found, send it */
    if (cur)
    {
        // allocate a buffer
        pTxBuffer = pbuf_alloc(PBUF_RAW, ETH_MTU_LEN, PBUF_RAM);
        // copy the data
        memcpy((void *)(pTxBuffer->payload), (void *)(cur->ptrData), cur->dataLen);
        pTxBuffer->len = cur->dataLen;
        // release the unused buffer
        PN_memFlagsClear(cur, PN_MEM_FLAG_TX);
        OAL_releaseBuffer(&cur);
        //
        g_netif.linkoutput(&g_netif, pTxBuffer);
        // release the buffer
        pbuf_free(pTxBuffer);
        pTxBuffer = NULL;
    }

    /* release lock */
    PN_lockPut(&gEthSendLock);

    return res;
}


/** PROFINET Main Loop Network Data Handler
 *
 * This handles non-RT network data and must be called in the main loop.
 * The location of this function may be changed later, because it's very stack
 * dependend.
 */
void OAL_handleNonRT(
    void
)
{
    OAL_BUFFER_T *frame = NULL;             /**< frame buffer */

    /* read an element from the receive queue */
    while ((IOD_OK == OAL_QGetElemFromQueue(gEthRxLowQueue, (void **) &frame)) && frame) {

        OAL_ASSERT(PN_memFlagsGet(frame, PN_MEM_FLAG_USED));

        /* call handler depending on frame type */
        switch (frame->etherType) {

            case ETH_P_PNIO:
                LM_handleNonCyclic(frame);

                /* release buffer */
                OAL_releaseBuffer(&frame);

                break;

#ifdef CONFIG_LLDP_RX_ENABLED
            case ETH_P_LLDP:
                PN_LLDP_FUNC_RECV(frame);

                /* release buffer */
                OAL_releaseBuffer(&frame);

                break;
#endif

            default:
                // do nothing, just release the OAL Buffer
                OAL_releaseBuffer(&frame);
        }
    }

}


void pnio_input(struct pbuf *p, struct netif *netif, uint16_t frame_id)
{
    uint16_t pn_frame_id;
    OAL_BUFFER_T *pBuf = NULL;
    OAL_BUFFER_T *pTemp = NULL;

    /* check if frame isn't a cyclic frame */
    if (!(((0x8000 <= frame_id) && (0xbfff >= frame_id)) || \
          ((0xc000 <= frame_id) && (0xfbff >= frame_id)) || \
          ((0x0100 <= frame_id) && (0x7fff >= frame_id))))
    {
        //
        memcpy((pRxBufPNGeneral->ptrData), (p->payload), p->tot_len);
        pRxBufPNGeneral->dataLen = p->tot_len;
        pRxBufPNGeneral->netPort = 0;
        //
        PN_netRawGetFrameID(pRxBufPNGeneral, &pn_frame_id);
        // put it to the queue, check if the RX queue is NOT full (return 0)
        if (MM_isQueueFull(gEthRxLowQueue) == 0)
        {
            if (IOD_OK == OAL_getBufferById(&pBuf, MM_QID_NET, PN_USAGE_OAL_ETH_RECV))
            {
                // exchange OAL BUFFER and RX BUFFER
                pTemp = pRxBufPNGeneral;
                pRxBufPNGeneral = pBuf;
                pBuf = pTemp;
                //
                OAL_ASSERT(PN_memFlagsGet(pBuf, PN_MEM_FLAG_USED));
                // add current buffer to RX Queue
                if (IOD_OK != OAL_QAddElemToQueue(gEthRxLowQueue, pBuf))
                {
                    OAL_releaseBuffer(&pBuf);
                }
            }// end of get buffer by id
        }// end of queue full check
    }
    else
    {
        // it is a cyclic frame
        memcpy((void *)(pRxBufPNCyclic->ptrData), (void *)(p->payload), p->tot_len);
        pRxBufPNCyclic->dataLen = p->tot_len;
        pRxBufPNCyclic->netPort = 0;
        //
        PN_netRawGetFrameID(pRxBufPNCyclic, &pn_frame_id);
        // if the buffer can be handled by the cyclic handler, it will be swapped
        LM_handleCyclic(&pRxBufPNCyclic);
    }

    pbuf_free(p);
    return;
}

void lldp_input(struct pbuf *p, struct netif *netif)
{
    uint16_t pn_frame_id;
    OAL_BUFFER_T *pBuf = NULL;
    OAL_BUFFER_T *pTemp = NULL;

    memcpy((void *)(pRxBufLLDP->ptrData), (void *)(p->payload), p->tot_len);
    pRxBufLLDP->dataLen = p->tot_len;
    pRxBufLLDP->netPort = 0;
    //
    PN_netRawGetFrameID(pRxBufLLDP, &pn_frame_id);
    // put it into the Queue, check if the Rx queue if NOT full
    if (MM_isQueueFull(gEthRxLowQueue) == 0)
    {
        if (IOD_OK == OAL_getBufferById(&pBuf, MM_QID_NET, PN_USAGE_OAL_ETH_RECV))
        {
            // exchange OAL BUFFER and RX BUFFER
            pTemp = pRxBufLLDP;
            pRxBufLLDP = pBuf;
            pBuf = pTemp;
            //
            OAL_ASSERT(PN_memFlagsGet(pBuf, PN_MEM_FLAG_USED));
            // add current buffer to RX Queue
            if (IOD_OK != OAL_QAddElemToQueue(gEthRxLowQueue, pBuf))
            {
                OAL_releaseBuffer(&pBuf);
            }
        }// end of get buffer by id
    }// end of queue full check

    pbuf_free(p);
    return;
}



#ifdef CONFIG_LOGGING_TARGET_SYSLOG
/** Initialize Syslog Buffers
 *
 * Reserves CONFIG_LOGGING_TARGET_SYSLOG_BUF buffers to send syslog frames.
 */
static IOD_STATUS_T OAL_msgSyslogInit(
    void
)
{
    IOD_STATUS_T retVal = IOD_OK;           /**< return value */
    OAL_BUFFER_T *pBuf = NULL;              /**< OAL buffer */
    unsigned int cnt;                       /**< counter */

    /* initialize queue */
    retVal = OAL_memInitQueue(&gSyslogTxQueue, CONFIG_LOGGING_TARGET_SYSLOG_BUF, 0, 0);

    /* initialize queue buffers */
    if (IOD_OK == retVal) {

        for (cnt = 0; cnt < CONFIG_LOGGING_TARGET_SYSLOG_BUF; cnt++) {

            /* get buffer from small queue */
            retVal = OAL_getBuffer(&pBuf, MEM_SIZE_SBUF, PN_USAGE_LOG_SYSLOG);
            if (IOD_OK != retVal) {
                break;
            }

            /* put buffer into syslog queue */
            if (IOD_OK != OAL_releaseBufferToNewQueue(gSyslogTxQueue, &pBuf)) {
                retVal = IOD_ALLOC_ERR;
                break;
            }
        }
    }

    return retVal;
}


/** Output Syslog Message via Ethernet
 *
 * Take a buffer from the syslog queue, copy the message into it and transfer
 * it to the given target.
 */
void OAL_msgSyslog(
    const char *str,            /**< log string */
    unsigned int len            /**< string length */
)
{
    OAL_BUFFER_T *pBuf = NULL;  /**< OAL buffer desc */
    unsigned int clen;          /**< copy length */

    /* try to get syslog buffer */
    if (IOD_OK != OAL_getBufferFromQueue(&pBuf, gSyslogTxQueue, PN_USAGE_LOG_SYSLOG)) {

        /* all buffers in transmission, drop message */
        return;
    }

    /* copy message to buffer */
    clen = (len < MEM_SIZE_SBUF) ? len : MEM_SIZE_SBUF;
    OAL_MEMCPY(pBuf->ptrData, str, clen);
    pBuf->dataLen = PN_UINT2U16(clen);

    /* transmit the message */
    OAL_ethSendAndRelease((void **) &pBuf, PN_NET_TX_LOW);
}
#endif /* CONFIG_LOGGING_TARGET_SYSLOG */
