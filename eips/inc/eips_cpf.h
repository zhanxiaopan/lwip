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
 *     Module Name: eips_cpf.h
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains EtherNet/IP Server "Common Packet Format"
 * definitions.
 *
 */

#ifndef __EIPS_CPF_H__
#define __EIPS_CPF_H__

#ifdef EIPS_PCCC_USED
    #define PCCC_CMD       0x0F
    #define PCCC_FNC_READ  0x68
    #define PCCC_FNC_WRITE 0x67

    /* STS Error Codes */
    #define STSERR_SUCCESS 0x00  /* SUCCESS */
    #define STSERR_ILLCMD  0x10  /* Illegal command or format (only command is 0x0F) */
    #define STSERR_EXTSTS  0xF0  /* Error code in the EXT STS byte */

    /* EXT STS Error Codes */
    #define EXTSTS_NONE	           0x00  /* No additional error */
    #define EXTSTS_FIELDILLVAL     0x01  /* A field has an illegal value */
    #define EXTSTS_TOOFEWLEVELS    0x02  /* Less levels specified in address than min for any address */
    #define EXTSTS_TOOMANYLEVELS   0x03  /* More levels specified in address than system supports */
    #define EXTSTS_SYMBOLNOTFOUND  0x04  /* Symbol not found */
    #define EXTSTS_SYMBOLBADFORMAT 0x05  /* Symbol is of improper format */
    #define EXTSTS_BADADDRESS      0x06  /* Address doesn't point to something useful */
    #define EXTSTS_DATAFILETOOLRG  0x09  /* Data or file is too large */
    #define EXTSTS_TRANSSIZETOOLRG 0x0A  /* Transaction size plus word address is too large */
    #define EXTSTS_REGISRO	       0x0B  /* Access denied, improper privilege (WR to RO Reg) */
#endif

#define LINKOBJ_PROD		0x00
#define LINKOBJ_CONS		0x01

#define LINKOBJSTATE_NONEXIST	0x00
#define LINKOBJSTATE_RUNNING	0x01

#define CNXNST_UNCONNECTED	0x00
#define CNXNST_CONNECTED	0x01

#define EIPS_CPF_MAX_ITEM_DATA_LEN  EIPS_USER_MSGRTR_DATA_SIZE
#define EIPS_CPF_MAX_RSP_SIZ	    EIPS_USER_MAX_ENCAP_BUFFER_SIZ

#define LOGSEG_8BITCLASS	    0x20
#define LOGSEG_16BITCLASS       0x21
#define LOGSEG_32BITCLASS       0x22
#define LOGSEG_RSVDCLASS        0x23
#define LOGSEG_8BITINST 	    0x24
#define LOGSEG_16BITINST 	    0x25
#define LOGSEG_32BITINST 	    0x26
#define LOGSEG_RSVDBITINST 	    0x27
#define LOGSEG_8BITMEMBER       0x28
#define LOGSEG_16BITMEMBER      0x29
#define LOGSEG_32BITMEMBER      0x2A
#define LOGSEG_RSVDBITMEMBER    0x2B
#define LOGSEG_8BITCNXNPT	    0x2C
#define LOGSEG_16BITCNXNPT	    0x2D
#define LOGSEG_32BITCNXNPT      0x2E
#define LOGSEG_RSVDBITCNXNPT    0x2F
#define LOGSEG_8BITATTR 	    0x30
#define LOGSEG_16BITATTR 	    0x31
#define LOGSEG_32BITATTR 	    0x32
#define LOGSEG_RSVDBITATTR      0x33
#define LOGSEG_ELECKEY		    0x34
#define ELECKEY_FORMAT		    0x04
#define NETSEG_PRODINHIBIT      0x43    /* Production Inhibit Time Network Segment (1 byte to follow in ms) */
#define DATASEG_SIMPLE          0x80    /* Simple Data Segment (1 byte to follow - len in words), then word data */
#define ANSIEXT_SYMSEG          0x91    /* ANSI Extended Symbol Segment (1 byte to follow - len if bytes), then ASCII string */

