/**
 *  \file ws_func.c
 *  \brief implement ws control
 *  \history
 *  @20160622
 *   - refine the data link with filedbus io data object
 *   - the control signal in eth IO is now high volt valid. (would have some conflict with webserver)
 *   - add new signals in ws_read_eth_input and ws_update_eth_output.
 *   - ws_o_is_oktoweld will be set to 0 in case of detecting leakage.
 *   - change eips_userobj_callback() to reset all O2T data to 0 if not receiving data successfully.
 *   - use the nEvent argument in eips_userobj_callback to determine whether the eips communication is down. If down, set a flag for future use. Such flag will bypass reading cmd input from eth io object in ws_func.c.
 *   - Change the control logic so that valve will be opened automatically after resetting the leak error.
 *   -
 *  @20160809
 *   - re-organize project setup so this file is now shared with ws_profinet.
 *   - macro definition and identification added so it can be shared with both eips and pnio version project.
 *
 *  @20170222
 *   - change the behavior or reset. Reset operation will not lead the auto re-open of the valve.
 *   - flowrate is included now in the output IO data (from ws to remote)
 *   - add new paras for fastest and slowest.
 *   - re-organize the setting mechanism of parameters.
 */


/* Includes ---------------------------------------------------------- */
#include "ws_func.h"
#include "eeprom.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utilities.h"
#include "io_data_struct.h"
#include "dig_led.h"
#include "bsp_eeprom_const.h"

#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS
#include "netconf.h"
#endif

/* Private declaration ----------------------------------------------- */
// decide which io data object to use based the fieldbus type.
#if WS_FIELDBUS_TYPE != FIELDBUS_TYPE_NONE
#define ETH_IO_DATA_OBJ_OUTPUT iodata_out
#define ETH_IO_DATA_OBJ_INPUT iodata_in
#endif

#ifdef USE_LSR_FOR_LEAK_DETECT
#include <math.h>
#define PNT_NUM_TO_FIT (160)
typedef struct FLOWRATE_LQR_T {
    double *data_src;   // the src of flowrate data.
    double data_buf[PNT_NUM_TO_FIT];
    double ramp;
} FLOWRATE_LQR_T;
FLOWRATE_LQR_T lqr_fr1;
FLOWRATE_LQR_T lqr_fr2;
#endif

extern uint8_t flag_fieldbus_down;			// indicate fieldbus (eips/pnio) is lost.

//vars for debug
uint32_t debug_count_status = 0;
uint32_t debug_count_ack = 0;
uint32_t debug_count_status_changed = 0;
uint32_t debug_last_status = 0;

uint32_t debug_sensor_error_count       = 0;
uint32_t debug_flow_fault_error_count   = 0;

/* Exported variables ------------------------------------------------ */
// input - cmd
// the really active ones
uint8_t ws_is_new_ipaddr_configured = 0;
uint8_t ws_i_cmd_reset = 0;
uint8_t ws_i_cmd_valve_on = 0;
uint8_t ws_i_cmd_bypass = 0;
uint8_t ws_i_cmd_brwr_write_access = 0;

// control signal buf of web
uint8_t ws_i_web_reset = 0;
uint8_t ws_i_web_valveon = 0;
uint8_t ws_i_web_bypass = 0;

// control signal buf of fieldbus (eip or pnio)
uint8_t ws_i_bus_reset = 0;
uint8_t ws_i_bus_valveon = 0;
uint8_t ws_i_bus_bypass = 0;

// array[0] -> reset, array[1]->valve, array[2]->bypass
#define WS_CMD_INDEX_RESET (0)
#define WS_CMD_INDEX_VALVEON (1)
#define WS_CMD_INDEX_BYPASS (2)
static DIO_BLOCK_T ctrl_signal_web[3];
static DIO_BLOCK_T ctrl_signal_bus[3];

/**
 * The flag which indicates the ack to
 * be sent after receiving the ctrl param 
 * from the TPU.
 */
uint8_t ws_should_ack = 0;
/**
 * The flag which indicates the valve is turned 
 * off against the error.
 */
uint8_t ws_valve_is_auto_off = 0;
/**
 * The flag which indicates that the setup params
 * on the webpage should be updated.
 */
volatile uint8_t param_should_update = 0;

// input - properties
double ws_i_warning_flow = 11.4;
double ws_i_fault_flow = 7.6;
double ws_i_stablization_delay = 2;
double ws_i_startup_leak = 4;
#if defined USE_WS_ORIGIN_WEBFILES
uint32_t ws_i_cmd_leak_response = 2;				    // 0: fastest, 1:fast, 2:normal, 3:slow, 4:slowest only 1,2,3 are used so far.
#else
uint32_t ws_i_cmd_leak_response = 1;				    // 0: slow, 1: normal, 2: fast. PN still use internal.shtml method so far. if enable official setting method, the value should be 2 as well.
#endif

// internal - default paras out of factory
#ifdef RESERVE_INTERNAL_PARA_SETTING
uint8_t isNewInternalSetting = 0;
#endif /* RESERVE_INTERNAL_PARA_SETTING */
uint8_t isThereNewCtrParas = 0;
double ws_def_warning_flow = 11.4;				    // 11.4 LPM
double ws_def_fault_flow = 7.6;					// 7.6 LPM
double ws_def_stablization_delay = 2;			    // 2 s
double ws_def_startup_leak = 4;				    // 3.8 LPM, on-hand ws only support the value of 2, 4, 6, 8, 10
uint8_t ws_def_cmd_leak_response = 1;			    // 1 for normal
char ws_i_ipaddr_text[20] = {'s'};
char ws_o_ipaddr_text[20] = {'s'};
uint8_t ws_isNewIPAddr = 0;
uint8_t ws_is_ip_change_success = 0;

// internal - parameters/attributes
uint8_t ws_attr_err_flag = 0;						// Set by leak detection, reset by reset cmd from remote
uint32_t ws_valveon_timestamp = 0;
uint8_t ws_flag_after_startup_delay = 0;
double ws_i_startup_leak_in_flowvolme_for_detection = 0;	// in our algorithm, such detection is campare flow volume instead of flowrate.
// define the process loop interval in ms.
#define ws_process_interval WS_PROCESS_RUN_PERIOD
#define qv_buf_size (200)
#define qv_buf_size_a_quarter (qv_buf_size/4)
#define LEAK_INDEX_BUF_SIZE (25)
double qv_flowrate_1 = 0;			                // volume flowrate in l/min
double qv_flowrate_2 = 0;			                // volume flowrate in l/min
double qv_flowrate_buf_1[qv_buf_size] = {0};
double qv_flowrate_buf_2[qv_buf_size] = {0};
double leak_index_buf[LEAK_INDEX_BUF_SIZE] = {0};
double leak_index_average = 0;

// internal - coefficient for flowrate calculation.
static double kf_coff_freq_output = 0.1824;              // for DIN15, kf=0.1843 (l/min)/f
//static double q0_axis_intercept = 0;	            // For DIN15, q0 = -0.2

// internal - threshold for leakage detection
double threshold_deviation_aver = 5.55;			// the deviation of 2 sensors' average
//static double threshold_deviation_rate = 0;			// the changing rate of 2 sensor aver's dev
double threshold_deviation_int = 0.075;			// the integral of 2 sensors' dev
double ws_para_sense_quantized_index = 0.1;			// the coff of short-time flowrate average based leadk volume calculation.
double threshold_leak_detection_quantized_index = 0.07;

// output - feedback
double ws_o_current_flow = 0;
uint8_t ws_o_is_valve_on = 0;
uint8_t ws_o_is_Bypassed = 0;
uint8_t ws_o_is_leak_detected = 0;
uint8_t ws_o_is_flow_warning = 0;
uint8_t ws_o_is_flow_fault = 0;
uint8_t ws_o_is_flow_ok = 0;
uint8_t ws_o_inflow_status_index = 0;				// 2: 0k, 1:warning, 0: fault
uint8_t ws_o_is_oktoweld = 0;
uint8_t ws_o_is_minflow = 0;		                //????
uint8_t ws_o_is_caploss = 0;                        // the difference with leak_detection
uint8_t ws_o_status_index = 0;

