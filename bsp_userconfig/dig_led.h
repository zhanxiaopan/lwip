/*
 * dig_led.h
 *
 *  Created on: Sep 14, 2017
 *      Author: CNBIJIA2
 */

#ifndef _DIG_LED_H_
#define _DIG_LED_H_

/**
 * call this function periodically to update the digital led display.
 * before calling this function, the PA0~PA7 and PK0~PK7 must be configured.
 *
 * as GPIO output.
 * @param[input] data = the range of the data could be displayed is [0.0, 99.9].
 *                      the display will become blank if the data is out of range.
 * @return[none]
 */
void dig_led_update(float data);

/**
 * Initialize the Port A and K as GPIO outputs for
 * the digital LED module.
 */
void dig_led_init();

#endif /* _DIG_LED_H_ */
