/*
 * oal_lwip.c
 *
 *  OS Abstraction Layer based on the LwIP
 *
 */

#include <stdint.h>
#include <stdbool.h>

#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/tcp_impl.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"

#include "netif/etharp.h"

#include "oal_lwip.h"

#include "hal.h"
#include "pn_oal_eth.h"
#include "tivaif.h"


//extern uint32_t g_ui32ClkFreq;

struct netif g_netif;

ip_addr_t g_ip_addr;

extern uint32_t g_ui32LocalTimer;
static uint32_t g_ui32TCPTimer = 0;
static uint32_t g_ui32HostTimer = 0;
static uint32_t g_ui32ARPTimer = 0;

//struct timer periodic_timer;

static PN_LOCK_T udp_chan_mutex;
static HAL_UDP_CHAN_T udp_chan_slots[HAL_NUM_UDP_PORTS];


static void lwip_host_timer_handler()
{
    ip_addr_t local_ip_addr;

    // Get the current IP address.
    local_ip_addr.addr = g_netif.ip_addr.addr;

    // See if the IP address has changed.
    if(local_ip_addr.addr != g_ip_addr.addr)
    {
        // Save the new IP address.
        g_ip_addr.addr = local_ip_addr.addr;
    }

    // If there is not an IP address.
    if((local_ip_addr.addr == 0) || (local_ip_addr.addr == 0xffffffff))
    {
        // Loop through the LED animation.
        // Toggle the user led
        //HAL_GPIOPinToggle(&HAL_GPIO_LED8);
    }
    else
    {
        // Turn LED ON.
        //HAL_GPIOPinWrite(&HAL_GPIO_LED8, true);
    }
}


//*****************************************************************************
//
// This function services all of the lwIP periodic timers, including TCP and
// Host timers.  This should be called from the lwIP context, which may be
// the Ethernet interrupt (in the case of a non-RTOS system) or the lwIP
// thread, in the event that an RTOS is used.
//
//*****************************************************************************
static void OAL_LwIPTimersRun()
{
    //
    // Service the host timer.
    //
#if HOST_TMR_INTERVAL
    if((g_ui32LocalTimer - g_ui32HostTimer) >= HOST_TMR_INTERVAL)
    {
        g_ui32HostTimer = g_ui32LocalTimer;
        //lwIPHostTimerHandler();
        lwip_host_timer_handler();
    }
#endif

    //
    // Service the ARP timer.
    //
#if LWIP_ARP
    if((g_ui32LocalTimer - g_ui32ARPTimer) >= ARP_TMR_INTERVAL)
    {
        g_ui32ARPTimer = g_ui32LocalTimer;
        etharp_tmr();
    }
#endif

    //
    // Service the TCP timer.
    //
#if LWIP_TCP
    if((g_ui32LocalTimer - g_ui32TCPTimer) >= TCP_TMR_INTERVAL)
    {
        g_ui32TCPTimer = g_ui32LocalTimer;
        tcp_tmr();
    }
#endif

    //
    // Service the AutoIP timer.
    //
#if LWIP_AUTOIP
    if((g_ui32LocalTimer - g_ui32AutoIPTimer) >= AUTOIP_TMR_INTERVAL)
    {
        g_ui32AutoIPTimer = g_ui32LocalTimer;
        autoip_tmr();
    }
#endif

    //
    // Service the DCHP Coarse Timer.
    //
#if LWIP_DHCP
    if((g_ui32LocalTimer - g_ui32DHCPCoarseTimer) >= DHCP_COARSE_TIMER_MSECS)
    {
        g_ui32DHCPCoarseTimer = g_ui32LocalTimer;
        dhcp_coarse_tmr();
    }
#endif

    //
    // Service the DCHP Fine Timer.
    //
#if LWIP_DHCP
    if((g_ui32LocalTimer - g_ui32DHCPFineTimer) >= DHCP_FINE_TIMER_MSECS)
    {
        g_ui32DHCPFineTimer = g_ui32LocalTimer;
        dhcp_fine_tmr();
    }
#endif

    //
    // Service the IP Reassembly Timer
    //
#if IP_REASSEMBLY
    if((g_ui32LocalTimer - g_ui32IPReassemblyTimer) >= IP_TMR_INTERVAL)
    {
        g_ui32IPReassemblyTimer = g_ui32LocalTimer;
        ip_reass_tmr();
    }
#endif

    //
    // Service the IGMP Timer
    //
#if LWIP_IGMP
    if((g_ui32LocalTimer - g_ui32IGMPTimer) >= IGMP_TMR_INTERVAL)
    {
        g_ui32IGMPTimer = g_ui32LocalTimer;
        igmp_tmr();
    }
#endif

    //
    // Service the DNS Timer
    //
#if LWIP_DNS
    if((g_ui32LocalTimer - g_ui32DNSTimer) >= DNS_TMR_INTERVAL)
    {
        g_ui32DNSTimer = g_ui32LocalTimer;
        dns_tmr();
    }
#endif

    //
    // Service the link timer.
    //
#if LWIP_AUTOIP || LWIP_DHCP
    if((g_ui32LocalTimer - g_ui32LinkTimer) >= LINK_TMR_INTERVAL)
    {
        g_ui32LinkTimer = g_ui32LocalTimer;
//        lwIPLinkDetect();
        lwip_link_detect();
    }
#endif

    return;
}


