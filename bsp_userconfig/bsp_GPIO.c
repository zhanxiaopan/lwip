/**
 *  \file new 1
 *  \brief Brief
 */

/* Includes --------------------------------------------------------------------------------*/
#include "bsp_GPIO.h"		// header file

/* Private declarations --------------------------------------------------------------------*/

/* Peripherals functions -------------------------------------------------------------------*/
/**
  * @brief	Parse the tag into port and pin.
  * @param	tag: The tag that specify the IO port and pin.
  * @retval	GPIO_pinInfo: The parsed IO port and pin.
  */
GPIO_pinInfo GPIO_Tag2PinInfo(GPIOTag_TypeDef tag)
{
	GPIO_pinInfo tempPinInfo;
	tempPinInfo.GPIO_PORTx_BASE = tag & 0xFFFFF000;
	tempPinInfo.GPIO_PIN_x = tag & 0xFF;
	return tempPinInfo;
}
/**
  * @brief	Parse the tag into GPIO_PinData.
  * @param	tag: The tag that specify the IO port and pin.
  * @retval	GPIO_PinData: The parsed IO port and pin data.
  */
GPIO_PinData GPIO_Tag2PinData(GPIOTag_TypeDef tag)
{
	// Get Pin Info
	GPIO_pinInfo tempPinInfo = GPIO_Tag2PinInfo(tag);
	// Get Pin Data
	GPIO_PinData tempPinData;
	tempPinData.PinRegister = (uint32_t *)(tempPinInfo.GPIO_PORTx_BASE + (tempPinInfo.GPIO_PIN_x << 2));
	tempPinData.PinValue = tempPinInfo.GPIO_PIN_x;
	return tempPinData;
}

/**
  * @brief	Config the basic properties for specific GPIO pin.
  * @param	tag: The tag that specify the IO port and pin.
  * @param	GPIO_SET: Specify the set to be used for the pin.
  * @param	GPIO_SPD: Specify the output speed for the pin.
  * @retval	None.
  */
inline void GPIO_TagConfigProperties(GPIOTag_TypeDef tag,
									 GPIO_SET_TypeDef GPIO_SET, GPIO_SPD_TypeDef GPIO_SPD)
{
	// Call the AF function, to have just one function where to deal with GPIO initialization
	GPIO_TagConfigPropertiesAF(tag, GPIO_SET, GPIO_SPD, 0);
}

/**
  * @brief	Config the properties for specific GPIO pin.
  * @param	tag: The tag that specify the IO port and pin.
  * @param	GPIO_SET: Specify the set to be used for the pin.
  * @param	GPIO_SPD: Specify the output speed for the pin.
  * @param	GPIO_AF: selects the pin to used as Alternate function.
  * @retval	None.
  */
void GPIO_TagConfigPropertiesAF(GPIOTag_TypeDef tag,
								GPIO_SET_TypeDef GPIO_SET, GPIO_SPD_TypeDef GPIO_SPD,
								uint32_t GPIO_AF)
{
	// Get GPIO_Mode and GPIO_Type from GPIO_SET
    uint32_t GPIO_Mode;
    uint32_t GPIO_Type;
	switch (GPIO_SET)
	{
		case GPIO_SET_IN_ANALOG:
			GPIO_Mode = GPIO_DIR_MODE_IN;
			GPIO_Type = GPIO_PIN_TYPE_ANALOG;
			break;
		case GPIO_SET_IN_FLOATING:
			GPIO_Mode = GPIO_DIR_MODE_IN;
			GPIO_Type = GPIO_PIN_TYPE_OD;
			break;
		case GPIO_SET_IN_PULLUP:
			GPIO_Mode = GPIO_DIR_MODE_IN;
			GPIO_Type = GPIO_PIN_TYPE_STD_WPU;
			break;
		case GPIO_SET_IN_PULLDOWN:
			GPIO_Mode = GPIO_DIR_MODE_IN;
			GPIO_Type = GPIO_PIN_TYPE_STD_WPD;
			break;
		case GPIO_SET_OUT_OPENDRAIN:
			GPIO_Mode = GPIO_DIR_MODE_OUT;
			GPIO_Type = GPIO_PIN_TYPE_OD;
			break;
		case GPIO_SET_OUT_PUSHPULL:
			GPIO_Mode = GPIO_DIR_MODE_OUT;
			GPIO_Type = GPIO_PIN_TYPE_STD;
			break;
		case GPIO_SET_AF_OPENDRAIN:
			GPIO_Mode = GPIO_DIR_MODE_HW;
			GPIO_Type = GPIO_PIN_TYPE_STD;
			break;
		case GPIO_SET_AF_PUSHPULL:
			GPIO_Mode = GPIO_DIR_MODE_HW;
			GPIO_Type = GPIO_PIN_TYPE_STD;
			break;
		default:
			GPIO_Mode = GPIO_DIR_MODE_IN;
			GPIO_Type = GPIO_PIN_TYPE_STD;
			break;
	}
	// Get GPIO_Strength from GPIO_SPD
    uint32_t GPIO_Strength;
	switch (GPIO_SPD)
	{
		case GPIO_SPD_LOW:
			GPIO_Strength = GPIO_STRENGTH_2MA;
			break;
		case GPIO_SPD_MID:
			GPIO_Strength = GPIO_STRENGTH_6MA;
			break;
		case GPIO_SPD_FAST:
			GPIO_Strength = GPIO_STRENGTH_10MA;
			break;
		case GPIO_SPD_HIGH:
			GPIO_Strength = GPIO_STRENGTH_12MA;
			break;
		default:
			GPIO_Strength = GPIO_STRENGTH_2MA;
			break;
	}
	// Get Pin Info
	GPIO_pinInfo tempPinInfo = GPIO_Tag2PinInfo(tag);
	// Configure IO
    GPIODirModeSet(tempPinInfo.GPIO_PORTx_BASE, tempPinInfo.GPIO_PIN_x, GPIO_Mode);
    GPIOPadConfigSet(tempPinInfo.GPIO_PORTx_BASE, tempPinInfo.GPIO_PIN_x, GPIO_Strength, GPIO_Type);
	// IF AF mode, set it up
	if (GPIO_Mode==GPIO_DIR_MODE_HW) GPIOPinConfigure(GPIO_AF);
}

