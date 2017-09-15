/*
 *            Copyright (c) 2002-2010 by Real Time Automation, Inc.
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
 *    Version Date: 11/10/2010
 *         Version: 2.19
 *    Conformed To: EtherNet/IP Protocol Conformance Test A-8 (28-OCT-2010)
 *     Module Name: eips_usersys.c
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains system functions that need to be written by the user.
 * This includes timer, NVRAM, and task calls.
 *
 */

/* ---------------------------- */
/* INCLUDE FILES                */
/* ---------------------------- */
#include "eips_system.h"

#ifdef EIPS_TACL_OBJ_USED

/* used for debug */
#undef  __RTA_FILE__
#define __RTA_FILE__ "eips_tacl.c"

/* ---------------------------- */
/* EXTERN FUNCTIONS             */
/* ---------------------------- */

/* ---------------------------- */
/* LOCAL STRUCTURE DEFINITIONS  */
/* ---------------------------- */

/* ---------------------------- */
/* STATIC VARIABLES             */
/* ---------------------------- */
static EIPS_TACL_OBJ_STRUCT TaCLObj;

/* ---------------------------- */
/* EXTERN VARIABLES             */
/* ---------------------------- */

/* ---------------------------- */
/* LOCAL FUNCTIONS              */
/* ---------------------------- */
static void local_get_attr (uint16 class_id, uint16 inst_id, uint8 attr_id, EIPS_USER_MSGRTR_RSP_FMT *response);
static void local_cnxn_read (uint16 class_id, uint16 inst_id, uint16 data_siz, uint8 *data_ptr, EIPS_USER_MSGRTR_RSP_FMT *response);

/* ---------------------------- */
/* MISCELLANEOUS                */
/* ---------------------------- */

