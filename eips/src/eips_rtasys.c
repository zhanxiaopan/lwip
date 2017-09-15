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
 *     Module Name: eips_rtasys.c
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains the code needed for state machine processing of the
 * data.
 *
 */

/* ---------------------------- */
/*      INCLUDE FILES           */
/* ---------------------------- */

/* Application definitions */
#include "eips_system.h"

/* used for debug */
#undef  __RTA_FILE__
#define __RTA_FILE__ "eips_rtasys.c"

/* ---------------------------- */
/*      FUNCTION PROTOTYPES     */
/* ---------------------------- */
static uint8 local_getState  (EIPS_USERSYS_SOCKTYPE socket_id);
static void  local_initAll   (uint8 initType);
static void  local_closeAllSockets (void);
static int16 local_findSocketIx (EIPS_USERSYS_SOCKTYPE socket_id);
static void  local_freeSocketIx (EIPS_USERSYS_SOCKTYPE socket_id);
static int16 local_findFreeSocketIx (void);

/* ---------------------------- */
/*      STATIC VARIABLES        */
/* ---------------------------- */
static struct
{
    uint16 inactivity_tmo;
    uint16 inactivity_ticks_elapsed;
    uint8  buf[EIPS_USER_MAX_ENCAP_BUFFER_SIZ];
    uint16 buf_size;
    EIPS_USERSYS_SOCKTYPE socket_id;
    uint8  state;

#ifdef EIPC_CLIENT_USED
    uint8  client_owned;
#endif
}struct_data[EIPS_USER_MAX_NUM_SOCKETS];

static uint8 local_state;

/* ---------------------------- */
/*      MISCELLANEOUS           */
/* ---------------------------- */

/**/
/* ******************************************************************** */
/*		 GLOBAL FUNCTIONS					*/
/* ******************************************************************** */
/* ====================================================================
Function:   eips_rtasys_init
Parameters: N/A
Returns:    N/A

This function initializes all variables for the EtherNet/IP Server stack.
======================================================================= */
void eips_rtasys_init (void)
{
    uint8 initType;

    /* determine the init code */
    initType = EIPSINIT_NORMAL;

    /* call all init code */
    local_initAll (initType);
    local_state = EIPS_STATE_RUNNING;
}

/* ====================================================================
Function:   eips_rtasys_process
Parameters: ticks passed since the last call to this function
Returns:    N/A

This function processes the states of the different sockets.  For now
we only have one state.
======================================================================= */
void eips_rtasys_process (uint32 ticks_passed_since_last_call)
{
    uint16 i;

    switch(local_state)
    {
        /* if the new state is valid, change the state */
        case EIPS_STATE_RESETOUTOFBOX:
#ifdef EIPS_STATE_RESETNORMAL
        case EIPS_STATE_RESETNORMAL:
#endif
#ifdef EIPS_STATE_RESETOUTOFBOX_NOCOMM
        case EIPS_STATE_RESETOUTOFBOX_NOCOMM:
#endif
            break;

        case EIPS_STATE_RUNNING:
            /* process the LED's */
            eips_cnxn_Process();
            eips_iomsg_process();
            break;
    };

    /* process inactivity timeout on sockets */
    if(ticks_passed_since_last_call)
    {
        for(i=0; i<EIPS_USER_MAX_NUM_SOCKETS; i++)
        {
            /* socket in use */
            if( (struct_data[i].socket_id != EIPS_USER_SOCKET_UNUSED) &&
                (struct_data[i].inactivity_tmo > 0))
            {
                /* increment the ticker */
                struct_data[i].inactivity_ticks_elapsed += (uint16)ticks_passed_since_last_call;

                /* check if the socket timed out */
                if(struct_data[i].inactivity_ticks_elapsed > struct_data[i].inactivity_tmo)
                {
                    /* close the socket */
                    eips_usersock_tcpClose(struct_data[i].socket_id);

                    /* reset the inactivity ticker */
                    struct_data[i].inactivity_ticks_elapsed = 0;
                } 
            }
        }
    }

    /* process the timer */
    eips_timer_process (ticks_passed_since_last_call);

    /* process the sockets (use non-blocking calls) */
    eips_usersock_process();
    eips_userobj_process();
    eips_usersys_process();

#ifdef EIPC_CLIENT_USED
    /* process the client code (if supported) */
    eipc_client_process(ticks_passed_since_last_call);
    eipc_usersys_process(ticks_passed_since_last_call);
#endif
}

