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
 *     Module Name: eips_encap.c
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains the definitions needed for parsing and processing
 * EtherNet/IP Server Encapsulation Messages.
 *
 */

/* ---------------------------- */
/* INCLUDE FILES                */
/* ---------------------------- */
#include "eips_system.h"

/* used for debug */
#undef  __RTA_FILE__
#define __RTA_FILE__ "eips_encap.c"

/* ---------------------------- */
/* LOCAL FUNCTION PROTOTYPES    */
/* ---------------------------- */

/* supported Encapsulated messages */
static void local_Nop               (EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock);
static void local_RegisterSession   (EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock);
static void local_UnregisterSession (EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock);
static void local_ListServices      (EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock);
static void local_ListIdentity      (EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock);
static void local_ListInterfaces    (EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock);
static void local_SendRRData        (EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock);
static void local_SendUnitData      (EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock);

/* needed for debug tools only */
//#define EIPS_ENCAPCMD_LISTTARGETS 0x0001
#ifdef EIPS_ENCAPCMD_LISTTARGETS
    static void local_ListTargets   (EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock);
#endif

/* used to validate and issue/free session ids */
static void  local_SessionInit  (EIPS_SESSION_STRUCT *session);
static int16 local_SessionFind  (EIPS_USERSYS_SOCKTYPE sock_id);
static int16 local_GetSessionIx (EIPS_USERSYS_SOCKTYPE sock_id);

/* ---------------------------- */
/*      EXTERN VARIABLES        */
/* ---------------------------- */
extern  EIPS_IDENTITY_OBJ_STRUCT    eips_IDObj;
#if EIPS_USEROBJ_ASM_MAXNUM_O2TINST > 0  
    extern  EIPS_ASSEMBLY_INST_STRUCT   eips_AsmO2TObj[EIPS_USEROBJ_ASM_MAXNUM_O2TINST];
#endif

#if EIPS_USEROBJ_ASM_MAXNUM_T2OINST > 0  
    extern  EIPS_ASSEMBLY_INST_STRUCT   eips_AsmT2OObj[EIPS_USEROBJ_ASM_MAXNUM_T2OINST];
#endif

/* ---------------------------- */
/* MISCELLANEOUS                */
/* ---------------------------- */
EIPS_SESSION_STRUCT EIP_Sessions[EIPS_USER_MAX_NUM_EIP_SESSIONS];
uint32 EIPSessionID = 0x00000001L;

/**/
/* ******************************************************************** */
/*                      GLOBAL FUNCTIONS                                */
/* ******************************************************************** */

/* ====================================================================
Function:   eips_encap_init
Parameters: init type
Returns:    N/A

This function initializes all static variables used in this file.
======================================================================= */
void eips_encap_init(uint8 init_type)
{
    uint16 i;

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

    /* initialize session structures */
    for(i=0; i<EIPS_USER_MAX_NUM_EIP_SESSIONS; i++)
        local_SessionInit(&EIP_Sessions[i]);
}

/* ====================================================================
Function:   eips_encap_getSessionStruct
Parameters: session ID
Returns:    session structure pointer or NULL

This function looks up the passed session ID and returns the session
structure that matches the session ID, NULL on error.
======================================================================= */
EIPS_SESSION_STRUCT * eips_encap_getSessionStruct (uint32 sessionID)
{
    uint16 i;

    /* if the session ID is 0, there is an error */
    if(sessionID == 0)
        return(NULL);

    /* look to match the session ID */
    for(i=0; i<EIPS_USER_MAX_NUM_EIP_SESSIONS; i++)
    {
        /* we found the session, return the structure pointer */
        if(EIP_Sessions[i].SessionHandle == sessionID)
        {
            return(&EIP_Sessions[i]);
        }
    }

    /* Error, return NULL */
    return(NULL);
}

/* ====================================================================
Function:   eips_encap_freeSessionIx
Parameters: socket id
Returns:    N/A

This function compares the passed socket id to see if a session exists.
If a session exists, re-init the socket id.
======================================================================= */
void eips_encap_freeSessionIx (EIPS_USERSYS_SOCKTYPE sock_id)
{
    int16 session_ix;
    uint16 i;

    session_ix = local_SessionFind (sock_id);

    /* the session is valid */
    if(session_ix != -1)
    {
        /* close all used connections */
        for(i=0; i<EIPS_USER_MAX_NUM_EM_CNXNS_PER_SESSION; i++)
        {
            /* if the pointer is NULL, nothing happens. */
            eips_cnxn_free (EIP_Sessions[session_ix].CnxnPtrs[i]);
        }

        /* free the session */
        local_SessionInit(&EIP_Sessions[session_ix]);
    }
}

