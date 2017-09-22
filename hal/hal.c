/*
 * hal.c
 *
 * This is the Hardware Abstraction Layer of the TM4C129 application
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_emac.h"
#include "inc/hw_nvic.h"

#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/emac.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/systick.h"

#include "utils/uartstdio.h"

#include "hal.h"


#define SYSTICK_INT_PRIORITY    0x80
#define ETHERNET_INT_PRIORITY   0xC0
#define FAULT_SYSTICK           15                      /**< interrupt number */

#define SYSTICK_HZ              1000    // 1K Hz
#define SYSTICK_MS              (1000 / SYSTICK_HZ)

// board specific IOs
const HAL_GPIO_t HAL_GPIO_PE0 = {GPIO_PORTE_BASE, GPIO_PIN_0};
const HAL_GPIO_t HAL_GPIO_PE1 = {GPIO_PORTE_BASE, GPIO_PIN_1};
const HAL_GPIO_t HAL_GPIO_PE2 = {GPIO_PORTE_BASE, GPIO_PIN_2};

const HAL_GPIO_t HAL_GPIO_PB0 = {GPIO_PORTB_BASE, GPIO_PIN_0};
const HAL_GPIO_t HAL_GPIO_PB1 = {GPIO_PORTB_BASE, GPIO_PIN_1};

const HAL_GPIO_t HAL_GPIO_PB2 = {GPIO_PORTB_BASE, GPIO_PIN_2};
const HAL_GPIO_t HAL_GPIO_PB3 = {GPIO_PORTB_BASE, GPIO_PIN_3};

const HAL_GPIO_t HAL_GPIO_PC6 = {GPIO_PORTC_BASE, GPIO_PIN_6};
const HAL_GPIO_t HAL_GPIO_PC7 = {GPIO_PORTC_BASE, GPIO_PIN_7};

// board specific LEDs
const HAL_GPIO_t HAL_GPIO_LED5 = {GPIO_PORTK_BASE, GPIO_PIN_4}; //
const HAL_GPIO_t HAL_GPIO_LED6 = {GPIO_PORTK_BASE, GPIO_PIN_6};
const HAL_GPIO_t HAL_GPIO_LED7 = {GPIO_PORTK_BASE, GPIO_PIN_5};
const HAL_GPIO_t HAL_GPIO_LED8 = {GPIO_PORTK_BASE, GPIO_PIN_7};

// tick counter used as timer base
volatile uint64_t g_ulTickCounter = 0;

uint32_t g_ui32LocalTimer = 0;

uint32_t g_ui32ClkFreq;

static uint32_t g_led_tick = 0;


uint32_t HAL_setupClockFreq();
void HAL_setupGpio();
//void HAL_setupEthernet();
void HAL_setupUart();
void HAL_setupSpi(uint32_t sys_clock);
void HAL_emptySpiFifo(uint32_t spi_base);
void HAL_setupSysTick(uint32_t sys_clk_hz);
void HAL_setupTimer0(uint32_t sys_clock);


void HAL_setup()
{
    //char mac_addr[] = DEVICE_MAC_ADDR; //here we use MAC_ADDR instead
    // initialize the clock frequency
    g_ui32ClkFreq = HAL_setupClockFreq();

    // initialize the gpio pins and the alternate functions
    HAL_setupGpio();

    // initialize the UART
    HAL_setupUart();

    // initialize the SPI communication
//    HAL_setupSpi(g_ui32ClkFreq);

    // initialize SysTick for a periodic interrupt.
    HAL_setupSysTick(g_ui32ClkFreq);
    uint8_t mac_addr[6] = {1,2,3,4,5,6};
//    gen_mac_addr();
//    mac_addr[0] =  get_mac_addr(0);//MAC_ADDR0;
//    mac_addr[1] =  get_mac_addr(1);//MAC_ADDR1;
//    mac_addr[2] =  get_mac_addr(2);//MAC_ADDR2;
//    mac_addr[3] =  get_mac_addr(3);//MAC_ADDR3;
//    mac_addr[4] =  get_mac_addr(4);//MAC_ADDR4;
//    mac_addr[5] =  get_mac_addr(5);//MAC_ADDR5;
    // Initialize the Ethernet Controller
    HAL_setupEthernet(mac_addr);

    // interrupt
    /* set interrupt priorities */
    //printf("hal.c::HAL_setup()::IntPrioritySet\n");
    IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);
    IntPrioritySet(FAULT_SYSTICK, SYSTICK_INT_PRIORITY);

    /* Disable SysTick */
    // it will be enabled in the timer init
