/**
 *  \file new 1
 *  \brief Brief
 */

/* Includes --------------------------------------------------------------------------------*/
#include "netconf.h"				// header file

// global ip addr variables
uint8_t uip_add_0 = 0;
uint8_t uip_add_1 = 0;
uint8_t uip_add_2 = 0;
uint8_t uip_add_3 = 0;

/* Private declarations --------------------------------------------------------------------*/
#ifdef USE_DHCP
	#define MAX_DHCP_TRIES 4
	uint32_t DHCPfineTimer = 0;
	uint32_t DHCPcoarseTimer = 0;
	void LwIP_DHCP_Process_Handle(void);
#endif
struct netif lwip_netif;
static uint32_t TCPTimer = 0;
static uint32_t ARPTimer = 0;
uint32_t IPaddress = 0;
void LWIP_LinkCallback(struct netif *netif);

/* Initialization functions ----------------------------------------------------------------*/
/**
  * @brief	Initialize the lwIP stack.
  * @param	none
  * @retval	none.
  */
void LwIP_Init(void)
{
	// Initializes the LwIP modules
    lwip_init();
	// IP Addresses
	struct ip_addr ipaddr, netmask, gw;
#ifdef USE_DHCP
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else
	//IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
	IP4_ADDR(&ipaddr, uip_add_0, uip_add_1, uip_add_2, uip_add_3);
	IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
	IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
#endif
	// Register the lwip_netif network interface
    netif_add(&lwip_netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, ip_input);
	// Set it as default network interface
	netif_set_default(&lwip_netif);
	// Initialize interface as link down
	netif_set_link_down(&lwip_netif);
	// Set the link callback function, this function is called on change of link status
	netif_set_link_callback(&lwip_netif, LWIP_LinkCallback);
#ifdef USE_DHCP
	DHCP_state = DHCP_LINK_DOWN;
#endif
	// Bring the interface up
	netif_set_up(&lwip_netif);
}

/**
  * @brief	Handles Ethernet interrupts for the lwIP TCP/IP stack.
  * @param	none.
  * @retval	none.
  */
void Ethernet_IntHandler(void)
{
    // Read and Clear the interrupt status
    uint32_t ui32Status = EMACIntStatus(EMAC0_BASE, true);
    EMACIntClear(EMAC0_BASE, ui32Status);
    // Call the low-level interrupt handler
	ethernetif_interrupt(&lwip_netif, ui32Status);
}

/* Ethernet Link functions -----------------------------------------------------------------*/
/**
  * @brief	Set the Link state of the lwip_netif network interface.
  * @param	linkState: the actual link state (1 link, 0 NO link)
  * @retval	none.
  */
void LwIP_Set_Link_State(uint8_t linkState)
{
	if (linkState==1) netif_set_link_up(&lwip_netif);
	else netif_set_link_down(&lwip_netif);
}
/**
  * @brief  Link callback function, this function is called on change of link status.
  * @param  The network interface
  * @retval None
  */
void LWIP_LinkCallback(struct netif *netif)
{
	struct ip_addr ipaddr, netmask, gw;
	//-----------------------------------------------------------------------
	// Link went Up
	if(netif_is_link_up(netif))
	{
		//Set the addresses
#ifdef USE_DHCP
		ipaddr.addr = 0;
		netmask.addr = 0;
		gw.addr = 0;
		DHCP_state = DHCP_START;
#else
//		IP4_ADDR(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
		IP4_ADDR(&ipaddr, uip_add_0, uip_add_1, uip_add_2, uip_add_3);
		IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1 , NETMASK_ADDR2, NETMASK_ADDR3);
		IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
#endif
		netif_set_addr(&lwip_netif, &ipaddr , &netmask, &gw);
		// Set Interface as up
		netif_set_up(&lwip_netif);
		// Print new link state
		LwIP_Printf("Ethernet Connected");
		#ifndef USE_DHCP
			LwIP_Printf("Static IP address: %d.%d.%d.%d", IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
		#endif
	}
	//-----------------------------------------------------------------------
	// Link went Down
	else
	{
		#ifdef USE_DHCP
			// Stop DHCP
			DHCP_state = DHCP_LINK_DOWN;
			dhcp_stop(netif);
		#endif
		// Set Interface as down
		netif_set_down(&lwip_netif);
		// Print new link state
		LwIP_Printf("Ethernet Unplugged");
	}
}