/* ====================================================================
Function:   eips_rtasys_setState
Parameters: socket_id
            new timeout value
Returns:    N/A

This function is called to modify the inactivity timeout for the TCP
socket.
======================================================================= */
void eips_rtasys_set_inactivity_tmo (EIPS_USERSYS_SOCKTYPE socket_id, uint16 inactivity_tmo)
{
    int16 i;

    /* make sure we have the socket */
    i = local_findSocketIx(socket_id);
    if(i == -1)
        return;

    /* store the inactivity tmo */
    struct_data[i].inactivity_tmo = inactivity_tmo;
}

/* ====================================================================
Function:   eips_rtasys_setState
Parameters: socket_id
            new state
Returns:    N/A

This function is called to change the state of the EIP Server.	The
socket_id that wants the state changed is passed to allow a send to
complete prior to causing a reset.
======================================================================= */
void eips_rtasys_setState (EIPS_USERSYS_SOCKTYPE socket_id, uint8 new_state)
{
    int16 i;

    /* we only use one state for now */
    if( (new_state == EIPS_STATE_RESETNORMAL) ||
#ifdef EIPS_STATE_RESETOUTOFBOX
        (new_state == EIPS_STATE_RESETOUTOFBOX) ||
#endif
#ifdef EIPS_STATE_RESETOUTOFBOX_NOCOMM
        (new_state == EIPS_STATE_RESETOUTOFBOX_NOCOMM) ||
#endif
        (new_state == EIPS_STATE_RUNNING))
    {
        local_state = new_state;
    }

    /* make sure we have the socket */
    i = local_findSocketIx(socket_id);
    if(i == -1)
        return;

    switch (new_state)
    {
        /* if the new state is valid, change the state */
        case EIPS_STATE_RESETNORMAL:
#ifdef EIPS_STATE_RESETOUTOFBOX
        case EIPS_STATE_RESETOUTOFBOX:
#endif
#ifdef EIPS_STATE_RESETOUTOFBOX_NOCOMM
        case EIPS_STATE_RESETOUTOFBOX_NOCOMM:
#endif
        case EIPS_STATE_RUNNING:
	        struct_data[i].state = new_state;
	        break;

        /* if the passed state isn't valid, don't do anything */
        default:
	        break;
    };
}

/* ====================================================================
Function:   eips_rtasys_onTCPAccept
Parameters: socket_id
Returns:    N/A

This function is called when the listening socket accepts a new
connection.  We need to set up a session structure to match the new
socket so we can receive data from the socket.
======================================================================= */
void eips_rtasys_onTCPAccept (EIPS_USERSYS_SOCKTYPE socket_id)
{
    int16 i;

    #if EIPS_PRINTALL > 1
        eips_user_dbprint1("[Socket %d] eips_rtasys_onTCPAccept\r\n", socket_id);
    #endif

    /* see if we can accept the new connection */
    i = local_findFreeSocketIx ();

    /* we can't receive the socket, close it */
    if(i == -1)
    {
        RTA_TCP_CLOSE_WITH_PRINT(socket_id);
    }
    /* set up a structure to store messages from the socket */
    else
    {
        struct_data[i].inactivity_tmo = EIPS_USER_TCP_INACTIVITY_TMO;
        struct_data[i].inactivity_ticks_elapsed = 0;
        struct_data[i].socket_id = socket_id;
        struct_data[i].state = EIPS_STATE_RUNNING;
        struct_data[i].buf_size = 0;
        memset(struct_data[i].buf, 0, sizeof(struct_data[i].buf));
    }
}

