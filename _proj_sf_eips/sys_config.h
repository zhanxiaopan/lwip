/**
 *  \file sys_config.h
 *  \brief define system/application level properties of ws.
 */

#ifndef __SYS_CONFIG_H_
#define __SYS_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * Select which type of FieldBus would be used in WS.
 * */
#define FIELDBUS_TYPE_NONE	(0)
#define FIELDBUS_TYPE_EIPS	(1)
#define FIELDBUS_TYPE_PNIO	(2)
#define FIELDBUS_TYPE_BL    (3)
#define WS_FIELDBUS_TYPE FIELDBUS_TYPE_EIPS
#if WS_FIELDBUS_TYPE > FIELDBUS_TYPE_BL
#error "A wrong IO bus type has been defined!"
#endif


/*
 * Uncomment below to indicate the usage of TIVA launchpad instead of actual ws hardware.
 * It mainly affect the definition of the used LED.
 * */
//#define __USE_LAUNCH_PAD


/*
 * enable to save data as txt file in web browser.
 * conflict with USE_WS_ORIGIN_WEBFILES so far.
 * */
//#define ENABLE_DATA_TXT_LOG


/*
 * Enable the flowrate calculation with discrete leap.
 * */
#define USE_FLOWRATE_BENCHMARK


/*
 * copy the design of original WS webserver html files.
 * conflict with ENABLE_DATA_TXT_LOG so far.
 * */
#define USE_WS_ORIGIN_WEBFILES


/*
 * Due to the less flexibility in http_cgi,
 * ENABLE_DATA_TXT_LOG and USE_WS_ORIGIN_WEBFILES can NOT co-exist.
 * */
#if defined ENABLE_DATA_TXT_LOG && defined USE_WS_ORIGIN_WEBFILES
#error "ENABLE_DATA_TXT_LOG and USE_WS_ORIGIN_WEBFILES can NOT be defined together!"
#endif


/*
 * Invert valve control signal.
 * In GM's FANUC deployment, 0 means valve-open. WS_EIPS follow this rule.
 * */
#define WS_VALVE_CTRL_INVERTED

/*
 * Supposed that only EIPS version should have valve control signal inverted.
 * PNIO or DIDO version should NOT invert the signal.
 * */
#if defined WS_VALVE_CTRL_INVERTED && WS_FIELDBUS_TYPE != FIELDBUS_TYPE_EIPS
#warning "Valve control signal should only be inverted in EIPS version!"
#endif

/*
 * WS_process call period in unit of ms.
 * e.g. 5 means ws_process is called every 5 ms.
 * */
#define WS_PROCESS_RUN_PERIOD (5)
#define WS_DIG_LED_UPDATE_PERIOD    493

/*
 * Define to calculate the flowrate by counting directly the pulse number within a predefined time.
 * Currently it coexists with calculation by frequency.
 * */
//#define WS_CAL_FLOWRATE_BY_COUNT_PULSE

// todo: a mess for the 2 macros below. should be tuned.

/*
 * define to use official leak response adjustment on webserver.
 * if commented we can only use internal setting on webserver. should not be a official method after release.
 * */
#define DISABLE_INTERNAL_SENSE_CONFIG


/*
 * enable the use of isNewInternalSetting
 * then we could keep both internal para seetint and leak_resp setting.
 * */
#define RESERVE_INTERNAL_PARA_SETTING


/*
 * Uncomment the macro below to Enable the LSR (least square regression) to calculate the ramp of flowrate.
 * */
#define USE_LSR_FOR_LEAK_DETECT


/*
 * If XM4C129ENCPDT chip is used, uncomment the macro below.
 * */
//#define USE_XM4C129ENCPDT_CHIP


/*
 * Due to poor thinking-through of code structure, co-existence of ENABLE_DIDO_IN_WS and WS_FILEDBUS_NONE_BUT_DIDO is not allowed.
 * todo: should be further tuned.
 * */
#if  defined ENABLE_DIDO_IN_WS && defined WS_FILEDBUS_NONE_BUT_DIDO
#error "ENABLE_DIDO_IN_WS and WS_FILEDBUS_NONE_BUT_DIDO can NOT be defined together!"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SYS_CONFIG_H_ */