//    SysTickDisable();
//    SysTickIntDisable();


    /* disable Ethernet interrupt */
    // it will be enabled in the TARGET_Ether_Open
    //printf("hal.c::HAL_setup()::IntDisable\n");
    IntDisable(INT_EMAC0);

    // disable SPI interrupt
    IntDisable(INT_SSI0);

    /* Enable all processor interrupts. */
    IntMasterEnable();

    return;
}


void HAL_setupIntPriority()
{
    /* set interrupt priorities */
    IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);
    IntPrioritySet(FAULT_SYSTICK, SYSTICK_INT_PRIORITY);
    return;
}

uint32_t HAL_setupClockFreq()
{
    uint32_t sys_clock;
    // Make sure the main oscillator is enabled because this is required by
    // the PHY.  The system must have a 25MHz crystal attached to the OSC
    // pins. The SYSCTL_MOSC_HIGHFREQ parameter is used when the crystal
    // frequency is 10MHz or higher.
    SysCtlMOSCConfigSet(SYSCTL_MOSC_HIGHFREQ);
    // set the clocking to run from the PLL at 120MHz
    sys_clock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                    SYSCTL_OSC_MAIN |
                                    SYSCTL_USE_PLL |
                                    SYSCTL_CFG_VCO_480), 120000000);

    return sys_clock;
}


