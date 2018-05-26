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
 *     Module Name: eips_iomsg.c
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains the definitions needed for supporting EtherNet/IP
 * I/O messaging.
 *
 */

/* ---------------------------- */
/* INCLUDE FILES                */
/* ---------------------------- */
#include "eips_system.h"

/* used for debug */
#undef  __RTA_FILE__
#define __RTA_FILE__ "eips_iomsg.c"

int16 temptest = 0;
/* there are now errors that need more than one word of extended status */
#define IOMSG_MAX_EXT_STAT_SIZE 6

/* ---------------------------- */
/* FUNCTION PROTOTYPES		*/
/* ---------------------------- */
static void     local_iomsg_StructInit (EIPS_IOCNXN_STRUCT *temp_cnxn); /* used to free connection */
static void     local_iomsg_produce    (EIPS_IOCNXN_STRUCT *temp_cnxn);
static void     local_iomsg_free_tmo   (EIPS_IOCNXN_STRUCT *io_cnxn);
static void     local_iomsg_t2oTimeout (uint16 timer_num);
static void     local_iomsg_o2tTimeout (uint16 timer_num);
static EIPS_IOCNXN_STRUCT * local_iomsg_GetUnusedCnxn(void);
static void     local_iomsg_buildError (CPF_MESSAGE_STRUCT *cpf, uint8 gen_stat, uint8 ext_size, uint16 *ext_err);
static uint32   local_iomsg_getMulticastAddr (void);
static uint8    local_iomsg_getConnectionType (uint8 o2t_cnxnpt);
static void     local_iomsg_KickStartMulticast (uint32 addr, uint32 seqnum);

static EIPS_IOCNXN_STRUCT * local_iomsg_getOwningConnection (uint16 t2o_cnxnid);
static EIPS_IOCNXN_STRUCT * local_iomsg_getExOwningConnection (uint16 t2o_cnxnid, uint16 o2t_cnxnid);

/* ---------------------------- */
/*      EXTERN VARIABLES        */
/* ---------------------------- */
extern EIPS_IDENTITY_OBJ_STRUCT eips_IDObj;
#if EIPS_USEROBJ_ASM_MAXNUM_O2TINST > 0
    extern EIPS_ASSEMBLY_INST_STRUCT eips_AsmO2TObj[EIPS_USEROBJ_ASM_MAXNUM_O2TINST];
#endif
#if EIPS_USEROBJ_ASM_MAXNUM_T2OINST > 0
    extern EIPS_ASSEMBLY_INST_STRUCT eips_AsmT2OObj[EIPS_USEROBJ_ASM_MAXNUM_T2OINST];
#endif
#ifdef EIPS_QOS_USED
    extern EIPS_QOS_OBJ_STRUCT eips_QoSObj;
#endif

extern uint8 eips_cnxn_timeout_flag;

/* ---------------------------- */
/* MISCELLANEOUS		*/
/* ---------------------------- */
static EIPS_IOCNXN_STRUCT eips_stc_iocnxns[EIPS_USER_MAX_NUM_IO_CNXNS];

#ifdef EIPS_NTWK_LED_USED
    uint8 eips_iomsg_connections_open;
#endif

/**/
/* ******************************************************************** */
/*                          GLOBAL FUNCTIONS                            */
/* ******************************************************************** */
/* ====================================================================
Function:   eips_iomsg_init
Parameters: init_type
Returns:    N/A

This function initializes all static variables used for EtherNet/IP I/O
messaging.
======================================================================= */
void eips_iomsg_init (uint8 init_type)
{
    uint16  i;
    EIPS_TIMER_DATA_STRUCT *timer_struct;

#ifdef EIPS_NTWK_LED_USED
    /* used for LED processing */
    eips_iomsg_connections_open = 0;
#endif

    /* initialize all I/O message structures */
    for(i=0; i<EIPS_USER_MAX_NUM_IO_CNXNS; i++)
    {
        /* ***************** */
        /* get the O2T timer */
        /* ***************** */
        eips_stc_iocnxns[i].CnxnStruct.O2T_TimerNum = eips_timer_new();

        /* get timer structure */
        timer_struct = eips_timer_get_struct(eips_stc_iocnxns[i].CnxnStruct.O2T_TimerNum);

        /* we couldn't get the timer structure */
        if(timer_struct == NULL)
        {
            /* we couldn't get the timer */
            eips_usersys_fatalError("eips_iomsg_init (o2t)", i);
            return;
        }

        /* set up the timeout conditions (flag or function pointer) */
        timer_struct->timeout_flag_ptr = NULL;
        timer_struct->timeout_func_ptr = local_iomsg_o2tTimeout;
#ifdef EIPS_TIMER_NAME_USED
        sprintf(timer_struct->timer_name, "Server_O2T_%03d", i);
#endif

        /* ***************** */
        /* get the T2O timer */
        /* ***************** */
        eips_stc_iocnxns[i].CnxnStruct.T2O_TimerNum = eips_timer_new();

        /* get timer structure */
        timer_struct = eips_timer_get_struct(eips_stc_iocnxns[i].CnxnStruct.T2O_TimerNum);

        /* we couldn't get the timer structure */
        if(timer_struct == NULL)
        {
            /* we couldn't get the timer */
            eips_usersys_fatalError("eips_iomsg_init (t2o)", i);
            return;
        }

        /* set up the timeout conditions (flag or function pointer) */
        timer_struct->timeout_flag_ptr = NULL;
        timer_struct->timeout_func_ptr = local_iomsg_t2oTimeout;
#ifdef EIPS_TIMER_NAME_USED
        sprintf(timer_struct->timer_name, "Server_T2O_%03d", i);
#endif
        local_iomsg_StructInit (&eips_stc_iocnxns[i]);
        eips_stc_iocnxns[i].index = i;

        /* set the Run/Idle mode to IDLE */
        eips_stc_iocnxns[i].RunIdleInfo = 0;
    }

    /* keep the compiler happy */
    if(init_type){}
}

/* ====================================================================
Function:   eips_iomsg_process
Parameters: N/A
Returns:    N/A

This function processes the I/O connections and looks for a timeout
or for the Run/Idle status.
======================================================================= */
void eips_iomsg_process (void)
{
#if EIPS_IO_LED_USED || EIPS_NTWK_LED_USED
    uint16  i;
#endif

#ifdef EIPS_IO_LED_USED
    uint8   led_run_mode, led_idle_mode;

    /* find if any connections are allocated */
    led_run_mode = 0;
    led_idle_mode = 0;

    for(i=0; i<EIPS_USER_MAX_NUM_IO_CNXNS; i++)
    {
        /* if an Exclusive Owning connection structure is used, return TRUE */
        if( (eips_stc_iocnxns[i].state == EIPS_CNXN_STATE_USED) &&
            (eips_stc_iocnxns[i].IO_Type == EIPS_IOTYPE_EXOWNER) )
        {
            if(eips_stc_iocnxns[i].RunIdleInfo & 0x00000001L)
                led_run_mode++;
            else
                led_idle_mode++;
        }
    }

    /* process the I/O LED     */
    if(led_run_mode && !led_idle_mode)
        eips_usersys_ioLedUpdate(EIPS_LEDSTATE_STEADY_GREEN);
    else
        eips_usersys_ioLedUpdate(EIPS_LEDSTATE_FLASH_GREEN);
#endif

#ifdef EIPS_NTWK_LED_USED
    /* find if any connections are allocated */
    for(i=0, eips_iomsg_connections_open = 0; i<EIPS_USER_MAX_NUM_IO_CNXNS && !eips_iomsg_connections_open; i++)
    {
        /* if a connection structure is used, return TRUE */
        if(eips_stc_iocnxns[i].state == EIPS_CNXN_STATE_USED)
        {
            eips_iomsg_connections_open = 1;
        }
    }
#endif
}

/* ====================================================================
Function:   eips_iomsg_allocated
Parameters: O->T Instance ID
Returns:    TRUE/FALSE

This function returns TRUE if any I/O connections are used, else FALSE.
======================================================================= */
uint8 eips_iomsg_allocated (uint16 o2t_inst_id)
{
    uint16  i;

    /* find if any connections are allocated */
    for(i=0; i<EIPS_USER_MAX_NUM_IO_CNXNS; i++)
    {
        /* if a connection structure is used, return TRUE */
        if( (eips_stc_iocnxns[i].state == EIPS_CNXN_STATE_USED) &&
            (eips_stc_iocnxns[i].O2T_InstID == o2t_inst_id))
        {
            return(TRUE);
        }
    }
    return(FALSE);
}

/* ====================================================================
Function:   eips_iomsg_outputs_valid
Parameters: O->T Instance ID
Returns:    TRUE/FALSE

This function returns FALSE if the device is in IDLE mode and TRUE if
the I/O connection is allocated and the mode is RUN.
======================================================================= */
uint8 eips_iomsg_outputs_valid (uint16 o2t_inst_id)
{
    uint16  i;

    /* find if any connections are allocated */
    for(i=0; i<EIPS_USER_MAX_NUM_IO_CNXNS; i++)
    {
        /* if a connection structure is used, return TRUE */
        if( (eips_stc_iocnxns[i].state == EIPS_CNXN_STATE_USED) &&
            (eips_stc_iocnxns[i].O2T_InstID == o2t_inst_id))
        {
            if(eips_stc_iocnxns[i].RunIdleInfo & 1)
			{
				//temptest = i;
				return(TRUE);
			}
                
            else
			{
				//printf("cxn idle!!");
				//temptest++;
				return(FALSE);
			}        
        }
		else
		{
			//temptest = 88;
			//printf("no cxn!!!");
		}
    }

    /* instance not found */
	//temptest = o2t_inst_id;
    return(FALSE);
}

