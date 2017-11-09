/**
 *  \file new 1
 *  \brief Brief
 */

/* Includes --------------------------------------------------------------------------------*/
#include "bsp_config.h"		// header file
#include "sys_config.h"
#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS
#include "netconf.h"
#elif WS_FIELDBUS_TYPE == FIELDBUS_TYPE_PNIO || WS_FIELDBUS_TYPE ==FIELDBUS_TYPE_PNIOIO
#include "inc/hw_nvic.h"
#endif

/* Private declarations --------------------------------------------------------------------*/
typedef union
{
	struct
	{						// bit	description
		uint8_t bit00:1;	// 0		  1ms
		uint8_t bit01:1;	// 1		  2ms
		uint8_t bit02:1;	// 2		  4ms
		uint8_t bit03:1;	// 3		  8ms
		uint8_t bit04:1;	// 4		 16ms
		uint8_t bit05:1;	// 5		 32ms
		uint8_t bit06:1;	// 6		 64ms
		uint8_t bit07:1;	// 7		128ms
		uint8_t bit08:1;	// 8		256ms
		uint8_t bit09:1;	// 9		512ms
		uint8_t bit10:1;	// 10		 ~1s
		uint8_t bit11:1;	// 11		 ~2s
		uint32_t rsvd:20;	// 31:12	unused
	} blinkBits;
	uint32_t time;
} TIME_COUNTER_Union;

#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_PNIO || WS_FIELDBUS_TYPE ==FIELDBUS_TYPE_PNIOIO
extern uint32_t g_ui32ClkFreq;
extern volatile uint64_t g_ulTickCounter;
extern uint32_t g_ui32LocalTimer;

#define FAULT_SYSTICK           15                      /**< interrupt number */

#define SYSTICK_HZ              1000    // 1K Hz
#define SYSTICK_MS              (1000 / SYSTICK_HZ)
#endif

uint8_t sysloop_overrun = 0;

AliveMonitor_TypeDef alive_monitor_1;
AliveMonitor_TypeDef alive_monitor_2;
#ifdef ENABLE_PN_DCP_ALIVE_MONITOR
AliveMonitor_TypeDef alive_monitor_pn_dcp_cmd;
#endif /* ENABLE_PN_DCP_ALIVE_MONITOR */

volatile TIME_COUNTER_Union time_counter = {.time = 0};
volatile uint32_t main_app_systick_counter = 0;

/**
 * If a process with high priority is 
 * executed and its period is longer than 
 * 1ms, this flag is set to 1.
 * If so, the process with lower priority 
 * will not be executed in this time slot.
 */
volatile uint8_t time_slot_busy = 0;

static uint8_t tickTaskActivate = 0;
uint16_t overRunFlag = 0;
uint8_t def_ipaddr_0 = 192;
uint8_t def_ipaddr_1 = 168;
uint8_t def_ipaddr_2 = 125;
uint8_t def_ipaddr_3 = 67;
// The control table used by the uDMA controller. This table must be aligned to a 1024 byte boundary
#if defined(ewarm)
#pragma data_alignment=1024
uint8_t pui8ControlTable[1024];
#elif defined(ccs)
#pragma DATA_ALIGN(pui8ControlTable, 1024)
uint8_t pui8ControlTable[1024];
#else
uint8_t pui8ControlTable[1024] __attribute__ ((aligned(1024)));
#endif

void SysTick_IntHandler();
extern EthernetLoop_EIP_Process(void);

/* System related functions ----------------------------------------------------------------*/
/** 
  * @brief	Initialize the system.
  * @param	none.
  * @retval	none.
  */
void BSP_Init()
{
    // Enable lazy stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    FPULazyStackingEnable();
    FPUEnable();
    // Make sure the main oscillator is enabled because this is required by
    // the PHY.  The system must have a 25MHz crystal attached to the OSC
    // pins. The SYSCTL_MOSC_HIGHFREQ parameter is used when the crystal
    // frequency is 10MHz or higher.
    SysCtlMOSCConfigSet(SYSCTL_MOSC_HIGHFREQ);
    // Run from the PLL at 120 MHz.
    SystemCoreClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480), 120000000);
