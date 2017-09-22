/**
 *  \file new 1
 *  \brief Brief
 */

#ifndef __BSP_CONFIG_H_
#define __BSP_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes --------------------------------------------------------------------------------*/
#include <stddef.h>					// NULL definitions
#include <stdbool.h>				// Boolean definitions, it is required by TivaWare, but it is NEVER used in this bsp elsewhere
#include <stdint.h>					// Integer definitions
#include "inc/hw_memmap.h"			// Device memory map definition
#include "inc/hw_types.h"			// Helper for hardware acess(thumb) and silicon revision
#include "driverlib/sysctl.h"		// System Control Header
#include "driverlib/fpu.h"			// FPU Header
#include "driverlib/systick.h"		// SisTick Header
#include "inc/hw_ints.h"			// Interrupts definition
//#include "inc/tm4c1294ncpdt.h"      // ** use this instead of hw_ints.h
#include "driverlib/interrupt.h"	// Interrupts Header
#include "driverlib/gpio.h"			// GPIO Header
#include "driverlib/pin_map.h"		// GPIO pin map definition
#include "driverlib/eeprom.h"		// EEPROM Header
#include "driverlib/udma.h"			// uDMA Header

/* Configurations --------------------------------------------------------------------------*/
#define APP_NAME "WSV1"
#define APP_NAME_EXTENDED "Weld Server V1"
#define APP_VERSION_MAJ 0
#define APP_VERSION_MID 0
#define APP_VERSION_MIN 3
// Uncomment if you want to use the ethernet Link-Akt leds
//#define ETH_LEDS_USED

/* Global Definitions ----------------------------------------------------------------------*/
// Execute a block atomically
#define ATOMIC_BLOCK(block) do {uint32_t intMaskBit = IntMasterDisable(); block; if (!intMaskBit) IntMasterEnable();} while(0)
// Execute a block once
#define RUN_ONCE(block) do {static uint8_t flag = 0; if (flag==0){flag=1;block;};} while(0)
// Store the System clock
uint32_t SystemCoreClock;
/**
  * @brief	Available Interrupt priority
  * 		The top 3 bits of these values are significant with
  * 		lower values indicating higher priority interrupts
  */
typedef enum
{
	INTERRUPT_PRIORITY_SYS =	0x80,		// Cannot be interrupted, use just for system calls (such as SysTick)
	INTERRUPT_PRIORITY_PHER =	0xA0,		// Can be interrupted just by system calls, use it for peripherals
	INTERRUPT_PRIORITY_HIGH =	0xC0,		// Use for High priority calls that shouldn't be interrupted
	INTERRUPT_PRIORITY_LOW =	0xE0		// Use for the rest of interrupts
} INTERRUPT_PRIORITY_TypeDef;

/* MCU Configuration -----------------------------------------------------------------------*/
/**
  * @brief  Simplified definition of TIMER_A TIMER_B to allow channeling it with TIMER_BASE.
  */
#define BSP_TIMER_16A			1			// If used, please mind that prescaler has not been implemented yet
#define BSP_TIMER_16B			2			// If used, please mind that prescaler has not been implemented yet
#define BSP_TIMER_32			4
/**
  * @brief  Definition for error class.
  */
#define ERR_CLASS_SYSTEM			0x00
#define ERR_CLASS_COMMAND			0x40
#define ERR_CLASS_MOTOR				0x80
#define ERR_CLASS_SSD				0x20 | ERR_CLASS_MOTOR
#define ERR_CLASS_MASK				0x3F
/**
  * @brief	Tag definition for used GPIO pins.
  *			Elements should have a value of GPIO_PORTx_BASE | GPIO_PIN_x
  */
