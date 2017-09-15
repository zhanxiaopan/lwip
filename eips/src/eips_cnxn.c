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
 *     Module Name: eips_cnxn.c
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains the definitions needed for the EtherNet/IP Server
 * connection processing.
 *
 */

/* ---------------------------- */
/* INCLUDE FILES                */
/* ---------------------------- */
#include "eips_system.h"

/* used for debug */
#undef  __RTA_FILE__
#define __RTA_FILE__ "eips_cnxn.c"

int16 cnxntest = 0;
/* ---------------------------- */
/* LOCAL FUNCTION PROTOTYPES    */
/* ---------------------------- */
/* Forward Open Functions */
void eips_cnxn_parse_fwdopen (EIPS_CONNECTION_DATA_STRUCT *temp_cnxn, CPF_MESSAGE_STRUCT *cpf);

/* Forward Close Functions */
void eips_cnxn_parse_fwdclose (EIPS_CONNECTION_DATA_STRUCT *temp_cnxn, CPF_MESSAGE_STRUCT *cpf);

/* ---------------------------- */
/* STATIC VARIABLES             */
/* ---------------------------- */
static EIPS_CONNECTION_DATA_STRUCT eips_CnxnStruct[EIPS_CNXN_MAXTOTALCNXNS];
uint8 eips_cnxn_timeout_flag;

/* ---------------------------- */
/*      EXTERN VARIABLES        */
/* ---------------------------- */
extern EIPS_IDENTITY_OBJ_STRUCT    eips_IDObj;

#if EIPS_USEROBJ_ASM_MAXNUM_O2TINST > 0
    extern EIPS_ASSEMBLY_INST_STRUCT   eips_AsmO2TObj[EIPS_USEROBJ_ASM_MAXNUM_O2TINST];
#endif

#if EIPS_USEROBJ_ASM_MAXNUM_T2OINST > 0
    extern EIPS_ASSEMBLY_INST_STRUCT   eips_AsmT2OObj[EIPS_USEROBJ_ASM_MAXNUM_T2OINST];
#endif

#ifdef EIPS_NTWK_LED_USED
    extern uint8 eips_iomsg_connections_open;
#endif


/* ---------------------------- */
/* MISCELLANEOUS                */
/* ---------------------------- */
#define EIPS_MAXCNXNTMOMULT 7
EIPS_CODESPACE uint16 EIPS_CNXNTMOMULT[(EIPS_MAXCNXNTMOMULT+1)] = {4,8,16,32,64,128,256,512};
EIPS_CODESPACE uint32 EIPS_TIMETICKVALENUM[16] = {1L,2L,4L,8L,16L,32L,64L,128L,256L,512L,1024L,2048L,4096L,8192L,16384L,32768L};

#define EIPS_FWDOPENTAB_SIZE 10
EIPS_CODESPACE struct
{
    uint8 ClassID;
    uint8 InstID;
    uint8 Suported;
}EIPSFwdOpenTable[EIPS_FWDOPENTAB_SIZE] =
{
    /* ************************************* */
    /*    Class       Inst  Supported        */
    /* ************************************* */
    {CLASS_IDENTITY,   0,   FALSE},
    {CLASS_IDENTITY,   1,   FALSE},
    {CLASS_MSGROUTER,  0,   FALSE},
    {CLASS_MSGROUTER,  1,   FALSE},
    {CLASS_CONNECTMGR, 0,   FALSE},
    {CLASS_CONNECTMGR, 1,   TRUE},
    {CLASS_TCPIP,      0,   FALSE},
    {CLASS_TCPIP,      1,   FALSE},
    {CLASS_ENETLINK,   0,   FALSE},
    {CLASS_ENETLINK,   1,   FALSE},
};

#define EIPS_FWDCLOSETAB_SIZE 10
EIPS_CODESPACE struct
{
    uint8 ClassID;
    uint8 InstID;
    uint8 Suported;
}EIPSFwdCloseTable[EIPS_FWDCLOSETAB_SIZE] =
{
    /* ************************************* */
    /*    Class       Inst  Supported        */
    /* ************************************* */
    {CLASS_IDENTITY,   0,   FALSE},
    {CLASS_IDENTITY,   1,   FALSE},
    {CLASS_MSGROUTER,  0,   FALSE},
    {CLASS_MSGROUTER,  1,   FALSE},
    {CLASS_CONNECTMGR, 0,   FALSE},
    {CLASS_CONNECTMGR, 1,   TRUE},
    {CLASS_TCPIP,      0,   FALSE},
    {CLASS_TCPIP,      1,   FALSE},
    {CLASS_ENETLINK,   0,   FALSE},
    {CLASS_ENETLINK,   1,   FALSE},
};

/**/
/* ******************************************************************** */
/*                      GLOBAL FUNCTIONS                                */
/* ******************************************************************** */

/* ====================================================================
Function:   eips_cnxn_init
Parameters: init type
Returns:    N/A

This function initializes all static variables used in this file.
======================================================================= */
void eips_cnxn_init (uint8 init_type)
{
    uint16 i;
    EIPS_TIMER_DATA_STRUCT *timer_struct;

    if(init_type){} /*keep the compiler happy */

    eips_cnxn_timeout_flag = 0;

    /* look through all connections */
    for(i=0; i<EIPS_CNXN_MAXTOTALCNXNS; i++)
    {
        /* get the 2 needed timers */
        /* ***************** */
        /* get the O2T timer */
        /* ***************** */
        eips_CnxnStruct[i].O2T_TimerNum = eips_timer_new();

        /* get timer structure */
        timer_struct = eips_timer_get_struct(eips_CnxnStruct[i].O2T_TimerNum);

        /* we couldn't get the timer structure */
        if(timer_struct == NULL)
        {
            /* we couldn't get the timer */
            eips_usersys_fatalError("eips_cnxn_init (o2t)", i);
            return;
        }

        /* set up the timeout conditions (flag or function pointer) */
        timer_struct->timeout_flag_ptr = NULL;
        timer_struct->timeout_func_ptr = eips_cnxn_emTimeout;
#ifdef EIPS_TIMER_NAME_USED
        sprintf(timer_struct->timer_name, "Server_EM_%03d", i);
#endif

        /* ***************** */
        /* get the T2O timer */
        /* ***************** */
        eips_CnxnStruct[i].T2O_TimerNum = EIPS_TIMER_NULL;

        /* initialize the timer structures */
        eips_cnxn_structInit(&eips_CnxnStruct[i]);

        /* this shouldn't be modified by anyone */
        eips_CnxnStruct[i].unused_index = i;
    }
}

/* ====================================================================
Function:   eips_cnxn_Process
Parameters: N/A
Returns:    N/A

This function processes all connections.
======================================================================= */
void eips_cnxn_Process (void)
{
#ifdef EIPS_NTWK_LED_USED
    uint16 i;
    uint32 temp32;

    /* check for Ethernet Link */
    eips_usersock_getEthLinkObj_Attr02_IntfFlags(&temp32);
    if(!(temp32 & 0x00000001L)) /* bit 0 is the link status */
    {
        /* turn the Network LED Off */
        eips_usersys_nsLedUpdate (EIPS_LEDSTATE_OFF);
        return;
    }

    /* check for timeouts */
    if(eips_cnxn_timeout_flag)
    {
        /* flash the Network LED Red */
        eips_usersys_nsLedUpdate (EIPS_LEDSTATE_FLASH_RED);
        return;
    }

    /* see which connections are used */
    for(i=0; i<EIPS_CNXN_MAXTOTALCNXNS; i++)
    {
        /* one or more connection is established  */
        if( (eips_CnxnStruct[i].state == EIPS_CNXN_STATE_USED) ||
            (eips_iomsg_connections_open) )
        {
            /* steady Green Network LED  */
            eips_usersys_nsLedUpdate (EIPS_LEDSTATE_STEADY_GREEN);
            return;
        }
    }

    /* flashing Green Network LED - no connections */
    eips_usersys_nsLedUpdate (EIPS_LEDSTATE_FLASH_GREEN);
#endif
}

/* ====================================================================
Function:   eips_cnxn_timeout
Parameters: N/A
Returns:    N/A

This function is called when any connection times out.
======================================================================= */
void eips_cnxn_timeout (void)
{
    /* used for LED processing */
    eips_cnxn_timeout_flag = 1; /* cleared when a new connection is allocated */
}

