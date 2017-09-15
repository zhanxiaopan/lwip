/**
 *  @file eips_main.c
 *  @brief the main routine of eips process.
 *  @history
 *      created@20170209, wenlong li
 *  
 */

/* Includes ---------------------------------------------------------- */
#include "eips_main.h"
#include "eips_system.h"


/* Private declaration ----------------------------------------------- */
#define EIPS_IODATA_T2O iodata_out
#define EIPS_IODATA_O2T iodata_in

#define EIPS_IODATA_SIZE_O2T		(VS_MAX_DISCRETE_INPUTS_ON_UINT * EIPS_USEROBJ_ASM_ELEMENT_SIZE)
#define EIPS_IODATA_SIZE_T2O	    (VS_MAX_DISCRETE_OUTPUTS_ON_UINT * EIPS_USEROBJ_ASM_ELEMENT_SIZE)

#if EIPS_IODATA_SIZE_O2T != IO_DATA_INTPUT_LEN || EIPS_IODATA_SIZE_T2O != IO_DATA_OUTPUT_LEN
#error "Wrong size definition in EIPS IO data!"
#endif

void eips_userobj_callback(uint8_t nEvent, uint16_t nInst, uint8_t *pDataBuf, uint16_t nDataLen);

/* Exported variables ------------------------------------------------ */
uint8_t flag_fieldbus_down = 1;


/* Exported functions ------------------------------------------------ */
/**
 *  @brief the main eips process polled in mainloop
 *  @return none
 *  @details this should be called in same rate as the parameter in eips_rtasys_process.
 */
void eips_process_loop (void)
{
	// todo: add poll rate check, whether it's 10ms.
    
    eips_userobj_data_send(EIPS_IODATA_T2O.row, EIPS_IODATA_SIZE_T2O);
	eips_rtasys_process(10); // signal 10ms between calls
}

/**
 *  @brief init eips system
 *  @return none
 *  @details this should be called before we start eips loop.
 *  		 shall this be only called after the init of lwip?
 */
void eips_process_init (void)
{
	// Init EIPS IO data block. All value set to 0.
	memset(&EIPS_IODATA_T2O, 0, EIPS_IODATA_SIZE_T2O);
	memset(&EIPS_IODATA_O2T, 0, EIPS_IODATA_SIZE_O2T);
	// Init eips system.
	eips_rtasys_init();
	// Register eips callback function
	eips_userobj_cbf_register(&eips_userobj_callback);
}

/**
  * @brief	Callback function of EIPS.
  * @param	nEvent: may be EIPS_USEROBJ_EVENT_ASM_O2T_SAFE or EIPS_USEROBJ_EVENT_ASM_O2T_RCVD.
  * @param	nInst: instance ID to identify the client connected.
  * @param	pDataBuf: pointer where the data is stored.
  * @param	nDataLen: length of the data in bytes.
  * @retval	none.
  */
void eips_userobj_callback(uint8_t nEvent, uint16_t nInst, uint8_t *pDataBuf, uint16_t nDataLen)
{
	// We just care of data received (EIPS_USEROBJ_EVENT_ASM_O2T_RCVD)
	if (nEvent==EIPS_USEROBJ_EVENT_ASM_O2T_RCVD)
	{
		flag_fieldbus_down = 0;

		// Verify that nDataLen is of the right size
		if (nDataLen==EIPS_IODATA_SIZE_O2T)
		{
			memcpy(EIPS_IODATA_O2T.row, pDataBuf, nDataLen);
		}
		else
		{
			//Console_Error("EIPS CBk - Wrong data size: %dbytes", nDataLen);
		}
	}
	else if (nEvent == EIPS_USEROBJ_EVENT_ASM_O2T_SAFE) {
		flag_fieldbus_down = 1;
		// in case of not receiving the data successfully, reset O2T data to 0.
		memset(EIPS_IODATA_O2T.row, 0, nDataLen);
	}
}