//*****************************************************************************
//
//! Handles Ethernet interrupts for the lwIP TCP/IP stack.
//!
//! This function handles Ethernet interrupts for the lwIP TCP/IP stack.  At
//! the lowest level, all receive packets are placed into a packet queue for
//! processing at a higher level.  Also, the transmit packet queue is checked
//! and packets are drained and transmitted through the Ethernet MAC as needed.
//! If the system is configured without an RTOS, additional processing is
//! performed at the interrupt level.  The packet queues are processed by the
//! lwIP TCP/IP code, and lwIP periodic timers are serviced (as needed).
//!
//! \return None.
//
//*****************************************************************************
void LwIPEthernetIntHandler(void)
{
    uint32_t status;

    // Read and Clear the interrupt.
    status = EMACIntStatus(EMAC0_BASE, true);
    // If the interrupt came from the Ethernet and not our timer, clear it.
    if(status)
    {
        EMACIntClear(EMAC0_BASE, status);
    }

    // If a transmit/receive interrupt was active,
    // run the low-level interrupt handler.
    if(status)
    {
        tivaif_interrupt(&g_netif, status);
        OAL_ethSendTrigger();
    }

    // service the PN timers
    PN_timerRun();

    // Service the lwIP timers.
    OAL_LwIPTimersRun();

    return;
}


//*****************************************************************************
//
// Completes the initialization of lwIP.  This is directly called when not
// using a RTOS and provided as a callback to the TCP/IP thread when using a
// RTOS.
//
//*****************************************************************************
void OAL_setupLwIP(uint32_t ip, uint32_t netmask, uint32_t gateway)
{
    struct ip_addr ip_addr;
    struct ip_addr net_mask;
    struct ip_addr gw_addr;

    // initialize the lwIP stack.
    lwip_init();

    ip_addr.addr = ip;//htonl
    net_mask.addr = netmask;//htonl
    gw_addr.addr = gateway;//htonl

    // Create, configure and add the Ethernet controller interface with
    // default settings.  ip_input should be used to send packets directly to
    // the stack when not using a RTOS and tcpip_input should be used to send
    // packets to the TCP/IP thread's queue when using a RTOS.
    netif_add(&g_netif, &ip_addr, &net_mask, &gw_addr, NULL, tivaif_init, ip_input);

    netif_set_default(&g_netif);

    // Bring the interface up.
    netif_set_up(&g_netif);

    return;
}