uint8_t ws_web_datalog_alive = 0;
uint32_t ws_web_datalog_alive_cnt = 0;

double flow_dev_aver = 0;
double flow_dev_aver_quar = 0;
double flow_dev_int = 0;		// integral of dev in unit of Litre
double flow_aver_1 = 0, flow_aver_2 = 0;
double flow_aver_1_quar = 0, flow_aver_2_quar = 0;		// average calculated in 1/4 of duration.
double leak_detection_quantized_index = 0;
double dif_flowrate_ramp = 0;
double ws_def_dif_flowrate_ramp = 99999999.333;			// it's the threshold actually.


void ws_read_get_sensitivity ();

// private function pre-declaration.


#ifdef USE_LSR_FOR_LEAK_DETECT
/**
 *  @brief Brief
 *  @param [in] obj Parameter_Description
 *  @param [in] src Parameter_Description
 *  @return Return_Description
 *  @details Details
 */
void flowrate_acc_constrcut (FLOWRATE_LQR_T *obj, double *src) {
    obj->data_src = src;
    memset(obj->data_buf, 0, sizeof(obj->data_buf));
    obj->ramp = 0;
}


/*
 * x= a*t + b
 * t is time, and relatively constant.
 * we only care about a here as it indicates the ramp.
 * a = 6/N(N^2-1)*((1-N)*A + 2*B)
 *   = M*(N*A+ 2*B)
*/
void flowrate_lqr_fit(FLOWRATE_LQR_T *obj) {
    double current_fr;  // latest value of flowrate.
    double sum_x;   // sigma sum of Xt
    double sum_xt;  // sigma sum of t*Xt
    double coff_a = 0;
    uint8_t i = 0;
    double temp_M = 0, temp_N = 0;

    i = 0;
    sum_x = 0;
    sum_xt = 0;
    coff_a = 0;
    current_fr = (double)*(obj->data_src);
    // update the buf and calculate sum_x and sum_xt.
    for (i = PNT_NUM_TO_FIT - 1; i>0; i--) {
        obj->data_buf[i] = obj->data_buf[i-1];
        sum_x += obj->data_buf[i];
        sum_xt += obj->data_buf[i] * (PNT_NUM_TO_FIT - 1 - i);
    }
    sum_x += current_fr;
    sum_xt += current_fr * (PNT_NUM_TO_FIT-1);
    obj->data_buf[0] = current_fr;

    temp_M = (double)6 / (PNT_NUM_TO_FIT*PNT_NUM_TO_FIT*PNT_NUM_TO_FIT - PNT_NUM_TO_FIT);
    temp_N = 1 - PNT_NUM_TO_FIT;

    coff_a = temp_M * (temp_N * sum_x + 2 * sum_xt);
    obj->ramp = coff_a;
}
#endif

// Exported functions definition.
void ws_read_flowsensor1 ()
{
#ifndef WS_CAL_FLOWRATE_BY_COUNT_PULSE
	uint16_t ws_flow_sensor_freq = 0;	// sensor pulse output freq in Hz
	double q0_axis_intercept = 0;
    
    // update the value of ws_flow_sensor_freq of sensor 1.
	if (alive_monitor_1.AliveFlag == 1) {
		ws_flow_sensor_freq = fls_1.pulse_freq;
	}
	else {
		ws_flow_sensor_freq = 0;
	}

    // moving average required here?
	qv_flowrate_1 = kf_coff_freq_output * ws_flow_sensor_freq + q0_axis_intercept;
	qv_flowrate_1/=2;

#else /* WS_CAL_FLOWRATE_BY_COUNT_PULSE */
	uint16_t i = 0;
	i = FS_TIME_LEN_TO_COUNT_PULSE - 1;
	fls_1.pulse_count = 0;
	for (i = FS_TIME_LEN_TO_COUNT_PULSE - 1; i > 0;  i--) {
		fls_1.pulse_in_period[i] = fls_1.pulse_in_period[i-1];
		fls_1.pulse_count += fls_1.pulse_in_period[i];
	}
	fls_1.pulse_in_period[0] = fls_1.pulse_num_in_pre_period;
	fls_1.pulse_count += fls_1.pulse_in_period[0];
	fls_1.pulse_num_in_pre_period = 0;

	qv_flowrate_1 = kf_coff_freq_output * fls_1.pulse_count / 2 * 200 / FS_TIME_LEN_TO_COUNT_PULSE;
	qv_flowrate_1 = qv_flowrate_1 * qv_flowrate_1 /(qv_flowrate_1 - 0.2);
#endif /* WS_CAL_FLOWRATE_BY_COUNT_PULSE */
}


void ws_read_flowsensor2 ()
{
#ifndef WS_CAL_FLOWRATE_BY_COUNT_PULSE
	uint16_t ws_flow_sensor_freq = 0;	// sensor pulse output freq in Hz
	double q0_axis_intercept = 0;

    // update the value of ws_flow_sensor_freq of sensor 2.
	if (alive_monitor_2.AliveFlag == 1) {
		ws_flow_sensor_freq = fls_2.pulse_freq;
	}
	else {
		ws_flow_sensor_freq = 0;
	}

  	// moving average required here?
	qv_flowrate_2 = kf_coff_freq_output * ws_flow_sensor_freq + q0_axis_intercept;
	qv_flowrate_2 /=2;

#else /* WS_CAL_FLOWRATE_BY_COUNT_PULSE */
	uint16_t i = 0;
	i = FS_TIME_LEN_TO_COUNT_PULSE - 1;
	fls_2.pulse_count = 0;
//	fls_2.pulse_count += (fls_2.pulse_num_in_pre_period - fls_2.pulse_in_period[FS_TIME_LEN_TO_COUNT_PULSE-1]);		// different way of calculating pulse_count for fls_1 and fls_2.
	for (i = FS_TIME_LEN_TO_COUNT_PULSE - 1; i > 0;  i--) {
		fls_2.pulse_in_period[i] = fls_2.pulse_in_period[i-1];
		fls_2.pulse_count += fls_2.pulse_in_period[i];
	}
	fls_2.pulse_in_period[0] = fls_2.pulse_num_in_pre_period;
	fls_2.pulse_count += fls_2.pulse_in_period[0];
	fls_2.pulse_num_in_pre_period = 0;

	qv_flowrate_2 = kf_coff_freq_output * fls_2.pulse_count / 2 * 200 / FS_TIME_LEN_TO_COUNT_PULSE;
	qv_flowrate_2 = qv_flowrate_2 * qv_flowrate_2 /(qv_flowrate_2 + 0.2);
#endif /* WS_CAL_FLOWRATE_BY_COUNT_PULSE */
}


