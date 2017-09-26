/** @file
 *
 * @brief
 * PROFINET Driver for LX4f
 *
 * @details
 * This module implements the PROFINET low level driver.
 *
 * @copyright
 * Copyright 2010-2016 port GmbH Halle/Saale.
 * This software is protected Intellectual Property and may only be used
 * according to the license agreement.
 */

#include <pn_includes.h>

#include "driverlib/flash.h"

#include "pn_oal_eth.h"

#include "hal.h"

PN_LOG_SET_ID(PN_LOG_ID_OAL)

/* TCP/IP stack */
//#include <tcpip_pnio_oal.h>


/* external variables
---------------------------------------------------------------------------*/
#if defined(CONFIG_LOGGING) && defined(CONFIG_LOGGING_TARGET_RAW)
extern Unsigned8 uart_buf[UARTBUF_SIZE]; /* buffer the UART interrupt can send from */
extern Integer32 uart_rd; /* points to last sent character in uart_buf */
extern Integer32 uart_wr; /* points to first free slot in uart_buf */
extern Unsigned8 uart_sending; /* flag indicating the UART0 Interrupt will send the next character */
#endif /* defined(CONFIG_LOGGING) && defined(CONFIG_LOGGING_TARGET_RAW) */

#ifdef CONFIG_DEBUG_SOFTSCOPE
extern Unsigned32 g_ui32ClkFreq;    /**< system clock frequency in Hz */
#endif

extern uint32_t g_sys_clock;

/** OAL Initialization
 *
 * This function initialises the OAL and provides unified interfaces to the PROFINET stack.
 *
 * @retval IOD_OK - success
 * @retval other - failed
 */
IOD_STATUS_T OAL_init(void)
{
    /* initialize target */
//    TARGET_init();

    return IOD_OK;
}


/** Read the Ethernet Medium Attachment Unit Status
 *
 * Returns the set or detected MAU type like 100BASETXFD.
 *
 * @retval IOD_OK successful
 * @retval other failed
 */
IOD_STATUS_T OAL_ethMAUType(
    unsigned int portIdx,                       /**< interface index */
    Unsigned16 *MAUType                         /**< pointer to store MAU type */
)
{
    Unsigned16 status;                          /**< PHY status */

    UNUSEDARG(portIdx);

    /*
     * Read the Ethernet PHY Basic Mode Status Register. 
     * The Addres 0 is used for the internal PHY.
     */
    status = MAP_EMACPHYRead(EMAC0_BASE, 0, EPHY_BMSR);

    if (EPHY_BMSR_100BTXFD & status) {
        *MAUType =  PNIO_MAUTYPE_100TXFD;
    } else if (EPHY_BMSR_10BTFD & status) {
        *MAUType = PNIO_MAUTYPE_10TXFD;
    } else if (EPHY_BMSR_100BTXHD & status) {
        *MAUType =  PNIO_MAUTYPE_100TXHD;
    } else if (EPHY_BMSR_10BTHD & status) {
        *MAUType = PNIO_MAUTYPE_10TXHD;
    } else {
        *MAUType = PNIO_MAUTYPE_RESERVED;
    }

    return IOD_OK;
}


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
)
{
    UNUSEDARG(portIdx);
    UNUSEDARG(mauType);
    UNUSEDARG(setFlag);

    return IOD_NOT_SUPPORTED;
}


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
)
{
    Unsigned16 status;

    UNUSEDARG(portIdx);

    /* Read the Ethernet PHY Basic Mode Status Register.
     * Address 0 is the internal PHY
     */
    status = MAP_EMACPHYRead(EMAC0_BASE, 0, EPHY_BMSR);
    *portState = (EPHY_BMSR_LINKSTAT & status) ? PNIO_LINKSTATE_UP : PNIO_LINKSTATE_DOWN;

    return IOD_OK;
}


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
)
{
    UNUSEDARG(portIdx);
    UNUSEDARG(portState);
    UNUSEDARG(setFlag);

    return IOD_NOT_SUPPORTED;
}


