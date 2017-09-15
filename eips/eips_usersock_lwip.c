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
 *    Version Date: 05JAN2013
 *         Version: 2.31
 *    Conformed To: EtherNet/IP Protocol Conformance Test A-9 (16-NOV-2011)
 *     Module Name: eips_usersock.c
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains the definitions needed to handle the TCP processing.
 *
 */

/* ---------------------------- */
/* INCLUDE FILES                */
/* ---------------------------- */
#include <string.h>
#include "netconf.h"					// LWIP entry point
// EIPS Header Files
#include "eips_system.h"
#include "eips_usersock.h"
#include "eips_usersys.h"
// Keep TCP Socket alive
//extern uint32_t Time_GetMs();

/* used for debug */
#undef  __RTA_FILE__
#define __RTA_FILE__ "eips_usersock.c"

/* ---------------------------- */
/* LOCAL STRUCTURE DEFINITIONS  */
/* ---------------------------- */
/* this must be this size or smaller */
#define USERSOCK_MAX_BUF_SIZ (EIPS_USER_MAX_ENCAP_BUFFER_SIZ-EIP_ENCAP_HEADER_SIZ)
#define USERSOCK_UDP_IX         0
#define USERSOCK_IO_IX          1
#define USERSOCK_TCPSTART_IX    2
#define USERSOCK_SOCKID_UNUSED  -1
#define LOCAL_MAX_NUM_SOCKETS   (EIPS_USER_MAX_NUM_SOCKETS+USERSOCK_TCPSTART_IX)

typedef struct
{
	/* UDP necessary */
	ip_addr_t addr;
	uint16    port;

	struct tcp_pcb *pcb;
	struct udp_pcb *udp_pcb;

	EIPS_USERSYS_SOCKTYPE sockid;

	uint8  send_buf[USERSOCK_MAX_BUF_SIZ];
	uint8  queue_send_buf[USERSOCK_MAX_BUF_SIZ];
	uint16 send_buf_size;	// If non-zero, send!!!
	uint16 queue_send_buf_size;	// If non-zero, send!!!
	uint8  tx_in_prog_flag;	// Transmission ongoing
	uint32 last_tx_time;	// Last transmission time
} USERSOCK_SOCKSTRUCT;

/* ---------------------------- */
/* STATIC VARIABLES             */
/* ---------------------------- */
static USERSOCK_SOCKSTRUCT usersock_sockets [LOCAL_MAX_NUM_SOCKETS];
static struct tcp_pcb *listen_pcb;
//static struct tcp_pcb *listen_pcb_bak[4];

/* ---------------------------- */
/* LOCAL FUNCTIONS              */
/* ---------------------------- */
static void		local_openUdpSocket(USERSOCK_SOCKSTRUCT *user_sock);
static void		local_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port);
static void		local_openUdpIOSocket(USERSOCK_SOCKSTRUCT *user_sock);
static void		local_udpio_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port);