/* ====================================================================
Function:   eips_encap_process
Parameters: socket structure pointer
            message pointer
            message size
Returns:    bytes used from message pointer

This function processes the encapsulated message.  If more than one
message is available (even partial messages), this function tries
to process the partial message before returning.

We support the following encapsulated messages:
All fields but the encapsulated data content are validated prior to
sending the message to the response building function. The following
is the order of the validation:

    Prior to the command process
    1. Length vs. Encapsulated Data received
    2. Status must equal 0
    3. Options must equal 0
    4. Sender Context - don't care, add to reply
    5. Command

    Within the command process:
    1. Session Handle (if applicable)
    2. Pass the message to the CommandFunction

------ ----------------  ----------------------------------------------
 CODE  NAME              NOTES
------ ----------------  ----------------------------------------------
0x0000 NOP               (May be sent only using TCP)
0x0001 ListTargets       (This is for debug only, remove for production)
0x0004 ListServices      (May be sent using either UDP or TCP)
0x0063 ListIdentity      (May be sent using either UDP or TCP)
0x0065 RegisterSession   (May be sent only using TCP)
0x0066 UnRegisterSession (May be sent only using TCP)
0x006F SendRRData        (May be sent only using TCP)
0x0070 SendUnitData      (May be sent only using TCP)
------ ----------------  ----------------------------------------------

An Encapsulation Message has the following format:
  ----------------------------------------------------------------------
 |  FIELD NAME       |        TYPE       |         DESCRIPTION          |
 |----------------------------------------------------------------------|
 | Command           | UINT16            | Encapsulated Command         |
 |----------------------------------------------------------------------|
 | Length            | UINT16            | Len (in bytes) of encap data |
 |----------------------------------------------------------------------|
 | Session Handle    | UINT32            | Session ID (Appl. Dependent) |
 |----------------------------------------------------------------------|
 | Status            | UINT32            | Status Code                  |
 |----------------------------------------------------------------------|
 | Sender Context    | ARRAY[8] OF UINT8 | Info for sender              |
 |----------------------------------------------------------------------|
 | Options           | UINT32            | Options Flags                |
 |----------------------------------------------------------------------|
 | Encapsulated Data | ARRAY OF UINT8    | The encap. data portion      |
  ----------------------------------------------------------------------
======================================================================= */
uint16 eips_encap_process (EIPS_SOCKET_STRUCT *sock, uint8 *msg, uint16 msg_siz)
{
    EIPS_ENCAP_MSG_STRUCT encap;
    int i;
    uint8 done_flag = FALSE;
    uint16 return_val = msg_siz;
    uint16 encap_length; /* length is sometimes modified along the way */

    /* *************************************************** */
    /* don't leave this function if more data is ready !!! */
    /* *************************************************** */
    while (done_flag == FALSE)
    {
        /* make sure we have enough bytes for the header */
        if(msg_siz < EIP_ENCAP_HEADER_SIZ)
        {
            /* set the done flag */
            done_flag = TRUE;
        }

        /* we have a valid header, continue */
        else
        {
            /* Parse the Encapsulated message */
            encap.Command       = rta_GetLitEndian16 (msg+0);
            encap.Length        = rta_GetLitEndian16 (msg+2);
            encap.SessionHandle = rta_GetLitEndian32 (msg+4);
            encap.Status        = rta_GetLitEndian32 (msg+8);
            for(i=0; i<8; i++)
                encap.SenderContext[i] = *(msg+12+i);
            encap.Options = rta_GetLitEndian32(msg+20);
            encap.EncapsulatedData = (msg + EIP_ENCAP_HEADER_SIZ);

            /* *************************** */
            /* Valid lengths are 0 - 65511 */
            /* *************************** */
            if(encap.Length > 65511) /* ignore the message if too much data */
            {
                /* store more data after the header */
                return_val = EIP_ENCAP_HEADER_SIZ;

                /* decrement the length based on the number of data bytes */
                encap.Length = (uint16)(encap.Length - (msg_siz-EIP_ENCAP_HEADER_SIZ));

                /* store the new length of the NOP */
                rta_PutLitEndian16(encap.Length, msg+2);

                /* return the header size */
                return(return_val);
            }

            /* Message Size should be EIP_ENCAP_HEADER_SIZ + encap.Length */
            if (((encap.Length + EIP_ENCAP_HEADER_SIZ) > msg_siz) &&
                (sock->sock_type == SOCKTYPE_TCP))
            {
                /* set the done flag */
                done_flag = TRUE;

                /*  We need special code to receive NOP messages. The size of the NOP
                    data can be 0 to 65511.  This is far more data than any other
                    message we receive.  When a NOP message is processed, we receive
                    and discard all data until we get the end of the message, then we
                    return.  */
                if(encap.Command == EIP_ENCAPCMD_NOP)
                {
                    /* store more data after the header */
                    return_val = EIP_ENCAP_HEADER_SIZ;

                    /* decrement the length based on the number of data bytes */
                    encap.Length = (uint16)(encap.Length - (msg_siz-EIP_ENCAP_HEADER_SIZ));

                    /* store the new length of the NOP */
                    rta_PutLitEndian16(encap.Length, msg+2);

                    /* return the header size */
                    return(return_val);
                }
            }

            /* UDP length must be exact */
            else if ((sock->sock_type == SOCKTYPE_UDP) &&
                    ((encap.Length + EIP_ENCAP_HEADER_SIZ) != msg_siz))
            {
                /* Always return 0 for UDP messages */
                return(0);
            }

            /* We have a valid message */
            else
            {
                /* save the length of the request for purge purposes */
                encap_length = encap.Length;

                /* *************************************************************** */
                /* *************************************************************** */
                /* Switch on the supported commands if status and options are zero */
                /* *************************************************************** */
                /* *************************************************************** */
                if(encap.Status == 0)
                {
                    /* Just return the Sender Context - Don't Care about content */
                    switch (encap.Command)
                    {
                        /* ********************************** */
                        /* Encapsulated Command 0x0000: "NOP" */
                        /* ********************************** */
                        case EIP_ENCAPCMD_NOP:
                            /* if this is a UDP message send an error message */
                            if(sock->sock_type == SOCKTYPE_UDP)
                            {
                                encap.Status = EIP_ENCAPSTATUSERR_COMMAND;
                                eips_encap_unSupEncapMsg(&encap, sock);
                            }

                            /* valid for TCP connections only */
                            else
                                local_Nop(&encap,sock);
                            break;

                        /* **************************************************** */
                        /* Encapsulated Command 0x0001: "ListTargets" (DB ONLY) */
                        /* **************************************************** */
#ifdef EIPS_ENCAPCMD_LISTTARGETS
                        case EIPS_ENCAPCMD_LISTTARGETS:
                            local_ListTargets(&encap,sock);
                            break;
#endif

                        /* ******************************************* */
                        /* Encapsulated Command 0x0004: "ListServices" */
                        /* ******************************************* */
                        case EIP_ENCAPCMD_LISTSERVICES:
                            local_ListServices(&encap,sock);
                            break;

                        /* ******************************************* */
                        /* Encapsulated Command 0x0063: "ListIdentity" */
                        /* ******************************************* */
                        case EIP_ENCAPCMD_LISTIDENTITY:
                            local_ListIdentity(&encap,sock);
                            break;

                        /* ********************************************* */
                        /* Encapsulated Command 0x0064: "ListInterfaces" */
                        /* ********************************************* */
                        case EIP_ENCAPCMD_LISTINTERFACES:
                            local_ListInterfaces(&encap, sock);
                            break;

                        /* ********************************************** */
                        /* Encapsulated Command 0x0065: "RegisterSession" */
                        /* ********************************************** */
                        case EIP_ENCAPCMD_REGISTERSESSION:
                            /* if this is a UDP message send an error message */
                            if(sock->sock_type == SOCKTYPE_UDP)
                            {
                                encap.Status = EIP_ENCAPSTATUSERR_COMMAND;
                                eips_encap_unSupEncapMsg(&encap, sock);
                            }

                            /* valid for TCP connections only */
                            else
                                local_RegisterSession(&encap,sock);
                            break;

                        /* ************************************************ */
                        /* Encapsulated Command 0x0066: "UnRegisterSession" */
                        /* ************************************************ */
                        case EIP_ENCAPCMD_UNREGISTERSESSION:
                            /* if this is a UDP message send an error message */
                            if(sock->sock_type == SOCKTYPE_UDP)
                            {
                                encap.Status = EIP_ENCAPSTATUSERR_COMMAND;
                                eips_encap_unSupEncapMsg(&encap, sock);
                            }

                            /* valid for TCP connections only */
                            else
                                local_UnregisterSession(&encap,sock);
                            break;

                        /* ***************************************** */
                        /* Encapsulated Command 0x006F: "SendRRData" */
                        /* ***************************************** */
                        case EIP_ENCAPCMD_SENDRRDATA:
                            /* if this is a UDP message send an error message */
                            if(sock->sock_type == SOCKTYPE_UDP)
                            {
                                encap.Status = EIP_ENCAPSTATUSERR_COMMAND;
                                eips_encap_unSupEncapMsg(&encap, sock);
                            }

                            /* valid for TCP connections only */
                            else
                                local_SendRRData(&encap,sock);
                            break;

                        /* ******************************************* */
                        /* Encapsulated Command 0x0070: "SendUnitData" */
                        /* ******************************************* */
                        case EIP_ENCAPCMD_SENDUNITDATA:
                            /* if this is a UDP message send an error message */
                            if(sock->sock_type == SOCKTYPE_UDP)
                            {
                                encap.Status = EIP_ENCAPSTATUSERR_COMMAND;
                                eips_encap_unSupEncapMsg(&encap, sock);
                            }

                            /* valid for TCP connections only */
                            else
                                local_SendUnitData(&encap,sock);
                            break;

                        /* ***************************************** */
                        /* Unsupported Command - Send Error Response */       
                        /* ***************************************** */
                        default:
                            encap.Status = EIP_ENCAPSTATUSERR_COMMAND;
                            eips_encap_unSupEncapMsg(&encap, sock);
            	            break;
                    } /* END-> "switch (encap.Command) */
                } /* END-> "if(encap.Status == 0) */

                /*  return_val is the number of bytes left in the buffer once the
                    message is processed */
                return_val = (uint16)(msg_siz - (encap_length + EIP_ENCAP_HEADER_SIZ));
                msg_siz = return_val;
            
                /* remove the message */
                rta_ByteMove (msg, msg+(encap_length + EIP_ENCAP_HEADER_SIZ), return_val);

                /* we don't have any more messages to process */
                if(return_val == 0)
                    done_flag = TRUE;
            } /* END-> "We have a valid message" */
        } /* END-> "Message Size should be EIP_ENCAP_HEADER_SIZ + encap.Length" */
    } /* END-> "while (done_flag == FALSE)" */

    /* UDP messages must be received in one message */
    if(sock->sock_type == SOCKTYPE_UDP)
        return(0);

    /* TCP messages return the offset to store the next block of data */
    return(return_val);

} /* END-> "EIPProcess()" */