/* ====================================================================
Function:   eips_cnxn_emTimeout
Parameters: timer_index
Returns:    N/A

This function finds which timer expired and processes it.
======================================================================= */
void eips_cnxn_emTimeout (uint16 timer_num)
{
    uint16 i,j;
    EIPS_SESSION_STRUCT *session;
    uint8 cnxn_used = 0;

    /*eips_user_dbprint1("eips_cnxn_EMTimeout %d\r\n", timer_num);*/
	cnxntest = timer_num;

    /* find which connection timed out */
    for(i=0; i<EIPS_CNXN_MAXTOTALCNXNS; i++)
    {
        /* we found a connection that timed out */
        if(eips_CnxnStruct[i].O2T_TimerNum == timer_num)
        {
            /* make sure the session exists */
            session = eips_encap_getSessionStruct (eips_CnxnStruct[i].SessionID);
            if(session == NULL)
            {
                /* we should never get here... this is a fatal error */
                return;
            }

            /* find the connection */
            for(j=0; j<EIPS_USER_MAX_NUM_EM_CNXNS_PER_SESSION; j++)
            {
                /* we have a valid connection */
                if(session->CnxnPtrs[j] != NULLPTR)
                {
                    /* we found the connection that timed out */
                    if(session->CnxnPtrs[j]->O2T_TimerNum == timer_num)
                    {
                        eips_cnxn_free (session->CnxnPtrs[j]);
                        session->CnxnPtrs[j] = NULL;
                        /*eips_user_dbprint0("eips_cnxn_EMTimeout free\r\n");*/
                    }
                }
            }

            /* if no one else is using the TCP connection, close it */
            for(j=0; j<EIPS_USER_MAX_NUM_EM_CNXNS_PER_SESSION; j++)
            {
                /* we have a valid connection */
                if(session->CnxnPtrs[j] != NULLPTR)
                    cnxn_used = 1;
            }
            if(!cnxn_used)
                RTA_TCP_CLOSE_WITH_PRINT(session->sock_id);
        }
   }
}

/* ====================================================================
Function:   eips_cnxn_getUnusedCnxn
Parameters: N/A
Returns:    Pointer to Free Connection or NULL on Error

This function looks for the next unused connection.  NULL is returned
on errors.  The valid connection pointer is returned otherwise.
======================================================================= */
EIPS_CONNECTION_DATA_STRUCT * eips_cnxn_getUnusedCnxn(void)
{
    uint16 i;

    /* find the next unused connection */
    for(i=0; i<EIPS_CNXN_MAXTOTALCNXNS; i++)
    {
        /* if a connection structure is free, return it */
        if(eips_CnxnStruct[i].state == EIPS_CNXN_STATE_FREE)
        {
            /* change the state to USED */
            eips_CnxnStruct[i].state = EIPS_CNXN_STATE_USED;

            /* return the connection pointer */
            return(&eips_CnxnStruct[i]);
        }
    }

    return(NULL);
}

/* ====================================================================
Function:   eips_cnxn_free
Parameters: pointer to the connection element
Returns:    N/A

This function free the connection and all associated timers.
======================================================================= */
void eips_cnxn_free (EIPS_CONNECTION_DATA_STRUCT *cnxnptr)
{
    /* don't do anything if the connection is NULL */
    if(cnxnptr == NULL)
        return;

    /* stop all associated timers */
    eips_timer_stop(cnxnptr->O2T_TimerNum);
    eips_timer_stop(cnxnptr->T2O_TimerNum);

    /* free the connection */
    eips_cnxn_structInit(cnxnptr);
}

/* ====================================================================
Function:   eips_cnxn_structInit
Parameters: pointer to the connection element
Returns:    N/A

This function initializes all variables in the connection structure.
======================================================================= */
void eips_cnxn_structInit(EIPS_CONNECTION_DATA_STRUCT *cnxnptr)
{
    /* stop the timers */
    eips_timer_stop(cnxnptr->O2T_TimerNum);
    eips_timer_stop(cnxnptr->T2O_TimerNum);

    /* set the connection data back to the default */
    cnxnptr->priority_and_tick = 0;
    cnxnptr->connection_time_out_ticks = 0;
    cnxnptr->O2T_CID = 0;
    cnxnptr->T2O_CID = 0;
    cnxnptr->connection_serial_number = 0;
    cnxnptr->vendor_ID = 0;
    cnxnptr->originator_serial_number = 0;
    cnxnptr->connection_timeout_multiplier = 0;
    cnxnptr->O2T_RPI = 0;
    cnxnptr->O2T_API = 0;
    cnxnptr->O2T_cnxn_params = 0;
    cnxnptr->O2T_cnxn_size = 0;
    cnxnptr->T2O_RPI = 0;
    cnxnptr->T2O_API = 0;
    cnxnptr->T2O_cnxn_params = 0;
    cnxnptr->T2O_cnxn_size = 0;
    cnxnptr->xport_type_and_trigger = 0;
    cnxnptr->connection_path_size = 0;

    /* variable for the connection processing */
    cnxnptr->SessionID = 0;
    cnxnptr->state = EIPS_CNXN_STATE_FREE;
    cnxnptr->O2T_SeqNum = 0;
    cnxnptr->T2O_SeqNum = 0;
}

/* ====================================================================
Function:   eips_cnxn_structCopy
Parameters: pointer to destination connection structure
            pointer to source connection structure
Returns:    N/A

This function copies all connection structure data from the source to
the destination connection structure.
======================================================================= */
void eips_cnxn_structCopy (EIPS_CONNECTION_DATA_STRUCT *dst_cnxnptr, EIPS_CONNECTION_DATA_STRUCT *src_cnxnptr)
{
    memcpy((uint8 *)&dst_cnxnptr->priority_and_tick, (uint8 *)&src_cnxnptr->priority_and_tick, sizeof(dst_cnxnptr->priority_and_tick));
    memcpy((uint8 *)&dst_cnxnptr->connection_time_out_ticks, (uint8 *)&src_cnxnptr->connection_time_out_ticks, sizeof(dst_cnxnptr->connection_time_out_ticks));
    memcpy((uint8 *)&dst_cnxnptr->O2T_CID, (uint8 *)&src_cnxnptr->O2T_CID, sizeof(dst_cnxnptr->O2T_CID));
    memcpy((uint8 *)&dst_cnxnptr->T2O_CID, (uint8 *)&src_cnxnptr->T2O_CID, sizeof(dst_cnxnptr->T2O_CID));
    memcpy((uint8 *)&dst_cnxnptr->connection_serial_number, (uint8 *)&src_cnxnptr->connection_serial_number, sizeof(dst_cnxnptr->connection_serial_number));
    memcpy((uint8 *)&dst_cnxnptr->vendor_ID, (uint8 *)&src_cnxnptr->vendor_ID, sizeof(dst_cnxnptr->vendor_ID));
    memcpy((uint8 *)&dst_cnxnptr->originator_serial_number, (uint8 *)&src_cnxnptr->originator_serial_number, sizeof(dst_cnxnptr->originator_serial_number));
    memcpy((uint8 *)&dst_cnxnptr->connection_timeout_multiplier, (uint8 *)&src_cnxnptr->connection_timeout_multiplier, sizeof(dst_cnxnptr->connection_timeout_multiplier));
    memcpy((uint8 *)&dst_cnxnptr->O2T_RPI, (uint8 *)&src_cnxnptr->O2T_RPI, sizeof(dst_cnxnptr->O2T_RPI));
    memcpy((uint8 *)&dst_cnxnptr->O2T_cnxn_params, (uint8 *)&src_cnxnptr->O2T_cnxn_params, sizeof(dst_cnxnptr->O2T_cnxn_params));
    memcpy((uint8 *)&dst_cnxnptr->O2T_cnxn_size, (uint8 *)&src_cnxnptr->O2T_cnxn_size, sizeof(dst_cnxnptr->O2T_cnxn_size));
    memcpy((uint8 *)&dst_cnxnptr->T2O_RPI, (uint8 *)&src_cnxnptr->T2O_RPI, sizeof(dst_cnxnptr->T2O_RPI));
    memcpy((uint8 *)&dst_cnxnptr->T2O_cnxn_params, (uint8 *)&src_cnxnptr->T2O_cnxn_params, sizeof(dst_cnxnptr->T2O_cnxn_params));
    memcpy((uint8 *)&dst_cnxnptr->T2O_cnxn_size, (uint8 *)&src_cnxnptr->T2O_cnxn_size, sizeof(dst_cnxnptr->T2O_cnxn_size));
    memcpy((uint8 *)&dst_cnxnptr->xport_type_and_trigger, (uint8 *)&src_cnxnptr->xport_type_and_trigger, sizeof(dst_cnxnptr->xport_type_and_trigger));
    memcpy((uint8 *)&dst_cnxnptr->connection_path_size, (uint8 *)&src_cnxnptr->connection_path_size, sizeof(dst_cnxnptr->connection_path_size));
    memcpy((uint8 *)dst_cnxnptr->connection_path, (uint8 *)src_cnxnptr->connection_path, sizeof(dst_cnxnptr->connection_path));
    memcpy((uint8 *)&dst_cnxnptr->O2T_API, (uint8 *)&src_cnxnptr->O2T_API, sizeof(dst_cnxnptr->O2T_API));
    memcpy((uint8 *)&dst_cnxnptr->T2O_API, (uint8 *)&src_cnxnptr->T2O_API, sizeof(dst_cnxnptr->T2O_API));
}

