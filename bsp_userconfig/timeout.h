/**
 *  \file new 1
 *  \brief Brief
 */

#ifndef __TIMEOUT_H_
#define __TIMEOUT_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes --------------------------------------------------------------------------------*/
#include "bsp_config.h"					// bsp_CONFIG Header file

/* Configurations --------------------------------------------------------------------------*/

/* Definitions -----------------------------------------------------------------------------*/
/**
  * @brief	TIMEOUT types
  */
typedef enum
{
	TIMEOUT_TYPE_START_RELOADED =	0x01,	// If TIMEOUT_Struct_Init function is called, reload the time
	TIMEOUT_TYPE_START_EXPIRED =	0x02,	// If TIMEOUT_Struct_Init function is called, make sure it is expired
	TIMEOUT_TYPE_MANUAL_RELOAD =	0x04,	// Once expired it requires manual reloading
	TIMEOUT_TYPE_AUTO_RELOAD =		0x08,	// Once expired, reload on call of TIMEOUT_IsExpired()
	TIMEOUT_TYPE_USE_TICK_CALL =	0x10	// Instead of using system time use function call as tick
} TIMEOUT_TYPE_TypeDef;
/**
 * @brief	TIMEOUT Structure
 */
typedef struct
{
	TIMEOUT_TYPE_TypeDef type;		// Defines the type of timeout
	uint32_t startTime;				// Initial counting time
	uint32_t tickCount;				// Tick counted (used instead of startTime if TIMEOUT_TYPE_USE_TICK_CALL is used)
	uint32_t timeoutTime;			// Timeout Value
} TIMEOUT_Struct;

/* Functions -------------------------------------------------------------------------------*/
void			TIMEOUT_Struct_Init(TIMEOUT_Struct * TO, TIMEOUT_TYPE_TypeDef type, uint32_t timeoutTime);
void			TIMEOUT_SetTimeOut(TIMEOUT_Struct * TO, uint32_t timeoutTime);
void			TIMEOUT_Start(TIMEOUT_Struct * TO);
void			TIMEOUT_SetAndStart(TIMEOUT_Struct * TO, uint32_t timeoutTime);
inline uint8_t	TIMEOUT_IsExpired(TIMEOUT_Struct * TO);
inline uint32_t	TIMEOUT_GetTimeSet(TIMEOUT_Struct * TO);
inline uint32_t	TIMEOUT_GetTimeLeft(TIMEOUT_Struct * TO);
// Macro for calling a block of code periodically
//#define	TIMEOUT_CallPeriodically(blockToCall, periodMs) 															\
//do {																												\
//	static TIMEOUT_Struct TO_flag = {.type = TIMEOUT_TYPE_AUTO_RELOAD, .startTime = 0, .timeoutTime = periodMs};	\
//	if (TIMEOUT_IsExpired(&TO_flag)==1)																				\
//	{																												\
//		blockToCall;																								\
//	}																												\
//} while(0)



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __TIMEOUT_H_ */