/* ====================================================================
Function:   eips_rtasys_onTCPData
Parameters: socket_id
            pointer to the data
            size of the data
Returns:    N/A

This function is called when TCP or UDP I/O data is received.  This
function forwards the data to the Encapsulation layer processing.
======================================================================= */
void eips_rtasys_onTCPData (EIPS_USERSYS_SOCKTYPE socket_id, uint8 * socket_data, uint16 socket_data_size)
{
    EIPS_SOCKET_STRUCT sock;
    int16 i;

    #if EIPS_PRINTALL > 1
        eips_user_dbprint2("[Socket %d] eips_rtasys_onTCPData %d bytes\r\n",socket_id, socket_data_size);
    #endif

    /* see if we have this socket */
    i = local_findSocketIx (socket_id);
    if(i == -1)
    {
        eips_user_dbprint1("ON TCP Data Error [Socket %d]\r\n",socket_id);
        return;
    }

    /* reset the inactivity ticker */
    struct_data[i].inactivity_ticks_elapsed = 0;

    /* close the socket if the data won't fit, there is a serious problem or we are out of sync */    
    if((struct_data[i].buf_size + socket_data_size) > sizeof(struct_data[i].buf))
    {
        eips_user_dbprint1("eips_rtasys_onTCPData [Socket %d] - Data won't fit\r\n",socket_id);
        eips_usersock_tcpClose(socket_id);
        return;        
    }

    /* add the message to the data buffer */
    memcpy(struct_data[i].buf+struct_data[i].buf_size, socket_data, socket_data_size);
    struct_data[i].buf_size = (uint16)(struct_data[i].buf_size + socket_data_size);

#ifdef EIPC_CLIENT_USED
    if(struct_data[i].client_owned == TRUE)
    {
        struct_data[i].buf_size = eipc_client_onTCPData (socket_id, struct_data[i].buf, struct_data[i].buf_size);
    }

    else
    {
        /* process the message */
        sock.sock_id = socket_id;
        sock.sock_type = SOCKTYPE_TCP;
        sock.sock_port = 0;
        sock.sock_addr = 0;
        struct_data[i].buf_size = eips_encap_process (&sock, struct_data[i].buf, struct_data[i].buf_size);
    }
#else
    /* process the message */
    sock.sock_id = socket_id;
    sock.sock_type = SOCKTYPE_TCP;
    sock.sock_port = 0;
    sock.sock_addr = 0;
    struct_data[i].buf_size = eips_encap_process (&sock, struct_data[i].buf, struct_data[i].buf_size);
#endif
}

/* ====================================================================
Function:   eips_rtasys_onUDPData
Parameters: socket_id
            pointer to the data
            size of the data
Returns:    N/A

This function is called when UDP data is received.  This function
forwards the data to the Encapsulation layer processing.
======================================================================= */
void eips_rtasys_onUDPData (uint32 src_addr, uint16 src_port, uint8 * socket_data, uint16 socket_data_size)
{
    EIPS_SOCKET_STRUCT sock;

    #if EIPS_PRINTALL > 1
        eips_user_dbprint1("eips_rtasys_onUDPData %d bytes\r\n",socket_data_size);
    #endif

    /* process the message */
    sock.sock_id     = (EIPS_USERSYS_SOCKTYPE) EIPS_USER_SOCKET_UNUSED;
    sock.sock_type   = SOCKTYPE_UDP;
    sock.sock_port   = src_port;
    sock.sock_addr   = src_addr;
    eips_encap_process (&sock, socket_data, socket_data_size);
}

/* ====================================================================
Function:   eips_rtasys_onUDPBroadcastData
Parameters: ip address
            pointer to the data
            size of the data
Returns:    N/A

This function is called when UDP broadcast response data is received.  
This function forwards the data to the Encapsulation layer processing.
======================================================================= */
#ifdef EIPC_CLIENT_USED
void eips_rtasys_onUDPBroadcastData (uint32 ipaddr, uint8 * buffer, uint16 buffer_size)
{
    eips_encap_broadcast_process (ipaddr, buffer, buffer_size);
}
#endif