void ws_read_newctrlparas ()
{
	uint8_t _ip_addr[4] = {0, 0, 0, 0};
	char ip_str[4] = {'0', '0', '0', '\0'};
	uint8_t i = 0;
	char *pStart;
	char *pEnd;
	uint32_t flag_is_board_used = 0;

#ifdef RESERVE_INTERNAL_PARA_SETTING
	if (isNewInternalSetting != 0) {
		isNewInternalSetting = 0;
		EEPROMProgram ((uint32_t*)&ws_para_sense_quantized_index, EEPROM_ADDR_SENSE_QTZ_ID, 8);
		EEPROMProgram ((uint32_t*)&threshold_leak_detection_quantized_index, EEPROM_ADDR_LEAK_DET_QTZ_ID, 8);

#if 0
		// to fix the bug of using a fresh new board if only internal paras are set ever.
		EEPROMProgram ((uint32_t*)&ws_i_warning_flow, EEPROM_ADDR_WARNING_FLOW, 8);
		EEPROMProgram ((uint32_t*)&ws_i_fault_flow, EEPROM_ADDR_FAULT_FLOW, 8);
		EEPROMProgram ((uint32_t*)&ws_i_cmd_leak_response, EEPROM_ADDR_LEAK_RESPONSE, 4);

		EEPROMProgram ((uint32_t*)&ws_i_stablization_delay, EEPROM_ADDR_DELAY, 8);
		EEPROMProgram ((uint32_t*)&ws_i_startup_leak, EEPROM_ADDR_STARTUP_LEAK, 8);

		// update new startup leak detection threshold.
		ws_i_startup_leak_in_flowvolme_for_detection = ws_i_startup_leak/60*ws_i_stablization_delay;

		flag_is_board_used = EEPROM_VAL_IS_BOARD_USED;
		// store data to eeprom to indicate this board has been configured before.
		EEPROMProgram ((uint32_t*)&flag_is_board_used, EEPROM_ADDR_IS_BOARD_USED, 4);
#endif
	}
#endif

	if (isThereNewCtrParas !=0) {
		isThereNewCtrParas = 0;

		EEPROMProgram ((uint32_t*)&ws_i_warning_flow, EEPROM_ADDR_WARNING_FLOW, 8);
		EEPROMProgram ((uint32_t*)&ws_i_fault_flow, EEPROM_ADDR_FAULT_FLOW, 8);
		EEPROMProgram ((uint32_t*)&ws_i_cmd_leak_response, EEPROM_ADDR_LEAK_RESPONSE, 4);

		EEPROMProgram ((uint32_t*)&ws_i_stablization_delay, EEPROM_ADDR_DELAY, 8);
		EEPROMProgram ((uint32_t*)&ws_i_startup_leak, EEPROM_ADDR_STARTUP_LEAK, 8);

#ifdef DISABLE_INTERNAL_SENSE_CONFIG
		// in such implementation, only leak_resp will be updated. no value of ws_para_sense_quantized_index
		// and threshold_leak_detection_quantized_index are stored.
		// so ws_read_get_sensitivity() must be called find new sense/index value from new leak_response, but before storing sense/index to eeprom.
		// so after "submit", then re-power, ws will still use the directly stored value of the last one.
	ws_read_get_sensitivity ();
#endif /* DISABLE_INTERNAL_SENSE_CONFIG */

		EEPROMProgram ((uint32_t*)&ws_para_sense_quantized_index, EEPROM_ADDR_SENSE_QTZ_ID, 8);
		EEPROMProgram ((uint32_t*)&threshold_leak_detection_quantized_index, EEPROM_ADDR_LEAK_DET_QTZ_ID, 8);

		// update new startup leak detection threshold.
		ws_i_startup_leak_in_flowvolme_for_detection = ws_i_startup_leak/60*ws_i_stablization_delay;

		flag_is_board_used = EEPROM_VAL_IS_BOARD_USED;
		// store data to eeprom to indicate this board has been configured before.
		EEPROMProgram ((uint32_t*)&flag_is_board_used, EEPROM_ADDR_IS_BOARD_USED, 4);
	}

	if (ws_isNewIPAddr != 0) {
		ws_isNewIPAddr = 0;
#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS
		ws_is_new_ipaddr_configured = EEPROM_VAL_NEW_IP_SAVED;

		uint32_t temp_ip_byte = 0;

		pStart = ws_i_ipaddr_text;
		for (i = 0; i < 3; i++) {
			pEnd = strchr(pStart + 1, '.');
			if (pEnd) {
				*pEnd = '\0';
				memcpy (ip_str, pStart, pEnd-pStart+1);
				temp_ip_byte = strtol(ip_str,0,0);
				//_ip_addr[i] = (uint8_t)atoi (ip_str);
				if (temp_ip_byte >= 255 || temp_ip_byte <= 0) {
					return;
				} else {
					_ip_addr[i] = (uint8_t)temp_ip_byte;
				}
			}
			else {
				// reach the end of str
				return;
			}
			pStart = pEnd+1;
		}

		memcpy (ip_str, pStart, 4);	//there would be null char to indicate the end of num.
		temp_ip_byte = strtol(ip_str,0,0);
		if(temp_ip_byte > 0 && temp_ip_byte < 255)
			_ip_addr[i] = (uint8_t)temp_ip_byte;
		else
			return;

		if(((uint32_t*)_ip_addr)[0] != ip_addr.raw) {
			// store the new IP addr into EEPROM.
			EEPROMProgram ((uint32_t*)&_ip_addr[0], EEPROM_ADDR_IP_ADDR0, 4);

			EEPROMProgram ((uint32_t*)&ws_is_new_ipaddr_configured, EEPROM_ADDR_NEW_IP_IS_SAVED, 4);

			SysCtlReset();
		}
#endif
	}
	else {
		return;
	}
}

#ifdef DISABLE_INTERNAL_SENSE_CONFIG
void ws_read_get_sensitivity ()
{
	/*
	 * in current implementation, only fast, normal, slow is used and implemented.
	 * 0 - fastest
	 * 1 - fast
	 * 2 - normal
	 * 3 - slow
	 * 4 - slowest
	 * */
	switch (ws_i_cmd_leak_response) {
		case 0:
			ws_para_sense_quantized_index = 0.1;
			threshold_leak_detection_quantized_index = 0.05;
			break;
		case 1:
			ws_para_sense_quantized_index = 0.1;
			threshold_leak_detection_quantized_index = 0.06;
			break;
		case 2:
			ws_para_sense_quantized_index = 0.1;
			threshold_leak_detection_quantized_index = 0.07;
			break;
		case 3:
			ws_para_sense_quantized_index = 0.07;
			threshold_leak_detection_quantized_index = 0.08;
			break;
		case 4:
			ws_para_sense_quantized_index = 0.07;
			threshold_leak_detection_quantized_index = 0.09;
			break;
		default:
			ws_para_sense_quantized_index = 0.1;
			threshold_leak_detection_quantized_index = 0.07;
			break;
	}
	// we dont need write to eeprom to record as we always get these 2 values by calculation based on lead_response.
//	EEPROMProgram ((uint32_t*)&ws_para_sense_quantized_index, EEPROMAddrFromBlock(1)+36, 8);
//	EEPROMProgram ((uint32_t*)&threshold_leak_detection_quantized_index, EEPROMAddrFromBlock(1)+44, 8);
}
#endif /* DISABLE_INTERNAL_SENSE_CONFIG */

void ws_handle_error_state()
{
	// once error flag is set, only clear it by reset cmd.
	// in the meanwhile, reset the reset cmd.
	// this will not work with EIP!!.
	if (ws_i_cmd_reset == 1 && ws_attr_err_flag==1) {
		ws_attr_err_flag = 0;
		ws_i_cmd_reset = 0;

		// reset only take effect in case of an error.
		// with a reset operation, rest valve cmd as well.
		// updated in 20170222, comment the code below to address new requirement: Reset should not re-open the valve.

		// updated in 20170927, automatically turn on the valve if the valve is previously turned off against
		// caploss
		if(ws_valve_is_auto_off)
		{
		    ws_i_cmd_valve_on = 1;
		    ws_valve_is_auto_off = 0;
		}

		//added by TMS 0n 21092017
	}

	// to sync web cmd of reset.
	ws_i_web_reset = ws_i_cmd_reset;
}



/**
 *  @brief Read input from either webserver or fieldbus IO. (not including DIDO yet.)
 *  @return None
 *  @details Only signal from bus is inverted if necessary.
 *  		valve is still open by rising edge. closed by falling edge.
 *  		we dont need to invert the valve control signal from web
 *
 */