/* ====================================================================
Function:   eips_iomsg_open
Parameters: pointer to Common Packet Format structure
            pointer to Connection structure
Returns:    N/A

This function is called to validate the ForwardOpen message that tries
to open an I/O connection.
======================================================================= */
void eips_iomsg_open (CPF_MESSAGE_STRUCT *cpf, EIPS_CONNECTION_DATA_STRUCT *temp_cnxn)
{
    uint8   io_type;
    uint16  temp_o2t_size, temp_t2o_size;
    uint16  i, temp16, o2t_header_size, t2o_header_size, j;
    uint32  temp32, current_temp;
    uint8   class_id, cfg_inst_id, cnxnpt_o2t, cnxnpt_t2o;
    uint8   class_found, cfg_inst_found, cnxnpt1_found, cnxnpt2_found;
    uint8   path_seg, path_val;
    uint8   minor_rev, major_rev, compatibility;
    uint16  vend_id, dev_type, prod_code;
    uint8   prod_inhibit_timeout;
    static  uint8   config_data[450];
    uint8   config_data_size;
    uint8   response_code;
    uint16  additional_response_code[IOMSG_MAX_EXT_STAT_SIZE];
    uint8   null_fwd_open = 0;
    uint8   matching_fwd_open = 0;
    int16   null_match_ix = -1;

#ifdef EIPS_QOS_USED
    uint8 dscp_type;
    uint8 dscp_value;
#endif

    EIPS_IOCNXN_STRUCT *iomsg_struct_ptr = NULL;
    EIPS_IOCNXN_STRUCT *owning_iomsg_struct_ptr = NULL;

    /* initialize variables */
    class_id = 0;
    cfg_inst_id = 0;
    cnxnpt_o2t = 0;
    cnxnpt_t2o = 0;

    /* **************************************** */
    /* Check if this is a Matching Forward Open */
    /* **************************************** */
    matching_fwd_open = 0;
    null_match_ix = -1;
    /* Validate vendor_ID, connection_serial_number, originator_serial_number */
    for(i=0; i<EIPS_USER_MAX_NUM_IO_CNXNS && null_match_ix==-1; i++)
    {
        /* if this is true, we have a Duplicate Forward Open (connection triad) */
        if( (eips_stc_iocnxns[i].CnxnStruct.vendor_ID == temp_cnxn->vendor_ID) &&
            (eips_stc_iocnxns[i].CnxnStruct.connection_serial_number == temp_cnxn->connection_serial_number) &&
            (eips_stc_iocnxns[i].CnxnStruct.originator_serial_number == temp_cnxn->originator_serial_number))
        {
            /* matching forward open */
            matching_fwd_open = 1;
            null_match_ix = i;
        }
    }

    /* ************************************************** */
    /* Check if this is a Duplicate/Matching Forward Open */
    /* ************************************************** */
    null_fwd_open = 0;

    /* First check if we are reconfiguring a connection */
    if( ((temp_cnxn->T2O_cnxn_params & NTWKCNXNPRM_CNXNTYP_ANDVAL) == NTWKCNXNPRM_CNXNTYP_NULL) &&
        ((temp_cnxn->O2T_cnxn_params & NTWKCNXNPRM_CNXNTYP_ANDVAL) == NTWKCNXNPRM_CNXNTYP_NULL))
    {
        null_fwd_open = 1;
    }

    else /* non-null */
    {
        /* Validate the T->O network connection parameters */
        temp16 = (uint16)(temp_cnxn->T2O_cnxn_params & NTWKCNXNPRM_CNXNTYP_ANDVAL);
        if( (temp16 != NTWKCNXNPRM_CNXNTYP_P2P) && (temp16 != NTWKCNXNPRM_CNXNTYP_MULTI) )
        {
            /* Error: "Invalid Connection Type" */
            additional_response_code[0] = CMERR_T2O_CNXNTYP_NOT_SUPPORTED;
            local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
            eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
            return;
        }

        /* Validate the O->T network connection parameters */
        temp16 = (uint16)(temp_cnxn->O2T_cnxn_params & NTWKCNXNPRM_CNXNTYP_ANDVAL);
        if(temp16 != NTWKCNXNPRM_CNXNTYP_P2P)
        {
            /* Error: "Invalid Connection Type" */
            additional_response_code[0] = CMERR_O2T_CNXNTYP_NOT_SUPPORTED;
            local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
            eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
            return;
        }
    }

    /*
    ***************************************************************
    PARSE THE PATH...

    For now, the only valid paths are:
        Class 0x04, Instance Cfg_Inst, CnxnPt1, CnxnPt2
	    Class 0x04, CnxnPt1, CnxnPt2
    ***************************************************************
    */

    /* initialize the found flags */
    class_found	    = FALSE;
    cfg_inst_found  = FALSE;
    cnxnpt1_found   = FALSE;
    cnxnpt2_found   = FALSE;
    config_data_size = 0;
    memset (config_data, 0, sizeof(config_data));

    /* look through the path */
    for(i=0; i<temp_cnxn->connection_path_size; i++)
    {
        path_seg = EIPS_LO(temp_cnxn->connection_path[i]);
        path_val = EIPS_HI(temp_cnxn->connection_path[i]);

        switch (path_seg)
        {
            /* Production Inhibit Time Value */
            case NETSEG_PRODINHIBIT:
                /* store the production inhibit time */
                prod_inhibit_timeout = path_val;
                if (prod_inhibit_timeout);
                break;

            /* Configuration Assembly Data */
            case DATASEG_SIMPLE:
                /* store the size */
                config_data_size = path_val;
                i++; /* skip to the next word */

                /* store the data */
                for(j=0; j<config_data_size; j++, i++)
                {
                    /* store the data */
                    if(i<temp_cnxn->connection_path_size)
                    {
                        config_data[j*2]   = EIPS_LO(temp_cnxn->connection_path[i]);
                        config_data[j*2+1] = EIPS_HI(temp_cnxn->connection_path[i]);
                    }

                    /* invalid size found - "Not Enough Data" */
                    else
                    {
                        /* Error: "Invalid Segment Type or Value in Path" */
                        additional_response_code[0] = CMERR_BAD_SEGMENT;
                        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
                        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                        return;
                    }
                }
                break;

	        /* Store the Class */
            case LOGSEG_8BITCLASS:
                /* The class must be the first path segment */
                if( (class_found == FALSE) && (cfg_inst_found == FALSE) &&
                    (cnxnpt1_found == FALSE) && (cnxnpt2_found == FALSE) )
	            {
	                class_id = path_val;
	                class_found = TRUE;
	            }

	            else
	            {
                    /* Error: "Invalid Segment Type or Value in Path" */
                    additional_response_code[0] = CMERR_BAD_SEGMENT;
                    local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
                    eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                    return;
                }
                break;

            /* Store the Inst */
            case LOGSEG_8BITINST:
                /* The instance must be the second path segment */
                if( (class_found == TRUE) && (cfg_inst_found == FALSE) &&
                    (cnxnpt1_found == FALSE) && (cnxnpt2_found == FALSE) )
                {
                    cfg_inst_id = path_val;
					//eips_user_dbprint1("cfg_inst_id is: %d", cfg_inst_id);
                    cfg_inst_found = TRUE;
                }

                else
                {
	                /* Error: "Invalid Segment Type or Value in Path" */
                    additional_response_code[0] = CMERR_BAD_SEGMENT;
                    local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
                    eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                    return;
                }
                break;

            /* we should get 2 connection points */
            case LOGSEG_8BITCNXNPT:
                if(class_found == TRUE)
                {
	                /* we found connection point 1 */
	                if((cnxnpt1_found == FALSE) && (cnxnpt2_found == FALSE))
	                {
		                cnxnpt_o2t = path_val;
		                cnxnpt1_found = TRUE;
	                }

	                /* we found connection point 2 */
	                else if ((cnxnpt1_found == TRUE) && (cnxnpt2_found == FALSE))
	                {
		                cnxnpt_t2o = path_val;
		                cnxnpt2_found = TRUE;
	                }

	                /* invalid */
	                else
	                {
	                    /* Error: "Invalid Segment Type or Value in Path" */
                        additional_response_code[0] = CMERR_BAD_SEGMENT;
                        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
		                eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
		                return;
	                }
                }

	            else
	            {
	                /* Error: "Invalid Segment Type or Value in Path" */
                    additional_response_code[0] = CMERR_BAD_SEGMENT;
                    local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
                    eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
	                return;
	            }
	            break;

	        /* Electronic Key Validation */
	        case LOGSEG_ELECKEY:
                /* path_val needs to be ELECKEY_FORMAT */
	            if(path_val == ELECKEY_FORMAT)
	            {
	                /* make sure we have enough paths left (4) */
	                if(i+4 >= temp_cnxn->connection_path_size)
	                {
	                    /* Error: "Invalid Segment Type or Value in Path" */
                        additional_response_code[0] = CMERR_BAD_SEGMENT;
                        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
		                eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
		                return;
	                }

                    /* store the identity object attributes */
	                vend_id	 = temp_cnxn->connection_path[i+1];
	                dev_type  = temp_cnxn->connection_path[i+2];
	                prod_code = temp_cnxn->connection_path[i+3];
	                minor_rev = EIPS_HI(temp_cnxn->connection_path[i+4]);
	                major_rev = (uint8)(EIPS_LO(temp_cnxn->connection_path[i+4]) & 0x7F);

	                /*  bit 7 of the Major Revision specifies Compatibility
		                bit 7 = 0 --> All non-zero validation params must match.
		                bit 7 = 1 --> any key we can emulate is supported. */
	                compatibility = (uint8)((EIPS_LO(temp_cnxn->connection_path[i+4]) & 0x80) >> 7);

                    /* if compatibility is 1, 0 is invalid for vend_id, prod_code and maj_rev */
                    if(compatibility) 
                    {
                        if((vend_id == 0) || (prod_code == 0) || (major_rev == 0))
	                    {
		                    /* Error: "Path Segment Error" */
                            local_iomsg_buildError(cpf, ERR_PATHSEGMENT, 0, 0);
		                    eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
		                    return;
	                    }    
                    }

	                /* advanve the path index */
	                i += 4;

	                /* validate the Vendor ID (if non-zero) */
	                if( ((vend_id) && (vend_id != eips_IDObj.Inst.Vendor)) ||
		                ((prod_code) && (prod_code != eips_IDObj.Inst.ProductCode)) )
	                {
		                /* Error: "Either the Vendor ID or Product Code Didn't Match" */
                        additional_response_code[0] = CMERR_BAD_VENDOR_PRODUCT;
                        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
		                eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
		                return;
	                }

	                /* validate the Device Type (if non-zero) */
	                if((dev_type) && (dev_type != eips_IDObj.Inst.DeviceType))
	                {
		                /* Error: "The Device Type Didn't Match" */
                        additional_response_code[0] = CMERR_BAD_DEVICE_TYPE;
                        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
		                eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
		                return;
	                }

	                /* Validate the major/minor revision seperate */
	                if(major_rev)
	                {
		                /* Invalid Revision */
		                if(major_rev != EIPS_MAJOR_REV)
		                {
		                    /* Error: "The Revision Didn't Match" */
                            additional_response_code[0] = CMERR_BAD_REVISION;
                            local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
		                    eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
		                    return;
		                }

		                /*  The minor revision is dependent on the major revision
		                    if the major revision != 0 and the major revision matches
		                    we need to match an non-zero minor revision exactly if
		                    the compatibility bit is clear or be <= our revision if the
		                    bit is set. */
		                if(minor_rev)
		                {
		                    /* Invalid Revision */
		                    if((minor_rev != EIPS_MINOR_REV) && (compatibility == 0))
		                    {
			                    /* Error: "The Revision Didn't Match" */
                                additional_response_code[0] = CMERR_BAD_REVISION;
                                local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
			                    eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
			                    return;
		                    }
		                }
	                }
	            }

	            /* invalid path (bad Electronic Key Format) */
	            else
                {
	                /* Error: "Invalid Segment Type or Value in Path" */
                    additional_response_code[0] = CMERR_BAD_SEGMENT;
                    local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
	                eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
	                return;
                }
	            break;

	        /* Error: "Invalid Segment Type or Value in Path" */
	        default:
                additional_response_code[0] = CMERR_BAD_SEGMENT;
                local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
	            eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
	            return;
        }; /* END-> "switch" */
    } /* END-> "Parse the Path" */

    /*  we need to validate the class */
    if((class_found == FALSE) || (class_id != CLASS_ASSEMBLY))
    {
        /* Error: "Invalid Segment Type or Value in Path" */
        additional_response_code[0] = CMERR_BAD_SEGMENT;
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }

    /*  we need to validate cnxnpt1 (O_to_T) */
    if((cnxnpt1_found == FALSE) || (eips_cpf_o2tCnxnPtValid(cnxnpt_o2t) == FALSE))
    {
        /* Error: "Invalid O2T Type or Value in Path" */
        additional_response_code[0] = CMERR_INV_O2T_PATH;
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }

    /*  we need to validate the cnxnpt2 (T_to_O) */
    if((cnxnpt2_found == FALSE) || (eips_cpf_t2oCnxnPtValid(cnxnpt_t2o) == FALSE))
    {
        /* Error: "Invalid T2O Type or Value in Path" */
        additional_response_code[0] = CMERR_INV_T2O_PATH;
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }

    /* process the configuration assembly (if passed) */
    if(cfg_inst_found == TRUE)
    {
        /* set the response codes to success */
        response_code = 0;
        memset(additional_response_code, 0, sizeof(additional_response_code));
		//printf("the cfg data size is: %d", config_data_size);
        /* validate the configuration assembly */
        if(eips_userobj_cfg_asm_process((uint16)cfg_inst_id, config_data, (uint16)(config_data_size*2), &response_code, &additional_response_code[0]) != SUCCESS)
        {            
			/* we have an error code */
            if(response_code != 0)
            {
                /* we have an additional error code */
                if(additional_response_code[0] != 0)
                {
                    local_iomsg_buildError(cpf, response_code, 1, additional_response_code);
                }

                else
                {
                    local_iomsg_buildError(cpf, response_code, 0, 0);
                }

                /* common error processing */
                eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                return;
            }
        }
    }

    /* ****************************************************** */
    /* Validate the connection transport type (COS or Cyclic) */
    /* ****************************************************** */
    if( (temp_cnxn->xport_type_and_trigger == EIPS_XPORTCLASS_APP_SEQ) ||
        (temp_cnxn->xport_type_and_trigger == EIPS_XPORTCLASS_APP_NOSEQ) )
    {
        /* Error: "Invalid Transport Type/Trigger" */
        additional_response_code[0] = CMERR_BAD_TRANSPORT;
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }

    /* ***************************** */
    /* validate the connection sizes  */
    /* ***************************** */

    /* ------- */
    /* T2O/O2T */
    /* ------- */
    /* default to a header size of 0 */
    t2o_header_size = 0;
    o2t_header_size = 0;

    /* if the transport class is 1, add the 16-bit PDU Sequence Number */
    if( (temp_cnxn->xport_type_and_trigger == EIPS_XPORTCLASS_CYC_SEQ) ||
        (temp_cnxn->xport_type_and_trigger == EIPS_XPORTCLASS_COS_SEQ) )
    {
        t2o_header_size += 2;
        o2t_header_size += 2;
    }

    /* --- */
    /* T2O */
    /* --- */
    /* if the 32-bit Run Time Header is used */
#ifdef EIPS_USEROBJ_T2O_RUNTIME_HEADER_USED
    t2o_header_size += 4;
#endif

/* ********************************** */
/* VARIABLE SIZE ASSEMBLIES SUPPORTED */
/* ********************************** */
#ifdef EIPS_USEROBJ_ASM_VAR_SIZE_ENABLE

#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
    /* T->O Connection = 2 + ASM size * 4 */
    if ((((temp_cnxn->T2O_cnxn_params & 0x01FF) - t2o_header_size)%4) != 0) /* check alignment */
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_T2O_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_t2o)*4)+t2o_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
    if(((eips_cpf_getAsmSize(cnxnpt_t2o)*4)+t2o_header_size) < (temp_cnxn->T2O_cnxn_params & 0x01FF))
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_T2O_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_t2o)*4)+t2o_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
    /* T->O Connection = 2 + ASM size * 2 */
    if ((((temp_cnxn->T2O_cnxn_params & 0x01FF) - t2o_header_size)%2) != 0) /* check alignment */
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_T2O_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_t2o)*2)+t2o_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
    /* T->O Connection = 2 + ASM size * 2 */
    if(((eips_cpf_getAsmSize(cnxnpt_t2o)*2)+t2o_header_size) < (temp_cnxn->T2O_cnxn_params & 0x01FF))
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_T2O_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_t2o)*2)+t2o_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
#else                                       /* BYTES */
    /* T->O Connection = 2 + ASM size * 1 */
    if(((eips_cpf_getAsmSize(cnxnpt_t2o)*1)+t2o_header_size) < (temp_cnxn->T2O_cnxn_params & 0x01FF))
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_T2O_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_t2o)*1)+t2o_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
#endif

