/**
 *  @file lec.hc
 *  @author wenlong.li
 *  @version v0.1
 *  @brief implement the non-block control of led
 *  @date 2016.08.03
 *  @details only bi-color LED implemented here.
 *           also rely on the implementation of GPIO tag mechanism.
 */

 
#include "led.h"

static led_bicolor_setcolor (BICOLOR_LED_T* led, LED_BI_COLOR_T color);

/**
 *  @brief control the behavior of bi color leds, based on the timer in unit of ms.
 *  @param [in] led led to control
 *  @return none
 *  @details In this function we use lots of "magic" nummeric operation.
 *           So highly rely on the enum value of stage(1 for on, 0 for off), color_grn(1), color_red(2). dont change them in led.h.
 *           function implementation is a bit "conciser" but not robust. take care when using it.
 */ 
void bicolor_led_control (BICOLOR_LED_T* led)
{
    static uint32_t last_timerval = 0;
	uint16_t flash_timer = 0, period = 0;
    LED_BI_COLOR_T stage = LED_BI_COLOR_GRN;
    
    period = led->flash_off_period + led->flash_on_period;
    flash_timer = led->Timer() % period;
    
    // just simply use the global system timer, we dont care the prefix(0~period ms) time when starting control.
    // the time deviation when changing period is also negligible here.
    // stage - 0: flash off, 1: flash on,
    if (flash_timer >= led->flash_on_period) stage = LED_BI_COLOR_OFF;
    else stage = LED_BI_COLOR_GRN;

    if(led->alive_time > 0) {
    	led->alive_time -= (led->Timer() - last_timerval);
    } else if (led->alive_time <= 0){
    	// timer down to 0, turn off the LED. reset to forver-alive mode.
    	if (led->alive_time != LED_ALIVE_TIME_INFINITE) {
    		led->state = LED_BI_STATE_OFF;
    	}
    	led->alive_time = LED_ALIVE_TIME_INFINITE;
    }
    last_timerval = led->Timer();
	
    switch (led->state)
	{
        case LED_BI_STATE_OFF:
            led_bicolor_setcolor(led, LED_BI_COLOR_OFF);
//            led_bicolor_setcolor(led, (LED_BI_COLOR_T)(stage & LED_BI_COLOR_GRN));
            break;
        case LED_BI_STATE_FLASH_GREEN:
            led_bicolor_setcolor(led, (LED_BI_COLOR_T)(stage & LED_BI_COLOR_GRN));
            break;
        case LED_BI_STATE_FLASH_RED:
            led_bicolor_setcolor(led, (LED_BI_COLOR_T)(stage * LED_BI_COLOR_RED));
            break;
        case LED_BI_STATE_STEADY_GREEN:
            led_bicolor_setcolor(led, LED_BI_COLOR_GRN);
            break;
        case LED_BI_STATE_STEADY_RED:
            led_bicolor_setcolor(led, LED_BI_COLOR_RED);
            break;
        case LED_BI_STATE_ALT_REDGREEN:
            // on_period for grn, off_period for red
            // if stage=1, LED_BI_COLOR_RED-stage is 1, which means grn.
            // if stage=0 (off),, LED_BI_COLOR_RED-stage is 2, which means red. 
            led_bicolor_setcolor(led, (LED_BI_COLOR_T)(LED_BI_COLOR_RED-stage));
            break;
        case LED_BI_STATE_CTRL_OFFLINE:
        	// the control is taken over otherwhere, do nothing here.
        	break;
        default:
            // wrong state received. turn off the led.
            led_bicolor_setcolor(led, LED_BI_COLOR_OFF);
            break;
	};
}

/**
 *  @brief Set the state of bi color LED.
 *  @param [in] led     whose state to set.
 *  @param [in] bistate state to set.
 *  @return none
 *  @details Details
 */
void led_bicolor_setstate (BICOLOR_LED_T* led, LED_BI_STATE_T bistate) {
    led->state = bistate;
    led->alive_time = LED_ALIVE_TIME_INFINITE;		// By default, keep the led alvie for ever.
}


/**
 *  @brief Set the state of bi color LED.
 *  @param [in] led     whose state to set.
 *  @param [in] bistate state to set.
 *  @param [in] atime alive time of the led
 *  @return none
 *  @details Details
 */
void led_bicolor_setstate_temp (BICOLOR_LED_T* led, LED_BI_STATE_T bistate, int16_t atime) {
	if(atime <= 0) {
		// report the error
		// return.
	}
    led->state = bistate;
    led->alive_time = atime;
}

/**
 *  @brief Brief
 *  @param [in] led        Parameter_Description
 *  @param [in] on_period  Parameter_Description
 *  @param [in] off_period Parameter_Description
 *  @return Return_Description
 *  @details Details
 */
void led_bicolor_setperiod (BICOLOR_LED_T* led, uint16_t on_period, uint16_t off_period) {
    led->flash_on_period = on_period;
    led->flash_off_period = off_period;
}

/**
 *  @brief Brief
 *  @param [in] led              Parameter_Description
 *  @param [in] pin_red          Parameter_Description
 *  @param [in] pin_grn          Parameter_Description
 *  @param [in] state            Parameter_Description
 *  @param [in] flash_on_period  Parameter_Description
 *  @param [in] flash_off_period Parameter_Description
 *  @param [in] Timer            Parameter_Description
 *  @return Return_Description
 *  @details Details
 */
void led_bicolor_construct (BICOLOR_LED_T* led,
                            GPIOTag_TypeDef pin_red,
                            GPIOTag_TypeDef pin_grn,
                            LED_BI_STATE_T state,
                            uint16_t flash_on_period,
                            uint16_t flash_off_period
                            ) {
    led->pin_red = pin_red;
    led->pin_grn = pin_grn;
    led->state = LED_BI_STATE_OFF;
    led->flash_on_period = flash_on_period;
    led->flash_off_period = flash_off_period;
    led->Timer = Time_GetMs;

    led->alive_time = LED_ALIVE_TIME_INFINITE; 		// default is infinite (-9999).
}


/**
 *  @brief set the color of bicolor led
 *  @param [in] led led to operate
 *  @param [in] color color, including off state
 *  @return none
 *  @details this is the interface to hardware circuit of LED.
 *           should be customized to fit different hardware.
 */
static led_bicolor_setcolor (BICOLOR_LED_T* led, LED_BI_COLOR_T color) {
    switch (color) {
        case LED_BI_COLOR_OFF:
            GPIO_TagWrite(led->pin_grn, LED_BICOLOR_TURNOFF);
            GPIO_TagWrite(led->pin_red, LED_BICOLOR_TURNOFF);
            break;
        case LED_BI_COLOR_GRN:
            GPIO_TagWrite(led->pin_grn, LED_BICOLOR_TURNON);
            GPIO_TagWrite(led->pin_red, LED_BICOLOR_TURNOFF);
            break;
        case LED_BI_COLOR_RED:
            GPIO_TagWrite(led->pin_grn, LED_BICOLOR_TURNOFF);
            GPIO_TagWrite(led->pin_red, LED_BICOLOR_TURNON);
            break;
        default:
            GPIO_TagWrite(led->pin_grn, LED_BICOLOR_TURNOFF);
            GPIO_TagWrite(led->pin_red, LED_BICOLOR_TURNOFF);
            break;
    }    
}

