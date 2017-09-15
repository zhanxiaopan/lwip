/**
 *  \file new 1
 *  \brief Brief
 */

#ifndef __BSP_UART_H_
#define __BSP_UART_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes --------------------------------------------------------------------------------*/
#include "bsp_config.h"			// bsp_CONFIG Header file
#include "driverlib/uart.h"		// UART Header file

/* Configurations --------------------------------------------------------------------------*/

/* Definitions -----------------------------------------------------------------------------*/

/* Functions -------------------------------------------------------------------------------*/
// UART Functions
void		UART_ConfigProperties(uint32_t UARTx_BASE, uint32_t UART_BaudRate,
								  uint32_t UART_WordLength, uint32_t UART_StopBits, uint32_t UART_Parity);
void		UART_ConfigRXInterrupt(uint32_t UARTx_BASE, uint8_t UART_IntChannel);
uint8_t		UART_GetRXInterrupt(uint32_t UARTx_BASE);
void		UART_SendByte(uint32_t UARTx_BASE, char charToSend);
void		UART_SendBytes(uint32_t UARTx_BASE, char *charsToSend, uint16_t ulCount);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_UART_H_ */
