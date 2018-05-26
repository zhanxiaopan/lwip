/**
 *  \file led.h
 *  \brief interface to LED config/control
 */

#ifndef LED_H
#define LED_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "bsp_config.h"
#include "bsp_GPIO.h"
 
// in our system, LED is low active.
#define LED_BICOLOR_TURNON 0
#define LED_BICOLOR_TURNOFF 1

#define LED_ALIVE_TIME_INFINITE (-9999)

typedef enum {
	LED_BI_STATE_OFF = 0,
	LED_BI_STATE_FLASH_GREEN = 1,
	LED_BI_STATE_FLASH_RED = 2,
	LED_BI_STATE_STEADY_GREEN = 3,
	LED_BI_STATE_STEADY_RED = 4,
	LED_BI_STATE_ALT_REDGREEN = 5,
	LED_BI_STATE_CTRL_OFFLINE = 6			// release the control of led to external/non-periodic operations.
} LED_BI_STATE_T;

typedef enum {
    LED_BI_COLOR_OFF = 0,
	LED_BI_COLOR_GRN = 1,
	LED_BI_COLOR_RED = 2   
} LED_BI_COLOR_T;

typedef struct BICOLOR_LED_T{
	// attributes
    GPIOTag_TypeDef pin_red;
	GPIOTag_TypeDef pin_grn;
	LED_BI_STATE_T state;
	uint16_t flash_on_period;				// in ms. the period of on state of LED.
    uint16_t flash_off_period;             	// in ms. the period of off state of LED
    uint16_t flash_timer;
    int16_t alive_time;					// in ms, -1 means infinite. 100 means the led will blink or keep on for 100ms.
    // operators
    uint32_t (*Timer) (void);
}BICOLOR_LED_T;



extern void bicolor_led_control (BICOLOR_LED_T* led);
void led_bicolor_setstate (BICOLOR_LED_T* led, LED_BI_STATE_T bistate);
void led_bicolor_setstate_temp (BICOLOR_LED_T* led, LED_BI_STATE_T bistate, int16_t atime);
void led_bicolor_setperiod (BICOLOR_LED_T* led, uint16_t on_period, uint16_t off_period);
void led_bicolor_construct (BICOLOR_LED_T* led, GPIOTag_TypeDef pin_red, GPIOTag_TypeDef pin_grn, LED_BI_STATE_T state, uint16_t flash_on_period,
        					uint16_t flash_off_period);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LED_H */
