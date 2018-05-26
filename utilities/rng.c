/**
 * rng.c
 *
 * @author	J.Xie
 * @version	1.0
 * @date	11.07.2017
 *
 * This program facilitates the random number
 * generation as bytes for MAC address.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "inc/hw_memmap.h"
#include "driverlib/adc.h"
#include "driverlib/sysctl.h"
#include "utilities_debug.h"

#define ADC_SEQUENCER 1

#define PERIPH_ADC  SYSCTL_PERIPH_ADC0
#define ADC_BASE    ADC0_BASE

bool rng_initialized = false;

/**
 * This function read the values from ADC,
 * the values are used to seed the srand()
 * function.
 *
 * "rng_seed" should be invoked before any
 * other "rng_*" functions.
 */
void rng_seed(void) {
	uint32_t rng_adc_values[4];
	int32_t temp_seed;
	uint32_t seed;

    //enable the ADC0
    SysCtlPeripheralEnable(PERIPH_ADC);

    while(!SysCtlPeripheralReady(PERIPH_ADC));

    //define the ADC Sequencer
    ADCSequenceConfigure(ADC_BASE, ADC_SEQUENCER, ADC_TRIGGER_PROCESSOR, 0);

    //configure the steps
    ADCSequenceStepConfigure(ADC_BASE, ADC_SEQUENCER, 0, ADC_CTL_CH0 | ADC_CTL_IE );
    ADCSequenceStepConfigure(ADC_BASE, ADC_SEQUENCER, 1, ADC_CTL_CH1 | ADC_CTL_IE );
    ADCSequenceStepConfigure(ADC_BASE, ADC_SEQUENCER, 2, ADC_CTL_CH0 | ADC_CTL_IE );
    ADCSequenceStepConfigure(ADC_BASE, ADC_SEQUENCER, 3, ADC_CTL_CH1 | ADC_CTL_IE | ADC_CTL_END );

    //enable the sequence
    ADCSequenceEnable(ADC_BASE, ADC_SEQUENCER);

    //clear the interrupt flag
    ADCIntClear(ADC_BASE, ADC_SEQUENCER);

    //trigger ADC
    ADCProcessorTrigger(ADC_BASE, 1);

    //get sequencer values
    while(!ADCIntStatus(ADC_BASE, ADC_SEQUENCER, false));
    ADCIntClear(ADC_BASE, ADC_SEQUENCER);
    ADCSequenceDataGet(ADC_BASE, ADC_SEQUENCER,rng_adc_values);

    //generate the seed;
    temp_seed = (rng_adc_values[0]-rng_adc_values[2])*(rng_adc_values[1]-rng_adc_values[3]);
    seed = abs(temp_seed);
    //seed the rng
    srand(seed);

    //set rng_initialized flag
    rng_initialized = true;
}

/**
 * "rng_rand" generate a random 32-bit integer in
 * a given range.
 *
 * This function is only enable if the function
 * "rng_seed" has once been called.
 *
 * @param	min		the lower bound of the range
 * @param 	max		the upper bound of the range
 * @return 	a random signed integer in the range
 */
int32_t rng_rand(int32_t min, int32_t max) {
    UTILS_ASSERT((rng_initialized && max > min));
	int32_t rand_num = (rand() % (max - min)) + min;
	return rand_num;
}

/**
 * "rng_rand_byte" generate a byte value range
 * from 0 to 255.
 *
 * This function is only enable if the function
 * "rng_seed" has once been called.
 *
 * @return a uint8_t value representing the byte
 */
uint8_t rng_rand_byte() {
    UTILS_ASSERT(rng_initialized);
	uint8_t rand_byte = (uint8_t)(rand() % 256);
	return rand_byte;
}

/**
 * "rng_rand_byte_with_seed" generate a byte value
 * which is seeded by the given unsigned integer
 *
 * @param   seed    the user defined seed
 * @return  a random uint8_t random value
 */
uint8_t rng_rand_byte_with_seed(uint32_t seed) {
    srand(seed);
    return (uint8_t)(rand() % 256);
}
