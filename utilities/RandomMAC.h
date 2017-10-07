/**
 * This header file is used with RandomMAC.c
 */

//#include "eeprom.h"
#ifndef RANDOM_MAC
#define RANDOM_MAC

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "rng.h"
#include "hw_types.h"
#include "eeprom.h"

extern uint8_t MAC0;
extern uint8_t MAC1;
extern uint8_t MAC2;
extern uint8_t MAC3;
extern uint8_t MAC4;
extern uint8_t MAC5;

void gen_mac_addr();

#endif