/* ====================================================================
Function:   eips_cnxn_getCnxnTmoMult
Parameters: connection timeout multiplier enumeration
Returns:    actual timeout multiplier (0 on error)

This function translates the passed timeout multiplier to the actual
multiplier.
======================================================================= */
uint16 eips_cnxn_getCnxnTmoMult (uint8 tmomult)
{
    /* there is an error */
    if(tmomult > EIPS_MAXCNXNTMOMULT)
        return(0);

    /* return the actual tmo mult */
    return (EIPS_CNXNTMOMULT[tmomult]);
}

/* ====================================================================
Function:   eips_cnxn_getConnectionID
Parameters: N/A
Returns:    22-bit IP Address

This function returns the next available connection id.
======================================================================= */
uint32 eips_cnxn_getConnectionID (void)
{
    static uint8    first_time = TRUE;
    static uint32   connection_id = 0;

    /* on the first time, set the base connection id  */
    /* this needs to be uniques on every power up */
    if(first_time == TRUE)
    {
        first_time = FALSE;
        connection_id = (eips_usersys_getIncarnationID() << 16);
    }

    /* get the next connection id and return it */
    connection_id++;
    return(connection_id);
}

/* ====================================================================
Function:   EIPS_APIROUND
Parameters: RPI
Returns:    API

This function rounds the API to our ticker resolution and returns the
new timer value.  If the rpi is less than our minimum tick resolution
return 0 so the RPI Error is returned.
======================================================================= */
uint32 EIPS_APIROUND (uint32 rpi)
{
#ifdef EIPS_USER_MIN_TICK_RES_IN_USEC
    /* the RPI value is too small (user def) */
	if(rpi < EIPS_USER_MIN_TICK_RES_IN_USEC)
		return(0);
#endif

    /* the RPI value is too small */
    if(rpi < EIPS_USER_TICK_RES_IN_USECS)
        return(0);

    /* rounding is needed */
    if(Xmod(rpi, EIPS_USER_TICK_RES_IN_USECS) > 0)
    {
        /* round the RPI up */
        rpi = rpi - Xmod(rpi, EIPS_USER_TICK_RES_IN_USECS);
        rpi = rpi + EIPS_USER_TICK_RES_IN_USECS;
    }
    return(rpi);
}

/* ====================================================================
Function:   EIPS_APITICKS
Parameters: API
Returns:    timer subsystem number of ticks

This function converts the API to the number of ticks the timer
subsystem needs.
======================================================================= */
uint32 EIPS_APITICKS (uint32 api)
{
    return((api / EIPS_USER_TICK_RES_IN_USECS));
}