/* ---------------------------- */
/* MISCELLANEOUS / MACROS	*/
/* ---------------------------- */
/*
 * TCP sequence numbers are 32 bit integers operated
 * on with modular arithmetric.  These macros can be
 * used to compare such integers.
 */

#define SEQ_LT (a,b)	((int)((a)-(b)) <  0)
#define SEQ_LEQ(a,b)	((int)((a)-(b)) <= 0)
#define SEQ_GT (a,b)	((int)((a)-(b)) >  0)
#define SEQ_GEQ(a,b)	((int)((a)-(b)) >= 0)

#define SEQ32_LT(a,b)	(((int32)((uint32)a - (uint32)b)) <  0)
#define SEQ32_LEQ(a,b)	(((int32)((uint32)a - (uint32)b)) <= 0)
#define SEQ32_GT(a,b)	(((int32)((uint32)a - (uint32)b)) >  0)
#define SEQ32_GEQ(a,b)	(((int32)((uint32)a - (uint32)b)) >= 0)

#define SEQ16_LT(a,b)	(((int16)((uint16)a - (uint16)b)) <  0)
#define SEQ16_LEQ(a,b)	(((int16)((uint16)a - (uint16)b)) <= 0)
#define SEQ16_GT(a,b)	(((int16)((uint16)a - (uint16)b)) >  0)
#define SEQ16_GEQ(a,b)	(((int16)((uint16)a - (uint16)b)) >= 0)

/*
 * The bulk of the information I found was in the ControlNet
 * International Specification, Version 2.0, Errata 2,
 * Part 4 "Network and Transport Layer", Section 8
 * "TCP/IP Encapsulation".
 *
 * I Receive the C.P.F. part of the encapsulated Ethernet/IP
 * message (Part 4, Section 8.9 "Common packet format").
 *
 * (Table 4-62: Common packet format)
 *  --------------------------------------------------------------------
 * |  FIELD NAME  |	   TYPE        |	 DESCRIPTION		|
 * |--------------------------------------------------------------------|
 * | Item Count   | uint16	       | # of items to follow (>1)	|
 * |--------------------------------------------------------------------|
 * | Address Item | Struct (see below) | Address info for encap. packet |
 * |--------------------------------------------------------------------|
 * | Data Item	  | Struct (see below) | The encap. data packet 	|
 * |--------------------------------------------------------------------|
 * | Optional additional items						|
 *  --------------------------------------------------------------------
 *
 * (Table 4-63: Address and data item structure)
 *  -------------------------------------------------------------
 * |  FIELD NAME  |   TYPE   |		DESCRIPTION		 |
 * |-------------------------------------------------------------|
 * | Type ID	  | uint16   | Type of item encapsulated	 |
 * |-------------------------------------------------------------|
 * | Length	  | uint16   | Length in bytes of data to follow |
 * |-------------------------------------------------------------|
 * | Data	  | Variable | The data (if Length > 0) 	 |
 *  -------------------------------------------------------------
 */

/* ***************************************************** */
/* ADDRESS TYPE IDS (From Table 4-64: Address type ID's) */
/* ***************************************************** */
#define ADDR_TYP_ID_NULL	0x0000	/* NULL (used for UCMM messages) */
/* 0x0001 - 0x00A0 -> Allocated for compatiblity with existing protocols) */
#define ADDR_TYP_ID_CNXNBASED	0x00A1	/* Connection-based (used for connected msgs) */
/* 0x00A2 - 0x7FFF -> Allocated for compatiblity with existing protocols) */
#define ADDR_TYP_ID_ENBT    0x0085  /* ENBT IP Address as String */
/* 0x8000 - 0x8001 -> Area for future expansion */
#define ADDR_TYP_ID_SEQADDRTYP	0x8002	/* Sequenced Address Type */
/* 0x8003 - 0xFFFF -> Area for future expansion */

