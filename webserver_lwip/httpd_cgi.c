/**
  ******************************************************************************
  * @file    httpd_cg_ssi.c
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    31-July-2013
  * @brief   Webserver SSI and CGI handlers
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  * History
  * @20170222
  *  - use 2nd flowsensor's value to show as flowrate in GUI.
  */

/* Includes ------------------------------------------------------------------*/


#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"


#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#include "ws_func.h"

#include "bsp_eeprom_addr.h"

#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS
#include "netconf.h"
#elif WS_FIELDBUS_TYPE == FIELDBUS_TYPE_PNIO
#include <pn_includes.h>
#include "pn_user.h"
#include "pn_config.h"
#else
#endif

extern uint8_t flag_fieldbus_down;

//tSSIHandler ADC_Page_SSI_Handler;
#define str_buf_len_1 50
uint32_t test_time_tick_in_http = 0;
uint32_t val_to_display = 0;
uint32_t val_to_display2 = 9999;
char text_to_display1[str_buf_len_1] = {' '};
uint8_t isto_info_update = 1; // 0 stop the update, 1 updating!

/* we will use character "t" as tag for CGI */
char const* TAGCHAR = "disp1";

#ifdef ENABLE_DATA_TXT_LOG
#define SSI_TAG_NUM (25)
#else
#ifdef USE_WS_ORIGIN_WEBFILES
#define SSI_TAG_NUM (30)
#else
#define SSI_TAG_NUM (23)
#endif
#endif
//char const** TAGS = &TAGCHAR;
char const* TAGS[SSI_TAG_NUM] = {"disp1",
						"disp2",
						"disp3",
						"disp4",
						"disp5",
						"disp6",
						"disp7",
						"disp8",
						"disp9",
						"disp10",
						"disp11",
						"disp12",
						"disp13",
						"disp14",
						"disp15",
						"disp16",
						"disp17",
						"disp18",
						"disp19",
						"disp20",
						"disp21",
						"disp22",
						"disp23"
#ifdef ENABLE_DATA_TXT_LOG
						,
						"disp24",
						"disp25"
#endif /* ENABLE_DATA_TXT_LOG */
#ifdef USE_WS_ORIGIN_WEBFILES
						,
						"flow_0",
						"flow_1",
						"flow_2",
						"info_update",
						"disp28",
						"disp29",
						"disp30"
#endif /* ENABLE_DATA_TXT_LOG */
};

/* CGI handler for LED control */ 
const char * LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

/* Html request for "/leds.cgi" will start LEDS_CGI_Handler */
const tCGI LEDS_CGI={"/leds.cgi", LEDS_CGI_Handler};

/* Cgi call table, only one CGI used */
tCGI CGI_TAB[1];

// convert int to c type string.
// only fit int within 0~2^16
void int2cstr (int val, char *str_ary)
{
	char digit[5] = {0};
	char dec_order = 5;
	int i = 0;

	digit[0] = val/10000;
	digit[1] = (val-digit[0]*10000)/1000;
	digit[2] = (val-digit[0]*10000 - digit[1]*1000)/100;
	digit[3] = (val-digit[0]*10000-digit[1]*1000-digit[2]*100)/10;
	digit[4] = val-digit[0]*10000-digit[1]*1000-digit[2]*100-digit[3]*10;

	if (digit[0]) dec_order = 5;
	else if(digit[1]) dec_order = 4;
	else if(digit[2]) dec_order = 3;
	else if(digit[3]) dec_order = 2;
	//else if(digit[4]) dec_order = 1;
	else dec_order = 1;

	for (i = 0; i < dec_order; i++) {
		*(str_ary + i) 	= (char) (digit[5-dec_order+i] + 0x30);
	}
	*(char *)(str_ary + dec_order)	= (char) 0;

}