/* ====================================================================
Function:   eips_cnxn_fwdOpen
Parameters: N/A
Returns:    N/A

This function processes the Forward_Open service code.

********************************************
* The Request Message format is as follows *
********************************************
********  ********************  ************************************
DataType  Parameter Name        Description
********  ********************  ************************************
(UINT8)   Priority/Time_tick    Used to calc req timeout info
(UINT8)   Time-out_ticks        Used to calc req timeout info
(UINT32)  O_to_T Connection ID  Cnxn ID to be used for O to T
(UINT32)  T_to_O Connection ID  Cnxn ID to be used for T to O
(UINT16)  Connection Serial #   SEE REQ / RSP PARAMS BELOW
(UINT16)  Originator Vendor ID  Vendor ID of the originating node
(UINT32)  Originator Serial #   Serial # of the originating node
(UINT8)   Cnxn Timeout Mult     SEE REQ / RSP PARAMS BELOW
(UINT8)   Reserved              Reserved
(UINT8)   Reserved              Reserved
(UINT8)   Reserved              Reserved
(UINT32)  O_to_T RPI            O to T req packet rate in microsecs
(UINT16/32) O_to_T Cnxn Params  SEE REQ / RSP PARAMS BELOW
(UINT32)  T_to_O RPI            T to O req packet rate in microsecs
(UINT16/32) T_to_O Cnxn Params  SEE REQ / RSP PARAMS BELOW
(UINT8)   Xport Type/Trigger    SEE REQ / RSP PARAMS BELOW
(UINT8)   Cnxn Path Size        The # of 16 bit words in Cnxn Path
(VARIES)  Cxnx Path             Route to Remote Target Device
======================================================================= */
void eips_cnxn_fwdOpen (CPF_MESSAGE_STRUCT *cpf)
{
    EIPS_CONNECTION_DATA_STRUCT temp_cnxn;
    uint8 class_id, inst_id;
    uint8 class_found, inst_found;
    uint8 path_seg, path_val;
    int16 i, table_ix, buf_ix;
    uint32 temp32;

    /* initialize variables */
    class_id = 0;
    inst_id = 0;
    class_found = FALSE;
    inst_found  = FALSE;
    eips_cnxn_structInit(&temp_cnxn);

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

    /* find if the Class and Instance are supported */
    class_found = FALSE;
    inst_found  = FALSE;
    table_ix = -1;
    for(i=0; ((i<EIPS_FWDOPENTAB_SIZE) && (inst_found == FALSE)); i++)
    {
        /* We found the Class */
        if(EIPSFwdOpenTable[i].ClassID == class_id)
        {
            class_found = TRUE;

            /* We also found the Instance */
            if(EIPSFwdOpenTable[i].InstID == inst_id)
            {
                inst_found = TRUE;
                table_ix = i;
            }
        }
    }

    /* We don't have a match in the lookup table */
    if((class_found == FALSE) || (inst_found == FALSE))
    {
        /* Error: Path Destination Unknown */
        cpf->emrsp->user_struct.gen_stat = ERR_PATHDESTUNKNOWN;
        cpf->emrsp->user_struct.ext_stat_size = 0;
        return;
    }

    /* The service code isn't supported */
    if(EIPSFwdOpenTable[table_ix].Suported == FALSE)
    {
        /* Error: Service Not Supported */
        cpf->emrsp->user_struct.gen_stat = ERR_SERV_UNSUPP;
        cpf->emrsp->user_struct.ext_stat_size = 0;
        return;
    }

    /* ****************************************** */
    /* We have a valid match, process the message */
    /* ****************************************** */
    buf_ix = 0;

    /* make sure our state is "EIPS_CNXN_STATE_USED" */
    temp_cnxn.state = EIPS_CNXN_STATE_USED;

    /* Store the Priority/Tick_time */
    temp_cnxn.priority_and_tick = cpf->emreq->user_struct.req_data[buf_ix++];

    /* Store the Time-out ticks */
    temp_cnxn.connection_time_out_ticks = cpf->emreq->user_struct.req_data[buf_ix++];

    /* Store the O_to_T Connection ID */
    temp_cnxn.O2T_CID = rta_GetLitEndian32(cpf->emreq->user_struct.req_data + buf_ix);
    buf_ix += 4; /* advance the pointer */

    /* Store the T_to_O Connection ID */
    temp_cnxn.T2O_CID = rta_GetLitEndian32(cpf->emreq->user_struct.req_data + buf_ix);
    buf_ix += 4; /* advance the pointer */

    /* Store the Connection Serial # */
    temp_cnxn.connection_serial_number = rta_GetLitEndian16(cpf->emreq->user_struct.req_data + buf_ix);
    buf_ix += 2; /* advance the pointer */

    /* Store the Originator Vendor ID */
    temp_cnxn.vendor_ID = rta_GetLitEndian16(cpf->emreq->user_struct.req_data + buf_ix);
    buf_ix += 2; /* advance the pointer */

    /* Store the Originator Serial Number */
    temp_cnxn.originator_serial_number = rta_GetLitEndian32(cpf->emreq->user_struct.req_data + buf_ix);
    buf_ix += 4; /* advance the pointer */

    /* Store the Connection Timeout Multiplier */
    temp_cnxn.connection_timeout_multiplier = cpf->emreq->user_struct.req_data[buf_ix++];

    /* The next 3 bytes are reserved... ignore them */
    buf_ix += 3;

    /* Store the O_to_T Requested Packet Interval (RPI) */
    temp_cnxn.O2T_RPI = rta_GetLitEndian32(cpf->emreq->user_struct.req_data + buf_ix);
    buf_ix += 4; /* advance the pointer */

    /* Store the O_to_T Connection Parameters */
    if(cpf->emreq->user_struct.service == CIP_SC_LRG_FWD_OPEN_REQ)
    {
        temp32 = rta_GetLitEndian32(cpf->emreq->user_struct.req_data + buf_ix);
        buf_ix += 4; /* advance the pointer */
        
        /* top 16 bits can remain the connection params so the existing code can work */
        temp_cnxn.O2T_cnxn_params = (uint16)((temp32 & 0xFFFF0000l) >> 16);

        /* bottom 16 bits is the connection size */
        temp_cnxn.O2T_cnxn_size = (uint16)(temp32 & 0x0000FFFFl);
    }
    else
    {
        /* standard 16 bits can remain the connection params so the existing code can work */
        temp_cnxn.O2T_cnxn_params = rta_GetLitEndian16(cpf->emreq->user_struct.req_data + buf_ix);
        buf_ix += 2; /* advance the pointer */

        /* mask off the connection params so we are left with the size */
        temp_cnxn.O2T_cnxn_size = (uint16)(temp_cnxn.O2T_cnxn_params & NTWKCNXNPRM_CNXN_SIZ_ANDVAL);
    }

    /* Store the T_to_O Requested Packet Interval (RPI) */
    temp_cnxn.T2O_RPI = rta_GetLitEndian32(cpf->emreq->user_struct.req_data + buf_ix);
    buf_ix += 4; /* advance the pointer */

    /* Store the T_to_O Connection Parameters */
    if(cpf->emreq->user_struct.service == CIP_SC_LRG_FWD_OPEN_REQ)
    {
        temp32 = rta_GetLitEndian32(cpf->emreq->user_struct.req_data + buf_ix);
        buf_ix += 4; /* advance the pointer */
        
        /* top 16 bits can remain the connection params so the existing code can work */
        temp_cnxn.T2O_cnxn_params = (uint16)((temp32 & 0xFFFF0000l) >> 16);

        /* bottom 16 bits is the connection size */
        temp_cnxn.T2O_cnxn_size = (uint16)(temp32 & 0x0000FFFFl);
    }
    else
    {
        /* standard 16 bits can remain the connection params so the existing code can work */
        temp_cnxn.T2O_cnxn_params = rta_GetLitEndian16(cpf->emreq->user_struct.req_data + buf_ix);
        buf_ix += 2; /* advance the pointer */

        /* mask off the connection params so we are left with the size */
        temp_cnxn.T2O_cnxn_size = (uint16)(temp_cnxn.T2O_cnxn_params & NTWKCNXNPRM_CNXN_SIZ_ANDVAL);
    }

    /* Store the Transport Type and Trigger */
    temp_cnxn.xport_type_and_trigger = cpf->emreq->user_struct.req_data[buf_ix++];

    /* Store the Connection Path Size */
    temp_cnxn.connection_path_size = cpf->emreq->user_struct.req_data[buf_ix++];

    /* Validate the Connection Path fits (size in words) */
    if((temp_cnxn.connection_path_size*2) > sizeof(temp_cnxn.connection_path))
    {
        /* Error: Resource Unavailable */
        cpf->emrsp->user_struct.gen_stat = ERR_RESOURCE_UNAVAIL;
        cpf->emrsp->user_struct.ext_stat_size = 0;
        return; /* exit on an error */
    }

    /* Store the Connection Path */
    for(i=0; i<temp_cnxn.connection_path_size; i++)
    {
        temp_cnxn.connection_path[i] = rta_GetLitEndian16(cpf->emreq->user_struct.req_data + buf_ix);
        buf_ix += 2; /* advance the pointer */
    }

    /* Not Enough Data was sent */
    if(buf_ix > cpf->emreq->user_struct.req_data_size)
    {
        /* Error: Not Enough Data */
        cpf->emrsp->user_struct.gen_stat = ERR_INSUFF_DATA;
        cpf->emrsp->user_struct.ext_stat_size = 0;
        return; /* exit on an error */
    }

    /* Too Much Data was sent */
    if(buf_ix < cpf->emreq->user_struct.req_data_size)
    {
        /* Error: Too Much Data */
        cpf->emrsp->user_struct.gen_stat = ERR_TOOMUCH_DATA;
        cpf->emrsp->user_struct.ext_stat_size = 0;
        return; /* exit on an error */
    }

    /* parse and process the forward open */
    eips_cnxn_parse_fwdopen (&temp_cnxn, cpf);
}

