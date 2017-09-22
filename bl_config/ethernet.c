/**
 *  \file new 1
 *  \brief Brief
 */

/* Includes --------------------------------------------------------------------------------*/
#include "ethernet.h"     	// header file
#include "timeout.h"     	// Timeout header file
#include "sys_config.h"
#include <stdio.h>
#include <bsp_eeprom_const.h>

extern char ws_o_ipaddr_text[20];


// exported variables
// the flag indicating eip communication status.
// inited to 1 during system init bc eip com has not been established yet.
uint8_t flag_fieldbus_down = 1;

/* Private declarations --------------------------------------------------------------------*/
void Ethernet_InitMACPHYDMA(void);
void Ethernet_UpdateLink(void);
void eips_userobj_callback(uint8_t nEvent, uint16_t nInst, uint8_t *pDataBuf, uint16_t nDataLen);

/* Initialization functions ----------------------------------------------------------------*/
/**
  * @brief	Initialize the Ethernet hardware.
  * @param	none.
  * @retval	none.
  */
void Ethernet_Init()
{
	uint8_t is_new_ip_saved_before = 0;

	// Detailed initializations
	Ethernet_InitMACPHYDMA();		// MAC, PHY and DMA

	// read IP from EEPROM.
	EEPROMRead ((uint32_t*)&uip_add_0, EEPROM_ADDR_IP_ADDR0, 4);
	EEPROMRead ((uint32_t*)&uip_add_1, EEPROM_ADDR_IP_ADDR1, 4);
	EEPROMRead ((uint32_t*)&uip_add_2, EEPROM_ADDR_IP_ADDR2, 4);
	EEPROMRead ((uint32_t*)&uip_add_3, EEPROM_ADDR_IP_ADDR3, 4);
	EEPROMRead ((uint32_t*)&is_new_ip_saved_before, EEPROM_ADDR_NEW_IP_IS_SAVED, 4);
//	if (uip_add_0 == 0 || uip_add_1 ==0 || uip_add_2 == 0 || uip_add_3 == 0 || is_new_ip_saved_before != 167) {
//		uip_add_0 = 192;
//		uip_add_1 = 168;
//		uip_add_2 = 125;
//		uip_add_3 = 67;
//	}
	uip_add_0 = 172;
	uip_add_1 = 24;
	uip_add_2 = 1;
	uip_add_3 = 1;
//	uip_add_0 = 192;
//	uip_add_1 = 168;
//	uip_add_2 = 125;
//	uip_add_3 = 67;

	LwIP_Init();
}
/**
  * @brief	Ethernet timer interrupt handler.
  * 		It manages the loop frequency.
  * @param	none.
  * @retval	none.
  */
