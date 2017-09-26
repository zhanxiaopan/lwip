/** @file
 *
 * @brief
 * PROFINET Simple I/O Example
 *
 * @details
 * This example provides a possible test environment for a simplified
 * conformance test.
 *
 * @copyright
 * Copyright 2010-2016 port GmbH Halle/Saale.
 * This software is protected Intellectual Property and may only be used
 * according to the license agreement.
 */
 
/* Includes ---------------------------------------------------------- */
#include <pn_includes.h>
#include "io_data_struct.h"
#include "system.h"
#include "RandomMAC.h"

extern void HAL_setupEthernet(uint8_t mac_addr[]);

PN_LOG_SET_ID(PN_LOG_ID_APPLICATION)


/* Private declaration ----------------------------------------------- */
#define MODULE_SIZE 64                      /**< module sizes */
#define PNIO_DATA_INPUT_LEN     IO_DATA_OUTPUT_LEN
#define PNIO_DATA_OUTPUT_LEN    IO_DATA_INTPUT_LEN

void main_initDevice(void);
IOD_STATUS_T main_callback(IOD_CALLBACK_ID_T, IOD_CALLBACK_T *);

static PN_BOOL_T flag_appReady = PN_FALSE;  /**< app ready flag */
static Integer32 flag_connID = 0;           /**< connection ID */

/* Exported variables ------------------------------------------------ */
#define pnio_data_out   iodata_in			// pnio_data_out means data from master(controller) to slave(device).
#define pnio_data_in    iodata_out			// pnio_data_out means data from slave to master

uint8_t flag_fieldbus_down = 1;				// flags used in our applicaton.


/**
 *  @brief Application Init.
 *  @return Return_Description
 *  @details Build up the device structure and initialize the Profinet stack.
 */
void pnio_app_init(void)
{
    RET_T status = PN_OK;                   /**< result status */

    // ** added to initialize the ethernet controller for PROFINET
    uint8_t mac_addr[] = {MAC0,MAC1,MAC2,MAC3,MAC4,MAC5};
//    printf("%x\n",MAC0);
//    printf("%x\n",MAC1);
//    printf("%x\n",MAC2);
//    printf("%x\n",MAC3);
//    printf("%x\n",MAC4);
//    printf("%x\n",MAC5);
//    uint8_t mac_addr[6];
//    gen_mac_addr();
//    mac_addr[0] =  l_mac[0];//MAC_ADDR0;
//    mac_addr[1] =  l_mac[1];//MAC_ADDR1;
//    mac_addr[2] =  l_mac[2];//MAC_ADDR2;
//    mac_addr[3] =  l_mac[3];//MAC_ADDR3;
//    mac_addr[4] =  l_mac[4];//MAC_ADDR4;
//    mac_addr[5] =  l_mac[5];//MAC_ADDR5;
    // Initialize the Ethernet Controller
    HAL_setupEthernet(mac_addr);
    IntPrioritySet(INT_EMAC0, INTERRUPT_PRIORITY_HIGH);

    /* Initialize PNIO Stack with callback function */
    status = IOD_Init(main_callback);
    //IOD_setDeviceName("portiodev12", 11);
    if (PN_OK != status) {
        PN_logErr("IOD_init FAILED - aborting...");
        return;
    }

    /* create slot and module structures */
    main_initDevice();

    memset(pnio_data_out.row, 0, PNIO_DATA_OUTPUT_LEN);
    memset(pnio_data_in.row, 0, PNIO_DATA_INPUT_LEN);

    /* Init a DCP cmd alive monitor due to the fucking implementation of DCP in PN. */
	alive_monitor_pn_dcp_cmd.AliveFlag = 0;
    alive_monitor_pn_dcp_cmd.IsAwaken = 0;
    alive_monitor_pn_dcp_cmd.TrigCount = 0;
    alive_monitor_pn_dcp_cmd.TrigThreshold = 3000;			// 3000 means 3s.
}


/**
 *  @brief Main process func of pnio.
 *  @return Return_Description
 *  @details No idea yet how frequent this func should be called.
 */
void pnio_process(void)
{
    /* perform periodical stack tasks
     * (will sleep 1s on multithreaded architectures) */
    OAL_stack_exec();


}


/**
 *  @brief Get and set io data in pnio channel.
 *  @return Return_Description
 *  @details This frequency is free to set according to user app. (10ms is currently in use)
 */
