/*
 * httpd_cgi.h
 *
 *  Created on: Sep 28, 2017
 *      Author: CNTHXIE
 */

#ifndef WEBSERVER_LWIP_HTTPD_CGI_H_
#define WEBSERVER_LWIP_HTTPD_CGI_H_

//#define VAR_STPO    45
//#define VAR_LR      6
//#define VAR_SD      47
//#define VAR_SL      48
//#define VAR_UPD     49

//#ifdef ENABLE_DATA_TXT_LOG
//#define SSI_TAG_NUM (25)
//#else
//#ifdef USE_WS_ORIGIN_WEBFILES
//#define SSI_TAG_NUM (50)
//#else
//#define SSI_TAG_NUM (23)
//#endif
//#endif

#define AIO_EIPS        0
#define AIO_PNIO        1
#define AIO_PNIOIO      2
#define AIO_SF          4
#define AIO_FF          8

typedef enum
{
    DISP1 = 0,
    DISP2,
    DISP3,
    DISP4,
    DISP_F_W,      //4
    DISP_F_F,
    VAR_LR,        //6, LEAK RESPONSE
    DISP8,
    DISP9,
    DISP10,
    DISP11,
    DISP12,
    DISP13,
    DISP14,
    DISP15,
    DISP16,
    DISP17,
    DISP18,
    DISP19,
    DISP20,
    DISP21,
    DISP22,
    DISP23,
#ifdef ENABLE_DATA_TXT_LOG
    DISP24,
    DISP25,
#endif /* ENABLE_DATA_TXT_LOG */
#ifdef USE_WS_ORIGIN_WEBFILES
    FLOW_0,
    FLOW_1,
    FLOW_2,
    INFO_UPDATE,
    DISP28,
    IE_PTC,
    IPADDR,
    DSP_LR0,
    DSP_LR1,
    DSP_LR2,
    DSP_LR3,
    DSP_LR4,
    DSP_SD1,
    DSP_SD2,
    DSP_SD4,
    DSP_SD8,
    DSP_SD16,
    DSP_SL2,
    DSP_SL4,
    DSP_SL6,
    DSP_SL8,
    DSP_SL10,
    VAR_STPO,
    RESERVED0,
    VAR_SD,
    VAR_SL,
    VAR_UPD,
	IP_DIS,
	HD_LOGO,
	DP_AIO_NETWORK,
    SSI_TAG_NUM // Here the SSI_TAG_NUM will automatically increase!
#endif
}   TAG_INDEXS;

//char const** TAGS = &TAGCHAR;
char const* TAGS[SSI_TAG_NUM] = {
    "disp1",
    "disp2",
    "disp3",
    "disp4",
    "disp_f_w",      //4
    "disp_f_f",
    "var_lr",        //6, leak response
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
    "ie_ptc",		//Industrial Ethernet Protocol
    "ipaddr",
    "dsp_lr0",
    "dsp_lr1",
    "dsp_lr2",
    "dsp_lr3",
    "dsp_lr4",
    "dsp_sd1",
    "dsp_sd2",
    "dsp_sd4",
    "dsp_sd8",
    "dsp_sd16",
    "dsp_sl2",
    "dsp_sl4",
    "dsp_sl6",
    "dsp_sl8",
    "dsp_sl10",
    "var_stpo",
    "reserved0",
    "var_sd",
    "var_sl",
    "var_upd",
	"ip_dis",
	"hd_logo",
	"dpaionw"
#endif /* ENABLE_DATA_TXT_LOG */
};

char const* SELECTED ="selected=\"selected\"";
char const* NONE = "";
char const* STR_TRUE = "true";
char const* STR_FALSE = "false";

char const* STR_SYS_NAME_SF = "SmartFlow";
char const* STR_SYS_NAME_FF = "FlexFlow";

char const* STR_SYS_NETWORK_EIPS = "Ethernet/IP";
char const* STR_SYS_NETWORK_PNIO = "Profinet";
char const* STR_SYS_NETWORK_PNIOIO = "Profinet + IO";

/* CGI handler for LED control */
const char * LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

/* Html request for "/leds.cgi" will start LEDS_CGI_Handler */
const tCGI LEDS_CGI={"/leds.cgi", LEDS_CGI_Handler};

/* Cgi call table, only one CGI used */
tCGI CGI_TAB[1];

#define FLOW_AVER   ((flow_aver_2>0)?flow_aver_2:0)

#endif /* WEBSERVER_LWIP_HTTPD_CGI_H_ */
