/*
 * dig_led.c
 *
 *  Created on: Sep 14, 2017
 *      Author: CNBIJIA2
 */

#include <stdint.h>
#include <stdbool.h>
/* the gpio.h provided by tivaware */
#include "driverlib/gpio.h"
/* hw_memmap.h for GPIO_PORTx_BASE */
#include "inc/hw_memmap.h"

#include "sysctl.h"

#define GPIO_ALL_PINS   255

static uint32_t convert_to_bcd(uint16_t input);

/**
 * initialize the Port A and K as GPIO outputs for
 * the digital LED module.
 */
void dig_led_init()
{
    //init port A
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_ALL_PINS);
    //init port K
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOK);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOK));
    GPIOPinTypeGPIOOutput(GPIO_PORTK_BASE, GPIO_ALL_PINS);
}


void dig_led_write_blank()
{
	// step 1: set the LE_N high to prevent un-reliable display
	GPIOPinWrite(GPIO_PORTK_BASE, 0x80, 0x80);

	// step 2: turn off the dots
	GPIOPinWrite(GPIO_PORTK_BASE, 0x70, 0x00);

	// step 3: set the 7-segment leds as blank
	GPIOPinWrite(GPIO_PORTA_BASE, 0x0F, 0x0F);
	GPIOPinWrite(GPIO_PORTA_BASE, 0xF0, 0xF0);
	GPIOPinWrite(GPIO_PORTK_BASE, 0x0F, 0x0F);

	// step 4, set LE_N low to update the display
	GPIOPinWrite(GPIO_PORTK_BASE, 0x80, 0x00);
}

void dig_led_write_decimal(uint16_t decimal, uint16_t dot_pos)
{
    uint16_t data_bcd;
    bool first_dig_led_is_zero = false;
    // step 0: check and bound the input
    if (decimal > 999)
    {
        decimal = 999;
    }

    if (dot_pos > 3)
    {
        dot_pos = 0;
    }

    // step 1: set the LE_N high to prevent un-reliable display
    GPIOPinWrite(GPIO_PORTK_BASE, 0x80, 0x80);

    // step 2: convert the decimal to bcd data
    data_bcd = convert_to_bcd(decimal);

    // step 3: set the dot point
    switch (dot_pos)
    {
    case 0:
        GPIOPinWrite(GPIO_PORTK_BASE, 0x70, 0x00);
        break;
    case 1:
        GPIOPinWrite(GPIO_PORTK_BASE, 0x70, 0x40);
        break;
    case 2:
        GPIOPinWrite(GPIO_PORTK_BASE, 0x70, 0x20);
        break;
    case 3:
        GPIOPinWrite(GPIO_PORTK_BASE, 0x70, 0x10);
        break;
    default:
        GPIOPinWrite(GPIO_PORTK_BASE, 0x70, 0x00);
        break;
    }

    // step 4: set the 7-segment digital led
    if ((data_bcd>>8)&0x000F)
    {
        // the first digit is not 0
        GPIOPinWrite(GPIO_PORTA_BASE, 0x0F, (data_bcd >> 8)&0x0F);
    }
    else
    {
    	// the first digit is 0, then blank
    	GPIOPinWrite(GPIO_PORTA_BASE, 0x0F, 0xF);
    	first_dig_led_is_zero = true;
    }
/*
 * Next lines 113-131 were modified on 23/Sept./2017
 * to fix the problem that the second dig_led blink
 * when first was not.
 */
    if (first_dig_led_is_zero)
    {
        if (data_bcd&0xF0)
        {
            // the second digit is not 0
            GPIOPinWrite(GPIO_PORTA_BASE, 0xF0, data_bcd&0xF0);
        }
        else
        {

            // the first and second digit is 0, blank
            GPIOPinWrite(GPIO_PORTA_BASE, 0xF0, 0xF0);
        }
    }
    else
    {
        // the first digit is not 0, then do not blank
        GPIOPinWrite(GPIO_PORTA_BASE, 0xF0, data_bcd&0xF0);
    }

	// the third digital
	GPIOPinWrite(GPIO_PORTK_BASE, 0x0F, data_bcd&0x0F);

	// step 5, set LE_N low to update the display
	GPIOPinWrite(GPIO_PORTK_BASE, 0x80, 0x00);
}

/*
 * periodically call this function, to update the digital led display.
 * recommend to update ditital led display every 0.5s or every 1s.
 */
void dig_led_update(float data)
{
	uint16_t data_x_10;
	uint16_t data_bcd;

	// first set the LE_N high to prevent un-reliable display
	GPIOPinWrite(GPIO_PORTK_BASE, 0x80, 0x80);

	// do the calculation
	if ((data > 99.99) || (data < 0))
	{
		// error, blank
		data_bcd = 0x0FFF;
	}
	else
	{
		//
		data_x_10 = (uint16_t)(data*10.0);
		data_bcd = convert_to_bcd(data_x_10);
	}

	// set the dot point
	GPIOPinWrite(GPIO_PORTK_BASE, 0x70, 0x20);

	// set the 7-segment led
	if ((data_bcd>>8)&0x000F)
	{
		// the first digital is not 0
		GPIOPinWrite(GPIO_PORTA_BASE, 0x0F, (data_bcd >> 8)&0x0F);
	}
	else
	{
		// the first digital is 0, then blank
		GPIOPinWrite(GPIO_PORTA_BASE, 0x0F, 0xF);
	}
	// the second digital
	GPIOPinWrite(GPIO_PORTA_BASE, 0xF0, data_bcd&0xF0);
	// the third digital
	GPIOPinWrite(GPIO_PORTK_BASE, 0x0F, data_bcd&0x0F);

	// finally, set LE_N low to update the display
	GPIOPinWrite(GPIO_PORTK_BASE, 0x80, 0x00);
}

static uint32_t convert_to_bcd(uint16_t input)
{
	uint16_t shift = 0;
	uint16_t bcd = 0;
	while (input > 0)
	{
		bcd |= (input % 10) << (shift++ << 2);
		input /= 10;
	}
	return bcd;
}

