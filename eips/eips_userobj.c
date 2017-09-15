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
 *     Module Name: eips_userobj.c
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains vendor specific object model definitions.
 *
 */


/* ---------------------------- */
/*      INCLUDE FILES           */
/* ---------------------------- */
#include "eips_system.h"
#include "eips_userobj.h"

/* used for debug */
#undef  __RTA_FILE__
#define __RTA_FILE__ "eips_userobj.c"

/* ---------------------------- */
/* LOCAL STRUCTURE DEFINITIONS	*/
/* ---------------------------- */

/* ---------------------------- */
/* STATIC VARIABLES             */
/* ---------------------------- */

/* Ethernet/IP send tick counter */
uint16	eip_send_tick = 0;


#if EIPS_USEROBJ_ASM_MAXNUM_O2TINST > 0  
static struct
{
	uint16 instID;
	uint16 size;    // in data_ptr units

#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
	uint32 data_ptr[125];
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
	uint16 data_ptr[250];
#else                                       /* BYTES */
	uint8  data_ptr[500];
#endif
} local_o2t_asm_struct[EIPS_USEROBJ_ASM_MAXNUM_O2TINST];
#endif

#if EIPS_USEROBJ_ASM_MAXNUM_T2OINST > 0
static struct
{
    uint16 instID;
    uint16 size;    // in data_ptr units

#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
    uint32 data_ptr[125];
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
    uint16 data_ptr[250];
#else                                       /* BYTES */
    uint8  data_ptr[500];
#endif
} local_t2o_asm_struct[EIPS_USEROBJ_ASM_MAXNUM_T2OINST];
#endif

EIPS_VS_DISCRETE_INPUT_DATA_STRUCT  eips_vs_discreteInDataObj;
EIPS_VS_DISCRETE_OUTPUT_DATA_STRUCT eips_vs_discreteOutDataObj;

#ifdef EIPS_PCCC_USED /* OPTIONAL */
    #define PCCC_BIT_SIZE       120 // 16-bits per
    #define PCCC_INT_SIZE       120
    #define PCCC_FLOAT_SIZE     60
    #define PCCC_STRING_SIZE    10

    uint16 pccc_bits[PCCC_BIT_SIZE];
    uint16 pccc_integers[PCCC_INT_SIZE];
    float  pccc_floats[PCCC_FLOAT_SIZE];
    EIPS_PCCC_STRING_TYPE pccc_strings[PCCC_STRING_SIZE];
#endif

static Eips_userobj__PCbf gpCbf = NULL;  /* Cbf for application level */

/* ---------------------------- */
/* LOCAL FUNCTIONS		*/
/* ---------------------------- */
void  local_get_attr (uint16 classid, uint16 instid, uint8 attrid, EIPS_USER_MSGRTR_RSP_FMT *response);
void  local_set_attr (uint16 classid, uint16 instid, uint8 attrid, uint16 data_siz, uint8 *data_ptr, EIPS_USER_MSGRTR_RSP_FMT *response);
uint8 local_dlc_valid (uint16 needed_siz, uint16 actual_siz, EIPS_USER_MSGRTR_RSP_FMT *response);

/* ---------------------------- */
/*      MISCELLANEOUS           */
/* ---------------------------- */

/**/
/* ******************************************************************** */
/*                      GLOBAL FUNCTIONS                                */
/* ******************************************************************** */

void
eips_userobj_cbf_register(Eips_userobj__PCbf pCbf)
{
  gpCbf = pCbf; /* Register the callback function */
}

void
eips_userobj_data_send(unsigned char * p, unsigned short n)
{
  memcpy(&local_t2o_asm_struct[0].data_ptr[0], p, n);
}

/* ====================================================================
Function:   eips_userobj_init
Parameters: init type
Returns:    N/A

This function initialize all user object model variables.
======================================================================= */
void eips_userobj_init (uint8 init_type)
{
    uint16 i;

    // *************************************************
    //		 static variables
    // *************************************************

    gpCbf = NULL;

    // discrete input object
    eips_vs_discreteInDataObj.Class.ClassRev = 1;
    eips_vs_discreteInDataObj.Inst.DataSize = VS_MAX_DISCRETE_INPUT_UNITS;
    memset((uint8 *)eips_vs_discreteInDataObj.Inst.DataPtr, 0, sizeof(eips_vs_discreteInDataObj.Inst.DataPtr));
       
    // discrete output object
    eips_vs_discreteOutDataObj.Class.ClassRev = 1;
    eips_vs_discreteOutDataObj.Inst.DataSize = VS_MAX_DISCRETE_OUTPUT_UNITS;
    memset((uint8 *)eips_vs_discreteOutDataObj.Inst.DataPtr, 0, sizeof(eips_vs_discreteOutDataObj.Inst.DataPtr));

#ifdef EIPS_PCCC_USED /* OPTIONAL */
    memset(pccc_bits,       0, sizeof(pccc_bits));
    memset(pccc_integers,   0, sizeof(pccc_integers));
    memset(pccc_floats,     0, sizeof(pccc_floats));
    memset(pccc_strings,    0, sizeof(pccc_strings));
#endif

    // *************************************************
    //		 non-volatile variables
    // *************************************************

    /* different initialization based on based parameters */
    switch (init_type)
    {
        /* Out of Box Initialization */
        case EIPSINIT_OUTOFBOX:
	        break;

        /* Normal Initialization */
        case EIPSINIT_NORMAL:
        default:
            break;
    };
   
    /* initialized user I/O data structures */

    /* t2o (produce) */
    for(i=0; i<EIPS_USEROBJ_ASM_MAXNUM_T2OINST; i++)
    {
        local_t2o_asm_struct[i].instID = (uint16)(VS_OBJ_DISCRETE_OUTPUT_DATA);
        local_t2o_asm_struct[i].size = (uint16)(VS_MAX_DISCRETE_OUTPUTS_ON_UINT);
        memset(local_t2o_asm_struct[i].data_ptr, 0, sizeof(local_t2o_asm_struct[i].data_ptr));
    }

    /* o2t (consume) */
    for(i=0; i<EIPS_USEROBJ_ASM_MAXNUM_O2TINST; i++)
    {
        local_o2t_asm_struct[i].instID = (uint16)(VS_OBJ_DISCRETE_INPUT_DATA);
        local_o2t_asm_struct[i].size = (uint16)(VS_MAX_DISCRETE_INPUTS_ON_UINT);
        memset(local_o2t_asm_struct[i].data_ptr, 0, sizeof(local_o2t_asm_struct[i].data_ptr));
    }
}

