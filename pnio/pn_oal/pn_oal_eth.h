

#ifndef PN_OAL_ETH_H
#define PN_OAL_ETH_H

#include <pn_includes.h>
#include <pn_mem.h>
#include "lwip/pbuf.h"
#include "lwip/netif.h"

/* Ethernet queue sizes */
#define ETH_RX_LOW_QUEUE_NUM    14
#define ETH_TX_LOW_QUEUE_NUM    14
#define ETH_TX_HIGH_QUEUE_NUM   12


/* Prototypes */
void OAL_ethRecv(Unsigned32, OAL_BUFFER_T **ppBuf);
IOD_STATUS_T OAL_ethSendTrigger(void);
void OAL_handleNonRT(void);

void pnio_input(struct pbuf *p, struct netif *netif, uint16_t frame_id);
void lldp_input(struct pbuf *p, struct netif *netif);

#endif /* PN_OAL_ETH_H */
