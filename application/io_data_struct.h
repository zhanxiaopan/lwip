/*
 * io_data_struct.h
 *
 *  Created on: 20170209
 *      Author: wenlong li
 */

#ifndef __IO_DATA_STRUCT_H
#define __IO_DATA_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ---------------------------------------------------------- */
#include <stdint.h>


/* Exported constants ------------------------------------------------ */
/*
 * input means data from remote to our device
 * output means data from our device to remote
*/
#define IO_DATA_OUTPUT_LEN 8
#define IO_DATA_INTPUT_LEN 8


/* Exported typedef ------------------------------------------------ */
/* start of input data struct definition*/
typedef union IO_DATA_INPUT_T
{
	struct
	{
		// di received from PNIO controller
		uint8_t		cmd_reset:1;			// doRstGxWaterSvr
		uint8_t		cmd_valve_ctr:1;		// doGxWateroff
		uint8_t		cmd_bypass:1;			// doGxWSBypass
		uint8_t		di_rvd_bit01:1;			// unused.
		uint8_t		di_rvd_bit02:1;			// unused.
		uint8_t		di_rvd_bit03:1;			// unused.
		uint8_t		di_rvd_bit04:1;			// unused.
		uint8_t		cmd_tpu_ctrl_update:1;			// #added by TMS

		// defined by ourselves
		uint8_t		cmd_flow_warning;		// goFlowWarningValue
		uint8_t     cmd_flow_fault;         // goFlowFaultValue
		uint8_t     cmd_leak_response;      // goLeakResponse
		uint8_t     cmd_delay;              // goDelay
		uint8_t     cmd_startup_leak;       // goStartupLeak

		uint16_t    di_rvd_byte_6_7;

	} data;
	uint8_t row[IO_DATA_INTPUT_LEN];
} IO_DATA_INPUT_T;
/* end of input data struct definition*/


/* start of output data struct definition*/
typedef union IO_DATA_OUTPUT_T
{
	struct
	{
		// must to have according to specs
		uint8_t 	diWS_OkToWeld:1;		// diGxWS_OktoWeld diGxWS_OktoWeld output for ws itself.
		uint8_t		isValveOff:1;			// diGxWS_ValveClsd 1: valve on, 0: valve off (opposite ??? )
		uint8_t		isBypassed:1;			// diGxWS_Bypassed 1: bypassed, 0: not bypassed
		uint8_t 	diWS_MinFlow:1;			// diGxWS_MinFlow
		uint8_t		diWS_CapLoss:1;			// diGxWS_CapLoss
		uint8_t		diWS_ACK:1;			    // #added by TMS
		uint8_t		sys_error:1;			// reserved
		uint8_t		diWS_PowerOk:1;			// diGxWS_PowerOK

		// defined by ourselves
		uint8_t 	nFlowrate;				// flowrate in 10*l/min

		uint8_t 	isLeakDetected:1;		// threshold of warning flow
		uint8_t		isFLowOK:1;				// 1: flow ok, 0: flow not ok
		uint8_t		isFlowWarning:1;		// 1: flow in warning state, 0: flow not in warning state
		uint8_t		isFlowFault:1;			// 1: flow in fault state, 0: flow not in fault state.
		uint8_t		isFlowOff:1;			// 1: flow off, 0: flow not off
		uint8_t		isValveFault:1;			// 1: valve in fault state, 0: valve not in fault state.
		uint8_t		t2o_rvd_bit3:1;			// reserved
		uint8_t		t2o_rvd_bit4:1;			// reserved

		uint8_t 	flow_warning;			// 8: flow warning, by TMS
		uint8_t 	flow_fault;			    // 8: flow fault, by TMS
		uint8_t		leak_response;			// 8: flow response, by TMS
		uint8_t     delay;                  // 8: delay, by TMS
		uint8_t     startup_leak;           // 8: startup_leak, by TMS
	} data;
	uint8_t row[IO_DATA_OUTPUT_LEN];
} IO_DATA_OUTPUT_T;
/* end of output data struct definition*/

/* Exported variables ------------------------------------------------ */
extern IO_DATA_OUTPUT_T iodata_out;
extern IO_DATA_INPUT_T iodata_in;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __IO_DATA_STRUCT_H */