/* ====================================================================
Function:   eips_userobj_process
Parameters: N/A
Returns:    N/A

This function is called every loop to handle user defined processing.
======================================================================= */
void eips_userobj_process (void)
{
#if EIPS_USEROBJ_ASM_MAXNUM_O2TINST > 0
  // check if the outputs are valid (based on a I/O connection in RUN mode)
  if(eips_iomsg_outputs_valid(local_o2t_asm_struct[0].instID) == FALSE)
  {
    /* set the outputs to the safe state */
    if (gpCbf != NULL)
    { 
      (gpCbf)(EIPS_USEROBJ_EVENT_ASM_O2T_SAFE, 0, NULL, 0);
    }
  }
#endif
}

/* ====================================================================
Function:   eips_userobj_read_B_reg_pccc
Parameters: File Number
            File Offset
            Length (in words)
            Data Pointer (store the register data here)
Returns:    (see below)

EXTSTS_NONE	           - No additional error
EXTSTS_FIELDILLVAL     - A field has an illegal value
EXTSTS_SYMBOLNOTFOUND  - Symbol not found
EXTSTS_SYMBOLBADFORMAT - Symbol is of improper format
EXTSTS_BADADDRESS      - Address doesn't point to something useful
EXTSTS_DATAFILETOOLRG  - Data or file is too large
EXTSTS_TRANSSIZETOOLRG - Transaction size plus word address is too large
EXTSTS_REGISRO	       - Access denied, improper privilege (WR to RO Reg)

This function is called when a PCCC read of B (Bit) Registers 
is received. Bits are accessed in 16-bit blocks.

We don't always know what the data type is. If the file number is
invalid, return EXTSTS_BADADDRESS. You may see a call to all reads 
for the same address unless EXTSTS_NONE or an error code other than
EXTSTS_BADADDRESS is returned for a previous read.

SAMPLE CODE: B3:0 - B3:(PCCC_BIT_SIZE-1)
======================================================================= */
#ifdef EIPS_PCCC_USED /* OPTIONAL */
int8 eips_userobj_read_B_reg_pccc (uint16 file_num, uint16 file_offset, uint16 length, uint16 *reg_data)
{
    // validate the file number
    if(file_num != 3)
        return(EXTSTS_BADADDRESS);

    // validate the request is in range
    if((file_offset + length) > PCCC_BIT_SIZE)
        return(EXTSTS_DATAFILETOOLRG);
    
    // successful
    memcpy((uint8 *)reg_data, (uint8 *)&pccc_bits[file_offset], (length*(sizeof(uint16))));

    eips_user_dbprint3("eips_userobj_read_B_reg_pccc B%d:%d, Len %d\r\n", file_num, file_offset, length);
    return(EXTSTS_NONE);
}
#endif

/* ====================================================================
Function:   eips_userobj_read_N_reg_pccc
Parameters: File Number
            File Offset
            Length (in words)
            Data Pointer (store the register data here)
Returns:    (see below)

EXTSTS_NONE	           - No additional error
EXTSTS_FIELDILLVAL     - A field has an illegal value
EXTSTS_SYMBOLNOTFOUND  - Symbol not found
EXTSTS_SYMBOLBADFORMAT - Symbol is of improper format
EXTSTS_BADADDRESS      - Address doesn't point to something useful
EXTSTS_DATAFILETOOLRG  - Data or file is too large
EXTSTS_TRANSSIZETOOLRG - Transaction size plus word address is too large
EXTSTS_REGISRO	       - Access denied, improper privilege (WR to RO Reg)

This function is called when a PCCC read of N (Integer) Registers 
is received.

We don't always know what the data type is. If the file number is
invalid, return EXTSTS_BADADDRESS. You may see a call to all reads 
for the same address unless EXTSTS_NONE or an error code other than
EXTSTS_BADADDRESS is returned for a previous read.

SAMPLE CODE: N7:0 - N7:(PCCC_INT_SIZE-1)
======================================================================= */
#ifdef EIPS_PCCC_USED /* OPTIONAL */
int8 eips_userobj_read_N_reg_pccc (uint16 file_num, uint16 file_offset, uint16 length, uint16 *reg_data)
{
    // validate the file number
    if(file_num != 7)
        return(EXTSTS_BADADDRESS);

    // validate the request is in range
    if((file_offset + length) > PCCC_INT_SIZE)
        return(EXTSTS_DATAFILETOOLRG);
    
    // successful
    memcpy((uint8 *)reg_data, (uint8 *)&pccc_integers[file_offset], (length*(sizeof(uint16))));

    eips_user_dbprint3("eips_userobj_read_N_reg_pccc N%d:%d, Len %d\r\n", file_num, file_offset, length);
    return(EXTSTS_NONE);
}
#endif

/* ====================================================================
Function:   eips_userobj_read_F_reg_pccc
Parameters: File Number
            File Offset
            Length (in words)
            Data Pointer (store the register data here)
Returns:    (see below)

EXTSTS_NONE	           - No additional error
EXTSTS_FIELDILLVAL     - A field has an illegal value
EXTSTS_SYMBOLNOTFOUND  - Symbol not found
EXTSTS_SYMBOLBADFORMAT - Symbol is of improper format
EXTSTS_BADADDRESS      - Address doesn't point to something useful
EXTSTS_DATAFILETOOLRG  - Data or file is too large
EXTSTS_TRANSSIZETOOLRG - Transaction size plus word address is too large
EXTSTS_REGISRO	       - Access denied, improper privilege (WR to RO Reg)

This function is called when a PCCC read of F (Float) Registers 
is received.

We don't always know what the data type is. If the file number is
invalid, return EXTSTS_BADADDRESS. You may see a call to all reads 
for the same address unless EXTSTS_NONE or an error code other than
EXTSTS_BADADDRESS is returned for a previous read.

SAMPLE CODE: F8:0 - F8:(PCCC_FLOAT_SIZE-1)
======================================================================= */
#ifdef EIPS_PCCC_USED /* OPTIONAL */
int8 eips_userobj_read_F_reg_pccc (uint16 file_num, uint16 file_offset, uint16 length, float *reg_data)
{
    // validate the file number
    if(file_num != 8)
        return(EXTSTS_BADADDRESS);

    // validate the request is in range
    if((file_offset + length) > PCCC_FLOAT_SIZE)
        return(EXTSTS_DATAFILETOOLRG);
    
    // successful
    memcpy((uint8 *)reg_data, (uint8 *)&pccc_floats[file_offset], (length*(sizeof(float))));

    eips_user_dbprint3("eips_userobj_read_F_reg_pccc F%d:%d, Len %d\r\n", file_num, file_offset, length);
    return(EXTSTS_NONE);
}
#endif

