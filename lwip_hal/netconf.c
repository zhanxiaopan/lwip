/**
 *  @file netconf.c
 *  @brief Complete the setup of ethernet and lwip. referred to Matteo Russi's codes.
 *  @history
 *      created@20170209 by wenlong li
 *  
 */
/* Includes ---------------------------------------------------------- */
#include "bsp_TIMER.h"
#include "netconf.h"					// header file
#include "driverlib/emac.h"				// Ethernet Module Header file
#include "inc/hw_emac.h"
#include "inc/hw_nvic.h"
#include "RandomMAC.h"

/* Private declarations ------------------------------------------------ */
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


/* Exported variables ------------------------------------------------- */
// global ip addr variables
uint8_t uip_add_0 = 172;
uint8_t uip_add_1 = 24;
uint8_t uip_add_2 = 1;
uint8_t uip_add_3 = 6;


/* Exported functions ------------------------------------------------- */
/**
  * @brief	Initialize the lwIP stack.
  * @param	none
  * @retval	none.
  */
void lwip_user_init(void)
{
	// config IP addr.
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

	// Initializes the LwIP modules
    lwip_init();

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
  * @brief	Initialize MAC, PHY and DMA.
  * @param	none
  * @retval	none.
  */
void Ethernet_InitMACPHYDMA()
{
	// Initialize Ethernet LINK/ACT LEDs
#ifdef ETH_LEDS_USED
	GPIO_TagConfigPropertiesAF(GPIOTag_LED_ETH_LNK, GPIO_SET_AF_PUSHPULL, GPIO_SPD_FAST, ETH_LED_AF_LNK);
	GPIO_TagConfigPropertiesAF(GPIOTag_LED_ETH_ACT, GPIO_SET_AF_PUSHPULL, GPIO_SPD_FAST, ETH_LED_AF_ACT);
#endif
    // Enable and reset the ethernet peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EMAC0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_EMAC0);
    // Verify that internal PHY is present
    if(!SysCtlPeripheralPresent(SYSCTL_PERIPH_EPHY0))
    {
        // Internal PHY is not present on this device so hang here.
        while(1);
    }
    // Enable and reset the PHY peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EPHY0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_EPHY0);
    // Wait for the MAC to come out of reset.
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EMAC0));
    // Configure PHY
    EMACPHYConfigSet(EMAC0_BASE, EMAC_PHY_TYPE_INTERNAL |
    							 EMAC_PHY_INT_MDIX_EN |
								 EMAC_PHY_AN_100B_T_FULL_DUPLEX);
    // Initialize the MAC and set the DMA mode
    EMACInit(EMAC0_BASE, SystemCoreClock,
    		 EMAC_BCONFIG_MIXED_BURST | EMAC_BCONFIG_PRIORITY_FIXED,
			 4, 4, 0);
    // Set MAC configuration options.
    EMACConfigSet(EMAC0_BASE, (EMAC_CONFIG_FULL_DUPLEX |
    							   EMAC_CONFIG_CHECKSUM_OFFLOAD |
								   EMAC_CONFIG_7BYTE_PREAMBLE |
								   EMAC_CONFIG_IF_GAP_96BITS |
								   EMAC_CONFIG_USE_MACADDR0 |
								   EMAC_CONFIG_SA_FROM_DESCRIPTOR |
								   EMAC_CONFIG_BO_LIMIT_1024),
    							  (EMAC_MODE_RX_STORE_FORWARD |
    							   EMAC_MODE_TX_STORE_FORWARD |
								   EMAC_MODE_TX_THRESHOLD_64_BYTES |
								   EMAC_MODE_RX_THRESHOLD_64_BYTES),
								   0);

#ifdef USE_XM4C129ENCPDT_CHIP
    uint16_t temppol = 0;
    temppol = HWREGH(0x44054000 + 0x018);
    HWREGH(0x44054000 + 0x018) &= 0xFE00;
    temppol = HWREGH(0x44054000 + 0x018);
#else /* USE_XM4C129ENCPDT_CHIP not defined */
    HWREG(EMAC0_BASE+ EMAC_O_CC) |= EMAC_CC_POL;	// invert the PHY led polarity
#endif /* USE_XM4C129ENCPDT_CHIP */

    // Program the hardware with its MAC address (for filtering).
    uint8_t MAC_ADDR[6];
    gen_mac_addr();
    MAC_ADDR[0] =  l_mac[0];
    MAC_ADDR[1] =  l_mac[1];
    MAC_ADDR[2] =  l_mac[2];
    MAC_ADDR[3] =  l_mac[3];
    MAC_ADDR[4] =  l_mac[4];
    MAC_ADDR[5] =  l_mac[5];
    EMACAddrSet(EMAC0_BASE, 0, MAC_ADDR);

    // Set the interrupt priority to peripheral
    IntPrioritySet(INT_EMAC0, INTERRUPT_PRIORITY_LOW);
}



