

#ifndef __TIVAIF_H__
#define __TIVAIF_H__

//#include <stdint.h>

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_emac.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/emac.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "netif/ppp_oe.h"


extern int tivaif_input(struct netif *psNetif);
extern err_t tivaif_init(struct netif *psNetif);
extern void tivaif_interrupt(struct netif *netif, uint32_t ui32Status);

#if NETIF_DEBUG
void tivaif_debug_print(struct pbuf *psBuf);
#else
#define tivaif_debug_print(psBuf)
#endif /* NETIF_DEBUG */

#endif // __TIVAIF_H__