/* ******************************* */
/* FIXED SIZE ASSEMBLIES SUPPORTED */
/* ******************************* */
#else /* #ifdef EIPS_USEROBJ_ASM_VAR_SIZE_ENABLE */

#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4 /* DWORDS */
    /* T->O Connection = 2 + ASM size * 4 */
    if(((eips_cpf_getAsmSize(cnxnpt_t2o)*4)+t2o_header_size) != temp_cnxn->T2O_cnxn_size)
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_T2O_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_t2o)*4)+t2o_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2 /* WORDS */
    /* T->O Connection = 2 + ASM size * 2 */
    if(((eips_cpf_getAsmSize(cnxnpt_t2o)*2)+t2o_header_size) != temp_cnxn->T2O_cnxn_size)
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_T2O_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_t2o)*2)+t2o_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
#else /* BYTES */
    /* T->O Connection = 2 + ASM size * 1 */
    if(((eips_cpf_getAsmSize(cnxnpt_t2o)*1)+t2o_header_size) != temp_cnxn->T2O_cnxn_size)
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_T2O_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_t2o)*1)+t2o_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
#endif

#endif /* #else (EIPS_USEROBJ_ASM_VAR_SIZE_ENABLE) */

    /* --- */
    /* O2T */
    /* --- */
    /* if the size is > 0 we use the 32-bit RUN/IDLE check for the O->T Connection */
    if(eips_cpf_getAsmSize(cnxnpt_o2t) > 0)
        o2t_header_size += 4;

/* ********************************** */
/* VARIABLE SIZE ASSEMBLIES SUPPORTED */
/* ********************************** */
#ifdef EIPS_USEROBJ_ASM_VAR_SIZE_ENABLE

#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4 /* DWORDS */
    /* O->T Connection = 6 + ASM size * 4 */
    if ((((temp_cnxn->O2T_cnxn_params & 0x01FF) - o2t_header_size)%4) != 0) /* check alignment */
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_O2T_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_o2t)*4)+o2t_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
    /* O->T Connection = 6 + ASM size * 4 */
    if(((eips_cpf_getAsmSize(cnxnpt_o2t)*4)+o2t_header_size) < (temp_cnxn->O2T_cnxn_params & 0x01FF))
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_O2T_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_o2t)*4)+o2t_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2 /* WORDS */
    /* O->T Connection = 6 + ASM size * 2 */
    if ((((temp_cnxn->O2T_cnxn_params & 0x01FF) - o2t_header_size)%2) != 0) /* check alignment */
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_O2T_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_o2t)*2)+o2t_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
    /* O->T Connection = 6 + ASM size * 2 */
    if(((eips_cpf_getAsmSize(cnxnpt_o2t)*2)+o2t_header_size) < (temp_cnxn->O2T_cnxn_params & 0x01FF))
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_O2T_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_o2t)*2)+o2t_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
#else /* BYTES */
    /* O->T Connection = 6 + ASM size * 1 */
    if(((eips_cpf_getAsmSize(cnxnpt_o2t)*1)+o2t_header_size) < (temp_cnxn->O2T_cnxn_params & 0x01FF))
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_O2T_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_o2t)*1)+o2t_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
#endif

/* ******************************* */
/* FIXED SIZE ASSEMBLIES SUPPORTED */
/* ******************************* */
#else /* #ifdef EIPS_USEROBJ_ASM_VAR_SIZE_ENABLE */

