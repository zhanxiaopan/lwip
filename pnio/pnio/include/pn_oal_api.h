

#ifndef PN_OAL_API_H
#define PN_OAL_API_H

#include <stdint.h>
#include <pn_oal.h>


/* types */
typedef struct {
    Unsigned32      localIp;        /**< local IP address */
    Unsigned16      localPort;      /**< local port number */
    Unsigned32      remoteIp;       /**< remote IP address */
    Unsigned16      remotePort;     /**< remote port number */
} OAL_UDP_ADDR_T;

typedef void (*OAL_UDP_RECV_CB_T)(
    int             chan,           /**< channel descriptor */
    OAL_UDP_ADDR_T  *pAddr,         /**< connection data */
    void            *buf,           /**< buffer */
    unsigned int    bufLen          /**< buffer length */
);


/** OAL Initialization - Init Stage
 *
 * This function initializes the OAL and provides unified interfaces to the
 * PROFINET stack.
 *
 * @retval IOD_OK - success
 * @retval other - failed
 */
IOD_STATUS_T OAL_init(void);


/** Ethernet Handling Initialization
 *
 * This function initializes the Ethernet handling.
 */
IOD_STATUS_T OAL_ethInit(
    unsigned char *macAddr,          /**< MAC address */
    uint32_t ip,
    uint32_t nm,
    uint32_t gw
);


/** Ethernet Handling Shutdown
 *
 * This function shuts the Ethernet handling down.
 */
void OAL_ethShutdown(void);


/** Global Stack Halt Function
 *
 * This functions makes it easier to focus on one breakpoint while debugging.
 * See the stack for details of the caller.
 */
//CONFIG_NORETURN_PRE void OAL_halt(void) CONFIG_NORETURN;
CONFIG_NORETURN_PRE void OAL_halt(uint32_t index) CONFIG_NORETURN;

/** Shutdown the OAL module
 *
 * Should do all necessary steps to shutdown the OAL - on some architectures,
 * this halts the device.
 */
void OAL_shutdown(void);


/** Set Interface Name
 *
 * Set the interface name for this target (eg. "eth0").
 */
void OAL_setInterface(
    const char *name                /**< interface name */
);


/** Get Interface Name
 *
 * Get the interface name for this target (eg. "eth0").
 */
const char * OAL_getInterface(void);


/** Read MAC address from Interface
 *
 * The MAC address buffer macAddr needs to have at least MAC_ADDR_LEN bytes.
 *
 * @retval IOD_OK - successful
 * @retval other - failed
 */
IOD_STATUS_T OAL_getMacAddr(
    unsigned int portIdx,           /**< port index */
    char *macAddr                   /**< buffer to store MAC addr */
);


/** Perform Periodical Stack Tasks
 *
 * This function must be called periodically on non-threaded architectures to
 * perform internal stack tasks like querying the TCP/IP stack for new frames,
 * checking timers etc.
 */
void OAL_stack_exec(void);


/** Get Flash Size
 *
 * Get the flash size that can be used to store values for this target.
 */
unsigned int OAL_nvsGetSize(void);


/** Get the data stored in NVS
 *
 * The size of the NVS should be verified with \see OAL_nvsGetSize.
 *
 * @retval IOD_OK - successful
 * @retval other - failed
 */
IOD_STATUS_T OAL_nvsReadData(
    void *data,                     /**< pointer to store data */
    unsigned int size               /**< data size */
);


/** Store data in NVS
 *
 * The size of the NVS should be verified with \see OAL_nvsGetSize.
 *
 * @retval IOD_OK - successful
 * @retval other - failed
 */
IOD_STATUS_T OAL_nvsWriteData(
    void *data,                     /**< pointer to read data from */
    unsigned int size               /**< data size */
);


/** Create a Binary or Counting Lock
 *
 * A binary lock represents a mutex, where a counting lock represents a
 * semaphore. When a counting lock is created, the inital an maximal value can
 * be set. For example if 3 instances of a ressource are available and 2 should
 * be marked as already in use, set the valInit to 2 and the valMax to 3.
 *
 * It is not necessary to set pLock->type to lockType, because this will be
 * done in pn_lock.c.
 *
 * @return IOD_OK - success
 * @return other - fail
 */
