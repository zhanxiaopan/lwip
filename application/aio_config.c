/*
 * aio_config.c
 *
 *  Created on: Dec 3, 2017
 *      Author: tms
 */

#include "aio_config.h"

#include "bsp_eeprom_const.h"

#include "sysctl.h"

#define _AIO_RECORD_BYTES    4
#define _AIO_RECORD_LENGTH   8
#define _AIO_RECORD_CHK     0xA9

/**
 * global vars
 *
 * in eeprom the data should be saved
 * as: aio_pnio_config :: aio_network_sel :: aio_logo_sel :: _AIO_RECORD_HEAD
 */
uint8_t aio_pnio_config;        //pnio or pnioio
uint8_t aio_network_sel;        //pnio/pnioio or eips
uint8_t aio_logo_sel;           //logo

void aio_initAioConfig();

/**
 * retrieve the config from the eeprom
 * and check the integrity
 */
void aio_readConfig()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0));
    EEPROMInit();

    uint32_t restore = 0;
    EEPROMRead ((uint32_t*)&restore, EEPROM_ALL_IN_ONE_CONFIG, _AIO_RECORD_BYTES);

    //aio_pnio_config :: aio_network_sel :: aio_logo_sel :: _AIO_RECORD_CHK
    uint8_t chk = restore;
    if(chk == _AIO_RECORD_CHK)
    {
        // 0 :: aio_pnio_config :: aio_network_sel :: aio_logo_sel
        restore >>= _AIO_RECORD_LENGTH;
        aio_logo_sel = restore;

        // 0 :: 0 :: aio_pnio_config :: aio_network_sel
        restore >>= _AIO_RECORD_LENGTH;
        aio_network_sel = restore;

        // 0 :: 0 :: 0 :: aio_pnio_config
        restore >>= _AIO_RECORD_LENGTH;
        aio_pnio_config = restore;
    }
    else
        aio_initAioConfig();
}

/**
 * write the config into eeprom
 */
void aio_writeConfig()
{
    uint32_t config = 0;

    // 0 :: 0 :: 0 :: aio_pnio_config
    config += aio_pnio_config;

    // 0 :: 0 :: aio_pnio_config :: aio_network_sel
    config <<= _AIO_RECORD_LENGTH;
    config += aio_network_sel;

    //0 :: aio_pnio_config :: aio_network_sel :: aio_logo_sel
    config <<= _AIO_RECORD_LENGTH;
    config += aio_logo_sel;

    //aio_pnio_config :: aio_network_sel :: aio_logo_sel :: _AIO_RECORD_CHK
    config <<= _AIO_RECORD_LENGTH;
    config += _AIO_RECORD_CHK;

    //write eeprom
    EEPROMProgram ((uint32_t*)&config, EEPROM_ALL_IN_ONE_CONFIG, _AIO_RECORD_BYTES);
}

/**
 * initialize the default values and
 * write into eeprom.
 */
void aio_initAioConfig()
{
    //set default values
    aio_pnio_config = AIO_PNIO_NONE;
    aio_network_sel = AIO_NET_SEL_EIPS;
    aio_logo_sel = AIO_LOGO_SMARTFLOW;
    //write
    aio_writeConfig();
}