/* ====================================================================
Function:   eips_encap_unSupEncapMsg
Parameters: Encapsulated message structure
            socket structure pointer
Returns:    N/A

This function builds an error message to transmit over the encapsulated
layer.
======================================================================= */
void eips_encap_unSupEncapMsg(EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock)
{
    uint8 resp_msg[EIP_ENCAP_HEADER_SIZ], i;
    int16 resp_msg_siz;

    /* Error response size is fixed */
    EncapReq->Length = 0;
    resp_msg_siz = EIP_ENCAP_HEADER_SIZ;

    /* build the response message */
    rta_PutLitEndian16(EncapReq->Command,       resp_msg+0);
    rta_PutLitEndian16(EncapReq->Length,        resp_msg+2);
    rta_PutLitEndian32(EncapReq->SessionHandle, resp_msg+4);
    rta_PutLitEndian32(EncapReq->Status,        resp_msg+8);
    for(i=0; i<8; i++)
        resp_msg[12+i] = EncapReq->SenderContext[i];
    rta_PutLitEndian32(EncapReq->Options,       resp_msg+20);

    /* Send TCP or UDP response */
    if(sock->sock_type == SOCKTYPE_TCP)
        eips_usersock_sendTCPData (sock->sock_id, resp_msg, resp_msg_siz);
    else
        eips_usersock_sendUDPData (sock->sock_addr, sock->sock_port, resp_msg, resp_msg_siz);
}

/* ====================================================================
Function:   eips_encap_SendListIdentityRequest
Parameters: Directed Broadcast (if non-zero, else global broadcast)
Returns:    N/A

This function transmits the List Identity Request.

The ListIdentity command code request (no data).
======================================================================= */
#ifdef EIPC_CLIENT_USED
void eips_encap_SendListIdentityRequest(uint8 directed_broadcast)
{
    uint8 req_msg[EIP_ENCAP_HEADER_SIZ];
	uint32 addr;
	EIPS_IPADDR_STRUCT ip;

	/*
        ==================
        REQUEST VALIDATION
        ==================

        FIELD NAME         FIElD VALUE
        ----------         -----------
        Command            ListIdentity (0x63)
        Length             shall be 0
        Session Handle     Don't Care
        Status             shall be 0
        Sender Context     Don't Care
        Options            shall be 0
        Encapsulated Data  No Data
    */

    /* build the request message */
	memset(req_msg, 0, sizeof(req_msg));
    rta_PutLitEndian16(EIP_ENCAPCMD_LISTIDENTITY, req_msg);

	/* decide the address to use */
	if(directed_broadcast)
	{
		/* we need the subnet mask and IP address */
		eips_usersock_getTCPObj_Attr05_IntfCfg (&ip);

		addr = ((ip.IPAddress & ip.NtwkMask) | (~ip.NtwkMask));
	}
	else
		addr = 0xFFFFFFFF;

    eips_usersock_sendUDPBroadcast (addr, EIP_CIP_PORT, req_msg, EIP_ENCAP_HEADER_SIZ);
}
#endif