/**
  * @brief	Read the IO pin value of the specific GPIO pin
  * @param	tag: The tag that specify the IO port and pin.
  * @retval	The specified pin value.
  * @note	Recommend to only use it for general-purpose DI and DO, not for AF mode.
  */
uint8_t GPIO_TagRead(GPIOTag_TypeDef tag)
{
	GPIO_pinInfo tempPinInfo = GPIO_Tag2PinInfo(tag);
	return (GPIOPinRead(tempPinInfo.GPIO_PORTx_BASE, tempPinInfo.GPIO_PIN_x)==0?0:1);
}

/**
  * @brief  Similar function as TagRead's. The difference is the return value is a 8-bit uint number rather than 1 or 0
  * @param  tag: The tag that specify the IO port and pins.
  * @retval The specified pin value.
  * @note   Recommend to only use it for general-purpose DI and DO, not for AF mode.
  */
uint8_t GPIO_TagStateRead(GPIOTag_TypeDef tag)
{
    GPIO_pinInfo tempPinInfo = GPIO_Tag2PinInfo(tag);
    return (GPIOPinRead(tempPinInfo.GPIO_PORTx_BASE, tempPinInfo.GPIO_PIN_x));
}
/**
  * @brief	Write the output value of the specific GPIO pin.
  * @param	tag: The tag that specify the IO port and pin.
  * @param	bitVal: The value to be written to the pin. 0 or !0(mostly use 0 or 1).
  * @retval	none.
  */
void GPIO_TagWrite(GPIOTag_TypeDef tag, uint8_t bitVal)
{
	GPIO_pinInfo tempPinInfo = GPIO_Tag2PinInfo(tag);
	GPIOPinWrite(tempPinInfo.GPIO_PORTx_BASE, tempPinInfo.GPIO_PIN_x, (bitVal?tempPinInfo.GPIO_PIN_x:0));
}
/**
  * @brief	Write the output value of the specific GPIO port (or pin composition).
  * @param	tag: The tag that specify the IO port and pin.
  * @param	portVal:  The value to be written to the pin. Set nBit to 0 or 1.
  * @retval	none.
  */
void GPIO_TagWritePort(GPIOTag_TypeDef tag, uint8_t portVal)
{
	GPIO_pinInfo tempPinInfo = GPIO_Tag2PinInfo(tag);
	GPIOPinWrite(tempPinInfo.GPIO_PORTx_BASE, tempPinInfo.GPIO_PIN_x, portVal);
}
/**
  * @brief  Write the output value of the specific GPIO pin if different from previous state.
  * @param  tag: The tag that specify the IO port and pin.
  * @param  bitVal:  The value to be written to the pin. 0 or !0(mostly use 0 or 1).
  * @retval none.
  */
void GPIO_TagWriteOnce(GPIOTag_TypeDef tag, uint8_t * bitOldVal, uint8_t bitNewVal)
{
  if (*bitOldVal!=bitNewVal) {
    *bitOldVal = bitNewVal;
    GPIO_TagWrite(tag, bitNewVal);
  }
}

