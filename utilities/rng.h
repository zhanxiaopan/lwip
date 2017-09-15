/**
 * This header file is used with rng.c
 */

/**
 * This function read the values from ADC,
 * the values are used to seed the srand()
 * function.
 *
 * "rng_seed" should be invoked before any
 * other "rng_*" functions.
 */
#include <stdio.h>
#include <stdint.h>


extern void rng_seed(void);

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
extern int32_t rng_rand(int32_t min, int32_t max);

/**
 * "rng_rand_byte" generate a byte value range
 * from 0 to 255.
 *
 * This function is only enable if the function
 * "rng_seed" has once been called.
 *
 * @return a uint32_t value representing the byte
 */
extern uint8_t rng_rand_byte();