void ws_read_eth_input()
{
	// the required ctl paras include
    //  - turn on bypass
    //  - turnon valve
	//  - reset error
#if 1
	dio_block_process(&ctrl_signal_web[WS_CMD_INDEX_RESET]);
	dio_block_process(&ctrl_signal_web[WS_CMD_INDEX_VALVEON]);
	dio_block_process(&ctrl_signal_web[WS_CMD_INDEX_BYPASS]);

	/*
	 * in di interrupt, ws_i_bus_reset, ws_i_bus_valveon, ws_i_bus_bypass is valued to reuse the cmd input channel with filedbus.
	 * assumed that dido will never be down - that means no dido connection leads to all-zero cmd input.
	 * */
#ifndef WS_FILEDBUS_NONE_BUT_DIDO
	if (flag_fieldbus_down) {
		// fieldbus communication is down.
		if (dio_detect_edge(&ctrl_signal_web[WS_CMD_INDEX_RESET])) {
		ws_i_cmd_reset = 2 - dio_detect_edge(&ctrl_signal_web[WS_CMD_INDEX_RESET]);
		}
		if (dio_detect_edge(&ctrl_signal_web[WS_CMD_INDEX_VALVEON])) {
		ws_i_cmd_valve_on = 2 - dio_detect_edge(&ctrl_signal_web[WS_CMD_INDEX_VALVEON]);

		}
		if (dio_detect_edge(&ctrl_signal_web[WS_CMD_INDEX_BYPASS])) {
			ws_i_cmd_bypass = 2 - dio_detect_edge(&ctrl_signal_web[WS_CMD_INDEX_BYPASS]);
		}
		return;
	}
#endif /* WS_FILEDBUS_NONE_BUT_DIDO */

#ifdef WS_FILEDBUS_NONE_BUT_DIDO

#else /* WS_FILEDBUS_NONE_BUT_DIDO */

	// update the src signal from bus interface.
	ws_i_bus_reset = ETH_IO_DATA_OBJ_INPUT.data.cmd_reset;
#ifndef WS_VALVE_CTRL_INVERTED
	ws_i_bus_valveon = ETH_IO_DATA_OBJ_INPUT.data.cmd_valve_ctr;
#else /* WS_VALVE_CTRL_INVERTED */
	ws_i_bus_valveon = ETH_IO_DATA_OBJ_INPUT.data.cmd_valve_ctr ? 0 : 1;
#endif /* WS_VALVE_CTRL_INVERTED */
	ws_i_bus_bypass = ETH_IO_DATA_OBJ_INPUT.data.cmd_bypass;
#endif /* WS_FILEDBUS_NONE_BUT_DIDO */

	// web channel is updated in httpd_cgi.c

	// process the dido's edge.

	dio_block_process(&ctrl_signal_bus[WS_CMD_INDEX_RESET]);
	dio_block_process(&ctrl_signal_bus[WS_CMD_INDEX_VALVEON]);
	dio_block_process(&ctrl_signal_bus[WS_CMD_INDEX_BYPASS]);

	// determine the reset cmd.
	// no priority/safety limit here. both channels should work in any case. rising-edge active
	// if no edge detected, set ws_i_cmd_reset to 0.
	// for valve/bypass control, if no edge, should maintain its current value of ws_i_cmd
	// for reset, just make sure only rising edge will trigger reset action.
	// actually reset only have effect when error_flag is set. so double protection here to prevent repeated trigger of reset action.
	if (dio_detect_edge(&ctrl_signal_bus[WS_CMD_INDEX_RESET])) {
		ws_i_cmd_reset = 2 - dio_detect_edge(&ctrl_signal_bus[WS_CMD_INDEX_RESET]);
	}
	else if  (dio_detect_edge(&ctrl_signal_web[WS_CMD_INDEX_RESET])) {
		ws_i_cmd_reset = 2 - dio_detect_edge(&ctrl_signal_web[WS_CMD_INDEX_RESET]);
	}
	else {
		ws_i_cmd_reset = 0;
	}

	// determine valve control cmd.
	// safety limit 1 - web cannot open valve if there presents close signal from fieldbus.
	if (dio_detect_edge(&ctrl_signal_web[WS_CMD_INDEX_VALVEON]) || dio_detect_edge(&ctrl_signal_bus[WS_CMD_INDEX_VALVEON])) {
		if (dio_detect_edge(&ctrl_signal_bus[WS_CMD_INDEX_VALVEON])) {
			ws_i_cmd_valve_on = 2 - dio_detect_edge(&ctrl_signal_bus[WS_CMD_INDEX_VALVEON]);

		}

		if (dio_detect_edge(&ctrl_signal_web[WS_CMD_INDEX_VALVEON])) {
			if (ws_i_bus_valveon == 0) return;
			ws_i_cmd_valve_on = 2 - dio_detect_edge(&ctrl_signal_web[WS_CMD_INDEX_VALVEON]);

		}
	}

	// determine bypass control cmd.
	// safety limit 2 - web cannot enable bypass if water is off.
	// safety limit 1 - web cannot disable bypass if there presents enable signal from fieldbus.
	if (dio_detect_edge(&ctrl_signal_web[WS_CMD_INDEX_BYPASS])  == 1) {
		ws_i_cmd_bypass = 2 - dio_detect_edge(&ctrl_signal_web[WS_CMD_INDEX_BYPASS]);
	}
	if (dio_detect_edge(&ctrl_signal_web[WS_CMD_INDEX_BYPASS])  == 2) {
		if (ws_i_bus_bypass != 1) {
			ws_i_cmd_bypass = 0;
		}
	}
	if (dio_detect_edge(&ctrl_signal_bus[WS_CMD_INDEX_BYPASS])) {
		ws_i_cmd_bypass = 2 - dio_detect_edge(&ctrl_signal_bus[WS_CMD_INDEX_BYPASS]);
	}

#else
	if (flag_fieldbus_down) {
		// fieldbus communication is down.

		ws_i_cmd_valve_on = ws_i_web_valveon;
		return;
	}

	// update the src signal from bus interface.
	ws_i_bus_reset = ETH_IO_DATA_OBJ_INPUT.data.cmd_reset;
#ifndef WS_VALVE_CTRL_INVERTED
	ws_i_bus_valveon = ETH_IO_DATA_OBJ_INPUT.data.cmd_valve_ctr;
#else /* WS_VALVE_CTRL_INVERTED */
	ws_i_bus_valveon = ETH_IO_DATA_OBJ_INPUT.data.cmd_valve_ctr ? 0 : 1;
#endif /* WS_VALVE_CTRL_INVERTED */

	ws_i_bus_bypass = ETH_IO_DATA_OBJ_INPUT.data.cmd_bypass;
#endif

	//
	//  Below is added by TMS to process the setup data from TPU.
	//  The development in the TPU side is completed by WangQi.
	//
	//  The control data includes: Flow Warning, Flow Fault, Leak
	//  Response, Stablization Delay and Startup Leak.
	//  We retrive the data from TPU only when the cmd_tpu_ctrl_update
	//  is set to '1'.
	//

	if(ETH_IO_DATA_OBJ_INPUT.data.cmd_tpu_ctrl_update)
	{
	    uint8_t temp_warning = ETH_IO_DATA_OBJ_INPUT.data.cmd_flow_warning;
	    uint8_t temp_fault = ETH_IO_DATA_OBJ_INPUT.data.cmd_flow_fault;

	    if(temp_warning > temp_fault)
	    {
	        param_should_update = 1;
	        isThereNewCtrParas = 1;
            ws_i_warning_flow       = (double)temp_warning / 10.0;
            ws_i_fault_flow         = (double)temp_fault / 10.0;
            ws_i_cmd_leak_response  = ETH_IO_DATA_OBJ_INPUT.data.cmd_leak_response;
            ws_i_stablization_delay = ETH_IO_DATA_OBJ_INPUT.data.cmd_delay;
            ws_i_startup_leak       = ETH_IO_DATA_OBJ_INPUT.data.cmd_startup_leak;

            ws_should_ack = 1;
            debug_count_status++;
            if(debug_last_status != ws_should_ack)
                debug_count_status_changed++;
	    }
	}
	debug_last_status = ws_should_ack;
}