/* ====================================================================
Function:   eips_cnxn_parse_fwdopen
Parameters: N/A
Returns:    N/A

This function processes the Forward_Open service code.  If the passed
parameters are valid and supported, a new connection is opened and the
successful rsp_data is build.  Otherwise, an error message is built.
======================================================================= */
void eips_cnxn_parse_fwdopen (EIPS_CONNECTION_DATA_STRUCT *temp_cnxn, CPF_MESSAGE_STRUCT *cpf)
{
    uint16 i, temp16, cnxn_ix;
    EIPS_SESSION_STRUCT *session;
    uint8 class_id, inst_id;
    uint8 class_found, inst_found;
    uint8 path_seg, path_val;
    uint8 minor_rev, major_rev, compatibility;
    uint16 vend_id, dev_type, prod_code;
    uint32 temp32, current_temp, temp_o2t_rpi, temp_t2o_rpi;

    /* set the Class and Inst ID to 0 */
    class_id = 0;
    inst_id = 0;

    /* make sure the session exists */
    session = eips_encap_getSessionStruct (cpf->encap->SessionHandle);
    if(session == NULL)
    {
        return;
    }

    /* store the Session that opened the connection so we can process timeouts */
    temp_cnxn->SessionID = cpf->encap->SessionHandle;

    /* ***************************************** */
    /* Check if this is a Duplicate Forward Open */
    /* ***************************************** */

    /* Validate vendor_ID, connection_serial_number, originator_serial_number */
    for(i=0; i<EIPS_USER_MAX_NUM_EM_CNXNS_PER_SESSION; i++)
    {
        /* we found an open connection */
        if(session->CnxnPtrs[i] != NULLPTR)
        {
            /* if this is true, we have a Duplicate Forward Open */
            if( (session->CnxnPtrs[i]->vendor_ID == temp_cnxn->vendor_ID) &&
                (session->CnxnPtrs[i]->connection_serial_number == temp_cnxn->connection_serial_number) &&
                (session->CnxnPtrs[i]->originator_serial_number == temp_cnxn->originator_serial_number))
            {
                /* First check if we are reconfiguring a connection */
                if( ((temp_cnxn->T2O_cnxn_params & NTWKCNXNPRM_CNXNTYP_ANDVAL) == NTWKCNXNPRM_CNXNTYP_NULL) &&
                    ((temp_cnxn->O2T_cnxn_params & NTWKCNXNPRM_CNXNTYP_ANDVAL) == NTWKCNXNPRM_CNXNTYP_NULL))
                {
                    /* Validate the transport trigger parameters match the stored value */
                    if(temp_cnxn->xport_type_and_trigger != session->CnxnPtrs[i]->xport_type_and_trigger)
                    {
                        /* Error: "Invalid Xport Class or Trigger" */
                        cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                        cpf->emrsp->user_struct.ext_stat_size = 1;
                        cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_TRANSPORT;
                        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                        return;
                    }

                    /* validate the path size is 0 */
                    if(temp_cnxn->connection_path_size != 0)
                    {
                        cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                        cpf->emrsp->user_struct.ext_stat_size = 1;
                        cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_SEGMENT;
                        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                        return;
                    }

                    /* *************************************** */
                    /* we received a valid Config Forward Open */
                    /* *************************************** */

                    /* API is the RPI rounded to a supported resolution */
                    temp_o2t_rpi = EIPS_APIROUND(temp_cnxn->O2T_RPI);
                    temp_t2o_rpi = EIPS_APIROUND(temp_cnxn->T2O_RPI);

    	            /* make sure both API are > 0 */
					if( temp_o2t_rpi == 0 ||
						temp_t2o_rpi == 0)
					{
						/* Error: "RPI Not Supported" */
                        cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                        cpf->emrsp->user_struct.ext_stat_size = 1;
                        cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_RPI;
						eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
						return;
					}

                    /* fill in the timer information and start the timer */
                    temp32 = EIPS_APITICKS(temp_o2t_rpi);

                    /* if the connection timeout multiplier is too large, return an error */
                    if(eips_cnxn_getCnxnTmoMult (temp_cnxn->connection_timeout_multiplier) == 0)
                    {
                        /* Error: "RPI, Connection Timeout Multiplier Problem" */
                        cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                        cpf->emrsp->user_struct.ext_stat_size = 1;
                        cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_RPI;
                        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                        return;
                    }

                    /* Use the connection timeout multiplier */
                    temp32 = (temp32 * eips_cnxn_getCnxnTmoMult(temp_cnxn->connection_timeout_multiplier));

                    /* API is the RPI rounded to a supported resolution */
                    /* we need to wait at least 10 seconds the first time */
                    if((temp32 != 0) && (temp32 < EIPS_USER_TICKS_PER_10SEC))
                        current_temp = EIPS_USER_TICKS_PER_10SEC;
                    else
                        current_temp = temp32;

                    /* store the new timer values */
                    eips_timer_changeValues (session->CnxnPtrs[i]->O2T_TimerNum, current_temp, temp32);

                    /* start the timer */
                    eips_timer_start(session->CnxnPtrs[i]->O2T_TimerNum);

                    /* store the new parameters */
                    session->CnxnPtrs[i]->priority_and_tick = temp_cnxn->priority_and_tick;
                    session->CnxnPtrs[i]->connection_time_out_ticks = temp_cnxn->connection_time_out_ticks;
                    temp_cnxn->O2T_CID = eips_cnxn_getConnectionID(); /* get the new Connection ID */
                    session->CnxnPtrs[i]->O2T_CID = temp_cnxn->O2T_CID;
                    session->CnxnPtrs[i]->T2O_CID = temp_cnxn->T2O_CID;
                    session->CnxnPtrs[i]->connection_timeout_multiplier = temp_cnxn->connection_timeout_multiplier;
                    session->CnxnPtrs[i]->O2T_RPI = temp_cnxn->O2T_RPI;
                    session->CnxnPtrs[i]->O2T_API = EIPS_APIROUND(temp_cnxn->O2T_API);
                    session->CnxnPtrs[i]->T2O_RPI = temp_cnxn->T2O_RPI;
                    session->CnxnPtrs[i]->T2O_API = EIPS_APIROUND(temp_cnxn->T2O_API);
                    session->CnxnPtrs[i]->xport_type_and_trigger = temp_cnxn->xport_type_and_trigger;

                    /* stop the inactivity timer for this socket since we use the class 3 connection */
                    eips_rtasys_set_inactivity_tmo(session->sock_id, 0);

                    /* build the success message */
                    eips_cnxn_validFwdOpen(temp_cnxn,cpf);
                    return;
                }

                else
                {
                    /* Error: "Duplicate Forward Open / Connection in Use" */
                    cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                    cpf->emrsp->user_struct.ext_stat_size = 1;
                    cpf->emrsp->user_struct.ext_stat[0] = CMERR_ALREADY_USED;
                    eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                    return;
                }
            }
        } /* END -> "see if we have connections open yet" */
    }

    /* ******************************** */
    /* WE HAVE A NEW CONNECTION REQUEST */
    /* ******************************** */
    /* if this is a new I/O connection, call the I/O open validation */
    if( (temp_cnxn->xport_type_and_trigger == EIPS_XPORTCLASS_CYC_NOSEQ) ||
        (temp_cnxn->xport_type_and_trigger == EIPS_XPORTCLASS_CYC_SEQ)   ||
        (temp_cnxn->xport_type_and_trigger == EIPS_XPORTCLASS_COS_NOSEQ) ||
        (temp_cnxn->xport_type_and_trigger == EIPS_XPORTCLASS_COS_SEQ)   ||
        (temp_cnxn->xport_type_and_trigger == EIPS_XPORTCLASS_APP_NOSEQ) ||
        (temp_cnxn->xport_type_and_trigger == EIPS_XPORTCLASS_APP_SEQ))
    {
        /* the I/O connection is validated and opened in a separate file */
        eips_iomsg_open(cpf,temp_cnxn);
        return;
    }

    /* Validate the transport trigger parameters are an Explicit Connection */
    if((temp_cnxn->xport_type_and_trigger & EIPS_XPORTCLASS_EM_ANDVAL) != EIPS_XPORTCLASS_EM_VALID)
    {
        /* Error: "Invalid Xport Class or Trigger" */
        cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
        cpf->emrsp->user_struct.ext_stat_size = 1;
        cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_TRANSPORT;
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }

    /* Validate the network connection parameters */
    temp16 = (uint16)(temp_cnxn->T2O_cnxn_params & NTWKCNXNPRM_CNXNTYP_ANDVAL);
    if(temp16 != NTWKCNXNPRM_CNXNTYP_P2P)
    {
        /* Error: "Invalid Connection Type" */
        cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
        cpf->emrsp->user_struct.ext_stat_size = 1;
        cpf->emrsp->user_struct.ext_stat[0] = CMERR_T2O_CNXNTYP_NOT_SUPPORTED;
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }
    temp16 = (uint16)(temp_cnxn->O2T_cnxn_params & NTWKCNXNPRM_CNXNTYP_ANDVAL);
    if(temp16 != NTWKCNXNPRM_CNXNTYP_P2P)
    {
        /* Error: "Invalid Connection Type" */
        cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
        cpf->emrsp->user_struct.ext_stat_size = 1;
        cpf->emrsp->user_struct.ext_stat[0] = CMERR_O2T_CNXNTYP_NOT_SUPPORTED;
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }

    /* ***************************************************************
                    PARSE THE PATH...

        For now, the only valid path is:
            Class 0x06, Instance 0x01 (Explicit Message)
    *************************************************************** */
    class_found = FALSE;
    inst_found  = FALSE;
    for(i=0; i<temp_cnxn->connection_path_size; i++)
    {
        path_seg = EIPS_LO(temp_cnxn->connection_path[i]);
        path_val = EIPS_HI(temp_cnxn->connection_path[i]);

        switch (path_seg)
        {
            case 1: break; /* PLC5E Needs this for PCCC */

            /* Store the Class */
            case LOGSEG_8BITCLASS:
                /* The class must be the first path segment */
                if((class_found == FALSE) && (inst_found == FALSE))
                {
                    class_id = path_val;
                    class_found = TRUE;
                }

                else
                {
                    /* Error: "Invalid Segment Type or Value in Path" */
                    cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                    cpf->emrsp->user_struct.ext_stat_size = 1;
                    cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_SEGMENT;
                    eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                    return;
                }
                break;

            /* Store the Inst */
            case LOGSEG_8BITINST:
                /* The class must be the first path segment */
                if((class_found == TRUE) && (inst_found == FALSE))
                {
                    inst_id = path_val;
                    inst_found = TRUE;
                }

                else
                {
                    /* Error: "Invalid Segment Type or Value in Path" */
                    cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                    cpf->emrsp->user_struct.ext_stat_size = 1;
                    cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_SEGMENT;
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
                        cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                        cpf->emrsp->user_struct.ext_stat_size = 1;
                        cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_SEGMENT;
                        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                        return;
                    }

                    vend_id   = temp_cnxn->connection_path[i+1];
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
#if 0
						if((vend_id == 0) || (prod_code == 0) || (major_rev == 0))
	                    {
		                    /* Error: "Path Segment Error" */
                            cpf->emrsp->user_struct.gen_stat = ERR_PATHSEGMENT;
                            eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
		                    return;
	                    } 
#else
                        if(vend_id == 0)
	                    {
							/* Error: "Either the Vendor ID or Product Code Didn't Match" */
							cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
							cpf->emrsp->user_struct.ext_stat_size = 1;
							cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_VENDOR_PRODUCT;
							eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
							return;
	                    }    
                        if(prod_code == 0) 
	                    {
							/* Error: "Either the Vendor ID or Product Code Didn't Match" */
							cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
							cpf->emrsp->user_struct.ext_stat_size = 1;
							cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_VENDOR_PRODUCT;
							eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
							return;
	                    }    
                        if(major_rev == 0)
	                    {
							/* Error: "Bad Revision" */
							cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
							cpf->emrsp->user_struct.ext_stat_size = 1;
							cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_REVISION;
							eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
							return;
	                    }    
                        if((minor_rev == 0) || (minor_rev > EIPS_MINOR_REV))
	                    {
							/* Error: "Bad Revision" */
							cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
							cpf->emrsp->user_struct.ext_stat_size = 1;
							cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_REVISION;
							eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
							return;
	                    }    
#endif
                    }
					
                    /* advanve the path index */
                    i += 4;

                    /* validate the Vendor ID (if non-zero) */
                    if( ((vend_id) && (vend_id != eips_IDObj.Inst.Vendor)) ||
                        ((prod_code) && (prod_code != eips_IDObj.Inst.ProductCode)) )
                    {
                        /* Error: "Either the Vendor ID or Product Code Didn't Match" */
                        cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                        cpf->emrsp->user_struct.ext_stat_size = 1;
                        cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_VENDOR_PRODUCT;
                        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                        return;
                    }

                    /* validate the Device Type (if non-zero) */
                    if((dev_type) && (dev_type != eips_IDObj.Inst.DeviceType))
                    {
                        /* Error: "The Device Type Didn't Match" */
                        cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                        cpf->emrsp->user_struct.ext_stat_size = 1;
                        cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_DEVICE_TYPE;
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
                            cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                            cpf->emrsp->user_struct.ext_stat_size = 1;
                            cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_REVISION;
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
                                cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                                cpf->emrsp->user_struct.ext_stat_size = 1;
                                cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_REVISION;
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
                    cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                    cpf->emrsp->user_struct.ext_stat_size = 1;
                    cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_SEGMENT;
                    eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                    return;
                }
                break;

            /* Error: "Invalid Segment Type or Value in Path" */
            default:
                cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                cpf->emrsp->user_struct.ext_stat_size = 1;
                cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_SEGMENT;
                eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
                return;
        }; /* END-> "switch" */
    } /* END-> "Parse the Path" */

    /* if the class and instance aren't found or are wrong */
    if((class_found == FALSE) || (class_id != CLASS_MSGROUTER) || (inst_found  == FALSE) || (inst_id  != 1))
    {
        /* Error: "Invalid Segment Type or Value in Path" */
        cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
        cpf->emrsp->user_struct.ext_stat_size = 1;
        cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_SEGMENT;
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }

    /* Check if there is a free connection for the session */
    cnxn_ix = EIPS_USER_MAX_NUM_EM_CNXNS_PER_SESSION;
    for(i=0; i<EIPS_USER_MAX_NUM_EM_CNXNS_PER_SESSION; i++)
    {
        /*  there is a free connection pointer, see if
            there are enough connection structures free */
        if(session->CnxnPtrs[i] == NULL)
        {
            /* try to get a new connection */
            cnxn_ix = i;
            session->CnxnPtrs[i] = eips_cnxn_getUnusedCnxn();

            /* we are done, stop looking */
            i = EIPS_USER_MAX_NUM_EM_CNXNS_PER_SESSION;
        }
    }

    /* connection failed */
    if( (cnxn_ix == EIPS_USER_MAX_NUM_EM_CNXNS_PER_SESSION) ||
        (session->CnxnPtrs[cnxn_ix] == NULL))
    {
        /* Error: "No more connections" */
        cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
        cpf->emrsp->user_struct.ext_stat_size = 1;
        cpf->emrsp->user_struct.ext_stat[0] = CMERR_NO_CM_RESOURCES;
        eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
        return;
    }

    /* store the connection data to the connection structure */
	temp_cnxn->O2T_TimerNum = session->CnxnPtrs[cnxn_ix]->O2T_TimerNum;
	temp_cnxn->T2O_TimerNum = session->CnxnPtrs[cnxn_ix]->T2O_TimerNum;
    temp_cnxn->O2T_CID = eips_cnxn_getConnectionID(); /* get the new Connection ID */
    rta_ByteMove((uint8*)session->CnxnPtrs[cnxn_ix],(uint8*)temp_cnxn, sizeof(EIPS_CONNECTION_DATA_STRUCT));

    /* API is the RPI rounded to a supported resolution */
    session->CnxnPtrs[cnxn_ix]->O2T_API = EIPS_APIROUND(session->CnxnPtrs[cnxn_ix]->O2T_RPI);
    session->CnxnPtrs[cnxn_ix]->T2O_API = EIPS_APIROUND(session->CnxnPtrs[cnxn_ix]->T2O_RPI);

    /* make sure both API are > 0 */
	if( session->CnxnPtrs[cnxn_ix]->O2T_API == 0 ||
		session->CnxnPtrs[cnxn_ix]->T2O_API == 0)
	{
		/* free connection structure	 */
		eips_cnxn_free(session->CnxnPtrs[cnxn_ix]);
		session->CnxnPtrs[cnxn_ix] = NULL;

		/* Error: "RPI Not Supported" */
        cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
        cpf->emrsp->user_struct.ext_stat_size = 1;
        cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_RPI;
		eips_cnxn_errorFwdOpen(temp_cnxn,cpf);
		return;
	}

    /* fill in the timer information and start the timer */
    temp32 = EIPS_APITICKS(session->CnxnPtrs[cnxn_ix]->O2T_API);

    /* if the connection timeout multiplier is too large, return an error */
    if(eips_cnxn_getCnxnTmoMult (session->CnxnPtrs[cnxn_ix]->connection_timeout_multiplier) == 0)
    {
		/* free connection structure	 */
		eips_cnxn_free(session->CnxnPtrs[cnxn_ix]);
		session->CnxnPtrs[cnxn_ix] = NULL;

        /* Error: "RPI, Connection Timeout Multiplier Problem" */
        cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
        cpf->emrsp->user_struct.ext_stat_size = 1;
        cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_RPI;
        eips_cnxn_errorFwdOpen(session->CnxnPtrs[cnxn_ix],cpf);
        return;
    }

    /* Use the connection timeout multiplier */
    temp32 = (temp32 * eips_cnxn_getCnxnTmoMult(session->CnxnPtrs[cnxn_ix]->connection_timeout_multiplier));

    /* we need to wait at least 10 seconds the first time */
    if((temp32 != 0) && (temp32 < EIPS_USER_TICKS_PER_10SEC))
        current_temp = EIPS_USER_TICKS_PER_10SEC;
    else
        current_temp = temp32;

    /* store the rest of the timer information */
    eips_timer_changeValues (session->CnxnPtrs[cnxn_ix]->O2T_TimerNum, current_temp, temp32);
    eips_timer_start(session->CnxnPtrs[cnxn_ix]->O2T_TimerNum);

    /* stop the T->O timer (we only send if we receive a message) */
    eips_timer_stop(session->CnxnPtrs[cnxn_ix]->T2O_TimerNum);

    /* stop the inactivity timer for this socket since we use the class 3 connection */
    eips_rtasys_set_inactivity_tmo(session->sock_id, 0);

    /* build the success message */
    eips_cnxn_validFwdOpen(session->CnxnPtrs[cnxn_ix],cpf);
}

