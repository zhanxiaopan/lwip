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
 *     Module Name: eips_cnxn.h
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains EtherNet/IP Server connection specific definitions.
 *
 */

#ifndef __EIPS_CNXN_H__
#define __EIPS_CNXN_H__

#define EIPS_CNXN_MAXTOTALCNXNS ((EIPS_USER_MAX_NUM_EIP_SESSIONS * EIPS_USER_MAX_NUM_EM_CNXNS_PER_SESSION)+EIPS_USER_MAX_NUM_IO_CNXNS)

/* define led states for the user */
#define EIPS_LEDSTATE_OFF            0
#define EIPS_LEDSTATE_FLASH_GREEN    1
#define EIPS_LEDSTATE_FLASH_RED      2
#define EIPS_LEDSTATE_STEADY_GREEN   3
#define EIPS_LEDSTATE_STEADY_RED     4
#define EIPS_LEDSTATE_ALT_REDGREEN   5

#define EIPS_CNXN_STATE_FREE 0
#define EIPS_CNXN_STATE_USED 1

/* the following is a valid EM transport class mask */
#define EIPS_XPORTCLASS_EM_ANDVAL 0x8F
#define EIPS_XPORTCLASS_EM_VALID  0x83

/* the following are valid I/O transport classes */
#define EIPS_XPORTCLASS_CYC_NOSEQ 0x00
#define EIPS_XPORTCLASS_CYC_SEQ   0x01
#define EIPS_XPORTCLASS_COS_NOSEQ 0x10
#define EIPS_XPORTCLASS_COS_SEQ   0x11
#define EIPS_XPORTCLASS_APP_NOSEQ 0x20
#define EIPS_XPORTCLASS_APP_SEQ   0x21

/* ***************************** */
/* Network Connection Parameters */
/* ***************************** */

/* Bit 15 -> Redundant Owner */
#define NTWKCNXNPRM_OWNER_ONE           0x0000
#define NTWKCNXNPRM_OWNER_MANY          0x8000
#define NTWKCNXNPRM_OWNER_ANDVAL        0x8000

/* Bits 13,14 -> Connection Type */
#define NTWKCNXNPRM_CNXNTYP_NULL        0x0000
#define NTWKCNXNPRM_CNXNTYP_MULTI       0x2000
#define NTWKCNXNPRM_CNXNTYP_P2P         0x4000
#define NTWKCNXNPRM_CNXNTYP_RSVD        0x6000
#define NTWKCNXNPRM_CNXNTYP_ANDVAL      0x6000

/* Bit 12 -> Reserved */

/* Bits 10,11 -> Priority */
#define NTWKCNXNPRM_PRIORITY_LOW        0x0000
#define NTWKCNXNPRM_PRIORITY_HIGH       0x0400
#define NTWKCNXNPRM_PRIORITY_SCHEDULE   0x0800
#define NTWKCNXNPRM_PRIORITY_URGENT     0x0C00
#define NTWKCNXNPRM_PRIORITY_ANDVAL     0x0C00

/* Bit 9 -> Variable Size */
#define NTWKCNXNPRM_FIXED_SIZ           0x0000
#define NTWKCNXNPRM_VAR_SIZ             0x0200
#define NTWKCNXNPRM_FIXED_ANDVAL        0x0200

/* Bits 0-8 -> Connection Size (in bytes) */
#define NTWKCNXNPRM_CNXN_SIZ_ANDVAL     0x01FF