/* ====================================================================
Function:   eips_userobj_read_ST_reg_pccc
Parameters: File Number
            File Offset
            Length (in words)
            Data Pointer (store the register data here)
Returns:    (see below)

EXTSTS_NONE	           - No additional error
EXTSTS_FIELDILLVAL     - A field has an illegal value
EXTSTS_SYMBOLNOTFOUND  - Symbol not found
EXTSTS_SYMBOLBADFORMAT - Symbol is of improper format
EXTSTS_BADADDRESS      - Address doesn't point to something useful
EXTSTS_DATAFILETOOLRG  - Data or file is too large
EXTSTS_TRANSSIZETOOLRG - Transaction size plus word address is too large
EXTSTS_REGISRO	       - Access denied, improper privilege (WR to RO Reg)

This function is called when a PCCC read of ST (String) Registers 
is received.

Stings have a unique structure:

#define EIPS_PCCC_MAX_STRING_LEN    82
typedef struct
{
    uint32  string_size;
    uint8   string_data[EIPS_PCCC_MAX_STRING_LEN];
}EIPS_PCCC_STRING_TYPE;

We don't always know what the data type is. If the file number is
invalid, return EXTSTS_BADADDRESS. You may see a call to all reads 
for the same address unless EXTSTS_NONE or an error code other than
EXTSTS_BADADDRESS is returned for a previous read.

SAMPLE CODE: ST10:0 - ST10:(PCCC_STRING_SIZE-1)
======================================================================= */
#ifdef EIPS_PCCC_USED /* OPTIONAL */
int8 eips_userobj_read_ST_reg_pccc (uint16 file_num, uint16 file_offset, uint16 length, EIPS_PCCC_STRING_TYPE *reg_data)
{
    // validate the file number
    if(file_num != 10)
        return(EXTSTS_BADADDRESS);

    // validate the request is in range
    if((file_offset + length) > PCCC_STRING_SIZE)
        return(EXTSTS_DATAFILETOOLRG);
    
    // successful
    memcpy((uint8 *)reg_data, (uint8 *)&pccc_strings[file_offset], (length*(sizeof(EIPS_PCCC_STRING_TYPE))));

    eips_user_dbprint3("eips_userobj_read_ST_reg_pccc ST%d:%d, Len %d\r\n", file_num, file_offset, length);
    return(EXTSTS_NONE);
}
#endif

/* ====================================================================
Function:   eips_userobj_write_B_reg_pccc
Parameters: File Number
            File Offset
            Length (in words)
            Data Pointer (new register data is stored here)
Returns:    (see below)

EXTSTS_NONE	           - No additional error
EXTSTS_FIELDILLVAL     - A field has an illegal value
EXTSTS_SYMBOLNOTFOUND  - Symbol not found
EXTSTS_SYMBOLBADFORMAT - Symbol is of improper format
EXTSTS_BADADDRESS      - Address doesn't point to something useful
EXTSTS_DATAFILETOOLRG  - Data or file is too large
EXTSTS_TRANSSIZETOOLRG - Transaction size plus word address is too large
EXTSTS_REGISRO	       - Access denied, improper privilege (WR to RO Reg)

This function is called when a PCCC write of B (Bit) Registers 
is received.

We don't always know what the data type is. If the file number is
invalid, return EXTSTS_BADADDRESS. You may see a call to all writes 
for the same address unless EXTSTS_NONE or an error code other than
EXTSTS_BADADDRESS is returned for a previous write.

SAMPLE CODE: B3:0 - B3:(PCCC_BIT_SIZE-1)
======================================================================= */
#ifdef EIPS_PCCC_USED /* OPTIONAL */    
int8 eips_userobj_write_B_reg_pccc (uint16 file_num, uint16 file_offset, uint16 length, uint16 *reg_data)
{
    // validate the file number
    if(file_num != 3)
        return(EXTSTS_BADADDRESS);

    // validate the request is in range
    if((file_offset + length) > PCCC_BIT_SIZE)
        return(EXTSTS_DATAFILETOOLRG);
    
    // successful
    memcpy((uint8 *)&pccc_bits[file_offset], (uint8 *)reg_data, (length*(sizeof(uint16))));

// dummy increment to see data change on the loop back!!
//pccc_bits[file_offset]++;
    
    eips_user_dbprint3("eips_userobj_write_B_reg_pccc B%d:%d, Len %d\r\n", file_num, file_offset, length);
    return(EXTSTS_NONE);
}
#endif

/* ====================================================================
Function:   eips_userobj_write_N_reg_pccc
Parameters: File Number
            File Offset
            Length (in words)
            Data Pointer (new register data is stored here)
Returns:    (see below)

EXTSTS_NONE	           - No additional error
EXTSTS_FIELDILLVAL     - A field has an illegal value
EXTSTS_SYMBOLNOTFOUND  - Symbol not found
EXTSTS_SYMBOLBADFORMAT - Symbol is of improper format
EXTSTS_BADADDRESS      - Address doesn't point to something useful
EXTSTS_DATAFILETOOLRG  - Data or file is too large
EXTSTS_TRANSSIZETOOLRG - Transaction size plus word address is too large
EXTSTS_REGISRO	       - Access denied, improper privilege (WR to RO Reg)

This function is called when a PCCC write of N (Integer) Registers 
is received.

We don't always know what the data type is. If the file number is
invalid, return EXTSTS_BADADDRESS. You may see a call to all writes 
for the same address unless EXTSTS_NONE or an error code other than
EXTSTS_BADADDRESS is returned for a previous write.

SAMPLE CODE: N7:0 - N7:(PCCC_INT_SIZE-1)
======================================================================= */
#ifdef EIPS_PCCC_USED /* OPTIONAL */    
int8 eips_userobj_write_N_reg_pccc (uint16 file_num, uint16 file_offset, uint16 length, uint16 *reg_data)
{
    // validate the file number
    if(file_num != 7)
        return(EXTSTS_BADADDRESS);

    // validate the request is in range
    if((file_offset + length) > PCCC_INT_SIZE)
        return(EXTSTS_DATAFILETOOLRG);
    
    // successful
    memcpy((uint8 *)&pccc_integers[file_offset], (uint8 *)reg_data, (length*(sizeof(uint16))));

// dummy increment to see data change on the loop back!!
//pccc_integers[file_offset]++;
    
    eips_user_dbprint3("eips_userobj_write_N_reg_pccc N%d:%d, Len %d\r\n", file_num, file_offset, length);
    return(EXTSTS_NONE);
}
#endif