#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
    /* O->T Connection = 6 + ASM size * 4 */
    if(((eips_cpf_getAsmSize(cnxnpt_o2t)*4)+o2t_header_size) != temp_cnxn->O2T_cnxn_size)
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_O2T_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_o2t)*4)+o2t_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
    /* O->T Connection = 6 + ASM size * 2 */
    if(((eips_cpf_getAsmSize(cnxnpt_o2t)*2)+o2t_header_size) != temp_cnxn->O2T_cnxn_size)
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_O2T_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_o2t)*2)+o2t_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
#else                                       /* BYTES */
    /* O->T Connection = 6 + ASM size * 1 */
    if(((eips_cpf_getAsmSize(cnxnpt_o2t)*1)+o2t_header_size) != temp_cnxn->O2T_cnxn_size)
    {
        /* Error: "Invalid T2O Connection Size" */
        additional_response_code[0] = CMERR_INV_O2T_SIZE;
        additional_response_code[1] = ((eips_cpf_getAsmSize(cnxnpt_o2t)*1)+o2t_header_size); /* max size in bytes */
        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
#endif

#endif /* #ifdef EIPS_USEROBJ_ASM_VAR_SIZE_ENABLE */

/* Calculate the actual T2O size in algined data types, not bytes */
temp_t2o_size = (temp_cnxn->T2O_cnxn_size-t2o_header_size);
#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
    temp_t2o_size = (temp_t2o_size / 4);
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
    temp_t2o_size = (temp_t2o_size / 2);
#else                                       /* BYTES */
    temp_t2o_size = (temp_t2o_size);
#endif

/* Calculate the actual O2T size in algined data types, not bytes */
temp_o2t_size = (temp_cnxn->O2T_cnxn_size-o2t_header_size);
#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
    temp_o2t_size = (temp_o2t_size / 4);
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
    temp_o2t_size = (temp_o2t_size / 2);
#else                                       /* BYTES */
    temp_o2t_size = (temp_o2t_size);
#endif

#ifdef EIPS_QOS_USED
    switch(temp_cnxn->T2O_cnxn_params & NTWKCNXNPRM_PRIORITY_ANDVAL)
    {
        case NTWKCNXNPRM_PRIORITY_LOW:
            dscp_type  = EIPS_QOS_DSCP_TYPE_LOW;
            dscp_value = eips_QoSObj.Inst.DSCP_Low;
            break;
        case NTWKCNXNPRM_PRIORITY_HIGH:
            dscp_type  = EIPS_QOS_DSCP_TYPE_HIGH;
            dscp_value = eips_QoSObj.Inst.DSCP_High;
            break;
        case NTWKCNXNPRM_PRIORITY_SCHEDULE:
            dscp_type  = EIPS_QOS_DSCP_TYPE_SCHEDULED;
            dscp_value = eips_QoSObj.Inst.DSCP_Scheduled;
            break;
        case NTWKCNXNPRM_PRIORITY_URGENT:
            dscp_type  = EIPS_QOS_DSCP_TYPE_URGENT;
            dscp_value = eips_QoSObj.Inst.DSCP_Urgent;
            break;
        default:
            dscp_type  = EIPS_QOS_DSCP_TYPE_NOTUSED;
            dscp_value = 0;
            break;
    };
#endif

    /* ******************************************************** */
    /* I/O TYPE DEFINES */
    /*  */
    /* Abbreviations Guide: */
    /* -------------------- */
    /* PP = PointToPoint; MC = Mulicast;  */
    /* IOHB = Input Only Heartbeat Instance */
    /* LOHB = Listen Only Heartbeat Instance */
    /* USER = User Defined Instance */
    /* O->T = O2T = Originator to Target = Our device consumes */
    /* T->O = T2O = Target to Originator = Our device produces */
    /* cnxn = connection */
    /* */
    /* Only the T->O can use Multicast!!!!! */
    /* */
    /* FORWARD OPEN ALLOCATION RULES:  */
    /* */
    /* -------------------------------------- */
    /*                            Dependent           */
    /* O->T (PP)     T->O       Alloc   Close   Cnxn Type */
    /* ---------    -------     -----   -----   ----------- */
    /* USER         USER PP      NO      NO     Ex Owner */
    /* USER         USER MC      NO      NO     Ex Owner    */
    /* IOHB         USER PP      NO      NO     Input Only */
    /* IOHB         USER MC      NO      YES    Input Only */
    /* LOHB         USER MC      YES     YES    Listen Only */
    /* LOHB         USER PP     ***Invalid*** */
    /* */
    /* If more than one O->T connection exists, each one can */
    /* have an Exclusive Owner. (FUTURE ADDITION????) */
    /* */
    /* All connections using the MC address for the "owning" */
    /* connection close if the "owning" connection timesout or */
    /* is closed explicitly.  This includes the Input Only */
    /* connection if the Exclusive Owner connection closes. */
    /* ******************************************************** */

    /* validate and store the connection type */
    temp16 = (uint16)(temp_cnxn->T2O_cnxn_params & NTWKCNXNPRM_CNXNTYP_ANDVAL);

    /* get the I/O type */
    io_type = local_iomsg_getConnectionType(cnxnpt_o2t);

    /* ***************************** */
    /* switch on the connection type */
    /* ***************************** */
    switch(io_type)
    {
        /* ----------------------------------------------------------------------------- */
        /*                  EXCLUSIVE OWNER */
        /* ----------------------------------------------------------------------------- */
        case EIPS_IOTYPE_EXOWNER:
        /* ----------------------------------------------------------------------------- */
        /*                  INPUT ONLY (Same as Exclusive Owner unless T2O Multicast) */
        /* ----------------------------------------------------------------------------- */
        case EIPS_IOTYPE_INPUTONLY:
            /* special code for a NULL forward open follows */
            if(!null_fwd_open)
            {
                /*  first validate the ownership (exclusive owerner only) */
                owning_iomsg_struct_ptr = local_iomsg_getExOwningConnection(cnxnpt_t2o, cnxnpt_o2t);
                if((owning_iomsg_struct_ptr != NULL) && (io_type == EIPS_IOTYPE_EXOWNER))
                {
                    /* Error: "Exclusive Owner Already Exists" */
                    additional_response_code[0] = CMERR_OWNER_CONFLICT;
                    local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
                    eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                    return;
                }

                /*  second we need to see if this connection is owned (T2O) before we perform
                    additional validation. */
                owning_iomsg_struct_ptr = local_iomsg_getOwningConnection(cnxnpt_t2o);

#ifdef EIPS_USEROBJ_ASM_VAR_SIZE_ENABLE
                /* check the requested size against the owning connection (if available) */
                if(owning_iomsg_struct_ptr)
                {
                    if(owning_iomsg_struct_ptr->T2O_Size != temp_t2o_size)
                    {
                        /* Error: "Invalid T2O Connection Size" */
                        additional_response_code[0] = CMERR_INV_O2T_SIZE;
                        additional_response_code[1] = owning_iomsg_struct_ptr->T2O_Size; /* max size in bytes */
                        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 2, additional_response_code);
                        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                        return;
                    }
                }
#endif
            }

            /* what to do on a NULL Forward Open */
            if(null_fwd_open && matching_fwd_open)
            {
                if(null_match_ix >= 0)
                {
                    iomsg_struct_ptr = &eips_stc_iocnxns[null_match_ix];
                }
            }

            /* non-null, get new pointer */
            else
            {
                /* check if there are free I/O connections */
                iomsg_struct_ptr = local_iomsg_GetUnusedCnxn();

                /* there are no I/O resources left */
                if(iomsg_struct_ptr == NULL)
                {
                    /* Error: "No more connections" */
                    additional_response_code[0] = CMERR_NO_CM_RESOURCES;
                    local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
                    eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                    return;
                }

                /* store the connection data to the connection structure */
                eips_cnxn_structCopy(&iomsg_struct_ptr->CnxnStruct,temp_cnxn);
            }

#ifdef EIPS_QOS_USED
            iomsg_struct_ptr->dscp_type = dscp_type;
            iomsg_struct_ptr->dscp_value = dscp_value;
#endif

            /* store the connection type */
            iomsg_struct_ptr->IO_Type = io_type;

            /* API is the RPI rounded to a supported resolution */
            iomsg_struct_ptr->CnxnStruct.O2T_API = EIPS_APIROUND(iomsg_struct_ptr->CnxnStruct.O2T_RPI);
            iomsg_struct_ptr->CnxnStruct.T2O_API = EIPS_APIROUND(iomsg_struct_ptr->CnxnStruct.T2O_RPI);

            /* make sure both API are > 0 */
            if( iomsg_struct_ptr->CnxnStruct.O2T_API == 0 ||
                iomsg_struct_ptr->CnxnStruct.T2O_API == 0)
            {
				/* free	connection structure */
                local_iomsg_StructInit(iomsg_struct_ptr);

                /* Error: "RPI Not Supported" */
                additional_response_code[0] = CMERR_BAD_RPI;
                local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
                eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                return;
            }

            /* if the connection timeout multiplier is invalid, return an error */
            if(eips_cnxn_getCnxnTmoMult (iomsg_struct_ptr->CnxnStruct.connection_timeout_multiplier) == 0)
            {
				/* free	connection structure	 */
                local_iomsg_StructInit(iomsg_struct_ptr);

                /* Error: "RPI, Connection Timeout Multiplier Problem" */
                additional_response_code[0] = CMERR_BAD_RPI;
                local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
                eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                return;
            }

            /* store the ports */
            iomsg_struct_ptr->O2T_Port = EIPS_UDPIO_PORT;
            iomsg_struct_ptr->T2O_Port = EIPS_UDPIO_PORT;
            iomsg_struct_ptr->O2T_InstID = cnxnpt_o2t;
            iomsg_struct_ptr->T2O_InstID = cnxnpt_t2o;

            /* *********************** */
            /* DATA POINTER VALIDATION */
            /* *********************** */
            /* try to get the pointer (if needed) */
            iomsg_struct_ptr->O2T_Size   = temp_o2t_size;
            if(iomsg_struct_ptr->O2T_Size > 0)
            {
                iomsg_struct_ptr->O2T_DataPtr = eips_userobj_getAsmPtr(cnxnpt_o2t, &iomsg_struct_ptr->O2T_Size);

	            /* we didn't get a pointer to the asm */
	            if(iomsg_struct_ptr->O2T_DataPtr == NULL)
	            {
					/* free	connection structure	 */
					local_iomsg_StructInit(iomsg_struct_ptr);

	                /* Error: "No Resources Available" */
                    additional_response_code[0] = CMERR_NO_SEC_RESOURCE;
                    local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
                    eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
	                return;
	            }
            }

            /* try to get the pointer (if needed) */
            iomsg_struct_ptr->T2O_Size   = temp_t2o_size;
            if(iomsg_struct_ptr->T2O_Size > 0)
            {
	            iomsg_struct_ptr->T2O_DataPtr = eips_userobj_getAsmPtr(cnxnpt_t2o, &iomsg_struct_ptr->T2O_Size);

	            /* we didn't get a pointer to the asm */
	            if(iomsg_struct_ptr->T2O_DataPtr == NULL)
	            {
					/* free	connection structure	 */
					local_iomsg_StructInit(iomsg_struct_ptr);

	                /* Error: "No Resources Available" */
                    additional_response_code[0] = CMERR_NO_SEC_RESOURCE;
                    local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
	                eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
	                return;
	            }
            }

            /* ********** */
            /* TIMER CODE */
            /* ********** */

            /* fill in the O2T timer information and start the timer */
            temp32 = EIPS_APITICKS(iomsg_struct_ptr->CnxnStruct.O2T_API);

            /* Use the connection timeout multiplier */
            temp32 = (temp32 * eips_cnxn_getCnxnTmoMult(iomsg_struct_ptr->CnxnStruct.connection_timeout_multiplier));

            /* we need to wait at least 10 seconds the first time */
            if(temp32 < EIPS_USER_TICKS_PER_10SEC)
                current_temp = EIPS_USER_TICKS_PER_10SEC;
            else
                current_temp = temp32;

            /* store the new timer values */
            eips_timer_changeValues (iomsg_struct_ptr->CnxnStruct.O2T_TimerNum, current_temp, temp32);

            /* start the timer */
            eips_timer_start(iomsg_struct_ptr->CnxnStruct.O2T_TimerNum);

            /* fill in the timer information and start the timer */
            temp32 = EIPS_APITICKS(iomsg_struct_ptr->CnxnStruct.T2O_API);

            /* store the new timer values */
            eips_timer_changeValues (iomsg_struct_ptr->CnxnStruct.T2O_TimerNum, temp32, temp32);

            /* start the timer */
            eips_timer_start(iomsg_struct_ptr->CnxnStruct.T2O_TimerNum);

            /* NULL Forward Open, we are done */
            if(null_fwd_open && matching_fwd_open)
            {
                /* store the modified connection data to the temp connection structure */
                eips_cnxn_structCopy(temp_cnxn, &iomsg_struct_ptr->CnxnStruct);
                break;
            }   
             
            /* *************************************** */
            /* O2T VALIDATION - MUST BE POINT TO POINT */
            /* *************************************** */

            /* find the IP address that is trying to open the connection */
            iomsg_struct_ptr->O2T_IPAddr = eips_usersock_getSockInfo_sin_addr(cpf->sock_id);

            /* store the O_to_T connection information */
            iomsg_struct_ptr->CnxnStruct.O2T_CID = eips_cnxn_getConnectionID(); /* get the new Connection ID */

            /* ********************************* */
            /* T2O VALIDATION - P2P OR MULTICAST */
            /* ********************************* */

            /*  O2T must be Point to Point, so we only need to check if the T2O
                connection is Multicast or Point to Point */
            temp16 = (uint16)(iomsg_struct_ptr->CnxnStruct.T2O_cnxn_params & NTWKCNXNPRM_CNXNTYP_ANDVAL);

            /* ----------------------------------------- */
            /* the T2O IP Address is a Multicast Address */
            /* ----------------------------------------- */
            if(temp16 == NTWKCNXNPRM_CNXNTYP_MULTI)
            {
                /* no owning connection */
                if(owning_iomsg_struct_ptr == NULL)
                {
                    iomsg_struct_ptr->T2O_IPAddr = local_iomsg_getMulticastAddr();
                    iomsg_struct_ptr->CnxnStruct.T2O_CID = eips_cnxn_getConnectionID();
                }

                /* we found an owning connection */
                else
                {
                    /* stop the timer */
                    eips_timer_stop(iomsg_struct_ptr->CnxnStruct.T2O_TimerNum);

                    /* validate the RPI (must match exactly)!!!! */
                    if(temp_cnxn->T2O_RPI != owning_iomsg_struct_ptr->CnxnStruct.T2O_RPI)
                    {
						/* free	connection structure	 */
                        local_iomsg_StructInit(iomsg_struct_ptr);

                        /* Error: "RPI Not Supported" */
                        additional_response_code[0] = CMERR_BAD_RPI;
                        local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
                        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                        return;
                    }

                    /* modify temp connection to reflect the T2O Connection ID from the owning connection */
                    /* use the rest of the information from the request */
                    iomsg_struct_ptr->T2O_IPAddr = owning_iomsg_struct_ptr->T2O_IPAddr;
                    iomsg_struct_ptr->CnxnStruct.T2O_CID = owning_iomsg_struct_ptr->CnxnStruct.T2O_CID;
                }
            }

            /* ---------------------------------------------- */
            /* the T2O IP Address is a Point-to-Point Address */
            /* ---------------------------------------------- */
            else
            {
                /* store the T_to_O connection information */
                iomsg_struct_ptr->T2O_IPAddr = eips_usersock_getSockInfo_sin_addr(cpf->sock_id);
                iomsg_struct_ptr->T2O_InstID = cnxnpt_t2o;
            }

            /* store the modified connection data to the temp connection structure */
            eips_cnxn_structCopy(temp_cnxn, &iomsg_struct_ptr->CnxnStruct);

            /* clear the timeout flag (on any successful exclusive owner open) */
            if(io_type == EIPS_IOTYPE_EXOWNER)
                eips_cnxn_timeout_flag = 0;
            break;

        /* ----------------------------------------------------------------------------- */
        /*                  LISTEN ONLY (we must match to a node with our T2O inst id) */
        /* ----------------------------------------------------------------------------- */
        case EIPS_IOTYPE_LISTENONLY:
            iomsg_struct_ptr = local_iomsg_getOwningConnection(cnxnpt_t2o);

            /* no match? return an error */
            if(iomsg_struct_ptr == NULL)
            {
                /* Error: "Non-Listen Only Connection Not Opened" */
                additional_response_code[0] = CMERR_CONN_REQ_FAILS;
                local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
                eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                return;
            }

            /* validate the RPI (must match exactly)!!!! */
            if(temp_cnxn->T2O_RPI != iomsg_struct_ptr->CnxnStruct.T2O_RPI)
            {
                /* Error: "RPI Not Supported" */
                additional_response_code[0] = CMERR_BAD_RPI;
                local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
                eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                return;
            }

            /* modify temp connection to reflect the new O2T Connection ID */
            temp_cnxn->O2T_CID = eips_cnxn_getConnectionID();

            /* modify temp connection to reflect the T2O Connection ID from the owning connection */
            /* use the rest of the information from the request */
            temp_cnxn->T2O_CID = iomsg_struct_ptr->CnxnStruct.T2O_CID;

            /* store the API Values (for the response) */
            temp_cnxn->O2T_API = iomsg_struct_ptr->CnxnStruct.O2T_API;
            temp_cnxn->T2O_API = iomsg_struct_ptr->CnxnStruct.T2O_API;
            break;

        /* DEFAULT -> Error */
        default:
            /* Error: "Invalid Connection Type" */
            additional_response_code[0] = CMERR_BAD_CONN_TYPE;
            local_iomsg_buildError(cpf, ERR_CNXN_FAILURE, 1, additional_response_code);
            eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
            return;
    };

    /* ************************************************ */
    /* add the additional items to the response message */
    /* ************************************************ */
    cpf->ItemCount = 4;

    /* ---------------- */
    /* O->T INFORMATION */
    /* ---------------- */

    /* store the Socket Info (O2T) */
    cpf->Items[2].TypeId = DATA_TYP_ID_SOCK_OT;
    cpf->Items[2].Length = 16;

    /* store (in Big-Endian) the sin_family (always 2) */
    rta_PutBigEndian16(2, cpf->Items[2].Data);

    /* store (in Big-Endian) the sin_port */
    rta_PutBigEndian16(EIPS_UDPIO_PORT, cpf->Items[2].Data+2);

    /* we always consume on our IP Address */
    rta_PutBigEndian32(eips_usersock_getOurIPAddr(), cpf->Items[2].Data+4);

    /* store (in Big-Endian) the sin_zero (always 0) */
    rta_PutBigEndian32(0, cpf->Items[2].Data+8);
    rta_PutBigEndian32(0, cpf->Items[2].Data+12);

    /* ---------------- */
    /* T->O INFORMATION */
    /* ---------------- */

    /* store the Socket Info (T2O) */
    cpf->Items[3].TypeId = DATA_TYP_ID_SOCK_TO;
    cpf->Items[3].Length = 16;

    /* store (in Big-Endian) the sin_family (always 2) */
    rta_PutBigEndian16(2, cpf->Items[3].Data);

    /* store (in Big-Endian) the sin_port */
    rta_PutBigEndian16(EIPS_UDPIO_PORT, cpf->Items[3].Data+2);

    /* store (in Big-Endian) the sin_addr */
    rta_PutBigEndian32(iomsg_struct_ptr->T2O_IPAddr, cpf->Items[3].Data+4);

    /* store (in Big-Endian) the sin_zero (always 0) */
    rta_PutBigEndian32(0, cpf->Items[3].Data+8);
    rta_PutBigEndian32(0, cpf->Items[3].Data+12);

    /* ************************* */
    /* build the success message */
    /* ************************* */
    eips_cnxn_validFwdOpen(temp_cnxn,cpf);
}