/* Management functions --------------------------------------------------------------------*/
/**
* @brief  LwIP periodic tasks
* @param  localtime the current LocalTime value in ms
* @retval None
*/
void LwIP_Periodic_Handle(volatile uint32_t localtime)
{
	// TCP periodic process every 250 ms
#if LWIP_TCP
	if (localtime - TCPTimer >= TCP_TMR_INTERVAL)
	{
		TCPTimer =  localtime;
		tcp_tmr();
	}
#endif
	// ARP periodic process every 5s
	if ((localtime - ARPTimer) >= ARP_TMR_INTERVAL)
	{
		ARPTimer =  localtime;
		etharp_tmr();
	}
	// Fine DHCP periodic process every 500ms
#ifdef USE_DHCP
	if (localtime - DHCPfineTimer >= DHCP_FINE_TIMER_MSECS)
	{
		DHCPfineTimer =  localtime;
		dhcp_fine_tmr();
		if ((DHCP_state != DHCP_ADDRESS_ASSIGNED) &&
			(DHCP_state != DHCP_TIMEOUT) &&
			(DHCP_state != DHCP_LINK_DOWN))
		{
			// process DHCP state machine
			LwIP_DHCP_Process_Handle();
		}
	}
	// DHCP Coarse periodic process every 60s
	if (localtime - DHCPcoarseTimer >= DHCP_COARSE_TIMER_MSECS)
	{
		DHCPcoarseTimer =  localtime;
		dhcp_coarse_tmr();
	}
#endif
}

#ifdef USE_DHCP
/**
* @brief  LwIP_DHCP_Process_Handle
* @param  None
* @retval None
*/
void LwIP_DHCP_Process_Handle()
{
	struct ip_addr ipaddr, netmask, gw;
	uint8_t iptab[4] = {0};
	// State Machine
	switch (DHCP_state)
	{
		case DHCP_START:
			DHCP_state = DHCP_WAIT_ADDRESS;
			dhcp_start(&lwip_netif);
			// IP address should be set to 0
			// every time we want to assign a new DHCP address
			IPaddress = 0;
			LwIP_Printf("Looking for DHCP server...");
			break;
		case DHCP_WAIT_ADDRESS:
			// Read the new IP address
			IPaddress = lwip_netif.ip_addr.addr;
			// Got it
			if (IPaddress!=0)
			{
				DHCP_state = DHCP_ADDRESS_ASSIGNED;
				// Stop DHCP
				dhcp_stop(&lwip_netif);
				iptab[0] = (uint8_t)(IPaddress >> 24);
				iptab[1] = (uint8_t)(IPaddress >> 16);
				iptab[2] = (uint8_t)(IPaddress >> 8);
				iptab[3] = (uint8_t)(IPaddress);
				LwIP_Printf("IP address assigned by a DHCP server: %d.%d.%d.%d", iptab[3], iptab[2], iptab[1], iptab[0]);
			}
			else
			{
				// Verify DHCP timeout */
				if (lwip_netif.dhcp->tries > MAX_DHCP_TRIES)
				{
					DHCP_state = DHCP_TIMEOUT;
					// Stop DHCP
					dhcp_stop(&lwip_netif);

					// Static address used
					IP4_ADDR(&ipaddr, IP_ADDR0 ,IP_ADDR1 , IP_ADDR2 , IP_ADDR3 );
					IP4_ADDR(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
					IP4_ADDR(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
					netif_set_addr(&lwip_netif, &ipaddr , &netmask, &gw);
					LwIP_Printf("DHCP timeout, Static IP address: %d.%d.%d.%d", IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
				}
			}
			break;
		default:
			break;
	}
}
#endif

