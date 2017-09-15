

#ifndef PN_LLDP_H
#define PN_LLDP_H
#ifdef CONFIG_LLDP_INTERNAL

/* public constant definitions
-------------------------------------------------------------------------- */
#define LLDP_MAN_ADDRESS    16
#define LLDP_PROTOID_LEN    64
#define LLDP_PNIO_ALIAS     64
#define LLDP_CHASSISID_LEN  64
#define LLDP_PORTID_LEN     16
#define LLDP_OBJECTID_LEN   8
#define LLDP_PORT_DESC_LEN  255
#define LLDP_SYS_DESC_LEN   255


/* list of defined structures
-------------------------------------------------------------------------- */
typedef struct {
    PN_BOOL_T               valid;
    char                    portDesc[LLDP_PORT_DESC_LEN];

#ifdef LLDP_OPT_TLV
    Unsigned32              mangAddr;
    char                    systemDesc[LLDP_SYS_DESC_LEN];
#endif
} PN_LLDP_NVS_T;


typedef struct
{
    Unsigned16              rtClass2PortStatus;
    Unsigned16              rtClass3PortStatus;
}
PNIO_PORT_STATUS_T;


typedef struct {
    Unsigned8               autoNegState;
    Unsigned16              speedBits;
    Unsigned16              mauType;
} LLDP_PHY_IF_T;


typedef struct {
    Unsigned16              holdTime;
    Unsigned16              txInterval;

#ifdef LLDP_OPT_TLV
    Unsigned32              systemCap;
    Unsigned8               objectId[LLDP_OBJECTID_LEN];
#endif

#ifdef LLDP_ORG_EXT
    PNIO_PORT_STATUS_T      portStatus;
    LLDP_PHY_IF_T           phyIf;
#endif
} LLDP_CONFIG_T;


/* list of public functions
-------------------------------------------------------------------------- */
IOD_STATUS_T PN_lldpInit(void);
IOD_STATUS_T PN_lldpGetPeer(unsigned int, unsigned int *, char **, char **, Unsigned8 **);
void PN_lldpRecv(OAL_BUFFER_T *);
void PN_lldpMgmtAddr(Unsigned8 *);
void PN_lldpShutdown(void);
IOD_STATUS_T PN_lldpEthUp(void);
IOD_STATUS_T PN_lldpEthDown(void);
void PN_lldpSetStateTx(unsigned int portIdx, PN_BOOL_T enable, PN_BOOL_T setFlag);


/* LLDP mapping macros
-------------------------------------------------------------------------- */
#define PN_LLDP_FUNC_INIT()             PN_lldpInit()
#define PN_LLDP_FUNC_RECV(...)          PN_lldpRecv(__VA_ARGS__)
#define PN_LLDP_FUNC_MGMTADDR(...)      PN_lldpMgmtAddr(__VA_ARGS__)
#define PN_LLDP_FUNC_GETPEER(...)       PN_lldpGetPeer(__VA_ARGS__)
#define PN_LLDP_FUNC_SHUTDOWN()         PN_lldpShutdown()
#define PN_LLDP_FUNC_ETH_UP()           PN_lldpEthUp()
#define PN_LLDP_FUNC_ETH_DOWN()         PN_lldpEthDown()
#define PN_LLDP_FUNC_SET_TX(...)        PN_lldpSetStateTx(__VA_ARGS__)
#define PN_LLDP_FUNC_GET_SYS_NAME(...)  PN_lldpGetSystemName(__VA_ARGS__)


#else

#  if defined(CONFIG_LLDP_SUPPORT)
#    if !defined(PN_LLDP_FUNC_INIT) || \
        !defined(PN_LLDP_FUNC_RECV) || \
        !defined(PN_LLDP_FUNC_MGMTADDR) || \
        !defined(PN_LLDP_FUNC_GETPEER) || \
        !defined(PN_LLDP_FUNC_SHUTDOWN) || \
        !defined(PN_LLDP_FUNC_ETH_UP) || \
        !defined(PN_LLDP_FUNC_ETH_DOWN)
#      error "Internal LLDP not used and LLDP macros not defined."
#    endif
#  else /* CONFIG_LLDP_SUPPORT */
#    define PN_LLDP_FUNC_INIT()             IOD_OK
#    define PN_LLDP_FUNC_RECV(...)
#    define PN_LLDP_FUNC_MGMTADDR(...)
#    define PN_LLDP_FUNC_GETPEER(x, y, ...) (*y = 0, IOD_NOTFOUND_ERR)
#    define PN_LLDP_FUNC_SHUTDOWN()
#    define PN_LLDP_FUNC_ETH_UP()
#    define PN_LLDP_FUNC_ETH_DOWN()
#  endif /* CONFIG_LLDP_SUPPORT */

#endif /* CONFIG_LLDP_INTERNAL */
#endif /* PN_LLDP_H */