/* ====================================================================
Function:   eips_userobj_write_F_reg_pccc
Parameters: File Number
            File Offset
            Length (in words)
            Data Pointer (new register data is stored here)
Returns:    (see below)

EXTSTS_NONE	           - No additional error
EXTSTS_FIELDILLVAL     - A field has an illegal value
EXTSTS_SYMBOLNOTFOUND  - Symbol not found
EXTSTS_SYMBOLBADFORMAT - Symbol is of improper format
EXTSTS_BADADDRESS      - Address doesn't point to something useful
EXTSTS_DATAFILETOOLRG  - Data or file is too large
EXTSTS_TRANSSIZETOOLRG - Transaction size plus word address is too large
EXTSTS_REGISRO	       - Access denied, improper privilege (WR to RO Reg)

This function is called when a PCCC write of F (Float) Registers 
is received.

We don't always know what the data type is. If the file number is
invalid, return EXTSTS_BADADDRESS. You may see a call to all writes 
for the same address unless EXTSTS_NONE or an error code other than
EXTSTS_BADADDRESS is returned for a previous write.

SAMPLE CODE: F8:0 - F8:(PCCC_FLOAT_SIZE-1)
======================================================================= */
#ifdef EIPS_PCCC_USED /* OPTIONAL */
int8 eips_userobj_write_F_reg_pccc (uint16 file_num, uint16 file_offset, uint16 length, float *reg_data)
{
    // validate the file number
    if(file_num != 8)
        return(EXTSTS_BADADDRESS);

    // validate the request is in range
    if((file_offset + length) > PCCC_FLOAT_SIZE)
        return(EXTSTS_DATAFILETOOLRG);
    
    // successful
    memcpy((uint8 *)&pccc_floats[file_offset], (uint8 *)reg_data, (length*(sizeof(float))));

// dummy increment to see data change on the loop back!!
//pccc_floats[file_offset] = (float)(pccc_floats[file_offset] + 1.23);

    eips_user_dbprint3("eips_userobj_write_F_reg_pccc F%d:%d, Len %d\r\n", file_num, file_offset, length);
    return(EXTSTS_NONE);
}
#endif

/* ====================================================================
Function:   eips_userobj_write_ST_reg_pccc
Parameters: File Number
            File Offset
            Length (in words)
            Data Pointer (new register data is stored here)
Returns:    (see below)

EXTSTS_NONE	           - No additional error
EXTSTS_FIELDILLVAL     - A field has an illegal value
EXTSTS_SYMBOLNOTFOUND  - Symbol not found
EXTSTS_SYMBOLBADFORMAT - Symbol is of improper format
EXTSTS_BADADDRESS      - Address doesn't point to something useful
EXTSTS_DATAFILETOOLRG  - Data or file is too large
EXTSTS_TRANSSIZETOOLRG - Transaction size plus word address is too large
EXTSTS_REGISRO	       - Access denied, improper privilege (WR to RO Reg)

This function is called when a PCCC write of F (Float) Registers 
is received.

We don't always know what the data type is. If the file number is
invalid, return EXTSTS_BADADDRESS. You may see a call to all writes 
for the same address unless EXTSTS_NONE or an error code other than
EXTSTS_BADADDRESS is returned for a previous write.

SAMPLE CODE: ST10:0 - ST10:(PCCC_STRING_SIZE-1)
======================================================================= */
#ifdef EIPS_PCCC_USED /* OPTIONAL */
int8 eips_userobj_write_ST_reg_pccc (uint16 file_num, uint16 file_offset, uint16 length, EIPS_PCCC_STRING_TYPE *reg_data)
{
    // validate the file number
    if(file_num != 10)
        return(EXTSTS_BADADDRESS);

    // validate the request is in range
    if((file_offset + length) > PCCC_FLOAT_SIZE)
        return(EXTSTS_DATAFILETOOLRG);
    
    // successful
    memcpy((uint8 *)&pccc_strings[file_offset], (uint8 *)reg_data, (length*(sizeof(EIPS_PCCC_STRING_TYPE))));

    eips_user_dbprint3("eips_userobj_write_ST_reg_pccc ST%d:%d, Len %d\r\n", file_num, file_offset, length);
    return(EXTSTS_NONE);
}
#endif

/* ====================================================================
Function:   eips_userobj_cfg_asm_process
Parameters: configuration assembly instance
            pointer to configuration data
            configuration data size (in bytes)
            pointer to the general error code (0 on success (default))
            pointer to the extended error code (0 on success (default))
Returns:    SUCCESS / FAILURE

This function validates the configuration instance id and processes the
configuration data.

If the instance id or size is incorrect
    *err_rsp_code = ERR_CNXN_FAILURE;
    *add_err_rsp_code = CMERR_BAD_SEGMENT;
======================================================================= */
uint8 eips_userobj_cfg_asm_process(uint16 cfg_inst_id, uint8 * config_data, uint16 config_data_size_in_bytes, uint8 *err_rsp_code, uint16 *add_err_rsp_code)
{
    RTA_UNUSED_PARAM(cfg_inst_id);

    // make sure all the passed pointers are valid
    if( (err_rsp_code == NULL) || 
        (add_err_rsp_code == NULL) || 
        ((config_data == NULL) && (config_data_size_in_bytes != 0)))
    {
        // invalid pointers 
        return(FAILURE);
    }
        
    /* match the id value (if used, else don't care); allow a size of n (valid) or 0 (NULL) */
//    if(cfg_inst_id == 128 && (config_data_size_in_bytes == 10 || config_data_size_in_bytes == 0))

    /* we don't care about the id, just that the size is 0 */
    if(config_data_size_in_bytes == 0)
        return(SUCCESS);
    
    // error with size or instance ID
    *err_rsp_code = ERR_CNXN_FAILURE;
    *add_err_rsp_code = CMERR_BAD_SEGMENT;           
    return(FAILURE);
}

