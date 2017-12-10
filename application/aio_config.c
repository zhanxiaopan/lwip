/*
 * aio_config.c
 *
 *  Created on: Dec 3, 2017
 *      Author: tms
 */

#include "aio_config.h"

#include "bsp_eeprom_const.h"

#include "sysctl.h"

#define _AIO_RECORD_BYTES    8
#define _AIO_RCD_SHIFT_LEN   8
#define _AIO_RECORD_CHK     0xA9

/**
 * global vars
 *
 * in eeprom the data should be saved
 * as: aio_pnio_config :: aio_network_sel :: aio_logo_sel :: _AIO_RECORD_HEAD
 */
uint8_t aio_bl_config;
uint8_t aio_network_sel;        //pnio/pnioio or eips
uint8_t aio_logo_sel;           //logo
uint8_t aio_bin_exist_flag;

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

    // 0 :: 0 :: 0 :: aio_bin_exist_flag :: aio_bl_config :: aio_network_sel :: aio_logo_sel :: _AIO_RECORD_CHK
    uint8_t chk = restore;
    //if(0)
    if(chk == _AIO_RECORD_CHK)
    {
        // 0 :: 0 :: 0 :: 0 :: aio_bin_exist_flag :: aio_bl_config :: aio_network_sel :: aio_logo_sel
        restore >>= _AIO_RCD_SHIFT_LEN;
        aio_logo_sel = restore;

        // 0 :: 0 :: 0 :: 0 :: 0 :: aio_bin_exist_flag :: aio_bl_config :: aio_network_sel
        restore >>= _AIO_RCD_SHIFT_LEN;
        aio_network_sel = restore;

        // 0 :: 0 :: 0 :: 0 :: 0 :: 0 :: aio_bin_exist_flag :: aio_bl_config
        restore >>= _AIO_RCD_SHIFT_LEN;
        aio_bl_config = restore;

        // 0 :: 0 :: 0 :: 0 :: 0 :: 0 :: 0 :: aio_bin_exist_flag
        restore >>= _AIO_RCD_SHIFT_LEN;
        aio_bin_exist_flag = restore;
    }
    else
        aio_initAioConfig();
}

/**
 * write the config into eeprom
 */
void aio_writeConfig()
{
    //
    //the configuration should be verified here first
    //
    if(aio_bin_exist_flag != AIO_BIN_BOTH && aio_bl_config == AIO_BL_REDI)
    {
        printf("Bad AIO State!@aio_WriteConfig");
        //should not reach this state!
        while(1);
    }

    //
    //if code above skipped!
    //

    uint32_t config = 0;

    // 0 :: 0 :: 0 :: 0 :: 0 :: 0 :: 0 :: aio_bin_exist_flag
    config += aio_bin_exist_flag;

    // 0 :: 0 :: 0 :: 0 :: 0 :: 0 :: aio_bin_exist_flag :: aio_bl_config
    config <<= _AIO_RCD_SHIFT_LEN;
    config += aio_bl_config;

    // 0 :: 0 :: 0 :: 0 :: 0 :: aio_bin_exist_flag :: aio_bl_config :: aio_network_sel
    config <<= _AIO_RCD_SHIFT_LEN;
    config += aio_network_sel;

    // 0 :: 0 :: 0 :: 0 :: aio_bin_exist_flag :: aio_bl_config :: aio_network_sel :: aio_logo_sel
    config <<= _AIO_RCD_SHIFT_LEN;
    config += aio_logo_sel;

    // 0 :: 0 :: 0 :: aio_bin_exist_flag :: aio_bl_config :: aio_network_sel :: aio_logo_sel :: _AIO_RECORD_CHK
    config <<= _AIO_RCD_SHIFT_LEN;
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
    aio_bl_config = AIO_BL_LOAD_EIPS;
    aio_network_sel = AIO_NETWORK_NONE;
    aio_logo_sel = AIO_LOGO_SMARTFLOW;
    aio_bin_exist_flag = AIO_BIN_NONE;
    //write
    aio_writeConfig();
}

/**
 * used when upgrader time out, reset to redirect state,
 * if the bins are not ready, wait for manually reset
 */
void aio_rollback()
{
    if( aio_bin_exist_flag == AIO_BIN_BOTH )
    {
        aio_bl_config = AIO_BL_REDI;
        //aio_network_sel unchanged;
        //aio_logo_sel unchanged;
        //aio_bin_exist_flag unchanged;
        aio_writeConfig();
        resetInit();
        resetLaunch();
    }
    else
        while(1);   //should manually restart the board
}
