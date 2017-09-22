/**
 *  \file new 1
 *  \brief Brief
 */

#ifndef WS_FUNC_H_
#define WS_FUNC_H_

#include "system.h"

// The version info our ws functionality implementation.
#define WS_FUNC_VERSION_MAJOR 1
#define WS_FUNC_VERSION_MINOR 1
#define WS_FUNC_VERSION_REVISION 1

// input - cmd
extern uint8_t ws_i_cmd_valve_on;
extern uint8_t ws_i_cmd_bypass;
extern uint8_t ws_i_cmd_reset;

extern uint8_t ws_i_web_reset;
extern uint8_t ws_i_web_valveon;
extern uint8_t ws_i_web_bypass;

extern uint8_t ws_i_cmd_brwr_write_access;

// input - properties
extern double ws_i_warning_flow;
extern double ws_i_fault_flow;
extern double ws_i_stablization_delay;
extern double ws_i_startup_leak;
extern uint32_t ws_i_cmd_leak_response;				    // 0: slow, 1: normal, 2: fast
extern uint8_t isThereNewCtrParas;

#ifdef RESERVE_INTERNAL_PARA_SETTING
extern uint8_t isNewInternalSetting;
#endif /* RESERVE_INTERNAL_PARA_SETTING */

// internal - parameters/attributes
//#define qv_buf_size 100
extern double qv_flowrate_1;			                // volume flowrate in l/min
extern double qv_flowrate_2;			                // volume flowrate in l/min

extern double ws_def_warning_flow;
extern double ws_def_fault_flow;
extern double ws_def_stablization_delay;
extern double ws_def_startup_leak;
extern uint8_t ws_def_cmd_leak_response;

extern char ws_i_ipaddr_text[20];
extern char ws_o_ipaddr_text[20];
extern uint8_t ws_isNewIPAddr;

// output - feedback
extern double ws_o_current_flow;
extern uint8_t ws_o_is_valve_on;
extern uint8_t ws_o_is_Bypassed;
extern uint8_t ws_o_is_leak_detected;
extern uint8_t ws_o_is_flow_warning;
extern uint8_t ws_o_is_flow_fault;
extern uint8_t ws_o_is_flow_ok;
extern uint8_t ws_o_inflow_status_index;
extern uint8_t ws_o_is_oktoweld;
extern uint8_t ws_o_is_minflow;		                //????
extern uint8_t ws_o_is_caploss;                        // the difference with leak_detection

extern double threshold_deviation_aver;			// the deviation of 2 sensors' average
extern double threshold_deviation_int;			// the integral of 2 sensors' dev
extern double ws_para_sense_quantized_index;			// the coff of short-time flowrate average based leadk volume calculation.
extern double threshold_leak_detection_quantized_index;
extern double ws_def_dif_flowrate_ramp;

extern double flow_dev_aver;
extern double flow_dev_int;		// integral of dev in unit of Litre
extern double flow_aver_1;
extern double flow_aver_2;

extern uint8_t ws_o_status_index;

extern uint8_t ws_web_datalog_alive;
extern uint32_t ws_web_datalog_alive_cnt;

// control signal buf of web
extern uint8_t ws_i_web_reset;
extern uint8_t ws_i_web_valveon;
extern uint8_t ws_i_web_bypass;

// control signal buf of fieldbus (eip or pnio)
extern uint8_t ws_i_bus_reset;
extern uint8_t ws_i_bus_valveon;
extern uint8_t ws_i_bus_bypass;

void ws_init(void);
void ws_process(void);
void ws_dig_led_update_daemon();
typedef enum
{
	ws_inflow_fault = 0,
	ws_inflow_warn =1,
	ws_inflow_ok =2,
	ws_leak_detected = 3,
	ws_ok_to_weld = 4,
	ws_valve_off =5,
	ws_valve_on_but_flowis0 = 6,
	ws_valve_on_and_bypassed = 7,
	ws_valve_flowon_warning = 9
} typedef_ws_status;



#endif /* WS_FUNC_H_ */
