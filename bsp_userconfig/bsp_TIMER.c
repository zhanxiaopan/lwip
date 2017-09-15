/**
 *  \file new 1
 *  \brief Brief
 */

/* Includes --------------------------------------------------------------------------------*/
#include "bsp_TIMER.h"	// header file

/* Private declarations --------------------------------------------------------------------*/
// Counting Up or Down is irrelevant for most of the applications, but a count Up mode
// could give benefit to applications such as the Hall Speed calculation, enabling the
// understanding of how much time it passed from previous HALL event
// Enable this define if you want to count UP, otherwise it will count down
#define TIMER_COUNT_UP
// Following variables are used for the calculation of time in common units
uint32_t TIMER_divide_for_ms;
uint32_t TIMER_divide_for_us;
uint32_t TIMER_divide_for_us_10;

/* Configurations Functions ----------------------------------------------------------------*/
/**
  * @brief	Configure the timer and the timer-update interrupt.
  * @param  TIMx: Timer directives. It should be a value composed of
  * 		TIMERx_BASE | BSP_TIMER_y where x is 1 to 7 and y is 16A, 16B or 32.
  * @param	TIMFrequency: Specify the timer frequency in Hz.
  * @param	TIMER_IntChannel: Specifies the IRQ channel to be enabled.
  *			This parameter should be INT_TIMERxy where x is 1 to 7 and y is A or B.
  * @param	priority: Specifies the priority of the timed interrupt.
  *			This parameter can be a value of @ref INTERRUPT_PRIORITY_TypeDef.
  * @retval	None.
 */
void TIMER_ConfigTimedInterrupt(uint32_t TIMx, uint32_t TIMFrequency, uint8_t TIMER_IntChannel, INTERRUPT_PRIORITY_TypeDef priority)
{
	// Get Timer directive
	uint32_t TIMERx_BASE = TIMx & ~0xF;
#ifdef TIMER_COUNT_UP
	uint32_t TIMER_TIMx_EVENT = (((TIMx & BSP_TIMER_32)||(TIMx & BSP_TIMER_16A))?TIMER_TIMA_MATCH:TIMER_TIMB_MATCH);
#else
	uint32_t TIMER_TIMx_EVENT = (((TIMx & BSP_TIMER_32)||(TIMx & BSP_TIMER_16A))?TIMER_TIMA_TIMEOUT:TIMER_TIMB_TIMEOUT);
#endif
    // Disable processor interrupts.
    IntMasterDisable();
	// Configure Timer
	TIMER_ConfigTimer(TIMx, TIMFrequency);
    // Setup the interrupts for the timeout.
    IntEnable(TIMER_IntChannel);
    TimerIntEnable(TIMERx_BASE, TIMER_TIMx_EVENT);
    // Clear the timer interrupt for robustness
    TimerIntClear(TIMERx_BASE, TIMER_TIMx_EVENT);
    // Set interrupt priority
    IntPrioritySet(TIMER_IntChannel, priority);
    // Enable processor interrupts.
    IntMasterEnable();
    // Calculate time dividers
    TIMER_divide_for_ms = SystemCoreClock/1000;
    TIMER_divide_for_us = SystemCoreClock/1000000;
    TIMER_divide_for_us_10 = SystemCoreClock/10000000;
}
/**
  * @brief	Configure the timer and the timer-update interrupt.
  * @param  TIMx: Timer directives. It should be a value composed of
  * 		TIMERx_BASE | BSP_TIMER_y where x is 1 to 7 and y is 16A, 16B or 32.
  * @param	TIMFrequency: Specify the timer frequency in Hz.
  * @retval	None.
 */
void TIMER_ConfigTimedADCTrigger(uint32_t TIMx, uint32_t TIMFrequency)
{
    // Get the timer parameters
	uint32_t TIMERx_BASE = TIMx & ~0xF;
	uint32_t TIMER_AB = (((TIMx & BSP_TIMER_32)||(TIMx & BSP_TIMER_16A))?TIMER_A:TIMER_B);
	// Configure Timer
	TIMER_ConfigTimer(TIMx, TIMFrequency);
	// Configure and enable the ADC trigger
#ifdef TIMER_COUNT_UP
	uint32_t TIMER_TIMx_EVENT = (((TIMx & BSP_TIMER_32)||(TIMx & BSP_TIMER_16A))?TIMER_TIMA_MATCH:TIMER_TIMB_MATCH);
#else
	uint32_t TIMER_TIMx_EVENT = (((TIMx & BSP_TIMER_32)||(TIMx & BSP_TIMER_16A))?TIMER_TIMA_TIMEOUT:TIMER_TIMB_TIMEOUT);
#endif
	TimerADCEventSet(TIMERx_BASE, TIMER_TIMx_EVENT);
	TimerControlTrigger(TIMERx_BASE, TIMER_AB, 1);
}
/**
  * @brief	Configure the timer.
  * @param  TIMx: Timer directives. It should be a value composed of
  * 		TIMERx_BASE | BSP_TIMER_y where x is 1 to 7 and y is 16A, 16B or 32.
  * @param	TIMFrequency: Specify the timer frequency in Hz.
  * @retval	The period of the timer.
 */