/* Identity Object */
/* ====================================================================
Function:   eips_userobj_getIDObj_Attr05_Status
Parameters: N/A
Returns:    status word

This function returns the status word. If necessary the "Owned" bit and
"Fault" bits are modified by the RTA software.
======================================================================= */
#ifdef EIPS_USEROBJ_ID_OBJ_EXTENDED
uint16 eips_userobj_getIDObj_Attr05_Status (void)
{
    return(0);
}
#endif
/* ====================================================================
Function:   eips_userobj_getSerialNumber
Parameters: N/A
Returns:    32-bit unique serial number

This function returns a unique 32-bit serial number.  The least
significant 4 bytes of the MAC ID can be used since they must be
unique. Every device from a vendor must have a unique serial number.
======================================================================= */
#ifdef EIPS_USEROBJ_ID_OBJ_EXTENDED
uint32 eips_userobj_getIDObj_Attr06_SerialNumber (void)
#else
uint32 eips_userobj_getSerialNumber (void)
#endif
{
    uint8 temp_buf[6];

    // get the MAC ID from the user
    eips_usersock_getEthLinkObj_Attr03_MacAddr(temp_buf);

    // return the last 4 bytes as a uint32
    return(rta_GetBigEndian32(&temp_buf[2]));
}
/* ====================================================================
Function:   eips_userobj_getIDObj_Attr08_State
Parameters: N/A
Returns:    state byte

This function returns the state byte.
======================================================================= */
#ifdef EIPS_USEROBJ_ID_OBJ_EXTENDED
uint8 eips_userobj_getIDObj_Attr08_State (void)
{
    return(3); /* operational */
}
#endif

/* ====================================================================
Function:   eips_userobj_getIDObj_Attr09_CCV
Parameters: N/A
Returns:    CCV word

This function returns the CCV word.
======================================================================= */
#ifdef EIPS_USEROBJ_ID_OBJ_EXTENDED
uint16 eips_userobj_getIDObj_Attr09_CCV (void)
{
    return(0);
}
#endif

/* Assembly Object */
/* ====================================================================
Function:   eips_userobj_asmGetO2TInstList
Parameters: pointer to store the array of supported instance
	        max number of instances allowed (EIPS_USEROBJ_ASM_MAXNUM_O2TINST)
Returns:    actual number of instances used

This function returns the list of O2T (Consume) Assembly Instances.
Each instance number must be unique (between all the O2T instances, 
all the T2O instances, EIPS_USEROBJ_ASM_INPUTONLY_HB_INST and 
EIPS_USEROBJ_ASM_LISTERONLY_HB_INST) and must be in the range of 100 -
199 (0x64 - 0xc7). This function is only called once at start up so
make sure you reset the code if changes are made to the assembly list.
======================================================================= */
uint16 eips_userobj_asmGetO2TInstList (uint16 * list_ptr, uint16 max_num_inst)
{
    uint16 i;

    // validate the list pointer
    if(list_ptr == NULL)
        return(0);

    // store the instances
    for(i=0; i<EIPS_USEROBJ_ASM_MAXNUM_O2TINST && i<max_num_inst; i++)
    {
        list_ptr[i] = local_o2t_asm_struct[i].instID;
    }

    return(EIPS_USEROBJ_ASM_MAXNUM_O2TINST);
}

/* ====================================================================
Function:   eips_userobj_asmGetT2OInstList
Parameters: pointer to store the array of supported instance
	        max number of instances allowed (EIPS_USEROBJ_ASM_MAXNUM_T2OINST)
Returns:    actual number of instances used

This function returns the list of T2O (Produce) Assembly Instances.
Each instance number must be unique (between all the O2T instances, 
all the T2O instances, EIPS_USEROBJ_ASM_INPUTONLY_HB_INST and 
EIPS_USEROBJ_ASM_LISTERONLY_HB_INST) and must be in the range of 100 -
199 (0x64 - 0xc7). This function is only called once at start up so
make sure you reset the code if changes are made to the assembly list.
======================================================================= */
uint16  eips_userobj_asmGetT2OInstList (uint16 * list_ptr, uint16 max_num_inst)
{
    uint16 i;

    // validate the list pointer
    if(list_ptr == NULL)
        return(0);

    // store the instances
    for(i=0; i<EIPS_USEROBJ_ASM_MAXNUM_T2OINST && i<max_num_inst; i++)
    {
        list_ptr[i] = local_t2o_asm_struct[i].instID;
    }

    return(EIPS_USEROBJ_ASM_MAXNUM_T2OINST);
}

/* ====================================================================
Function:   eips_userobj_getAsmPtr
Parameters: assembly instance id
	        pointer to expected size in words
Returns:    pointer (NULL on error)

This function validate the Assembly instance and size.	If they both
match an expected Assembly instance, return the pointer to the 
Assembly data.  If the size is incorrect, modify the size data pointer.  
If the instance doesn't match, return NULL.
======================================================================= */
#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
    uint32 * eips_userobj_getAsmPtr (uint16 inst_id, uint16 *size)
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
    uint16 * eips_userobj_getAsmPtr (uint16 inst_id, uint16 *size)
#else                                       /* BYTES */
    uint8 * eips_userobj_getAsmPtr (uint16 inst_id, uint16 *size)
#endif
{
    uint16 i;

    /* search through t2o instances */
    for(i=0; i<EIPS_USEROBJ_ASM_MAXNUM_T2OINST; i++)
    {
        if(local_t2o_asm_struct[i].instID == inst_id)
        {
            *size = local_t2o_asm_struct[i].size;
            return(local_t2o_asm_struct[i].data_ptr);
        }
    }

    /* search through o2t instances */
    for(i=0; i<EIPS_USEROBJ_ASM_MAXNUM_O2TINST; i++)
    {
        if(local_o2t_asm_struct[i].instID == inst_id)
        {
            *size = local_o2t_asm_struct[i].size;
            return(local_o2t_asm_struct[i].data_ptr);
        }
    }

    /* There is an error, set the passed pointer to NULL */
    *size = 0;
    return(NULL);
}

