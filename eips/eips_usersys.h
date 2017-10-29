/*
 *            Copyright (c) 2002-2013 by Real Time Automation, Inc.
 *
 *  This software is copyrighted by and is the sole property of
 *  Real Time Automation, Inc. (RTA).  All rights, title, ownership, 
 *  or other interests in the software remain the property of RTA.  
 *  This software may only be used in accordance with the corresponding
 *  license agreement.  Any unauthorized use, duplication, transmission,
 *  distribution, or disclosure of this software is expressly forbidden.
 *
 *  This Copyright notice MAY NOT be removed or modified without prior
 *  written consent of RTA.
 *
 *  RTA reserves the right to modify this software without notice.
 *
 *  Real Time Automation
 *  150 S. Sunny Slope Road            USA 262.439.4999
 *  Suite 130                          www.rtaautomation.com
 *  Brookfield, WI 53005               software@rtaautomation.com
 *
 *************************************************************************
 *
 *    Version Date: 05NOV2013
 *         Version: 2.37
 *    Conformed To: EtherNet/IP Protocol Conformance Test A-10 (28-AUG-2012)
 *     Module Name: eips_usersys.h
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains user definitions specific to EtherNet/IP General
 * behavior.
 *
 * Change log
 *   - remove the including of winsock2.h
 *
 */

#ifndef __EIPS_USERSYS_H__
#define __EIPS_USERSYS_H__

/* ------------------------------------------- */
/*             RTA "ADD ON" SUPPORT            */
/* ------------------------------------------- */
#define _CRT_SECURE_NO_WARNINGS /* we don't need to use safe sscanf() in Visual C++ */

/* --------------------------------- */
/* define if EIP Client is supported */
/* --------------------------------- */
/* #define EIPC_CLIENT_USED 1 */

/* ------------------------------------------------- */
/* define if Module/Network Status LED is supported  */
/* ------------------------------------------------- */
/* #define EIPS_MODNET_LED_USED 1 */

/* -------------------------------------------------------------------------------- */
/* define if Module Status LED is supported (can't use if Mod/Net LED is supported) */
/* -------------------------------------------------------------------------------- */
/* #define EIPS_MOD_LED_USED 1 */

/* --------------------------------------------------------------------------------- */
/* define if Network Status LED is supported (can't use if Mod/Net LED is supported) */
/* --------------------------------------------------------------------------------- */
#define EIPS_NTWK_LED_USED 1

/* ------------------------------------- */
/* define if I/O Status LED is supported */
/* ------------------------------------- */
//#define EIPS_IO_LED_USED 1

/* --------------------------------------------------------------------- */
/*  define if Quality of Service (QoS) is enabled. The RTA stack exposes
    the parameters and services. The customer is responsible for the
    actual tagging of the packets */
/* --------------------------------------------------------------------- */
/* #define EIPS_QOS_USED 1 */

/* --------------------------------------------------------------- */
/* define to support a 32 character name for Timer subsystem debug */
/* --------------------------------------------------------------- */
/* #define EIPS_TIMER_NAME_USED 1 */

/* ---------------------------------------------------- */
/* define if PCCC Server Emulation (PLC5E) is supported */
/* ---------------------------------------------------- */
/* #define EIPS_PCCC_USED      1 */

/* --------------------------------------------------------------- */
/*  define to support RA Tag Read/Write Data Services as a server
    RTA stack supports:
        RA Read Tag Service  (0x4C) - "RA_SC_RD_TAG_REQ_REQ"
        RA Write Tag Service (0x4D) - "RA_SC_WR_TAG_REQ_REQ"
NOTE: This is the original version with limited data type support. */
/* --------------------------------------------------------------- */
/* #define EIPS_CIP_READ_WRITE_DATA_LOGIX  1 */

/* --------------------------------------------------------------- */
/*  define to support RA Tag Fragmented Read/Write Services as a server
    RTA stack supports:
        RA Read Tag Service  (0x4C)      - "RA_SC_RD_TAG_REQ_REQ"
        RA Write Tag Service (0x4D)      - "RA_SC_WR_TAG_REQ_REQ"
        RA Read Frag Tag Service  (0x52) - "RA_SC_RD_FRAG_TAG_REQ_REQ"
        RA Write Frag Tag Service (0x53) - "RA_SC_WR_FRAG_TAG_REQ_REQ"
NOTE: This method requires the user to format ALL data in little
      endian byte ordering since UDTs are supported. */
