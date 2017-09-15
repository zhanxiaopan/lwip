/*
 * hal.h
 *
 * Hardware Abstraction Layer
 *
 */

#ifndef _HAL_H_
#define _HAL_H_

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_emac.h"
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"


#if defined(PART_TM4C129ENCZAD)
#  include "inc/tm4c129enczad.h"
#elif defined(PART_TM4C1294KCPDT)
#  include "inc/tm4c1294kcpdt.h"
#elif defined(PART_TM4C1294NCPDT)
#  include "inc/tm4c1294ncpdt.h"
#endif

#include "driverlib/emac.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom_map.h"

#include "pn_includes.h"


/* NVM Flash Section
 * Currently we take the bytes from 0x0F0000 up to 0x100000.
 */
#define OAL_NVM_BASE                                  0xF0000 // original 0x30000
#define OAL_NVM_SIZE                                  0x10000 // 64k == 16k block * 4

extern volatile uint64_t g_ulTickCounter;

extern uint32_t g_ui32ClkFreq;

typedef struct _HAL_GPIO_t_
{
	uint32_t base;
	uint8_t pin;
} HAL_GPIO_t;


// board specific IOs
extern const HAL_GPIO_t HAL_GPIO_PE0;
extern const HAL_GPIO_t HAL_GPIO_PE1;
extern const HAL_GPIO_t HAL_GPIO_PE2;

extern const HAL_GPIO_t HAL_GPIO_PB0;
extern const HAL_GPIO_t HAL_GPIO_PB1;
extern const HAL_GPIO_t HAL_GPIO_PB2;
extern const HAL_GPIO_t HAL_GPIO_PB3;

extern const HAL_GPIO_t HAL_GPIO_PC6;
extern const HAL_GPIO_t HAL_GPIO_PC7;

// board specific LEDs
extern const HAL_GPIO_t  HAL_GPIO_LED5; //
extern const HAL_GPIO_t  HAL_GPIO_LED6;
extern const HAL_GPIO_t  HAL_GPIO_LED7;
extern const HAL_GPIO_t  HAL_GPIO_LED8;


void HAL_setup();

void HAL_GPIOPinWrite(const HAL_GPIO_t *p_gpio, bool value);
bool HAL_GPIOPinRead(const HAL_GPIO_t *p_gpio);
void HAL_GPIOPinToggle(const HAL_GPIO_t *p_gpio);

void HAL_setupEthernet();

void SysTickIntHandler(void);

#endif /* _HAL_H_ */
