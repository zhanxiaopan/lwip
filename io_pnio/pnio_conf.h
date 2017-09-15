

#ifndef PNIO_CONF_H
#define PNIO_CONF_H


/***************************************************************************
 *
 * Resource Management
 *
 ***************************************************************************/

/* CONFIG_MAX_PART
 *
 * Adjust pre-allocated memory for internal structures.
 *
 * Currently, only application process 0 is supported, so changing
 * CONFIG_MAX_AP_COUNT from 1 to a higher value will lead to an undefined
 * behaviour.
 *
 * CONFIG_MAX_AP_COUNT            - Default:  1, application process count
 * CONFIG_MAX_SLOT_COUNT          - Default:  3, overall slot count
 * CONFIG_MAX_SUBSLOT_COUNT       - Default:  6, overall subslot count
 * CONFIG_MAX_MODULE_COUNT        - Default: 10, overall module count
 * CONFIG_MAX_SUBMOD_COUNT        - Default:  6, overall submodule count
 */
#define CONFIG_MAX_AP_COUNT           1
#define CONFIG_MAX_SLOT_COUNT         3
#define CONFIG_MAX_SUBSLOT_COUNT      6
#define CONFIG_MAX_MODULE_COUNT      10
#define CONFIG_MAX_SUBMOD_COUNT       6


/* CONFIG_MAX_AR_COUNT
 *
 * Number of application relations.
 *
 * Default: 2
 */
#define CONFIG_MAX_AR_COUNT 2


/* CONFIG_MAX_CR_COUNT
 *
 * Number of communication relations for an AR.
 *
 * Default: 10
 */
#define CONFIG_MAX_CR_COUNT 10


/* CONFIG_MAX_IOCR_BLOCKS_COUNT
 *
 * Number of IO communication releations in a connect request.
 *
 * Default: 10
 */
#define CONFIG_MAX_IOCR_BLOCKS_COUNT 10


/* CONFIG_MAX_PARAM_COUNT
 *
 * Number of device parameters in application storage.
 *
 * CONFIG_NUM_DEVICE_PARAMS is a reference to this define and will be removed soon.
 *
 * Default: 16
 */
#define CONFIG_MAX_PARAM_COUNT 16
#define CONFIG_NUM_DEVICE_PARAMS CONFIG_MAX_PARAM_COUNT


/* CONFIG_MAX_DIAGBUFFER_COUNT
 *
 * Number of diagnose buffers.
 *
 * Default: 20
 */
#define CONFIG_MAX_DIAGBUFFER_COUNT 20


/* CONFIG_MAX_DIAG_DATA_SIZE
 *
 * Data size per diagnose buffer in byte.
 *
 * Default: 4
 */
#define CONFIG_MAX_DIAG_DATA_SIZE 4


/***************************************************************************
 *
 * RPC
 *
 ***************************************************************************/

/* CONFIG_RPC_NUM_SESSIONS
 *
 * Number of RPC sessions.
 *
 * Default: 8
 */
#define CONFIG_RPC_NUM_SESSIONS 8


/* RPC_FRAG_SUPPORT
 *
 * RPC fragmentation support.
 *
 * Default: enabled
 * Disable: comment out
 */
#define RPC_FRAG_SUPPORT 1


/***************************************************************************
 *
 * Record Data Handling
 *
 ***************************************************************************/

/* CONFIG_RECORD_DATA_BUSY
 *
 * Store record data requests for later processing when the response is
 * available.
 *
 * Default: disabled
 * Enable:  remove commentation
 */
/* #define CONFIG_RECORD_DATA_BUSY 1 */


/* CONFIG_RECORD_DATA_BUSY_BUFSIZE
 *
 * Sets the count of busy record data request that can be hold in
 * memory. At least 2 requests are recommended.
 *
 * Default: 2
 */
#ifdef CONFIG_RECORD_DATA_BUSY
#  define CONFIG_RECORD_DATA_BUSY_BUFSIZE 2
#endif


/***************************************************************************
 *
 * I&M Data Handling
 *
 ***************************************************************************/

/* CONFIG_IM_0_CALLBACK
 *
 * Handle I&M0 data read via application callback IOD_CB_READ_RECORD.
 *
 * Default: disabled
 * Enable:  remove commentation
 */
/* #define CONFIG_IM_0_CALLBACK 1 */


/* CONFIG_IM_0_FILTER_CALLBACK
 *
 * Handle I&M0 filter data via application callback IOD_CB_READ_RECORD.
 *
 * Default: disabled
 * Enable:  remove commentation
 */
/* #define CONFIG_IM_0_FILTER_CALLBACK 1 */


