/**
 *  \file new 1
 *  \brief Brief
 */

/* Includes ---------------------------------------------------------- */
#include "system.h"
#include "sys_config.h"
#include "dig_led.h"
#include "RandomMAC.h"

#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS
#include "netconf.h"
#include "eips_main.h"
#elif WS_FIELDBUS_TYPE == FIELDBUS_TYPE_NONE
#include "netconf.h"
#elif WS_FIELDBUS_TYPE == FIELDBUS_TYPE_PNIO
extern void pnio_app_init(void);
extern void pnio_process(void);
extern void pnio_app_iodata_update(void);
#endif /* WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS */

#if WS_FIELDBUS_TYPE != FIELDBUS_TYPE_BL
#include "httpd.h"
extern uint32_t test_time_tick_in_http;
#endif

#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_BL
#include "ethernet.h"
#include "upgrader.h"
#include "tftpserver.h"
#include "tftputils.h"
#include "ff.h"
#include <string.h>
#include <stdint.h>
#endif

GPIO_pinInfo pin_swbtn_1;
GPIO_pinInfo pin_swbtn_2;

#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_NONE
GPIO_pinInfo pin_din_1;
GPIO_pinInfo pin_din_2;
GPIO_pinInfo pin_din_3;
GPIO_pinInfo pin_din_4;
void ws_dido_update (void);
#endif

BICOLOR_LED_T sys_led_ms;
BICOLOR_LED_T sys_led_ns;



/* Private declaration ------------------------------------------------ */
void 	system_init_leds (void);
void    system_init_network(void);
void 	USER_TIMER_TEST(void);
void 	ws_eip_led_ctrl (void);

#ifdef __USE_LAUNCH_PAD
void	USER_SW_Test(void);
#endif  /* __USE_LAUNCH_PAD */

/* Generic functions -----------------------------------------------------------------------*/
/**
  * @brief	Main loop for periodic tasks.
  * @param	None.
  * @retval	None.
  */
void system_loop()
{
	if (TickLoop_IsActivated() == 0) return;

#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_NONE
	TickLoop_PeriodicalCall(ws_dido_update, 100, 0);
	TickLoop_PeriodicalCall(ws_process, WS_PROCESS_RUN_PERIOD, 1);
#elif WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS
	TickLoop_PeriodicalCall(ws_eip_led_ctrl, 100, 0);
	TickLoop_PeriodicalCall(lwip_link_monitor, 1, 0);
	TickLoop_PeriodicalCall(eips_process_loop, 10, 0);
	TickLoop_PeriodicalCall(ws_process, WS_PROCESS_RUN_PERIOD, 0);
	TickLoop_PeriodicalCallAtIdle(ws_dig_led_update_daemon, WS_DIG_LED_UPDATE_PERIOD, 1);
#elif WS_FIELDBUS_TYPE == FIELDBUS_TYPE_PNIO
	TickLoop_PeriodicalCall(pnio_process, 8, 0);
	TickLoop_PeriodicalCall(pnio_app_iodata_update, 8, 0);
	TickLoop_PeriodicalCall(ws_process, WS_PROCESS_RUN_PERIOD, 0);
	TickLoop_PeriodicalCallAtIdle(ws_dig_led_update_daemon, WS_DIG_LED_UPDATE_PERIOD, 1);
#elif WS_FIELDBUS_TYPE == FIELDBUS_TYPE_BL
    TickLoop_PeriodicalCall(EthernetLoop_UpdateLink, 1, 0);
    TickLoop_PeriodicalCall(EthernetLoop_TCP_Process, 1, 0);
    TickLoop_PeriodicalCall(upgrader_process, UPGRADER_PROCESS_INTERVAL, 1);
#else
#warning "A wrong fieldbus definition has been detected!"
#endif
}

/**
  * @brief	Initialize the system modules.
  * @param	None.
  * @retval	None.
  */