static err_t	local_tcp_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t	local_tcp_receive(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static void		local_tcp_err(void *arg, err_t err);
static err_t	local_tcp_sent(void *arg, struct tcp_pcb *pcb, u16_t len);
static err_t	local_tcp_poll(void *arg, struct tcp_pcb *pcb);
static err_t	local_tcp_write(struct tcp_pcb *pcb, const void* ptr, u16_t *length, u8_t apiflags);


/* ******************************************************************** */
/*			           GLOBAL FUNCTIONS									*/
/* ******************************************************************** */
/* ====================================================================
Function:   eips_usersock_init
Parameters: init type
Returns:    N/A

This function initialize all user socket variables.
======================================================================= */
void eips_usersock_init (uint8 init_type)
{
	int16 i;
	err_t err;

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

	/* initialize all static variables */
	memset(usersock_sockets, 0, sizeof(usersock_sockets));
	for(i=0; i<LOCAL_MAX_NUM_SOCKETS; i++)
	{
		usersock_sockets[i].sockid = i+1;
		usersock_sockets[i].send_buf_size = 0;
		usersock_sockets[i].queue_send_buf_size = 0;
		usersock_sockets[i].tx_in_prog_flag = FALSE;
	}

	/* open a UDP socket for UCMM Messages */
	local_openUdpSocket (&usersock_sockets[USERSOCK_UDP_IX]);

	/* open a UDP I/O socket for I/O Messages (port = EIPS_UDPIO_PORT) */
	local_openUdpIOSocket (&usersock_sockets[USERSOCK_IO_IX]);

	/* start a listening socket (port = EIP_CIP_PORT) */
		listen_pcb = tcp_new();
		if(!listen_pcb) eips_user_printf("EIPS couldn't get a TCP socket\n");
		tcp_setprio(listen_pcb, TCP_PRIO_NORMAL);
		err = tcp_bind(listen_pcb, IP_ADDR_ANY, EIP_CIP_PORT);
		if (err != ERR_OK) eips_user_printf("EIPS couldn't bind to EIP_CIP_PORT\n");
		listen_pcb = tcp_listen(listen_pcb);
		/* initialize callback arg and accept callback */
		tcp_arg(listen_pcb, listen_pcb);
		tcp_accept(listen_pcb, local_tcp_accept);

//	for(i=USERSOCK_TCPSTART_IX+1; i<EIPS_USER_MAX_NUM_SOCKETS; i++) {
//		listen_pcb_bak[i-3] = tcp_new();
//		if(!listen_pcb_bak[i-3]) eips_user_printf("EIPS couldn't get a TCP socket");
//		tcp_setprio(listen_pcb_bak[i-3], TCP_PRIO_NORMAL);
//		err = tcp_bind(listen_pcb_bak[i-3], IP_ADDR_ANY, EIP_CIP_PORT);
//		if (err != ERR_OK) eips_user_printf("EIPS couldn't bind to EIP_CIP_PORT");
//		listen_pcb_bak[i-3] = tcp_listen(listen_pcb_bak[i-3]);
//		/* initialize callback arg and accept callback */
//		tcp_arg(listen_pcb_bak[i-3], listen_pcb_bak[i-3]);
//		tcp_accept(listen_pcb_bak[i-3], local_tcp_accept);
//	}
}
/* ====================================================================
Function:   eips_usersock_process
Parameters: N/A
Returns:    N/A

This function checks for socket activity.  This includes new connect
requests, close requests and data received.
======================================================================= */
void eips_usersock_process (void)
{
    uint16 i;
    struct pbuf *p;

    /* check if a UDP I/O send needs to go out */
    if(usersock_sockets[USERSOCK_IO_IX].send_buf_size > 0)
    {
        /* send data */
        p = pbuf_alloc(PBUF_TRANSPORT, usersock_sockets[USERSOCK_IO_IX].send_buf_size, PBUF_RAM);

        p->len = usersock_sockets[USERSOCK_IO_IX].send_buf_size;
        memcpy(p->payload, usersock_sockets[USERSOCK_IO_IX].send_buf, p->len);

        udp_sendto(usersock_sockets[USERSOCK_IO_IX].udp_pcb, p, &(usersock_sockets[USERSOCK_IO_IX].addr), usersock_sockets[USERSOCK_IO_IX].port);
        pbuf_free(p);

        #if EIPS_PRINTALL > 1
            eips_user_dbprint2("[Socket %d] eips_usersock_sendIOMessage %d bytes\r\n", usersock_sockets[USERSOCK_IO_IX].sockid, usersock_sockets[USERSOCK_IO_IX].send_buf_size);
        #endif

        /* free the message from our queue */
        usersock_sockets[USERSOCK_IO_IX].send_buf_size = 0; 
    }
    /* check if a UDP encap send needs to go out */
    if(usersock_sockets[USERSOCK_UDP_IX].send_buf_size > 0)
    {
        /* send data */
        p = pbuf_alloc(PBUF_TRANSPORT, usersock_sockets[USERSOCK_UDP_IX].send_buf_size, PBUF_RAM);

        p->len = usersock_sockets[USERSOCK_UDP_IX].send_buf_size;
        memcpy(p->payload, usersock_sockets[USERSOCK_UDP_IX].send_buf, p->len);

        udp_sendto(usersock_sockets[USERSOCK_UDP_IX].udp_pcb, p, &usersock_sockets[USERSOCK_UDP_IX].addr, usersock_sockets[USERSOCK_UDP_IX].port);
        pbuf_free(p);

        #if EIPS_PRINTALL > 1
            eips_user_dbprint2("[Socket %d] eips_usersock_sendUDPData %d bytes\r\n", usersock_sockets[USERSOCK_UDP_IX].sockid, usersock_sockets[USERSOCK_UDP_IX].send_buf_size);
        #endif

        /* free the message from our queue */
        usersock_sockets[USERSOCK_UDP_IX].send_buf_size = 0; 
    }

    /* check if TCP sends need to go out */
    for(i=USERSOCK_TCPSTART_IX; i<LOCAL_MAX_NUM_SOCKETS; i++)
    {
        if(usersock_sockets[i].send_buf_size > 0)
        {
            /* mark a flag to inform the user on the next pass about the send complete */
            usersock_sockets[i].tx_in_prog_flag = TRUE;

            /* send packet */
            local_tcp_write(usersock_sockets[i].pcb, usersock_sockets[i].send_buf, &usersock_sockets[i].send_buf_size, 0);

            #if EIPS_PRINTALL > 1
                eips_user_dbprint2("[Socket %d] eips_usersock_sendTCPData %d bytes\r\n", usersock_sockets[i].sockid, usersock_sockets[i].send_buf_size);
            #endif

            /* free the message from our queue */
            usersock_sockets[i].send_buf_size = 0; 
        }

        if(usersock_sockets[i].queue_send_buf_size > 0) {
            /* mark a flag to inform the user on the next pass about the send complete */
            usersock_sockets[i].tx_in_prog_flag = TRUE;

            /* send packet */
            local_tcp_write(usersock_sockets[i].pcb, usersock_sockets[i].queue_send_buf, &usersock_sockets[i].queue_send_buf_size, 0);

            #if EIPS_PRINTALL > 1
                eips_user_dbprint2("[Socket %d] eips_usersock_sendTCPData %d bytes\r\n", usersock_sockets[i].sockid, usersock_sockets[i].queue_send_buf_size);
            #endif

            /* free the message from our queue */
            usersock_sockets[i].queue_send_buf_size = 0;
        }
    }
}
/* ====================================================================
Function:   eips_usersock_close_all_sockets
Parameters: N/A
Returns:    N/A

This function closes all sockets (TCP and UDP). This would be used for
DLLs or other implementations that need to stop the EIP code from 
running. If the sockets aren't all closed, the next call to the 
eips_usersock_init() would fail when trying to bind the UDP and TCP 
listening sockets.
======================================================================= */
void eips_usersock_close_all_sockets (void)
{
    uint16 i;
#if 0
    if(usersock_sockets[USERSOCK_UDP_IX].udp_pcb != NULL)
        udp_remove(usersock_sockets[USERSOCK_UDP_IX].udp_pcb);

    if(usersock_sockets[USERSOCK_IO_IX].udp_pcb != NULL)
        udp_remove(usersock_sockets[USERSOCK_IO_IX].udp_pcb);
#endif
    /* close all sockets */
    for(i=USERSOCK_TCPSTART_IX; i<LOCAL_MAX_NUM_SOCKETS; i++)
    {
        if(usersock_sockets[i].pcb != NULL)
        {
            eips_usersock_tcpClose(usersock_sockets[i].sockid);
        }
    }
}
/* ====================================================================
Function:   eips_usersock_sendTCPData
Parameters: socket id
            pointer to socket data
            socket data size
Notes:		so far we dont have any queue mechanism here.
			if eips stack generate more than 1 packets of data to send, only the 1st one will be sent.

Returns:    N/A

This function sends the passed TCP data to the network client.
======================================================================= */
void eips_usersock_sendTCPData (EIPS_USERSYS_SOCKTYPE socket_id, uint8 * socket_data, uint16 socket_data_size)
{
    uint16 i;
    USERSOCK_SOCKSTRUCT *sock_ptr = NULL;
    
    /* see if thie soc is for us */
    for(i=USERSOCK_TCPSTART_IX; i<LOCAL_MAX_NUM_SOCKETS && sock_ptr==NULL; i++)
        if(usersock_sockets[i].sockid == socket_id)
            sock_ptr = &usersock_sockets[i];

    /* socket not found or not open */
    if((sock_ptr == NULL) || (sock_ptr->pcb == NULL))
        return;

    /* is the message too big */
    if(socket_data_size > sizeof(sock_ptr->send_buf))
        return;

//    if (socket_data_size == 28 && sock_ptr->send_buf_size != 0) {
//    	printf ("failed to respond to Register, send_buf_size is now %d\r\n", sock_ptr->send_buf_size);
//    }
    /* is there already a message pending? */
    if(sock_ptr->send_buf_size != 0) {
		sock_ptr->queue_send_buf_size = socket_data_size;
		memcpy(sock_ptr->queue_send_buf, socket_data, socket_data_size);
		return;
    }


    /* queue the message */
    sock_ptr->send_buf_size = socket_data_size;
    memcpy(sock_ptr->send_buf, socket_data, socket_data_size);
}

/* ====================================================================
Function:   eips_usersock_sendUDPData
Parameters: ip address to send the data to
            port to send the data to
            pointer to socket data
            socket data size
Returns:    N/A

This function sends the passed data to the network client.
======================================================================= */
void eips_usersock_sendUDPData (uint32 ipaddr, uint16 port, uint8 * socket_data, uint16 socket_data_size)
{
    /* known socket */
    USERSOCK_SOCKSTRUCT *sock_ptr = NULL;
    sock_ptr = &usersock_sockets[USERSOCK_UDP_IX];

    /* socket not found */
    if(sock_ptr == NULL)
        return;

    /* is the message too big */
    if(socket_data_size > sizeof(sock_ptr->send_buf))
        return;

    /* is there already a message pending? */
    if(sock_ptr->send_buf_size != 0)
        return;

    /* queue the message */
    sock_ptr->addr.addr = rta_Swap32(ipaddr);
    sock_ptr->port = port;
    sock_ptr->send_buf_size = socket_data_size;
    memcpy(sock_ptr->send_buf, socket_data, socket_data_size);
}

/* ====================================================================
Function:   eips_usersock_sendIOMessage
Parameters: IP Address
            Socket ID
            pointer to message to send
            message size
Returns:    N/A

This function tries to send the message over the passed socket.  Since
this is a udp socket, send failures are ignored.
======================================================================= */
void eips_usersock_sendIOMessage (uint32 ipaddr, uint8 *socket_data, int16 socket_data_size)
{
    /* known socket */
    USERSOCK_SOCKSTRUCT *sock_ptr = NULL;
    sock_ptr = &usersock_sockets[USERSOCK_IO_IX];

    /* socket not found */
    if(sock_ptr == NULL)
        return;

    /* is the message too big */
    if(socket_data_size > sizeof(sock_ptr->send_buf))
        return;

    /* is there already a message pending? */
    if(sock_ptr->send_buf_size != 0)
        return;

    /* queue the message */
    sock_ptr->addr.addr = rta_Swap32(ipaddr);
    sock_ptr->port = EIPS_UDPIO_PORT; /* fixed port */
    sock_ptr->send_buf_size = socket_data_size;
    memcpy(sock_ptr->send_buf, socket_data, socket_data_size);
}

/* ====================================================================
Function:   eips_usersock_tcpClose
Parameters: socket id
Returns:    N/A

This function closes the passed socket.
======================================================================= */
void eips_usersock_tcpClose (EIPS_USERSYS_SOCKTYPE socket_id)
{
    uint16 i;
    USERSOCK_SOCKSTRUCT *sock_ptr = NULL;
    
    /* see if this sock is for us */
    for(i=USERSOCK_TCPSTART_IX; i<LOCAL_MAX_NUM_SOCKETS && sock_ptr==NULL; i++)
        if(usersock_sockets[i].sockid == socket_id)
            sock_ptr = &usersock_sockets[i];

    /* socket not found */
    if((sock_ptr == NULL) || (sock_ptr->pcb == NULL))
        return;

    /* print Socket Close message */
    uint8 * TempVar;
    TempVar = (uint8 *)&(sock_ptr->pcb->remote_ip.addr); if (TempVar); // keep compiler happy
    //eips_user_printf("EIPS Socket Close: %d.%d.%d.%d", TempVar[0], TempVar[1], TempVar[2], TempVar[3]);

    /* reset all TCP callbacks */
    tcp_arg(sock_ptr->pcb, NULL);
    tcp_recv(sock_ptr->pcb, NULL);
    tcp_err(sock_ptr->pcb, NULL);
    tcp_poll(sock_ptr->pcb, NULL, 0);
    tcp_sent(sock_ptr->pcb, NULL);

    /* close the PCB */
    tcp_close(sock_ptr->pcb);

    sock_ptr->pcb = NULL;
    sock_ptr->send_buf_size = 0;
    eips_rtasys_onTCPClose(sock_ptr->sockid);
}

/* ====================================================================
Function:   eips_usersock_getSockInfo_sin_addr
Parameters: socket id
Returns:    sin_addr for the passed socket.

This function returns the sin_addr for the passed socket (0 on error).
======================================================================= */
uint32 eips_usersock_getSockInfo_sin_addr (EIPS_USERSYS_SOCKTYPE socket_id)
{
    uint16 i;
    USERSOCK_SOCKSTRUCT *sock_ptr = NULL;
    
    /* see if thie soc is for us */
    for(i=USERSOCK_TCPSTART_IX; i<LOCAL_MAX_NUM_SOCKETS && sock_ptr==NULL; i++)
        if(usersock_sockets[i].sockid == socket_id)
            sock_ptr = &usersock_sockets[i];

    /* socket not found */
    if((sock_ptr == NULL) || (sock_ptr->pcb == NULL))
        return(0);

    /* socket found, reutrn IP */
    return(rta_Swap32(sock_ptr->pcb->remote_ip.addr));
}

/* ====================================================================
Function:   eips_usersock_getOurIPAddr
Parameters: N/A
Returns:    32-bit IP Address

This function returns the current IP address of the device.
======================================================================= */
uint32 eips_usersock_getOurIPAddr (void)
{    
    EIPS_IPADDR_STRUCT ip_struct;

    /* get value from common function */
    if(eips_usersock_getTCPObj_Attr05_IntfCfg (&ip_struct) == ERR_SUCCESS)
    {
        return (ip_struct.IPAddress);
    }

    /* can't read IP, so return 0 */
    return(0);
}

/* ******************************************************************** */
/*                      LOCAL FUNCTIONS                                 */
/* ******************************************************************** */
/* ====================================================================
Function:   local_openUdpSocket
Parameters: user socket structure pointer
Returns:    N/A

This function attemps to open and bind a UDP socket based on the
parameters stored in the socket structure.
======================================================================= */
static void local_openUdpSocket (USERSOCK_SOCKSTRUCT *user_sock)
{
    if(!user_sock)
    {
        eips_usersys_fatalError("local_openUdpSocket1", 1);
        return;
    }

    /* allocate a socket */
    user_sock->udp_pcb = udp_new();

    /* couldn't get socket */
    if (!user_sock->udp_pcb) 
    {
        eips_usersys_fatalError("local_openUdpSocket3", 1);
        return;
    }

    /* if successful, open the socket */
    ip_set_option(user_sock->udp_pcb, SOF_BROADCAST); // allow receiving broadcast
    udp_bind(user_sock->udp_pcb, IP_ADDR_ANY, EIP_CIP_PORT);
    udp_recv(user_sock->udp_pcb, local_udp_recv, user_sock);
}
/**
 * Receive input function for UDP packets arriving for the Encapsulation Layer UDP pcb.
 *
 * @params see udp.h
 */
static void local_udp_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port)
{
    USERSOCK_SOCKSTRUCT *p_usersock = (USERSOCK_SOCKSTRUCT *)arg;
    if(!arg || !pcb || !p || !addr || !p_usersock)
        return;

    if(p_usersock->udp_pcb == pcb)
    {
        /* store the IP and port */
        p_usersock->addr.addr = addr->addr;
        p_usersock->port = port;

        /* inform RTA of the packet */
        eips_rtasys_onUDPData(rta_Swap32(addr->addr), port, p->payload, p->len);
    }

    /* we need to free the buffer */
    pbuf_free(p);
}

