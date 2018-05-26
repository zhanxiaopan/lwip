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
 *     Module Name: eips_pccc.c
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains the definitions needed for EtherNet/IP Server
 * PCCC processing.
 *
 */

/* ---------------------------- */
/* INCLUDE FILES                */
/* ---------------------------- */
#include "eips_system.h"

/* used for debug */
#undef  __RTA_FILE__
#define __RTA_FILE__ "eips_pccc.c"

/* see if this module is used (this line must be after the includes) */
#ifdef EIPS_PCCC_USED

/* ---------------------------- */
/* LOCAL FUNCTION PROTOTYPES	*/
/* ---------------------------- */

/* ---------------------------- */
/* MISCELLANEOUS		*/
/* ---------------------------- */
static struct
{
    uint8   gen_status;
    uint16  ext_err_size;
    uint16  ext_err[16];
}df1_error;

/* ---------------------------- */
/* GLOBAL VARIABLES		*/
/* ---------------------------- */
/* used to parse the request message */
struct
{
    uint16 req_vend_id;      /* requestor vendor id (echoed in response) */
    uint32 req_ser_num;      /* requestor serial number (echoed in response) */
    uint8  command;          /* command, must be 0x0F */
    uint8  STS;              /* status */
    uint16 TNS;              /* transaction number */
    uint8  FNC;              /* function code  ("Typed Read" - 0x68 /  "Typed Write" - 0x67") */
    uint16 pack_offset;      /* packet offset, must be 0 */
    uint16 total_trans;      /* total number of encoded transactions, must be 1 */
    uint16 num_regs;         /* number of regs to read/write */
    uint8  ascii_file_type;  /* used on ASCII addresses */
    uint16 file_num;         /* forwarded to user (i.e. N7:3 would be 7) */
    uint16 element_num;      /* element number (i.e.  N7:3 would be 3) */
}PCCC;

/**/
/* ******************************************************************** */
/*			GLOBAL FUNCTIONS				*/
/* ******************************************************************** */
/* ====================================================================
Function:   eips_pccc_init
Parameters: init type
Returns:    N/A

This function initialize all user PCCC variables.
======================================================================= */
void eips_pccc_init (uint8 init_type)
{
    if(init_type){} /* keep the compiler happy */
}

