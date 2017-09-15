/*
 * This is to generate MAC randomly and store the result in EEPROM.
 
*/
#include "RandomMAC.h"
#include "hw_sysctl.h"
#include "utilities_debug.h"

#define EEPROM_ADDR_MAC_EXIST ()
/* 'M', 'A', 'C', '!'*/
#define EEPROM_FLAG_MAC_EXIST (0x4D434121)
#define EEPROM_ADDR_MAC0
#define EEPROM_ADDR_MAC1
#define EEPROM_ADDR_MAC2
#define EEPROM_ADDR_MAC3
#define EEPROM_ADDR_MAC4
#define EEPROM_ADDR_MAC5

/* 
 * Here we utilize the "trick" that those l_mac* would be stored
 * in consecutive memory in most C compilers.
 * That means: &l_mac1 = &l_mac0 + 1, &l_mac2 = &l_mac1 + 1, ...
 * To avoid memory leak, l_mac6, l_mac7 is also defined as placeholder.
*/
//uint8_t l_mac0 = 0, l_mac1 = 0, l_mac2 = 0, l_mac3 = 0, l_mac4 = 0, l_mac5 = 0, l_mac6 = 0, l_mac7 = 0;
uint8_t l_mac[8];

//EEPROMRead ((uint32_t*)&ws_i_fault_flow, EEPROMAddrFromBlock(1)+8, 8);
//EEPROMProgram ((uint32_t*)&ipaddr[0], EEPROMAddrFromBlock(2), 4);


void gen_mac_addr() {
	uint32_t flag_mac_exist = 0;
	EEPROMRead ((uint32_t*)&flag_mac_exist, EEPROMAddrFromBlock(3)+0, 4);
	
	/* Check whether mac exist already and then read or generate mac addr accordingly. */
	if (flag_mac_exist == EEPROM_FLAG_MAC_EXIST) {
		/* MAC is set already. Read it from EEPROM */
		EEPROMRead ((uint32_t*)&l_mac[0], EEPROMAddrFromBlock(3)+4, 8);	// This would read value to mac0, mac1, mac2 and mac3.
		//EEPROMRead ((uint32_t*)&l_mac[4], EEPROMAddrFromBlock(3)+8, 4);	// This would read value to mac4, mac5, mac6 and mac7.
	}
	else {
		/* No mac generated for this board yet. */		
		uint32_t ran_seed;
		
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
		EEPROMProgram ((uint32_t*)&flag_mac_exist, EEPROMAddrFromBlock(3), 4);
		EEPROMProgram ((uint32_t*)&l_mac[0], EEPROMAddrFromBlock(3)+4, 8);
	}
}

uint8_t get_mac_addr(uint32_t index) {
    UTILS_ASSERT((index >= 0 && index < 6));
    return l_mac[index];
}