/** Perform Periodical Stack Tasks
 *
 * This function must be called periodically on non-threaded architectures to
 * perform internal stack tasks like querying the TCP/IP stack for new frames,
 * checking timers etc.
 */
void OAL_stack_exec(void)
{
    OAL_handleNonRT();
    // pnio background loop
    PN_pnioLoop();
}


/** Global Stack Halt Function
 *
 * This functions makes it easier to focus on one breakpoint while debugging.
 * See the stack for details of the caller.
 */
void OAL_halt(void)
{
    volatile unsigned int loop = 0;
    for (;; loop += 1);
}


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
)
{
    Unsigned8 *p_string = (Unsigned8 *) str;
    unsigned int i;

    /* 
     * - copy message into uart_buf
     * - uart_wr points to empty slot
     * - uart_fr points to last read slot
     */

    MAP_IntDisable(INT_UART0);

    for (i=0; i < len; i++) {

        /* check if there is still a free slot */
        if (uart_wr==uart_rd) 
        {
            /* no free slot left, drop (rest of) message */
            break;
        }

        uart_buf[uart_wr]= *p_string;

        /* update pointers */
        p_string++;
        uart_wr++;

        if (uart_wr == UARTBUF_SIZE)
        {
            /* we reached the end of the buffer, wrap around */
            uart_wr=0;
        }
    }

    MAP_IntEnable(INT_UART0);

    /* check if we will have to start the first transmission */
    if (uart_sending == 0)
    {
        uart_sending=1;

        /* increment rd pointer to next unsent element*/
        (uart_rd+1 == UARTBUF_SIZE) ? uart_rd=0 : uart_rd++;

        UART0_DR_R=uart_buf[uart_rd];
    }
}
#endif /* CONFIG_LOGGING_TARGET_RAW */


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
)
{
    char confMacAddr[] = {MAC0,MAC1,MAC2,MAC3,MAC4,MAC5};   /**< MAC address from configuration */
//    printf("%x\n",MAC0);
//    printf("%x\n",MAC1);
//    printf("%x\n",MAC2);
//    printf("%x\n",MAC3);
//    printf("%x\n",MAC4);
//    printf("%x\n",MAC5);
    UNUSEDARG(portIdx);

    OAL_MEMCPY(macAddr, confMacAddr, MAC_ADDR_LEN);
    return IOD_OK;
}


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
)
{
    OAL_MEMCPY(data, (void *) OAL_NVM_BASE, size);

    return IOD_OK;
}


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
)
{
    IOD_STATUS_T retVal = IOD_OK;   /**< return value */
    int res = 0;                    /**< flash results */

    /* make sure we get no interrupts */
    OAL_lockGet(0, PN_LOCK_INFINITE);

    /* erase flash block */
    res = MAP_FlashErase(OAL_NVM_BASE);
    if (0 != res) {
        PN_logErr("Could not erase flash block");
        retVal = IOD_NVS_WRITE_FAILED;
    }

    /* write data to flash */
    if (IOD_OK == retVal) {
        res = MAP_FlashProgram(data, OAL_NVM_BASE, size);
        if (0 != res) {
            PN_logErr("Could not write to flash");
            retVal = IOD_NVS_WRITE_FAILED;
        }
    }

    /* reenable interrupts */
    OAL_lockPut(0);

    return retVal;
}


/** Shutdown the OAL module
 *
 * Should do all necessary steps to shutdown the OAL - on some architectures,
 * this halts the device.
 */
CONFIG_NORETURN
void OAL_shutdown(void)
{
    OAL_halt();
}


/** Set Interface Name
 *
 * Set the interface name for this target (eg. "eth0").
 */
void OAL_setInterface(
    const char *name                /**< interface name */
)
{
    UNUSEDARG(name);
}


/** Get Interface Name
 *
 * Get the interface name for this target (eg. "eth0").
 */