/* --------------------------------------------------------------- */
/* #define EIPS_CIP_READ_WRITE_DATA_LOGIX_ADVANCED  1 */

/* --------------------------------------------------------------- */
/* define a CPU slot if we emulate the Logix processor (usually 0) */
/* --------------------------------------------------------------- */
/* #define EIPS_USERSYS_OUR_SLOT_ID 0 */

/* ------------------------------------------------ */
/* define if a hardware/watchdog reset is supported */
/* ------------------------------------------------ */
#define EIPS_USERSYS_HWRESET_SUPPORTED  0

/* *********************************************** */
/* include files needed for the sample application */
/* *********************************************** */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "bsp_config.h"					// bsp_CONFIG Header file

/* --------------------------------------------------------------- */
/*      TYPE DEFINITIONS                                           */
/* --------------------------------------------------------------- */
#ifndef int8
  #define int8   signed char
#endif

#ifndef uint8
  #define uint8  unsigned char
#endif

#ifndef int16
  #define int16  short
#endif

#ifndef uint16
  #define uint16 unsigned short
#endif

#ifndef int32
  #define int32  long
#endif

#ifndef uint32
  #define uint32 unsigned long
#endif

#ifndef NULLFNPTR
  #define NULLFNPTR ((void (*)()) 0)
#endif

/* --------------------------------------------------------------- */
/*      GENERIC DEFINITIONS                                        */
/* --------------------------------------------------------------- */
#ifndef SUCCESS
  #define SUCCESS 0
#endif

#ifndef FAILURE
  #define FAILURE 1
#endif

#ifndef FAIL
  #define FAIL 1
#endif

#ifndef TRUE
  #define TRUE 1
#endif

#ifndef FALSE
  #define FALSE 0
#endif

#ifndef YES
  #define YES 1
#endif

#ifndef NO
  #define NO 0
#endif

#ifndef ON
  #define ON 1
#endif

#ifndef OFF
  #define OFF 0
#endif

#ifndef NULL
  #define NULL 0x00
#endif

#ifndef NULLPTR
  #define NULLPTR NULL
#endif

/* we put lookup tables in code space to save RAM */
#define EIPS_CODESPACE const
#define RTA_CODESPACE EIPS_CODESPACE

/* --------------------------------------------------------------- */
/*      MACROS                                                     */
/* --------------------------------------------------------------- */
#define Xmod(v,b) (v - ((v/b)*b))
#define RTA_UNUSED_PARAM(a) if(a){}

#define EIPS_GET_MUTEX
#define EIPS_PUT_MUTEX

/* define macros to get the high and low bytes of a word */
#define EIPS_LO(l) (uint8)(l&0x00FF)
#define EIPS_HI(h) (uint8)((h&0xFF00)>>8)

#define RTA_MAX(a,b) (a>b?a:b)
#define RTA_MIN(a,b) (a<b?a:b)

/* Define the number of path words included in a single
   explicit message.  This should be at least 10. */
#define EIPS_USER_MSGRTR_PATH_WORDS    50

/* Define the number of extended error words included in
   a single explicit message.  This should be at least 2. */
#define EIPS_USER_MSGRTR_EXTSTAT_SIZE  16

/* Define the max number of I/O bytes that can be transmitted in
   each direction. */
#define EIPS_USER_IOBUF_SIZ 480

/* Define the amount of data that can sent in a single explicit message. */
#define EIPS_USER_MSGRTR_DATA_SIZE  (RTA_MAX((EIPS_USER_IOBUF_SIZ+25),600)) /* this number should the larger of the max I/O and max EM, plus a few bytes for overhead */

/* Define the max size of the encapsulated message.  This should
   be at least 600, but can go up to 64K based on the needs of
   the customer.  The NOP command is the only command that is
   supported that can be larger than 600 bytes, but the stack
   handles the NOP in a special fashion to save on RAM. */