/* ====================================================================
Function:   eips_pccc_execute
Parameters: init type
Returns:    N/A

This function processes the Forward_Open service code.

********************************************
* The Request Message format is as follows *
********************************************
********  ********************	************************************
DataType  Parameter Name        Description
********  ********************	************************************
(UINT8)   Req ID Length         Length of Req. ID (vend id + s/n + other + 1) should be 7
(UINT16)  Req Vendor ID         Vendor ID of Requestor
(UINT32)  Req Serial Number     Serial Number of Requestor

PASS TO DF1 PROCESSING FOR COMMONALITY

(UINT8)   Command Byte          Command Byte (should be 0x0F)
(UINT8)   STS                   STS (Status)
(UINT16)  TNS                   TNS (Transaction Number)
(UINT8)   FNC                   Function Code ("Typed Read" - 0x68 /  "Typed Write" - 0x67")
(UINT16)  Packet Offset         Packet Offset (should be 0)
(UINT16)  Total Transaction     Total Transaction (should be 1)
(STRING)  ASCII Address         ASCII ADDR (NULL $N7:0 NULL)
(UINT16)  Number of Registers   Number of registers to read/write
(n bytes) Data                  Data (writes ONLY)

*********************************************
* The Response Message format is as follows *
*********************************************
********  ********************	************************************
DataType  Parameter Name        Description
********  ********************	************************************
(UINT8)   Req ID Length         Length of Req. ID (vend id + s/n + other + 1) should be 7
(UINT16)  Req Vendor ID         Vendor ID of Requestor
(UINT32)  Req Serial Number     Serial Number of Requestor

RETURNED FROM DF1 PROCESSING FOR COMMONALITY

(UINT8)   Command Byte          Command Byte (should be 0x4F)
(UINT8)   STS                   STS (Status)
(UINT16)  TNS                   TNS (Transaction Number)
(UINT16)  Type Information      Type information (0xC3 for INT; 0xCA for Float)
(n bytes) Data                  Data (reads ONLY)
======================================================================= */
void eips_pccc_execute (CPF_MESSAGE_STRUCT *cpf)
{
    uint8   class_id, inst_id;
    uint8   class_found, inst_found;
    uint8   path_seg, path_val;
    uint16  i, msg_ix, rsp_ix;
    uint8   req_size;
    uint16  response_size;

    /* initialize variables */
    class_id = 0;
    inst_id = 0;
    class_found = FALSE;
    inst_found  = FALSE;
    memset(&PCCC, 0, sizeof(PCCC));

    /* parse the path */
    for(i=0; i<cpf->emreq->user_struct.path_size; i++)
    {
        path_seg = EIPS_LO(cpf->emreq->user_struct.path[i]);
        path_val = EIPS_HI(cpf->emreq->user_struct.path[i]);

        switch (path_seg)
        {
            /* 8 Bit Class */
            case LOGSEG_8BITCLASS:
                class_found = TRUE;
                class_id	= path_val;
                break;

            /* 8 Bit Instance */
            case LOGSEG_8BITINST:
                inst_found = TRUE;
                inst_id	= path_val;
                break;

            /* ERROR with path */
            default:
                /* Error: Path Segment Error */
                cpf->emrsp->user_struct.gen_stat = ERR_PATHSEGMENT;
                cpf->emrsp->user_struct.ext_stat_size = 0;
                return; /* exit on an error */
        }; /* END-> "switch (path_seg)" */
    }

    /* This function code requires a valid class, instance */
    if((class_found == FALSE) || (inst_found == FALSE))
    {
        /* Error: Path Destination Unknown */
        cpf->emrsp->user_struct.gen_stat = ERR_PATHDESTUNKNOWN;
        cpf->emrsp->user_struct.ext_stat_size = 0;
        return; /* exit on an error */
    }

    /* Validate the Class is "CLASS_PCCC" and the Instance is 1 */
    if((class_id != CLASS_PCCC) || (inst_id != 1))
    {
        /* Error: Path Destination Unknown */
        cpf->emrsp->user_struct.gen_stat = ERR_PATHDESTUNKNOWN;
        cpf->emrsp->user_struct.ext_stat_size = 0;
        return;
    }

    /* parse the request */
    msg_ix = 0;

    /* Validate the length of the requestor id is >= 7 */
    req_size = cpf->emreq->user_struct.req_data[msg_ix++];
    if(req_size < 7)
    {    
        /* Error: Not Enough Data */
        cpf->emrsp->user_struct.gen_stat = ERR_INSUFF_DATA;
        cpf->emrsp->user_struct.ext_stat_size = 0;
        return;
    }

    /* store the requestor vendor id */
    PCCC.req_vend_id = rta_GetLitEndian16(cpf->emreq->user_struct.req_data + msg_ix);
    msg_ix += 2;

    /* store the requestor serial number */
    PCCC.req_ser_num = rta_GetLitEndian32(cpf->emreq->user_struct.req_data + msg_ix);
    msg_ix += 4;


    /*  ********************************** 
        THIS IS THE SPLITING POINT FOR DF1

        Pre-build the response header.
        ********************************** */
    rsp_ix = 0;

    /* Length of Requestor is always 7 */
    cpf->emrsp->user_struct.rsp_data[rsp_ix++] = 7;

    /* echo the Requestor vendor id */
    rta_PutLitEndian16(PCCC.req_vend_id, (cpf->emrsp->user_struct.rsp_data+rsp_ix));
    rsp_ix+=2;

    /* echo the Requestor serial number */
    rta_PutLitEndian32(PCCC.req_ser_num, (cpf->emrsp->user_struct.rsp_data+rsp_ix));
    rsp_ix+=4;

    /* pass to user (clear error first) */
    memset(&df1_error, 0, sizeof(df1_error));
    response_size = eips_userdf1_proc_msg (&cpf->emreq->user_struct.req_data[msg_ix], (uint16)(cpf->emreq->user_struct.req_data_size-msg_ix), &cpf->emrsp->user_struct.rsp_data[rsp_ix], (uint16)(sizeof(cpf->emrsp->user_struct.rsp_data)-rsp_ix));

    /* build the error message (if necessary) */
    if(response_size == 0)
    {
        cpf->emrsp->user_struct.rsp_data_size = 0;
        cpf->emrsp->user_struct.gen_stat = df1_error.gen_status;
        cpf->emrsp->user_struct.ext_stat_size = (uint8)df1_error.ext_err_size;  
        for(i=0; i<df1_error.ext_err_size; i++)
        {
            cpf->emrsp->user_struct.ext_stat[i] = df1_error.ext_err[i];
        }      
    }

    /* build the successful message */
    else
    {
        cpf->emrsp->user_struct.rsp_data_size = (uint16)(rsp_ix+response_size);
    }
}

/* ====================================================================
Function:   eips_pccc_build_error
Parameters: init type
Returns:    N/A

This function builds the CPF error message.
======================================================================= */
void eips_pccc_build_error (uint8 gen_stat, uint16 ext_stat_size, uint16 * ext_stat)
{
    uint16 i;

    /* copy the general status */
    df1_error.gen_status = gen_stat;

    /* copy the extended error size */
    df1_error.ext_err_size = ext_stat_size;

    /* copy the extended error (if available) */
    if(ext_stat_size && ext_stat)
    {
        for(i=0; i<ext_stat_size; i++)
        {
            df1_error.ext_err[i] = ext_stat[i];
        }
    }
}

/**/
/* ******************************************************************** */
/*                      LOCAL FUNCTIONS                                 */
/* ******************************************************************** */

#endif
