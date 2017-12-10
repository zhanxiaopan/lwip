/*
 * aio_config.h
 *
 *  Created on: Dec 3, 2017
 *      Author: tms
 */

#ifndef APPLICATION_AIO_CONFIG_H_
#define APPLICATION_AIO_CONFIG_H_

#include <stdint.h>

#define AIO_EIPS_BIN_EXIST  (aio_bin_exist_flag & 0x01)
#define AIO_PNIO_BIN_EXIST  (aio_bin_exist_flag & 0x02)

#define AIO_EIPS_VTABLE_ADDR    0x20000
#define AIO_PNIO_VTABLE_ADDR    0x60000

//ummm
#define AIO_BL_LOAD_EIPS    0x21
#define AIO_BL_LOAD_PNIO    0x22
#define AIO_BL_REDI         0x23
//default value: AIO_BL_LOAD_EIPS
extern uint8_t aio_bl_config;

#define AIO_NETWORK_NONE    0x10
#define AIO_NETWORK_PNIO    0x11
#define AIO_NETWORK_PNIOIO  0x12
#define AIO_NETWORK_EIPS    0x13
//default value: AIO_NETWORK_EIPS
extern uint8_t aio_network_sel;

#define AIO_LOGO_SMARTFLOW   0x30
#define AIO_LOGO_FLEXFLOW    0x31
//default value: AIO_LOGO_SMARTFLOW
extern uint8_t aio_logo_sel;

#define AIO_BIN_NONE        0x40
#define AIO_BIN_EIPS        0x41
#define AIO_BIN_PNIO        0x42
#define AIO_BIN_BOTH        0x43
extern uint8_t aio_bin_exist_flag;

void aio_readConfig();
void aio_writeConfig();

#endif /* APPLICATION_ALL_IN_ONE_CONFIG_H_ */