/* ====================================================================
Function:   local_openUdpIOSocket
Parameters: user socket structure pointer
Returns:    N/A

This function attemps to open a UDP I/O Socket.
======================================================================= */
static void local_openUdpIOSocket (USERSOCK_SOCKSTRUCT *user_sock)
{
    if(!user_sock)
    {
        eips_usersys_fatalError("local_openUdpSocket1", 1);
        return;
    }

    /* allocate a socket */
    user_sock->udp_pcb = udp_new();

    /* couldn't get socket */
    if (!user_sock->udp_pcb) 
    {
        eips_usersys_fatalError("local_openUdpSocket3", 1);
        return;
    }

    /* if successful, open the socket */
    udp_bind(user_sock->udp_pcb, IP_ADDR_ANY, EIPS_UDPIO_PORT);
    udp_recv(user_sock->udp_pcb, local_udpio_recv, user_sock);
}
/**
 * Receive input function for UDP packets arriving for the Encapsulation Layer UDP pcb.
 *
 * @params see udp.h
 */
static void local_udpio_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, ip_addr_t *addr, u16_t port)
{
    USERSOCK_SOCKSTRUCT *p_usersock = (USERSOCK_SOCKSTRUCT *)arg;

    if(!arg || !pcb || !p || !addr || !p_usersock)
        return;

    if(p_usersock->udp_pcb == pcb)
    {
        /* store the IP */
        p_usersock->addr.addr = addr->addr;

        /* inform RTA of the packet */
        eips_rtasys_onIOData(rta_Swap32(addr->addr), p->payload, p->len);
    }

    /* we need to free the buffer */
    pbuf_free(p);
}