/* ====================================================================
Function:   eips_rtasys_onIOData
Parameters: socket_id
            pointer to the data
            size of the data
Returns:    N/A

This function is called when UDP I/O data is received.	This function
forwards the data to the Encapsulation layer processing.
======================================================================= */
void eips_rtasys_onIOData (uint32 src_addr, uint8 * socket_data, uint16 socket_data_size)
{
    #if EIPS_PRINTALL > 1
        eips_user_dbprint1("eips_rtasys_onIOData %d bytes\r\n",socket_data_size);
    #endif

    /* call the Server I/O Processing */
    eips_iomsg_proc_o2t_iomsg(src_addr, socket_data, socket_data_size);

    /* call the Client I/O Processing */
#if defined(EIPC_CLIENT_USED) && defined(EIPC_IO_CLIENT_USED)
    eipc_io_onUDPData (src_addr, socket_data, socket_data_size);
#endif
}

/* ====================================================================
Function:   eips_rtasys_onTCPSent
Parameters: socket_id
Returns:    N/A

This function is called once data is successfully sent.  If the state
for the passed socket isn't running, cause the appropriate reset.
======================================================================= */
void eips_rtasys_onTCPSent (EIPS_USERSYS_SOCKTYPE socket_id)
{
    uint8 state;
    int16 i;

    #if EIPS_PRINTALL > 1
        eips_user_dbprint1("eips_rtasys_onTCPSent [Socket %d]\r\n",socket_id);
    #endif

    /* see if we have this socket */
    i = local_findSocketIx (socket_id);
    if(i == -1)
    {
        eips_user_dbprint1("ON TCP Sent Error [Socket %d]\r\n",socket_id);
        return;
    }

    /* reset the inactivity ticker */
    struct_data[i].inactivity_ticks_elapsed = 0;

    /* get the state for the passed socket_id */
    state = local_getState(socket_id);

    /* if we are running, do nothing */
    if(state == EIPS_STATE_RUNNING)
    {
        return;
    }

    /* we need to reset */
    switch(state)
    {
        case EIPS_STATE_RESETNORMAL:
#ifdef EIPS_STATE_RESETOUTOFBOX
        case EIPS_STATE_RESETOUTOFBOX:
#endif
#ifdef EIPS_STATE_RESETOUTOFBOX_NOCOMM
        case EIPS_STATE_RESETOUTOFBOX_NOCOMM:
#endif
            eips_user_dbprint1("Reset(%d) - All sockets closed\r\n",state);

            /* Close all sockets */
            local_closeAllSockets ();

            /* call reboot */
#ifdef EIPS_USERSYS_HWRESET_SUPPORTED
            eips_usersys_noreturn_reboot();
#endif
            /* re-init */
            switch(state)
            {
                case EIPS_STATE_RESETNORMAL: local_initAll(EIPSINIT_NORMAL); break;           
#ifdef EIPS_STATE_RESETOUTOFBOX
                case EIPS_STATE_RESETOUTOFBOX: local_initAll(EIPSINIT_OUTOFBOX); break;           
#endif
#ifdef EIPS_STATE_RESETOUTOFBOX_NOCOMM
                case EIPS_STATE_RESETOUTOFBOX_NOCOMM: local_initAll(EIPSINIT_OUTOFBOX_NOCOMM); break;           
#endif 
                default: local_initAll(EIPSINIT_NORMAL); break;           
            };

            /* after we call the init routine, set the state to running */
            eips_rtasys_setState(socket_id, EIPS_STATE_RUNNING);
            break;
    };
}

