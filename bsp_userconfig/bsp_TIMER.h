/**
 *  \file new 1
 *  \brief Brief
 */

#ifndef __BSP_TIMER_H_
#define __BSP_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes --------------------------------------------------------------------------------*/
#include "bsp_config.h"				// bsp_CONFIG Header file
#include "driverlib/timer.h"		// TIMER Header file
#include "inc/hw_timer.h"			// Hardware TIMER Header file
#include "driverlib/interrupt.h"	// INTERRUPT Header file

/* Configurations --------------------------------------------------------------------------*/

/* Definitions -----------------------------------------------------------------------------*/

/* Functions -------------------------------------------------------------------------------*/
void		TIMER_ConfigTimedInterrupt(uint32_t TIMx, uint32_t TIMFrequency, uint8_t TIMER_IntChannel, INTERRUPT_PRIORITY_TypeDef priority);
void		TIMER_ConfigTimedADCTrigger(uint32_t TIMx, uint32_t TIMFrequency);
uint32_t	TIMER_ConfigTimer(uint32_t TIMx, uint32_t TIMFrequency);
uint8_t		TIMER_GetAndClearInt(uint32_t TIMx);
uint32_t	TIMER_GetAndClearValue(uint32_t TIMx);
uint32_t	TIMER_GetAndClearValueMs(uint32_t TIMx);
uint32_t	TIMER_GetAndClearValueUs(uint32_t TIMx);
uint32_t	TIMER_GetAndClearValueUs_10(uint32_t TIMx);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_TIMER_H_ */
