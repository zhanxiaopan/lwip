/**
 *  \file new 1
 *  \brief Brief
 */

#ifndef __NETCONF_H
#define __NETCONF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes --------------------------------------------------------------------------------*/
#include "bsp_config.h"					// bsp_CONFIG Header file
#include "driverlib/emac.h"				// EMAC header
#include "ethernetif.h"					// TI Port
#include "lwip/mem.h"					// LwIP Includes
#include "lwip/memp.h"					// ...
#include "lwip/init.h"					// ...
#include "lwip/tcp.h"					// ...
#include "lwip/tcp_impl.h"				// ...
#include "netif/etharp.h"				// ...
#include "lwip/dhcp.h"					// ...

/* Definitions -----------------------------------------------------------------------------*/
extern struct netif lwip_netif;			// Network Interface structure

/* IP Configurations -----------------------------------------------------------------------*/
//#define USE_DHCP						// Enable DHCP, if disabled static address is used
#ifdef USE_DHCP
	volatile uint8_t DHCP_state;
	#define DHCP_START                 1
	#define DHCP_WAIT_ADDRESS          2
	#define DHCP_ADDRESS_ASSIGNED      3
	#define DHCP_TIMEOUT               4
	#define DHCP_LINK_DOWN             5
#endif

// MAC ADDRESS: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5
#define MAC_ADDR0   0x00
#define MAC_ADDR1   0xE0
#define MAC_ADDR2   0x22
#define MAC_ADDR3   0xFE
#define MAC_ADDR4   0x49
#define MAC_ADDR5   0x76
// Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   125
#define IP_ADDR3   67
// NETMASK: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0
// Gateway Address: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3
#define GW_ADDR0   0
#define GW_ADDR1   0
#define GW_ADDR2   0
#define GW_ADDR3   0

extern	uint8_t uip_add_0;
extern	uint8_t uip_add_1;
extern	uint8_t uip_add_2;
extern	uint8_t uip_add_3;

/* Exported functions -------------------------------------------------------------------------------*/
void	Ethernet_InitMACPHYDMA(void);
void	lwip_user_init(void);
void 	lwip_timer(void);
void	lwip_set_link_state(uint8_t linkState);
void	lwip_periodic_handle(volatile uint32_t localtime);
void	lwip_link_monitor(void);
void	lwip_process(void);

#define LwIP_DEBUG 0 	// Define this to 1 to enable debug printing
#if LwIP_DEBUG > 0
extern void Console_PrintColor(uint8_t color, char * format, ...);
  #define LwIP_Printf(f, ...) Console_PrintColor(CONSOLE_COLOR_CYAN, f, ##__VA_ARGS__)
#else
  #define LwIP_Printf(f, ...)
#endif


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __NETCONF_H */