/**
  * @brief  Similar function as TagRead's. The difference is the written value is a 8-bit uint number rather than 1 or 0
  * @param  tag: The tag that specify the IO port and pins.
  * @param  bitVal:  The value to be written to the pin. 0 or !0(mostly use 0 or 1).
  * @retval none.
  */
void GPIO_TagStateWriteOnce(GPIOTag_TypeDef tag, uint8_t * bitOldVal, uint8_t bitNewVal)
{
  if (*bitOldVal!=bitNewVal) {
    *bitOldVal = bitNewVal;
    GPIO_pinInfo tempPinInfo = GPIO_Tag2PinInfo(tag);
    GPIOPinWrite(tempPinInfo.GPIO_PORTx_BASE, tempPinInfo.GPIO_PIN_x, bitNewVal);  }
}


/**
  * @brief	Toggle the output value of the specific GPIO pin.
  * @param	tag: The tag that specify the IO port and pin.
  * @retval	none.
  */
void GPIO_TagToggle(GPIOTag_TypeDef tag)
{

	GPIO_pinInfo tempPinInfo = GPIO_Tag2PinInfo(tag);
	uint8_t bitVal = GPIOPinRead(tempPinInfo.GPIO_PORTx_BASE, tempPinInfo.GPIO_PIN_x);
	GPIOPinWrite(tempPinInfo.GPIO_PORTx_BASE, tempPinInfo.GPIO_PIN_x, ((bitVal==0)?tempPinInfo.GPIO_PIN_x:0));
}

/* Trigger Interrupt functions -------------------------------------------------------------*/
/**
  * @brief  Configures GPIO interrupt for the selected tag
  * @param	tag: The tag that specify the IO port and pin.
  * @param	GPIO_TRG: Specifies the trigger type.
  *			This parameter can be an enumerator of @ref GPIO_TRG_TypeDef.
  * @param	GPIO_IntChannel: Specifies the IRQ channel to be enabled.
  *			This parameter should be INT_GPIOx where x defines the interrupt port.
  * @retval The PinInfo data of the passed tag.
  */
GPIO_pinInfo GPIO_ConfigInterrupt(GPIOTag_TypeDef tag, GPIO_TRG_TypeDef GPIO_TRG, uint8_t GPIO_IntChannel)
{
	// Get GPIO_Trigger from GPIO_TRG
    uint32_t GPIO_Trigger;
	switch (GPIO_TRG)
	{
		case GPIO_TRG_RISING:
			GPIO_Trigger = GPIO_RISING_EDGE;
			break;
		case GPIO_TRG_FALLING:
			GPIO_Trigger = GPIO_FALLING_EDGE;
			break;
		case GPIO_TRG_RISING_FALLING:
		default:
			GPIO_Trigger = GPIO_BOTH_EDGES;
			break;
	}
	// Get Pin Info
	GPIO_pinInfo tempPinInfo = GPIO_Tag2PinInfo(tag);

    // Disable processor interrupts.
    IntMasterDisable();
	// Configure the interrupt
    IntEnable(GPIO_IntChannel);
    GPIOIntTypeSet(tempPinInfo.GPIO_PORTx_BASE, tempPinInfo.GPIO_PIN_x, GPIO_Trigger);
    GPIOIntEnable(tempPinInfo.GPIO_PORTx_BASE, tempPinInfo.GPIO_PIN_x);
    // Clear the timer interrupt for robustness
    GPIOIntClear(tempPinInfo.GPIO_PORTx_BASE, tempPinInfo.GPIO_PIN_x);
    // Set interrupt priority
    IntPrioritySet(GPIO_IntChannel, INTERRUPT_PRIORITY_PHER);
    // Enable processor interrupts.
    IntMasterEnable();
    // Return the GPIO_pinInfo
    return tempPinInfo;
}
/**
  * @brief  Verify that one of the passed pin generated the interrupt,
  * 		then clear the interrupt state.
  * @param	pinInfo: used to derive which port generated the interrupt and
  * 				 to inspect if any of the pin in it was the cause.
  * @retval 1 if interrupt was caused by one of the pin passed, 0 else.
  */
uint8_t GPIO_GetAndClearInt(GPIO_pinInfo pinInfo)
{
	// Verify that one of our pin generated the interrupt
	uint8_t tempReturn = (pinInfo.GPIO_PIN_x && GPIOIntStatus(pinInfo.GPIO_PORTx_BASE, 0))?1:0;
	// Always clear the interrupt state
	GPIOIntClear(pinInfo.GPIO_PORTx_BASE, pinInfo.GPIO_PIN_x);
	// Return the interrupt state
	return tempReturn;
}