/**
 * A new incoming connection has been accepted.
 */
static err_t
local_tcp_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{   
    int i;
    USERSOCK_SOCKSTRUCT *p_usersock = NULL;

    /* validate the arg pointer is our listen socket */
    if((struct tcp_pcb *)arg != listen_pcb)
        return(ERR_MEM); /* not our argument */

    for(i=USERSOCK_TCPSTART_IX; i<LOCAL_MAX_NUM_SOCKETS && p_usersock==NULL; i++)
    {
        if(usersock_sockets[i].pcb == NULL)
            p_usersock = &usersock_sockets[i];
    }

    /* did we find a free socket? */
    if(p_usersock == NULL)
        return(ERR_MEM); /* not our argument */
    
    /* allocate this pcb */
    p_usersock->pcb = pcb;

    /* Tell RTA about the new connection */       
    eips_rtasys_onTCPAccept(p_usersock->sockid);

    /* Decrease the listen backlog counter */
    tcp_accepted(listen_pcb);

    /* Set priority */
    tcp_setprio(pcb, TCP_PRIO_NORMAL);

    /* Tell TCP that this is the structure we wish to be passed for our callbacks. */
    tcp_arg(pcb, p_usersock);
    tcp_nagle_disable(pcb);
    /* Set up the various callback functions */
    tcp_recv(pcb, local_tcp_receive);
    tcp_err(pcb,  local_tcp_err);
    tcp_sent(pcb, local_tcp_sent);
    // start polling and reset tx progresses
    tcp_poll(pcb, local_tcp_poll, 1); // every 0.5s
    p_usersock->tx_in_prog_flag = FALSE;
    p_usersock->last_tx_time = Time_GetMs();

    /* print Socket Open message */
    uint8 * TempVar;
    TempVar = (uint8 *)&(pcb->remote_ip.addr); if (TempVar); // keep compiler happy
    //eips_user_printf("EIPS Socket Open %d.%d.%d.%d", TempVar[0], TempVar[1], TempVar[2], TempVar[3]);

    return ERR_OK;
}

