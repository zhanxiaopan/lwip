/**
 *  \file new 1
 *  \brief Brief
 */

#ifndef __ETHERNET_H_
#define __ETHERNET_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes --------------------------------------------------------------------------------*/
#include "bsp_config.h"					// bsp_CONFIG Header file
#include "bsp_GPIO.h"					// bsp_GPIO Header file
#include "bsp_TIMER.h"					// TIMER Header file
#include "driverlib/emac.h"				// Ethernet Module Header file
#include "inc/hw_emac.h"				// Ethernet Hardware Header file
#include "inc/hw_nvic.h"				// NVIC Hardware Header file
#include "netconf.h"					// LWIP entry point
#include "RandomMAC.h"

/* Configurations --------------------------------------------------------------------------*/

/* Definitions -----------------------------------------------------------------------------*/


/* Functions -------------------------------------------------------------------------------*/
void 		Ethernet_Init(void);
//void 		Ethernet_Loop(void);

void		EthernetLoop_UpdateLink(void);
void		EthernetLoop_TCP_Process(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ETHERNET_H_ */
#include "bsp_config.h"