void system_init()
{
	// Init the bsp.
	BSP_Init();
	
    // Init LEDs.
    system_init_leds();

    // Init digtal led module
    dig_led_init();
#if WS_FIELDBUS_TYPE != FIELDBUS_TYPE_BL
    // Init application modules
	flowsensor_init();
	ws_init();

	// Init network module
    system_init_network();
#endif
#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_BL
#ifdef __USE_LAUNCH_PAD
    USER_SW_Test();
#endif  /* __USE_LAUNCH_PAD */
    upgrader_init(&g_upgrader);
    // Initialize Ethernet module
    Ethernet_Init();
    tftpd_init();
#endif

    // always set MS LED to STEADY_GRN during init.
	GPIO_TagWrite(GPIOTag_LED_2_RED, 1);
    GPIO_TagWrite(GPIOTag_LED_2_GRN, 0);
}

/**
 *  @brief Init the network functions used in WS
 *  @return none
 *  @details network here includes possibly 
 *              - basic ip parameters
 *              - basic lwip init
 *              - tcp init
 *              - eips/pnio/http/tftp/...
 */
void system_init_network(void)
{
    gen_mac_addr();
#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS || WS_FIELDBUS_TYPE == FIELDBUS_TYPE_NONE
    // Init basic ethernet system
	Ethernet_InitMACPHYDMA();
	// Init lwIP system and its app (http, tftp)
	lwip_user_init();
#elif WS_FIELDBUS_TYPE == FIELDBUS_TYPE_PNIO
	pnio_app_init();
#endif
#if WS_FIELDBUS_TYPE != FIELDBUS_TYPE_BL
	// Init http for webserver.
    httpd_init();
#endif

#ifdef USE_TFTP
    tftp_init();
#endif /* USE_TFTP */
    
    // Init network IO
#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS
    eips_process_init();
#elif WS_FIELDBUS_TYPE == FIELDBUS_TYPE_PNIO
#endif /* WS_FIELDBUS_TYPE */
}

/**
 *  @brief init system led, including led for dido and led for ms&ns
 *  @return none
 *  @details DO is actually defined as GPIOTag_OB_LED_x (DO and its led are controlled by the same pin)
 */