/* ====================================================================
Function:   eips_cnxn_validFwdOpen
Parameters: forward open message format
            common packet format message pointer
Returns:    N/A

This function builds the Forward_Open success message as follows:

********  ********************  ************************************
DataType  Parameter Name        Description
********  ********************  ************************************
(UINT32)  O_to_T Connection ID  ECHO BACK FROM REQUEST
(UINT32)  T_to_O Connection ID  ECHO BACK FROM REQUEST
(UINT16)  Connection Serial #   ECHO BACK FROM REQUEST
(UINT16)  Originator Vendor ID  ECHO BACK FROM REQUEST
(UINT32)  Originator Serial #   ECHO BACK FROM REQUEST
(UINT32)  O_to_T API            Actual Packet Rate (O to T)
(UINT32)  T_to_O API            Actual Packet Rate (T to O)
(UINT8)   Appl Reply Size       The # of 16 bit words in Appl Reply
(UINT8)   Reserved              Reserved
(UINT16)  Application Reply     Application Specific Data
======================================================================= */
void eips_cnxn_validFwdOpen (EIPS_CONNECTION_DATA_STRUCT *temp_cnxn, CPF_MESSAGE_STRUCT *cpf)
{
    uint16 i;

    /* Build a success rsp_data */
    i = 0;

    /* O_to_T Connection ID */
    rta_PutLitEndian32(temp_cnxn->O2T_CID, (cpf->emrsp->user_struct.rsp_data+i));
    i+=4;

    /* T_to_O Connection ID */
    rta_PutLitEndian32(temp_cnxn->T2O_CID, (cpf->emrsp->user_struct.rsp_data+i));
    i+=4;

    /* Connection Serial # */
    rta_PutLitEndian16(temp_cnxn->connection_serial_number, (cpf->emrsp->user_struct.rsp_data+i));
    i+=2;

    /* Originator Vendor ID */
    rta_PutLitEndian16(temp_cnxn->vendor_ID, (cpf->emrsp->user_struct.rsp_data+i));
    i+=2;

    /* Originator Serial # */
    rta_PutLitEndian32(temp_cnxn->originator_serial_number, (cpf->emrsp->user_struct.rsp_data+i));
    i+=4;

    /* O_to_T API */
    rta_PutLitEndian32(temp_cnxn->O2T_API, (cpf->emrsp->user_struct.rsp_data+i));
    i+=4;

    /* T_to_O API */
    rta_PutLitEndian32(temp_cnxn->T2O_API, (cpf->emrsp->user_struct.rsp_data+i));
    i+=4;

    /* Appl Reply Size -> Always 0 */
    cpf->emrsp->user_struct.rsp_data[i++] = 0;

    /* Reserved -> Always 0 */
    cpf->emrsp->user_struct.rsp_data[i++] = 0;

    /* store the rsp_data size */
    cpf->emrsp->user_struct.rsp_data_size = i;

    /* eips_user_dbprint0("Valid Forward Open\r\n"); */
}

