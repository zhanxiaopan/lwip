

#ifndef PN_NET_H
#define PN_NET_H

#include <pn_includes.h>


/***************************************************************************/
/* Defines and Enums */

/* Ethernet Port Defines
 *   PN_NET_PORT(x)
 *     External (switch) port with a virtual MAC address. Numbering goes from 0
 *     to (CONFIG_TARGET_ETH_PORT_COUNT - 1). If this is used as network port a
 *     frame signalizes that it came in from exactly this port or the frame
 *     should go out over exactly this port.
 *
 *   PN_NET_PORT_HOST
 *     Internal host port with the device MAC address. If this is used as
 *     network port for transmitting frames then the switch has to decide on
 *     which port the frame has to leave by his internal MAC table. Don't use
 *     this for incoming frames, if your device only has one Ethernet port then
 *     set the incoming port to PN_NET_PORT(0)
 */
#define PN_NET_PORT(x)          x
#define PN_NET_PORT_HOST        PN_MAX_U32


/***************************************************************************/
/* Prototypes */
IOD_STATUS_T PN_netInit(void);
void PN_netRawGetFrameID(OAL_BUFFER_T *, Unsigned16 *);
void PN_netEthUp(void);
void PN_netEthDown(void);
IOD_STATUS_T PN_netIpSet(
    Unsigned32 ip,
    Unsigned32 netmask,
    Unsigned32 gateway,
    PN_BOOL_T intFlag,
    PN_BOOL_T tempFlag,
    PN_BOOL_T cfgFlag
);
IOD_STATUS_T PN_netIpSetExt(Unsigned32 ip, Unsigned32 netmask, Unsigned32 gateway);
void PN_netIpGet(Unsigned32 *pIp, Unsigned32 *pNetmask, Unsigned32 *pGateway);
PN_BOOL_T PN_netIpIsTemp(void);
IOD_STATUS_T PN_netIpClear(void);
IOD_STATUS_T PN_netEthSend(OAL_BUFFER_T **ppBuf, PN_NET_TX_TYPE_T prio);
IOD_STATUS_T PN_netEthSendDone(OAL_BUFFER_T **ppBuf, PN_BOOL_T doneFlag);


#endif /* PN_NET_H */