void pnio_app_iodata_update(void)
{
	Unsigned8 iops;

	/* mirror output data from submod 0:2:1 to input data from submod 0:1:1
	 * every 1000 cnt-count cycles */
	if ((PN_TRUE == flag_appReady)) {
		/* read data from output module */
		IOD_GetOutputData(0, 2, 1, pnio_data_out.row, PNIO_DATA_OUTPUT_LEN, &iops);

		/* check the state of iops */
		if (!(iops & PNIO_IOXS_GOOD)){
			/* IO state went bad.
			 * so set the output(controller->device) to a safe value
			 * report the failed IO com by set the flag.
			 */
			memset(pnio_data_out.row, 0, PNIO_DATA_OUTPUT_LEN);
			flag_fieldbus_down = 1;

			/* if no flash cmd received, NS led should flash red here */
			if (alive_monitor_pn_dcp_cmd.AliveFlag != 1) {
				led_bicolor_setstate(&pnio_led_ns, LED_BI_STATE_FLASH_RED);
			}
		}
		else {
			/* IO state went back to good
			 * so outputs can be set back to the data stored in the data variable
			 * report the good IO com status by resetting the flag.
			 */
			flag_fieldbus_down = 0;

			/* if no flash cmd received, NS led should keep grn here */
			if (alive_monitor_pn_dcp_cmd.AliveFlag != 1) {
				led_bicolor_setstate(&pnio_led_ns, LED_BI_STATE_STEADY_GREEN);
			}
		}

		/* copy data to input module */
		IOD_SetInputData(0, 1, 1, pnio_data_in.row, PNIO_DATA_INPUT_LEN, PNIO_IOXS_GOOD);
	}
}


/**
 *  @brief Create the device configuration (slots/modules/params).
 *  @return Return_Description
 *  @details Setup slots/modules and link them together.
 */
void main_initDevice(void)
{
    PN_logInfo("Initializing device structure");

    /* create subslots for slot 1 and 2 in API 0 */
    IOD_devNewSubSlot(0, 1, 1, AUTO_GEN);
    IOD_devNewSubSlot(0, 2, 1, AUTO_GEN);

    /* create submodules for modules 0x30, 0x31 and 0x32 */
    IOD_devNewSubModule(0x00000030, 0x00000001, PN_INPUT,  PNIO_DATA_INPUT_LEN,           0, AUTO_GEN);
    IOD_devNewSubModule(0x00000031, 0x00000001, PN_OUTPUT,           0, PNIO_DATA_OUTPUT_LEN, AUTO_GEN);
//    IOD_devNewSubModule(0x00000032, 0x00000001, PN_IO,     MODULE_SIZE, MODULE_SIZE, AUTO_GEN);

    /* plug modules into slots */
    IOD_plugSubModule(0, 1, 1, 0x30, 0x01, NULL);
    IOD_plugSubModule(0, 2, 1, 0x31, 0x01, NULL);
}


/**
 *  @brief Profinet Callback Handler
 *  @param [in] id callback ID
 *  @param [in] cb callback parameters
 *  @return Return_Description
 *  @details This function collects all callbacks from the stack and decides if the
 *  callback must be handled.
 */
IOD_STATUS_T main_callback(
    IOD_CALLBACK_ID_T id,                   /**< callback ID */
    IOD_CALLBACK_T *cb                      /**< callback parameters */
)
{
    IOD_STATUS_T ret = IOD_OK;              /**< return value */

#ifdef _ENABLE_TO_PRINT_CALLBACK_IDS
    /* don't print IOD_CB_NEW_IO_DATA as it would be triggered every cycle */
    if (IOD_CB_NEW_IO_DATA != id) {
        PN_logInfo("Callback: %s", LOG_resKeyVal(APP_CALLBACK, id, NULL));
    }
#endif

    /* handle callback IDs */
    switch (id) {
        case IOD_CB_APPL_READY:
            /* application ready was confirmed, start data handling if not
             * already running */
            if (PN_TRUE != flag_appReady) {
                flag_appReady = PN_TRUE;
                flag_connID = cb->data[0].pAR->id;
            }
            break;

        case IOD_CB_RELEASE_AR:
            /* AR was released, stop data handling if it was the first AR */
            if (flag_connID == cb->data[0].pAR->id) {
                flag_appReady = PN_FALSE;
                flag_connID = 0;
            }
            break;
        case IOD_CB_BLINK:
        	/* This is a DCP request to flash LED.
        	 * Seems that this dcp request not received in every pnio loop.
        	 * By test, we found that this case will be executed about every 2~3s.
        	 * So we simply enable the led to flash in the next 3s.
        	 * */
        	alive_monitor_pn_dcp_cmd.IsAwaken = 1;
        	led_bicolor_setperiod(&pnio_led_ns, 250, 250);
        	led_bicolor_setstate_temp(&pnio_led_ns, LED_BI_STATE_FLASH_GREEN, 3000);
            break;

        default:
            break;
    }

    return ret;
}