/* *********************************************** */
/* DATA TYPE IDS (From Table 4-65: Data type ID's) */
/* *********************************************** */
/* 0x0000 - 0x00B0 -> Allocated for compatiblity with existing protocols) */
#define DATA_TYP_ID_PDU 	0x00B1	/* Connected Transport PDU */
#define DATA_TYP_ID_UCMM	0x00B2	/* Unconnected Message */
/* 0x00B3 - 0x7FFF -> Allocated for compatiblity with existing protocols) */
#define DATA_TYP_ID_ENBT    0x0091  /* ENBT DF1 String */
#define DATA_TYP_ID_SOCK_OT	0x8000	/* Sockaddr Info. originator-to-target */
#define DATA_TYP_ID_SOCK_TO	0x8001	/* Sockaddr Info. target-to-originator */
/* 0x8002 - 0xFFFF -> Area for future expansion */

/* -------------- 8-Bit RANGE DEFINITIONS ------------------------------ */
/* Class (pass to user) */
#define CLASSRANGE8_VENDSPEC_MIN 0x64
#define CLASSRANGE8_VENDSPEC_MAX 0xC7

/* Attribute (if class is supported, pass to user) */
#define ATTRRANGE8_VENDSPEC_MIN 0x64
#define ATTRRANGE8_VENDSPEC_MAX 0xC7

/* Service Code (pass to user) */
#define SERVICERANGE8_VENDSPEC_MIN 0x32
#define SERVICERANGE8_VENDSPEC_MAX 0x4A

/* -------------CLASS DEFINITIONS--------------------------------------- */
#define EIPS_IDENTITY_CLASS_REV  	1
#define EIPS_ASSEMBLY_CLASS_REV  	2
#define EIPS_TCP_CLASS_REV	     	3
#define EIPS_ENETLINK_CLASS_REV  	3

#ifdef EIPS_QOS_USED
    #define EIPS_QOS_CLASS_REV   	1
#endif

#ifdef EIPS_TACL_OBJ_USED
	#define EIPS_TACL_CLASS_REV		1
#endif

/* -------------GENERIC OBJECT CLASSES--------------------------------- */
#define CLASS_IDENTITY			0x01
#define CLASS_MSGROUTER 		0x02
#define CLASS_ASSEMBLY			0x04
#define CLASS_CONNECTMGR		0x06
#define CLASS_DIP               0x08
#define CLASS_DOP               0x09
#define CLASS_QOS               0x48
#define CLASS_TCPIP			    0xF5
#define CLASS_ENETLINK			0xF6

/* -------------COMMON SERVICE CODES----------------------------------- */
#define CIP_SC_GET_ATTR_ALL        	0x01
#define CIP_SC_SET_ATTR_ALL        	0x02
#define CIP_SC_RESET			   	0x05
#define CIP_SC_CREATE			   	0x08
#define CIP_SC_DELETE			   	0x09
#define CIP_SC_MULTI_SERV_PACKET   	0x0A
#define CIP_SC_APPLY_ATTRIBS       	0x0D
#define CIP_SC_GET_ATTR_SINGLE     	0x0E
#define CIP_SC_SET_ATTR_SINGLE     	0x10
#define CIP_SC_RESTORE			   	0x15
#define CIP_SC_ERROR_RESPONSE      	0x94

#ifdef EIPS_TACL_OBJ_USED
	#define CLASS_TACL				0xA0	/* this may change */
	#define CIP_SC_TACL_CNXN_READ	0x4C
#endif

#ifdef EIPS_PCCC_USED
   #define CLASS_PCCC          		0x67
   #define RA_SC_EXECUTE_PCCC		0x4B
#endif

/* Connection Manager Services (PDU) */
#define CIP_SC_FWD_OPEN_REQ         0x54    /* Open	a connection (0-511 bytes) */
#define CIP_SC_FWD_OPEN_RSP         0xD4    /* Open Response */

#define CIP_SC_LRG_FWD_OPEN_REQ     0x5B    /* Open	a large connection (512-65535 bytes) */
#define CIP_SC_LRG_FWD_OPEN_RSP     0xDB    /* Large Open Response */

#define CIP_SC_FWD_CLOSE_REQ        0x4E    /* Close a connection */
#define CIP_SC_FWD_CLOSE_RSP        0xCE    /* Close Response */

/* Unconnected Send */
#define CIP_SC_UNCON_SEND           0x52