/* ====================================================================
Function:   eips_encap_broadcast_process
Parameters: IP address of device responding to our broadcast
			message data pointer
			message length
Returns:    N/A

This function processes the List Identity Response and calls a user
function if the response is valid.
======================================================================= */
#ifdef EIPC_CLIENT_USED
void eips_encap_broadcast_process (uint32 ipaddr, uint8 * msg, uint16 buffer_size)
{
	EIPC_LIST_IDENTITY_STRUCT listid;
	EIPS_ENCAP_MSG_STRUCT encap;
	uint8 *ptr;
	uint16 i;

	RTA_UNUSED_PARAM(ipaddr);

	/* make sure we have enough bytes for the header */
    if(buffer_size < EIP_ENCAP_HEADER_SIZ)
		return;

    /* Parse the Encapsulated message */
    encap.Command       = rta_GetLitEndian16 (msg+0);
    encap.Length        = rta_GetLitEndian16 (msg+2);
    encap.SessionHandle = rta_GetLitEndian32 (msg+4);
    encap.Status        = rta_GetLitEndian32 (msg+8);
    for(i=0; i<8; i++)
        encap.SenderContext[i] = *(msg+12+i);
    encap.Options = rta_GetLitEndian32(msg+20);
    encap.EncapsulatedData = (msg + EIP_ENCAP_HEADER_SIZ);

	/* validate the header */
	switch(encap.Command)
	{
		/* we only support the list identity */
		case EIP_ENCAPCMD_LISTIDENTITY:
			/* validate length */
			if(buffer_size != (encap.Length + EIP_ENCAP_HEADER_SIZ))
				return;

			/* validate item count is 1 */
			ptr = encap.EncapsulatedData; /* use a temp pointer for parsing */
			if(rta_GetLitEndian16(ptr) != 1)
				return;
			ptr+=2;

			/* validate type id is List Identity response (0x000c) */
			if(rta_GetLitEndian16(ptr) != 0x000C)
				return;
			ptr+=2;

			/* validate length is 6 less than the encap len (item cnt, type and len) */
			if(rta_GetLitEndian16(ptr) != (uint16)(encap.Length-6))
				return;
			ptr+=2;

			/* store the encapsulation version */
			listid.EncapVersion = rta_GetLitEndian16(ptr); ptr+=2;

			/* store the socket address */
			listid.SinFamily	= rta_GetBigEndian16(ptr); ptr+=2;
			listid.SinPort		= rta_GetBigEndian16(ptr); ptr+=2;
			listid.SinAddr		= rta_GetBigEndian32(ptr); ptr+=4;
			rta_ByteMove(listid.SinZero8, ptr, 8); ptr+=8;

			/* store the Identity object */
			listid.Vendor		= rta_GetLitEndian16(ptr); ptr+=2;
			listid.DeviceType	= rta_GetLitEndian16(ptr); ptr+=2;
			listid.ProductCode	= rta_GetLitEndian16(ptr); ptr+=2;
			listid.Revision		= rta_GetLitEndian16(ptr); ptr+=2;
			listid.Status		= rta_GetLitEndian16(ptr); ptr+=2;
			listid.SerialNum	= rta_GetLitEndian32(ptr); ptr+=4;
			listid.SizOfName	= *ptr++;
			rta_ByteMove(listid.ProductName, ptr, RTA_MIN(listid.SizOfName, sizeof(listid.ProductName))); ptr+=listid.SizOfName; /* advance by their number, even if we trunc string */
			listid.State		= *ptr++;

			/* inform the user */
			eipc_usersys_onListIdentityResponse(&listid);
			break;
	};
}
#endif

/**/
/* ******************************************************************** */
/*                      LOCAL FUNCTIONS                                 */
/* ******************************************************************** */
/* ====================================================================
Function:   local_SessionInit
Parameters: session structure pointer
Returns:    N/A

This function closes the passed socket and sets the socket structure
element back to the default state.
======================================================================= */
static void local_SessionInit(EIPS_SESSION_STRUCT *session)
{
    uint16 i;

    session->sock_id = (EIPS_USERSYS_SOCKTYPE)EIPS_USER_SOCKET_UNUSED;
    session->SessionHandle = 0;

    /* Initialize the connection data */
    for(i=0; i<EIPS_USER_MAX_NUM_EM_CNXNS_PER_SESSION; i++)
    {
        session->CnxnPtrs[i] = NULL;
    }
}

/* ====================================================================
Function:   local_SessionFind
Parameters: socket id
Returns:    session index

This function compares the passed socket id to see if a session exists.
======================================================================= */
static int16 local_SessionFind (EIPS_USERSYS_SOCKTYPE sock_id)
{
    int16 i;

    for(i=0; i<EIPS_USER_MAX_NUM_EIP_SESSIONS; i++)
        if(EIP_Sessions[i].sock_id == sock_id)
            return(i);
    return(-1);
}

/* ====================================================================
Function:   local_GetSessionIx
Parameters: socket id
Returns:    session index

This function compares the passed socket id to see if a session exists.
If a session exists, return -1, else return the next session index and
store a session number.
======================================================================= */
static int16 local_GetSessionIx (EIPS_USERSYS_SOCKTYPE sock_id)
{
    int16 i;

    /* the sock_id is already used */
    if(local_SessionFind(sock_id) != -1)
        return(-1);

    /* find the next free session */
    for(i=0; i<EIPS_USER_MAX_NUM_EIP_SESSIONS; i++)
    {
        if(EIP_Sessions[i].sock_id == EIPS_USER_SOCKET_UNUSED)
        {
            EIP_Sessions[i].SessionHandle = EIPSessionID;
            EIPSessionID++;
            EIP_Sessions[i].sock_id = sock_id;
            return(i);
        }
    }
    return(-1);
}

/* ====================================================================
Function:   local_Nop
Parameters: Encapsulated message structure
            socket structure pointer
Returns:    N/A

This function processes the Nop command code.

The Nop command code response: No Response
======================================================================= */
static void local_Nop(EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock)
{
    /*
        ==================
        REQUEST VALIDATION
        ==================

        FIELD NAME         FIElD VALUE
        ----------         -----------
        Command            NOP (0x00)
        Length             0 to 65511
        Session Handle     Don't Care
        Status             shall be 0
        Sender Context     Ignored (since no reply)
        Options            shall be 0
        Encapsulated Data  No Data
    */

    /* use EncapReq to keep compiler happy */
    if(EncapReq) {}

    /* use sock to keep compiler happy */
    if(sock) {}
}