/**
 * Data has been received on this pcb.
 * For HTTP 1.0, this should normally only happen once (if the request fits in one packet).
 */
static err_t
local_tcp_receive(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
    USERSOCK_SOCKSTRUCT *p_usersock = (USERSOCK_SOCKSTRUCT *)arg;

    /* error or closed by other side? */    
    if ((err != ERR_OK) || (p == NULL) || (p_usersock == NULL)) 
    {
        if (p != NULL) 
        {
            /* Inform TCP that we have taken the data. */
            tcp_recved(pcb, p->tot_len);
            pbuf_free(p);
        }
        if(p_usersock != NULL)
        {
            /* tell RTA about the free */
            eips_usersock_tcpClose(p_usersock->sockid);
        }
        return ERR_OK;
    }

    /* did we receive data? */
    if(p_usersock && p)
    {
        /* is this our PCB? */
        if(p_usersock->pcb == pcb)
        {
            /* call the RTA code */
            eips_rtasys_onTCPData(p_usersock->sockid, p->payload, p->len);
        }

        /* free the buffer either way */
        tcp_recved(pcb, p->tot_len);
        pbuf_free(p);
    }

    return ERR_OK;
}


/**
 * The pcb had an error and is already deallocated.
 * The argument might still be valid (if != NULL).
 */
static void
local_tcp_err(void *arg, err_t err)
{
    USERSOCK_SOCKSTRUCT *p_usersock = (USERSOCK_SOCKSTRUCT *)arg;

    if(p_usersock && p_usersock->pcb)
    {
        /* tell RTA about the free */
        eips_usersock_tcpClose(p_usersock->sockid);
    }
}

/**
 * Data has been sent and acknowledged by the remote host.
 * This means that more data can be sent.
 */
static err_t
local_tcp_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
    USERSOCK_SOCKSTRUCT *p_usersock = (USERSOCK_SOCKSTRUCT *)arg;

    if(p_usersock->pcb == pcb)
    {
        eips_rtasys_onTCPSent(p_usersock->sockid);
    }

    p_usersock->tx_in_prog_flag = FALSE;
    p_usersock->last_tx_time = Time_GetMs();
    return ERR_OK;
}

/**
 * The poll function is called twice a second.
 * If there has been no data sent in 2 seconds, send a keepalive message (NOP).
 * If the last portion of a file has not been sent in 2 seconds, close.
 */
static err_t
local_tcp_poll(void *arg, struct tcp_pcb *pcb)
{
    static uint8 keepMeAlive[EIP_ENCAP_HEADER_SIZ] = { 0 };
    static uint16 keepMeAliveSize = EIP_ENCAP_HEADER_SIZ;
	USERSOCK_SOCKSTRUCT *p_usersock = (USERSOCK_SOCKSTRUCT *)arg;
	if ((p_usersock->tx_in_prog_flag==FALSE) &&
		((Time_GetMs()-p_usersock->last_tx_time) >= 2000))
	{
	    p_usersock->tx_in_prog_flag = TRUE;
		return local_tcp_write(p_usersock->pcb, keepMeAlive, &keepMeAliveSize, 0);
	}
    return ERR_OK;
}

