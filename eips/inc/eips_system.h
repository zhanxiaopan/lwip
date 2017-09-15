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
 *     Module Name: eips_system.h
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains EtherNet/IP Server specific definitions.
 *
 */

#ifndef __EIPS_SYSTEM_H__
#define __EIPS_SYSTEM_H__

#define EIPS_UDP_BIDIR	0
#define EIPS_UDP_RXONLY 1
#define EIPS_UDP_TXONLY 2

#define EIP_CIP_PORT	44818
#define EIPS_UDPIO_PORT 2222

/* NOTE: There are only 5 service codes available to the user, the rest are
   handled by the standard code.  If Set_Attribute_Single is supported, then
   Get_Attribute_Single MUST be supported.  The same is NOT true with
   Set_Attribute_All and Get_Attribute_All. Each entry in the table can only
   have one mask value.  This makes processing easier. */
#define EIPS_SCMASK_GETALL 0x01
#define EIPS_SCMASK_SETALL 0x02
#define EIPS_SCMASK_GETSNG 0x04
#define EIPS_SCMASK_SETSNG 0x08
#define EIPS_SCMASK_RESET  0x10

/* DeviceNet Data Type */
#define EIPS_TYPE_UINT8  0
#define EIPS_TYPE_UINT16 1
#define EIPS_TYPE_UINT32 2
#define EIPS_TYPE_INT8	 3
#define EIPS_TYPE_INT16  4
#define EIPS_TYPE_INT32  5
#define EIPS_TYPE_FLOAT  6
#define EIPS_TYPE_UNUSED 7

#define EIPS_CPF_ADDR_ITEM     0
#define EIPS_CPF_DATA_ITEM     1
#define EIPS_CPF_OPT_ITEM      2 /* CIP doesn't use these (always 2) */
#define EIPS_CPF_SOCKO2T_ITEM  2
#define EIPS_CPF_SOCKT2O_ITEM  3
#define EIPS_CPF_MAX_ITEM_CNT  4

/* List Services Capability Flags (Encap 0x0004)
   ---------------------------------------------
   bits 0 - 4  : reserved
   bit 5       : 1 = CIP over TCP supported, else 0
   bits 6 - 7  : reserved
   bit 8       : 1 = Supports CIP Class 0/1 UDP (I/O)
   bits 9 - 15 : reserved
*/
#define EIP_LIST_SERVICES_CAP_FLAGS 0x0120

#define SOCKTYPE_TCP 0
#define SOCKTYPE_UDP 1

#define EIP_SESSION_STATE_UNUSED 0
#define EIP_SESSION_STATE_USED	 1

/*
   -------------------------------------------------------------------
   NORMAL INITIALIZATION (0)
   -------------------------------------------------------------------
   Emulate as closely as possible cycling power on the item the Object
   represents. This value is the default if this parameter is omitted.

   -------------------------------------------------------------------
   OUT OF BOX INITIALIZATION (1)
   -------------------------------------------------------------------
   Return as closely as possible to the out–of–box configuration, then
   emulate cycling power as closely as possible.
*/
#define EIPSINIT_NORMAL             0x00
#define EIPSINIT_OUTOFBOX           0x01
#define EIPSINIT_OUTOFBOX_NOCOMM    0x02
#define EIPCINIT_NORMAL   EIPSINIT_NORMAL
#define EIPCINIT_OUTOFBOX EIPSINIT_OUTOFBOX

/* Valid EIP Server States */
#define EIPS_STATE_RESETNORMAL	        0
#define EIPS_STATE_RESETOUTOFBOX        1
/* #define EIPS_STATE_RESETOUTOFBOX_NOCOMM 2 */
#define EIPS_STATE_RUNNING	            255

#define EIPS_TIMER_NULL   0xFFFF

/* RTA DEFINED MACROS */
#define RTA_MIN(a,b)        (a<b?a:b)
#define RTA_CIP_INST_TO_IX(a)   (a>0?(a-1):a)

