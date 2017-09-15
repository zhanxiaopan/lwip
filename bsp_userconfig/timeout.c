/**
 *  \file new 1
 *  \brief Brief
 */

/* Includes --------------------------------------------------------------------------------*/
#include "timeout.h"     // header file

/* Private declarations --------------------------------------------------------------------*/

/* Initialization functions ----------------------------------------------------------------*/
/**
  * @brief	Initialize the timeout.
  * @param	TO: pointer to a TIMEOUT_Struct.
  * @param	type: type of timeout.
  *			This parameter can be an enumerator of @ref TIMEOUT_TYPE_TypeDef.
  * @param	timeoutTime: desired timeout time.
  * @retval	none.
  */
void TIMEOUT_Struct_Init(TIMEOUT_Struct * TO, TIMEOUT_TYPE_TypeDef type, uint32_t timeoutTime)
{
	TO->type = type;
	TO->timeoutTime = timeoutTime;
	if (TO->type & TIMEOUT_TYPE_START_RELOADED)
	{
		// In case of call used as a tick, set tickCount to 0
		if (TO->type & TIMEOUT_TYPE_USE_TICK_CALL) TO->tickCount = 0;
		// Else save initial time
		else TO->startTime = Time_GetMs();
	}
	if (TO->type & TIMEOUT_TYPE_START_EXPIRED)
	{
		// In case of call used as a tick, set tickCount to timeoutTime
		if (TO->type & TIMEOUT_TYPE_USE_TICK_CALL) TO->tickCount = TO->timeoutTime;
		// Else set startTime timeoutTime before
		else TO->startTime = Time_GetMs()-TO->timeoutTime;
	}
}
/* Management functions --------------------------------------------------------------------*/
/**
  * @brief	Set the desired timeout time.
  * @param	TO: pointer to a TIMEOUT_Struct.
  * @param	timeoutTime: desired timeout time.
  * @retval	none.
  */
void TIMEOUT_SetTimeOut(TIMEOUT_Struct * TO, uint32_t timeoutTime)
{
	TO->timeoutTime = timeoutTime;
}
/**
  * @brief	Start/Restart the timer.
  * @param	TO: pointer to a TIMEOUT_Struct.
  * @retval	none.
  */
void TIMEOUT_Start(TIMEOUT_Struct * TO)
{
	// In case of call used as a tick, set tickCount to 0
	if (TO->type & TIMEOUT_TYPE_USE_TICK_CALL) TO->tickCount = 0;
	// Otherwise reset the start time
	else TO->startTime = Time_GetMs();
}
/**
  * @brief	Set the desired timeout time, then start the count.
  * @param	TO: pointer to a TIMEOUT_Struct.
  * @param	timeoutTime: desired timeout time.
  * @retval	none.
  */
void TIMEOUT_SetAndStart(TIMEOUT_Struct * TO, uint32_t timeoutTime)
{
	TIMEOUT_SetTimeOut(TO, timeoutTime);
	TIMEOUT_Start(TO);
}

/**
  * @brief	Verify if timeout expired.
  * 		@note	If auto reload, it reset the time
  * @param	TO: pointer to a TIMEOUT_Struct.
  * @retval	1 if expired, 0 else.
  */
inline uint8_t TIMEOUT_IsExpired(TIMEOUT_Struct * TO)
{
	uint8_t IsExpired;
	// In case of call used as a tick
	if (TO->type & TIMEOUT_TYPE_USE_TICK_CALL)
	{
		// Increment tickCount
		TO->tickCount++;
		// Verify if the tick exceeded
		IsExpired = ((TO->tickCount) >= (TO->timeoutTime)) ? 1 : 0;
	}
	// Otherwise verify if the timeout expired
	else IsExpired = ((Time_GetMs()-TO->startTime) >= (TO->timeoutTime)) ? 1 : 0;
	// If auto reload, restart timer
	if ((IsExpired==1) && (TO->type & TIMEOUT_TYPE_AUTO_RELOAD)) TIMEOUT_Start(TO);
	return IsExpired;
}

/**
  * @brief	Get the timeoutTime time Set.
  * @param	TO: pointer to a TIMEOUT_Struct.
  * @retval	the timeoutTime time Set.
  */
inline uint32_t TIMEOUT_GetTimeSet(TIMEOUT_Struct * TO)
{
	return TO->timeoutTime;
}

/**
  * @brief	Get the timeoutTime time Left.
  * @param	TO: pointer to a TIMEOUT_Struct.
  * @retval	the timeoutTime time Left.
  */
inline uint32_t TIMEOUT_GetTimeLeft(TIMEOUT_Struct * TO)
{
	// In case of call used as a tick
	if (TO->type & TIMEOUT_TYPE_USE_TICK_CALL)
	{
		if ((TO->tickCount) >= (TO->timeoutTime)) return 0;
		else return ((TO->timeoutTime)-(TO->tickCount));
	}
	// In case of normal timer
	else
	{
		if ((Time_GetMs()-TO->startTime) >= (TO->timeoutTime)) return 0;
		else return ((TO->timeoutTime)-(Time_GetMs()));
	}
}