IOD_STATUS_T OAL_lockCreate(
    PN_LOCK_TYPE_T lockType,        /**< PN_LOCK_BINARY or PN_LOCK_COUNT */
    PN_LOCK_T *pLock,               /**< pointer to store the lock data */
    Unsigned32 valInit,             /**< initial lock value */
    Unsigned32 valMax               /**< maximal lock value */
);


/** Aquire a Lock
 *
 * Enter a binary or counting lock.
 * If this is not possible, the function will sleep until the timeout (in ms)
 * is reached. Setting the timeout to zero, will result in an infinite wait.
 *
 * Make sure the lock can be called recursively by the same thread (or when in
 * IRQ context, by the currently running IRQ). This is for example necessary,
 * because the timer handler locks the timer structure while calling the timer
 * callback and the timer callback must be able to create new or stop active
 * timers.
 *
 * @returns IOD_OK - success
 * @returns IOD_LOCK_GET_TIMEOUT_FAIL - timeout reached
 * @returns other - fail
 */
IOD_STATUS_T OAL_lockGet(
    PN_LOCK_T *pLock,               /**< lock data */
    Unsigned32 timeout              /**< lock timeout */
);


/** Release a Lock
 *
 * Leave a binary or counting lock.
 *
 * @returns IOD_OK - success
 * @returns other - fail
 */
IOD_STATUS_T OAL_lockPut(
    PN_LOCK_T *pLock                /**< lock data */
);


/** Free Lock Resources
 *
 * Cleans up the locks resources.
 *
 * @returns IOD_OK - success
 * @returns other - fail
 */
IOD_STATUS_T OAL_lockDelete(
    PN_LOCK_T *pLock                /**< lock data */
);


/** Open an UDP Channel
 *
 * Open an UDP channel on the specified port.
 *
 * @returns IOD_OK - success
 * @returns other - fail
 */
#ifndef PORT_GMBH_GOAL
IOD_STATUS_T OAL_udpOpen(
    int *pSockIdx,                      /**< pointer to store socket index */
    Unsigned16 port,                    /**< UDP port number */
    OAL_UDP_RECV_CB_T callback          /**< receive callback */
);
#else
IOD_STATUS_T OAL_udpOpen(
    int *pSockIdx,                      /**< pointer to store socket index */
    Unsigned16 port,                    /**< UDP port number */
    GOAL_NET_RECV_CB_T callback         /**< receive callback */
);
#endif


/** Close an UDP Channel
 *
 * Close the specified UDP channel handle.
 *
 * @returns IOD_OK - success
 * @returns other - fail
 */
IOD_STATUS_T OAL_udpClose(
    int *pConn                      /**< UDP channel handle */
);


/** Reopen an UDP Channel
 *
 * Reopen an UDP channel on the specified port.
 *
 * @returns IOD_OK - success
 * @returns other - fail
 */
IOD_STATUS_T OAL_udpReopen(
    void
);


/** Send Data over UDP Channel
 *
 * Send data in pBuf with bufLen size over the UDP channel to the destination
 * address given in pDA.
 *
 * @returns IOD_OK - success
 * @returns other - fail
 */
IOD_STATUS_T OAL_udpSend(
    int                 chan,       /**< connection handle */
    OAL_UDP_ADDR_T     *pAddr,      /**< UDP address */
    Unsigned8          *pBuf,       /**< send buffer */
    Unsigned16          bufLen      /**< send buffer size */
);


/** Generic Ethernet Send Function for PROFINET Buffers
 *
 * This function is called when an Ethernet frame that is embedded in a
 * PROFINET buffer is ready to send.
 */
IOD_STATUS_T OAL_ethSendAndRelease(
    void **ppOAL,                       /**< pointer to PROFINET buffer */
    PN_NET_TX_TYPE_T type               /**< frame channel type */
);


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
);