void system_init_leds (void)
{
	// the following 4 are for 2 bi-color LEDS, mainly used as field indicator.
	GPIO_TagConfigProperties(GPIOTag_LED_1_GRN, GPIO_SET_OUT_PUSHPULL, GPIO_SPD_LOW);
	GPIO_TagConfigProperties(GPIOTag_LED_1_RED, GPIO_SET_OUT_PUSHPULL, GPIO_SPD_LOW);
	GPIO_TagConfigProperties(GPIOTag_LED_2_GRN, GPIO_SET_OUT_PUSHPULL, GPIO_SPD_LOW);
	GPIO_TagConfigProperties(GPIOTag_LED_2_RED, GPIO_SET_OUT_PUSHPULL, GPIO_SPD_LOW);

	// the following 2 LED can be controlled by integrated PHY.
	GPIO_TagConfigPropertiesAF(GPIOTag_LED_3, GPIO_SET_AF_PUSHPULL, GPIO_SPD_FAST, GPIO_PF0_EN0LED0);
	GPIO_TagConfigPropertiesAF(GPIOTag_LED_4, GPIO_SET_AF_PUSHPULL, GPIO_SPD_FAST, GPIO_PF4_EN0LED1);

	// the following 4 are for DOUT.
	GPIO_TagConfigProperties(GPIOTag_OB_LED_1, GPIO_SET_OUT_PUSHPULL, GPIO_SPD_LOW);
	GPIO_TagConfigProperties(GPIOTag_OB_LED_2, GPIO_SET_OUT_PUSHPULL, GPIO_SPD_LOW);
	GPIO_TagConfigProperties(GPIOTag_OB_LED_3, GPIO_SET_OUT_PUSHPULL, GPIO_SPD_LOW);
	GPIO_TagConfigProperties(GPIOTag_OB_LED_4, GPIO_SET_OUT_PUSHPULL, GPIO_SPD_LOW);

	// Configure the pins used for DI.
	// So far the used pin is DIN1, DIN2, DIN3.
#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_NONE
	GPIO_TagConfigProperties(GPIOTag_DIN_1, GPIO_SET_IN_PULLDOWN, GPIO_SPD_MID);
	GPIO_TagConfigProperties(GPIOTag_DIN_2, GPIO_SET_IN_PULLDOWN, GPIO_SPD_MID);
	GPIO_TagConfigProperties(GPIOTag_DIN_3, GPIO_SET_IN_PULLDOWN, GPIO_SPD_MID);
	//GPIO_TagConfigProperties(GPIOTag_DIN_4, GPIO_SET_IN_PULLDOWN, GPIO_SPD_MID);

	pin_din_1 = GPIO_ConfigInterrupt(GPIOTag_DIN_1, GPIO_TRG_RISING_FALLING, INT_GPIOE);
	pin_din_2 = GPIO_ConfigInterrupt(GPIOTag_DIN_2, GPIO_TRG_RISING_FALLING, INT_GPIOE);
	pin_din_3 = GPIO_ConfigInterrupt(GPIOTag_DIN_3, GPIO_TRG_RISING_FALLING, INT_GPIOE);
	//pin_din_4 = GPIO_ConfigInterrupt(GPIOTag_DIN_4, GPIO_TRG_RISING_FALLING, INT_GPIOE);
#endif /* WS_FILEDBUS_NONE_BUT_DIDO */

	// init the properties of bi-color LED.
#ifndef __USE_LAUNCH_PAD
	led_bicolor_construct(&sys_led_ms, GPIOTag_LED_2_RED, GPIOTag_LED_2_GRN, LED_BI_STATE_STEADY_GREEN, 500, 500);
	led_bicolor_construct(&sys_led_ns, GPIOTag_LED_1_RED, GPIOTag_LED_1_GRN, LED_BI_STATE_OFF, 500, 500);
#else /* __USE_LAUNCH_PAD */
	//led_bicolor_construct(&pnio_led_ns, GPIOTag_LPD_D1, GPIOTag_LPD_D2, LED_BI_STATE_OFF, 500, 500);
#endif /* __USE_LAUNCH_PAD */

	// init all LED to off.
    // bi-color led is low active in our system.
	GPIO_TagWrite(GPIOTag_LED_1_GRN, 1);
	GPIO_TagWrite(GPIOTag_LED_1_RED, 1);
	GPIO_TagWrite(GPIOTag_LED_2_GRN, 1);
	GPIO_TagWrite(GPIOTag_LED_2_RED, 1);
	GPIO_TagWrite(GPIOTag_OB_LED_1, 0);
	GPIO_TagWrite(GPIOTag_OB_LED_2, 0);
	GPIO_TagWrite(GPIOTag_OB_LED_3, 0);
	GPIO_TagWrite(GPIOTag_OB_LED_4, 0);
}


#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_NONE
/**
 *  @brief update do as per current ws paras.
 *  @return none
 *  @details the used io pins are actually DIN1,2,4, DOUT1,2,4.
 */
void ws_dido_update ()
{
	// DI are read and updated in GPIO_ISR.

	// update DO accordingly.
	GPIO_TagWrite(GPIOTag_OB_LED_1, ws_o_is_oktoweld);
	GPIO_TagWrite(GPIOTag_OB_LED_2, ws_o_is_flow_warning);
	GPIO_TagWrite(GPIOTag_OB_LED_3, ws_o_is_leak_detected);
}
#endif /* ENABLE_DIDO_IN_WS || WS_FILEDBUS_NONE_BUT_DIDO */




/**
 *  @brief control led of ms and ns
 *  @return none
 *  @details to be polled in system_loop
 */
void ws_eip_led_ctrl ()
{
#ifndef __USE_LAUNCH_PAD
	//bicolor_led_control(&sys_led_ms);
#endif /* __USE_LAUNCH_PAD */
	bicolor_led_control(&sys_led_ns);
}