/* ****************************************** */
/* Connection Manager Extended Response Codes */
/* ****************************************** */
#define CMERR_ALREADY_USED              0x100 /* Connection already in use */
#define CMERR_BAD_TRANSPORT             0x103 /* Transport type not supported */
#define CMERR_OWNER_CONFLICT            0x106 /* More than one guy configuring */
#define CMERR_BAD_CONNECTION            0x107 /* Trying to close inactive conn */
#define CMERR_BAD_CONN_TYPE            0x108 /* Unsupported connection type */
#if 0
DEPRECATED-> #define CMERR_BAD_CONN_SIZE        0x109 /* Connection size mismatch */
#endif
#define CMERR_CONN_UNCONFIGURED         0x110 /* Connection unconfigured */
#define CMERR_BAD_RPI                   0x111 /* Unsupportable RPI */
#define CMERR_RPI_VAL_NOT_ACCEPT        0x112 /* RPI Value(s) not acceptable */
#define CMERR_NO_CM_RESOURCES           0x113 /* Conn Mgr out of connections */
#define CMERR_BAD_VENDOR_PRODUCT        0x114 /* Mismatch in electronic key (vendor id / product code) */
#define CMERR_BAD_DEVICE_TYPE           0x115 /* Mismatch in electronic key (product type) */
#define CMERR_BAD_REVISION              0x116 /* Mismatch in electronic key (revision mismatch) */
#if 0
DEPRECATED-> #define CMERR_BAD_CONN_POINT       0x117 /* Nonexistant instance number */
DEPRECATED-> #define CMERR_BAD_CONFIGURATION    0x118 /* Bad config instance number */
#endif
#define CMERR_CONN_REQ_FAILS            0x119 /* No controlling connection open */
#define CMERR_NO_APP_RESOURCES          0x11A /* App out of connections */
#define CMERR_RPI_LESS_PRODINHIB        0x11B /* RPI is smaller than ProdInhibTimer */
#define CMERR_XPORT_CLASS_NOT_SUPRTED   0x11C /* Transport Class Not Supported */
#define CMERR_PROD_TRIG_NOT_SUPPORTED   0x11D /* Production Trigger Not Supported */
#define CMERR_DIRECTION_NOT_SUPPORTED   0x11E /* Direction Not Supported */
#define CMERR_O2T_FIXVAR_NOT_SUPPORTED  0x11F /* O2T Fixed/Variable Flag Not Supported */
#define CMERR_T2O_FIXVAR_NOT_SUPPORTED  0x120 /* T2O Fixed/Variable Flag Not Supported */
#define CMERR_O2T_PRI_NOT_SUPPORTED     0x121 /* O2T Priority Not Supported */
#define CMERR_T2O_PRI_NOT_SUPPORTED     0x122 /* T2O Priority Not Supported */
#define CMERR_O2T_CNXNTYP_NOT_SUPPORTED 0x123 /* O2T Connection Type Not Supported */
#define CMERR_T2O_CNXNTYP_NOT_SUPPORTED 0x124 /* T2O Connection Type Not Supported */
#define CMERR_REDUND_OWN_NOT_SUPPORTED  0x125 /* Redundant Owner Flag Not Supported */

#define CMERR_INV_CNXN_SIZE             0x126 /* Invalid Configuration Size (ext stat with valid value) */
#define CMERR_INV_O2T_SIZE              0x127 /* Invalid O2T Size (ext stat with valid value) */
#define CMERR_INV_T2O_SIZE              0x128 /* Invalid T2O Size (ext stat with valid value) */

#define CMERR_INV_CNXN_PATH             0x129 /* Invalid Configuration Path */
#define CMERR_INV_O2T_PATH              0x12A /* Invalid O2T / Consume Path */
#define CMERR_INV_T2O_PATH              0x12B /* Invalid T2O / Produce Path */

#define CMERR_CONN_TIMED_OUT            0x203 /* Using a timed out connection */
#define CMERR_UCSEND_TIMED_OUT          0x204 /* Unconnected Send timed out */
#define CMERR_UCSEND_PARAM_ERR          0x205 /* Unconnected Send parameter error */
#define CMERR_MSGTOOLARGE4UCMM          0x206 /* Message too large for UCMM */
#define CMERR_UCMMACKNOREPLY            0x207 /* UCMM Ack without reply */
#define CMERR_NO_BUFFER                 0x301 /* No buffer memory available */
#define CMERR_NO_BANDWIDTH              0x302 /* Insufficient bandwidth left */
#define CMERR_NO_SCREENER               0x303 /* Out of gen screeners */
#define CMERR_NOT_CONF_FOR_RT           0x304 /* Not configured to send RT data*/
#define CMERR_SIGNATURE_MISMATCH        0x305 /* sig does not match sig store in CCM */
#define CMERR_CCM_NOT_AVAIL             0x306 /* ccm is not responding to requests */
#define CMERR_BAD_PORT                  0x311 /* Nonexistant port */
#define CMERR_BAD_LINK_ADDRESS          0x312 /* Invalid link address in path */
#define CMERR_BAD_SEGMENT               0x315 /* Invalid segment in path */
#define CMERR_BAD_CLOSE                 0x316 /* Path & conn not equal in close */
#define CMERR_BAD_NET_SEGMENT           0x317 /* Net seg not present or bad */
#define CMERR_BAD_LINK_AD_TO_SELF       0x318 /* Link address to self invalid */
#define CMERR_NO_SEC_RESOURCE           0x319 /* Resources in secondary unavail */
#define CMERR_RACKCNXN_ALREADY_ESTAB    0x31A /* Rack connection already established */
#define CMERR_MODULECNXN_ALREADY_ESTAB  0x31B /* Module connection already established */
#define CMERR_MISC_CATCHALL             0x31C /* miscellaneous (catch all) */
#define CMERR_REDUND_CONN_MISMATCH      0x31D /* Redundant connection mismatch */

#endif /* __EIPS_CNXN_H__ */
