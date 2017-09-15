

#ifndef PN_CONFIG_H
#define PN_CONFIG_H

/* Config Key Structure */
#define CFG_KEY_ENTRY(id) { id, id ## _LEN, id ## _LEN, 0 }
typedef struct {
    unsigned int id;                    /**< key id */
    Unsigned32 size;                    /**< value max size */
    unsigned int ofsLen;                /**< length offset */
    unsigned int ofsData;               /**< data offset */
} CFG_KEY_ENTRIES_T;


/* Config Key IDs and Lengths */
typedef enum {
    CFG_KEY_META_CHECKSUM,              /**< meta: check if NVS checksum matches */
    CFG_KEY_META_VERSION,               /**< meta: check if NVS version matches */

#ifdef CONFIG_LLDP_INTERNAL
    CFG_KEY_LLDP,
#endif

    CFG_KEY_DCP_MACADDR,                /**< MAC address */
    CFG_KEY_DCP_VENDOR_ID,              /**< vendor ID */
    CFG_KEY_DCP_DEVICE_ID,              /**< device ID */
    CFG_KEY_DCP_DEVICE_VENDOR,          /**< device vendor */
    CFG_KEY_DCP_STATION_NAME,           /**< station name */

    CFG_KEY_IM_1_TAG_FUNC,              /**< Identification and Maintenance 1 */
    CFG_KEY_IM_1_TAG_LOC,               /**< Identification and Maintenance 1 */
    CFG_KEY_IM_2_DATE,                  /**< Identification and Maintenance 2 */
    CFG_KEY_IM_3_DESC,                  /**< Identification and Maintenance 3 */
    CFG_KEY_IM_4_SIG,                   /**< Identification and Maintenance 4 */

    CFG_KEY_NET_IP,                     /**< net: IP address */
    CFG_KEY_NET_NETMASK,                /**< net: netmask */
    CFG_KEY_NET_GATEWAY,                /**< net: gateway */

    CFG_KEY_RPC_PDPORTDATA,             /**< rpc: PDPortData */

    CFG_KEY_META_ALIGN,                 /**< meta: make sure struct is 4-byte alignable */
    CFG_KEY_COUNT_NVS,                  /**< count of NVS stored values in this enum */

    /* ------- the following values are not stored in NVS ------- */
    CFG_KEY_DCP_TMP_STATION_NAME,       /**< dcp: station name */

    CFG_KEY_NET_TEMP_IP,                /**< net: IP address */
    CFG_KEY_NET_TEMP_NETMASK,           /**< net: netmask */
    CFG_KEY_NET_TEMP_GATEWAY,           /**< net: gateway */

    CFG_KEY_HWREV,                      /**< hw rev */
    CFG_KEY_SWREV_PREFIX,               /**< sw rev prefix */
    CFG_KEY_SWREV_ENH,                  /**< sw rev functional enhancement */
    CFG_KEY_SWREV_BUGFIX,               /**< sw rev bugfix */
    CFG_KEY_SWREV_INTCHG,               /**< sw rev internal change */
    CFG_KEY_SWREV_REVCNT,               /**< sw rev counter */
    CFG_KEY_PROFILE_ID,                 /**< profile ID */
    CFG_KEY_PROFILE_TYPE,               /**< profile type */
    CFG_KEY_ORDER_ID,                   /**< order ID */
    CFG_KEY_SERIAL_NR,                  /**< serial nr */

    CFG_KEY_COUNT_ALL                   /**< count of runtime storage values in this enum */
} CFG_KEYS_T;


#define CFG_KEY_META_VERSION_LEN        sizeof(Unsigned32)
#define CFG_KEY_META_CHECKSUM_LEN       sizeof(Unsigned32)
#ifdef CONFIG_LLDP_INTERNAL
#   define CFG_KEY_LLDP_LEN             sizeof(PN_LLDP_NVS_T)
#endif
#define CFG_KEY_DCP_MACADDR_LEN         MAC_ADDR_LEN
#define CFG_KEY_DCP_VENDOR_ID_LEN       sizeof(Unsigned16)
#define CFG_KEY_DCP_DEVICE_ID_LEN       sizeof(Unsigned16)
#define CFG_KEY_DCP_DEVICE_VENDOR_LEN   PN_DCP_LEN_VENDOR
#define CFG_KEY_DCP_STATION_NAME_LEN    PN_DCP_LEN_SNAME
#define CFG_KEY_DCP_TMP_STATION_NAME_LEN PN_DCP_LEN_SNAME

#define CFG_KEY_IM_1_TAG_FUNC_LEN       PN_IM_1_TAG_FUNC_LEN
#define CFG_KEY_IM_1_TAG_LOC_LEN        PN_IM_1_TAG_LOC_LEN
#define CFG_KEY_IM_2_DATE_LEN           PN_IM_2_DATE_LEN
#define CFG_KEY_IM_3_DESC_LEN           PN_IM_3_DESC_LEN
#define CFG_KEY_IM_4_SIG_LEN            PN_IM_4_SIG_LEN

#define CFG_KEY_RPC_PDPORTDATA_LEN      sizeof(PN_REC_PDPORTDATA_CFG_T)

#define CFG_KEY_META_ALIGN_LEN          4

#define CFG_KEY_NET_IP_LEN              IPV4_ADDR_LEN
#define CFG_KEY_NET_NETMASK_LEN         IPV4_ADDR_LEN
#define CFG_KEY_NET_GATEWAY_LEN         IPV4_ADDR_LEN

#define CFG_KEY_NET_TEMP_IP_LEN         IPV4_ADDR_LEN
#define CFG_KEY_NET_TEMP_NETMASK_LEN    IPV4_ADDR_LEN
#define CFG_KEY_NET_TEMP_GATEWAY_LEN    IPV4_ADDR_LEN

#define CFG_KEY_HWREV_LEN               PN_LEN_HWREV
#define CFG_KEY_SWREV_PREFIX_LEN        PN_LEN_SWREV_PREFIX
#define CFG_KEY_SWREV_ENH_LEN           PN_LEN_SWREV_ENH
#define CFG_KEY_SWREV_BUGFIX_LEN        PN_LEN_SWREV_BUGFIX
#define CFG_KEY_SWREV_INTCHG_LEN        PN_LEN_SWREV_INTCHG
#define CFG_KEY_SWREV_REVCNT_LEN        PN_LEN_SWREV_REVCNT
#define CFG_KEY_PROFILE_ID_LEN          PN_LEN_PROFILE_ID
#define CFG_KEY_PROFILE_TYPE_LEN        PN_LEN_PROFILE_TYPE
#define CFG_KEY_ORDER_ID_LEN            (PN_LEN_ORDER_ID + PN_LEN_STRTERM)
#define CFG_KEY_SERIAL_NR_LEN           (PN_LEN_SERIAL_NR + PN_LEN_STRTERM)


/* Meta Init String: The magic value that decides if NVS is valid. */
#define CFG_KEY_META_VERSION_ID         0x00000004


/* Prototypes */
IOD_STATUS_T CFG_init(void);


#endif /* PN_CONFIG_H */