/* ====================================================================
Function:   eips_cnxn_errorFwdOpen
Parameters: forward open message format
            common packet format message pointer
Returns:    N/A

This function builds the Forward_Open error message as follows:

********  ********************  ***********************************
DataType  Parameter Name        Description
********  ********************  ***********************************
(UINT16)  Connection Serial #   ECHO BACK FROM REQUEST
(UINT16)  Originator Vendor ID  ECHO BACK FROM REQUEST
(UINT32)  Originator Serial #   ECHO BACK FROM REQUEST
(UINT8)   Remainig Path Size    Only with route errors - # words
                                in original route path (Cnxn Path)
(UINT8)   Reserved              Shall be set to zero
======================================================================= */
void eips_cnxn_errorFwdOpen (EIPS_CONNECTION_DATA_STRUCT *temp_cnxn, CPF_MESSAGE_STRUCT *cpf)
{
    uint16 i;

    /* Build an error rsp_data */
    i = 0;

    /* Connection Serial # */
    rta_PutLitEndian16(temp_cnxn->connection_serial_number, (cpf->emrsp->user_struct.rsp_data+i));
    i+=2;

    /* Originator Vendor ID */
    rta_PutLitEndian16(temp_cnxn->vendor_ID, (cpf->emrsp->user_struct.rsp_data+i));
    i+=2;

    /* Originator Serial # */
    rta_PutLitEndian32(temp_cnxn->originator_serial_number, (cpf->emrsp->user_struct.rsp_data+i));
    i+=4;

    /* Remaining Path -> Always 0 (for now) */
    cpf->emrsp->user_struct.rsp_data[i++] = 0;

    /* Reserved -> Always 0 */
    cpf->emrsp->user_struct.rsp_data[i++] = 0;

    /* store the rsp_data size */
    cpf->emrsp->user_struct.rsp_data_size = i;

    /*
eips_user_dbprint2("Error Forward Open (gen 0x%02x) (ext 0x%04x)\r\n",cpf->emrsp->user_struct.gen_stat,cpf->emrsp->user_struct.ext_stat[0]);
    */
}

/* ====================================================================
Function:   eips_cnxn_fwdClose
Parameters: N/A
Returns:    N/A

This function processes the Forward_Close service code.

********************************************
* The Request Message format is as follows *
********************************************
********  ********************  ************************************
DataType  Parameter Name        Description
********  ********************  ************************************
(UINT8)   Priority/Time_tick    Used to calc req timeout info
(UINT8)   Time-out_ticks        Used to calc req timeout info
(UINT16)  Connection Serial #   SEE REQ / RSP PARAMS BELOW
(UINT16)  Originator Vendor ID  Vendor ID of the originating node
(UINT32)  Originator Serial #   Serial # of the originating node
(UINT8)   Cnxn Path Size        The # of 16 bit words in Cnxn Path
(UINT8)   Reserved              Reserved
(VARIES)  Cxnx Path             Route to Remote Target Device
======================================================================= */
void eips_cnxn_fwdClose (CPF_MESSAGE_STRUCT *cpf)
{
    EIPS_CONNECTION_DATA_STRUCT temp_cnxn;
    uint8 class_id, inst_id;
    uint8 class_found, inst_found;
    uint8 path_seg, path_val;
    int16 i, table_ix, buf_ix;

    /* initialize variables */
    class_id = 0;
    inst_id = 0;
    class_found = FALSE;
    inst_found  = FALSE;
    eips_cnxn_structInit(&temp_cnxn);

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

    /* find if the Class and Instance are supported */
    class_found = FALSE;
    inst_found  = FALSE;
    table_ix = -1;
    for(i=0; ((i<EIPS_FWDCLOSETAB_SIZE) && (inst_found == FALSE)); i++)
    {
        /* We found the Class */
        if(EIPSFwdCloseTable[i].ClassID == class_id)
        {
            class_found = TRUE;

            /* We also found the Instance */
            if(EIPSFwdCloseTable[i].InstID == inst_id)
            {
                inst_found = TRUE;
                table_ix = i;
            }
        }
    }

    /* We don't have a match in the lookup table */
    if((class_found == FALSE) || (inst_found == FALSE))
    {
        /* Error: Path Destination Unknown */
        cpf->emrsp->user_struct.gen_stat = ERR_PATHDESTUNKNOWN;
        cpf->emrsp->user_struct.ext_stat_size = 0;
        return;
    }

    /* The service code isn't supported */
    if(EIPSFwdCloseTable[table_ix].Suported == FALSE)
    {
        /* Error: Service Not Supported */
        cpf->emrsp->user_struct.gen_stat = ERR_SERV_UNSUPP;
        cpf->emrsp->user_struct.ext_stat_size = 0;
        return;
    }

    /* ****************************************** */
    /* We have a valid match, process the message */
    /* ****************************************** */
    buf_ix = 0;

    /* Store the Priority/Tick_time */
    temp_cnxn.priority_and_tick = cpf->emreq->user_struct.req_data[buf_ix++];

    /* Store the Time-out ticks */
    temp_cnxn.connection_time_out_ticks = cpf->emreq->user_struct.req_data[buf_ix++];

    /* Store the Connection Serial # */
    temp_cnxn.connection_serial_number = rta_GetLitEndian16(cpf->emreq->user_struct.req_data + buf_ix);
    buf_ix += 2; /* advance the pointer */

    /* Store the Originator Vendor ID */
    temp_cnxn.vendor_ID = rta_GetLitEndian16(cpf->emreq->user_struct.req_data + buf_ix);
    buf_ix += 2; /* advance the pointer */

    /* Store the Originator Serial Number */
    temp_cnxn.originator_serial_number = rta_GetLitEndian32(cpf->emreq->user_struct.req_data + buf_ix);
    buf_ix += 4; /* advance the pointer */

    /* Store the Connection Path Size */
    temp_cnxn.connection_path_size = cpf->emreq->user_struct.req_data[buf_ix++];

    /* The next byte is reserved... ignore it */
    buf_ix += 1;

    /* Store the Connection Path */
    for(i=0; i<temp_cnxn.connection_path_size; i++)
    {
        temp_cnxn.connection_path[i] = rta_GetLitEndian16(cpf->emreq->user_struct.req_data + buf_ix);
        buf_ix += 2; /* advance the pointer */
    }

    /*  all we care about on a Forward_Close is getting the 10 bytes of data
        used to validate the connection is ours. */

    /* Not Enough Data was sent */
    if(cpf->emreq->user_struct.req_data_size < 10)
    {
        /* Error: Not Enough Data */
        cpf->emrsp->user_struct.gen_stat = ERR_INSUFF_DATA;
        cpf->emrsp->user_struct.ext_stat_size = 0;
        return; /* exit on an error */
    }

    /* parse and process the forward close */
    eips_cnxn_parse_fwdclose (&temp_cnxn, cpf);
}