/* ====================================================================
Function:   eips_rtasys_onTCPClose
Parameters: socket_id
Returns:    N/A

This function is called when a socket is closed.  Free all resources
associated with the socket id.
======================================================================= */
void eips_rtasys_onTCPClose (EIPS_USERSYS_SOCKTYPE socket_id)
{
#ifdef EIPC_CLIENT_USED
    int16 i;
#endif

    #if EIPS_PRINTALL > 1
        eips_user_dbprint1("eips_rtasys_onTCPClose [Socket %d]\r\n",socket_id);
    #endif

#ifdef EIPC_CLIENT_USED
    /* make sure we have the socket */
    i = local_findSocketIx(socket_id);
    if(i == -1)
        return;

    /* client connection */
    if(struct_data[i].client_owned == TRUE)
    {
        /* inform the client of the close */
        eipc_client_onClose(socket_id);
    }

    /* server connection */
    else
    {
        /* Free the session info */
        eips_encap_freeSessionIx(socket_id);
    }
#else
    /* Free the session info */
    eips_encap_freeSessionIx(socket_id);
#endif

    /* free the local socket structure */
    local_freeSocketIx (socket_id);
}

/* ====================================================================
Function:   eips_rtasys_onTCPConnect
Parameters: socket_id
Returns:    N/A

This function is called when a server is connected to.
======================================================================= */
#ifdef EIPC_CLIENT_USED
void eips_rtasys_onTCPConnect (EIPS_USERSYS_SOCKTYPE socket_id)
{
    int16 i;

    #if EIPS_PRINTALL > 1
        eips_user_dbprint1("eips_rtasys_onTCPConnect [Socket %d]\r\n",socket_id);
    #endif

    /* see if we can accept the new connection */
    i = local_findFreeSocketIx ();

    /* we can't receive the socket, close it */
    if(i == -1)
    {
        RTA_TCP_CLOSE_WITH_PRINT(socket_id);
    }
    /* set up a structure to store messages from the socket */
    else
    {
        struct_data[i].socket_id = socket_id;
        struct_data[i].state = EIPS_STATE_RUNNING;
        struct_data[i].buf_size = 0;
        struct_data[i].inactivity_tmo = EIPS_USER_TCP_INACTIVITY_TMO;
        struct_data[i].inactivity_ticks_elapsed = 0;
        memset(struct_data[i].buf, 0, sizeof(struct_data[i].buf));
        struct_data[i].client_owned = TRUE;
    }

    /* inform the client of the connect */
    eipc_client_onConnect (socket_id);
}
#endif

/**/
/* ******************************************************************** */
/*		 LOCAL FUNCTIONS					*/
/* ******************************************************************** */
/* ====================================================================
Function:   local_getState
Parameters: socket_id
Returns:    Current State

This function returns the current state of the EIP Server state
machine for the passed socket_id.
======================================================================= */
static uint8 local_getState (EIPS_USERSYS_SOCKTYPE socket_id)
{
    int16 i;

    /* make sure we have the socket */
    i = local_findSocketIx(socket_id);
    if(i == -1)
        return(EIPS_STATE_RUNNING);

    /* we found the socket, return the state */
    return(struct_data[i].state);
}

/* ====================================================================
Function:   local_initAll
Parameters: initType
Returns:    N/A

This function initializes all variables based on the passed initType.
======================================================================= */
static void local_initAll (uint8 initType)
{
    int16 i;

    /* Force to Out-of-Box on errors */
    if(initType != EIPSINIT_NORMAL)
    {
        initType = EIPSINIT_OUTOFBOX;
    }

    /* timer init must be first */
    eips_timer_init    (initType);

    /* user inits prior to rta inits */
    eips_userobj_init  (initType);
    eips_usersock_init (initType);
    eips_usersys_init  (initType);

#if EIPS_NTWK_LED_USED || EIPS_IO_LED_USED
    eips_usersys_ledTest ();
#endif

#if EIPS_NTWK_LED_USED
    eips_usersys_nsLedUpdate (EIPS_LEDSTATE_FLASH_GREEN);
#endif

#if EIPS_IO_LED_USED
    eips_usersys_ioLedUpdate (EIPS_LEDSTATE_FLASH_GREEN);
#endif

    /* rta inits */
    eips_cnxn_init   (initType);
    eips_cpf_init    (initType);
    eips_encap_init  (initType);
    eips_iomsg_init  (initType);

    /* call if TaCL object is supported */
#ifdef EIPS_TACL_OBJ_USED
    eips_TaCL_init (initType);
#endif

    /* call if PCCC messaging is supported */
#ifdef EIPS_PCCC_USED
        eips_pccc_init (initType);
#endif

#ifdef EIPC_CLIENT_USED
    /* call if client code is supported */
    eipc_client_init(initType);
    eipc_usersys_init(initType); /* call user code second */
#endif

    /* initialize local structures */
    for(i=0; i<EIPS_USER_MAX_NUM_SOCKETS; i++)
    {
        struct_data[i].socket_id = (EIPS_USERSYS_SOCKTYPE) EIPS_USER_SOCKET_UNUSED;
        struct_data[i].state = EIPS_STATE_RUNNING;
        struct_data[i].buf_size = 0;
        struct_data[i].inactivity_tmo = 0;
        struct_data[i].inactivity_ticks_elapsed = 0;
        memset(struct_data[i].buf, 0, sizeof(struct_data[i].buf));

#ifdef EIPC_CLIENT_USED
        struct_data[i].client_owned = FALSE;
#endif
    }
}