/* Rockwell Automation Tag Read/Write */
#define RA_SC_RD_TAG_REQ_REQ            0x4C
#define RA_SC_RD_FRAG_TAG_REQ_REQ       0x52
#define RA_SC_WR_TAG_REQ_REQ            0x4D
#define RA_SC_WR_FRAG_TAG_REQ_REQ       0x53
#define RA_SC_RD_MOD_WR_TAG_REQ_REQ     0x4E

#define RA_SC_RD_TAG_REQ_RSP            0xCC
#define RA_SC_RD_FRAG_TAG_REQ_RSP       0xD2
#define RA_SC_WR_TAG_REQ_RSP            0xCD
#define RA_SC_WR_FRAG_TAG_REQ_RSP       0xD3
#define RA_SC_RD_MOD_WR_TAG_REQ_RSP     0xCE

/* TCP Object Configuration Capabilities attribute */
#define EIPS_TCPCFG_BOOTPCLIENT_SUPPORTED    0x00000001L
#define EIPS_TCPCFG_DNSCLIENT_SUPPORTED      0x00000002L
#define EIPS_TCPCFG_DHCPCLIENT_SUPPORTED     0x00000004L
#define EIPS_TCPCFG_DHCPDNS_SUPPORTED	     0x00000008L
#define EIPS_TCPCFG_CONFIGSETTABLE_SUPPORTED 0x00000010L

/* -------------ERROR CODES-------------------------------------------- */
#define ERR_SUCCESS             0x00
#define ERR_CNXN_FAILURE        0x01
#define ERR_RESOURCE_UNAVAIL    0x02
#define ERR_INV_PARAMNAME       0x03
#define ERR_PATHSEGMENT         0x04
#define ERR_PATHDESTUNKNOWN     0x05
#define ERR_PARTIALXFER         0x06
#define ERR_CNXNLOST            0x07
#define ERR_SERV_UNSUPP         0x08
#define ERR_INV_ATTRIBVAL       0x09
#define ERR_ATTR_LIST_ERR       0x0A
#define ERR_IN_REQ_STATE        0x0B
#define ERR_OBJ_STATE_CONFLICT  0x0C
#define ERR_OBJ_ALREADY_EXISTS  0x0D
#define ERR_ATTR_READONLY       0x0E
#define ERR_PRIV_VIOLATION      0x0F
#define ERR_DEV_STATE_CONFLICT  0x10
#define ERR_REPLY_SIZE          0x11
#define ERR_FRAG_PRIM_VAL       0x12
#define ERR_INSUFF_DATA         0x13
#define ERR_ATTR_UNSUPP         0x14
#define ERR_TOOMUCH_DATA        0x15
#define ERR_UNEXISTANT_OBJ      0x16
#define ERR_SERV_FRAG_SEQ       0x17
#define ERR_NO_ATTR_DATA        0x18
#define ERR_STORE_FAILURE       0x19
#define ERR_ROUTE_REQ_TOO_LRG   0x1A
#define ERR_ROUTE_RSP_TOO_LRG   0x1B
#define ERR_MISSING_ATTR_LIST   0x1C
#define ERR_INV_ATTR_LIST       0x1D
#define ERR_EMBEDDED_SERV_ERR   0x1E
#define ERR_VENDOR_SPECIFIC     0x1F
#define ERR_INV_SERVICE_PARM    0x20
#define ERR_WO_VAL_ALREADY_W    0x21
#define ERR_INV_REP_RECV        0x22
#define ERR_BUFFER_OVERFLOW     0x23
#define ERR_MSG_FORMAT_ERR      0x24
#define ERR_KEY_ERR_IN_PATH     0x25
#define ERR_PATH_SIZE_INV       0x26
#define ERR_UNEXP_ATTR_IN_LIST  0x27
#define ERR_INV_MEMBER_ID       0x28
#define ERR_MEMBER_READONLY     0x29
#define ERR_G2ONLY_GEN_ERR      0x2A
#define ERR_UNKNOWN_MB_ERR      0x2B
#define ERR_ATTR_NOT_GETTABLE   0x2C