/** Read the Ethernet Medium Attachment Unit Status
 *
 * Returns the set or detected MAU type like 100BASETXFD.
 *
 * @returns IOD_OK - success
 * @returns other - fail
 */
IOD_STATUS_T OAL_ethMAUType(
    unsigned int portIdx,       /**< interface index */
    Unsigned16 *MAUType         /**< pointer to store MAU type */
);


/** Read the Ethernet Port State
 *
 * Returns if the interface is up or down.
 *
 * See specification 6.2.12.16 Coding of the field LinkState.
 *
 * @returns IOD_OK - success
 * @returns other - fail
 */
IOD_STATUS_T OAL_ethPortState(
    unsigned int portIdx,       /**< interface index */
    Unsigned16 *portState       /**< pointer to store port state */
);


/** Set the Ethernet Port State
 *
 * Set the given Ethernet port to a given link state.
 * If setFlag isn't set to PN_TRUE the default state must be resetted.
 *
 * @returns IOD_OK - success
 * @returns IOD_NOT_SUPPORTED - unsupported or failed
 */
IOD_STATUS_T OAL_ethPortStateSet(
    unsigned int portIdx,       /**< interface index */
    Unsigned16 portState,       /**< port state */
    PN_BOOL_T setFlag           /**< set flag */
);


/** Set the Ethernet Medium Attachment Unit Status
 *
 * Sets the MAU type like 100BASETXFD.
 * If setFlag isn't set to PN_TRUE the default state must be resetted.
 *
 * @returns IOD_OK - success
 * @returns IOD_NOT_SUPPORTED - unsupported or failed
 */
IOD_STATUS_T OAL_ethMAUTypeSet(
    unsigned int portIdx,       /**< interface index */
    Unsigned16 mauType,         /**< MAU type */
    PN_BOOL_T setFlag           /**< set flag */
);


/** Init Timer
 *
 * Initializes the OAL timer management.
 *
 * @returns IOD_OK - success
 * @returns IOD_TIMER_INIT_FAILED - error initializing timers
 */
IOD_STATUS_T OAL_timerInit(
    void
);


/** Create Timer
 *
 * Creates a timer for the given timer ID with the given priority.
 * Timer ID range: 0..(CONFIG_TIMER_MAX - 1)
 *
 * @returns IOD_OK - success
 * @returns IOD_TIMER_CREATE_FAILED - error creating timer
 */
IOD_STATUS_T OAL_timerCreate(
    PN_TIMER_ID_T id,           /**< timer ID */
    PN_TIMER_PRIO_T prio        /**< PN_TIMER_PRIO_LOW, PN_TIMER_PRIO_HIGH */
);


/** Setup Timer
 *
 * Setup a timer.
 * Timer ID range: 0..(CONFIG_TIMER_MAX - 1)
 *
 * @returns IOD_OK - success
 * @returns IOD_TIMER_SETUP_FAILED - error setting up timer
 */
IOD_STATUS_T OAL_timerSetup(
    PN_TIMER_ID_T id,           /**< timer ID */
    PN_TIMER_TYPE_T type,       /**< timer type: single or periodic */
    Unsigned32 period           /**< period in ms after which timer is triggered */
);


/** Start Timer
 *
 * Start a timer.
 * Timer ID range: 0..(CONFIG_TIMER_MAX - 1)
 *
 * @returns IOD_OK - success
 * @returns IOD_TIMER_START_FAILED - error starting timer
 */
IOD_STATUS_T OAL_timerStart(
    PN_TIMER_ID_T id            /**< timer ID */
);


/** Stop Timer
 *
 * Stop a timer.
 * Timer ID range: 0..(CONFIG_TIMER_MAX - 1)
 *
 * @returns IOD_OK - success
 * @returns other - fail
 */
IOD_STATUS_T OAL_timerStop(
    PN_TIMER_ID_T id            /**< timer ID */
);


/** Read the available Buttons
 *
 * Return the state of the available buttons as bits. If a bit is set, it means
 * the button is pressed.
 *
 * @returns Unsigned32 button status bit field
 */
Unsigned32 OAL_getButtons(
    void
);