/* CONFIG_IM_1_4_SUPPORT
 *
 * Support I&M1, I&M2, I&M3 and I&M4 data. If CONFIG_IM_1_4_CALLBACK isn't set
 * the data is handled stack internally.
 *
 * Default: enabled
 * Disable: comment out
 */
#define CONFIG_IM_1_4_SUPPORT 1


/* CONFIG_IM_1_4_CALLBACK
 *
 * Handle I&M1, I&M2, I&M3 and I&M4 data via user callback.
 *
 * Default: enabled
 * Disable: comment out
 * Requires: CONFIG_IM_1_4_SUPPORT
 */
/* #define CONFIG_IM_1_4_CALLBACK 1 */


/***************************************************************************
 *
 * LLDP Handling
 *
 ***************************************************************************/

/* CONFIG_LLDP_SUPPORT
 *
 * Enable LLDP support in the PROFINET stack for internal or external LLDP
 * handling.
 *
 * Default: enabled
 * Disable: comment out
 */
#define CONFIG_LLDP_SUPPORT 1


/* CONFIG_LLDP_INTERNAL
 *
 * Enable the internal LLDP module.
 *
 * Requires: CONFIG_LLDP_SUPPORT
 * Default: enabled
 * Disable: comment out
 */
#define CONFIG_LLDP_INTERNAL 1


/* CONFIG_LLDP_GENERATE_MAC
 *
 * Generate the LLDP specific port MAC address by adding the port index plus
 * one to the device MAC address.
 *
 * Example:
 *   Device MAC: 01:02:03:04:05:06
 *   Port 1 MAC: 01:02:03:04:05:07
 *   Port 2 MAC: 01:02:03:04:05:08
 *
 * Warning: Replace this demonstration function with an OAL implementation
 *          OAL_getMacAddrLldp if your final device will have a other MAC
 *          address source.
 *
 * Requires: CONFIG_LLDP_SUPPORT && CONFIG_LLDP_INTERNAL
 * Default:  enabled
 * Disable:  comment out
 */
#define CONFIG_LLDP_GENERATE_MAC 1


/* CONFIG_LLDP_RX_ENABLED
 *
 * Enable the receiption of LLDP frames and embed the information into the
 * LLDP frames send from the device.
 *
 * Default: enabled
 * Disable: comment out
 */
#define CONFIG_LLDP_RX_ENABLED 1


/* CONFIG_NO_LLDP_IN_VLAN
 *
 * Disables processing of LLDP frames with VLAN tag.
 *
 * Platform: Linux
 * Default:  enabled
 * Disable:  comment out
 */
#define CONFIG_NO_LLDP_IN_VLAN 1


/* LLDP_OPT_TLV
 *
 * Enables sending of the following optional LLDP parameters:
 *  - port description
 *  - system name
 *  - system description
 *  - system cap
 *  - management address
 *  - object ID
 *
 * Default: enabled
 * Disable: comment out
 */
#define LLDP_OPT_TLV 1


/* LLDP_ORG_EXT
 *
 * Enables sending of the following PROFINET specific LLDP parameters:
 *  - port status
 *  - RTC Class 2 port status (unused)
 *  - RTC Class 3 port status (unused)
 *  - chassis MAC
 *  - MAC PHY config status
 *
 * Default: enabled
 * Disable: comment out
 */
#define LLDP_ORG_EXT 1


/***************************************************************************
 *
 * SNMP Handling
 *
 ***************************************************************************/

/* CONFIG_SNMP_SUPPORT
 *
 * Enable SNMP support in the PROFINET stack for internal or external SNMP
 * handling.
 *
 * Default: disabled
 * Enable: remove commentation
 */
/* #define CONFIG_SNMP_SUPPORT 1 */


/* CONFIG_SNMP_INTERNAL
 *
 * Enable the internal SNMP module.
 *
 * Requires: CONFIG_SNMP_SUPPORT
 * Default: disabled
 * Enable: remove commentation
 */
/* #define CONFIG_SNMP_INTERNAL 1 */


/***************************************************************************
 *
 * Miscellaneous
 *
 ***************************************************************************/


/* CONFIG_NEW_IODATA_CB
 *
 * Enable the callback IOD_newIODataInd to signalise that new data is
 * available.
 *
 * Default: enabled
 * Disable: comment out
 */
#define CONFIG_NEW_IODATA_CB 1



/* CONFIG_MAX_TIMERS
 *
 * Set the count of available low and high priority timers.
 *
 * Default: 16
 */
#define CONFIG_MAX_TIMERS_LOW 24
#define CONFIG_MAX_TIMERS_HIGH 8