uint32_t TIMER_ConfigTimer(uint32_t TIMx, uint32_t TIMFrequency)
{
	// Get Timer directive
	uint32_t TIMERx_BASE = TIMx & ~0xF;
	uint32_t TIMER_AB = 0;
    // Configure the 32-bit periodic timer
	if (TIMx & BSP_TIMER_32)
	{
#ifdef TIMER_COUNT_UP
	TimerConfigure(TIMERx_BASE, TIMER_CFG_PERIODIC_UP);
#else
	TimerConfigure(TIMERx_BASE, TIMER_CFG_PERIODIC);
#endif
		TIMER_AB |= TIMER_A;
	}
    // Configure the 16-bit periodic timer/s
	else
	{
		if (TIMx & BSP_TIMER_16A)
		{
			TimerConfigure(TIMERx_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC);
			TIMER_AB |= TIMER_A;
		}
		if (TIMx & BSP_TIMER_16B)
		{
			TimerConfigure(TIMERx_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC);
			TIMER_AB |= TIMER_B;
		}
	}
    TimerLoadSet(TIMERx_BASE, TIMER_AB, (SystemCoreClock/TIMFrequency));
#ifdef TIMER_COUNT_UP
    TimerMatchSet(TIMERx_BASE, TIMER_AB, (SystemCoreClock/TIMFrequency));
#endif
    // Enable the timer/s
    TimerEnable(TIMERx_BASE, TIMER_AB);
    // Return the period of the timer
	return (SystemCoreClock/TIMFrequency);
}

/* Interrupt Functions ---------------------------------------------------------------------*/
/**
  * @brief  Verify that the interrupt was the expected one,
  * 		then clear the interrupt state.
  * @param  TIMx: Timer directives. It should be a value composed of
  * 		TIMERx_BASE | BSP_TIMER_y where x is 1 to 7 and y is 16A, 16B or 32.
  * @retval 1 if interrupt was the one expected, 0 else.
 */
uint8_t TIMER_GetAndClearInt(uint32_t TIMx)
{
	uint32_t TIMERx_BASE = TIMx & ~0xF;
#ifdef TIMER_COUNT_UP
	uint32_t TIMER_TIMx_EVENT = (((TIMx & BSP_TIMER_32)||(TIMx & BSP_TIMER_16A))?TIMER_TIMA_MATCH:TIMER_TIMB_MATCH);
#else
	uint32_t TIMER_TIMx_EVENT = (((TIMx & BSP_TIMER_32)||(TIMx & BSP_TIMER_16A))?TIMER_TIMA_TIMEOUT:TIMER_TIMB_TIMEOUT);
#endif
    TimerIntClear(TIMERx_BASE, TIMER_TIMx_EVENT);
	// At the moment just return true because there is just the TIMEOUT interrupt enabled
	// If in future there will be more interrupts enabled, prepare the GET
	return 1;
}


/* Timer Value Functions -------------------------------------------------------------------*/
// Just in case we are in TIMER_COUNT_UP mode
#ifdef TIMER_COUNT_UP
/**
  * @brief  Get actual value of timer, then reset value.
  * @param  TIMx: Timer directives. It should be a value composed of
  * 		TIMERx_BASE | BSP_TIMER_y where x is 1 to 7 and y is 16A, 16B or 32.
  * @retval actual value of timer.
 */
uint32_t TIMER_GetAndClearValue(uint32_t TIMx)
{
	uint32_t TIMERx_BASE = TIMx & ~0xF;
	uint32_t value;

	if (TIMx & BSP_TIMER_32)
	{
		value = ((HWREG(TIMERx_BASE + TIMER_O_TBV)& 0xFFFF)<<16) | (HWREG(TIMERx_BASE + TIMER_O_TAV)& 0xFFFF);
		HWREG(TIMERx_BASE + TIMER_O_TAV) = 0;
		HWREG(TIMERx_BASE + TIMER_O_TBV) = 0;
	}
	else if (TIMx & BSP_TIMER_16A)
	{
		value = HWREG(TIMERx_BASE + TIMER_O_TAV);
		HWREG(TIMERx_BASE + TIMER_O_TAV) = 0;
	}
	else
	{
		value = HWREG(TIMERx_BASE + TIMER_O_TBV);
		HWREG(TIMERx_BASE + TIMER_O_TBV) = 0;
	}

	return value;
}
/**
  * @brief  Get actual value of timer in ms, then reset value.
  * @param  TIMx: Timer directives. It should be a value composed of
  * 		TIMERx_BASE | BSP_TIMER_y where x is 1 to 7 and y is 16A, 16B or 32.
  * @retval actual value of timer.
 */
uint32_t TIMER_GetAndClearValueMs(uint32_t TIMx)
{
	return TIMER_GetAndClearValue(TIMx)/TIMER_divide_for_ms;
}
/**
  * @brief  Get actual value of timer in us, then reset value.
  * @param  TIMx: Timer directives. It should be a value composed of
  * 		TIMERx_BASE | BSP_TIMER_y where x is 1 to 7 and y is 16A, 16B or 32.
  * @retval actual value of timer.
 */
uint32_t TIMER_GetAndClearValueUs(uint32_t TIMx)
{
	return TIMER_GetAndClearValue(TIMx)/TIMER_divide_for_us;
}
/**
  * @brief  Get actual value of timer in us/10, then reset value.
  * @param  TIMx: Timer directives. It should be a value composed of
  * 		TIMERx_BASE | BSP_TIMER_y where x is 1 to 7 and y is 16A, 16B or 32.
  * @retval actual value of timer.
 */
uint32_t TIMER_GetAndClearValueUs_10(uint32_t TIMx)
{
	return TIMER_GetAndClearValue(TIMx)/TIMER_divide_for_us_10;
}
#endif
