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
 *     Module Name: eips_encap.h
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains EtherNet/IP Server encapsulation definitions.
 *
 */

#ifndef __EIPS_ENCAP_H__
#define __EIPS_ENCAP_H__

#define EIP_ENCAP_PROT_VER   1
#define EIP_ENCAP_HEADER_SIZ 24

/* Encapsulated Commands */
#define EIP_ENCAPCMD_NOP               0x0000
#define EIP_ENCAPCMD_LISTTARGETS       0x0001
#define EIP_ENCAPCMD_LISTSERVICES      0x0004
#define EIP_ENCAPCMD_LISTIDENTITY      0x0063
#define EIP_ENCAPCMD_LISTINTERFACES    0x0064
#define EIP_ENCAPCMD_REGISTERSESSION   0x0065
#define EIP_ENCAPCMD_UNREGISTERSESSION 0x0066
#define EIP_ENCAPCMD_SENDRRDATA        0x006F
#define EIP_ENCAPCMD_SENDUNITDATA      0x0070

/* Encap Error Codes */
#define EIP_ENCAPSTATUSERR_SUCCESS  0x0000
#define EIP_ENCAPSTATUSERR_COMMAND  0x0001
#define EIP_ENCAPSTATUSERR_MEMORY   0x0002
#define EIP_ENCAPSTATUSERR_FORMAT   0x0003
#define EIP_ENCAPSTATUSERR_SESSION  0x0064
#define EIP_ENCAPSTATUSERR_LENGTH   0x0065
#define EIP_ENCAPSTATUSERR_PROTREV  0x0069

#endif /* __EIPS_ENCAP_H__ */