IOD_STATUS_T OAL_udpInit()
{
//    /* setup timers depending on delayed-ACK support */
//#ifdef CONFIG_UIP_TCP_DELAYED_ACK
//    timer_set(&periodic_timer, CLOCK_SECOND / 200);
//#else
//    timer_set(&periodic_timer, CLOCK_SECOND / 2);
//#endif

//    timer_set(&periodic_timer, CLOCK_SECOND / 2);

    /* init UDP channel array */
    memset(udp_chan_slots, 0, sizeof(HAL_UDP_CHAN_T) * HAL_NUM_UDP_PORTS);

    /* create UDP slot mutex */
    PN_lockCreate(PN_LOCK_BINARY, &udp_chan_mutex, 0, 1, PN_USAGE_OAL_UDP_SLOTS);

    return IOD_OK;
}

/** Set the IP Configuration
 *
 * Set the IP, netmask and gateway address.
 *
 * @returns IOD_OK - success
 * @returns other - fail
 */
IOD_STATUS_T OAL_ipConfig(
    Unsigned8 *pIPAddr,         /**< pointer to 4 byte IP address */
    Unsigned8 *pIPMask,         /**< pointer to 4 byte subnet mask */
    Unsigned8 *pDefaultGW,      /**< pointer to 4 byte gateway address */
    PN_BOOL_T  tempFlag         /**< temporary IP conf flag */
)
{
    unsigned char tmp[IPV4_ADDR_LEN];           /**< temp address */

    ip_addr_t ipaddr;

    UNUSEDARG(tempFlag);

    PN_htobe32_p((Unsigned32 *) tmp, *((Unsigned32 *) pIPAddr));
    ipaddr.addr = *((uint32_t *)tmp);
    netif_set_ipaddr(&g_netif, &ipaddr);

    PN_htobe32_p((Unsigned32 *) tmp, *((Unsigned32 *) pIPMask));
    ipaddr.addr = *((uint32_t *)tmp);
    netif_set_netmask(&g_netif, &ipaddr);

    PN_htobe32_p((Unsigned32 *) tmp, *((Unsigned32 *) pDefaultGW));
    ipaddr.addr = *((uint32_t *)tmp);
    netif_set_gw(&g_netif, &ipaddr);

    return IOD_OK;
}


static void OAL_udpRecv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
               struct ip_addr *addr, u16_t port)
{
    OAL_UDP_ADDR_T udp_addr;            /**< address structure */
    int cnt;                            /**< counter */

    // lock udp slots
    if (IOD_OK != PN_lockGet(&udp_chan_mutex, PN_LOCK_INFINITE))
    {
        return;
    }
    // find a matching UDP slot
    for (cnt = 0; cnt < HAL_NUM_UDP_PORTS; cnt++)
    {
        if (PN_TRUE == udp_chan_slots[cnt].used)
        {
            if (pcb->local_port == udp_chan_slots[cnt].port)
            {
                break;
            }
        }
    }
    // unlock the udp slots
    PN_lockPut(&udp_chan_mutex);
    // check if the matching slot was found
    if (HAL_NUM_UDP_PORTS <= cnt)
    {
        return;
    }
    // update address structure and then call the callback
    udp_addr.localPort = pcb->local_port;
    udp_addr.remoteIp = ntohl(addr->addr);
    udp_addr.remotePort = port;

    udp_chan_slots[cnt].callback(cnt, &udp_addr, p->payload, p->tot_len);

    // Free the pbuf.
    pbuf_free(p);
    return;
}