/* ====================================================================
Function:   local_ListServices
Parameters: Encapsulated message structure
            socket structure pointer
Returns:    N/A

This function processes the ListServices command code.

The ListServices command code response:
  ----------------------------------------------------------------------
 |  FIELD NAME       |        TYPE          |      DESCRIPTION          |
 |----------------------------------------------------------------------|
 | Item Count        | UINT16               | Always 0x0001             |
 |----------------------------------------------------------------------|
 | Item Code         | UINT16               | Always 0x0100             |
 |----------------------------------------------------------------------|
 | Item Length       | UINT16               | Always 0x0014             |
 |----------------------------------------------------------------------|
 | Item Data         | ARRAY[20] OF UINT8   |                           |
 | ==================  =====================  ========================= |
 | Protocol Version  | ItemData[0] -  [1]   | Always 0x0001             |
 | Capability Flags  | ItemData[2] -  [3]   | Always 0x0120             |
 | Name              | ItemData[4] - [19]   | Name String (NULL TERM)   |
  ----------------------------------------------------------------------
======================================================================= */
static void local_ListServices(EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock)
{
    uint8 resp_msg[(EIP_ENCAP_HEADER_SIZ+26)];
    int16 resp_msg_siz, i;

    /*
        ==================
        REQUEST VALIDATION
        ==================

        FIELD NAME         FIElD VALUE
        ----------         -----------
        Command            ListServices (0x0004)
        Length             shall be 0
        Session Handle     Don't Care
        Status             shall be 0
        Sender Context     Don't Care
        Options            shall be 0
        Encapsulated Data  No Data
    */

    /* Session Handle Don't Care */

    /* Valid Length = 0 */
    if (EncapReq->Length != 0)
    {
        EncapReq->Status = EIP_ENCAPSTATUSERR_LENGTH;
        eips_encap_unSupEncapMsg(EncapReq, sock);
        return; /* Stop this function on an error */
    }

    /* Response size is fixed */
    EncapReq->Length = 26;
    resp_msg_siz = EIP_ENCAP_HEADER_SIZ; /* incremented as response is built */

    /* build the response message */
    rta_PutLitEndian16(EncapReq->Command,       resp_msg+0);
    rta_PutLitEndian16(EncapReq->Length,        resp_msg+2);
    rta_PutLitEndian32(EncapReq->SessionHandle, resp_msg+4);
    rta_PutLitEndian32(EncapReq->Status,        resp_msg+8);
    for(i=0; i<8; i++)
        resp_msg[12+i] = EncapReq->SenderContext[i];
    rta_PutLitEndian32(EncapReq->Options,       resp_msg+20);

    /* ****************************** */
    /* Build the rest of the response */
    /* ****************************** */

    /* CPF Data - Item Count (always 0x0001) */
    rta_PutLitEndian16(0x0001, resp_msg + resp_msg_siz);
    resp_msg_siz += 2; /* advance the count by the 2 bytes added */

    /* CPF Data - Item Type Code (always 0x0100) */
    rta_PutLitEndian16(0x0100, resp_msg + resp_msg_siz);
    resp_msg_siz += 2; /* advance the count by the 2 bytes added */

    /* CPF Data - Item Length (always 0x0014) */
    rta_PutLitEndian16(0x0014, resp_msg + resp_msg_siz);
    resp_msg_siz += 2; /* advance the count by the 2 bytes added */

    /* CPF Data - Item Data - Protocol Version (always 0x0001) */
    rta_PutLitEndian16(0x0001, resp_msg + resp_msg_siz);
    resp_msg_siz += 2; /* advance the count by the 2 bytes added */

    /* CPF Data - Item Data - Capablities Flags */
    rta_PutLitEndian16(EIP_LIST_SERVICES_CAP_FLAGS, resp_msg + resp_msg_siz);
    resp_msg_siz += 2; /* advance the count by the 2 bytes added */

    /* CPF Data - Item Data - Socket Address - Name (always "Communications " */
    resp_msg[resp_msg_siz++] = 'C';
    resp_msg[resp_msg_siz++] = 'o';
    resp_msg[resp_msg_siz++] = 'm';
    resp_msg[resp_msg_siz++] = 'm';
    resp_msg[resp_msg_siz++] = 'u';
    resp_msg[resp_msg_siz++] = 'n';
    resp_msg[resp_msg_siz++] = 'i';
    resp_msg[resp_msg_siz++] = 'c';
    resp_msg[resp_msg_siz++] = 'a';
    resp_msg[resp_msg_siz++] = 't';
    resp_msg[resp_msg_siz++] = 'i';
    resp_msg[resp_msg_siz++] = 'o';
    resp_msg[resp_msg_siz++] = 'n';
    resp_msg[resp_msg_siz++] = 's';
    resp_msg[resp_msg_siz++] = ' ';
    resp_msg[resp_msg_siz++] = 0x00;

    /* Send TCP or UDP response */
    if(sock->sock_type == SOCKTYPE_TCP)
        eips_usersock_sendTCPData (sock->sock_id, resp_msg, resp_msg_siz);
    else
        eips_usersock_sendUDPData (sock->sock_addr, sock->sock_port, resp_msg, resp_msg_siz);
}