/* ====================================================================
Function:   eips_userobj_asmDataWrote
Parameters: O_to_T assembly instance
	        Number of bytes written
Returns:    N/A

This function is called when an Assembly is written.
======================================================================= */
#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
    void eips_userobj_asmDataWrote (uint16 inst_id, uint16 dwords_written)
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
    void eips_userobj_asmDataWrote (uint16 inst_id, uint16 words_written)
#else                                       /* BYTES */
    void eips_userobj_asmDataWrote (uint16 inst_id, uint16 bytes_written)
#endif
{
#if 0
    // simple echo test
    local_t2o_asm_struct[0].data_ptr[0] = local_o2t_asm_struct[0].data_ptr[0];
#endif

/*
 * Issue this function is only called after sending trigger by the state-of-change, so the
 * T2O data is not updated periodically ot client.
 */
	if (gpCbf != NULL)
	{
#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
	(gpCbf)(EIPS_USEROBJ_EVENT_ASM_O2T_RCVD, inst_id, (unsigned char *)&local_o2t_asm_struct[0].data_ptr[0], dwords_written*4);
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
	(gpCbf)(EIPS_USEROBJ_EVENT_ASM_O2T_RCVD, inst_id, (unsigned char *)&local_o2t_asm_struct[0].data_ptr[0], words_written*2);
#else                                       /* BYTES */
	(gpCbf)(EIPS_USEROBJ_EVENT_ASM_O2T_RCVD, inst_id, (unsigned char *)&local_o2t_asm_struct[0].data_ptr[0], bytes_written);
#endif
	}

#ifdef EIPS_PCCC_USED
   // copy the output data to the PCCC registers
#endif

}

/* ====================================================================
Function:   eips_userobj_procVendSpecObject
Parameters: Pointer to Message Router Request structure
            Pointer to Message Router Response structure
Returns:    SUCCESS - Use response in "response" structure
            FAILURE - Use default error "Object does not exist"

This function passes the Message Router Request and Response structures.
If the user supports any vendor specific objects, parse the message,
validate the passed request and build a response message in the passed
response strucute.  If the passed object isn't supported, return
FAILURE and don't fill in the passed response structure.
======================================================================= */
uint8 eips_userobj_procVendSpecObject (EIPS_USER_MSGRTR_REQ_FMT *request, EIPS_USER_MSGRTR_RSP_FMT *response)
{
    uint16  class_id, inst_id;
    uint8   attr_id;
    uint8   class_found, inst_found, attr_found;
    uint8   path_seg, path_val;
    int16   i;

    /* set the response to success */
    response->gen_stat = ERR_SUCCESS;
    response->ext_stat_size = 0;

    /* initialize variables */
    class_id = 0;
    inst_id = 0;
    attr_id = 0;
    class_found = FALSE;
    inst_found  = FALSE;
    attr_found  = FALSE;

    /* parse the path */
    for(i=0; i<request->path_size; i++)
    {
        path_seg = EIPS_LO(request->path[i]);
        path_val = EIPS_HI(request->path[i]);

        /* The only valid order is Class, Inst, Attr */
        switch (path_seg)
        {
            /* 8 Bit Class */
            case LOGSEG_8BITCLASS:
                /* This must be the first path */
	            if( (class_found == FALSE) &&
		            ( inst_found == FALSE) &&
		            ( attr_found == FALSE) )
	            {
	                class_found = TRUE;
	                class_id = path_val;
	            }

	            /* Bad Path order */
	            else
	            {
                    return(FAILURE); /* let the RTA app set the error */
	            }
	            break;

            /* 16 Bit Class */
            case LOGSEG_16BITCLASS:
                /* This must be the first path */
	            if( (class_found == FALSE) &&
		            ( inst_found == FALSE) &&
		            ( attr_found == FALSE) )
	            {
	                class_found = TRUE;
                    i++; /* the class id is stored in the next word) */
	                class_id = (uint16)(request->path[i]);
	            }

	            /* Bad Path order */
	            else
	            {
                    return(FAILURE); /* let the RTA app set the error */
	            }
	            break;

	        /* 8 Bit Instance */
	        case LOGSEG_8BITINST:
	            /* This must be the second path */
	            if( (class_found == TRUE)  &&
		            ( inst_found == FALSE) &&
		            ( attr_found == FALSE) )
	            {
                    inst_found = TRUE;
                    inst_id = path_val;
	            }

	            /* Bad Path order */
	            else
	            {
                    return(FAILURE); /* let the RTA app set the error */
	            }
                break;

	        /* 16 Bit Instance */
	        case LOGSEG_16BITINST:
	            /* This must be the second path */
	            if( (class_found == TRUE)  &&
		            ( inst_found == FALSE) &&
		            ( attr_found == FALSE) )
	            {
                    inst_found = TRUE;
                    i++; /* the class id is stored in the next word) */
	                inst_id = (uint16)(request->path[i]);
	            }

	            /* Bad Path order */
	            else
	            {
                    return(FAILURE); /* let the RTA app set the error */
	            }
                break;

	        /* 8 Bit Attribute */
	        case LOGSEG_8BITATTR:
	            /* This must be the third path */
	            if( (class_found == TRUE) &&
		            ( inst_found == TRUE) &&
		            ( attr_found == FALSE) )
	            {
	                attr_found = TRUE;
	                attr_id = path_val;
	            }

	            /* Bad Path order */
	            else
	            {
                    return(FAILURE); /* let the RTA app set the error */
	            }
	            break;

	        /* ERROR with path */
	        default:
                    return(FAILURE); /* let the RTA app set the error */
        }; /* END-> "switch (path_seg)" */
    }

    /* This function code requires a valid class, instance, attribute */
    if((class_found == FALSE) || (inst_found == FALSE))
    {
        return(FAILURE); /* let the RTA app set the error */
    }

    // validate the object exists
    if( (class_id != VS_OBJ_DISCRETE_INPUT_DATA) &&
        (class_id != VS_OBJ_DISCRETE_OUTPUT_DATA) )
    {
        return(FAILURE); /* let the RTA app set the error */
    }

    // at this point we can take over the error code handling....

    /* switch on the service code */
    switch(request->service)
    {
        /* Get_Attribute_Single Service Code */
        case CIP_SC_GET_ATTR_SINGLE:
            /* make sure we have the attribute */
            if(attr_found == TRUE)
                local_get_attr (class_id, inst_id, attr_id, response);
            else
                return(FAILURE); /* let the RTA app set the error */
            break;

        /* Set_Attribute_Single Service Code */
        case CIP_SC_SET_ATTR_SINGLE:
            /* make sure we have the attribute */
            if(attr_found == TRUE)
                local_set_attr (class_id, inst_id, attr_id, request->req_data_size, request->req_data, response);
            else
                return(FAILURE); /* let the RTA app set the error */
            break;

        default:
            /* Error: Path Destination Unknown */
            response->gen_stat = ERR_SERV_UNSUPP;
	        return(SUCCESS); /* exit on an error */
    };

    return(SUCCESS);
}