void ws_update_eth_output ()
{
#ifndef WS_FILEDBUS_NONE_BUT_DIDO
	//ETH_IO_DATA_OBJ_OUTPUT.data.nFlowrate= (uint8_t)(qv_flowrate_1 *10);       // feedback flowrate in 10*l/min.
#ifndef WS_VALVE_CTRL_INVERTED
	ETH_IO_DATA_OBJ_OUTPUT.data.isValveOn = ws_o_is_valve_on;
#else
	ETH_IO_DATA_OBJ_OUTPUT.data.isValveOff = !ws_o_is_valve_on;
#endif
	ETH_IO_DATA_OBJ_OUTPUT.data.isBypassed = ws_o_is_Bypassed;
	ETH_IO_DATA_OBJ_OUTPUT.data.diWS_CapLoss = ws_o_is_leak_detected;
	ETH_IO_DATA_OBJ_OUTPUT.data.diWS_OkToWeld = ws_o_is_oktoweld;
	ETH_IO_DATA_OBJ_OUTPUT.data.diWS_MinFlow = ws_o_is_minflow;
	ETH_IO_DATA_OBJ_OUTPUT.data.diWS_PowerOk = 1;

	// update the ack when ws_should_ack is 1
	// reset it to 0 afterwards
	// if ws_should_ack is 0, keep diWS_ACK and ws_should_ack being 0

	//write the local control parameters to the test rack
	ETH_IO_DATA_OBJ_OUTPUT.data.flow_warning = (uint8_t)(ws_i_warning_flow * 10.0);
	ETH_IO_DATA_OBJ_OUTPUT.data.flow_fault = (uint8_t)(ws_i_fault_flow * 10.0);
	ETH_IO_DATA_OBJ_OUTPUT.data.leak_response = (uint8_t)ws_i_cmd_leak_response;
	ETH_IO_DATA_OBJ_OUTPUT.data.delay = (uint8_t)ws_i_stablization_delay;
	ETH_IO_DATA_OBJ_OUTPUT.data.startup_leak = (uint8_t)ws_i_startup_leak;

	// optional, not standard configuration.
	ETH_IO_DATA_OBJ_OUTPUT.data.nFlowrate = (uint8_t)(flow_aver_2*10.0);		//flowrate in 0.1*l/min

	ETH_IO_DATA_OBJ_OUTPUT.data.diWS_ACK = ws_should_ack;

	ETH_IO_DATA_OBJ_OUTPUT.data.sys_error = ws_attr_err_flag;
	if(ws_should_ack)
	    debug_count_ack++;
	ws_should_ack = 0;
#endif /* WS_FILEDBUS_NONE_BUT_DIDO */
}

// currently http input and output are updated directly in http SSI and CGI routines.
// this approach could be changed in future for more flexibility such as allowing other configuration source through Telnet.
void ws_bypass_control ()
{
	if (ws_o_is_valve_on == 0) {
		ws_o_is_Bypassed = 0;
		ws_i_web_bypass = 0;
		return;
	}
	ws_o_is_Bypassed = (ws_i_cmd_bypass == 0)?0:1;
	ws_i_web_bypass = ws_o_is_Bypassed;
}


void ws_valve_control ()
{
	uint8_t local_valvectrl = 0;
	local_valvectrl = ws_i_cmd_valve_on;

	// TBD: shall AND process result here to finally decide whether to open/close valve.
	if (ws_attr_err_flag == 0) {
		// only response to valve control when ws is not in err state
		if (local_valvectrl != 0) {
			if (ws_o_is_valve_on == 0) {
				// only set when previously valve is off.
				ws_valveon_timestamp = Time_GetMs();
				ws_flag_after_startup_delay = 0;
			}
			GPIO_TagWrite(GPIOTag_VALVE_CMD, 1);
			ws_o_is_valve_on = 1;
		}
		else {
			GPIO_TagWrite(GPIOTag_VALVE_CMD, 0);
			ws_o_is_valve_on = 0;
		}
		ws_valve_is_auto_off = 0;
	}
	else {
		//if in err state, keep the valve off, regardless of the value of ws_i_cmd_valve_on
		GPIO_TagWrite(GPIOTag_VALVE_CMD, 0);
		// we dont reset ws_i_cmd_valve_on here.
		// so the valve will be opened again automatically after error flag is reset.
		//ws_i_cmd_valve_on = 0;
		// but we still report the right valve status back to host controller.
		ws_o_is_valve_on = 0;
		ws_valve_is_auto_off = 1;
	}

	//if (ws_o_is_valve_on) ws_i_web_valveon = 1;
	// set the wed cmd here so web can trigger a right edge to dido_block.
	ws_i_web_valveon = ws_o_is_valve_on;
}

// determine whether in startup period.
void ws_startup_detecion()
{
	if (ws_flag_after_startup_delay == 0 && ws_o_is_valve_on==1) {
		if ( (Time_GetMs() - ws_valveon_timestamp) > (ws_i_stablization_delay*1000) ) {
			ws_flag_after_startup_delay = 1;
		}
	}
}

// detect the flow inlet
// not sure about this point. does it make sense to have only sensor involved to detect supply flow?
// besides, does this detection require smooth flowrate after filtering.
void ws_flowrate_detect_flowin ()
{
    double flowrate_to_detect = 0;

    // we use flow_aver_1 here.
    flowrate_to_detect = flow_aver_1;
    
    // firstly identify the startup and make some detection accordingly.
    

	// detect the input flowrate status
	// and update flow status indication output accordingly.
    // this detection has only sensor 1 value involved.
	// bypass will NOT disable this detection!
    // ***
    // edited by TMS on 21-09-2017, add the caploss arbitration
    if(ws_o_is_leak_detected && !ws_o_is_Bypassed) {
        //ws_o_is_oktoweld = 0;
        ws_o_is_minflow = 0;

        ws_o_is_flow_ok = 0;
        ws_o_is_flow_warning = 0;
        ws_o_is_flow_fault = 1;
        ws_o_inflow_status_index = 0;       //TODO: decide if we should add the status
    }
    else if (flowrate_to_detect > ws_i_warning_flow) {
	//if (qv_flowrate_1 > ws_i_warning_flow || ws_i_cmd_bypass == 1) {
		//if bypassed, or flow is ok, set of status monitor to OK.
		//ws_o_is_oktoweld = 1;
		ws_o_is_minflow = 1;

		ws_o_is_flow_ok = 1;
		ws_o_is_flow_warning = 0;
		ws_o_is_flow_fault = 0;
		ws_o_inflow_status_index = 2;
	}
	else if (flowrate_to_detect > ws_i_fault_flow) {
		//ws_o_is_oktoweld = 1;
		ws_o_is_minflow = 0;

		ws_o_is_flow_ok = 0;
		ws_o_is_flow_warning = 1;
		ws_o_is_flow_fault = 0;
		ws_o_inflow_status_index = 1;
	}
	else {
		//ws_o_is_oktoweld = 0;
		ws_o_is_minflow = 0;

		ws_o_is_flow_ok = 0;
		ws_o_is_flow_warning = 0;
		ws_o_is_flow_fault = 1;
		ws_o_inflow_status_index = 0;
	}

    //added by TMS on 27/09/2017
    //  flow ok should be only if:
    //  1.  flow rate is larger than the warning flow rate,
    //      which means ws_o_is_minflow is set to 1
    //  2.  out of stabilization delay period

    if( ws_flag_after_startup_delay == 1 )
        if( ws_o_is_flow_fault == 0 )
        {
            ws_o_is_oktoweld = 1;
        }
        else
        {
            ws_o_is_oktoweld = 0;
            //disabled by TMS on 07/10/2017
#ifdef TURN_OFF_VALVE_IF_FLOW_FAULT_AFTER_DELAY
            //
            //  edited by TMS
            //  error if the flow rate is lower than faulty flow rate
            //  after the valve is on for a certain period.
            //
            if(ws_o_is_valve_on && (!ws_o_is_leak_detected || ws_o_is_Bypassed))
            {
                ws_attr_err_flag = 1;
                debug_flow_fault_error_count++;
            }
#endif
        }
}

#ifdef USE_FLOWRATE_BENCHMARK
double leak_index_with_est = 0;
#define BENCHMARK_RATEING_COFF (0.16)
#define BENCHMARK_UPDATE_TRIGGER (20)
double flowrate_benchmark_1 = 0, fr_bm_candidate_1 = 0, pre_fr_bm_1 = 0;
double flowrate_benchmark_2 = 0, fr_bm_candidate_2 = 0, pre_fr_bm_2 = 0;
int8_t flag_updated_benchmark_1 = 0, trigger_bm_update_1 = 0;
int8_t flag_updated_benchmark_2 = 0, trigger_bm_update_2 = 0;