/* ====================================================================
Function:   local_ListIdentity
Parameters: Encapsulated message structure
            socket structure pointer
Returns:    N/A

This function processes the ListServices command code.

The ListIdentity command code response:
  ----------------------------------------------------------------------
 |  FIELD NAME       |        TYPE          |      DESCRIPTION          |
 |----------------------------------------------------------------------|
 | Item Count        | UINT16               | Always 0x0001             |
 |----------------------------------------------------------------------|
 | Item Code         | UINT16               | Always 0x000C             |
 |----------------------------------------------------------------------|
 | Item Length       | UINT16               | Varies                    |
 |----------------------------------------------------------------------|
 | Item Data         | Varies               | Varies                    |
  ----------------------------------------------------------------------
======================================================================= */
static void local_ListIdentity(EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock)
{
    uint8 resp_msg[(EIP_ENCAP_HEADER_SIZ + 40 + EIPS_STRUCT_MAX_PROD_NAME_SIZE)];
    uint16 resp_msg_siz, i;

    /*
        ==================
        REQUEST VALIDATION
        ==================

        FIELD NAME         FIElD VALUE
        ----------         -----------
        Command            ListIdentity (0x63)
        Length             shall be 0
        Session Handle     Don't Care
        Status             shall be 0
        Sender Context     Don't Care
        Options            shall be 0
        Encapsulated Data  No Data
    */

    /* Session Handle Don't Care */

    /* Valid Length = 0 */
    if (EncapReq->Length != 0)
    {
        EncapReq->Status = EIP_ENCAPSTATUSERR_LENGTH;
        eips_encap_unSupEncapMsg(EncapReq, sock);
        return; /* Stop this function on an error */
    }

    /* Response size is fixed */
    EncapReq->Length = (uint16)(40 + RTA_MIN(EIPS_USER_IDOBJ_NAME_SIZE, EIPS_STRUCT_MAX_PROD_NAME_SIZE));
    resp_msg_siz = EIP_ENCAP_HEADER_SIZ; /* incremented as response is built */

    /* build the response message */
    rta_PutLitEndian16(EncapReq->Command,       resp_msg+0);
    rta_PutLitEndian16(EncapReq->Length,        resp_msg+2);
    rta_PutLitEndian32(EncapReq->SessionHandle, resp_msg+4);
    rta_PutLitEndian32(EncapReq->Status,        resp_msg+8);
    for(i=0; i<8; i++)
        resp_msg[12+i] = EncapReq->SenderContext[i];
    rta_PutLitEndian32(EncapReq->Options,       resp_msg+20);

    /* ****************************** */
    /* Build the rest of the response */
    /* ****************************** */

    /* CPF Data - Item Count (always 0x0001) */
    rta_PutLitEndian16(0x0001, resp_msg + resp_msg_siz);
    resp_msg_siz += 2; /* advance the count by the 2 bytes added */

    /* CPF Data - Item Type Code (always 0x000C) */
    rta_PutLitEndian16(0x000C, resp_msg + resp_msg_siz);
    resp_msg_siz += 2; /* advance the count by the 2 bytes added */

    /* CPF Data - Item Length */
    rta_PutLitEndian16((uint16)(34 + RTA_MIN(EIPS_USER_IDOBJ_NAME_SIZE, EIPS_STRUCT_MAX_PROD_NAME_SIZE)), resp_msg + resp_msg_siz);
    resp_msg_siz += 2; /* advance the count by the 2 bytes added */

    /* CPF Data - Item Data - Protocol Version */
    rta_PutLitEndian16(0x0001, resp_msg + resp_msg_siz);
    resp_msg_siz += 2; /* advance the count by the 2 bytes added */

    /* CPF Data - Item Data - Socket Address - sin_family (big-endian) (always 0x0002)*/
    resp_msg[resp_msg_siz++] = 0x00;
    resp_msg[resp_msg_siz++] = 0x02;   

    /* CPF Data - Item Data - Socket Address - sin_port (big-endian) (always 0xAF12) */
    resp_msg[resp_msg_siz++] = 0xaf;
    resp_msg[resp_msg_siz++] = 0x12;      

    /* CPF Data - Item Data - Socket Address - sin_addr (big-endian) (our IP address) */
    rta_PutBigEndian32(eips_usersock_getOurIPAddr (), resp_msg + resp_msg_siz);   
    resp_msg_siz += 4; /* advance the count by the 4 bytes added */

    /* CPF Data - Item Data - Socket Address - sin_zero (len 8) (big-endian) */
    for(i=0; i<8; i++)
        resp_msg[resp_msg_siz++] = 0;

    /* CPF Data - Item Data - Vendor ID */
    rta_PutLitEndian16(eips_IDObj.Inst.Vendor, resp_msg + resp_msg_siz);
    resp_msg_siz += 2; /* advance the count by the 2 bytes added */

    /* CPF Data - Item Data - Device Type */
    rta_PutLitEndian16(eips_IDObj.Inst.DeviceType, resp_msg + resp_msg_siz);
    resp_msg_siz += 2; /* advance the count by the 2 bytes added */

    /* CPF Data - Item Data - Product Code */
    rta_PutLitEndian16(eips_IDObj.Inst.ProductCode, resp_msg + resp_msg_siz);
    resp_msg_siz += 2; /* advance the count by the 2 bytes added */

    /* CPF Data - Item Data - Revision */
    resp_msg[resp_msg_siz++] = EIPS_MAJOR_REV;
    resp_msg[resp_msg_siz++] = EIPS_MINOR_REV;

    /* CPF Data - Item Data - Status */
    rta_PutLitEndian16(eips_cpf_getIDObjStatus(), resp_msg + resp_msg_siz);
    resp_msg_siz += 2; /* advance the count by the 2 bytes added */

    /* CPF Data - Item Data - Serial Number */
    rta_PutLitEndian32(eips_IDObj.Inst.SerialNum, resp_msg + resp_msg_siz);
    resp_msg_siz += 4; /* advance the count by the 4 bytes added */

    /* CPF Data - Item Data - Product Name Size */
    resp_msg[resp_msg_siz++] = (uint8)(RTA_MIN(EIPS_USER_IDOBJ_NAME_SIZE, EIPS_STRUCT_MAX_PROD_NAME_SIZE));
    rta_ByteMove((resp_msg+resp_msg_siz), (uint8*)EIPS_USER_IDOBJ_NAME, (uint16)(RTA_MIN(EIPS_USER_IDOBJ_NAME_SIZE, EIPS_STRUCT_MAX_PROD_NAME_SIZE)));
    resp_msg_siz += (uint16)(RTA_MIN(EIPS_USER_IDOBJ_NAME_SIZE, EIPS_STRUCT_MAX_PROD_NAME_SIZE));

    /* CPF Data - Item Data - State */
#ifdef EIPS_USEROBJ_ID_OBJ_EXTENDED
    resp_msg[resp_msg_siz++] = eips_userobj_getIDObj_Attr08_State();
#else
    resp_msg[resp_msg_siz++] = eips_IDObj.Inst.State;
#endif

    /* Send TCP or UDP response */
    if(sock->sock_type == SOCKTYPE_TCP)
        eips_usersock_sendTCPData (sock->sock_id, resp_msg, resp_msg_siz);
    else
        eips_usersock_sendUDPData (sock->sock_addr, sock->sock_port, resp_msg, resp_msg_siz);
}

/* ====================================================================
Function:   local_ListInterfaces
Parameters: Encapsulated message structure
            socket structure pointer
Returns:    N/A

This function processes the ListInterfaces command code.

The ListIdentity command code response:
  ----------------------------------------------------------------------
 |  FIELD NAME       |        TYPE          |      DESCRIPTION          |
 |----------------------------------------------------------------------|
 | Item Count        | UINT16               | Always 0x0000             |
  ----------------------------------------------------------------------
======================================================================= */
static void local_ListInterfaces (EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock)
{
    uint8 resp_msg[(EIP_ENCAP_HEADER_SIZ + 2)];
    int16 resp_msg_siz, i;

    /*
        ==================
        REQUEST VALIDATION
        ==================

        FIELD NAME         FIElD VALUE
        ----------         -----------
        Command            ListInterfaces (0x64)
        Length             shall be 0
        Session Handle     Don't Care
        Status             shall be 0
        Sender Context     Don't Care
        Options            shall be 0
        Encapsulated Data  No Data
    */

    /* Session Handle Don't Care */

    /* Valid Length = 0 */
    if (EncapReq->Length != 0)
    {
        EncapReq->Status = EIP_ENCAPSTATUSERR_LENGTH;
        eips_encap_unSupEncapMsg(EncapReq, sock);
        return; /* Stop this function on an error */
    }

    /* Response size is fixed */
    EncapReq->Length = 2;
    resp_msg_siz = EIP_ENCAP_HEADER_SIZ; /* incremented as response is built */

    /* build the response message */
    rta_PutLitEndian16(EncapReq->Command,       resp_msg+0);
    rta_PutLitEndian16(EncapReq->Length,        resp_msg+2);
    rta_PutLitEndian32(EncapReq->SessionHandle, resp_msg+4);
    rta_PutLitEndian32(EncapReq->Status,        resp_msg+8);
    for(i=0; i<8; i++)
        resp_msg[12+i] = EncapReq->SenderContext[i];
    rta_PutLitEndian32(EncapReq->Options,       resp_msg+20);

    /* ****************************** */
    /* Build the rest of the response */
    /* ****************************** */

    /* CPF Data - Item Count (always 0x0000) */
    rta_PutLitEndian16(0x0000, resp_msg + resp_msg_siz);
    resp_msg_siz += 2; /* advance the count by the 2 bytes added */

    /* Send TCP or UDP response */
    if(sock->sock_type == SOCKTYPE_TCP)
        eips_usersock_sendTCPData (sock->sock_id, resp_msg, resp_msg_siz);
    else
        eips_usersock_sendUDPData (sock->sock_addr, sock->sock_port, resp_msg, resp_msg_siz);
}