/* ====================================================================
Function:   eips_iomsg_close
Parameters: Pointer to Common Packet Format structure
	        Pointer to Connection structure
Returns:    TRUE/FALSE

This function is called to validate the ForwardClose message that tries
to close an I/O connection. If the connection is closed, TRUE is
returned, else FALSE is returned.
======================================================================= */
uint8 eips_iomsg_close (EIPS_CONNECTION_DATA_STRUCT *temp_cnxn)
{
    uint16 i;
    uint32 mult_addr, seq_num;

    if(temp_cnxn == NULL)
        return(FALSE);

    /*  on a close, we need to find the I/O message that corresponds to
        the connection that was just closed and free the UDP resources. */
    for(i=0; i<EIPS_USER_MAX_NUM_IO_CNXNS; i++)
    {
        /* try to find the connection that matches */
        if( (eips_stc_iocnxns[i].CnxnStruct.connection_serial_number == temp_cnxn->connection_serial_number) &&
	        (eips_stc_iocnxns[i].CnxnStruct.vendor_ID == temp_cnxn->vendor_ID) &&
	        (eips_stc_iocnxns[i].CnxnStruct.originator_serial_number == temp_cnxn->originator_serial_number))
        {
            /* free the connection */
            mult_addr = eips_stc_iocnxns[i].T2O_IPAddr;
            seq_num = eips_stc_iocnxns[i].T2O_SeqNum;
            local_iomsg_StructInit(&eips_stc_iocnxns[i]);

            /* make sure at least one device is still transmitting on our multicast address */
            local_iomsg_KickStartMulticast(mult_addr, seq_num);
            return(TRUE);
        }
    }

    /* no connection found */
    return(FALSE);
}