/** Call tcp_write() in a loop trying smaller and smaller length
 *
 * @param pcb tcp_pcb to send
 * @param ptr Data to send
 * @param length Length of data to send (in/out: on return, contains the
 *        amount of data sent)
 * @param apiflags directly passed to tcp_write
 * @return the return value of tcp_write
 */
static err_t local_tcp_write(struct tcp_pcb *pcb, const void* ptr, u16_t *length, u8_t apiflags)
{
    u16_t len;
    err_t err;
    LWIP_ASSERT("length != NULL", length != NULL);
    len = *length;
    do 
    {
        err = tcp_write(pcb, ptr, len, apiflags);
        if (err == ERR_MEM) 
        {
            if ((tcp_sndbuf(pcb) == 0) || (tcp_sndqueuelen(pcb) >= TCP_SND_QUEUELEN)) 
            {
                /* no need to try smaller sizes */
                len = 1;
            } 
            else 
            {
                len /= 2;
            }
        }
    } while ((err == ERR_MEM) && (len > 1));

    if (err == ERR_OK) 
    {
        LWIP_DEBUGF(LWIP_DBG_TRACE, ("Sent %d bytes\n", len));
    } 
    else 
    {
        LWIP_DEBUGF(LWIP_DBG_TRACE, ("Send failed with err %d (\"%s\")\n", err, lwip_strerr(err)));
    }

    tcp_output(pcb);

    *length = len;
    return err;
}



/* ******************************************************************** */
/*                      TCP/IP Object                                 	*/
/* ******************************************************************** */
/* ====================================================================
Function:   eips_usersock_getTCPObj_Attr01_Status
Parameters: pointer to store data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF5,0x01,0x01

See the TCP/IP Interface Object (0xF5) for information on this attribute.
======================================================================= */
uint8 eips_usersock_getTCPObj_Attr01_Status (uint32 *data_ptr)
{
	if(data_ptr == NULL)						// validate the passed data pointer
	{
		return(ERR_NO_ATTR_DATA);
	}
	*data_ptr = 0x01;							// 1 for valid configuration

	return(ERR_SUCCESS);
}

/* ====================================================================
Function:   eips_usersock_getTCPObj_Attr02_CfgCap
Parameters: pointer to store data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF5,0x01,0x02

See the TCP/IP Interface Object (0xF5) for information on this attribute.
======================================================================= */
uint8 eips_usersock_getTCPObj_Attr02_CfgCap (uint32 *data_ptr)
{
	if(data_ptr == NULL)						// validate the passed data pointer
	{
		return(ERR_NO_ATTR_DATA);
	}
	*data_ptr = 0x04;							// bit 2 - DHCP Client

	/* set default to 0 */
    /* *data_ptr |= 0x01; */    /* add in if BootP Client is supported */
    /* *data_ptr |= 0x02; */    /* add in if DNS Client is supported */
    /* *data_ptr |= 0x04; */    /* add in if DHCP Client is supported */
    /* *data_ptr |= 0x08; */    /* reserved, shouldn't be set */
    /* *data_ptr |= 0x10; */    /* add in if Attr5 is settable */
    /* *data_ptr |= 0x20; */    /* add in if hardware configurable */
    /* *data_ptr |= 0x40; */    /* add in if Attr5 set requires a reset */
    /* *data_ptr |= 0x80; */    /* add in ACD is supported */

	return(ERR_SUCCESS);
}

/* ====================================================================
Function:   eips_usersock_getTCPObj_Attr03_CfgCtrl
Parameters: pointer to store data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF5,0x01,0x03

See the TCP/IP Interface Object (0xF5) for information on this attribute.
======================================================================= */
uint8 eips_usersock_getTCPObj_Attr03_CfgCtrl (uint32 *data_ptr)
{
	if(data_ptr == NULL)						// validate the passed data pointer
	{
		return(ERR_NO_ATTR_DATA);
	}
    *data_ptr = 0x2;							// Use DHCP on Startup

    return(ERR_SUCCESS);
}

/* ====================================================================
Function:   eips_usersock_getTCPObj_Attr04_PhyLinkObj
Parameters: pointer to store data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF5,0x01,0x04

See the TCP/IP Interface Object (0xF5) for information on this attribute.

typedef struct
{
  uint16  PathSiz;
  uint8   Path[12];
}EIPS_PHY_LINK_STRUCT;
======================================================================= */
uint8 eips_usersock_getTCPObj_Attr04_PhyLinkObj (EIPS_PHY_LINK_STRUCT *data_ptr)
{
    if(data_ptr == NULL)						// validate the passed data pointer
	{
		return(ERR_NO_ATTR_DATA);
	}

    // store the data
    data_ptr->PathSiz = 2;		// in words
    data_ptr->Path[0] = 0x20;   // 8-bit Class
    data_ptr->Path[1] = 0xF6;   // Ethernet Link Class
    data_ptr->Path[2] = 0x24;   // 8-bit Instance
    data_ptr->Path[3] = 0x01;   // Instance 1

    return(ERR_SUCCESS);
}