#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_PNIO || WS_FIELDBUS_TYPE ==FIELDBUS_TYPE_PNIOIO
    // ** g_ui32ClkFreq is used by pnio.
    g_ui32ClkFreq = SystemCoreClock;
#endif
    // SysTick initialization - 1ms
    SysTickPeriodSet(SystemCoreClock/1000);
    //SysTickIntEnable();
    SysTickIntRegister(SysTick_IntHandler);
    SysTickEnable();
    IntPrioritySet(FAULT_SYSTICK, INTERRUPT_PRIORITY_SYS);
    IntMasterEnable();

	// De-initializes the GPIOs peripheral registers to their default reset values.
    BSP_GPIODeinit();
	// Enables the clock of the used peripherals
	BSP_ClockEnable();

	// Initialize User Modules
	//Console_Init();
	//if (EEPROMInit()!=EEPROM_INIT_OK) Console_Error("EEPROM initialization failed");
	while(EEPROMInit()!=EEPROM_INIT_OK);

	// Welcome String
	//Console_PrintColor(CONSOLE_COLOR_GREY, "Welcome to %s: %s, Version %d.%d.%d", APP_NAME, APP_NAME_EXTENDED, APP_VERSION_MAJ, APP_VERSION_MID, APP_VERSION_MIN);
}
/**
  * @brief	Loop the bsp modules.
  * @param	none.
  * @retval	none.
  */
void BSP_Loop()
{
	// Loop console data
	//Console_Loop();
}

/* DMA functions ---------------------------------------------------------------------------*/
/**
  * @brief	Set the preallocated memory for DMA usage.
  * @param	none.
  * @retval	none.
  */
void BSP_uDMAControlMemorySet()
{
	uDMAControlBaseSet(pui8ControlTable);
}
/**
  * @brief	DMA error interrupt handler.
  * @param	none.
  * @retval	none.
  */
void DMAError_IntHandler()
{
    // Print uDMA error bit
    //Console_Error("DMA Error: %d", uDMAErrorStatusGet());
}

/* Time functions --------------------------------------------------------------------------*/
/** 
  * @brief	check whether the object to monitor is inactive for certain time.
  * @param	am, the alivemonitor to process
  * @retval	none.
  */
void alive_monitor_process(AliveMonitor_TypeDef* am)
{
	// only set alive after awaken.
	// only set inalive after reaching limit.
	// by default it's inactive.
	if (am->IsAwaken == 1) {
        am->TrigCount = 0;
        am->AliveFlag = 1;
        am->IsAwaken = 0;
    }
    else {
        if (am->TrigCount >= am->TrigThreshold) {
            am->AliveFlag = 0;
        }
        else {
        	am->TrigCount++;
        }
    }
}

/**
  * @brief	SysTick interrupt handler: increment the time counter.
  * @param	none.
  * @retval	none.
  */
void SysTick_IntHandler()
{
	//if (SysTickIntFlagGet() != 0) {
		time_counter.time++;
	//};

	// Detect the overrun task.
	if (tickTaskActivate == 1) overRunFlag++;

	if (main_app_systick_counter >= 999) main_app_systick_counter = 0;
	else {
		main_app_systick_counter++;
//		if (main_app_systick_counter % 10 == 0) EthernetLoop_EIP_Process();
	}

	tickTaskActivate = 1;

	alive_monitor_process(&alive_monitor_1);
	alive_monitor_process(&alive_monitor_2);
#ifdef ENABLE_PN_DCP_ALIVE_MONITOR
	alive_monitor_process(&alive_monitor_pn_dcp_cmd);
#endif /* ENABLE_PN_DCP_ALIVE_MONITOR */

#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS
    // Call lwip timer handler.
	lwip_timer();
#elif WS_FIELDBUS_TYPE == FIELDBUS_TYPE_PNIO || WS_FIELDBUS_TYPE ==FIELDBUS_TYPE_PNIOIO
	// **
    /* increment global tick counter */
    g_ulTickCounter++;

    g_ui32LocalTimer += SYSTICK_MS;

    // Generate an Ethernet interrupt.
    HWREG(NVIC_SW_TRIG) |= INT_EMAC0 - 16;
#endif

    //clear the time slot busy flag, added by Tms
    time_slot_busy = 0;
}
/** 
  * @brief	Get the system time in ms.
  * @param	none.
  * @retval	The system time in ms.
  */
