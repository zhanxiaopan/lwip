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
 */

#ifndef __EIPS_TAG_H__
#define __EIPS_TAG_H__

/* User Defines */
#define EIPS_TAG_MAX_LEN_PER_STRING     40
#define EIPS_TAG_MAX_NUM_ELEMENT_LEVELS 5
#define EIPS_TAG_MAX_NUM_ARRAY_LEVELS   1

/* 
    Tags can be simple 
        TestInt (elements_used=1; array_ix_used=0)

    Tags can be arrays
        TestInt[3] (elements_used=1; array_ix_used=1)

    Tags can be complicated
        Tag1[3].Tag2[1][2].Value (elements_used=3; array_ix_used=1,2,0)
*/

/* Structure Defines */
typedef struct
{
    uint16  str_len;
    char    str[EIPS_TAG_MAX_LEN_PER_STRING];
    uint8   array_ix_used;
    uint16  array_ix[EIPS_TAG_MAX_NUM_ARRAY_LEVELS];
}EIPS_TAG_ELEMENT;

typedef struct
{
    uint8 elements_used;
    EIPS_TAG_ELEMENT element[EIPS_TAG_MAX_NUM_ELEMENT_LEVELS];
}EIPS_TAG;

#define MAX_STRING_TAGSIZE 82 /* we have 82 characters */
typedef struct
{
    uint32  len;     /* 0-82 */
    char    dat[(MAX_STRING_TAGSIZE+2)]; /* 82+2 pad (32-bit align) */
}EIPS_TAG_LOGIX_STRING_FORMAT;

/* Data Types (defined by ODVA) */
#define EIPS_CL_DATATYPE_BOOL        0x00C1
#define EIPS_CL_DATATYPE_SINT        0x00C2
#define EIPS_CL_DATATYPE_INT         0x00C3
#define EIPS_CL_DATATYPE_DINT        0x00C4
#define EIPS_CL_DATATYPE_REAL        0x00CA
#define EIPS_CL_DATATYPE_BITARRAY    0x00D3
#define EIPS_CL_DATATYPE_STRUCTURE   0x02A0
#define EIPS_CL_STRUCTHANDLE_STRING  0x0FCE

/* function return codes */
#define EIPS_TAGERR_SUCCESS         0
#define EIPS_TAGERR_PASSED_PARAM    -1
#define EIPS_TAGERR_TAG_NOT_FOUND   -2
#define EIPS_TAGERR_TAG_READ_ONLY   -3      
#define EIPS_TAGERR_DATA_SIZE_ERROR -4          /* passed data pointer not big enough for request or mismatch */
#define EIPS_TAGERR_PARTIAL_XFER    -5          /* partial transfer (used with fragmentation) */
#define EIPS_TAGERR_OFFSET          0x2105      /* num elements or byte offset error */
#define EIPS_TAGERR_DATA_TYPE       0x2107

#define EIPS_FRAG_TAG_MAX_DATA_ON_READ  400

/* ****************************** */
/* Global Functions in eips_tag.c */
/* ****************************** */
int16 eips_tag_reassemble_tagname (EIPS_TAG *tag, char *str, uint16 max_len_str);

#ifdef EIPS_CIP_READ_WRITE_DATA_LOGIX
    uint8 eips_tag_process_msg (EIPS_USER_MSGRTR_REQ_FMT *request, EIPS_USER_MSGRTR_RSP_FMT *response);
#endif
#ifdef EIPS_CIP_READ_WRITE_DATA_LOGIX_ADVANCED
    uint8 eips_tag_process_msg_adv (EIPS_USER_MSGRTR_REQ_FMT *request, EIPS_USER_MSGRTR_RSP_FMT *response);
#endif

/* ********************************** */
/* Global Functions in eips_usertag.c */
/* ********************************** */
#ifdef EIPS_CIP_READ_WRITE_DATA_LOGIX
    int16 eips_usertag_get_read_data  (EIPS_TAG *tag, uint16 size_in_elements, uint16 *data_type_from_user, void *data_ptr, uint16 max_data_ptr_len_in_bytes);
    int16 eips_usertag_set_write_data (EIPS_TAG *tag, uint16 size_in_elements, uint16 data_type, void *data_ptr, uint16 data_ptr_len_in_bytes);
#endif

/* ************************************** */
/* Global Functions in eips_usertag_adv.c */
/* ************************************** */
#ifdef EIPS_CIP_READ_WRITE_DATA_LOGIX_ADVANCED
    /* data must fit */
    int16 eips_usertag_get_read_data_formatted  (EIPS_TAG *tag, uint16 size_in_elements, uint16 *data_type_from_user, uint16 *struct_handle_from_user, uint8 *data_ptr, uint16 *max_data_ptr_len_in_bytes);
    int16 eips_usertag_set_write_data_formatted (EIPS_TAG *tag, uint16 size_in_elements, uint16 data_type, uint16 struct_handle, uint8 *data_ptr, uint16 data_ptr_len_in_bytes);

    /* partial transfer OK */
    int16 eips_usertag_get_frag_read_data_formatted  (EIPS_TAG *tag, uint16 size_in_elements, uint32 byte_data_offset, uint16 *data_type_from_user, uint16 *struct_handle_from_user, uint8 *data_ptr, uint16 *max_data_ptr_len_in_bytes);
    int16 eips_usertag_set_frag_write_data_formatted (EIPS_TAG *tag, uint16 size_in_elements, uint32 byte_data_offset, uint16 data_type, uint16 struct_handle, uint8 *data_ptr, uint16 data_ptr_len_in_bytes);
#endif

#endif /* __EIPS_TAG_H__ */

/* *********** */
/* END OF FILE */
/* *********** */