/* ====================================================================
Function:   eips_usersock_getTCPObj_Attr05_IntfCfg
Parameters: pointer to store data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF5,0x01,0x05

See the TCP/IP Interface Object (0xF5) for information on this attribute.

typedef struct
{
  uint32 IPAddress;
  uint32 NtwkMask;
  uint32 GtwyIPAddress;
  uint32 NameServer;
  uint32 NameServer2;
  EIPS_TCPOBJ_BUF_STRUCT DomainName;
}EIPS_IPADDR_STRUCT;
======================================================================= */
uint8 eips_usersock_getTCPObj_Attr05_IntfCfg (EIPS_IPADDR_STRUCT *data_ptr)
{
    if(data_ptr == NULL)						// validate the passed data pointer
	{
		return(ERR_NO_ATTR_DATA);
	}

	data_ptr->IPAddress = rta_GetBigEndian32((uint8 *)&(lwip_netif.ip_addr.addr));
	data_ptr->NtwkMask = rta_GetBigEndian32((uint8 *)&(lwip_netif.netmask.addr));
	data_ptr->GtwyIPAddress = rta_GetBigEndian32((uint8 *)&(lwip_netif.gw.addr));

	data_ptr->NameServer        = 0;			// Name Server IP Address
	data_ptr->NameServer2       = 0;			// Name Server 2 IP Address
	data_ptr->DomainName.Bufsiz = 0;			// Domain Name

	return(ERR_SUCCESS);
}

/* ====================================================================
Function:   eips_usersock_getTCPObj_Attr06_HostName
Parameters: pointer to store data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF5,0x01,0x06

See the TCP/IP Interface Object (0xF5) for information on this attribute.

typedef struct
{
  uint16  Bufsiz;
  uint8   Buf[64];
}EIPS_TCPOBJ_BUF_STRUCT;
======================================================================= */
uint8 eips_usersock_getTCPObj_Attr06_HostName (EIPS_TCPOBJ_BUF_STRUCT *data_ptr)
{
    if(data_ptr == NULL)							// validate the passed data pointer
	{
		return(ERR_NO_ATTR_DATA);
	}

    // store the data
    //data_ptr->Bufsiz = SYS_INFO_DATA_LENGTH;		// data length
	//memcpy(data_ptr->Buf, SMARTCORE_INFO_GET(SMARTCORE_IF_HOST_NAME), SYS_INFO_DATA_LENGTH);
	data_ptr->Bufsiz = 0;			// not used

    return(ERR_SUCCESS);
}

/* ====================================================================
Function:   eips_usersock_setTCPObj_Attr03_CfgCtrl
Parameters: pointer with new data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF5,0x01,0x03

See the TCP/IP Interface Object (0xF5) for information on this attribute.
======================================================================= */
uint8 eips_usersock_setTCPObj_Attr03_CfgCtrl (uint32 *data_ptr)
{
    if(data_ptr == NULL)							// validate the passed data pointer
	{
		return(ERR_NO_ATTR_DATA);
	}

    return(ERR_ATTR_READONLY);						// data is read only
}

/* ====================================================================
Function:   eips_usersock_setTCPObj_Attr05_IntfCfg
Parameters: pointer with new data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF5,0x01,0x05

See the TCP/IP Interface Object (0xF5) for information on this attribute.

typedef struct
{
  uint32 IPAddress;
  uint32 NtwkMask;
  uint32 GtwyIPAddress;
  uint32 NameServer;
  uint32 NameServer2;
  EIPS_TCPOBJ_BUF_STRUCT DomainName;
}EIPS_IPADDR_STRUCT;
======================================================================= */
uint8 eips_usersock_setTCPObj_Attr05_IntfCfg (EIPS_IPADDR_STRUCT *data_ptr)
{
    if(data_ptr == NULL)							// validate the passed data pointer
	{
		return(ERR_NO_ATTR_DATA);
	}

    // data is read only
    return(ERR_ATTR_READONLY);
}

/* ====================================================================
Function:   eips_usersock_setTCPObj_Attr06_HostName
Parameters: pointer with new data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF5,0x01,0x06

See the TCP/IP Interface Object (0xF5) for information on this attribute.

typedef struct
{
  uint16  Bufsiz;
  uint8   Buf[64];
}EIPS_TCPOBJ_BUF_STRUCT;
======================================================================= */
uint8 eips_usersock_setTCPObj_Attr06_HostName (EIPS_TCPOBJ_BUF_STRUCT *data_ptr)
{
    if(data_ptr == NULL)							// validate the passed data pointer
	{
		return(ERR_NO_ATTR_DATA);
	}

    // data is read only
    return(ERR_ATTR_READONLY);
}

/* ******************************************************************** */
/*                  Ethernet Link Object                                */
/* ******************************************************************** */
/* ====================================================================
Function:   eips_usersock_getEthLinkObj_Attr01_IntfSpeed
Parameters: pointer to store data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF6,0x01,0x01

See the Ethernet Link Object (0xF6) for information on this attribute.
======================================================================= */
uint8 eips_usersock_getEthLinkObj_Attr01_IntfSpeed (uint32 *data_ptr)
{
   // uint32 nSpeed;

	if(data_ptr == NULL)							// validate the passed data pointer
	{
		return(ERR_NO_ATTR_DATA);
	}

    /* store the data */
    *data_ptr = 100;	//100 MB
    //*data_ptr = 10;		//10 MB

  // return Success
  return(ERR_SUCCESS);
}