/* --------------------------------------------------------------- */
/*	EIPS STANDARD INCLUDE FILES                                    */
/* --------------------------------------------------------------- */
#include "eips_usersys.h"
#include "eips_userobj.h"
#include "eips_usersock.h"
#include "eips_encap.h"
#include "eips_cpf.h"
#include "eips_struct.h"
#include "eips_cnxn.h"
#include "eips_proto.h"
#include "eips_revision.h"
#include "eips_iomsg.h"

#ifdef EIPC_CLIENT_USED
    #include "eipc_client.h"

    #ifdef EIPC_CCO_CLIENT_USED
        #include "eipc_cco.h"
    #endif
#endif

#if defined (EIPS_CIP_READ_WRITE_DATA_LOGIX) && defined (EIPS_CIP_READ_WRITE_DATA_LOGIX_ADVANCED)
    #error "Can't support EIPS_CIP_READ_WRITE_DATA_LOGIX and EIPS_CIP_READ_WRITE_DATA_LOGIX_ADVANCED"
#endif

#if defined (EIPS_CIP_READ_WRITE_DATA_LOGIX) || defined (EIPS_CIP_READ_WRITE_DATA_LOGIX_ADVANCED)
    #include "eips_tag.h"
#endif

extern uint32 free_running_ms;

/* --------------------------------------------------------------- */
/*	BEGIN: EIP TIMER DEFINITIONS (DON'T DEFINE MORE THAN WE NEED)  */
/* --------------------------------------------------------------- */

/*
EIPS_USER_MAX_NUM_TIMERS        -   User defined, default is 0
EIPS_CNXN_MAXTOTALCNXNS         -   1 per for Class 3 EM as a server/adapter
EIPS_USER_MAX_NUM_IO_CNXNS      -   2 per for Class 1 IO as a server/adapter 
EIPC_USERSYS_MAX_NUM_SERVERS    -   1 per for general use as a client/scanner 
EIPC_USERSYS_MAX_NUM_SERVERS    -   1 per for class 3 EM use as a client/scanner 
EIPC_IO_MAX_NUM_IO_SERVERS      -   2 per client/scanner for I/O
*/

/* conditionally add timers for optional client functionality */
#ifdef EIPC_CLIENT_USED
	#define RTA_MAX_NUM_CLIENT_TIMERS   EIPC_USERSYS_MAX_NUM_SERVERS
#else
	#define RTA_MAX_NUM_CLIENT_TIMERS   0
#endif

/* conditionally add timers for optional client explicit connection functionality */
#ifdef EIPC_EM_CLASS3_CNXN_USED
	#define RTA_MAX_NUM_EM_CLIENT_TIMERS    EIPC_USERSYS_MAX_NUM_SERVERS
#else
	#define RTA_MAX_NUM_EM_CLIENT_TIMERS    0
#endif

/* conditionally add timers for optional client I/O functionality */
#ifdef EIPC_IO_CLIENT_USED
	#define RTA_MAX_NUM_IO_CLIENT_TIMERS    (EIPC_IO_MAX_NUM_IO_SERVERS * 2)
#else    
	#define RTA_MAX_NUM_IO_CLIENT_TIMERS    0
#endif

/* make sure we have enough timers (add one since we are 1 based (0 is invalid)) */
#define RTA_MAX_NUM_TIMERS  (   1 + (EIPS_CNXN_MAXTOTALCNXNS)  \
								+ (EIPS_USER_MAX_NUM_IO_CNXNS*2) \
								+ EIPS_USER_MAX_NUM_TIMERS  \
								+ RTA_MAX_NUM_CLIENT_TIMERS \
								+ RTA_MAX_NUM_EM_CLIENT_TIMERS \
								+ RTA_MAX_NUM_IO_CLIENT_TIMERS )

/* --------------------------------------------------------------- */
/*	END: EIP TIMER DEFINITIONS                                     */
/* --------------------------------------------------------------- */

#endif /* __EIPS_SYSTEM_H__ */