/**/
/* ******************************************************************** */
/*			LOCAL FUNCTIONS 				*/
/* ******************************************************************** */
/* ====================================================================
Function:   local_get_attr
Parameters: 16-bit class id
            16-bit instance id
            8-bit attribute id
            Pointer to Message Router Response structure
Returns:    N/A

This function is called to process the Get Attribute Single request.
======================================================================= */
void local_get_attr (uint16 class_id, uint16 inst_id, uint8 attr_id, EIPS_USER_MSGRTR_RSP_FMT *response)
{
    uint16 i;

//    eips_user_dbprint3("VS_GetSingle Class 0x%02x Inst 0x%02x, Attr 0x%02x\r\n", class_id, inst_id, attr_id);

    // switch on the class id
    switch(class_id)
    {
        // ****************************************************************************
        //			     Discrete Input Data Object
        // ****************************************************************************
        case VS_OBJ_DISCRETE_INPUT_DATA:
            // Validate the Instance ID
            if(inst_id > 1) // class and 1 instance
            {
                /* Error: Object Does Not Exist */
                response->gen_stat = ERR_UNEXISTANT_OBJ;
                return;
            }

            // Class Attributes
            if(inst_id == 0)
            {
                // switch on the Attribute ID
                switch(attr_id)
                {
                    case 1: // Revision (uint16)
                        rta_PutLitEndian16(eips_vs_discreteInDataObj.Class.ClassRev, response->rsp_data);
                        response->rsp_data_size = 2;
                        break;
                        
                    // Attribute Unsupported
                    default:
                        /* Error: Attribute Not Supported */
                        response->gen_stat = ERR_ATTR_UNSUPP;
                        break;
                }
            }

            // Instance Attributes
            else
            {
                // switch on the Attribute ID
                switch(attr_id)
                {
                    case 1: // Number of Input Units (uint16)
                        rta_PutLitEndian16(eips_vs_discreteInDataObj.Inst.DataSize, response->rsp_data);
                        response->rsp_data_size = 2;
                        break;

	                case 3: // Input Data (uint8[], uint16[] or uint32[])
                        response->rsp_data_size = 0;

                        for(i=0; i<eips_vs_discreteInDataObj.Inst.DataSize; i++)
                        {
#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
                            rta_PutLitEndian32(eips_vs_discreteInDataObj.Inst.DataPtr[i], (response->rsp_data+response->rsp_data_size));
                            response->rsp_data_size += 4;
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
                            rta_PutLitEndian16(eips_vs_discreteInDataObj.Inst.DataPtr[i], (response->rsp_data+response->rsp_data_size));
                            response->rsp_data_size += 2;
#else                                       /* BYTES */
                            response->rsp_data[response->rsp_data_size] = eips_vs_discreteInDataObj.Inst.DataPtr[i];
                            response->rsp_data_size += 1;
#endif
                        }
	                    break;

                    // Attribute Unsupported
                    default:
                        /* Error: Attribute Not Supported */
                        response->gen_stat = ERR_ATTR_UNSUPP;
                        break;
                }
            }
            break;

        // ********************************************************************
        //			     Discrete Output Data Object
        // ********************************************************************
        case VS_OBJ_DISCRETE_OUTPUT_DATA:
            // Validate the Instance ID
            if(inst_id > 1) // class and 1 instance
            {
                /* Error: Object Does Not Exist */
                response->gen_stat = ERR_UNEXISTANT_OBJ;
                return;
            }

            // Class Attributes
            if(inst_id == 0)
            {
                // switch on the Attribute ID
                switch(attr_id)
                {
                    case 1: // Revision (uint16)
                        rta_PutLitEndian16(eips_vs_discreteOutDataObj.Class.ClassRev, response->rsp_data);
                        response->rsp_data_size = 2;
                        break;
                        
                    // Attribute Unsupported
                    default:
                        /* Error: Attribute Not Supported */
                        response->gen_stat = ERR_ATTR_UNSUPP;
                        break;
                }
            }

            // Instance Attributes
            else
            {
                // switch on the Attribute ID
                switch(attr_id)
                {
                    case 1: // Number of Output Units (uint16)
                        rta_PutLitEndian16(eips_vs_discreteOutDataObj.Inst.DataSize, response->rsp_data);
                        response->rsp_data_size = 2;
                        break;

	                case 3: // Output Data (uint8[], uint16[] or uint32[])
                        response->rsp_data_size = 0;
                        for(i=0; i<eips_vs_discreteOutDataObj.Inst.DataSize; i++)
                        {
#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
                            rta_PutLitEndian32(eips_vs_discreteOutDataObj.Inst.DataPtr[i], (response->rsp_data+response->rsp_data_size));
                            response->rsp_data_size += 4;
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
                            rta_PutLitEndian16(eips_vs_discreteOutDataObj.Inst.DataPtr[i], (response->rsp_data+response->rsp_data_size));
                            response->rsp_data_size += 2;
#else                                       /* BYTES */
                            response->rsp_data[response->rsp_data_size] = eips_vs_discreteOutDataObj.Inst.DataPtr[i];
                            response->rsp_data_size += 1;
#endif
                        }
	                    break;

                    // Attribute Unsupported
                    default:
                        /* Error: Attribute Not Supported */
                        response->gen_stat = ERR_ATTR_UNSUPP;
                        break;
                }
            }
            break;       

        // ********************************************************************
        //			     Unknown Object
        // ********************************************************************
        default:
	        /* Error: Object Does Not Exist */
	        response->gen_stat = ERR_UNEXISTANT_OBJ;
	        break;
    };
}

