/*
 * aio_config.h
 *
 *  Created on: Dec 3, 2017
 *      Author: tms
 */

#ifndef APPLICATION_AIO_CONFIG_H_
#define APPLICATION_AIO_CONFIG_H_

#include <stdint.h>

//ummm

#define AIO_PNIO_NONE       0x10
#define AIO_PNIO_WITHOUT_IO 0x11
#define AIO_PNIO_WITH_IO    0x12
//default value: AIO_PNIO_NONE
extern uint8_t aio_pnio_config;

#define AIO_NET_SEL_PNIO  0x20
#define AIO_NET_SEL_EIPS  0x21
//default value: AIO_NET_SEL_EIPS
extern uint8_t aio_network_sel;

#define AIO_LOGO_SMARTFLOW   0x30
#define AIO_LOGO_FLEXFLOW    0x31
//default value: AIO_LOGO_SMARTFLOW
extern uint8_t aio_logo_sel;

void aio_readConfig();
void aio_writeConfig();

#endif /* APPLICATION_ALL_IN_ONE_CONFIG_H_ */