/**/
/* ******************************************************************** */
/*                  GLOBAL FUNCTIONS CALLED BY RTA                      */
/* ******************************************************************** */
/* ====================================================================
Function:   eips_usersys_init
Parameters: init type
Returns:    N/A

This function initialize all user system variables.
======================================================================= */
void  eips_TaCL_init (uint8 init_type)
{
	uint16 i, num_inst_total, num_inst_per_group;
	uint16 next_cip_inst = 10;

	memset(&TaCLObj, 0, sizeof(TaCLObj));

	TaCLObj.ClassLevel.ClassRev = 1;
	TaCLObj.ClassLevel.MaxInst = 0;
	num_inst_total = 0;
	num_inst_per_group = 0;

	/* *************************** */
	/* Exclusive Owner Connections */
	/* *************************** */
#if	EIPS_TACL_OBJ_MAX_NUM_EXOWNER_CNXNS > 0
	// get the groupings from the user
	num_inst_per_group = eips_userobj_TaCL_GetGroupings (EIPS_TACL_OBJ_INST_TYPE_EXOWNER, &TaCLObj.group[num_inst_total], EIPS_TACL_OBJ_MAX_NUM_EXOWNER_CNXNS);

	// store the default instance
	if(num_inst_per_group)
	{
		TaCLObj.InstLevel[num_inst_total].InstID = 1;
		TaCLObj.InstLevel[num_inst_total].InstType = EIPS_TACL_OBJ_INST_TYPE_EXOWNER;
		num_inst_total++;
	}

	// store additional instances
	for(i=1; i<num_inst_per_group; i++)
	{
		TaCLObj.InstLevel[num_inst_total].InstID = next_cip_inst++;
		TaCLObj.InstLevel[num_inst_total].InstType = EIPS_TACL_OBJ_INST_TYPE_EXOWNER;
		num_inst_total++;
	}
#endif

	/* ********************** */
	/* Input Only Connections */
	/* ********************** */
#if	EIPS_TACL_OBJ_MAX_NUM_INONLY_CNXNS > 0
	// get the groupings from the user
	num_inst_per_group = eips_userobj_TaCL_GetGroupings (EIPS_TACL_OBJ_INST_TYPE_INONLY, &TaCLObj.group[num_inst_total], EIPS_TACL_OBJ_MAX_NUM_INONLY_CNXNS);

	// store the default instance
	if(num_inst_per_group)
	{
		TaCLObj.InstLevel[num_inst_total].InstID = 2;
		TaCLObj.InstLevel[num_inst_total].InstType = EIPS_TACL_OBJ_INST_TYPE_INONLY;
		num_inst_total++;
	}

	// store additional instances
	for(i=1; i<num_inst_per_group; i++)
	{
		TaCLObj.InstLevel[num_inst_total].InstID = next_cip_inst++;
		TaCLObj.InstLevel[num_inst_total].InstType = EIPS_TACL_OBJ_INST_TYPE_INONLY;
		num_inst_total++;
	}
#endif

	/* *********************** */
	/* Listen Only Connections */
	/* *********************** */
#if	EIPS_TACL_OBJ_MAX_NUM_LISTENONLY_CNXNS > 0
	// get the groupings from the user
	num_inst_per_group = eips_userobj_TaCL_GetGroupings (EIPS_TACL_OBJ_INST_TYPE_LISTENONLY, &TaCLObj.group[num_inst_total], EIPS_TACL_OBJ_MAX_NUM_LISTENONLY_CNXNS);

	// store the default instance
	if(num_inst_per_group)
	{
		TaCLObj.InstLevel[num_inst_total].InstID = 3;
		TaCLObj.InstLevel[num_inst_total].InstType = EIPS_TACL_OBJ_INST_TYPE_LISTENONLY;
		num_inst_total++;
	}

	// store additional instances
	for(i=1; i<num_inst_per_group; i++)
	{
		TaCLObj.InstLevel[num_inst_total].InstID = next_cip_inst++;
		TaCLObj.InstLevel[num_inst_total].InstType = EIPS_TACL_OBJ_INST_TYPE_LISTENONLY;
		num_inst_total++;
	}
#endif

	/* *********************** */
	/* Output Only Connections */
	/* *********************** */
#if	EIPS_TACL_OBJ_MAX_NUM_OUTONLY_CNXNS > 0
	// get the groupings from the user
	num_inst_per_group = eips_userobj_TaCL_GetGroupings (EIPS_TACL_OBJ_INST_TYPE_OUTONLY, &TaCLObj.group[num_inst_total], EIPS_TACL_OBJ_MAX_NUM_OUTONLY_CNXNS);

	// no default

	// store additional instances
	for(i=0; i<num_inst_per_group; i++)
	{
		TaCLObj.InstLevel[num_inst_total].InstID = next_cip_inst++;
		TaCLObj.InstLevel[num_inst_total].InstType = EIPS_TACL_OBJ_INST_TYPE_OUTONLY;
		num_inst_total++;
	}
#endif

	/* ************************* */
	/* Data Exchange Connections */
	/* ************************* */
#if	EIPS_TACL_OBJ_MAX_NUM_DATAEXCHANGE_CNXNS > 0
	// get the groupings from the user
	num_inst_per_group = eips_userobj_TaCL_GetGroupings (EIPS_TACL_OBJ_INST_TYPE_DATAEXCHANGE, &TaCLObj.group[num_inst_total], EIPS_TACL_OBJ_MAX_NUM_DATAEXCHANGE_CNXNS);

	// store the default instance
	if(num_inst_per_group)
	{
		TaCLObj.InstLevel[num_inst_total].InstID = 4;
		TaCLObj.InstLevel[num_inst_total].InstType = EIPS_TACL_OBJ_INST_TYPE_DATAEXCHANGE;
		num_inst_total++;
	}

	// store additional instances
	for(i=1; i<num_inst_per_group; i++)
	{
		TaCLObj.InstLevel[num_inst_total].InstID = next_cip_inst++;
		TaCLObj.InstLevel[num_inst_total].InstType = EIPS_TACL_OBJ_INST_TYPE_DATAEXCHANGE;
		num_inst_total++;
	}
#endif

	/* find the max instance */
	for(i=0; i<EIPS_TACL_OBJ_MAX_NUM_INST; i++)
	{
		if(TaCLObj.InstLevel[i].InstID > TaCLObj.ClassLevel.MaxInst)
			TaCLObj.ClassLevel.MaxInst = TaCLObj.InstLevel[i].InstID;

		//printf("TaCL[%d] Inst=%d Type=%d O2T:%d T2O:%d CFG:%d\r\n",i, TaCLObj.InstLevel[i].InstID, TaCLObj.InstLevel[i].InstType, TaCLObj.group[i].o2t_inst, TaCLObj.group[i].t2o_inst, TaCLObj.group[i].cfg_inst);
	}

}