void ws_update_flow_benchmark () {
	if ((flow_aver_1 - flowrate_benchmark_1) > flowrate_benchmark_1 * BENCHMARK_RATEING_COFF || (flow_aver_1 - flowrate_benchmark_1) < (0 - flowrate_benchmark_1 * BENCHMARK_RATEING_COFF)) {
		trigger_bm_update_1++;

		if (trigger_bm_update_1 == 1) {
			fr_bm_candidate_1 = flow_aver_1;
		}
	}
	else {
		trigger_bm_update_1 = 0;
	}

	if (trigger_bm_update_1 > BENCHMARK_UPDATE_TRIGGER) {
		if (flow_aver_1 > flowrate_benchmark_1) {
			flag_updated_benchmark_1 = 1;
		}
		else {
			flag_updated_benchmark_1 = -1;
		}
		pre_fr_bm_1 = flowrate_benchmark_1;
		flowrate_benchmark_1 = fr_bm_candidate_1;
	}
	else {
		flag_updated_benchmark_1 = 0;
	}

	if ((flow_aver_2 - flowrate_benchmark_2) > flowrate_benchmark_2 * BENCHMARK_RATEING_COFF || (flow_aver_2 - flowrate_benchmark_2) < (0 - flowrate_benchmark_2 * BENCHMARK_RATEING_COFF)) {
		trigger_bm_update_2++;

		if (trigger_bm_update_2 == 1) {
			fr_bm_candidate_2 = flow_aver_2;
		}
	}
	else {
		trigger_bm_update_2 = 0;
	}

	if (trigger_bm_update_2 > BENCHMARK_UPDATE_TRIGGER) {
		if (flow_aver_2 > flowrate_benchmark_2) {
			flag_updated_benchmark_2 = 1;
		}
		else {
			flag_updated_benchmark_2 = -1;
		}
		pre_fr_bm_2 = flowrate_benchmark_2;
		flowrate_benchmark_2 = fr_bm_candidate_2;
	}
	else {
		flag_updated_benchmark_2 = 0;
	}
}

double weighted_leak_index = 0;
double weighted_rampgap_threshold = 0;
void ws_update_weighted_index_threshold () {
	// we thought here floware of 12lpm should have threshold of 0.045.
	weighted_leak_index = threshold_leak_detection_quantized_index - (12 - (flow_aver_1 + flow_aver_2)/2)*0.004;
	//	weighted_leak_index = 0.002143*flowrate_benchmark_1 + 0.019286;
	//	weighted_leak_index = 0.002143*(flowrate_benchmark_1 + flowrate_benchmark_2)/2 + 0.019286;
	//	weighted_leak_index = 0.002143*(flow_aver_1 + flow_aver_2)/2 + 0.019286;
	//	weighted_leak_index = flowrate_benchmark_1 / 12 * threshold_leak_detection_quantized_index;
	//	weighted_leak_index = flowrate_benchmark_2 / 12 * threshold_leak_detection_quantized_index;
	if (weighted_leak_index < 0.005) weighted_leak_index = 0.005;
	if (weighted_leak_index > 0.2) weighted_leak_index = 0.2;
	weighted_rampgap_threshold = flowrate_benchmark_1 / 12 * ws_def_dif_flowrate_ramp;
}
#endif