/* ====================================================================
Function:   local_RegisterSession
Parameters: Encapsulated message structure
            socket structure pointer
Returns:    N/A

This function processes the RegisterSession command code.

The RegisterSession command code response: See Below
======================================================================= */
static void local_RegisterSession(EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock)
{
    uint8 resp_msg[EIP_ENCAP_HEADER_SIZ+4], i;
    int16 resp_msg_siz;
    int16 session_ix;
    uint16 prot_ver, opt_flags;

    /*
        ==================
        REQUEST VALIDATION
        ==================

        FIELD NAME          FIElD VALUE
        ----------          -----------
        Command             RegisterSession (0x65)
        Length              4 Bytes of Data
        Session Handle      shall be 0
        Status              shall be 0
        Sender Context      Don't Care
        Options             shall be 0
        Encapsulated Data   Protocol Version (2 bytes)
                            Options flags (2 bytes)
    */

    /*
        ===============
        RESPONSE FORMAT
        ===============

        FIELD NAME          FIElD VALUE
        ----------          -----------
        Command             RegisterSession (0x65)
        Length              4 Bytes of Data
        Session Handle      SET BY US
        Status              shall be 0
        Sender Context      all 0's
        Options             shall be 0
        Encapsulated Data   0x0100 Protocol Version (2 bytes LSB MSB)
                            0x0000 Options flags (2 bytes LSB MSB)
    */

    /* Session Handle Don't Care */

    /* Valid Length = 4 */
    if (EncapReq->Length != 4)
    {
        EncapReq->Status = EIP_ENCAPSTATUSERR_LENGTH;
        eips_encap_unSupEncapMsg(EncapReq, sock);
        return;
    }

    /* Valid Options MUST be 0, else just return!!! */
    if(EncapReq->Options != 0)
        return;

    /* Validate Protocol Version should be the current version, options should be 0 */
    prot_ver = rta_GetLitEndian16(EncapReq->EncapsulatedData);
    opt_flags = rta_GetLitEndian16(EncapReq->EncapsulatedData+2);
    if((prot_ver != EIP_ENCAP_PROT_VER) || (opt_flags != 0))
    {
#if 1
        /* Error response size is fixed */
        EncapReq->Length = 4;
        resp_msg_siz = (uint16)(EIP_ENCAP_HEADER_SIZ + EncapReq->Length);

        /* build the response message */
        rta_PutLitEndian16(EncapReq->Command,           resp_msg+0);
        rta_PutLitEndian16(EncapReq->Length,            resp_msg+2);
        rta_PutLitEndian32(EncapReq->SessionHandle,     resp_msg+4);
        rta_PutLitEndian32(EIP_ENCAPSTATUSERR_PROTREV,  resp_msg+8);
        for(i=0; i<8; i++)
            resp_msg[12+i] = EncapReq->SenderContext[i];
        rta_PutLitEndian32(EncapReq->Options,           resp_msg+20);
        rta_PutLitEndian16(EIP_ENCAP_PROT_VER,          resp_msg+24);
        rta_PutLitEndian16(0,                           resp_msg+26);
        eips_usersock_sendTCPData (sock->sock_id, resp_msg, resp_msg_siz);
#else
        EncapReq->Status = EIP_ENCAPSTATUSERR_PROTREV;
        eips_encap_unSupEncapMsg(EncapReq, sock);
#endif
        return; /* Stop this function on an error */
    }
    /* Validate we don't already have a Session Handle */
    session_ix = local_GetSessionIx(sock->sock_id);
    if(session_ix == -1)
    {
        EncapReq->Status = EIP_ENCAPSTATUSERR_COMMAND;
        eips_encap_unSupEncapMsg(EncapReq, sock);
        return; /* Stop this function on an error */
    }

    /* Valid Protocol Version */
    EncapReq->SessionHandle = EIP_Sessions[session_ix].SessionHandle;

    /* Error response size is fixed */
    EncapReq->Length = 4;
    resp_msg_siz = (uint16)(EIP_ENCAP_HEADER_SIZ + EncapReq->Length);

    /* build the response message */
    rta_PutLitEndian16(EncapReq->Command,       resp_msg+0);
    rta_PutLitEndian16(EncapReq->Length,        resp_msg+2);
    rta_PutLitEndian32(EncapReq->SessionHandle, resp_msg+4);
    rta_PutLitEndian32(EncapReq->Status,        resp_msg+8);
    for(i=0; i<8; i++)
        resp_msg[12+i] = EncapReq->SenderContext[i];
    rta_PutLitEndian32(EncapReq->Options,       resp_msg+20);
    for(i=0; i<4; i++)
        resp_msg[24+i] = EncapReq->EncapsulatedData[i];

    eips_usersock_sendTCPData (sock->sock_id, resp_msg, resp_msg_siz);
}

/* ====================================================================
Function:   local_UnregisterSession
Parameters: Encapsulated message structure
            socket structure pointer
Returns:    N/A

This function processes the UnRegisterSession command code.

The local_UnregisterSession command code response: No Response
======================================================================= */
static void local_UnregisterSession(EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock)
{
    int16 session_ix;

    /*
        ==================
        REQUEST VALIDATION
        ==================

        FIELD NAME         FIElD VALUE
        ----------         -----------
        Command            UnRegisterSession (0x66)
        Length             shall be 0
        Session Handle     match ours
        Status             shall be 0
        Sender Context     Don't Care
        Options            shall be 0
        Encapsulated Data  No Data
    */

    /* Validate the Session Handle (-1 on Error) */
    session_ix = local_SessionFind(sock->sock_id);
    if(session_ix == -1)
    {
        eips_user_dbprint1("Invalid Session Close %08lx \r\n", EncapReq->SessionHandle);
        return; /* Stop this function on an error */
    }

    /* No Response */
    RTA_TCP_CLOSE_WITH_PRINT(sock->sock_id);

    if(EncapReq){} /* keep compiler happy */
}

/* ====================================================================
Function:   local_SendRRData
Parameters: Encapsulated message structure
            socket structure pointer
Returns:    N/A

This function processes the SendRRData command code.

The SendRRData command code response: See Below
======================================================================= */
static void local_SendRRData(EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock)
{
    int16  session_ix;

    /*
        ==================
        REQUEST VALIDATION
        ==================

        FIELD NAME          FIElD VALUE
        ----------          -----------
        Command             SendRRData (0x6F)
        Length              length of data structure
        Session Handle      match ours
        Status              shall be 0
        Sender Context      Don't Care
        Options             shall be 0
        Encapsulated Data   Interface Handle (shall be 0)
                            Timeout (UINT16) 0 = no timeout, >0 in seconds
                            C.P.F. (Passed to ProcessCipMessage)
    */

    /*
        ===============
        RESPONSE FORMAT
        ===============

        FIELD NAME         FIElD VALUE
        ----------         -----------
        Command            SendRRData (0x6F)
        Length             length of data structure
        Session Handle     match ours
        Status             shall be 0
        Sender Context     Preserved from the corresponding ListService req.
        Options            shall be 0
        Encapsulated Data  Built by ProcessCipMessage
    */

    /* Validate the Session Handle (-1 on Error) */
    session_ix = local_SessionFind(sock->sock_id);
    if(session_ix == -1)
    {
        EncapReq->Status = EIP_ENCAPSTATUSERR_SESSION;
        eips_encap_unSupEncapMsg(EncapReq, sock);
        eips_user_dbprint1("Invalid Session Handle %08lx \r\n", EncapReq->SessionHandle);
        return; /* Stop this function on an error */
    }

    /* Validate the Interface Handle (shall be 0 for CIP) */
    if(rta_GetLitEndian32(EncapReq->EncapsulatedData) != 0)
    {
        EncapReq->Status = EIP_ENCAPSTATUSERR_FORMAT;
        eips_encap_unSupEncapMsg(EncapReq, sock);
        return; /* Stop this function on an error */
    }

    /* Process the Common Packet Format message */
    eips_cpf_ProcessCPF(EncapReq, sock);
}

