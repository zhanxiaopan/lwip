/**
 *  \file new 1
 *  \brief Brief
 */

#ifndef __SYSTEM_H_
#define __SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes --------------------------------------------------------------------------------*/
#include "sys_config.h"									// system level configuration
#include "bsp_config.h"									// bsp_CONFIG Header file
#include "led.h"    									// LED header file
#include "timeout.h"    								// timeout header file
#include "flowsensor.h"
#include "ws_func.h"

/* Configurations --------------------------------------------------------------------------*/
// At the moment, memory functions are inherited from "motor.h"-->"PID.h"
#define DCBUS_REF_INPUT_IN_VOLT		  3515				// Initial DCBUS refernce. Unit: V*100
#define SYSTEM_MEM_BLOCK				10				// Memory block location (so far PID uses 0 to 9)
#define SYSTEM_MEM_KEYWORD			0x55AA				// Memory Keyword for default values usage

/* Definitions -----------------------------------------------------------------------------*/
/**
  * @brief	System States
  */
typedef enum
{
    SYSTEM_STATE_OK =			0x00,					// System is OK
    SYSTEM_STATE_IDLE =			ERR_CLASS_SYSTEM | 1,	// No test performed yet
    SYSTEM_STATE_DCBUSWARNING =	ERR_CLASS_SYSTEM | 2,	// DCBUS is a little out of the limits
    SYSTEM_STATE_DCBUSALARM =	ERR_CLASS_SYSTEM | 3,	// DCBUS is a lot out of limits
	SYSTEM_STATE_TEMPWARNING =	ERR_CLASS_SYSTEM | 4,	// TEMP is a little out of the limits
	SYSTEM_STATE_TEMPALARM =	ERR_CLASS_SYSTEM | 5,	// TEMP is a lot out of limits
} SYSTEM_STATE_TypeDef;
/**
  * @brief	Possible communication commands
  */
typedef enum
{
    SYSTEM_COM_IDLE =			0x00,					// No command Ongoing
    SYSTEM_COM_READY =			0x01,					// Go on ready
    SYSTEM_COM_ERROR =			0x02,					// Reset Error
    SYSTEM_COM_ABORT =			0x04,					// Abort the previous command
    SYSTEM_COM_SCREW_CW =		0x10,					// Screw clockwise
    SYSTEM_COM_SCREW_CW_OK =	0x11,					// Screw clockwise Done
	SYSTEM_COM_SCREW_CCW =		0x20,					// Screw counter-clockwise
	SYSTEM_COM_SCREW_CCW_OK =	0x21,					// Screw counter-clockwise Done
    SYSTEM_COM_UNSCREW_CW =		0x30,					// Un-Screw clockwise
    SYSTEM_COM_UNSCREW_CW_OK =	0x31,					// Un-Screw clockwise Done
	SYSTEM_COM_UNSCREW_CCW =	0x40,					// Un-Screw counter-clockwise
	SYSTEM_COM_UNSCREW_CCW_OK =	0x41,					// Un-Screw counter-clockwise Done
} SYSTEM_COMMAND_TypeDef;
/**
  * @brief	Command Errors
  */
typedef enum
{
    SYSTEM_COMERR_NONE =		0x00,					// No error
	SYSTEM_COMERR_WRONGCOM =	ERR_CLASS_COMMAND | 1,	//65 Wrong command received
	// Screw - Wrong value inserted for
	SYSTEM_COMERR_ENSPEED =		ERR_CLASS_COMMAND | 2,	//66 Engage Speed
	SYSTEM_COMERR_ENTIME =		ERR_CLASS_COMMAND | 3,	//67 Engage Duration
	SYSTEM_COMERR_ENLIMIT =		ERR_CLASS_COMMAND | 4,	//68 Engage Torque Limit
	SYSTEM_COMERR_RDSPEED =		ERR_CLASS_COMMAND | 5,	//69 Rundown Speed
	SYSTEM_COMERR_RDTRIG =		ERR_CLASS_COMMAND | 6,	//70 Rundown Torque trigger
	SYSTEM_COMERR_RDTOUT =		ERR_CLASS_COMMAND | 7,	//71 Rundown Timeout
	SYSTEM_COMERR_CLTORQUE =	ERR_CLASS_COMMAND | 8,	//72 Clamping Torque
	SYSTEM_COMERR_CLTIME =		ERR_CLASS_COMMAND | 9,	//73 Clamping Duration
	// UnScrew - Wrong value inserted for
	//SYSTEM_COMERR_ENSPEED =	ERR_CLASS_COMMAND | 2,	//66 Engage Speed
	SYSTEM_COMERR_ENTOUT =		ERR_CLASS_COMMAND | 10,	//74 Engage Timeout
	SYSTEM_COMERR_ENTRIG =		ERR_CLASS_COMMAND | 11,	//75 Engage Torque trigger
	SYSTEM_COMERR_UCSPEED =		ERR_CLASS_COMMAND | 12,	//76 UnClamp Speed
	SYSTEM_COMERR_UCLIMIT =		ERR_CLASS_COMMAND | 13,	//77 UnClamp Torque Limit
	SYSTEM_COMERR_UCTOUT =		ERR_CLASS_COMMAND | 14,	//78 UnClamp Timeout
	SYSTEM_COMERR_RUSPEED =		ERR_CLASS_COMMAND | 15,	//79 Runup Speed
	SYSTEM_COMERR_RUTIME =		ERR_CLASS_COMMAND | 16,	//80 Runup Duration
} SYSTEM_COMERR_TypeDef;
/**
 * @brief	Variables associated to the system
 */
typedef volatile struct
{
	// System
	SYSTEM_STATE_TypeDef		state;					// State of the system (OK or error number)
	uint8_t						error;					// Inclusive error (SYSTEM+COMMAND+MOTOR)
	uint16_t					dcbus_ref;				// Input voltage - Reference value
	uint16_t					voltage;				// Input voltage
	uint16_t					temperature;			// Board temperature
	// Commands
	SYSTEM_COMMAND_TypeDef		command;				// The actual command state
	SYSTEM_COMERR_TypeDef		commandErr;				// Error related to commands
} SYSTEM_Variables;

/* Functions -------------------------------------------------------------------------------*/
void 		system_init(void);
void 		system_loop(void);
// EIPS Functions
void		SYSTEM_UpdateEIPSOutput(void);
// HTTPD Functions
uint32_t	SYSTEM_RequestFromBrowser(const char * request, char * answer);

#ifdef ENABLE_DIDO_IN_WS
extern uint8_t cmd_di_1;
extern uint8_t cmd_di_2;
extern uint8_t cmd_di_3;
extern uint8_t cmd_di_4;
#endif


#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS
#define eip_led_ms sys_led_ms
#define eip_led_ns sys_led_ns
#elif WS_FIELDBUS_TYPE == FIELDBUS_TYPE_PNIO
#define pnio_led_ms sys_led_ms
#define pnio_led_ns sys_led_ns
#endif /* WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS */

extern BICOLOR_LED_T sys_led_ms;
extern BICOLOR_LED_T sys_led_ns;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SYSTEM_H_ */
