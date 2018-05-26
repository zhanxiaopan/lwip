/*
 * flowsensor.h
 *
 *  Created on: Mar 21, 2016
 *      Author: CNWELI4
 */

#ifndef FLOWSENSOR_H_
#define FLOWSENSOR_H_

#include "bsp_GPIO.h"

#define FS_TIME_LEN_TO_COUNT_PULSE (100)

// time_interval from MCU start has no meaning actually. and result could be a very large one.dont care about it.
typedef struct
{
	double volumerate_lpm;
	uint32_t pulse_count;		// the pulse num within the time, which is decided by .time_len_to_cnt.
	uint16_t time_interval;		// the time interval between 2 pulse. when capture the 1st pulse form 0lpm, this value could be quite big.
	uint16_t pulse_freq;
	uint8_t pulse_num_in_pre_period;
	uint8_t pulse_in_period[FS_TIME_LEN_TO_COUNT_PULSE];	// the num of pulse within one ws_process period. (currently the period is WS_PROCESS_RUN_PERIOD ms)
	//uint8_t time_len_to_cnt;	// the time lenth (how manu periods) to count the pulse. the result is stored in pulse_count.
} flowrate_typedef;

extern flowrate_typedef fls_1;
extern flowrate_typedef fls_2;
extern GPIO_pinInfo pin_fs_input_1;
extern GPIO_pinInfo pin_fs_input_2;

extern void flowsensor_init (void);

#endif /* FLOWSENSOR_H_ */