IOD_STATUS_T OAL_udpOpen(
    int *pSockIdx,                      /**< pointer to store socket index */
    Unsigned16 port,                    /**< UDP port number */
    OAL_UDP_RECV_CB_T callback          /**< receive callback */
)
{
    IOD_STATUS_T res;                   /**< return status */
    int cnt;                            /**< counter */

    /* lock UDP slots */
    res = PN_lockGet(&udp_chan_mutex, PN_LOCK_INFINITE);
    if (IOD_OK != res)
    {
        return res;
    }

    /* find a free UDP slot */
    for (cnt = 0; cnt < HAL_NUM_UDP_PORTS; cnt++)
    {
        if (PN_FALSE == udp_chan_slots[cnt].used)
        {
            break;
        }
    }

    /* check if a free slot was found */
    if (HAL_NUM_UDP_PORTS <= cnt)
    {

        /* unlock UDP slots */
        PN_lockPut(&udp_chan_mutex);

        return IOD_UDP_OPEN_FAILED;
    }

    udp_chan_slots[cnt].conn = udp_new();
    if (NULL == udp_chan_slots[cnt].conn)
    {
        /* unlock UDP slots */
        PN_lockPut(&udp_chan_mutex);
        return IOD_UDP_OPEN_FAILED;
    }
    //
    udp_recv(udp_chan_slots[cnt].conn, &OAL_udpRecv, NULL);
    udp_bind(udp_chan_slots[cnt].conn, IP_ADDR_ANY, port);
    //
    udp_chan_slots[cnt].port = port;
    udp_chan_slots[cnt].callback = callback;
    udp_chan_slots[cnt].used = PN_TRUE;

    *pSockIdx = cnt;

    /* unlock UDP slots */
    PN_lockPut(&udp_chan_mutex);

    return IOD_OK;
}


/** Close an UDP Channel
 *
 * Close the specified UDP channel handle.
 *
 * @returns IOD_OK - success
 * @returns other - fail
 */
IOD_STATUS_T OAL_udpClose(
    int *pChan                      /**< UDP channel handle */
)
{
    struct udp_pcb *conn;

    conn = udp_chan_slots[*pChan].conn;
    udp_chan_slots[*pChan].used = PN_FALSE;
    //
    udp_remove(conn);
    *pChan = -1;

    return IOD_OK;
}


/** Send Data over UDP Channel
 *
 * Send data in pBuf with bufLen size over the UDP channel to the destination
 * address given in pDA.
 *
 * @returns IOD_OK - success
 * @returns other - fail
 */
IOD_STATUS_T OAL_udpSend(
    int             chan,           /**< UDP channel handle */
    OAL_UDP_ADDR_T *pAddr,          /**< UDP address */
    Unsigned8      *pBuf,           /**< buffer with data to send */
    Unsigned16      bufLen          /**< size of data to be send */
)
{
    IOD_STATUS_T res;               /**< return result */

    struct udp_pcb *udp_conn;
    struct pbuf *udp_buf;
    ip_addr_t dest_ip;

    /* lock UDP slots */
    res = PN_lockGet(&udp_chan_mutex, PN_LOCK_INFINITE);
    if (IOD_OK != res) {
        return res;
    }

    /* check if the selected UDP slot is active */
    if (PN_FALSE == udp_chan_slots[chan].used)
    {
        /* unlock UDP slots */
        PN_lockPut(&udp_chan_mutex);
        return IOD_UDP_SEND_FAILED;
    }

    /* set UDP channel handle */
    udp_conn = udp_chan_slots[chan].conn;

    /* allocate space for whole UDP frame */
    udp_buf = pbuf_alloc(PBUF_TRANSPORT, bufLen, PBUF_RAM);
    if (NULL == udp_buf)
    {
        /* unlock UDP slots */
        PN_lockPut(&udp_chan_mutex);
//        PN_logErr("could not allocate UDP pbuf");
        return IOD_NET_UDP_SEND_FAILED;
    }

    /* copy data into UDP frame */
    memcpy(udp_buf->payload, pBuf, bufLen);

    /* copy dest IP data from pAddr */
    dest_ip.addr = htonl(pAddr->remoteIp);

    /* send frame  */
    udp_sendto(udp_conn, udp_buf, &dest_ip, pAddr->remotePort);

    // and release buffer
    pbuf_free(udp_buf);

    /* unlock UDP slots */
    PN_lockPut(&udp_chan_mutex);

    return IOD_OK;
}

