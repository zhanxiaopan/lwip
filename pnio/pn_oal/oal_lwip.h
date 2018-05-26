/*
 * oal_lwip.h
 *
 * OS Abstraction Layer based on LwIP
 *
 */

#ifndef PN_OAL_LWIP_H_
#define PN_OAL_LWIP_H_

#include "pn_includes.h"


/* Types */
typedef struct {
    PN_BOOL_T used;                  /**< usage indicator */
    uint16_t port;
    struct udp_pcb *conn;            /**< udp connection handle */
    OAL_UDP_RECV_CB_T callback;      /**< receive callback */
} HAL_UDP_CHAN_T;


#define HAL_NUM_UDP_PORTS 2

extern void LwIPEthernetIntHandler(void);

void OAL_setupLwIP(uint32_t ip, uint32_t netmask, uint32_t gateway);

IOD_STATUS_T OAL_udpInit();

#endif /* PN_OAL_LWIP_H_ */