typedef enum
{
	// --- USER LED Tags (Lamp)----------------------------------------------------------
	GPIOTag_LED_1_RED =	GPIO_PORTL_BASE | GPIO_PIN_0,		//PL0, LED1_grn
	GPIOTag_LED_1_GRN =	GPIO_PORTL_BASE | GPIO_PIN_1,		//PL1, LED1_red
	GPIOTag_LED_2_RED =	GPIO_PORTL_BASE | GPIO_PIN_2,		//PL2, LED1_grn
	GPIOTag_LED_2_GRN =	GPIO_PORTL_BASE | GPIO_PIN_3,		//PL3, LED1_red
	GPIOTag_LED_3 =		GPIO_PORTF_BASE | GPIO_PIN_0,		//PF0, LED3
	GPIOTag_LED_4 =		GPIO_PORTF_BASE | GPIO_PIN_4,		//PF4, LED4

	// --- Onboard LED Tags ----------------------------------------------------------
	GPIOTag_OB_LED_1 =		GPIO_PORTN_BASE | GPIO_PIN_0,		//PN0, LED1
	GPIOTag_OB_LED_2 =		GPIO_PORTN_BASE | GPIO_PIN_1,		//PN1, LED2
	GPIOTag_OB_LED_3 =		GPIO_PORTN_BASE | GPIO_PIN_2,		//PN2, LED3
	GPIOTag_OB_LED_4 =		GPIO_PORTN_BASE | GPIO_PIN_3,		//PN3, LED4

	//GPIOTag_LED_SYS_OK =	GPIO_PORTF_BASE | GPIO_PIN_5,		//SYS OK, F5
	//GPIOTag_LED_SYS_FT =	GPIO_PORTF_BASE | GPIO_PIN_4,		//SYS FAULT, F4
	//GPIOTag_LED_ETHIP_OK =	GPIO_PORTN_BASE | GPIO_PIN_0,		//ETHIP OK, S4
	//GPIOTag_LED_ETHIP_FT =	GPIO_PORTN_BASE | GPIO_PIN_1,		//ETHIP FAULT, R6

	// flowsensor reading channel
	GPIOTag_FLOWSENSOR_1 =	GPIO_PORTD_AHB_BASE | GPIO_PIN_0,		//PD0, flow sensor 1 ccp
	GPIOTag_FLOWSENSOR_2 =	GPIO_PORTD_AHB_BASE | GPIO_PIN_1,		//PD1, flow sensor 2 ccp

	// valve control output
	GPIOTag_VALVE_CMD =	GPIO_PORTQ_BASE | GPIO_PIN_4,			//PQ4, valve control

	// only sw test in launch pad
	GPIOTag_SWBTN_1 =		GPIO_PORTJ_BASE | GPIO_PIN_0,		//PJ0, user switch btn 1
	GPIOTag_SWBTN_2 =		GPIO_PORTJ_BASE | GPIO_PIN_1,		//PJ1, user switch btn 1
	//GPIOTag_USER_LED_1 =	GPIO_PORTN_BASE | GPIO_PIN_1,		//User LED 1
	//GPIOTag_USER_LED_2 =	GPIO_PORTN_BASE | GPIO_PIN_0,		//User LED 2

	GPIOTag_DIN_1 =		GPIO_PORTE_BASE | GPIO_PIN_0,		//PE0, DIN1
	GPIOTag_DIN_2 =		GPIO_PORTE_BASE | GPIO_PIN_1,		//PE1, DIN2
	GPIOTag_DIN_3 =		GPIO_PORTE_BASE | GPIO_PIN_2,		//PE2, DIN3
	GPIOTag_DIN_4 =		GPIO_PORTE_BASE | GPIO_PIN_3,		//PE3, DIN4

} GPIOTag_TypeDef;

#define GPIOTag_DOUT_1 (GPIOTag_OB_LED_1)
#define GPIOTag_DOUT_2 (GPIOTag_OB_LED_2)
#define GPIOTag_DOUT_3 (GPIOTag_OB_LED_3)
#define GPIOTag_DOUT_4 (GPIOTag_OB_LED_4)

typedef struct AliveMonitor_TypeDef{
    uint8_t AliveFlag;
    uint32_t TrigThreshold;
    uint32_t TrigCount;
    uint8_t IsAwaken;
} AliveMonitor_TypeDef;


// --- DCBUS Tags -------------------------------------------------------------
#define DCBUS_ADC 			ADC0_BASE
#define DCBUS_CHN 			ADC_CTL_CH1
// --- Console Tags ------------------------------------------------------------
	#define CONS_UART 			UART1_BASE
	#define CONS_UART_AF_TX		GPIO_PB1_U1TX
	#define CONS_UART_AF_RX		GPIO_PB0_U1RX
	#define CONS_UART_IRQ		INT_UART1
	#define CONS_UART_HANDLER	UART1_IntHandler
// --- Ethernet Tags -----------------------------------------------------------
#ifdef ETH_LEDS_USED
	#define ETH_LED_AF_LNK		GPIO_PF0_EN0LED0
	#define ETH_LED_AF_ACT		GPIO_PF4_EN0LED1
