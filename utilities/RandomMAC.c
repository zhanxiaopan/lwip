/*
 * This is to generate MAC randomly and store the result in EEPROM.
 
*/
#include "RandomMAC.h"
#include "hw_sysctl.h"
#include "utilities_debug.h"

#include "bsp_eeprom_const.h"

#include <stdio.h>

/* 'M', 'A', 'C', '!'*/
#define EEPROM_FLAG_MAC_EXIST (0x4D434121)

/* 
 * Here we utilize the "trick" that those l_mac* would be stored
 * in consecutive memory in most C compilers.
 * That means: &l_mac1 = &l_mac0 + 1, &l_mac2 = &l_mac1 + 1, ...
 * To avoid memory leak, l_mac6, l_mac7 is also defined as placeholder.
*/
//uint8_t l_mac0 = 0, l_mac1 = 0, l_mac2 = 0, l_mac3 = 0, l_mac4 = 0, l_mac5 = 0, l_mac6 = 0, l_mac7 = 0;
static uint8_t mac_ready = 0;
uint8_t l_mac[8];

//EEPROMRead ((uint32_t*)&ws_i_fault_flow, EEPROMAddrFromBlock(1)+8, 8);
//EEPROMProgram ((uint32_t*)&ipaddr[0], EEPROMAddrFromBlock(2), 4);


void gen_mac_addr() {
    if(!mac_ready) {
        uint32_t flag_mac_exist = 0;
        EEPROMRead ((uint32_t*)&flag_mac_exist, EEPROM_ADDR_MAC_EXIST, 4);

        /* Check whether mac exist already and then read or generate mac addr accordingly. */
        if (flag_mac_exist == EEPROM_FLAG_MAC_EXIST) {
            /* MAC is set already. Read it from EEPROM */
            EEPROMRead ((uint32_t*)&l_mac[0], EEPROM_ADDR_MAC0, 8);	// This would read value to mac0, mac1, mac2 and mac3.
            //EEPROMRead ((uint32_t*)&l_mac[4], EEPROMAddrFromBlock(3)+8, 4);	// This would read value to mac4, mac5, mac6 and mac7.
            //printf("MAC Exists:");
        }
        else {
            /* No mac generated for this board yet. */
            //uint32_t ran_seed;

            // Get random number from the unique chip ID and use them as mac addr 0~2
            // here the UNIQUEID1 is ignored because it is always 0
            l_mac[0] =  rng_rand_byte_with_seed(HWREG(SYSCTL_UNIQUEID0));
            l_mac[1] =  rng_rand_byte_with_seed(HWREG(SYSCTL_UNIQUEID2));
            l_mac[2] =  rng_rand_byte_with_seed(HWREG(SYSCTL_UNIQUEID3));

            // sed the rng with adc noise
            rng_seed();
            l_mac[3] = rng_rand_byte();
            l_mac[4] = rng_rand_byte();
            l_mac[5] = rng_rand_byte();

            flag_mac_exist = EEPROM_FLAG_MAC_EXIST;
            EEPROMProgram ((uint32_t*)&flag_mac_exist, EEPROM_ADDR_MAC_EXIST, 4);
            EEPROMProgram ((uint32_t*)&l_mac[0], EEPROM_ADDR_MAC0, 8);
            //printf("MAC Generated:");
        }
        mac_ready = 1;
    }
//    } else  {
//        printf("MAC ready");
//    }
        printf("%X,",l_mac[0]);
        printf("%X,",l_mac[1]);
        printf("%X,",l_mac[2]);
        printf("%X,",l_mac[3]);
        printf("%X,",l_mac[4]);
        printf("%X\n",l_mac[5]);
}

uint8_t get_mac_addr(uint32_t index) {
    UTILS_ASSERT((index >= 0 && index < 6));
    return l_mac[index];
}
