/**
 *  \file new 1
 *  \brief Brief
 */

#ifndef __BSP_GPIO_H_
#define __BSP_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes --------------------------------------------------------------------------------*/
#include "bsp_config.h"			// bsp_CONFIG Header file
#include "driverlib/gpio.h"		// GPIO Header file

/* Configurations --------------------------------------------------------------------------*/

/* Definitions -----------------------------------------------------------------------------*/
/**
  * @brief	GPIO Port and PinSource information.
  */
typedef struct
{
  uint32_t GPIO_PORTx_BASE;
  uint8_t GPIO_PIN_x;
} GPIO_pinInfo;
/**
  * @brief	GPIO information.
  * 		Used in case of fast access to the GPIO
  */
typedef struct
{
  uint32_t * PinRegister;
  uint8_t PinValue;
} GPIO_PinData;
/**
  * @brief  GPIO setting parameters
  */
typedef enum
{
	GPIO_SET_IN_ANALOG =		0x01,
	GPIO_SET_IN_FLOATING =		0x02,
	GPIO_SET_IN_PULLUP =		0x04,
	GPIO_SET_IN_PULLDOWN =		0x08,
	GPIO_SET_OUT_OPENDRAIN =	0x10,
	GPIO_SET_OUT_PUSHPULL =		0x11,
	GPIO_SET_AF_OPENDRAIN =		0x12,
	GPIO_SET_AF_PUSHPULL =		0x13
}GPIO_SET_TypeDef;
/**
  * @brief  GPIO speed parameters
  */
typedef enum
{
	GPIO_SPD_LOW =				0x01,
	GPIO_SPD_MID =				0x02,
	GPIO_SPD_FAST =				0x03,
	GPIO_SPD_HIGH =				0x04
}GPIO_SPD_TypeDef;
/**
  * @brief  GPIO Trigger parameters
  */

typedef enum
{
	GPIO_TRG_RISING =			0x01,
	GPIO_TRG_FALLING =			0x02,
	GPIO_TRG_RISING_FALLING =	0x03
}GPIO_TRG_TypeDef;
/**
  * @brief	GPIO Fast operations
  */
#define GPIOFAST_LOW(PinData)			(*(PinData.PinRegister) = 0)
#define GPIOFAST_HIGH(PinData)			(*(PinData.PinRegister) = PinData.PinValue)
#define GPIOFAST_WRITE(PinData, bitVal)	(*(PinData.PinRegister) = ((bitVal==0)?0:PinData.PinValue))
#define GPIOFAST_READ_I(PinData)		((*(PinData.PinRegister) == 0)?0:1)
#define GPIOFAST_READ_O(PinData)		GPIOFAST_READ_I(PinData)
#define GPIOFAST_READINV_I(PinData)		((*(PinData.PinRegister) == 0)?1:0)
#define GPIOFAST_READINV_O(PinData)		GPIOFAST_READINV_I(PinData)

/* Functions -------------------------------------------------------------------------------*/
GPIO_PinData	GPIO_Tag2PinData(GPIOTag_TypeDef tag);
// GPIO byTag functions
inline void		GPIO_TagConfigProperties(GPIOTag_TypeDef tag,
		 	 	 	 	 	 	 	 	 GPIO_SET_TypeDef GPIO_SET, GPIO_SPD_TypeDef GPIO_SPD);
void 			GPIO_TagConfigPropertiesAF(GPIOTag_TypeDef tag,
		 	 	 	 	 	 	 	   	   GPIO_SET_TypeDef GPIO_SET, GPIO_SPD_TypeDef GPIO_SPD,
										   uint32_t GPIO_AF);
uint8_t			GPIO_TagRead(GPIOTag_TypeDef tag);
uint8_t         GPIO_TagStateRead(GPIOTag_TypeDef tag);
void 			GPIO_TagWrite(GPIOTag_TypeDef tag, uint8_t bitVal);
void			GPIO_TagWritePort(GPIOTag_TypeDef tag, uint8_t portVal);
void			GPIO_TagWriteOnce(GPIOTag_TypeDef tag, uint8_t * bitOldVal, uint8_t bitNewVal);
void            GPIO_TagStateWriteOnce(GPIOTag_TypeDef tag, uint8_t * bitOldVal, uint8_t bitNewVal);
void 			GPIO_TagToggle(GPIOTag_TypeDef tag);
// Trigger Interrupt functions
GPIO_pinInfo	GPIO_ConfigInterrupt(GPIOTag_TypeDef tag, GPIO_TRG_TypeDef GPIO_TRG, uint8_t GPIO_IntChannel);
uint8_t			GPIO_GetAndClearInt(GPIO_pinInfo pinInfo);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BSP_GPIO_H_ */