#define ERR_NOADD_ERRCODE		0xFF

/* -------------VENDOR ERROR CONDITIONS-------------------------------- */
#define ERRV_UNDEF_SERV 		0x01

/* -------------LED FLASH RATE----------------------------------------- */
#define LED_FLASH_RATE	TICKS_PER_50MSEC  /* (50 * 10msec) = 500msec */

/* -------------MESSAGE CONTROL BITS----------------------------------- */
#define DNBIT_RR	  0x80
#define DNBIT_FRAG	  0x80
#define DNBIT_XID	  0x40
#define DNBIT_XPORT_DIR   0x80
#define DNBITS_FRAGCNT	  0x3F

/* -------------FRAGMENT PROCESSING------------------------------------ */
#define FRAGTYP_1ST	0
#define FRAGTYP_MID	1
#define FRAGTYP_LAST	2
#define FRAGTYP_ACK	3
#define FRAGCNT_END	0x3F

/* -------------CONNECTION STATES-------------------------------------- */
#define CXST_NONEXIST	  0x00
#define CXST_CONFIGURING  0x01
#define CXST_WAIT4CID	  0x02
#define CXST_ESTABLISHED  0x03
#define CXST_TIMEOUT	  0x04
#define CXST_DEFERDELETE  0x05

/* -------------INSTANCE TYPES----------------------------------------- */
#define INST_TYP_EM	0x00
#define INST_TYP_IO	0x01

/* -------------MESSAGE BODY FORMAT------------------------------------ */
#define BDYFMT_C8I8	0x00		/* Class = 8, Instance = 8	*/
#define BDYFMT_C8I6	0x01		/* Class = 8, Instance = 16	*/
#define BDYFMT_C6I6	0x02		/* Class = 16, Instance = 16	*/
#define BDYFMT_C6I8	0x03		/* Class = 16, Instance = 8	*/

/* -------------INIT PROD CHARS---------------------------------------- */
#define PRODGRP1	0x00
#define PRODGRP2DEST	0x01
#define PRODGRP2SRC	0x02
#define PRODGRP3	0x03
#define PRODDEFAULT	0x0F

/* -------------INIT CONS CHARS---------------------------------------- */
#define CONSGRP1	0x00
#define CONSGRP2DEST	0x01
#define CONSGRP2SRC	0x02
#define CONSGRP3	0x03
#define CONSDEFAULT	0x0F

/* -------------MAX PROD SIZE and MAX CONS SIZE------------------------ */
/*** The values for the Max Prod Size and Max Cons Size are listed in pw.h ***/

/* -------------TRANSPORT DIRECTIONS----------------------------------- */
#define XPORTDIR_CLIENT 	0x00
#define XPORTDIR_SERVER 	0x80

/* -------------TIMEOUT ACTION DEF------------------------------------- */
#define TMOACTION_TMO		0x00
#define TMOACTION_DELETE	0x01
#define TMOACTION_AUTORESET	0x02
#define TMOACTION_DEFERDELETE	0x03

/* ************************************ */
/* IDENTITY STATUS WORD BIT DEFINITIONS */
/* ************************************ */
#define IDSTATWORD_OWNED	0x0001
#define IDSTATWORD_RSVD1	0x0002
#define IDSTATWORD_CONFIGURED	0x0004
#define IDSTATWORD_RSVD2	0x0008
#define IDSTATWORD_VENDOR1	0x0010
#define IDSTATWORD_VENDOR2	0x0020
#define IDSTATWORD_VENDOR3	0x0040
#define IDSTATWORD_VENDOR4	0x0080
#define IDSTATWORD_MINRFLT	0x0100
#define IDSTATWORD_MINNRFLT	0x0200
#define IDSTATWORD_MAJRFLT	0x0400
#define IDSTATWORD_MAJNRFLT	0x0800
#define IDSTATWORD_RSVD3	0x1000
#define IDSTATWORD_RSVD4	0x2000
#define IDSTATWORD_RSVD5	0x4000
#define IDSTATWORD_RSVD6	0x8000

#endif /* __EIPS_CPF_H__ */