// calculate and check whether there is a leakage
// when bypassed, calculation will be still done.
void ws_flowrate_detect_leakage ()
{
    uint16_t i = qv_buf_size - 1;

    // for effiency, calculate moving average in advance of buffer update.
#if 0
    // what's ther fucking worong here!!
    // keep it as it's actually tested in many cases...
	flow_aver_1 = (qv_flowrate_buf_1[0] - qv_flowrate_buf_1[qv_buf_size-1]) / qv_buf_size + flow_aver_1;
	flow_aver_2 = (qv_flowrate_buf_2[0] - qv_flowrate_buf_2[qv_buf_size-1]) / qv_buf_size + flow_aver_2;
	flow_aver_1_quar = (qv_flowrate_buf_1[0] - qv_flowrate_buf_1[qv_buf_size-1]) / qv_buf_size_a_quarter + flow_aver_1;
	flow_aver_2_quar = (qv_flowrate_buf_2[0] - qv_flowrate_buf_2[qv_buf_size-1]) / qv_buf_size_a_quarter + flow_aver_2;
#else
	// right one, but could be of stability issues.
	flow_aver_1 = (qv_flowrate_1 - qv_flowrate_buf_1[qv_buf_size-1]) / qv_buf_size + flow_aver_1;
	flow_aver_2 = (qv_flowrate_2 - qv_flowrate_buf_2[qv_buf_size-1]) / qv_buf_size + flow_aver_2;
	flow_aver_1_quar = (qv_flowrate_1 - qv_flowrate_buf_1[qv_buf_size_a_quarter-1]) / qv_buf_size_a_quarter + flow_aver_1_quar;
	flow_aver_2_quar = (qv_flowrate_2 - qv_flowrate_buf_2[qv_buf_size_a_quarter-1]) / qv_buf_size_a_quarter + flow_aver_2_quar;
#endif
	flow_dev_aver = flow_aver_1 - flow_aver_2;
	flow_dev_aver_quar = flow_aver_1_quar - flow_aver_2_quar;

	// calculate the intergal of dev of buffered samples.
	// assume that flowrate was sampled every 5ms.
	// flow_dev_int is in unit of Litre.
	flow_dev_int = flow_dev_int  + ( (qv_flowrate_1 - qv_flowrate_2) - (qv_flowrate_buf_1[qv_buf_size-1] - qv_flowrate_buf_2[qv_buf_size-1]) ) *ws_process_interval /60000;

	// update the data buffer.
	i = qv_buf_size - 1;
    for (i = qv_buf_size - 1; i > 0; i--) {

        qv_flowrate_buf_1[i] = qv_flowrate_buf_1[i - 1];
        qv_flowrate_buf_2[i] = qv_flowrate_buf_2[i - 1];
    }
    qv_flowrate_buf_1[0] = qv_flowrate_1;
    qv_flowrate_buf_2[0] = qv_flowrate_2;
    

	// calculate and check whether leak condition is met.
	// even in case of bypass or within startup delay, all
    // calculation will still be completed, just without comparision.
	// in case of software bypass, still record/report the leak detection
    // result, but without setting err flag.
    leak_detection_quantized_index = flow_dev_aver_quar * ws_process_interval * qv_buf_size / 60000 * ws_para_sense_quantized_index + flow_dev_int * (1 - ws_para_sense_quantized_index);

    // calculate the moving average of leak index and update the data buffer.
    leak_index_average = (leak_detection_quantized_index - leak_index_buf[LEAK_INDEX_BUF_SIZE - 1]) / LEAK_INDEX_BUF_SIZE + leak_index_average;
    i = LEAK_INDEX_BUF_SIZE - 1;
    for (i = LEAK_INDEX_BUF_SIZE - 1; i > 0; i--) {
    	leak_index_buf[i] = leak_index_buf[i - 1];
    }
    leak_index_buf[0] = leak_detection_quantized_index;

#ifdef USE_LSR_FOR_LEAK_DETECT
    // use LSR to calculate the ramp of 2 flowrates.
#if 0
	// only for the test of LQR calculation.
    flow_aver_1 = (double)sin((double)Time_GetMs()/1000) * 200;
    flow_aver_2 = (double)cos((double)Time_GetMs()/2000) * 200;
#endif
    flowrate_lqr_fit(&lqr_fr1);
    flowrate_lqr_fit(&lqr_fr2);
    dif_flowrate_ramp = lqr_fr1.ramp - lqr_fr2.ramp;
#endif

#ifdef USE_LSR_FOR_LEAK_DETECT
    double est_flow_dev = 0;

    if ((lqr_fr1.ramp > ((weighted_rampgap_threshold-0.02) / 2) && lqr_fr2.ramp < (0-(weighted_rampgap_threshold-0.02) /2) && flow_aver_2 / pre_fr_bm_2 <= 0.6)
    		|| ((flow_aver_2 / pre_fr_bm_2 <= 0.065) && (flow_aver_1 / pre_fr_bm_1 >= 1.35) )) {
    	// estimated flow gap in future is 0.5*rampgap*T^2.
    	// T is 0.5s here.
    	est_flow_dev = 0.5 * dif_flowrate_ramp * 0.5 *0.5 + (flow_aver_1 - flow_aver_2) / 60 * 0.5;

    }
    else {
    	est_flow_dev = 0;
    }
    leak_index_with_est = leak_index_average + est_flow_dev;
#endif

    // even cal is done, detecion will only be active after startup_delay.
    // take care all calculation must still be done before this check as it could return directly.
    // for leak status check, only comparison could behind this check.
    if (ws_flag_after_startup_delay == 0)
    	return;

    // even the calculation is done, we dont update leak detection result if last error is not cleared.
    // note that cal is still done before this return.
    if (ws_attr_err_flag == 1) {
    	ws_o_is_oktoweld = 0;		// set to 0 as it could be set to 1 again by ws_flowrate_detect_flowin().
    	return;
    }

    // for index,2 conditions should be met together to determine the presence of leak.
    //   a - leak_index_average is bigger than the threshold.
    //   b - current leak index should also be bigger than the threshold.
    // for ramp, it seems that 0.06 could be a proper threshold.
    //if ((leak_index_average > threshold_leak_detection_quantized_index && leak_detection_quantized_index > threshold_leak_detection_quantized_index)
#ifdef USE_LSR_FOR_LEAK_DETECT

//    if ((leak_index_average > threshold_leak_detection_quantized_index && leak_detection_quantized_index > threshold_leak_detection_quantized_index && dif_flowrate_ramp > ws_def_dif_flowrate_ramp)
//    		|| (leak_index_average > 0.05 && leak_detection_quantized_index > 0.05)

    if ( leak_index_with_est > weighted_leak_index)
#else
	if ((leak_index_average > threshold_leak_detection_quantized_index && leak_detection_quantized_index > threshold_leak_detection_quantized_index)
#endif
     {
    	ws_o_is_leak_detected = 1;

    	if (ws_o_is_Bypassed == 0) {
    	    //printf("leak detected!\n");
    		ws_attr_err_flag = 1;		// set error flag here. Once it's set, it will be reset only by reset cmd.
    		// set ws_o_is_oktoweld to 0 if leak is detected.
    		ws_o_is_oktoweld = 0;
    	}
    }
    else {
    	ws_o_is_leak_detected = 0;
    }
}

// exported functions
void ws_init ()
{
	//define a temp var
	uint32_t flag_is_board_used = 0;
	//uint32_t local_temp;

	// flow sensor is inited in flowsensor.c
#ifdef USE_LSR_FOR_LEAK_DETECT
	flowrate_acc_constrcut(&lqr_fr1, &flow_aver_1);
	flowrate_acc_constrcut(&lqr_fr2, &flow_aver_2);
#endif

#if defined WS_FILEDBUS_NONE_BUT_DIDO
	flag_fieldbus_down = 0;
#endif

	// init all buffer.
#if 0
	// further test required here as array may overflow
	// what is the behavior of memset for array - use sizeof,
	memset(qv_flowrate_buf_1, 0, sizeof(qv_flowrate_buf_1));
	memset(qv_flowrate_buf_2, 0, sizeof(qv_flowrate_buf_2));
	memset(leak_index_buf, 0, sizeof(leak_index_buf));
#endif

	// init io block
	memset(ctrl_signal_web, 0, sizeof(ctrl_signal_web));
	memset(ctrl_signal_bus, 0, sizeof(ctrl_signal_bus));
	ctrl_signal_web[WS_CMD_INDEX_RESET].src = &ws_i_web_reset;
	ctrl_signal_web[WS_CMD_INDEX_RESET].current_state = 0;
	ctrl_signal_web[WS_CMD_INDEX_VALVEON].src = &ws_i_web_valveon;
	ctrl_signal_web[WS_CMD_INDEX_BYPASS].src = &ws_i_web_bypass;

	ctrl_signal_bus[WS_CMD_INDEX_RESET].src = &ws_i_bus_reset;
	ctrl_signal_bus[WS_CMD_INDEX_VALVEON].src = &ws_i_bus_valveon;
	ctrl_signal_bus[WS_CMD_INDEX_BYPASS].src = &ws_i_bus_bypass;

	// init valve control port
	GPIO_TagConfigProperties(GPIOTag_VALVE_CMD, GPIO_SET_OUT_PUSHPULL, GPIO_SPD_MID);

	// check whether this is a fresh new board.
//	EEPROMRead ((uint32_t*)&local_temp, EEPROMAddrFromBlock(1), 8);
//	if (local_temp != 0x004E4557) {
//		// this is a fresh new board, return.
//		// simple return here requires the all ws var should be inited with the default value.
//		return;
//	}
    
	EEPROMRead ((uint32_t*)&flag_is_board_used, EEPROM_ADDR_IS_BOARD_USED, 4);

	if(flag_is_board_used == EEPROM_VAL_IS_BOARD_USED) {
		// init ws related variables.
		//read values from eeprom
		EEPROMRead ((uint32_t*)&ws_i_warning_flow, EEPROM_ADDR_WARNING_FLOW, 8);
		EEPROMRead ((uint32_t*)&ws_i_fault_flow, EEPROM_ADDR_FAULT_FLOW, 8);
		EEPROMRead ((uint32_t*)&ws_i_cmd_leak_response, EEPROM_ADDR_LEAK_RESPONSE, 4);

		EEPROMRead ((uint32_t*)&ws_i_stablization_delay, EEPROM_ADDR_DELAY, 8);
		EEPROMRead ((uint32_t*)&ws_i_startup_leak, EEPROM_ADDR_STARTUP_LEAK, 8);

		EEPROMRead ((uint32_t*)&ws_para_sense_quantized_index, EEPROM_ADDR_SENSE_QTZ_ID, 8);
		EEPROMRead ((uint32_t*)&threshold_leak_detection_quantized_index, EEPROM_ADDR_LEAK_DET_QTZ_ID, 8);
	}

	// start up detection of cap loss is done by checking average of (flowin-flowout)
	// a smaller ws_i_startup_leak means s sensitive detection. (here the value's meansing is actually the flowrate of leak flow through cap)
	// the most non sensitive detection means all deviation of flowin-flowout are caused by delay.
	ws_i_startup_leak_in_flowvolme_for_detection = ws_i_startup_leak/60*ws_i_stablization_delay;
}

void ws_status_update ()
{
	//
	//Debug by TMS:
	//	1. "caploss" triggers "valve off". However, the webpage
	//		should show "CAP OFF" instead of "FLOW OFF".
	//	2. decide the status according to ws_o_is_XXXX variables
	//
	//bypassed?
	if (ws_o_is_flow_ok && ws_o_is_Bypassed) {
			ws_o_status_index = ws_valve_on_and_bypassed;
	}
	//cap off
	else if (ws_o_is_leak_detected) {
		ws_o_status_index = ws_leak_detected;
	}
	//valve off
	else if (!ws_o_is_valve_on) {
		ws_o_status_index = ws_valve_off;
	}
	//valve on but no flow
	//else if (flow_aver_1 <0.6) {
	else if (ws_o_is_flow_fault) {
		ws_o_status_index = ws_valve_on_but_flowis0;
	}
	//valve on but the flow rate is lower than ws_i_warning_flow
	//else if (flow_aver_1 < (ws_i_warning_flow-1.5) ) {
	else if (ws_o_is_flow_warning) {
		ws_o_status_index = ws_valve_flowon_warning;
	}
	//ok to weld?
	else if(ws_o_is_oktoweld) {
	//if (ws_o_is_valve_on ==1 && ws_o_is_flow_ok==1 && ws_o_is_leak_detected==0) {
		ws_o_status_index = ws_ok_to_weld;
	}
}


// only for data log. should be disable in final release.
#ifdef ENABLE_DATA_TXT_LOG
#include "user_fifo.h"
#define MEASURE_DATA_UNIT_SIZE (100)
FIFO_BUFFER_T sensebuf;

void ws_sense_data_write2fifo () {
    // simply web_alive detection
	if (ws_web_datalog_alive) {
		// web_server is alive. so here we count and return.
		ws_web_datalog_alive_cnt++;
		if (ws_web_datalog_alive_cnt >= 4000) {
			// trig time is now 4000*5=20000ms=20s.
			ws_web_datalog_alive = 0;
		}
	}

    // depend on the format of the data you want to write.
    char temp_str_buf[MEASURE_DATA_UNIT_SIZE];
    int temp_str_len;
//    uint32_t temp_time_stamp;
//    temp_time_stamp = Time_GetMs();
    memset(temp_str_buf, 0, MEASURE_DATA_UNIT_SIZE);
#ifdef USE_LSR_FOR_LEAK_DETECT
    temp_str_len = snprintf (temp_str_buf, MEASURE_DATA_UNIT_SIZE, "<br>%.2f %.2f %.4f %.4f %.4f",  flow_aver_1, flow_aver_2, leak_index_average, weighted_leak_index, leak_index_with_est);
    //temp_str_len = snprintf (temp_str_buf, MEASURE_DATA_UNIT_SIZE, "<br>%d %d %.4f %.4f",  fls_1.time_interval, fls_2.time_interval, leak_index_average, dif_flowrate_ramp);
#else
    temp_str_len = snprintf (temp_str_buf, MEASURE_DATA_UNIT_SIZE, "<br>%d %d %.3f",  fls_1.time_interval, fls_2.time_interval, leak_detection_quantized_index);
#endif
    //temp_str_len = snprintf (temp_str_buf, MEASURE_DATA_UNIT_SIZE, "<br>%d %d %.3f",  temp_time_stamp, temp_time_stamp+1, (float)temp_time_stamp/1000);
    if (ws_web_datalog_alive) {
    	// only write when web is alive.
    	fifo_write(&sensebuf, temp_str_buf, temp_str_len);
    }
    fls_1.time_interval = 0; // only new pulse will generate a non-zero value again.
    fls_2.time_interval = 0;
}
#endif /* ENABLE_DATA_TXT_LOG */

/**
 * Added by TMS
 * Detect if the sensor cable is unplugged
 * set the error flag if the sensor is not
 * connected for WS_SENSOR_ABSENCE_DELAY ms.
 */
uint8_t ws_sensor_absense_count = 0;
void ws_sensor_absence_detection() {
#ifdef FLOW_SENSOR_ABSENCE_DETECTION
    //disable the detect if the system is already in error state
    if(!ws_attr_err_flag && ws_flag_after_startup_delay && ws_o_is_valve_on && !ws_o_is_leak_detected)
        if(alive_monitor_1.AliveFlag==0||alive_monitor_2.AliveFlag==0)
        {
            ws_sensor_absense_count+= WS_PROCESS_RUN_PERIOD;
            if(ws_sensor_absense_count >= WS_SENSOR_ABSENCE_DELAY) {
                ws_attr_err_flag = 1;
                debug_sensor_error_count++;
            }
        } else {
            ws_sensor_absense_count = 0;
        }
    else
        ws_sensor_absense_count = 0;
#endif
}

// no method in this fun is subject to the risk of being blocked.
// this fun should be called by main loop periodically.
void ws_process ()
{
	// read cmd/paras input
	ws_read_newctrlparas();

	ws_read_eth_input();
	ws_handle_error_state();
	
	// control actuator.
	// read the setting of bypass
	ws_bypass_control();
	ws_valve_control();
	ws_startup_detecion();

	// read sensor values
	ws_read_flowsensor1();
	ws_read_flowsensor2();

	// process sensor values and set properties accordingly.
	ws_flowrate_detect_flowin();
#ifdef USE_FLOWRATE_BENCHMARK
	ws_update_flow_benchmark();
#endif
	ws_flowrate_detect_leakage();
	
	//
	//added by TMS to detect if the cable 
	//for the flowsensor is corrected plugged 
	//in.
	//
	ws_sensor_absence_detection();
	
#ifdef USE_FLOWRATE_BENCHMARK
	ws_update_weighted_index_threshold();
#endif

#ifdef ENABLE_DATA_TXT_LOG
	ws_sense_data_write2fifo();
#endif /* ENABLE_DATA_TXT_LOG */

	// update output in ETH.
	ws_update_eth_output();

	// update status bt a index. this will be used by http routine.
	ws_status_update();
}

#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS
uint8_t led_ip_display_index = 0;
void _ws_display_ip()
{
	switch (led_ip_display_index)
	{
		case 1:
		case 3:
		case 5:
		case 7:
			dig_led_write_decimal(ip_addr.array[((led_ip_display_index-1)/2)], 0);
			break;
		case 9:
		    dig_led_write_ip_end();
			break;
		default:
		    dig_led_write_blank();
			break;
	}

	if (++led_ip_display_index>9)
		led_ip_display_index = 0;
}
#endif

/**
 * the daemon process which synchronize the value on 
 * the LED display with "flow_aver_2".
 */
#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS
extern volatile uint8_t eips_conn_established;		//added by TMS
volatile uint8_t eips_show_ip = 31;           //counter for showing ip address only three times 31 = 2*3times*(4ipaddr+1end)+1
uint32_t led_mac_display_delay = 0;
#endif

void ws_dig_led_update_daemon() {
#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS
	//if network connection is NOT established
    if(!(eips_conn_established) && eips_show_ip)
    {
        led_mac_display_delay+=WS_DIG_LED_UPDATE_PERIOD;
        if(led_mac_display_delay > 950) //around 1s
        {
            led_mac_display_delay = 0;
            _ws_display_ip();
            eips_show_ip--;
        }
    }
    else if(eips_show_ip)
    {
        eips_show_ip = eips_show_ip%10;
        led_mac_display_delay+=WS_DIG_LED_UPDATE_PERIOD;
        if(led_mac_display_delay > 950) //around 1s
        {
            led_mac_display_delay = 0;
            _ws_display_ip();
            eips_show_ip--;
        }
    }
    else
    {
        float dig_led_val = flow_aver_2<0?0:(float)flow_aver_2;
        dig_led_update(dig_led_val);
        led_ip_display_index = 0;
    }
#else
    float dig_led_val;
    dig_led_val = flow_aver_2<0?0:(float)flow_aver_2;
    dig_led_update(dig_led_val);
#endif
}

/*
 * Functions for GPIO
 */
#define GPIOTAG_DIN_ALL (GPIOTag_DIN_1 | GPIOTag_DIN_2 | GPIOTag_DIN_3)
#define GPIOTAG_DOUT_ALL (GPIOTag_DOUT_1 | GPIOTag_DOUT_2 | GPIOTag_DOUT_3)

//typedef union {
//	struct {
//		uint8_t bit0:1;
//		uint8_t bit1:1;
//		uint8_t bit2:1;
//	} single;
//	struct {
//		uint8_t bit0_3:3;
//	} map;
//	uint8_t raw;
//	uint8_t * pointer;
//} GPIO_DATA;

//GPIO_DATA _old_dout_data;
//GPIO_DATA _new_dout_data;
//GPIO_DATA _din_data;

uint8_t _ws_dout_old = 0;

void ws_gpio_init() {

	GPIO_TagConfigProperties(
			GPIOTAG_DIN_ALL,
			GPIO_SET_IN_PULLDOWN, GPIO_SPD_HIGH);

	GPIO_TagConfigProperties(
			GPIOTAG_DOUT_ALL,
			GPIO_SET_OUT_PUSHPULL, GPIO_SPD_HIGH);

	GPIOIntDisable(GPIO_PORTE_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2);
}

void ws_update_io() {
	ETH_IO_DATA_OBJ_OUTPUT.data.gpio_din_1_3 = GPIO_TagStateRead(GPIOTAG_DIN_ALL);//( GPIO_TagRead(GPIOTag_DIN_1) +2*(GPIO_TagRead(GPIOTag_DIN_2))+4*(GPIO_TagRead(GPIOTag_DIN_3)));
	GPIO_TagStateWriteOnce(GPIOTAG_DOUT_ALL, (uint8_t*)&_ws_dout_old, ETH_IO_DATA_OBJ_INPUT.data.gpio_dout_1_3);
}