void ETH_TIM_HAND()
{
	// Verify that the right interrupt has occurred
	if (!TIMER_GetAndClearInt(ETH_TIM)) return;
	// Call Ethernet_Loop() for compatibility with not-interrupted mode
	//Ethernet_Loop();
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
//    HWREGH(0x44054000 + 0x018) &= 0xFE00;	// invert the PHY led polarity
//    HWREGH(0x44054000 + 0x018) |= 0x10;	// invert the PHY led polarity
//    char temp_str[100] = {' '};
//    int temp_str_len = 0;
//    temp_str_len = sprintf (temp_str, "%d", HWREGH(0x44054000 + 0x018));
//    puts(temp_str);
    uint16_t temppol = 0;
    temppol = HWREGH(0x44054000 + 0x018);
    HWREGH(0x44054000 + 0x018) &= 0xFE00;
    temppol = HWREGH(0x44054000 + 0x018);
#else /* USE_XM4C129ENCPDT_CHIP not defined */
#ifndef __USE_LAUNCH_PAD
    HWREG(EMAC0_BASE+ EMAC_O_CC) |= EMAC_CC_POL;	// invert the PHY led polarity
#endif
#endif /* USE_XM4C129ENCPDT_CHIP */

    // Program the hardware with its MAC address (for filtering).
    uint8_t MAC_ADDR[6] = {0x00,0x1A,0xB6,0x03,0x20,0x12};

//    gen_mac_addr();
//    MAC_ADDR[0] =  get_mac_addr(0);//MAC_ADDR0;
//    MAC_ADDR[1] =  get_mac_addr(1);//MAC_ADDR1;
//    MAC_ADDR[2] =  get_mac_addr(2);//MAC_ADDR2;
//    MAC_ADDR[3] =  get_mac_addr(3);//MAC_ADDR3;
//    MAC_ADDR[4] =  get_mac_addr(4);//MAC_ADDR4;
//    MAC_ADDR[5] =  get_mac_addr(5);//MAC_ADDR5;
//    MAC_ADDR[0] =  l_mac[0];//MAC_ADDR0;
//    MAC_ADDR[1] =  l_mac[1];//MAC_ADDR1;
//    MAC_ADDR[2] =  l_mac[2];//MAC_ADDR2;
//    MAC_ADDR[3] =  l_mac[3];//MAC_ADDR3;
//    MAC_ADDR[4] =  l_mac[4];//MAC_ADDR4;
//    MAC_ADDR[5] =  l_mac[5];//MAC_ADDR5;
    EMACAddrSet(EMAC0_BASE, 0, MAC_ADDR);

    // Set the interrupt priority to peripheral
    IntPrioritySet(INT_EMAC0, INTERRUPT_PRIORITY_LOW);
}

/* Loop functions --------------------------------------------------------------------------*/
extern void SYSTEM_UpdateEIPSOutput(void);
/**
  * @brief	Allow Ethernet to work properly.
  * @param	none.
  * @retval	none.
  */
//void Ethernet_Loop()
//{
//	// Update Ethernet link status
//	Ethernet_UpdateLink();
//	// Handle any received frame as soon as they arrive...For TivaC we use Ethernet_IntHandler!
//	//if (ETH_CheckFrameReceived()) LwIP_Pkt_Handle(); // Left here just for reference
//	// Loop TCP Console to send data as soon as there is any
//	tcp_console_loop();
//	// Other Ethernet functions are handled every 10ms
//	TIMEOUT_CallPeriodically(
//	{
//		ATOMIC_BLOCK(
//			// Loop LwIP
//			LwIP_Periodic_Handle(Time_GetMs());
//			// Send EIPS_MSG_Output and loop EIPS
//			SYSTEM_UpdateEIPSOutput();
//			eips_userobj_data_send(EIPS_MSG_Output.row, EIPS_MSG_Output_Size);
//			eips_rtasys_process(10); // signal 10ms between calls
//		);
//	}, 10);
//}
void EthernetLoop_UpdateLink ()
{
	Ethernet_UpdateLink();
	//tcp_console_loop();
}

void EthernetLoop_TCP_Process ()
{
	LwIP_Periodic_Handle(Time_GetMs());
}

void EthernetLoop_EIP_Process ()
{

}


/**
  * @brief  Update Ethernet link status of MC.
  * 		Try to force Auto-Negotiation, and deal with MAC and LwIP.
  * @param  None.
  * @retval None.
  */
void Ethernet_UpdateLink()
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
			// Start LwIP
			LwIP_Set_Link_State(1);
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
		    LwIP_Set_Link_State(0);
		    // Reset MII Register
			EMACPHYWrite(EMAC0_BASE, PHY_PHYS_ADDR, EPHY_BMCR, (EPHY_BMCR_MIIRESET));
			// Update LINK status
		    storedLinkStatus = 0;
		}
	}
}


/* HTTPD functions -------------------------------------------------------------------------*/
/**
  * @brief	Deals with requests coming from the WebServer.
  * @param	None.
  * @retval	The number of char used.
  */
uint32_t Ethernet_RequestFromBrowser(const char * request, char * answer)
{
	return 0;
}