/* ====================================================================
Function:   local_set_attr
Parameters: 16-bit class id
            16-bit instance id
	        8-bit attribute id
	        data buffer size
	        data buffer
	        Pointer to Message Router Response structure
Returns:    N/A

This function is called to process the Get Attribute Single request.
======================================================================= */
void local_set_attr (uint16 class_id, uint16 inst_id, uint8 attr_id, uint16 data_siz, uint8 *data_ptr, EIPS_USER_MSGRTR_RSP_FMT *response)
{
    uint16 i;
//    eips_user_dbprint3("VS_SetSingle Class 0x%02x Inst 0x%02x, Attr 0x%02x\r\n", class_id, inst_id, attr_id);

    // switch on the class id
    switch(class_id)
    {
        // ****************************************************************************
        //			     Discrete Input Data Object
        //			     Analog Input Data Object
        // ****************************************************************************
        case VS_OBJ_DISCRETE_INPUT_DATA:
            // Validate the Instance ID
            if(inst_id > 1) // class and 1 instance
            {
                /* Error: Object Does Not Exist */
                response->gen_stat = ERR_UNEXISTANT_OBJ;
                return;
	        }

            // Service isn't supported for this object
   	        response->gen_stat = ERR_SERV_UNSUPP;
	        return;
	      
	        // break; // this isn't needed since we return

        // ****************************************************************************
        //			     Discrete Output Data Object
        // ****************************************************************************
        case VS_OBJ_DISCRETE_OUTPUT_DATA:
            // Validate the Instance ID
            if(inst_id > 1) // class and 1 instance
            {
    	        /* Error: Object Does Not Exist */
	            response->gen_stat = ERR_UNEXISTANT_OBJ;
  	            return;
	        }

	        // Class Attributes
	        if(inst_id == 0)
	        {
                // Service isn't supported for the class instance
   	            response->gen_stat = ERR_SERV_UNSUPP;	    
   	            return;
   	        }
   	        
   	        // Instance Attributes
   	        else
   	        {
   	            // data byte 0 is the Attribute ID
   	            switch(attr_id)
   	            {
   	                /* ***************************** */
   	                /* Error: Attribute(s) Read Only */
   	                /* ***************************** */
                    case 1: // Number of Output Words (uint16)
                        response->gen_stat = ERR_ATTR_READONLY;
                        return;

	                case 3: // Output Data (uint16[])
                        // validate we don't have an I/O connection (else "Device State Conflict")
                        if(eips_iomsg_allocated(local_o2t_asm_struct[0].instID) == TRUE)
                        {
                            // "Device State Conflict"
                            response->gen_stat = ERR_DEV_STATE_CONFLICT;
                            return; // return on error
                        }

#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
	                    // validate the data length (function builds the err msg if needed)
	                    if(local_dlc_valid (data_siz, (uint16)(eips_vs_discreteOutDataObj.Inst.DataSize*4), response) == FALSE)
	                        return; // return on error

	                    // we don't validate the data, just store it
                        for(i=0; i<eips_vs_discreteOutDataObj.Inst.DataSize; i++)
                        {
                            eips_vs_discreteOutDataObj.Inst.DataPtr[i] = rta_GetLitEndian32((data_ptr+(i*4)));                        

                            // update the assembly with the output data
                            local_o2t_asm_struct[0].data_ptr[i] = eips_vs_discreteOutDataObj.Inst.DataPtr[i];
                        }

#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
	                    // validate the data length (function builds the err msg if needed)
	                    if(local_dlc_valid (data_siz, (uint16)(eips_vs_discreteOutDataObj.Inst.DataSize*2), response) == FALSE)
	                        return; // return on error

	                    // we don't validate the data, just store it
                        for(i=0; i<eips_vs_discreteOutDataObj.Inst.DataSize; i++)
                        {
                            eips_vs_discreteOutDataObj.Inst.DataPtr[i] = rta_GetLitEndian16((data_ptr+(i*2)));                        

#if EIPS_USEROBJ_ASM_MAXNUM_O2TINST > 0  
                            // update the assembly with the output data
                            local_o2t_asm_struct[0].data_ptr[i] = eips_vs_discreteOutDataObj.Inst.DataPtr[i];
#endif
                        }

#else                                       /* BYTES */
	                    // validate the data length (function builds the err msg if needed)
	                    if(local_dlc_valid (data_siz, (uint16)(eips_vs_discreteOutDataObj.Inst.DataSize*2), response) == FALSE)
	                        return; // return on error

	                    // we don't validate the data, just store it
                        for(i=0; i<eips_vs_discreteOutDataObj.Inst.DataSize; i++)
                        {
                            eips_vs_discreteOutDataObj.Inst.DataPtr[i] = data_ptr[i];                        

                            // update the assembly with the output data
                            local_o2t_asm_struct[0].data_ptr[i] = eips_vs_discreteOutDataObj.Inst.DataPtr[i];
                        }
#endif
                        break;
                        
                    // Attribute Unsupported
                    default:
                        /* Error: Attribute Not Supported */
                        response->gen_stat = ERR_ATTR_UNSUPP;
                        break;
                }; // END-> "switch(attr_id)"      
            } // END-> else "Instance Attributes" 
            break;

        default:
            /* Error: Object Does Not Exist */
            response->gen_stat = ERR_UNEXISTANT_OBJ;
	        break;
    }; // END-> "switch(class_id)"
}

/* ====================================================================
Function:   local_dlc_valid
Parameters: actual size
            needed size
            pointer for the error message (if needed)
Returns:    TRUE/FALSE

This function is called to start the SPI task.
======================================================================= */
uint8 local_dlc_valid (uint16 actual_siz, uint16 needed_siz, EIPS_USER_MSGRTR_RSP_FMT *response)
{
    /* Error: Not Enough Data */
    if(actual_siz < needed_siz)
    {
        response->gen_stat = ERR_INSUFF_DATA;
        return(FALSE);
    }

    /* Error: Too Much Data */
    if(actual_siz > needed_siz)
    {
        response->gen_stat = ERR_TOOMUCH_DATA;
        return(FALSE);
    }

    /* size is correct */
    return(TRUE);
}
/* *********** */
/* END OF FILE */
/* *********** */