#if WS_FIELDBUS_TYPE != FIELDBUS_TYPE_BL
/**
 *  @brief Simply for test.
 *  @return none
 *  @details test_time_tick_in_http +1 every sec and shown in webserver.
 */
void 	USER_TIMER_TEST()
{
	test_time_tick_in_http++;
}
#endif

#ifdef __USE_LAUNCH_PAD
/**
 *  @brief Read and store the input of on/off button on TIVA LaunchPad.
 *  @return none
 *  @details only for test in launch pad
 */
void USER_SW_Test(void)
{
	GPIO_TagConfigProperties(GPIOTag_SWBTN_1, GPIO_SET_IN_PULLUP, GPIO_SPD_MID);
	GPIO_TagConfigProperties(GPIOTag_SWBTN_2, GPIO_SET_IN_PULLUP, GPIO_SPD_MID);

	pin_swbtn_1 = GPIO_ConfigInterrupt(GPIOTag_SWBTN_1, GPIO_TRG_FALLING, INT_GPIOJ);
	pin_swbtn_2 = GPIO_ConfigInterrupt(GPIOTag_SWBTN_2, GPIO_TRG_RISING, INT_GPIOJ);
}
#endif /* __USE_LAUNCH_PAD */


/**
 *  @brief GPIOJ ISR
 *  @return none
 *  @details 
 *   - GPIOJ is where TIVA LaunchPad's user_switch_buttons are connected.   
 *   - only used on TIVA LaunchPad.
 *   - here we reset the IP address to default 192.168.125.67
 */
void GPIOJ_ISR (void) {
#ifdef __USE_LAUNCH_PAD
	if (HWREG(GPIO_PORTJ_BASE + 0x00000418) & 0x00000001) {
		//GPIO_TagToggle(GPIOTag_USER_LED_1);
		GPIOIntClear(GPIO_PORTJ_BASE, 0x00000001);

		EEPROMProgram ((uint32_t*)&def_ipaddr_0, EEPROMAddrFromBlock(2), 8);
		EEPROMProgram ((uint32_t*)&def_ipaddr_1, EEPROMAddrFromBlock(2)+8, 8);
		EEPROMProgram ((uint32_t*)&def_ipaddr_2, EEPROMAddrFromBlock(2)+16, 8);
		EEPROMProgram ((uint32_t*)&def_ipaddr_3, EEPROMAddrFromBlock(2)+24, 8);
	}
	else if (HWREG(GPIO_PORTJ_BASE + 0x00000418) & 0x00000002) {
		//GPIO_TagToggle(GPIOTag_USER_LED_2);
		GPIOIntClear(GPIO_PORTJ_BASE, 0x00000002);
	}
	else return;
#endif /* __USE_LAUNCH_PAD */
}


/**
 *  @brief respond to DI if activated
 *  @return none
 *  @details serve for DIN/DOUT untility
 */
void GPIOE_ISR (void) {
#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_NONE
	if (HWREG(GPIO_PORTE_BASE + 0x00000418) & 0x00000001) {
		GPIOIntClear(GPIO_PORTE_BASE, 0x00000001);
		ws_i_bus_reset = GPIO_TagRead(GPIOTag_DIN_1);
	}
	else if (HWREG(GPIO_PORTE_BASE + 0x00000418) & 0x00000002) {
		GPIOIntClear(GPIO_PORTE_BASE, 0x00000002);
		ws_i_bus_valveon = GPIO_TagRead(GPIOTag_DIN_2);
	}
	else if (HWREG(GPIO_PORTE_BASE + 0x00000418) & 0x00000004) {
		GPIOIntClear(GPIO_PORTE_BASE, 0x00000004);
		ws_i_bus_bypass = GPIO_TagRead(GPIOTag_DIN_3);
	}
	else if (HWREG(GPIO_PORTE_BASE + 0x00000418) & 0x00000008) {
		GPIOIntClear(GPIO_PORTE_BASE, 0x00000008);
	}
	else return;
#endif /* WS_FIELDBUS_TYPE == FIELDBUS_TYPE_NONE */
}