// convert double to c type string
// only fit the double within 1.00~65535.99
void double2cstr (double val, char *str_ary)
{
	int d_int_part = 0;
	int d_dec_part2bit = 0;
	int dec_point_pos = 0;
	char dec[3] = {'.', 0, 0};

	d_int_part = floor(val);
	d_dec_part2bit = floor ((double )(val - d_int_part) * 100);
	dec[1] = d_dec_part2bit/10;
	dec[2] = d_dec_part2bit - dec[1]*10;

	dec[1] += 0x30;
	dec[2] += 0x30;

	int2cstr(d_int_part, str_ary);
	dec_point_pos = strlen(str_ary);
	memcpy((char*)(str_ary+dec_point_pos), dec, 3);

}


/**
  * @brief  SSI handler, update status/info accordingly.
  */
u16_t ADC_Handler(int iIndex, char *pcInsert, int iInsertLen)
{
  // update values tag by tag
#ifdef USE_WS_ORIGIN_WEBFILES
	uint8_t tmpvar = 0;
#endif
	int temp_str_len = 0;
	int i = 0;

#ifdef ENABLE_DATA_TXT_LOG
	char temp_str[9600] = {' '};
	memset(temp_str, 0, 9600);
#else /* ENABLE_DATA_TXT_LOG */
	char temp_str[100] = {' '};
	memset(temp_str, 0, 100);
#endif /* ENABLE_DATA_TXT_LOG */

#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_PNIO
	uint32_t pnio_ip;
	uint8_t *p_temp;
	uint8_t ip0, ip1, ip2, ip3;
#endif

	switch (iIndex) {
#ifndef USE_WS_ORIGIN_WEBFILES
	case 0:	/* flow ok status */
		//temp_str_len = sprintf (temp_str, "%d", ws_o_is_flow_ok);
		if(ws_o_inflow_status_index == 2) temp_str_len = sprintf (temp_str, "%s", "OK");
		else if(ws_o_inflow_status_index == 1) temp_str_len = sprintf (temp_str, "%s", "!");
		else temp_str_len = sprintf (temp_str, "%s", "X");
		break;
	case 1: /*valve state*/
		//temp_str_len = sprintf (temp_str, "%d", ws_o_is_valve_on);
		if (ws_o_is_valve_on == 0) temp_str_len = sprintf (temp_str, "%s", "OFF");
		else temp_str_len = sprintf (temp_str, "%s", "ON");
		break;
	case 2: /*  detection bypass? */
		//temp_str_len = sprintf (temp_str, "%d", ws_o_is_Bypassed);
		if (ws_o_is_Bypassed == 0) temp_str_len = sprintf (temp_str, "%s", "ON");
		else temp_str_len = sprintf (temp_str, "%s", "OFF");
		break;
	case 3: /* value of flowrate */
		temp_str_len = sprintf (temp_str, "%.2f", flow_aver_1);
		break;
	case 4: /* current setting of flowrate warnning */
		temp_str_len = sprintf (temp_str, "%.2f", ws_i_warning_flow);
		break;
	case 5: /* current setting of flowrate fault */
		temp_str_len = sprintf (temp_str, "%.2f", ws_i_fault_flow);
		break;
	case 6: /* current setting of leas response sensitivity */
		temp_str_len = sprintf (temp_str, "%d", ws_i_cmd_leak_response);
		break;
	case 7: /* is ok to weld */
		temp_str_len = sprintf (temp_str, "%d", ws_o_is_oktoweld);
		break;
	case 8: /* time tick in sec*/
		temp_str_len = sprintf (temp_str, "%d", test_time_tick_in_http);
		break;
	case 9: /* leak response in text */
		if (ws_i_cmd_leak_response == 0) temp_str_len = sprintf (temp_str, "%s", "Slow");
		else if (ws_i_cmd_leak_response == 1) temp_str_len = sprintf (temp_str, "%s", "Normal");
		else if (ws_i_cmd_leak_response == 2) temp_str_len = sprintf (temp_str, "%s", "Fast");
		else temp_str_len = sprintf (temp_str, "%s", "Wrong input setting");
		break;
	case 10: /* is leak detected*/
		temp_str_len = sprintf (temp_str, "%d", ws_o_is_leak_detected);
		break;
	case 11: /* default of flowrate_waring */
		temp_str_len = sprintf (temp_str, "%.2f", ws_def_warning_flow);
		break;
	case 12: /* default of flowrate_fault */
		temp_str_len = sprintf (temp_str, "%.2f", ws_def_fault_flow);
		break;
	case 13: /* default of leak response */
		temp_str_len = sprintf (temp_str, "%d", ws_def_cmd_leak_response);
		break;
	case 14: /* ws status index in num */
		temp_str_len = sprintf (temp_str, "%d", ws_o_status_index);
		break;
	case 15: /* ws status in text, according to current ws_o_status_index value  */
		if (ws_o_status_index == 3) {
			temp_str_len = sprintf (temp_str, "%s", "Leakage is detected");
		}
		else if (ws_o_status_index == 4) {
			temp_str_len = sprintf (temp_str, "%s", "OK To Weld");
		}
		else if (ws_o_status_index == 5) {
			temp_str_len = sprintf (temp_str, "%s", "Valve is off");
		}
		else if (ws_o_status_index == 6) {
			temp_str_len = sprintf (temp_str, "%s", "Valve is on but too little water!");
		}
		else {
			temp_str_len = sprintf (temp_str, "%s", "Status not defined!");
		}
		break;
	case 16: /* current startup delay */
		temp_str_len = sprintf (temp_str, "%.2f", ws_i_stablization_delay);
		break;
	case 17: /* current startup leak threshold  */
		temp_str_len = sprintf (temp_str, "%.2f", ws_i_startup_leak);
		break;
	case 18: /* default startup delay */
		temp_str_len = sprintf (temp_str, "%.2f", ws_def_stablization_delay);
		break;
	case 19: /* default startup leak thresh */
		temp_str_len = sprintf (temp_str, "%.2f", ws_def_startup_leak);
		break;
	case 20: /* IP addr */
		temp_str_len = sprintf (temp_str, "%s", ws_o_ipaddr_text);
		break;
	case 21: /* show ws_para_sense_quantized_index */
		temp_str_len = sprintf (temp_str, "%.2f", ws_para_sense_quantized_index);
		break;
	case 22: /* show threshold_deviation_int */
		temp_str_len = sprintf (temp_str, "%.3f", threshold_leak_detection_quantized_index);
		break;
	case 24: /* data log file */
		temp_str_len = sprintf (temp_str, "%.3f", ws_def_dif_flowrate_ramp);
		break;
#ifdef ENABLE_DATA_TXT_LOG
#include "user_fifo.h"
		extern FIFO_BUFFER_T sensebuf;
	case 23: /* data log file */
		temp_str_len = fifo_get_clear (&sensebuf, temp_str);
		break;
#endif /* ENABLE_DATA_TXT_LOG */
	default: /* default */
		break;
#else /* USE_WS_ORIGIN_WEBFILES */
	case 0:	/* flow ok status */
		// 0-fault, 1-warning, 2-ok, 3-closed.
		if (ws_o_is_valve_on == 0) tmpvar = 3;
		else tmpvar = ws_o_inflow_status_index;
		temp_str_len = sprintf (temp_str, "./status_flow_%d.gif", tmpvar);
		break;
	case 1: /*valve state*/
		if (ws_o_is_valve_on == 0) {
			if (ws_i_bus_valveon == 0 && flag_fieldbus_down == 0) {
				temp_str_len = sprintf (temp_str, "%s", "./status_valve_closed_locked.gif");
			}
			else {
				temp_str_len = sprintf (temp_str, "%s", "./status_valve_closed.gif");
			}
		}
		else temp_str_len = sprintf (temp_str, "%s", "./status_valve_open.gif");
		break;
	case 2: /*  detection bypass? */
		if ((ws_i_bus_bypass == 1 && ws_o_is_Bypassed==1)) {
			// return pic with lock.
			tmpvar = 2;
		}
		else {
			tmpvar = ws_o_is_Bypassed;
		}
		temp_str_len = sprintf (temp_str, "./status_detection_%d.gif", tmpvar);

		break;
	case 3: /* value of flowrate */
		temp_str_len = sprintf (temp_str, "%.1f", flow_aver_2);
		break;
	case 4: /* current setting of flowrate warnning */
		temp_str_len = sprintf (temp_str, "%.1f", ws_i_warning_flow);
		break;
	case 5: /* current setting of flowrate fault */
		temp_str_len = sprintf (temp_str, "%.1f", ws_i_fault_flow);
		break;
	case 8: /* time tick in sec*/
		temp_str_len = sprintf (temp_str, "%d", test_time_tick_in_http);
		break;
	case 9: /* leak response in text */
		if (ws_i_cmd_leak_response == 0) temp_str_len = sprintf (temp_str, "%s", "Fastest");
		else if (ws_i_cmd_leak_response == 1) temp_str_len = sprintf (temp_str, "%s", "Fast");
		else if (ws_i_cmd_leak_response == 2) temp_str_len = sprintf (temp_str, "%s", "Normal");
		else if (ws_i_cmd_leak_response == 3) temp_str_len = sprintf (temp_str, "%s", "Slow");
		else if (ws_i_cmd_leak_response == 4) temp_str_len = sprintf (temp_str, "%s", "Slowest");
		else temp_str_len = sprintf (temp_str, "%s", "Wrong input setting");
		break;
	/*
	 * 	ws_inflow_fault = 0,
		ws_inflow_warn =1,
		ws_inflow_ok =2,
		ws_leak_detected = 3,
		ws_ok_to_weld = 4,
		ws_valve_off =5,
		ws_valve_on_but_flowis0 = 6,
		ws_valve_on_and_bypassed = 7,
		ws_valve_flowon_warning = 9
	 *  */
	case 15: /* ws status in text, according to current ws_o_status_index value  */
		temp_str_len = sprintf (temp_str, "/src/info_st_%d.gif", ws_o_status_index);
		break;
	case 16: /* current startup delay */
		temp_str_len = sprintf (temp_str, "%.1f", ws_i_stablization_delay);
		break;
	case 17: /* current startup leak threshold  */
		temp_str_len = sprintf (temp_str, "%.1f", ws_i_startup_leak);
		break;
	case 21: /* show ws_para_sense_quantized_index */
		temp_str_len = sprintf (temp_str, "%.3f", ws_para_sense_quantized_index);
		break;
	case 22: /* show threshold_deviation_int */
		temp_str_len = sprintf (temp_str, "%.3f", threshold_leak_detection_quantized_index);
		break;
	/* pare flowrate into singel num*/
	case 23: /* current startup delay */
		tmpvar = (uint8_t)floor(flow_aver_1);
		tmpvar = tmpvar / 10;
		if (tmpvar == 0) {
			temp_str_len = sprintf (temp_str, "%s", "flowrate_blank.gif");
		}
		else {
			temp_str_len = sprintf (temp_str, "./flowrate_%d.gif", tmpvar);
		}
		break;
	case 24: /* current startup delay */
		tmpvar = (uint8_t)floor(flow_aver_1);
		tmpvar = tmpvar % 10;
		temp_str_len = sprintf (temp_str, "./flowrate_%d.gif", tmpvar);
		break;
	case 25: /* current startup delay */
		tmpvar = (uint8_t)floor((flow_aver_1 - floor(flow_aver_1))*10);
		temp_str_len = sprintf (temp_str, "./flowrate_%d.gif", tmpvar);
		break;
	case 26: /* info_update */
		tmpvar = (uint8_t)floor((flow_aver_1 - floor(flow_aver_1))*10);
		temp_str_len = sprintf (temp_str, "./flowrate_%d.gif", tmpvar);
		break;
	case 27: /* ws var, ver of bus */
		temp_str_len = sprintf (temp_str, "%s", "V2.1.0");
		break;
	case 28: /* ws logo (eips/pnio/dido) */
#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS
		temp_str_len = sprintf (temp_str, "%s", "eips");
#elif WS_FIELDBUS_TYPE == FIELDBUS_TYPE_PNIO
		temp_str_len = sprintf (temp_str, "%s", "pnio");
#elif defined WS_FILEDBUS_NONE_BUT_DIDO
		temp_str_len = sprintf (temp_str, "%s", "dido");
#endif
		break;
	case 29: /* ws variant of bus(eips, pnio, dido) */
#if WS_FIELDBUS_TYPE == FIELDBUS_TYPE_EIPS
		temp_str_len = sprintf (temp_str, "%d.%d.%d.%d", uip_add_0, uip_add_1, uip_add_2, uip_add_3);
#elif WS_FIELDBUS_TYPE == FIELDBUS_TYPE_PNIO
		IOD_cfgGet(CFG_KEY_NET_TEMP_IP, &pnio_ip);
		p_temp = (uint8_t*)&pnio_ip;
		ip3 =  *p_temp;
		ip2 = *(p_temp+1);
		ip1 = *(p_temp+2);
		ip0 = *(p_temp+3);
		temp_str_len = sprintf (temp_str, "%d.%d.%d.%d", ip0, ip1, ip2, ip3);
#else
		/* this is a dido version. use fixed IP for web server. */
		temp_str_len = sprintf (temp_str, "%s", "172.24.1.1");
#endif
		break;
	default:
		break;
#endif /* USE_WS_ORIGIN_WEBFILES */
	}

	for (i = 0; i < temp_str_len; i++) {
		*(pcInsert + i) = temp_str[i];
	}

	return temp_str_len;
}