/* ====================================================================
Function:   local_SendUnitData
Parameters: Encapsulated message structure
            socket structure pointer
Returns:    N/A

This function processes the SendUnitData command code.

The SendUnitData command code response: See Below
======================================================================= */
static void local_SendUnitData(EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock)
{
    int16  session_ix;

    /*
        ==================
        REQUEST VALIDATION
        ==================

        FIELD NAME          FIElD VALUE
        ----------          -----------
        Command             SendUnitData (0x70)
        Length              length of data structure
        Session Handle      match ours
        Status              shall be 0
        Sender Context      Don't Care
        Options             shall be 0
        Encapsulated Data   Interface Handle (shall be 0)
                            Timeout (shall be 0)
                            C.P.F. (Passed to ProcessCipMessage)
    */

    /*
        ===============
        RESPONSE FORMAT
        ===============

        FIELD NAME          FIElD VALUE
        ----------          -----------
        Command             SendUnitData (0x70)
        Length              length of data structure
        Session Handle      match ours
        Status              shall be 0
        Sender Context      Preserved from the corresponding ListService req.
        Options             shall be 0
        Encapsulated Data   Built by ProcessCipMessage
    */

    /* Valid Length >= 6 */
    if (EncapReq->Length < 6)
    {
        EncapReq->Status = EIP_ENCAPSTATUSERR_LENGTH;
        eips_encap_unSupEncapMsg(EncapReq, sock);
        return; /* Stop this function on an error */
    }

    /* Validate the Session Handle (-1 on Error) */
    session_ix = local_SessionFind(sock->sock_id);
    if(session_ix == -1)
    {
        EncapReq->Status = EIP_ENCAPSTATUSERR_SESSION;
        eips_encap_unSupEncapMsg(EncapReq, sock);
        eips_user_dbprint1("Invalid Session Handle %08lx \r\n", EncapReq->SessionHandle);
        return; /* Stop this function on an error */
    }

    /* Validate the Interface Handle (shall be 0 for CIP) */
    if(rta_GetLitEndian32(EncapReq->EncapsulatedData) != 0)
    {
        EncapReq->Status = EIP_ENCAPSTATUSERR_FORMAT;
        eips_encap_unSupEncapMsg(EncapReq, sock);
        eips_user_dbprint0("Invalid Interface Handle\r\n");
        return; /* Stop this function on an error */
    }

    /* Process the Common Packet Format message */
    eips_cpf_ProcessCPF(EncapReq, sock);
}

/* ====================================================================
Function:   local_ListTargets
Parameters: Encapsulated message structure
            socket structure pointer
Returns:    N/A

This function processes the ListTargets command code.  This function is
only for testing purposes.
======================================================================= */
#ifdef EIPS_ENCAPCMD_LISTTARGETS
static void local_ListTargets (EIPS_ENCAP_MSG_STRUCT *EncapReq, EIPS_SOCKET_STRUCT *sock)
{
    uint8 resp_msg[100], i;
    int16 resp_msg_siz;

    /* Response size is fixed */
    EncapReq->Length = 0x2A;
    resp_msg_siz = (uint16)(EIP_ENCAP_HEADER_SIZ + EncapReq->Length);

    /* build the response message */
    rta_PutLitEndian16(EncapReq->Command,       resp_msg+0);
    rta_PutLitEndian16(EncapReq->Length,        resp_msg+2);
    rta_PutLitEndian32(EncapReq->SessionHandle, resp_msg+4);
    rta_PutLitEndian32(EncapReq->Status,        resp_msg+8);
    for(i=0; i<8; i++)
        resp_msg[12+i] = EncapReq->SenderContext[i];
    rta_PutLitEndian32(EncapReq->Options,       resp_msg+20);
    for(i=0; i<4; i++)
        resp_msg[24+i] = EncapReq->EncapsulatedData[i];

    /* Build the rest of the response */
    resp_msg_siz = EIP_ENCAP_HEADER_SIZ;

    /* CPF Data - Item Count (always 1) */
    rta_PutLitEndian16(1, resp_msg+resp_msg_siz);
    resp_msg_siz+=2;

    /* CPF Data - Item Code (always 1) */
    rta_PutLitEndian16(1, resp_msg+resp_msg_siz);
    resp_msg_siz+=2;

    /* CPF Data - Item Length (always 0x24) */
    rta_PutLitEndian16(0x24, resp_msg+resp_msg_siz);
    resp_msg_siz+=2;

    /* CPF Data - Item Data - Protocol Version (always 1) */
    rta_PutLitEndian16(1, resp_msg+resp_msg_siz);
    resp_msg_siz+=2;

    /* CPF Data - Item Data - Capablities Flags (always 0) */
    rta_PutLitEndian16(0, resp_msg+resp_msg_siz);
    resp_msg_siz+=2;

    /* 16 bytes of 0 */
    for(i=0; i<16; i++)
        resp_msg[resp_msg_siz++] = 0;

    /* IP Address "000.000.000.000" */
    resp_msg[resp_msg_siz++] = '1';
    resp_msg[resp_msg_siz++] = '7';
    resp_msg[resp_msg_siz++] = '2';
    resp_msg[resp_msg_siz++] = '.';
    resp_msg[resp_msg_siz++] = '0';
    resp_msg[resp_msg_siz++] = '2';
    resp_msg[resp_msg_siz++] = '4';
    resp_msg[resp_msg_siz++] = '.';
    resp_msg[resp_msg_siz++] = '0';
    resp_msg[resp_msg_siz++] = '0';
    resp_msg[resp_msg_siz++] = '1';
    resp_msg[resp_msg_siz++] = '.';
    resp_msg[resp_msg_siz++] = '0';
    resp_msg[resp_msg_siz++] = '0';
    resp_msg[resp_msg_siz++] = '1';
    resp_msg[resp_msg_siz++] = 0;

    /* send the response */
    eips_usersock_sendTCPData (sock->sock_id, resp_msg, resp_msg_siz);
}
#endif

/* *********** */
/* END OF FILE */
/* *********** */