/* ====================================================================
Function:   eips_iomsg_proc_o2t_iomsg
Parameters: socket structure pointer
	        message buffer
	        message size
Returns:    SUCCESS/FAILURE

This function is called when a UDP message is received on one of the
open I/O ports.  If the connection ID matches the port the message was
received on, the message is validated.
======================================================================= */
uint16 eips_iomsg_proc_o2t_iomsg (uint32 ipaddr, uint8 *msg, uint16 msg_siz)
{
    static EIPS_CPF_ITEMSTRUCT Items[EIPS_CPF_MAX_ITEM_CNT];
    uint16 ItemCount, buf_ix, PDUSeqNum, iomsg_ix;
    uint16 data_ix, i, j, output_size;
    uint32 PassedAddrSeqNum;
    uint32 ConnectionID;

    /* ************************ */
    /* Validate the CPF message */
    /* ************************ */
    buf_ix = 0;

    /* store the item count */
    ItemCount = rta_GetLitEndian16((msg+buf_ix));
    buf_ix+=2;

    /* if the item count is wrong, don't continue processing */
    if(ItemCount != 2)
        return(SUCCESS);

    /* Parse the CPF message into items */
    for(i=0; i<ItemCount; i++)
    {
        /* store the type */
        Items[i].TypeId = rta_GetLitEndian16((msg+buf_ix));
        buf_ix+=2;

        /* store the length */
        Items[i].Length = rta_GetLitEndian16((msg+buf_ix));
        buf_ix+=2;

        /* validate there is enough room for data */
        if(Items[i].Length > EIPS_CPF_MAX_ITEM_DATA_LEN)
	        return(FAILURE);

        /* store the data */
        rta_ByteMove(Items[i].Data, (msg+buf_ix), Items[i].Length);
        buf_ix = (uint16)(buf_ix + Items[i].Length);
    }

    /* if there wasn't the right amount of data, return */
    if(buf_ix != msg_siz)
    {
        eips_user_dbprint2("buf_ix (%d) != msg_siz (%d)\r\n", buf_ix, msg_siz);
        return(FAILURE);
    }

    /* ***************************************** */
    /* Addr Item Type: ADDR_TYP_ID_SEQADDRTYP */
    /* Addr Item Length: Always 8 */
    /* Data Item Type: DATA_TYP_ID_PDU */
    /* Data Item Length: Varies (>0) */
    /* Description: I/O Data */
    /* ***************************************** */
    if( (Items[EIPS_CPF_ADDR_ITEM].TypeId == ADDR_TYP_ID_SEQADDRTYP) &&
        (Items[EIPS_CPF_ADDR_ITEM].Length == 0x0008) &&
        (Items[EIPS_CPF_DATA_ITEM].TypeId == DATA_TYP_ID_PDU) &&
        (Items[EIPS_CPF_DATA_ITEM].Length > 0) )
    {
        /* store the passed connection id */
        ConnectionID = rta_GetLitEndian32(Items[EIPS_CPF_ADDR_ITEM].Data);

        /* store the passed address sequence id */
        PassedAddrSeqNum = rta_GetLitEndian32(Items[EIPS_CPF_ADDR_ITEM].Data+4);

/* eips_user_dbprint2("C:%08lx S:%08lx\r\n",ConnectionID, PassedAddrSeqNum); */

        /* validate the IP Address and CnxnID matches one of our connections */
        iomsg_ix = EIPS_USER_MAX_NUM_IO_CNXNS;
        for(i=0; ((i<EIPS_USER_MAX_NUM_IO_CNXNS)&&(iomsg_ix==EIPS_USER_MAX_NUM_IO_CNXNS)); i++)
        {
	        if( (eips_stc_iocnxns[i].O2T_IPAddr == ipaddr) &&
	            (eips_stc_iocnxns[i].CnxnStruct.O2T_CID == ConnectionID))
	        {
	            iomsg_ix = i;
	        }
        }

        /* validate the IP Address and Port matches one of our connections */
        if(iomsg_ix == EIPS_USER_MAX_NUM_IO_CNXNS)
        {
            /* this message isn't from our client */
			eips_user_dbprint1("IP Address Mismatch (%08lx) \r\n", eips_stc_iocnxns[iomsg_ix].CnxnStruct.O2T_CID);
            return(FAILURE);
        }

        /* validate the data is new */
        if(SEQ32_LEQ(PassedAddrSeqNum, eips_stc_iocnxns[iomsg_ix].O2T_SeqNum))
        {
            /* this message isn't new */
            eips_user_dbprint1("UDPIO Old Data (%08lx) \r\n", eips_stc_iocnxns[iomsg_ix].CnxnStruct.O2T_CID);
	        return(FAILURE);
        }

        /* this is new data, store the sequence number */
        eips_stc_iocnxns[iomsg_ix].O2T_SeqNum = PassedAddrSeqNum;

        /* if the transport class is 1, add the 16-bit PDU Sequence Number */
        if( (eips_stc_iocnxns[iomsg_ix].CnxnStruct.xport_type_and_trigger == EIPS_XPORTCLASS_CYC_SEQ) ||
	        (eips_stc_iocnxns[iomsg_ix].CnxnStruct.xport_type_and_trigger == EIPS_XPORTCLASS_COS_SEQ) ||
	        (eips_stc_iocnxns[iomsg_ix].CnxnStruct.xport_type_and_trigger == EIPS_XPORTCLASS_APP_SEQ))
        {
            /* make sure we have the minimum data */
            if(Items[EIPS_CPF_DATA_ITEM].Length < 2)
            {
                /* this message isn't new */
                eips_user_dbprint1("Items[EIPS_CPF_DATA_ITEM].Length < 2 (%08lx) \r\n", eips_stc_iocnxns[iomsg_ix].CnxnStruct.O2T_CID);
	            return(FAILURE);
            }

        /* Input Only     */
#ifdef EIPS_USEROBJ_ASM_INPUTONLY_HB_O2TINST
        /* don't tell the user about heartbeat messages */
        if(eips_stc_iocnxns[iomsg_ix].O2T_InstID == EIPS_USEROBJ_ASM_INPUTONLY_HB_O2TINST)
        {
            /* at this point, the message is valid, restart the timer */
            eips_timer_reset(eips_stc_iocnxns[iomsg_ix].CnxnStruct.O2T_TimerNum);
            return(SUCCESS);
        }
#endif

        /* Listen Only */
#ifdef EIPS_USEROBJ_ASM_LISTENONLY_HB_O2TINST
        /* don't tell the user about heartbeat messages     */
        if(eips_stc_iocnxns[iomsg_ix].O2T_InstID == EIPS_USEROBJ_ASM_LISTENONLY_HB_O2TINST)
        {
            /* at this point, the message is valid, restart the timer */
            eips_timer_reset(eips_stc_iocnxns[iomsg_ix].CnxnStruct.O2T_TimerNum);
            return(SUCCESS);
        }
#endif

            /* make sure we have the minimum data */
            if(Items[EIPS_CPF_DATA_ITEM].Length < 6)
            {
                eips_user_dbprint1("Items[EIPS_CPF_DATA_ITEM].Length < 6 (%08lx) \r\n", eips_stc_iocnxns[iomsg_ix].CnxnStruct.O2T_CID);
	            return(FAILURE);
            }

	        /* store the number of data bytes */
	        output_size = (uint16)(Items[EIPS_CPF_DATA_ITEM].Length - 6); /* account for RUN/IDLE & Seq # */

            /* see if too much data was passed */
#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
            if(output_size != (eips_stc_iocnxns[iomsg_ix].O2T_Size*4))
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
            if(output_size != (eips_stc_iocnxns[iomsg_ix].O2T_Size*2))
#else                                       /* BYTES */
            if(output_size != (eips_stc_iocnxns[iomsg_ix].O2T_Size))
#endif
            {
                return(FAILURE);
            }

            /* at this point, the message is valid, restart the timer */
            eips_timer_reset(eips_stc_iocnxns[iomsg_ix].CnxnStruct.O2T_TimerNum);
			temptest = eips_stc_iocnxns[iomsg_ix].CnxnStruct.O2T_TimerNum;
	        /* get the sequence number */
	        PDUSeqNum = rta_GetLitEndian16(Items[EIPS_CPF_DATA_ITEM].Data);

	        /* get the 4 byte RUN/IDLE info */
	        eips_stc_iocnxns[iomsg_ix].RunIdleInfo = rta_GetLitEndian32(Items[EIPS_CPF_DATA_ITEM].Data+2);

	        /* we are in IDLE mode, so return */
	        if((eips_stc_iocnxns[iomsg_ix].RunIdleInfo & 1) == 0)
			{
	            //eips_user_dbprint1("cxn idle! %d",ipaddr);
				return(SUCCESS);
			}

	        /* this is new data */
	        if(eips_stc_iocnxns[iomsg_ix].CnxnStruct.O2T_SeqNum != PDUSeqNum)
	        {
	            /* store the new sequence number */
	            eips_stc_iocnxns[iomsg_ix].CnxnStruct.O2T_SeqNum = PDUSeqNum;

                /* store the data index */
                data_ix = 6;
            }

            /* old data, return */
            else
                return(SUCCESS);

        } /* END "if the transport class is 1, add the 16-bit PDU Sequence Number" */

        /* no sequence number is used */
        else
        {
        /* Input Only     */
#ifdef EIPS_USEROBJ_ASM_INPUTONLY_HB_O2TINST
        /* don't tell the user about heartbeat messages */
        if(eips_stc_iocnxns[iomsg_ix].O2T_InstID == EIPS_USEROBJ_ASM_INPUTONLY_HB_O2TINST)
        {
            /* at this point, the message is valid, restart the timer */
            eips_timer_reset(eips_stc_iocnxns[iomsg_ix].CnxnStruct.O2T_TimerNum);
            return(SUCCESS);
        }
#endif

        /* Listen Only */
#ifdef EIPS_USEROBJ_ASM_LISTENONLY_HB_O2TINST
        /* don't tell the user about heartbeat messages     */
        if(eips_stc_iocnxns[iomsg_ix].O2T_InstID == EIPS_USEROBJ_ASM_LISTENONLY_HB_O2TINST)
        {
            /* at this point, the message is valid, restart the timer */
            eips_timer_reset(eips_stc_iocnxns[iomsg_ix].CnxnStruct.O2T_TimerNum);
            return(SUCCESS);
        }
#endif

	        /* make sure we have the minimum data */
	        if(Items[EIPS_CPF_DATA_ITEM].Length < 4)
	            return(FAILURE);

	        /* store the number of data bytes */
	        output_size = (uint16)(Items[EIPS_CPF_DATA_ITEM].Length - 4);

            /* see if too much data was passed */
#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
            if(output_size != (eips_stc_iocnxns[iomsg_ix].O2T_Size*4))
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
            if(output_size != (eips_stc_iocnxns[iomsg_ix].O2T_Size*2))
#else                                       /* BYTES */
            if(output_size != (eips_stc_iocnxns[iomsg_ix].O2T_Size))
#endif
            {
                return(FAILURE);
            }

            /* at this point, the message is valid, restart the timer */
            eips_timer_reset(eips_stc_iocnxns[iomsg_ix].CnxnStruct.O2T_TimerNum);

	        /* get the 4 byte RUN/IDLE info */
            eips_stc_iocnxns[iomsg_ix].RunIdleInfo = rta_GetLitEndian32(Items[EIPS_CPF_DATA_ITEM].Data+2);

            /* we are in IDLE mode, do nothing */
            if((eips_stc_iocnxns[iomsg_ix].RunIdleInfo & 1) == 0)
			{
                //eips_user_dbprint1("cxn idle! %d",ipaddr);
				return(SUCCESS);
			}

            /* store the data index */
            data_ix = 4;
        }

        /* get the MUTEX on the shared data */
        EIPS_GET_MUTEX;

        /* store the data and alert the user app with how much was written */
        for(i=0, j=0; i<output_size; )
        {
#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
            eips_stc_iocnxns[iomsg_ix].O2T_DataPtr[j++] = rta_GetLitEndian32(Items[EIPS_CPF_DATA_ITEM].Data+data_ix+i);
            i+=4;
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
            eips_stc_iocnxns[iomsg_ix].O2T_DataPtr[j++] = rta_GetLitEndian16(Items[EIPS_CPF_DATA_ITEM].Data+data_ix+i);
            i+=2;
#else                                       /* BYTES */
            eips_stc_iocnxns[iomsg_ix].O2T_DataPtr[j++] = Items[EIPS_CPF_DATA_ITEM].Data[data_ix+i];
            i+=1;
#endif
        }

        /* return the MUTEX on the shared data */
        EIPS_PUT_MUTEX;

        /* Input Only     */
#ifdef EIPS_USEROBJ_ASM_INPUTONLY_HB_O2TINST
        /* don't tell the user about heartbeat messages */
        if(eips_stc_iocnxns[iomsg_ix].O2T_InstID == EIPS_USEROBJ_ASM_INPUTONLY_HB_O2TINST)
            return(SUCCESS);
#endif

        /* Listen Only */
#ifdef EIPS_USEROBJ_ASM_LISTENONLY_HB_O2TINST
        /* don't tell the user about heartbeat messages     */
        if(eips_stc_iocnxns[iomsg_ix].O2T_InstID == EIPS_USEROBJ_ASM_LISTENONLY_HB_O2TINST)
            return(SUCCESS);
#endif

        /* inform the user of the write */
#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
        eips_userobj_asmDataWrote(eips_stc_iocnxns[iomsg_ix].O2T_InstID, (output_size/4));
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
        eips_userobj_asmDataWrote(eips_stc_iocnxns[iomsg_ix].O2T_InstID, (output_size/2));
#else                                       /* BYTES */
        eips_userobj_asmDataWrote(eips_stc_iocnxns[iomsg_ix].O2T_InstID, output_size);
#endif
    }

    /* we are done */
    return(SUCCESS);
}