/* CONFIG_NET_LINK_SAFETY
 *
 * With PDPortDataAdjust all Ethernet ports can be permanently disabled. If
 * this define is enabled, at least one link will always stay enabled.
 *
 * Default: enabled
 * Disable: add comment
 */
#define CONFIG_NET_LINK_SAFETY 1


/***************************************************************************
 *
 * Fast Start Up (FSU)
 *
 ***************************************************************************/

/* CONFIG_MAX_DCP_HELLO (Fast Start Up)
 *
 * WARNING: This feature is currently disabled.
 *
 * Configure how many DCP hello messages should be send out at the device
 * start. This feature is used for Fast Start Up.
 * To disable Fast Start Up, set the number to zero.
 *
 * Default: 0
 * Enable:  set to 4
 */
#define CONFIG_MAX_DCP_HELLO 0


/***************************************************************************
 *
 * PROFIsafe
 *
 ***************************************************************************/

/* CONFIG_PROFISAFE
 *
 * Enable frame send notification callback which is necessary for implementing
 * PROFIsafe.
 *
 * Default: disabled
 * Enable:  remove commentation
 */
/* #define CONFIG_PROFISAFE 1 */


/* CONFIG_PROFISAFE_PDU_MAX_LEN
 *
 * Maximum size of the PDU.
 *
 * Default:  1440
 * Requires: CONFIG_PROFISAFE
 */
#define CONFIG_PROFISAFE_PDU_MAX_LEN 1440


/***************************************************************************
 *
 * Device and Vendor Settings
 *
 ***************************************************************************/


/* DEVICE_MAC_ADDR
 *
 * Device MAC address - only applies if no other way of reading the MAC address
 * is available. For example on Linux, the MAC address is read directly from
 * the ethernet device. On Windows the MAC address is used to select the
 * corresponding interface.
 *
 * Default value:
 *   Device MAC address: 00:45:56:78:9A:BC
 */
#define _LOCAL_STATIC_MAC_ADDR_INDEX (2)
#if _LOCAL_STATIC_MAC_ADDR_INDEX == 1
/* MAC ADDR #1*/
#define DEVICE_MAC_ADDR { 0x00, 0x45, 0x56, 0x78, 0x9a, 0xbc }
#elif _LOCAL_STATIC_MAC_ADDR_INDEX == 2
/* MAC ADDR #2*/
#define DEVICE_MAC_ADDR { 0x00, 0x45, 0x56, 0x78, 0x9a, 0xbd }
#elif _LOCAL_STATIC_MAC_ADDR_INDEX == 3
/* MAC ADDR #3*/
#define DEVICE_MAC_ADDR { 0x00, 0x45, 0x56, 0x78, 0x9a, 0xbe }
#elif _LOCAL_STATIC_MAC_ADDR_INDEX == 4
/* MAC ADDR #4*/
#define DEVICE_MAC_ADDR { 0x00, 0x45, 0x56, 0x78, 0x9a, 0xbf }
#elif _LOCAL_STATIC_MAC_ADDR_INDEX == 5
/* MAC ADDR #5*/
#define DEVICE_MAC_ADDR { 0x00, 0x45, 0x56, 0x78, 0x9a, 0xc0 }
#elif _LOCAL_STATIC_MAC_ADDR_INDEX == 6
/* MAC ADDR #6*/
#define DEVICE_MAC_ADDR { 0x00, 0x45, 0x56, 0x78, 0x9a, 0xc1 }
#endif



/* PORT1_ID, PORT1_INTERFACE
 *
 * Ethernet port names and internal interface specifier.
 * The internal interface specifier (PORTx_INTERFACE) doesn't apply to all
 * platforms.
 *
 * In the current implementation the last three characters of the port ID must
 * represent ASCII numbers. They are used to calculate the LLDP MAC address by
 * adding them to the DEVICE_MAC_ADDR.
 *
 * Default values:
 *   Port1 ID: "port-001"
 *   Port1 Interface: "eth1"
 */
#define PORT1_ID        "port-001"
#define PORT1_INTERFACE "eth1"