uint32_t Time_GetMs()
{
	return time_counter.time;

}

// judge whether to start the loop task.
uint8_t TickLoop_IsActivated ()
{
	if (tickTaskActivate == 0) return 0;
	else return 1;
}

//since we dont have any task register mechanism, so we judge the task loop via isLastTaskInLoop tag.
void TickLoop_PeriodicalCall (void (*fPointer)(void), uint16_t period, uint8_t isLastTaskInLoop)
{
    sysloop_overrun = 1;
    if (main_app_systick_counter % period == 0 && tickTaskActivate == 1) {
        fPointer();

        //mark the time_slot_busy
        if(!time_slot_busy && period > 1) time_slot_busy = 1;
    }

    if (isLastTaskInLoop == 1) tickTaskActivate = 0;
    sysloop_overrun = 0;
}

/**
 * TickLoop_PeriodicalCallAtIdle implements the function of fPointer when no other task has been
 * implemented by TickLoop_PeriodicalCall. (The ones with period of 1ms does not count here!)
 *
 * It utilize the idle time slot of the processor for implementing the task which should not
 * interference the timing of the other process.
 *
 * This function should be called after all the TickLoop_PeriodicalCall's.
 *
 * The ones with period of 1ms should be added later than the ones with longer period.
 */
void TickLoop_PeriodicalCallAtIdle (void (*fPointer)(void), uint16_t period, uint8_t isLastTaskInLoop)
{
    sysloop_overrun = 1;
    if (main_app_systick_counter % period == 0 && tickTaskActivate == 1 && !time_slot_busy) {
        fPointer();

        if(period > 1) time_slot_busy = 1;
    }

    if (isLastTaskInLoop == 1) tickTaskActivate = 0;
    sysloop_overrun = 0;
}

/**
  * @brief	Halt the thread via software methods.
  * 		Even if interrupted will still count properly.
  * @param	nMs*1ms, halt period in uint of ms.
  * @retval none.
  */
void Time_Delay(uint32_t nMs)
{
  // Capture the current local time
  uint32_t endTime = time_counter.time + nMs;
  // Wait until the desired delay finish
  while(endTime > time_counter.time);
}
/**
  * @brief	Halt the thread via software methods.
  * 		If interrupted will count a longer time.
  * @param	halt period in uint of us.
  * @retval	none.
  */
void Time_WaitUs(uint32_t nUs)
{
    SysCtlDelay((SystemCoreClock/3000000)*nUs);
}
/**
  * @brief	Halt the thread via software methods.
  * 		If interrupted will count a longer time.
  * @param	halt period in uint of ms.
  * @retval	none.
  */
void Time_WaitMs(uint32_t nMs)
{
    SysCtlDelay((SystemCoreClock/3000)*nMs);
}

/**
  * @brief  Get a 128ms blinking bit.
  *     @note It uses the 7th bit of the time variable
  * @param  none.
  * @retval 1 for 128ms, 0 for 128ms...
  */
uint8_t Time_Blink125ms()
{
  return time_counter.blinkBits.bit07;
}
/**
  * @brief	Get a 256ms blinking bit.
  * 		@note	It uses the 8th bit of the time variable
  * @param	none.
  * @retval 1 for 256ms, 0 for 256ms...
  */
uint8_t Time_Blink250ms()
{
	return time_counter.blinkBits.bit08;
}
/**
  * @brief	Get a 512ms blinking bit.
  * 		@note	It uses the 9th bit of the time variable
  * @param	none.
  * @retval 1 for 512ms, 0 for 512ms...
  */
uint8_t Time_Blink500ms()
{
	return time_counter.blinkBits.bit09;
}
/**
  * @brief	Get a 1024ms blinking bit.
  * 		@note	It uses the 10th bit of the time variable
  * @param	none.
  * @retval 1 for 1024ms, 0 for 1024ms...
  */
uint8_t Time_Blink1s()
{
	return time_counter.blinkBits.bit10;
}


