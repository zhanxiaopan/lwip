/*
 * reset.h
 *
 *  Created on: Sep 22, 2017
 *      Author: CNTHXIE
 */

#ifndef UTILITIES_RESET_H_
#define UTILITIES_RESET_H_

#include <stdbool.h>

void resetInit();
void resetLaunch();
bool resetIsByWatchdog();

#endif /* UTILITIES_RESET_H_ */