/* ====================================================================
Function:   local_closeAllSockets
Parameters: N/A
Returns:    N/A

This function closes all open sockets prior to a reset.
======================================================================= */
static void local_closeAllSockets (void)
{
    int16 i;

    /* search through all sockets */
    for(i=0; i<EIPS_USER_MAX_NUM_SOCKETS; i++)
    {
        /* if the socket is used, close it */
        if(struct_data[i].socket_id != EIPS_USER_SOCKET_UNUSED)
            RTA_TCP_CLOSE_WITH_PRINT(struct_data[i].socket_id);

        /* free the resources */
        local_freeSocketIx(struct_data[i].socket_id);
    }
}

/* ====================================================================
Function:   local_findSocketIx
Parameters: Socket ID
Returns:    Index where socket info is stored (-1 on error)

This function finds which index stores the socket information.
======================================================================= */
static int16 local_findSocketIx (EIPS_USERSYS_SOCKTYPE socket_id)
{
    int16 i;

    /* search for a free element */
    for(i=0; i<EIPS_USER_MAX_NUM_SOCKETS; i++)
    {
        /* the socket element is free */
        if(struct_data[i].socket_id == socket_id)
        {
            return(i);
        }
    }

    /* socket index not found */
    return (-1);
}

/* ====================================================================
Function:   local_freeSocketIx
Parameters: Socket ID
Returns:    N/A

This function finds which index stores the passed socket id and frees
the resources.
======================================================================= */
static void local_freeSocketIx (EIPS_USERSYS_SOCKTYPE socket_id)
{
    int16 i;

    for(i=0; i<EIPS_USER_MAX_NUM_SOCKETS; i++)
    {
        /* socket found, free it */
        if(struct_data[i].socket_id == socket_id)
        {
	        struct_data[i].socket_id = (EIPS_USERSYS_SOCKTYPE) EIPS_USER_SOCKET_UNUSED;
	        struct_data[i].state = EIPS_STATE_RUNNING;
	        struct_data[i].buf_size = 0;
            struct_data[i].inactivity_tmo = 0;
            struct_data[i].inactivity_ticks_elapsed = 0;
	        memset(struct_data[i].buf, 0, sizeof(struct_data[i].buf));

#ifdef EIPC_CLIENT_USED
            struct_data[i].client_owned = FALSE;
#endif
        }
    }
}

/* ====================================================================
Function:   local_findFreeSocketIx
Parameters: N/A
Returns:    Index to store the socket info (-1 on error)

This function finds the next free index to store the socket info.
======================================================================= */
static int16 local_findFreeSocketIx (void)
{
    int16 i;

    /* search for a free element */
    for(i=0; i<EIPS_USER_MAX_NUM_SOCKETS; i++)
    {
        /* the socket element is free */
        if(struct_data[i].socket_id == EIPS_USER_SOCKET_UNUSED)
        {
            return(i);
        }
    }

    /* no free socket indexes */
    return (-1);
}

/* *********** */
/* END OF FILE */
/* *********** */