/* DEVICE_*
 *
 * Device information data. These values must match your GSDML file.
 *
 * DEVICE_VENDORID - Vendor ID
 *   - Unsigned16 value
 *   - use PI assigned number
 *
 * DEVICE_DEVICEID - Device ID
 *   - Unsigned16 value
 *   - vendor specific device ID
 *
 * DEVICE_PORT_DESC - Port Description
 *   - max 255 characters
 *   - LLDP: Port Description TLV
 *
 * DEVICE_SYSTEM_DESC - System Description
 *   - max 255 characters
 *   - LLDP: System Description TLV
 *
 * DEVICE_ORDERID - Order ID
 *   - max 20 characters
 *   - vendor specific
 *   - 4.10.2.2.2 - Coding of the field OrderID
 *   - GSDML: OrderNumber
 *
 * DEVICE_SERIALNUM - Serial Number
 *   - max 16 characters
 *   - 5.2.7.2 - Coding of the field IM_Serial_Number
 *
 * DEVICE_HWREV - Hardware Revision
 *   - Unsigned16 value
 *   - 5.2.7.3 - Coding of the field IM_Hardware_Revision
 *
 * DEVICE_SWREV_FUNCENH - Software Revision Functional Enhancement
 *   - Unsigned8 value
 *   - 5.2.7.4 - Coding of the field IM_SWRevision_Functional_Enhancement
 *
 * DEVICE_SWREV_BUGFIX - Software Revision Bug Fix
 *   - Unsigned8 value
 *   - 5.2.7.5 - Coding of the field IM_SWRevision_Bug_Fix
 *
 * DEVICE_SWREV_INTCHG - Software Revision Internal Change
 *   - Unsigned8 value
 *   - 5.2.7.6 - Coding of the field IM_SWRevision_Internal_Change
 *
 * DEVICE_SWREV_REVCNT - Revision Counter
 *   - Unsigned16 value
 *   - 5.2.7.7 - Coding of the field IM_Revision_Counter
 *
 * DEVICE_PROFILE_ID - Profile ID
 *   - Unsigned16 value
 *   - 5.2.7.8 - Coding of the field IM_Profile_ID
 *
 * DEVICE_PROFILE_SPECTYPE - Profile ID Specific Type
 *   - Unsigned16 value
 *   - 5.2.7.9 - Coding of the field IM_Profile_Specific_Type
 *
 * DEVICE_IM1_TAG_FUNC - Tag Function
 *   - 32 characters, left aligned, fill with blanks
 *   - factory default: empty
 *   - 5.2.7.13 - Coding of the field IM_Tag_Function
 *
 * DEVICE_IM1_TAG_LOC - Tag Location
 *   - 22 characters, left aligned, fill with blanks
 *   - factory default: empty
 *   - 5.2.7.14 - Coding of the field IM_Tag_Location
 *
 * DEVICE_IM2_DATE - Date
 *   - 16 characters, left aligned, fill with blanks
 *   - factory default: empty
 *   - 5.2.7.15 - Coding of the field IM_Date
 *
 * DEVICE_IM3_DESCRIPTOR - Descriptor
 *   - 54 characters, left aligned, fill with blanks
 *   - factory default: empty
 *   - 5.2.7.16 - Coding of the field IM_Descriptor
 *
 * DEVICE_IM4_SIGNATURE - Signature
 *   - 54 characters, left aligned, fill with blanks
 *   - factory default: empty
 *   - 5.2.7.17 - Coding of the field IM_Signature
 */
#define DEVICE_VENDORID               0x0ABC
#define DEVICE_DEVICEID               0x0001
#define DEVICE_PORT_DESC              "pnioTestPort"
#define DEVICE_SYSTEM_DESC            "PROFINET System"
#define DEVICE_ORDERID                "11111"
#define DEVICE_SERIALNUM              "23333"
#define DEVICE_HWREV                  0x0001
#define DEVICE_SWREV_PREFIX           PN_RPC_SWREV_PROTOTYPE
#define DEVICE_SWREV_FUNCENH          0x50
#define DEVICE_SWREV_BUGFIX           0x03
#define DEVICE_SWREV_INTCHG           0x18
#define DEVICE_SWREV_REVCNT           0x0000
#define DEVICE_PROFILE_ID             0xf600
#define DEVICE_PROFILE_SPECTYPE       0x0000
#define DEVICE_IM1_TAG_FUNC           "                                "
#define DEVICE_IM1_TAG_LOC            "                      "
#define DEVICE_IM2_DATE               "                "
#define DEVICE_IM3_DESCRIPTOR         "                                                      "
#define DEVICE_IM4_SIGNATURE          "                                                      "


/* DEVICE_DAP
 *
 * Device Access Point settings. If DEVICE_DAP_SIMPLE is set to 1, the DAP is
 * automatically created and all network interfaces are automatically attached.
 *
 * Default: enabled
 * Disable: undef DEVICE_DAP_SIMPLE
 */
#define DEVICE_DAP_SIMPLE             1
#define DEVICE_DAP_API                0
#define DEVICE_DAP_SLOT               0
#define DEVICE_DAP_SUBSLOT            1
#define DEVICE_DAP_MOD                0x01
#define DEVICE_DAP_SUBMOD             0x01



#endif /* PNIO_CONF_H */
