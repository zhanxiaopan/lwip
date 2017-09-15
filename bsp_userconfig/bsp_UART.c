/**
 *  \file new 1
 *  \brief Brief
 */

/* Includes --------------------------------------------------------------------------------*/
#include "bsp_UART.h" 		// header file

/* Private declarations --------------------------------------------------------------------*/

/* UART functions --------------------------------------------------------------------------*/
/**
  * @brief	Configure and activate the selected UART.
  * @param  UARTx_BASE: where x can be 1 to 7 to define the UART peripheral.
  * @param	UART_BaudRate: This member configures the UART communication baud rate.
  * @param	UART_WordLength: Specifies the number of data bits transmitted or received in a frame.
  *			This parameter should be UART_CONFIG_WLEN_x where x is 5 to 8.
  * @param	UART_StopBits: Specifies the number of stop bits transmitted.
  *			This parameter should be UART_CONFIG_STOP_x where x is ONE or TWO.
  * @param	UART_Parity: Specifies the parity mode.
  *			This parameter should be UART_CONFIG_PAR_x where x is NONE/EVEN/ODD/ONE/ZERO.
  * @retval	none.
  */
void UART_ConfigProperties(uint32_t UARTx_BASE, uint32_t UART_BaudRate,
						   uint32_t UART_WordLength, uint32_t UART_StopBits, uint32_t UART_Parity)
{
	// Initialize and enable the selected UART peripheral
	UARTConfigSetExpClk(UARTx_BASE, SystemCoreClock, UART_BaudRate, (UART_WordLength | UART_StopBits | UART_Parity));
}

/* Interrupt functions ---------------------------------------------------------------------*/
/**
  * @brief	Configure and activate the Receive interrupt.
  * @param  UARTx_BASE: where x can be 1 to 7 to define the UART peripheral.
  * @param	USART_IntChannel: Specifies the IRQ channel to be enabled.
  *			This parameter should be INT_UARTx where x can be 1 to 7.
  * @retval	None.
 */
void UART_ConfigRXInterrupt(uint32_t UARTx_BASE, uint8_t UART_IntChannel)
{
	// Reset the interrupt settings
    UARTIntDisable(UARTx_BASE, 0xFFFFFFFF);
    // Enable receive interrupts
    UARTIntEnable(UARTx_BASE, UART_INT_RX | UART_INT_RT);
    IntEnable(UART_IntChannel);
    // Set interrupt peripheral priority
    IntPrioritySet(UART_IntChannel, INTERRUPT_PRIORITY_PHER);
}
/**
  * @brief  Verify that the interrupt was the expected one,
  * 		then clear the interrupt state.
  * @param  UARTx_BASE: where x can be 1 to 7 to define the UART peripheral.
  * @retval 1 if interrupt was the one expected, 0 else.
 */
uint8_t UART_GetRXInterrupt(uint32_t UARTx_BASE)
{
    // Get and clear the current interrupt source(s)
    uint32_t UART_Interrupt = UARTIntStatus(UARTx_BASE, true);
    UARTIntClear(UARTx_BASE, UART_Interrupt);
    // Return the state of TX interrupt
    return ((UART_Interrupt & (UART_INT_RX | UART_INT_RT))?1:0);
}

/* Send functions --------------------------------------------------------------------------*/
/**
  * @brief  Transmits single data through the UARTx_BASE peripheral.
  * @param  UARTx_BASE: where x can be 1 to 7 to define the UART peripheral.
  * @param  charToSend: the data to transmit.
  * @retval None
  */
void UART_SendByte(uint32_t UARTx_BASE, char charToSend) {
	// Write a byte in the transmit FIFO: if FIFO full, wait
	UARTCharPut(UARTx_BASE, charToSend);
}
/**
  * @brief  Transmits a set of data through the UARTx_BASE peripheral.
  * @param  UARTx_BASE: where x can be 1 to 7 to define the UART peripheral.
  * @param  charsToSend: an array containing the data to transmit.
  * @param  ulCount: the number of chars to transmit.
  * @retval None
  */
void UART_SendBytes(uint32_t UARTx_BASE, char *charsToSend, uint16_t ulCount) {
    // Loop while there are more characters to send
    while(ulCount--)
    {
    	UART_SendByte(UARTx_BASE, *charsToSend++);
    }
}
