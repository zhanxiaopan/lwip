/*
 * upgrader.h
 *
 *  Created on: Dec 16, 2016
 *      Author: CNWELI4
 */

#ifndef APPLICATION_UPGRADER_H_
#define APPLICATION_UPGRADER_H_

#include <stdint.h>
#include "sys_config.h"
#include "bsp_config.h"

typedef enum {
	UPGRADER_IDLE = 0,
	REQUESTED = 1,
	DOWNLOADING = 2,
	DOWNLOADED = 3,
	FLASH_COPPYING = 4,
	FLASH_COPPIED = 5,
	READY_TO_JUMP = 6,
	UPGRADER_TIMEOUT = 7,
	UPGRADER_ERROR = 8
} FW_UPGRADER_STATUS_T;

typedef enum {
    NONE =  0,
    FLASHERASE_FAILED = 1,
    FLASHCOPY_FAILED = 2,
} TFTP_ERROR_LIST;

typedef struct FW_UPGRADER_T
{
	FW_UPGRADER_STATUS_T status;
	uint32_t *pDownloading;			// the pointer inside downloading buffer.

	uint32_t download_buf_start_addr;
	uint32_t download_buf_size;

	uint32_t main_app_buf_start_addr;
	uint32_t main_app_buf_size;

} FW_UPGRADER_T;

#ifndef USE_FILE_SYSTEM_IN_TFTP
#include <stdbool.h>
#include "flash.h"
extern uint32_t g_ui32TransferAddress;
extern uint32_t g_ui32DownloadBuffAddr;
extern uint32_t g_ui32WritingPosition;		// indicate the next byte position to be programmed. this could jump during downloading and copying stages.
extern uint8_t g_ui8DownloadError;
extern uint8_t g_ui8FlashError;
extern uint8_t g_ui8IsDownloadDone;
extern uint8_t g_ui8IsWrqReceived;
extern uint32_t g_ui32DownloadedSize;
#endif

extern FW_UPGRADER_T g_upgrader;
extern void upgrader_init(FW_UPGRADER_T *upgrader);
extern void upgrader_process();

#endif /* APPLICATION_UPGRADER_H_ */