#endif
#define ETH_TIM 			TIMER0_BASE | BSP_TIMER_32
#define ETH_TIM_IRQ 		INT_TIMER0A
#define ETH_TIM_HAND		Timer0A_IntHandler
#define ETH_TIM_FREQ		1000				// Frequency of the Ethernet loop = 1kHz = 1ms
#define ETH_TIM_PRIO		INTERRUPT_PRIORITY_LOW
// --- Motor Tags --------------------------------------------------------------
	// Current
	#define MOTOR_CURRENT_ADC 	ADC1_BASE
	#define MOTOR_CURRENT_CHN	ADC_CTL_CH16
	#define MOTOR_CURRCONV_DONE	ADC_ADC1_ConversionDone
	// Outputs
	#define MOTOR_PWM_U_AF 		GPIO_PF3_M0PWM3
	#define MOTOR_PWM_U_CH		3
	#define MOTOR_PWM_V_AF 		GPIO_PR1_M0PWM1
	#define MOTOR_PWM_V_CH		1
	#define MOTOR_PWM_W_AF 		GPIO_PR5_M0PWM5
	#define MOTOR_PWM_W_CH		5
	// Hall sensor
	#define FLOWSENSOR_IRQ 		INT_GPIOD
	#define MOTOR_HALL_HAND		GPIOD_IntHandler
	#define MOTOR_HALL_TIM 		TIMER1_BASE | BSP_TIMER_32
	#define MOTOR_HALL_TIM_IRQ 	INT_TIMER1A
	#define MOTOR_HALL_TIM_HAND	Timer1A_IntHandler
// Motion control
#define MOTOR_CONTROL_TIM	TIMER2_BASE | BSP_TIMER_32
#define MOTOR_CONTROL_FREQ	1000				// Frequency of the motor control loop = 1kHz = 1ms

/**
  * @brief  Macro to enable the clocks of the peripheral used.
  */
#define BSP_ClockEnable() 									\
do                                    						\
{   /* GPIOs */												\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);			\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);			\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);			\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);			\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);			\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);			\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);			\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);			\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOR);			\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOS);			\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);			\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);			\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);			\
	SysCtlPWMClockSet(SYSCTL_PWMDIV_1);						\
	/* EEPROM */											\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);			\
	/* uDMA */												\
/*	SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);			  */\
/*	uDMAEnable();										  */\
/*	BSP_uDMAControlMemorySet();							  */\
/*	IntEnable(INT_UDMAERR);								  */\
/*	IntPrioritySet(INT_UDMAERR, INTERRUPT_PRIORITY_PHER); */\
	/* DCBUS - Temperature */								\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);				\
	/* Ethernet */	                                        \
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EMAC0);           \
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EPHY0);           \
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);			\
	/* Console */											\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);			\
	/* Motor */												\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);				\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);				\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);			\
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);			\
} while(0)
/**
  * @brief  Macro to deinitialize GPIOs peripheral registers to their default reset values.
  */
#define BSP_GPIODeinit() 									\
do                                    						\
{   /* GPIOs */												\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOA);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOB);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOC);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOD);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOE);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOF);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOG);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOH);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOJ);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOK);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOL);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOM);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPION);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOP);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOQ);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOR);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOS);			\
	SysCtlPeripheralDisable(SYSCTL_PERIPH_GPIOT);			\
} while(0)

/* User Modules includes -------------------------------------------------------------------*/
// User modules use the above declared definitions, so they must be included here
//#include "console.h"			// Console Header file
extern AliveMonitor_TypeDef alive_monitor_1;
extern AliveMonitor_TypeDef alive_monitor_2;
extern AliveMonitor_TypeDef alive_monitor_pn_dcp_cmd;
extern uint8_t def_ipaddr_0;
extern uint8_t def_ipaddr_1;
extern uint8_t def_ipaddr_2;
extern uint8_t def_ipaddr_3;
/* Functions -------------------------------------------------------------------------------*/
void 		BSP_Init(void);
void		BSP_Loop(void);
// uDMA control memory set function
void 		BSP_uDMAControlMemorySet(void);
// Time functions
uint32_t	Time_GetMs(void);
void		Time_Delay(uint32_t nMs);
void		Time_WaitUs(uint32_t nUs);
void		Time_WaitMs(uint32_t nMs);
uint8_t		Time_Blink125ms(void);
uint8_t		Time_Blink250ms(void);
uint8_t		Time_Blink500ms(void);
uint8_t		Time_Blink1s(void);
void 		TickLoop_PeriodicalCall (void (*fPointer)(void), uint16_t period, uint8_t isLastTaskInLoop);
void        TickLoop_PeriodicalCallAtIdle (void (*fPointer)(void), uint16_t period, uint8_t isLastTaskInLoop);
uint8_t 	TickLoop_IsActivated(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_CONFIG_H_ */