/** Read state of available LEDS
 *
 * Get the state of the available LEDs. If a LED is on, the corresponding bit
 * is set to 1.
 */
Unsigned32 OAL_getLeds(
    void
);


/** Set the available LEDS
 *
 * Set the state of the available LEDs. If a bit is set, the available LED is
 * switched on.
 */
void OAL_setLeds(
    Unsigned32 leds             /**< LED bit field */
);


/** Print Message
 *
 * If logging is enabled, this function is called by the logging module to
 * print the message over serial, stdout or other ways like logfiles. It's up
 * to this function to add carriage returns and linefeeds.
 */
#ifdef CONFIG_LOGGING_TARGET_RAW
void OAL_msgRaw(
    const char *str,            /**< log string */
    unsigned int len            /**< string length */
);
#endif /* CONFIG_LOGGING_TARGET_RAW */


/** Output Syslog Message via Ethernet
 *
 * This function gets the syslog buffer and should transmit it 1:1 to the
 * ethernet. If the buffer can't be transmitted and a TX buffer is available,
 * the content must be copied instead of just storing the buffer pointer -
 * otherwise the next log message would kill the buffers content.
 */
#ifdef CONFIG_LOGGING_TARGET_SYSLOG
void OAL_msgSyslog(
    const char *str,            /**< log string */
    unsigned int len            /**< string length */
);
#endif /* CONFIG_LOGGING_TARGET_SYSLOG */


/** Get Current Tick Count as 64 Bit Value
 *
 * Returns the current tick counter in ms as PN_TIMESTAMP_T.
 * The 64 bit timestamp will overflow in 584942417 years.
 *
 * @retval PN_TIMESTAMP_T 64 bit timestamp structure
 */
PN_TIMESTAMP_T OAL_getTimestamp(
    void
);


/** Get Timestamp Difference to last Call in us
 *
 * Returns the current timestamp difference in us. If the timestamp difference
 * exceeds 32 bit, it returns 2^32 - 1.
 */
Unsigned32 OAL_getTimestampDiffUs(
    void
);


/** Get the Boottime in Seconds
 *
 * Returns the time since the system booted in seconds.
 */
Unsigned32 OAL_getBoottime(
    void
);


/** Release an OS specific Buffer
 *
 * Calls a OS/hardware specific function to release the buffer. This function
 * is called on every OAL_releaseBuffer call, even if the PNIO buffer isn't
 * released because its refcount is not 1.
 */
void OAL_releaseOSBuf(
    void *pkt                   /**< pointer to OS buffer */
);


/** Add multicast address to device
 *
 * Configure the forwarding of a specific multicast address. If extFlag is set,
 * Multicast is configured to be forwarded on all external ports and to the
 * internal port, otherwise only the internal port sees the frames.
 */
IOD_STATUS_T OAL_ethMulticastAdd(
    PN_BOOL_T extFlag,                      /**< external ports flag */
    char *mcAddr                            /**< multicast address */
);


/** Remove multicast address from device
 *
 * Remove a configured multicast address.
 */
IOD_STATUS_T OAL_ethMulticastDel(
    char *mcAddr                            /**< multicast address */
);


/***************************************************************************/
/** Get Ethernet Frame MTU Size
 *
 * Returns the maximal transmission unit size for Ethernet frames.
 *
 * \retval Ethernet MTU Size
 */
Unsigned16 OAL_ethGetMtu(
    void
);


/* Available Defines to be set in pn_oal.h
 * =======================================
 *
 * Network and Frame Handling Defines
 * ----------------------------------
 *
 * CONFIG_TARGET_MEM_RELEASE_FUNC - enable callback for released buffers (optional)
 * CONFIG_TARGET_ETH_PORT_COUNT - number of available Ethernet ports
 *
 * Lock Handling Defines
 * ---------------------
 * CONFIG_TARGET_LOCK_INT_ENABLE - command to enable interrupts
 * CONFIG_TARGET_LOCK_INT_DISABLE - command to disable interrupts
 */

#endif /* PN_OAL_API_H */