/**
  * @brief  CGI handler
  */
const char * LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	  uint32_t i=0;
	  uint8_t isNewCtrlParasSubmit = 0;
	  uint8_t isNewCmdSubmit = 0;

	  /* We have only one CGI handler iIndex = 0 */
	  if (iIndex==0)
	  {
		isNewCtrlParasSubmit = 0;
		isNewCmdSubmit = 0;
	    /* Check cgi parameter : example GET /leds.cgi?led=2&led=4 */
	    for (i=0; i<iNumParams; i++)
	    {
#ifndef USE_WS_ORIGIN_WEBFILES
	      // check the paras received.
	      if (strcmp(pcParam[i] , "para1")==0) {
	    	  //ws_i_web_reset = 1;
	    	  ws_i_web_reset = (ws_i_web_reset) ? 0 : 1;
	    	  isNewCmdSubmit = 1;

	      }
	      else if (strcmp(pcParam[i] , "para2")==0) {
	    	  // para2 bth clicked for valve control

	    	  ws_i_web_valveon = (ws_o_is_valve_on==1)?0:1;
	    	  isNewCmdSubmit = 1;
	    	  //return "/main_window.shtml";
	      }
	      else if (strcmp(pcParam[i] , "para3")==0) {
	    	  // para3 bth clicked for bypass control
	    	  ws_i_web_bypass = (ws_o_is_Bypassed==0)?1:0;
	    	  isNewCmdSubmit = 1;
	    	  //return "/main_window.shtml";
	      }
	      else if (strcmp(pcParam[i] , "para4")==0) {
	      }
	      else if (strcmp(pcParam[i] , "para5")==0) {
	    	  //start control para settings.
	    	  //isNewCmdSubmit = 1;
	    	  return "/para_setting.html";

	      }
	      else if (strcmp(pcParam[i] , "para6")==0) {
	    	  //show default paras
	    	  isNewCmdSubmit = 1;
//	    	  return "/main_window.shtml";
	      }
	      else if (strcmp(pcParam[i] , "para7")==0) {
	    	  //submit paras
	    	  //set submit flag, shall not return here.
	    	  isNewCtrlParasSubmit = 1;
	      }
	      else if (strcmp(pcParam[i] , "para8")==0) {
	    	  // read new flow_warn, shall not return here.
	    	  ws_i_warning_flow = strtod (pcValue[i], NULL);
	      }
	      else if (strcmp(pcParam[i] , "para9")==0) {
	    	  // read new flow_fault, shall not return here.
	    	  ws_i_fault_flow = strtod (pcValue[i], NULL);
	      }
	      else if (strcmp(pcParam[i] , "para10")==0) {
	    	  // read new leak_resp, shall not return here.
	    	  ws_i_cmd_leak_response = (uint8_t)atoi (pcValue[i]);
	      }
	      else if (strcmp(pcParam[i] , "para12")==0) {
	    	  // read startup delay, shall not return here.
	    	  ws_i_stablization_delay = strtod (pcValue[i], NULL);
	      }
	      else if (strcmp(pcParam[i] , "para13")==0) {
	    	  // read startup threshold, shall not return here.
	    	  ws_i_startup_leak = strtod (pcValue[i], NULL);
	      }
	      else if (strcmp(pcParam[i] , "cfg_ip_addr")==0) {
	    	  // new IP addr input, shall not return here.
	    	  //ws_i_ipaddr_text = pcValue[i];
	    	  memcpy (ws_i_ipaddr_text,  pcValue[i], 20);
	    	  ws_isNewIPAddr = 1;
	      }
	      else if (strcmp(pcParam[i] , "para11")==0) {
	    	  // go back (cancel the setting)
	    	  isNewCmdSubmit = 1;
//	    	  return "/main_window.shtml";
	      }
	      else if (strcmp(pcParam[i] , "ltsh_frate")==0) {
	    	  //leak detection threshold of flowrate
	    	  isThereNewCtrParas  = 1;
	    	  ws_para_sense_quantized_index = strtod (pcValue[i], NULL);
	      }
	      else if (strcmp(pcParam[i] , "ltsh_fvol")==0) {
	    	  //leak detection threshold of leaked volume
	    	  isThereNewCtrParas = 1;
	    	  threshold_leak_detection_quantized_index = strtod (pcValue[i], NULL);
	      }
#ifdef ENABLE_DATA_TXT_LOG
	      else if (strcmp(pcParam[i] , "logact")==0) {
	    	  // randpara means we receive cmd from web server. keep web_cmd_mode alive.
	    	  ws_web_datalog_alive = 1;
	    	  ws_web_datalog_alive_cnt = 0;
	      }
	      else if (strcmp(pcParam[i] , "def_rampdif")==0) {
	    	  // randpara means we receive cmd from web server. keep web_cmd_mode alive.
	    	  isThereNewCtrParas = 1;
	    	  ws_def_dif_flowrate_ramp = strtod (pcValue[i], NULL);
	      }
#endif /* ENABLE_DATA_TXT_LOG */
	    }

#else /* USE_WS_ORIGIN_WEBFILES */
	      // check the paras received.
	      if (strcmp(pcParam[i] , "para1")==0) {
	    	  ws_i_web_reset = (ws_i_web_reset) ? 0 : 1;
	    	  isNewCmdSubmit = 1;
	    	  return "/src/controls.shtml";

	      }
	      else if (strcmp(pcParam[i] , "para2")==0) {
	    	  ws_i_web_valveon = (ws_o_is_valve_on==1)?0:1;
	    	  isNewCmdSubmit = 1;
	    	  return "/src/controls.shtml";
	      }
	      else if (strcmp(pcParam[i] , "para3")==0) {
	    	  // para3 bth clicked for bypass control
	    	  ws_i_web_bypass = (ws_o_is_Bypassed==0)?1:0;
	    	  isNewCmdSubmit = 1;
	    	  return "/src/controls.shtml";
	      }
	      else if (strcmp(pcParam[i] , "para5")==0) {
	    	  //start control para settings.
	    	  isto_info_update = 0;
	    	  return "/src/info_set.shtml";
	      }
	      else if (strcmp(pcParam[i] , "para6")==0) {
	    	  //show default paras.
	    	  return "/src/info_set_default.shtml";
	      }
	      else if (strcmp(pcParam[i] , "para7")==0) {
	    	  //start control para settings.
	    	  isto_info_update = 1;
	    	  isThereNewCtrParas = 1;
	      }
	      else if (strcmp(pcParam[i] , "para8")==0) {
	    	  // read new flow_warn, shall not return here.
	    	  ws_i_warning_flow = strtod (pcValue[i], NULL);
	    	  isThereNewCtrParas = 1;
	      }
	      else if (strcmp(pcParam[i] , "para9")==0) {
	    	  // read new flow_fault, shall not return here.
	    	  ws_i_fault_flow = strtod (pcValue[i], NULL);
	    	  isThereNewCtrParas = 1;
	      }
	      else if (strcmp(pcParam[i] , "para10")==0) {
	    	  // read new leak_resp, shall not return here.
	    	  ws_i_cmd_leak_response = (uint8_t)atoi(pcValue[i]);
	    	  isThereNewCtrParas = 1;
	      }
	      else if (strcmp(pcParam[i] , "para12")==0) {
	    	  // read startup delay, shall not return here.
	    	  ws_i_stablization_delay = (double)atoi(pcValue[i]);
	    	  isThereNewCtrParas = 1;
	      }
	      else if (strcmp(pcParam[i] , "para13")==0) {
	    	  // read startup threshold, shall not return here.
	    	  ws_i_startup_leak = (double)atoi(pcValue[i]);
	    	  isThereNewCtrParas = 1;
	      }
	      else if (strcmp(pcParam[i] , "para11")==0) {
	    	  // go back (cancel the setting)
	    	  // return the info page here.
	    	  isNewCmdSubmit = 1;
	    	  isto_info_update = 1;
	    	  return "/src/info.shtml";
	      }
	      else if (strcmp(pcParam[i] , "setup")==0) {
	    	  // go back (cancel the setting)
	    	  // return the info page here.
	    	  isThereNewCtrParas = 1;
	    	  if ((uint8_t)atoi (pcValue[i]) == 0)
	    	  {
	    		  // all new para from form read. return here.
	    		  isto_info_update =  1;
	    		  return "/src/info.shtml";
	    	  }
	      }
//	      } else if (strcmp(pcParam[i], "update")==0) {
//	          if(atoi(pcVale[i]) == 1)
//	          {
//	              //write
//	              //JUMP_CONFIG jump_config = LONG_WAIT;
//	              //EEPROMProgram ((uint32_t*)&jump_config, EEPROM_ADDR_BL_JUMP_CONFIG, EEPROM_LEN_BL_JUMP_CONFIG);
//	              return "/src/jump_config_success.html";
//	          }
//	      }
#if defined RESERVE_INTERNAL_PARA_SETTING
	      // disabled as now we check and respond to leak_resp periodically in ws_process.
	      // we can read and respond to new leak_resp in only read eth_input.
	      else if (strcmp(pcParam[i] , "ltsh_frate")==0) {
	    	  //leak detection threshold of flowrate
	    	  isNewInternalSetting  = 1;
	    	  ws_para_sense_quantized_index = strtod (pcValue[i], NULL);
	      }
	      else if (strcmp(pcParam[i] , "ltsh_fvol")==0) {
	    	  //leak detection threshold of leaked volume
	    	  isNewInternalSetting = 1;
	    	  threshold_leak_detection_quantized_index = strtod (pcValue[i], NULL);
	      }
#endif
	    }
#endif /* USE_WS_ORIGIN_WEBFILES */
	  }
	  if (isNewCmdSubmit == 1) {
		  return "/main_window.shtml";
	  }

	  if (isNewCtrlParasSubmit == 1 || ws_isNewIPAddr == 1) {
		  isThereNewCtrParas = 1;
		  return "/main_window.shtml";
	  }
	  else {
#if defined RESERVE_INTERNAL_PARA_SETTING
		  return "/index.html";
#else
		  return "/index.shtml";
#endif
	  }

}

/**
 * Initialize SSI handlers
 */
void httpd_ssi_init(void)
{
  /* configure SSI handlers (ADC page SSI) */
  http_set_ssi_handler(ADC_Handler, (char const **)TAGS, SSI_TAG_NUM);
}


/**
 * Initialize CGI handlers
 */
void httpd_cgi_init(void)
{ 
  /* configure CGI handlers (LEDs control CGI) */
  CGI_TAB[0] = LEDS_CGI;
  http_set_cgi_handlers(CGI_TAB, 1);
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