/* ====================================================================
Function:   eips_iomsg_t2o_cos
Parameters: instance id that changed value
Returns:    N/A

This function is called by the user when the produce (t2o) data changes.
If the instance id is used, the sequence number is incremented.
======================================================================= */
void eips_iomsg_t2o_cos (uint16 t2o_inst_id)
{
#ifndef EIPS_USEROBJ_AUTOINC_CLASS1_PDUSEQNUM
    uint16 i;

    /* find which I/O connection uses this instance id */
    for(i=0; i<EIPS_USER_MAX_NUM_IO_CNXNS; i++)
    {
        /* make sure the connection is allocated and the instance is used */
        if( (eips_stc_iocnxns[i].state == EIPS_CNXN_STATE_USED) &&
            (eips_stc_iocnxns[i].T2O_InstID == t2o_inst_id))
        {
            /* increment the sequence number */
            eips_stc_iocnxns[i].CnxnStruct.T2O_SeqNum++;
        }
    }
#endif
}

/**/
/* ******************************************************************** */
/*                          LOCAL FUNCTIONS                             */
/* ******************************************************************** */
/* ====================================================================
Function:   local_iomsg_StructInit
Parameters: pointer to the I/O connection structure
Returns:    N/A

This function initializes all elements in the structure.
======================================================================= */
static void local_iomsg_StructInit (EIPS_IOCNXN_STRUCT *temp_cnxn)
{
    /* do nothing if this pointer is NULL */
    if(temp_cnxn == NULL)
        return;

    /* reset the connection parameters */
    eips_cnxn_structInit(&temp_cnxn->CnxnStruct);
    temp_cnxn->state = EIPS_CNXN_STATE_FREE;

    /* O_to_T Variables */
    temp_cnxn->O2T_IPAddr   = 0x00000000L;
    temp_cnxn->O2T_SeqNum   = 0xFFFFFFFFL;
    temp_cnxn->O2T_Port     = 0;
    temp_cnxn->O2T_InstID   = 0xFFFF;
    temp_cnxn->O2T_DataPtr  = NULL;
    temp_cnxn->O2T_Size	    = 0;

    /* T_to_O Variables */
    temp_cnxn->T2O_IPAddr   = 0x00000000L;
    temp_cnxn->T2O_SeqNum   = 0;
    temp_cnxn->T2O_Port	    = 0;
    temp_cnxn->T2O_InstID   = 0xFFFF;
    temp_cnxn->T2O_DataPtr  = NULL;
    temp_cnxn->T2O_Size	    = 0;
}

/* ====================================================================
Function:   local_iomsg_GetUnusedCnxn
Parameters: N/A
Returns:    Valid IO Message structure or NULL (on error)

This function initializes all static variables used for EtherNet/IP I/O
messaging.
======================================================================= */
static EIPS_IOCNXN_STRUCT * local_iomsg_GetUnusedCnxn(void)
{
    uint16 i;

    /* find the next unused connection */
    for(i=0; i<EIPS_USER_MAX_NUM_IO_CNXNS; i++)
    {
        /* if a connection structure is free, return it */
        if(eips_stc_iocnxns[i].state == EIPS_CNXN_STATE_FREE)
        {
	        /* change the state to USED */
	        eips_stc_iocnxns[i].state = EIPS_CNXN_STATE_USED;

	        /* return the connection pointer */
	        return(&eips_stc_iocnxns[i]);
        }
    }

    return(NULL);
}

/* ====================================================================
Function:   local_iomsg_produce
Parameters: pointer to the I/O connection structure
Returns:    N/A

This function builds and transmits the I/O data.
======================================================================= */
static void local_iomsg_produce (EIPS_IOCNXN_STRUCT *iomsg)
{
    static uint8  prod_buf [EIPS_USER_IOBUF_SIZ+50];
    uint16 prod_buf_siz, i;
    uint16 actual_data_size = 0;

#ifdef EIPS_USEROBJ_ASM_OUTPUTONLY_HB_O2TINST
    /* invalid input data pointer (and we expect one) */
    if( (iomsg->T2O_InstID != EIPS_USEROBJ_ASM_OUTPUTONLY_HB_O2TINST) &&
        (iomsg->T2O_DataPtr == NULL))
    {
        return;
    }
#else
    /* invalid input data pointer */
    if(iomsg->T2O_DataPtr == NULL)
    {
        return;
    }
#endif

    /* Build response message  - use bufsiz as the index */
    prod_buf_siz = 0;

    /* Store the ItemCount */
    rta_PutLitEndian16(2, prod_buf+prod_buf_siz);
    prod_buf_siz+=2;

    /* Store the Address Item - TypeID */
    rta_PutLitEndian16(ADDR_TYP_ID_SEQADDRTYP, prod_buf+prod_buf_siz);
    prod_buf_siz+=2;

    /* Store the Address Item - Length */
    rta_PutLitEndian16(8, prod_buf+prod_buf_siz);
    prod_buf_siz+=2;

    /* Store the Address Item - Data (T to O Connection ID) */
    rta_PutLitEndian32(iomsg->CnxnStruct.T2O_CID, prod_buf+prod_buf_siz);
    prod_buf_siz+=4;

    /* Store the Address Item - Data (T to O Sequence Number) */
    iomsg->T2O_SeqNum++;
    rta_PutLitEndian32(iomsg->T2O_SeqNum, prod_buf+prod_buf_siz);
    prod_buf_siz+=4;

    /* Store the Data Item - TypeID */
    rta_PutLitEndian16(DATA_TYP_ID_PDU, prod_buf+prod_buf_siz);
    prod_buf_siz+=2;

    /* get the MUTEX on the shared data */
    EIPS_GET_MUTEX;

    /* calc the data size */
#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
    actual_data_size = (iomsg->T2O_Size*4);
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
    actual_data_size = (iomsg->T2O_Size*2);
#else                                       /* BYTES */
    actual_data_size = iomsg->T2O_Size;
#endif

    /* optional code if the T2O Run Time Header is used */
#ifdef EIPS_USEROBJ_T2O_RUNTIME_HEADER_USED
    actual_data_size += 4;
#endif

    /* if the transport class is 1, add the 16-bit PDU Sequence Number */
    if( (iomsg->CnxnStruct.xport_type_and_trigger == EIPS_XPORTCLASS_CYC_SEQ) ||
        (iomsg->CnxnStruct.xport_type_and_trigger == EIPS_XPORTCLASS_COS_SEQ) ||
        (iomsg->CnxnStruct.xport_type_and_trigger == EIPS_XPORTCLASS_APP_SEQ))
    {
        /* store the size including the data, header (if used) and seq num */
        rta_PutLitEndian16((uint16)(actual_data_size+2), prod_buf+prod_buf_siz);

        /* increment the produce buffer size */
        prod_buf_siz+=2;

        /* check if the stack is supposed to increment the sequence number */
#ifdef EIPS_USEROBJ_AUTOINC_CLASS1_PDUSEQNUM
        iomsg->CnxnStruct.T2O_SeqNum++;
#endif

        /* Store the Transport PDU Sequence Number */
        rta_PutLitEndian16(iomsg->CnxnStruct.T2O_SeqNum, prod_buf+prod_buf_siz);
        prod_buf_siz+=2;
    }

    /* we don't use the 16-bit PDU Sequence Number */
    else
    {
        /* store the size including the data, header (if used) and seq num */
        rta_PutLitEndian16((uint16)(actual_data_size+2), prod_buf+prod_buf_siz);

        /* store the produce buffer size */
        prod_buf_siz+=2;
    }

    /* optional code if the T2O Run Time Header is used */
#ifdef EIPS_USEROBJ_T2O_RUNTIME_HEADER_USED
    rta_PutLitEndian32(eips_userobj_getT2ORuntimeHeader(iomsg->T2O_InstID), prod_buf+prod_buf_siz);

    /* store the produce buffer size */
    prod_buf_siz+=4;
#endif

    /* Store the Data */
    for(i=0; i<iomsg->T2O_Size; i++)
    {
#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
        rta_PutLitEndian32(iomsg->T2O_DataPtr[i], prod_buf+prod_buf_siz);
        prod_buf_siz+=4;
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
        rta_PutLitEndian16(iomsg->T2O_DataPtr[i], prod_buf+prod_buf_siz);
        prod_buf_siz+=2;
#else                                       /* BYTES */
        prod_buf[prod_buf_siz] = iomsg->T2O_DataPtr[i];
        prod_buf_siz+=1;
#endif
    }

    /* return the MUTEX on the shared data */
    EIPS_PUT_MUTEX;

    /* ****************************************** */
    /* Send UDP Message --> Use send(), NOT write */
    /* ****************************************** */

#ifdef EIPS_QOS_USED
    eips_usersock_sendIOMessage_QoS (iomsg->T2O_IPAddr, prod_buf, prod_buf_siz, iomsg->dscp_type, iomsg->dscp_value);
#else
    eips_usersock_sendIOMessage (iomsg->T2O_IPAddr, prod_buf, prod_buf_siz);
#endif
}

/* ====================================================================
Function:   local_iomsg_free_tmo
Parameters: pointer to I/O connection
Returns:    N/A

This function closes all resources allocated to I/O messaging.	This
is needed to ensure sockets are closed, otherwise init could be called
again.

IN - Input Only
LO - Listen Only
EO - Exclusive Owner

1st Conn    2nd Conn    Timeout 1st Conn        Close 1st Conn
--------    --------    ----------------        --------------
   IN          EO       EO stays open           EO stays open
   IN          IN       2nd IN stays open       2nd IN stays open
   IN          LO       LO closes               LO closes
   EO          IN       IN closes               IN stays open*
   EO          LO       LO closes               LO closes

   EO          EO       Invalid                 Invalid
   LO          N/A      Invalid                 Invalid

* The only difference between a timeout and a close is when an Exclusive
Owner is allocated first, then an Input Only. Since a ForwardClose is an
explicit act, the EO connection knows to stop receiving the Mulicast
traffic. On a timeout, the EO connection doesn't know to stop receiving
the traffic.
======================================================================= */
static void local_iomsg_free_tmo (EIPS_IOCNXN_STRUCT *io_cnxn)
{
    int16   i;
    uint32  mult_addr, seq_num;
    uint8   io_type;

    /* validate the pointer isn't NULL */
    if(io_cnxn == NULL)
        return;

    /* get the I/O connection type */
    io_type = local_iomsg_getConnectionType(((uint8)io_cnxn->O2T_InstID));
    mult_addr = io_cnxn->T2O_IPAddr;
    seq_num = io_cnxn->T2O_SeqNum;

    /* close the Exclusive Owner connection */
    local_iomsg_StructInit(io_cnxn);

    if(!eips_iomsg_ValidMulticastAddr(mult_addr))
        return;

    /* switch on the I/O connection type */
    switch(io_type)
    {
        /* ----------------------------------------------------------------------------- */
        /*                  EXCLUSIVE OWNER */
        /* */
        /* Close all connections with our Multicast address */
        /* ----------------------------------------------------------------------------- */
        case EIPS_IOTYPE_EXOWNER:
#ifdef EIPS_USEROBJ_ASM_INPUTONLY_HB_O2TINST
            /* check for connections with our multicast */
            for(i=0; i<EIPS_USER_MAX_NUM_IO_CNXNS; i++)
            {
                /* we found a connection that matches */
                if(eips_stc_iocnxns[i].T2O_IPAddr == mult_addr)
                {
                    local_iomsg_StructInit(&eips_stc_iocnxns[i]);
                }

            }
#endif
            break;

        default:
            /* we should never get here!!!! */
            break;
    }

    /* make sure at least one device is still transmitting on our multicast address */
    local_iomsg_KickStartMulticast(mult_addr, seq_num);
}

