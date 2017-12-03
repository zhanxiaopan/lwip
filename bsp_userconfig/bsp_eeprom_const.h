/*
 * bsp_eeprom_addr.h
 *
 *  Created on: Sep 13, 2017
 *      Author: CNTHXIE
 */

#ifndef BSP_USERCONFIG_BSP_EEPROM_CONST_H_
#define BSP_USERCONFIG_BSP_EEPROM_CONST_H_

#include <stdbool.h>
#include "eeprom.h"

#define EEPROM_ADDR_MAC_EXIST       EEPROMAddrFromBlock(3)
#define EEPROM_ADDR_MAC0            EEPROMAddrFromBlock(3)+4
#define EEPROM_ADDR_MAC4            EEPROMAddrFromBlock(3)+8

#define EEPROM_ADDR_WARNING_FLOW    EEPROMAddrFromBlock(1)
#define EEPROM_ADDR_FAULT_FLOW      EEPROMAddrFromBlock(1)+8
#define EEPROM_ADDR_LEAK_RESPONSE   EEPROMAddrFromBlock(1)+16
#define EEPROM_ADDR_DELAY           EEPROMAddrFromBlock(1)+20
#define EEPROM_ADDR_STARTUP_LEAK    EEPROMAddrFromBlock(1)+28

#define EEPROM_ADDR_SENSE_QTZ_ID    EEPROMAddrFromBlock(1)+36
#define EEPROM_ADDR_LEAK_DET_QTZ_ID EEPROMAddrFromBlock(1)+44

#define EEPROM_ADDR_IS_BOARD_USED   EEPROMAddrFromBlock(2)+16
#define EEPROM_VAL_IS_BOARD_USED    167

#define EEPROM_ADDR_NEW_IP_IS_SAVED EEPROMAddrFromBlock(2)+16
#define EEPROM_VAL_NEW_IP_SAVED     167

#define EEPROM_ADDR_IP_ADDR0        EEPROMAddrFromBlock(2)
#define EEPROM_ADDR_IP_ADDR1        EEPROMAddrFromBlock(2)+4
#define EEPROM_ADDR_IP_ADDR2        EEPROMAddrFromBlock(2)+8
#define EEPROM_ADDR_IP_ADDR3        EEPROMAddrFromBlock(2)+12

#define EEPROM_ADDR_BL_JUMP_CONFIG  EEPROMAddrFromBlock(2)+20
#define EEPROM_LEN_BL_JUMP_CONFIG   4

#define EEPROM_ALL_IN_ONE_CONFIG    EEPROMAddrFromBlock(8)


#define    JUMP_CONFIG_NAN          0xFFFFFFFF
#define    JUMP_CONFIG_NO_WAIT      0x00040004       //not avaliable yet
#define    JUMP_CONFIG_SHORT_WAIT   0x000A0000
#define    JUMP_CONFIG_LONG_WAIT    0x000AB000
#define    JUMP_CONFIG_FOREVER_WAIT 0xFF00FF00

#endif /* BSP_USERCONFIG_BSP_EEPROM_CONST_H_ */