/**
  * @brief	Ethernet timer interrupt handler.
  * 		We just clear the int flag in current implementation.
  * @param	none.
  * @retval	none.
  */

void ETH_TIM_HAND(void)
{
	// Verify that the right interrupt has occurred
	if (!TIMER_GetAndClearInt(ETH_TIM)) return;
}


// Get EIPS connection timeout flag
//extern uint8_t eips_cnxn_timeout_flag;
/**
 *  @brief  Update Ethernet link status of MC.
 * 		    Try to force Auto-Negotiation, and deal with MAC and LwIP.
 *  @return None
 *  @details In a system without RTOS and has AUTOIP or DHCP not enabled,
 *           such check seems to be meaningless.
 */
void lwip_link_monitor()
{
	// Track linkStatus
	static uint8_t storedLinkStatus = 0;
	// Verify actual link state
	uint16_t regValue = EMACPHYRead(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_STS);
	if ((regValue & EPHY_STS_LINK) || (regValue & EPHY_STS_ANS))
	{
		//-----------------------------------------------------------------------
		// -- LINK is UP --------------------------------------------------------
		if (storedLinkStatus!=1)	// Link was DOWN
		{
			// Reset EIPS connection timeout flag
//			eips_cnxn_timeout_flag = 0;
			// Start LwIP
			lwip_set_link_state(1);
			// Update LINK status
			storedLinkStatus = 1;
		}
	}
	else
	{
		//-----------------------------------------------------------------------
		// -- LINK is DOWN ------------------------------------------------------
		if (storedLinkStatus != 0)	// Link was UP
		{
		    // Stop LwIP
			lwip_set_link_state(0);
		    // Reset MII Register
			EMACPHYWrite(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_BMCR, (EPHY_BMCR_MIIRESET));
			// Update LINK status
		    storedLinkStatus = 0;
		}
	}
}


/**
 *  @brief Periodical task of TCP based on lwIP.
 *  @return None
 *  @details Details
 */
void lwip_process ()
{
	lwip_periodic_handle(Time_GetMs());
}


/**
  * @brief	Handles Ethernet interrupts for the lwIP TCP/IP stack.
  * @param	none.
  * @retval	none.
  * @details this int handler would be triggered by various ETH trigger src
  */
void Ethernet_IntHandler(void)
{
    uint32_t ui32Status;

    // Read and Clear the interrupt status
    ui32Status= EMACIntStatus(EMAC0_BASE, true);
    if (ui32Status) {
    	EMACIntClear(EMAC0_BASE, ui32Status);
    }

    // If a transmit/receive interrupt was active,
    // call the low-level interrupt handler
    if (ui32Status) {
    	ethernetif_interrupt(&lwip_netif, ui32Status);
    }

    // Call lwip timer functions.
    lwip_periodic_handle(Time_GetMs());
}


/**
  * @brief	Generate a ETH interrupt.
  * @param	none.
  * @retval	none.
  * @details This should be called in SYSTICK INT handler periodically.
  */
void lwip_timer(void) {
    //
    // Generate an Ethernet interrupt.  This will perform the actual work
    // of checking the lwIP timers and taking the appropriate actions.  This is
    // needed since lwIP is not re-entrant, and this allows all lwIP calls to
    // be placed inside the Ethernet interrupt handler ensuring that all calls
    // into lwIP are coming from the same context, preventing any reentrancy
    // issues.  Putting all the lwIP calls in the Ethernet interrupt handler
    // avoids the use of mutexes to avoid re-entering lwIP.
    //
    HWREG(NVIC_SW_TRIG) |= INT_EMAC0 - 16;
}

/**
  * @brief	Set the Link state of the lwip_netif network interface.
  * @param	linkState: the actual link state (1 link, 0 NO link)
  * @retval	none.
  */
void lwip_set_link_state(uint8_t linkState)
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

/**
 *  @brief LwIP periodic tasks
 *  @param [in] localtime Parameter_Description
 *  @return Return_Description
 *  @details This function services all of the lwIP periodic timers, including ARP and
 *           TCP.  This should be called from the lwIP context, which may be
 *           the Ethernet interrupt (in the case of a non-RTOS system) or the lwIP
 *           thread, in the event that an RTOS is used.
 */
void lwip_periodic_handle(volatile uint32_t localtime)
{
    // ARP periodic process every 5s
	if ((localtime - ARPTimer) >= ARP_TMR_INTERVAL)
	{
		ARPTimer =  localtime;
		etharp_tmr();
	}
    
	// TCP periodic process every 250 ms
#if LWIP_TCP
	if (localtime - TCPTimer >= TCP_TMR_INTERVAL)
	{
		TCPTimer =  localtime;
		tcp_tmr();
	}
#endif

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