/* ====================================================================
Function:   local_iomsg_t2oTimeout
Parameters: timer number that timed out.
Returns:    N/A

This function is called when a connection times out.  If the timer
number is valid, produce the current I/O data.
======================================================================= */
static void local_iomsg_t2oTimeout (uint16 timer_num)
{
    uint16 i, iomsg_ix;

    /* find which connection timed out */
    iomsg_ix = EIPS_USER_MAX_NUM_IO_CNXNS;
    for(i=0; ((i<EIPS_USER_MAX_NUM_IO_CNXNS)&&(iomsg_ix==EIPS_USER_MAX_NUM_IO_CNXNS)); i++)
    {
        if(eips_stc_iocnxns[i].CnxnStruct.T2O_TimerNum == timer_num)
        {
	        iomsg_ix = i;
        }
    }

    /* we found the connection that timed out, send an I/O message */
    if(iomsg_ix != EIPS_USER_MAX_NUM_IO_CNXNS)
    {
        local_iomsg_produce (&eips_stc_iocnxns[iomsg_ix]);
        eips_timer_reset(timer_num);
    }
}

/* ====================================================================
Function:   local_iomsg_o2tTimeout
Parameters: timer number that timed out.
Returns:    N/A

This function is called when a connection times out.  If the timer
number is valid, produce the current I/O data.
======================================================================= */
static void local_iomsg_o2tTimeout (uint16 timer_num)
{
    uint16 i;

    /* find which connection timed out */
    for(i=0; i<EIPS_USER_MAX_NUM_IO_CNXNS; i++)
    {
        if(eips_stc_iocnxns[i].CnxnStruct.O2T_TimerNum == timer_num)
        {
            /* exclusive owner is the only one to go red */
            if(eips_stc_iocnxns[i].IO_Type == EIPS_IOTYPE_EXOWNER)
                eips_cnxn_timeout();
            local_iomsg_free_tmo(&eips_stc_iocnxns[i]);
            return;
        }
    }
}

/* ====================================================================
Function:   local_iomsg_buildError
Parameters: pointer to CPF message structure
            general status
            extended status size (0 or 1)
            extended status code
Returns:    N/A

This function builds the error message for the I/O Forward Open or
Forward Close.
======================================================================= */
static void local_iomsg_buildError (CPF_MESSAGE_STRUCT *cpf, uint8 gen_stat, uint8 ext_size, uint16 *p_ext_err)
{
    uint16 i;

    /* validate we have a cpf pointer */
    if(!cpf || !p_ext_err)
        return;

    /* store the general response message */
    cpf->emrsp->user_struct.gen_stat = gen_stat;

    /* make sure the extended status size is valid */
    if ((ext_size > IOMSG_MAX_EXT_STAT_SIZE) || (ext_size > EIPS_USER_MSGRTR_EXTSTAT_SIZE))
        ext_size = RTA_MIN(IOMSG_MAX_EXT_STAT_SIZE, EIPS_USER_MSGRTR_EXTSTAT_SIZE);

    /* store the extended status */
    cpf->emrsp->user_struct.ext_stat_size = ext_size;

    for(i=0; i<ext_size; i++)
        cpf->emrsp->user_struct.ext_stat[i] = p_ext_err[i];
}

/* ====================================================================
Function:     local_iomsg_getMulticastAddr
Parameters:   N/A
Returns:      32-bit IP Address

This function returns the multicast address to transmit with.

Each host gets 32 multicast addresses.	The starting and ending
address are based on the IP address of the device.

(from the ODVA spec)
CIP_Mcast_Base_Addr = 0xEFC00100    -> 239.192.1.0 is the starting address
CIP_Host_Mask = 0x03FF              -> 10-bit of of host id

Host_id = (IP_addr & (~SubnetMask);
Mcast_index = (Host_id - 1);        -> x.x.x.0 is invalid for IP addresses
Mcast_index &= CIP_Host_Mask;       -> only use bottom 10-bits

Mcast_address = CIP_Mcast_Base_Addr + (Mcast_index * 32) + mult_id

i.e.
    xxx.xxx.xxx.0   would get 239.192.1.0    - 239.192.1.31
    xxx.xxx.xxx.1   would get 239.192.1.32   - 239.192.1.63
======================================================================= */
static uint32 local_iomsg_getMulticastAddr (void)
{
    static uint8 mult_id = 0;
    EIPS_IPADDR_STRUCT tcp;
    uint8   mult_used;
    uint16  i, j;
    uint32  mult_temp32;

    /* read the IP and subnet mask from the user */
    if(eips_usersock_getTCPObj_Attr05_IntfCfg (&tcp) != ERR_SUCCESS)
        return(0L);/* return an error */

    /* store the fixed portion of the address */
    mult_temp32 = (((tcp.IPAddress & (~tcp.NtwkMask))-1) & 0x000003FFl);
    mult_temp32 = (0xEFC00100l + (mult_temp32 * 32));

    /* calculate the next multicast address and see if it's used */
    for(i=0; i<32; i++)
    {
        /* increment the id and roll over if needed */
        mult_id++;
        if (mult_id >= 32)
            mult_id = 0;

        /* get the multicast address */
        mult_temp32 = (mult_temp32 + (uint32)mult_id);

        /* check if this IP is used by a second I/O connection */
        for(mult_used = FALSE, j=0; j<EIPS_USER_MAX_NUM_IO_CNXNS && mult_used == FALSE; j++)
        {
            /* we already are using this address */
            if(eips_stc_iocnxns[j].T2O_IPAddr == mult_temp32)
                mult_used = TRUE;
        }

        /* we found an unused IP */
        if(mult_used == FALSE)
        {
            /* return the new multicast address */
            return(mult_temp32);
        }
    }

    /* return an error */
    return(0L);
}

/* ====================================================================
Function:   eips_iomsg_ValidMulticastAddr
Parameters: 32-bit IP Address
Returns:    TRUE    - Multicast Address
            FALSE   - Not a Multicast Address

This function returns TRUE if the passed address is a multicast address.
======================================================================= */
uint8 eips_iomsg_ValidMulticastAddr (uint32 addr)
{
    uint8   mult_array[4];

    rta_PutBigEndian32(addr, mult_array);

    /* multicast found */
    if(mult_array[0] >= 224 && mult_array[0] <= 239)
        return(TRUE);

    return(FALSE);
}

/* ====================================================================
Function:     local_iomsg_getOwningConnection
Parameters:   T2O Connection ID
Returns:      Pointer to owning connection.

This function returns the first found owning connection pointer.
======================================================================= */
static EIPS_IOCNXN_STRUCT * local_iomsg_getOwningConnection (uint16 t2o_cnxnid)
{
    uint16 i;

    /* look for a use connection that matches our t2o and is multicast */
    for(i=0; i<EIPS_USER_MAX_NUM_IO_CNXNS; i++)
    {
        /* make sure the connection is multicast, the connection id's match  */
        /* and the connection serial number doesn't match */
        if ((((uint16)(eips_stc_iocnxns[i].CnxnStruct.T2O_cnxn_params & NTWKCNXNPRM_CNXNTYP_ANDVAL))
                == NTWKCNXNPRM_CNXNTYP_MULTI) && (eips_stc_iocnxns[i].T2O_InstID == t2o_cnxnid))
        {
            return(&eips_stc_iocnxns[i]);
        }
    }

    /* no connection found */
    return(NULL);
}

/* ====================================================================
Function:   local_iomsg_getExOwningConnection
Parameters: T2O Connection ID
            O2T Connection ID
Returns:    Pointer to owning connection.

This function returns the exclusive owning connection pointer.
======================================================================= */
static EIPS_IOCNXN_STRUCT * local_iomsg_getExOwningConnection (uint16 t2o_cnxnid, uint16 o2t_cnxnid)
{
    uint16 i;

    /* look for a use connection that matches our t2o and is multicast */
    for(i=0; i<EIPS_USER_MAX_NUM_IO_CNXNS; i++)
    {
        /* if this combination of O2T and T2O matches, we have an exclusive owner */
        if ((eips_stc_iocnxns[i].T2O_InstID == t2o_cnxnid) && (eips_stc_iocnxns[i].O2T_InstID == o2t_cnxnid))
        {
            return(&eips_stc_iocnxns[i]);
        }
    }

    /* no connection found */
    return(NULL);
}

/* ====================================================================
Function:   local_iomsg_getConnectionType
Parameters: I/O structure pointer
Returns:    EIPS_IOTYPE_EXOWNER     -   Exclusive Owner
            EIPS_IOTYPE_INPUTONLY   -   Input Only (if supported)
            EIPS_IOTYPE_LISTENONLY  -   Listen Only (if supported)
            EIPS_IOTYPE_INVALID     -   Error Type

This function returns the connection type for the passed connection
structure or EIPS_IOTYPE_INVALID on error.
======================================================================= */
static uint8 local_iomsg_getConnectionType (uint8 cnxnpt_o2t)
{
    /* switch on the O2T Connection Instance ID */
    switch(cnxnpt_o2t)
    {
#ifdef EIPS_USEROBJ_ASM_INPUTONLY_HB_O2TINST
        case EIPS_USEROBJ_ASM_INPUTONLY_HB_O2TINST:
            return(EIPS_IOTYPE_INPUTONLY);
#endif

#ifdef EIPS_USEROBJ_ASM_LISTENONLY_HB_O2TINST
        case EIPS_USEROBJ_ASM_LISTENONLY_HB_O2TINST:
            return(EIPS_IOTYPE_LISTENONLY);
#endif

        /* Exclusive Owner     */
        default:
            return(EIPS_IOTYPE_EXOWNER);
    };
}

/* ====================================================================
Function:   local_iomsg_KickStartMulticast
Parameters: multicast IP address
Returns:    N/A

This function makes sure one I/O connection is transmitting on the
passed multicast address.
======================================================================= */
static void local_iomsg_KickStartMulticast (uint32 multi_addr, uint32 seq_num)
{
    uint16 i;

    /* check if any one is using our multicast address */
    for(i=0; i<EIPS_USER_MAX_NUM_IO_CNXNS; i++)
    {
        /* someone is transmitting... return */
        if ((eips_stc_iocnxns[i].T2O_IPAddr == multi_addr) &&
            (eips_timer_running (eips_stc_iocnxns[i].CnxnStruct.T2O_TimerNum) == TRUE))
        {
            return;
        }
    }

    /* find the first connection with our IP and start the timer */
    for(i=0; i<EIPS_USER_MAX_NUM_IO_CNXNS; i++)
    {
        /* someone is transmitting... return */
        if (eips_stc_iocnxns[i].T2O_IPAddr == multi_addr)
        {
            eips_stc_iocnxns[i].T2O_SeqNum = seq_num;
            eips_timer_reset(eips_stc_iocnxns[i].CnxnStruct.T2O_TimerNum);
            return;
        }
    }
}

/* *********** */
/* END OF FILE */
/* *********** */