/* ====================================================================
Function:   eips_cnxn_parse_fwdclose
Parameters: N/A
Returns:    N/A

This function processes the Forward_Close service code.  If the passed
parameters match an open connection, a successful rsp_data is build.
Otherwise, an error message is built.
======================================================================= */
void eips_cnxn_parse_fwdclose (EIPS_CONNECTION_DATA_STRUCT *temp_cnxn, CPF_MESSAGE_STRUCT *cpf)
{
    uint16 i, j;
    EIPS_SESSION_STRUCT *session;
    uint16 cnxn_used = 0;

    /* close the I/O connection resources if needed */
    if(eips_iomsg_close (temp_cnxn) == TRUE)
    {
        /* build the success message */
        eips_cnxn_validFwdClose(temp_cnxn,cpf);
        return;
    }

    /* find which connection is trying to be closed */
    for(i=0; i<EIPS_CNXN_MAXTOTALCNXNS; i++)
    {
        /*  the connection serial number, vendor ID and originator serial number
            must match to close a connection */
        if( (eips_CnxnStruct[i].connection_serial_number == temp_cnxn->connection_serial_number) &&
            (eips_CnxnStruct[i].vendor_ID == temp_cnxn->vendor_ID) &&
            (eips_CnxnStruct[i].originator_serial_number == temp_cnxn->originator_serial_number))
        {
            /* ******************** */
            /* close the connection */
            /* ******************** */

            /* make sure the session exists */
            session = eips_encap_getSessionStruct (eips_CnxnStruct[i].SessionID);
            if(session == NULL)
            {
                /* Error: "Bad Connection" */
                cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
                cpf->emrsp->user_struct.ext_stat_size = 1;
                cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_CONNECTION;
                eips_cnxn_errorFwdClose(temp_cnxn,cpf);
                return;
            }

            /* find the EM connection reference */
            for(j=0; j<EIPS_USER_MAX_NUM_EM_CNXNS_PER_SESSION; j++)
            {
                /* we have a valid connection */
                if(session->CnxnPtrs[j] != NULLPTR)
                {
                    /* we found the connection that was closed */
                    if(session->CnxnPtrs[j]->O2T_TimerNum == eips_CnxnStruct[i].O2T_TimerNum)
                    {
                        session->CnxnPtrs[j] = NULL;
                    }
                    else
                        cnxn_used++;
                }
            }

            /* if no other connections on this socket, reset the inactivity timeout */
            if(!cnxn_used)
                eips_rtasys_set_inactivity_tmo(session->sock_id, EIPS_USER_TCP_INACTIVITY_TMO);

            /* if we didn't find the connection with a session, still free it */
            eips_cnxn_free (&eips_CnxnStruct[i]);

            /* build the success message */
            eips_cnxn_validFwdClose(temp_cnxn,cpf);
            return;
        }
    }

    /* Error: "Bad Connection" */
    cpf->emrsp->user_struct.gen_stat = ERR_CNXN_FAILURE;
    cpf->emrsp->user_struct.ext_stat_size = 1;
    cpf->emrsp->user_struct.ext_stat[0] = CMERR_BAD_CONNECTION;
    eips_cnxn_errorFwdClose(temp_cnxn,cpf);
}

/* ====================================================================
Function:   eips_cnxn_validFwdClose
Parameters: forward close message format
            common packet format message pointer
Returns:    N/A

This function builds the Forward_Close success message as follows:

********  ********************  ************************************
DataType  Parameter Name        Description
********  ********************  ************************************
(UINT16)  Connection Serial #   ECHO BACK FROM REQUEST
(UINT16)  Originator Vendor ID  ECHO BACK FROM REQUEST
(UINT32)  Originator Serial #   ECHO BACK FROM REQUEST
(UINT8)   Appl Reply Size       The # of 16 bit words in Appl Reply
(UINT8)   Reserved              Reserved
(UINT16)  Application Reply     Application Specific Data
======================================================================= */
void eips_cnxn_validFwdClose (EIPS_CONNECTION_DATA_STRUCT *temp_cnxn, CPF_MESSAGE_STRUCT *cpf)
{
    uint16 i;

    /* Build a success rsp_data */
    i = 0;

    /* Connection Serial # */
    rta_PutLitEndian16(temp_cnxn->connection_serial_number, (cpf->emrsp->user_struct.rsp_data+i));
    i+=2;

    /* Originator Vendor ID */
    rta_PutLitEndian16(temp_cnxn->vendor_ID, (cpf->emrsp->user_struct.rsp_data+i));
    i+=2;

    /* Originator Serial # */
    rta_PutLitEndian32(temp_cnxn->originator_serial_number, (cpf->emrsp->user_struct.rsp_data+i));
    i+=4;

    /* Appl Reply Size -> Always 0 */
    cpf->emrsp->user_struct.rsp_data[i++] = 0;

    /* Reserved -> Always 0 */
    cpf->emrsp->user_struct.rsp_data[i++] = 0;

    /* store the rsp_data size */
    cpf->emrsp->user_struct.rsp_data_size = i;
}

/* ====================================================================
Function:   eips_cnxn_errorFwdClose
Parameters: forward close message format
            common packet format message pointer
Returns:    N/A

This function builds the Forward_Close error message as follows:

********  ********************  ***********************************
DataType  Parameter Name        Description
********  ********************  ***********************************
(UINT16)  Connection Serial #   ECHO BACK FROM REQUEST
(UINT16)  Originator Vendor ID  ECHO BACK FROM REQUEST
(UINT32)  Originator Serial #   ECHO BACK FROM REQUEST
(UINT8)   Remainig Path Size    Only with route errors - # words
                                in original route path (Cnxn Path)
(UINT8)   Reserved              Shall be set to zero
======================================================================= */
void eips_cnxn_errorFwdClose (EIPS_CONNECTION_DATA_STRUCT *temp_cnxn, CPF_MESSAGE_STRUCT *cpf)
{
    uint16 i;

    /* Build an error rsp_data */
    i = 0;

    /* Connection Serial # */
    rta_PutLitEndian16(temp_cnxn->connection_serial_number, (cpf->emrsp->user_struct.rsp_data+i));
    i+=2;

    /* Originator Vendor ID */
    rta_PutLitEndian16(temp_cnxn->vendor_ID, (cpf->emrsp->user_struct.rsp_data+i));
    i+=2;

    /* Originator Serial # */
    rta_PutLitEndian32(temp_cnxn->originator_serial_number, (cpf->emrsp->user_struct.rsp_data+i));
    i+=4;

    /* Remaining Path -> Always 0 (for now) */
    cpf->emrsp->user_struct.rsp_data[i++] = 0;

    /* Reserved -> Always 0 */
    cpf->emrsp->user_struct.rsp_data[i++] = 0;

    /* store the rsp_data size */
    cpf->emrsp->user_struct.rsp_data_size = i;
}

/* *********** */
/* END OF FILE */
/* *********** */