/* ====================================================================
Function:   eips_usersock_getEthLinkObj_Attr02_IntfFlags
Parameters: pointer to store data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF6,0x01,0x02

See the Ethernet Link Object (0xF6) for information on this attribute.
======================================================================= */
uint8 eips_usersock_getEthLinkObj_Attr02_IntfFlags (uint32 *data_ptr)
{
    if(data_ptr == NULL)							// validate the passed data pointer
	{
		return(ERR_NO_ATTR_DATA);
	}

    // store the data
    //*data_ptr = 0x1;
    if (!(lwip_netif.flags & NETIF_FLAG_LINK_UP))
        /* link status, negotiation status = NO LINK */
        *data_ptr = 0x00;
    else
		/* link status, negotiation status = successful */
		*data_ptr = 0x0D;

    // return Success
    return(ERR_SUCCESS);
}

/* ====================================================================
Function:   eips_usersock_getEthLinkObj_Attr03_MacAddr
Parameters: pointer to store data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF6,0x01,0x03

See the Ethernet Link Object (0xF6) for information on this attribute.

uint8 madid_6byte_string[6];
======================================================================= */
uint8 eips_usersock_getEthLinkObj_Attr03_MacAddr (uint8 *macid_6byte_string)
{
    if(macid_6byte_string == NULL)							// validate the passed data pointer
	{
		return(ERR_NO_ATTR_DATA);
	}

    // store the data
    memcpy(macid_6byte_string, lwip_netif.hwaddr, 6);

    // return Success
    return(ERR_SUCCESS);
}

/* ====================================================================
Function:   eips_usersock_getEthLinkObj_Attr04_IntfCnts
Parameters: pointer to store data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF6,0x01,0x04 (Optional)

See the Ethernet Link Object (0xF6) for information on this attribute.

typedef struct
{
    uint32 InOctets;
    uint32 InUcastPackets;
    uint32 InNUcastPackets;
    uint32 InDiscards;
    uint32 InErrors;
    uint32 InUnknownProtos;
    uint32 OutOctets;
    uint32 OutUcastPackets;
    uint32 OutNUcastPackets;
    uint32 OutDiscards;
    uint32 OutErrors;
}EIPS_ELINKOBJ_INTFCNTS_STRUCT;
======================================================================= */
uint8 eips_usersock_getEthLinkObj_Attr04_IntfCnts (EIPS_ELINKOBJ_INTFCNTS_STRUCT *data_ptr)
{
    RTA_UNUSED_PARAM(data_ptr);

    // return "attribute unsupported"
    return(ERR_ATTR_UNSUPP);
}

/* ====================================================================
Function:   eips_usersock_getEthLinkObj_Attr05_MediaCnts
Parameters: pointer to store data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF6,0x01,0x05 (Optional)

See the Ethernet Link Object (0xF6) for information on this attribute.

typedef struct
{
    uint32 AlignmentErrors;
    uint32 FCSErrors;
    uint32 SingleCollisions;
    uint32 MultipleCollisions;
    uint32 SQETestErrors;
    uint32 DeferredTransmissions;
    uint32 LateCollisions;
    uint32 ExcessiveCollisions;
    uint32 MACTransmitErrors;
    uint32 CarrierSenseErrors;
    uint32 FrameTooLarge;
    uint32 MACReceiveErrors;
}EIPS_ELINKOBJ_MEDIACNTS_STRUCT;
======================================================================= */
uint8 eips_usersock_getEthLinkObj_Attr05_MediaCnts (EIPS_ELINKOBJ_MEDIACNTS_STRUCT *data_ptr)
{
    RTA_UNUSED_PARAM(data_ptr);

	// return "attribute unsupported"
    return(ERR_ATTR_UNSUPP);
}

/* ====================================================================
Function:   eips_usersock_getEthLinkObj_Attr06_IntfCtrl
Parameters: pointer to store data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF6,0x01,0x06 (Optional)

See the Ethernet Link Object (0xF6) for information on this attribute.

typedef struct
{
    uint16 ControlBits;
    uint16 ForcedInterfaceSpeed;
}EIPS_ELINKOBJ_INTFCTRL_STRUCT;
======================================================================= */
uint8 eips_usersock_getEthLinkObj_Attr06_IntfCtrl (EIPS_ELINKOBJ_INTFCTRL_STRUCT *data_ptr)
{
    RTA_UNUSED_PARAM(data_ptr);

    // return "attribute unsupported"
    return(ERR_ATTR_UNSUPP);
}

/* ====================================================================
Function:   eips_usersock_setEthLinkObj_Attr06_IntfCtrl
Parameters: pointer to store data
Returns:    General Status - "ERR_SUCCESS" on Success;
                             Error code from eips_cpf.h on Error
C/I/A       0xF6,0x01,0x06 (Optional)

See the Ethernet Link Object (0xF6) for information on this attribute.

typedef struct
{
    uint16 ControlBits;
    uint16 ForcedInterfaceSpeed;
}EIPS_ELINKOBJ_INTFCTRL_STRUCT;
======================================================================= */
uint8 eips_usersock_setEthLinkObj_Attr06_IntfCtrl (EIPS_ELINKOBJ_INTFCTRL_STRUCT *data_ptr)
{
    RTA_UNUSED_PARAM(data_ptr);

    // return "attribute unsupported" if some Ethernet Link attributes are settable
    return(ERR_ATTR_UNSUPP);
}


/* ******************************************************************** */
/*                      END OF FILE                               		*/
/* ******************************************************************** */
