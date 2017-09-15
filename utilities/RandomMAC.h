/**
 * This header file is used with RandomMAC.c
 */

//#include "eeprom.h"
#include "rng.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "hw_types.h"
#include "eeprom.h"

extern  uint8_t l_mac[10];
void gen_mac_addr();