const char * OAL_getInterface(void)
{
    return "";
}


/** Get Flash Size
 *
 * Get the flash size that can be used to store values for this target.
 */
unsigned int OAL_nvsGetSize(void)
{
    return OAL_NVM_SIZE;
}


/** Read the available Buttons
 *
 * Return the state of the available buttons as bits. If a bit is set, it means
 * the button is pressed.
 *
 * @returns Unsigned32 button status bit field
 */
Unsigned32 OAL_getButtons(void)
{
    return 0;
}


/** Set the available LEDS
 *
 * Set the state of the available LEDs. If a bit is set, the available LED is
 * switched on.
 */
void OAL_setLeds(
    Unsigned32 leds             /**< LED bit field */
)
{
    UNUSEDARG(leds);
}


/** Main Function
 *
 * The function that is first called by the C library.
 * It has to make sure that main_appl is called at the end.
 */
//int main(void)
//{
//    main_appl();
//    return 0;
//}


/** Get Current Tick Count  - 64 Bit
 *
 * Returns the current tick counter in ms as PN_TIMESTAMP_T.
 * The 64 bit variant will overflow in 584942417 years.
 *
 * @retval PN_TIMESTAMP_T 64 bit timestamp structure
 */
PN_TIMESTAMP_T OAL_getTimestamp(
    void
)
{
    PN_TIMESTAMP_T ts;                          /**< timestamp */

    ts.ts = g_ulTickCounter;
    return ts;
}


/** Get the Boottime in Seconds
 *
 * Returns the time since the system booted in seconds.
 */
Unsigned32 OAL_getBoottime(
    void
)
{
    return 0;
}


#ifdef CONFIG_DEBUG_SOFTSCOPE
/** Get Timestamp Difference to last Call in us
 *
 * Returns the current timestamp difference in us. If the timestamp difference
 * exceeds 32 bit, it returns 2^32 - 1.
 */
Unsigned32 OAL_getTimestampDiffUs(
    void
)
{
    static int first = 1;
    static Unsigned32 cur = 0;
    static Unsigned32 last = 0;
    static Unsigned32 ret = 0;
    static Unsigned32 period = 0;

    if (!first) {
        /* time in us */
        cur = g_ulTickCounter + ((MAP_SysTickValueGet() * 1000) / period);

        /* calculate difference between current and last timestamp */
        ret = (last <= cur) ? (cur - last) : 0xffffffff;
        last = cur;
    } else {
        /* just update last timestamp on first call */
        period = g_ui32ClkFreq / 1000;
        first = 0;
        last = g_ulTickCounter + ((MAP_SysTickValueGet() * 1000) / period);
    }

    return ret;
}
#endif /* CONFIG_DEBUG_SOFTSCOPE */


/***************************************************************************/
/** Add multicast address to device
 *
 * Configure the receiption of a specific multicast address.
 */
IOD_STATUS_T OAL_ethMulticastAdd(
    PN_BOOL_T extFlag,                      /**< external ports flag */
    char *mcAddr                            /**< multicast address */
)
{
    UNUSEDARG(extFlag);
    UNUSEDARG(mcAddr);
    return IOD_OK;
}


/***************************************************************************/
/** Remove multicast address from device
 *
 * Remove a configured multicast address.
 */
IOD_STATUS_T OAL_ethMulticastDel(
    char *mcAddr                            /**< multicast address */
)
{
    UNUSEDARG(mcAddr);
    return IOD_OK;
}


/***************************************************************************/
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
)
{
    UNUSEDARG(id);
    UNUSEDARG(prio);

    return IOD_OK;
}


/***************************************************************************/
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
)
{
    UNUSEDARG(id);
    UNUSEDARG(type);
    UNUSEDARG(period);

    return IOD_OK;
}


/***************************************************************************/
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
)
{
    UNUSEDARG(id);

    return IOD_OK;
}


/***************************************************************************/
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
)
{
    UNUSEDARG(id);

    return IOD_OK;
}