// this function will setup the GPIO pins and its alternative function
void HAL_setupGpio()
{
	//////////////////////////////////////////
	// enable GPIO PA
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	// GPIO PA0, U0RX
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_0);
	GPIOPinConfigure(GPIO_PA0_U0RX);

	// GPIO PA1, U0TX
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_1);
	GPIOPinConfigure(GPIO_PA1_U0TX);

	// UART
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// GPIO PA2, SSI0CLK
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_2);
	GPIOPinConfigure(GPIO_PA2_SSI0CLK);

	// GPIO PA3, SSI0FSS
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_3);
	GPIOPinConfigure(GPIO_PA3_SSI0FSS);

	// GPIO PA4, SSI0DAT0, SOMI
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_4);
	GPIOPinConfigure(GPIO_PA4_SSI0XDAT0);

	// GPIO PA5, SSI0DAT1, SIMO
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_5);
	GPIOPinConfigure(GPIO_PA5_SSI0XDAT1);

	// SSI0
	GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_5 | GPIO_PIN_4 | GPIO_PIN_3 | GPIO_PIN_2);

	// GPIO PA6
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_6);

	// GPIO PA7
	GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_7);


	//////////////////////////////////////////
	// enable GPIO PB
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

	// GPIO PB0 ~ PB5
	// PB0 --> output, Flag_EnableSystem
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_0, 0);
	// PB1 --> output, enable spi communication
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_1);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_1, 0);

	// PB2 --> output, speed selection 0
	//GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, 0);

	// PB3 --> output, speed selection 1
	//GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_3, 0);

	GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_5);


	//////////////////////////////////////////
	// enable GPIO PC
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);

	// GPIO PC0 ~ PC3 JTAG

	// GPIO PC4 ~ PC7
	GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_5);

	// PC6 --> output, DIR CCW
	//GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_6);
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_6);
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_6, 0);
	// PC7 --> output, DIR CW
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_7);
	GPIOPinWrite(GPIO_PORTC_BASE, GPIO_PIN_7, 0);


	//////////////////////////////////////////
	// enable GPIO PD
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

	// GPIO PD0 ~ PD7 (not used)
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_0);
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_5);
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_6);
	GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_7);


	//////////////////////////////////////////
	// enable GPIO PE
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

	// GPIO PE0 ~ PE3, DI1 ~ DI4
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_0);
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_3);

	// GPIO PE4, PE5 (not used)
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOInput(GPIO_PORTE_BASE, GPIO_PIN_5);


	//////////////////////////////////////////
	// enable GPIO PF
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

	// GPIO PF0, EN0LED0
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0);
	GPIOPinConfigure(GPIO_PF0_EN0LED0);

	// GPIO PF1, EN0LED2 --> EN0LED1
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinConfigure(GPIO_PF1_EN0LED2);

	// Ethernet LED
	GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	// GPIO PF2~4
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);


	//////////////////////////////////////////
	// enable GPIO PG
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);

	// GPIO PG0, PG1 (not used)
	GPIOPinTypeGPIOInput(GPIO_PORTG_BASE, GPIO_PIN_0);
	GPIOPinTypeGPIOInput(GPIO_PORTG_BASE, GPIO_PIN_1);


	//////////////////////////////////////////
	// enable GPIO PG
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);

	// GPIO PH0~PH3 (not used)
	GPIOPinTypeGPIOInput(GPIO_PORTH_BASE, GPIO_PIN_0);
	GPIOPinTypeGPIOInput(GPIO_PORTH_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOInput(GPIO_PORTH_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOInput(GPIO_PORTH_BASE, GPIO_PIN_3);


	//////////////////////////////////////////
	// enable GPIO PJ
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ);

	// GPIO PJ0, PJ1 (not used)
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0);
	GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_1);


	//////////////////////////////////////////
	// enable GPIO PK
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);

	// GPIO PK0~PK3 (not used)
	GPIOPinTypeGPIOInput(GPIO_PORTK_BASE, GPIO_PIN_0);
	GPIOPinTypeGPIOInput(GPIO_PORTK_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOInput(GPIO_PORTK_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOInput(GPIO_PORTK_BASE, GPIO_PIN_3);

	// GPIO PK4~PK7, LED
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_5);
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_6);
	GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_PIN_7);

	GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_4, 0);
	GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_5, 0);
	GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_6, GPIO_PIN_6);
	GPIOPinWrite(GPIO_PORTK_BASE, GPIO_PIN_7, GPIO_PIN_7);


	//////////////////////////////////////////
	// enable GPIO PL
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOL);

	// GPIO PL0~PL7, GPIO Input
	GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_0);
	GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_5);
	GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_6);
	GPIOPinTypeGPIOInput(GPIO_PORTL_BASE, GPIO_PIN_7);


	//////////////////////////////////////////
	// enable GPIO PM
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOM);
	// GPIO PM0~PM7 (not used)
	GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_0);
	GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_5);
	GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_6);
	GPIOPinTypeGPIOInput(GPIO_PORTM_BASE, GPIO_PIN_7);


	//////////////////////////////////////////
	// enable GPIO PN
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPION);

	// GPIO PN0, DO1, conflict with DSP,
	GPIOPinTypeGPIOInput(GPIO_PORTN_BASE, GPIO_PIN_0);

	// GPIO PN1, DO2, conflict with DSP,
	GPIOPinTypeGPIOInput(GPIO_PORTN_BASE, GPIO_PIN_1);

	// GPIO PN2, DOUT3,
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_2);
	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_2, 0);

	// GPIO PN3, DOUT4,
	GPIOPinTypeGPIOOutput(GPIO_PORTN_BASE, GPIO_PIN_3);
	GPIOPinWrite(GPIO_PORTN_BASE, GPIO_PIN_3, 0);

	// GPIO PN4, PN5, (not used)
	GPIOPinTypeGPIOInput(GPIO_PORTN_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOInput(GPIO_PORTN_BASE, GPIO_PIN_5);


	//////////////////////////////////////////
	// enable GPIO PP
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP);

	// GPIO PP0 ~ PP5, (not used)
	GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_0);
	GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_3);
	GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_4);
	GPIOPinTypeGPIOInput(GPIO_PORTP_BASE, GPIO_PIN_5);


	//////////////////////////////////////////
	// enable GPIO PQ
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOQ);

	// GPIO PQ0 ~ PQ3, (not used)
	GPIOPinTypeGPIOInput(GPIO_PORTQ_BASE, GPIO_PIN_0);
	GPIOPinTypeGPIOInput(GPIO_PORTQ_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOInput(GPIO_PORTQ_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOInput(GPIO_PORTQ_BASE, GPIO_PIN_3);

	// GPIO PQ4, relay on
	GPIOPinTypeGPIOOutput(GPIO_PORTQ_BASE, GPIO_PIN_4);
	GPIOPinWrite(GPIO_PORTQ_BASE, GPIO_PIN_4, 0);


	return;
}


void HAL_GPIOPinWrite(const HAL_GPIO_t *p_gpio, bool value)
{
	if (value == true)
	{
		GPIOPinWrite(p_gpio->base, p_gpio->pin, p_gpio->pin);
	}
	else
	{
		GPIOPinWrite(p_gpio->base, p_gpio->pin, 0);
	}

	return;
}


bool HAL_GPIOPinRead(const HAL_GPIO_t *p_gpio)
{
	uint32_t data;
	data =  GPIOPinRead(p_gpio->base, p_gpio->pin);
	return (bool)data;
}


void HAL_GPIOPinToggle(const HAL_GPIO_t *p_gpio)
{
	uint32_t data;
	data =  GPIOPinRead(p_gpio->base, p_gpio->pin);
	GPIOPinWrite(p_gpio->base, p_gpio->pin, data^(p_gpio->pin));
	return;
}


void HAL_setupUart()
{
	// Enable UART0 so that we can configure the clock.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

	// Use the internal 16MHz oscillator as the UART clock source.
	UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

	// Initialize the UART for console I/O.
	UARTStdioConfig(0, 115200, 16000000);

	return;
}


void HAL_emptySpiFifo(uint32_t spi_base)
{
	uint32_t temp;
    // Read any residual data from the SSI port.  This makes sure the receive
    // FIFOs are empty, so we don't read any unwanted junk.  This is done here
    // because the SPI SSI mode is full-duplex, which allows you to send and
    // receive at the same time.  The SSIDataGetNonBlocking function returns
    // "true" when data was returned, and "false" when no data was returned.
    // The "non-blocking" function checks if there is any data in the receive
    // FIFO and does not "hang" if there isn't.
    //
    while(SSIDataGetNonBlocking(spi_base, &temp))
    {
    }

    return;
}


void HAL_enableSPICommunication()
{
	// enable spi communication with DSP
    HAL_GPIOPinWrite(&HAL_GPIO_PB1, 1);

    SSIIntEnable(SSI0_BASE, SSI_RXFF);

    IntEnable(INT_SSI0);
}


void HAL_setupSysTick(uint32_t sys_clk_hz)
{
    // Configure SysTick for a periodic interrupt.
    SysTickPeriodSet(sys_clk_hz / SYSTICK_HZ);
    SysTickEnable();
    SysTickIntEnable();
}


void HAL_setupEthernet(uint8_t mac_addr[])
{
    printf("hal.c::HAL_setupEthernet\n");
    /* Enable and reset the Ethernet modules */
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EMAC0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EPHY0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_EMAC0);
    SysCtlPeripheralReset(SYSCTL_PERIPH_EPHY0);
    /* wait some cycles until the modules are fully activated */
    SysCtlDelay(1000);

    /* Wait for the MAC to be ready.*/
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EMAC0));

    HWREG(EMAC0_BASE + EMAC_O_CC) |= EMAC_CC_POL;
    /* config PHY: Autonegotiation on, 100MB Full Duplex prefered */
    EMACPHYConfigSet(EMAC0_BASE,(EMAC_PHY_TYPE_INTERNAL | EMAC_PHY_INT_MDIX_EN |
                EMAC_PHY_AN_100B_T_FULL_DUPLEX));

    /* reset MAC */
    EMACReset(EMAC0_BASE);

    EMACInit(EMAC0_BASE, g_ui32ClkFreq,(EMAC_BCONFIG_MIXED_BURST |
                EMAC_BCONFIG_PRIORITY_FIXED |EMAC_BCONFIG_TX_PRIORITY), 4, 4, 0);