#define EIPS_USER_MAX_ENCAP_BUFFER_SIZ (EIPS_USER_MSGRTR_DATA_SIZE+50)

/* Define the max number of I/O connections supported.	This
   can be 0 to (EIPS_USER_MAX_NUM_EM_CNXNS_PER_SESSION *
   EIPS_USER_MAX_NUM_EIP_SESSIONS). */
#define EIPS_USER_MAX_NUM_IO_CNXNS 10

/* Define the max number of Encapsulation Sessions supported
   by the device.  This should be smaller than the total
   number of sockets supported. */
#define EIPS_USER_MAX_NUM_EIP_SESSIONS 10

/* Define the max number of Explicit connections
   supported by the server. */
#define EIPS_USER_MAX_NUM_EM_CNXNS_PER_SESSION 2

/* define the resolution of the ticker in in microseconds.
	    1L - one microsecond ticker
	 1000L - one millisecond ticker
	10000L - ten millisecond ticker
      1000000L - one second ticker
*/
#ifdef CLOCKS_PER_SEC
#undef CLOCKS_PER_SEC
#endif
#define CLOCKS_PER_SEC 1000
#define EIPS_USER_MIN_TICK_RES_IN_USEC	1000L		/*  1ms min */
#define EIPS_USER_TICK_RES_IN_USECS (1000000L/CLOCKS_PER_SEC) /*  us in 1 tick */
#define EIPS_USER_TICKS_PER_1SEC    CLOCKS_PER_SEC     /*  ticks per second */
#define EIPS_USER_TICKS_PER_10SEC   (10*EIPS_USER_TICKS_PER_1SEC)

#define EIPS_USER_TCP_INACTIVITY_TMO (EIPS_USER_TICKS_PER_1SEC*5)

/* RTA depends on a timer subsystem. This is the optional number of timers the user code needs.*/
#define EIPS_USER_MAX_NUM_TIMERS    10

/* we need a semaphore to ensure we don't print too fast */
#define EIPS_DEBUG		0	/* 1: enable debug printing */
#define EIPS_PRINTALL	0	/* 0: just user printing (eips_user_printf)
							   1: assembly configuration print at start up
							   2: show all TCP and UDP traffic */

#if EIPS_DEBUG > 0
	#define eips_user_printf(f) printf(f)
	#define eips_user_dbprint0(f) printf(f)
	#define eips_user_dbprint1(f,a1) printf(f,a1)
	#define eips_user_dbprint2(f,a1,a2) printf(f,a1,a2)
	#define eips_user_dbprint3(f,a1,a2,a3) printf(f,a1,a2,a3)
	#define eips_user_dbprint4(f,a1,a2,a3,a4) printf(f,a1,a2,a3,a4)
	#define eips_user_dbprint5(f,a1,a2,a3,a4,a5) printf(f,a1,a2,a3,a4,a5)
	#define eips_user_dbprint6(f,a1,a2,a3,a4,a5,a6) printf(f,a1,a2,a3,a4,a5,a6)
#else
	#define eips_user_printf(f, ...)
	#define eips_user_dbprint0(f)
	#define eips_user_dbprint1(f,a1)
	#define eips_user_dbprint2(f,a1,a2)
	#define eips_user_dbprint3(f,a1,a2,a3)
	#define eips_user_dbprint4(f,a1,a2,a3,a4)
	#define eips_user_dbprint5(f,a1,a2,a3,a4,a5)
	#define eips_user_dbprint6(f,a1,a2,a3,a4,a5,a6)
#endif

/* debug print with the file and line number */
/* #define RTA_MARK_CODE_PATH(a)    eips_user_dbprint3("%s %d: %s\r\n", __RTA_FILE__,__LINE__,a) */
#define RTA_MARK_CODE_PATH(a)

/* #define RTA_MARK_TCPCODE_PATH(a) eips_user_dbprint3("%s %d: %s\r\n", __RTA_FILE__,__LINE__,a) */
#define RTA_MARK_TCPCODE_PATH(a)
#define RTA_TCP_CLOSE_WITH_PRINT(a)     {RTA_MARK_TCPCODE_PATH("TCP CLOSE"); eips_usersock_tcpClose(a);}

#endif /* __EIPS_USERSYS_H__ */