/* ====================================================================
Function:   eips_TaCL_procObject
Parameters: Pointer to Message Router Request structure
            Pointer to Message Router Response structure
Returns:    SUCCESS - Use response in "response" structure
            FAILURE - Use default error "Object does not exist"

This function passes the Message Router Request and Response structures.
If the user supports any vendor specific objects, parse the message,
validate the passed request and build a response message in the passed
response structure.  If the passed object isn't supported, return
FAILURE and don't fill in the passed response structure.
======================================================================= */
uint8 eips_TaCL_procObject (EIPS_USER_MSGRTR_REQ_FMT *request, EIPS_USER_MSGRTR_RSP_FMT *response)
{
    uint16  class_id, inst_id, size_ix;
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

    /* validate the object exists */
    if(class_id != CLASS_TACL)
    {
        return(FAILURE); /* let the RTA app set the error */
    }

    /* at this point we can take over the error code handling.... */

    /* switch on the service code */
    switch(request->service)
    {
		case CIP_SC_TACL_CNXN_READ:
            /* make sure we have the attribute */
            if(attr_found == TRUE)
                return(FAILURE); /* let the RTA app set the error */
            else
                local_cnxn_read (class_id, inst_id, request->req_data_size, request->req_data, response);
            break;

        /* Get_Attribute_Single Service Code */
        case CIP_SC_GET_ATTR_SINGLE:
            /* make sure we have the attribute */
            if(attr_found == TRUE)
                local_get_attr (class_id, inst_id, attr_id, response);
            else
                return(FAILURE); /* let the RTA app set the error */
            break;

#if 0
        /* Set_Attribute_Single Service Code */
        case CIP_SC_SET_ATTR_SINGLE:
            /* make sure we have the attribute */
            if(attr_found == TRUE)
                local_set_attr (class_id, inst_id, attr_id, request->req_data_size, request->req_data, response);
            else
                return(FAILURE); /* let the RTA app set the error */
            break;
#endif

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
static void local_get_attr (uint16 class_id, uint16 inst_id, uint8 attr_id, EIPS_USER_MSGRTR_RSP_FMT *response)
{
    uint16 i, inst_ix;

/*    eips_user_dbprint3("VS_GetSingle Class 0x%02x Inst 0x%02x, Attr 0x%02x\r\n", class_id, inst_id, attr_id); */

    /* switch on the class id */
    switch(class_id)
    {
        /* **************************************************************************** */
        /*			     Target Connection List Object 									*/
        /* **************************************************************************** */
        case CLASS_TACL:
            /* Validate the Instance ID */
            if(inst_id > TaCLObj.ClassLevel.MaxInst) /* class and 1 instance */
            {
                /* Error: Object Does Not Exist */
                response->gen_stat = ERR_UNEXISTANT_OBJ;
                return;
            }

            /* Class Attributes */
            if(inst_id == 0)
            {
                /* switch on the Attribute ID */
                switch(attr_id)
                {
                    case 1: /* Revision (uint16) */
                        rta_PutLitEndian16(TaCLObj.ClassLevel.ClassRev, response->rsp_data);
                        response->rsp_data_size = 2;
                        break;

                    case 2: /* Max Inst (uint16) */
                        rta_PutLitEndian16(TaCLObj.ClassLevel.MaxInst, response->rsp_data);
                        response->rsp_data_size = 2;
                        break;

                    /* Attribute Unsupported */
                    default:
                        /* Error: Attribute Not Supported */
                        response->gen_stat = ERR_ATTR_UNSUPP;
                        break;
                }
            }

            /* Instance Attributes */
            else
            {
            	/* find the instance */
            	for(i=0, inst_ix = EIPS_TACL_OBJ_MAX_NUM_INST; ((i<EIPS_TACL_OBJ_MAX_NUM_INST) && (inst_ix == EIPS_TACL_OBJ_MAX_NUM_INST)); i++)
            	{
            		if(TaCLObj.InstLevel[i].InstID == inst_id)
            			inst_ix = i;
            	}

            	/* instance not found */
            	if(inst_ix == EIPS_TACL_OBJ_MAX_NUM_INST)
            	{
					 /* Error: Object Does Not Exist */
					response->gen_stat = ERR_UNEXISTANT_OBJ;
					return;
            	}

                /* switch on the Attribute ID */
                switch(attr_id)
                {
					case 10:
						response->rsp_data[0] = RTA_MIN(sizeof(TaCLObj.group[inst_ix].name), strlen(TaCLObj.group[inst_ix].name));
						rta_ByteMove(&response->rsp_data[1], (uint8*)TaCLObj.group[inst_ix].name, response->rsp_data[0]);
						response->rsp_data_size = (1 + response->rsp_data[0]);
						break;

                    /* Attribute Unsupported */
                    default:
                        /* Error: Attribute Not Supported */
                        response->gen_stat = ERR_ATTR_UNSUPP;
                        break;
                }
            }
            break;

        /* ******************************************************************** */
        /*			     Unknown Object */
        /* ******************************************************************** */
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
#if 0
void local_set_attr (uint16 class_id, uint16 inst_id, uint8 attr_id, uint16 data_siz, uint8 *data_ptr, EIPS_USER_MSGRTR_RSP_FMT *response)
{
    uint16 i;
/*    eips_user_dbprint3("VS_SetSingle Class 0x%02x Inst 0x%02x, Attr 0x%02x\r\n", class_id, inst_id, attr_id); */

    /* switch on the class id */
    switch(class_id)
    {
        /* **************************************************************************** */
        /*			     Discrete Input Data Object */
        /*			     Analog Input Data Object */
        /* **************************************************************************** */
        case VS_OBJ_DISCRETE_INPUT_DATA:
            /* Validate the Instance ID */
            if(inst_id > 1) /* class and 1 instance */
            {
                /* Error: Object Does Not Exist */
                response->gen_stat = ERR_UNEXISTANT_OBJ;
                return;
	        }

            /* Service isn't supported for this object */
   	        response->gen_stat = ERR_SERV_UNSUPP;
	        return;

	        /* break;*/ /* this isn't needed since we return */

        /* **************************************************************************** */
        /*			     Discrete Output Data Object */
        /* **************************************************************************** */
        case VS_OBJ_DISCRETE_OUTPUT_DATA:
            /* Validate the Instance ID */
            if(inst_id > 1) /* class and 1 instance */
            {
    	        /* Error: Object Does Not Exist */
	            response->gen_stat = ERR_UNEXISTANT_OBJ;
  	            return;
	        }

	        /* Class Attributes */
	        if(inst_id == 0)
	        {
                /* Service isn't supported for the class instance */
   	            response->gen_stat = ERR_SERV_UNSUPP;
   	            return;
   	        }

   	        /* Instance Attributes */
   	        else
   	        {
   	            /* data byte 0 is the Attribute ID */
   	            switch(attr_id)
   	            {
   	                /* ***************************** */
   	                /* Error: Attribute(s) Read Only */
   	                /* ***************************** */
                    case 1: /* Number of Output Words (uint16) */
                        response->gen_stat = ERR_ATTR_READONLY;
                        return;

	                case 3: /* Output Data (uint16[]) */
                        /* validate we don't have an I/O connection (else "Device State Conflict") */
                        if(eips_iomsg_allocated(local_o2t_asm_struct[0].instID) == TRUE)
                        {
                            /* "Device State Conflict" */
                            response->gen_stat = ERR_DEV_STATE_CONFLICT;
                            return; /* return on error */
                        }

#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
	                    /* validate the data length (function builds the err msg if needed) */
	                    if(local_dlc_valid (data_siz, (uint16)(eips_vs_discreteOutDataObj.Inst.DataSize*4), response) == FALSE)
	                        return; /* return on error */

	                    /* we don't validate the data, just store it */
                        for(i=0; i<eips_vs_discreteOutDataObj.Inst.DataSize; i++)
                        {
                            eips_vs_discreteOutDataObj.Inst.DataPtr[i] = rta_GetLitEndian32((data_ptr+(i*4)));

                            /* update the assembly with the output data */
                            local_o2t_asm_struct[0].data_ptr[i] = eips_vs_discreteOutDataObj.Inst.DataPtr[i];
                        }

#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
	                    /* validate the data length (function builds the err msg if needed) */
	                    if(local_dlc_valid (data_siz, (uint16)(eips_vs_discreteOutDataObj.Inst.DataSize*2), response) == FALSE)
	                        return; /* return on error */

	                    /* we don't validate the data, just store it */
                        for(i=0; i<eips_vs_discreteOutDataObj.Inst.DataSize; i++)
                        {
                            eips_vs_discreteOutDataObj.Inst.DataPtr[i] = rta_GetLitEndian16((data_ptr+(i*2)));

#if EIPS_USEROBJ_ASM_MAXNUM_O2TINST > 0
                            /* update the assembly with the output data */
                            local_o2t_asm_struct[0].data_ptr[i] = eips_vs_discreteOutDataObj.Inst.DataPtr[i];
#endif
                        }

#else                                       /* BYTES */
	                    /* validate the data length (function builds the err msg if needed) */
	                    if(local_dlc_valid (data_siz, (uint16)(eips_vs_discreteOutDataObj.Inst.DataSize*2), response) == FALSE)
	                        return; /* return on error */

	                    /* we don't validate the data, just store it */
                        for(i=0; i<eips_vs_discreteOutDataObj.Inst.DataSize; i++)
                        {
                            eips_vs_discreteOutDataObj.Inst.DataPtr[i] = data_ptr[i];

                            /* update the assembly with the output data */
                            local_o2t_asm_struct[0].data_ptr[i] = eips_vs_discreteOutDataObj.Inst.DataPtr[i];
                        }
#endif
                        break;

                    /* Attribute Unsupported */
                    default:
                        /* Error: Attribute Not Supported */
                        response->gen_stat = ERR_ATTR_UNSUPP;
                        break;
                }; /* END-> "switch(attr_id)"       */
            } /* END-> else "Instance Attributes"  */
            break;

        default:
            /* Error: Object Does Not Exist */
            response->gen_stat = ERR_UNEXISTANT_OBJ;
	        break;
    }; /* END-> "switch(class_id)" */
}
#endif

/* ====================================================================
Function:   local_cnxn_read
Parameters: 16-bit class id
            16-bit instance id
	        data buffer size
	        data buffer
	        Pointer to Message Router Response structure
Returns:    N/A

This function is called to process the Connection Read request.
======================================================================= */
static void local_cnxn_read (uint16 class_id, uint16 inst_id, uint16 data_siz, uint8 *data_ptr, EIPS_USER_MSGRTR_RSP_FMT *response)
{
	uint32 	temp32;
    uint16 	i, size_ix, inst_ix;
    uint16 	o2t_size, t2o_size, cfg_size;
#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 1
    uint8	*o2t_ptr, *t2o_ptr, *cfg_ptr;
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2
    uint16	*o2t_ptr, *t2o_ptr, *cfg_ptr;
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4
    uint32	*o2t_ptr, *t2o_ptr, *cfg_ptr;
#else
    #error "EIPS_USEROBJ_ASM_ELEMENT_SIZE invalid"
#endif

/*    eips_user_dbprint2("VS_Connection Read Class 0x%02x Inst 0x%02x\r\n", class_id, inst_id); */

    /* switch on the class id */
    switch(class_id)
    {
        /* **************************************************************************** */
        /*			     Target Connection List Object 									*/
        /* **************************************************************************** */
        case CLASS_TACL:
            /* Validate the Instance ID */
            if(inst_id > TaCLObj.ClassLevel.MaxInst) /* class and 1 instance */
            {
                /* Error: Object Does Not Exist */
                response->gen_stat = ERR_UNEXISTANT_OBJ;
                return;
            }

            /* Class Level */
            if(inst_id == 0)
            {

            }

            /* Instance Level */
            else
            {
            	/* find the instance */
            	for(i=0, inst_ix = EIPS_TACL_OBJ_MAX_NUM_INST; ((i<EIPS_TACL_OBJ_MAX_NUM_INST) && (inst_ix == EIPS_TACL_OBJ_MAX_NUM_INST)); i++)
            	{
            		if(TaCLObj.InstLevel[i].InstID == inst_id)
            			inst_ix = i;
            	}

            	/* instance not found */
            	if(inst_ix == EIPS_TACL_OBJ_MAX_NUM_INST)
            	{
					 /* Error: Object Does Not Exist */
					response->gen_stat = ERR_UNEXISTANT_OBJ;
					return;
            	}

            	/* get the size and pointers for each instance */
            	o2t_ptr = eips_userobj_getAsmPtr(TaCLObj.group[inst_ix].o2t_inst, &o2t_size); // get size and ptr
            	o2t_size *= EIPS_USEROBJ_ASM_ELEMENT_SIZE; // adjust for element alignment
            	t2o_ptr = eips_userobj_getAsmPtr(TaCLObj.group[inst_ix].t2o_inst, &t2o_size); // get size and ptr
            	t2o_size *= EIPS_USEROBJ_ASM_ELEMENT_SIZE; // adjust for element alignment
            	cfg_ptr = eips_userobj_getAsmPtr(TaCLObj.group[inst_ix].cfg_inst, &cfg_size); // get size and ptr
            	cfg_size *= EIPS_USEROBJ_ASM_ELEMENT_SIZE; // adjust for element alignment

            	/* Connection Read Instance Level */
				response->rsp_data_size = 0;

				/* Number of Connections (uint16) */
				rta_PutLitEndian16(1, response->rsp_data+response->rsp_data_size);
				response->rsp_data_size += 2;

				/* Structure Size (uint16) */
				size_ix = response->rsp_data_size;
				//rta_PutLitEndian16((response->rsp_data_size), response->rsp_data+size_ix);
				response->rsp_data_size += 2;

				/* Connection Index (uint32) */
				rta_PutLitEndian32(0, response->rsp_data+response->rsp_data_size);
				response->rsp_data_size += 4;

				/* O2T Run/Idle Header (uint8) */
				switch(TaCLObj.InstLevel[inst_ix].InstType)
				{
					case EIPS_TACL_OBJ_INST_TYPE_EXOWNER:
					case EIPS_TACL_OBJ_INST_TYPE_DATAEXCHANGE:
					case EIPS_TACL_OBJ_INST_TYPE_OUTONLY:
						response->rsp_data[response->rsp_data_size++] = 1; // include header
						o2t_size+=6; // add seq_num (2) and header (4)
						break;

					case EIPS_TACL_OBJ_INST_TYPE_INONLY:
					case EIPS_TACL_OBJ_INST_TYPE_LISTENONLY:
					default:
						response->rsp_data[response->rsp_data_size++] = 0; // no header
						o2t_size+=2; // add seq_num (2)
						break;
				};

				/* T2O Run/Idle Header (uint8) */
#ifdef EIPS_USEROBJ_T2O_RUNTIME_HEADER_USED
				switch(TaCLObj.InstLevel[inst_ix].InstType)
				{
					case EIPS_TACL_OBJ_INST_TYPE_INONLY:
					case EIPS_TACL_OBJ_INST_TYPE_LISTENONLY:
					case EIPS_TACL_OBJ_INST_TYPE_EXOWNER:
					case EIPS_TACL_OBJ_INST_TYPE_DATAEXCHANGE:
						response->rsp_data[response->rsp_data_size++] = 1; // include header
						t2o_size+=6; // add seq_num (2) and header (4)
						break;

					case EIPS_TACL_OBJ_INST_TYPE_OUTONLY:
					default:
						response->rsp_data[response->rsp_data_size++] = 0; // no header
						t2o_size+=2; // add seq_num (2)
						break;
				};
#else
				response->rsp_data[response->rsp_data_size++] = 0; // no header
				t2o_size+=2; // add seq_num (2)
#endif

				/* Connection Timeout Multiplier (uint8) */
				response->rsp_data[response->rsp_data_size++] = EIPS_TACL_OBJ_DFLT_CNXNTMOMULT;

				/* Reserved (uint8[3]) */
				response->rsp_data[response->rsp_data_size++] = 0;
				response->rsp_data[response->rsp_data_size++] = 0;
				response->rsp_data[response->rsp_data_size++] = 0;

				/* O2T RPI (uint32) */
				rta_PutLitEndian32(EIPS_TACL_OBJ_DFLT_O2T_RPIUSEC, response->rsp_data+response->rsp_data_size);
				response->rsp_data_size += 4;

				/* O2T Net Params (uint32) */
				temp32 = (uint16)EIPS_TACL_OBJ_DFLT_O2T_NETPARAM;
				temp32 = (uint32)((temp32 << 16) | ((uint32)o2t_size));
				rta_PutLitEndian32(temp32, response->rsp_data+response->rsp_data_size);
				response->rsp_data_size += 4;

				/* T2O RPI (uint32) */
				rta_PutLitEndian32(EIPS_TACL_OBJ_DFLT_T2O_RPIUSEC, response->rsp_data+response->rsp_data_size);
				response->rsp_data_size += 4;

				/* T2O Net Params (uint32) */
				temp32 = (uint16)EIPS_TACL_OBJ_DFLT_T2O_NETPARAM;
				temp32 = (uint32)((temp32 << 16) | ((uint32)t2o_size));
				rta_PutLitEndian32(temp32, response->rsp_data+response->rsp_data_size);
				response->rsp_data_size += 4;

				/* Transport Class & Trigger (uint8) */
				response->rsp_data[response->rsp_data_size++] = EIPS_XPORTCLASS_CYC_SEQ;

#ifdef EIPS_TACL_OBJ_USE_ELEC_KEY
				/* Path Size (uint8) */
				response->rsp_data[response->rsp_data_size++] = 9;

				/* Path (uint16[]) */
				response->rsp_data[response->rsp_data_size++] = 0x34;	/* Electronic Key Segment */
				response->rsp_data[response->rsp_data_size++] = 0x04;	/* Key Format Table */

				rta_PutLitEndian16(EIPS_USER_IDOBJ_VENDORID, response->rsp_data+response->rsp_data_size);
				response->rsp_data_size += 2; /* Vendor ID */

				rta_PutLitEndian16(EIPS_USER_IDOBJ_DEVTYPE, response->rsp_data+response->rsp_data_size);
				response->rsp_data_size += 2; /* Device Type */

				rta_PutLitEndian16(EIPS_USER_IDOBJ_PRODCODE, response->rsp_data+response->rsp_data_size);
				response->rsp_data_size += 2; /* Product Code */

				response->rsp_data[response->rsp_data_size++] = EIPS_MAJOR_REV; /* Major Revision */
				response->rsp_data[response->rsp_data_size++] = EIPS_MINOR_REV; /* Minor Revision */
#else
				/* Path Size (uint8) */
				response->rsp_data[response->rsp_data_size++] = 4;
#endif

				/* I/O Connection Path */
				response->rsp_data[response->rsp_data_size++] = 0x20;	/* 8-bit Class ID */
				response->rsp_data[response->rsp_data_size++] = 0x04;   /* Assembly Object */

				response->rsp_data[response->rsp_data_size++] = 0x24;	/* 8-bit Instance ID */
				response->rsp_data[response->rsp_data_size++] = TaCLObj.group[inst_ix].cfg_inst;

				response->rsp_data[response->rsp_data_size++] = 0x2c;   /* 8-bit Connection Point O2T */
				response->rsp_data[response->rsp_data_size++] = TaCLObj.group[inst_ix].o2t_inst;

				response->rsp_data[response->rsp_data_size++] = 0x2c;	/* 8-bit Connection Point T2O */
				response->rsp_data[response->rsp_data_size++] = TaCLObj.group[inst_ix].t2o_inst;

				/* Structure Size (uint16) */
				rta_PutLitEndian16((response->rsp_data_size-4), response->rsp_data+size_ix);
            }
            break;

        /* ******************************************************************** */
        /*			     Unknown Object */
        /* ******************************************************************** */
        default:
	        /* Error: Object Does Not Exist */
	        response->gen_stat = ERR_UNEXISTANT_OBJ;
	        break;
    };
}

/* ====================================================================
Function:   local_dlc_valid
Parameters: actual size
            needed size
            pointer for the error message (if needed)
Returns:    TRUE/FALSE

This function is called to start the SPI task.
======================================================================= */
#if 0
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
#endif

#endif

/* *********** */
/* END OF FILE */
/* *********** */
