/**
 *  \file eips_main.h
 *  \brief declaration of eips_main.
 */

#ifndef __EIPS_MAIN_H
#define __EIPS_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ----------------------- */
#include "io_data_struct.h"

#define EIPS_REFRESHING_RATE	8
/* Exported macros ----------------------- */

/* Exported constants ----------------------- */

/* Exported variables ----------------------- */
extern uint8_t flag_fieldbus_down;

/* Exported functions ----------------------- */
void eips_process_init (void);
void eips_process_loop (void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __EIPS_MAIN_H */