//    EMACPHYWrite(EMAC0_BASE, 0, EPHY_LEDCR, EPHY_LEDCR_BLINKRATE_10HZ);
//
//    EMACPHYExtendedWrite(EMAC0_BASE,
//             0,
//             EPHY_LEDCFG,
//             EPHY_LEDCFG_LED0_LINKTXRX | EPHY_LEDCFG_LED2_LINK);

    EMACConfigSet(EMAC0_BASE,
            (EMAC_CONFIG_FULL_DUPLEX |
             EMAC_CONFIG_CHECKSUM_OFFLOAD |
             EMAC_CONFIG_7BYTE_PREAMBLE |
             EMAC_CONFIG_IF_GAP_96BITS |
             EMAC_CONFIG_USE_MACADDR0 |
             EMAC_CONFIG_SA_FROM_DESCRIPTOR |
             EMAC_CONFIG_BO_LIMIT_1024 |
             EMAC_CONFIG_STRIP_CRC),
            (EMAC_MODE_RX_STORE_FORWARD |
             EMAC_MODE_TX_STORE_FORWARD |
             EMAC_MODE_TX_THRESHOLD_64_BYTES |
             EMAC_MODE_RX_THRESHOLD_64_BYTES),
             0);

    EMACAddrSet(EMAC0_BASE, 0, mac_addr);

    return;
}

void HAL_setupTimer0(uint32_t sys_clock)
{
    // the Timer0 peripheral must be enabled for use
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // configure Timer0B as a 16 bit periodic timer
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC);

    // set the Timer0B load value to 1kHz
    TimerLoadSet(TIMER0_BASE, TIMER_B, sys_clock/15000);
}


/***************************************************************************/
/** SysTick Interrupt Handler
 *
 * This function calls the PROFINET timer handler function periodically every
 * millisecond.
 */
void SysTickIntHandler(void)
{
    g_led_tick++;
    if (g_led_tick >= 1000/2)
    {
        // toggle on-board led
        HAL_GPIOPinToggle(&HAL_GPIO_LED7);
        g_led_tick = 0;
    }

    /* increment global tick counter */
    g_ulTickCounter++;

    g_ui32LocalTimer += SYSTICK_MS;

    // Generate an Ethernet interrupt.
    HWREG(NVIC_SW_TRIG) |= INT_EMAC0 - 16;
}



