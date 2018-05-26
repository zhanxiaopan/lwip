

#ifndef PN_DCP_H
#define PN_DCP_H


/* Defines
---------------------------------------------------------------------------*/

/* DCP Options/Suboptions */
#define PN_DCP_OPT(maj, min)                ((maj << 8) | (min & 0xff))
#define PN_DCP_OPT_IP                       0x01
#define PN_DCP_OPT_IP_MAC                   0x01
#define PN_DCP_OPT_IP_PARAM                 0x02
#define PN_DCP_OPT_DEV                      0x02
#define PN_DCP_OPT_DEV_VENDOR               0x01
#define PN_DCP_OPT_DEV_NAME                 0x02
#define PN_DCP_OPT_DEV_ID                   0x03
#define PN_DCP_OPT_DEV_ROLE                 0x04
#define PN_DCP_OPT_DEV_OPTS                 0x05
#define PN_DCP_OPT_DEV_ALIAS                0x06
#define PN_DCP_OPT_CTRL                     0x05
#define PN_DCP_OPT_CTRL_START               0x01
#define PN_DCP_OPT_CTRL_STOP                0x02
#define PN_DCP_OPT_CTRL_SIGNAL              0x03
#define PN_DCP_OPT_CTRL_RESPONSE            0x04
#define PN_DCP_OPT_CTRL_FR                  0x05
#define PN_DCP_OPT_CTRL_RTF                 0x06
#define PN_DCP_OPT_INI                      0x06
#define PN_DCP_OPT_INI_SUB                  0x01
#define PN_DCP_OPT_ALL                      0xFF
#define PN_DCP_OPT_ALL_SUB                  0xFF

/* DCP Reset To Factory */
#define PN_DCP_RTF_RESET_APPL               0x01
#define PN_DCP_RTF_RESET_COMM               0x02
#define PN_DCP_RTF_RESET_ALL                0x04

/* DCP BlockQualifier */
#define DCP_BLKQUAL_TEMPORARY               0x0
#define DCP_BLKQUAL_PERMANENT               0x1

/* Length Defines */
#define PN_DCP_LEN_SNAME                    240
#define PN_DCP_LEN_SNAME_MIN                1
#define PN_DCP_LEN_SNAME_XYZ                3
#define PN_DCP_LEN_SNAME_ABCDE              5
#define PN_DCP_LEN_LABEL                    63
#define PN_DCP_LEN_LABEL_MIN                1
#define PN_DCP_LEN_PORT                     5
#define PN_DCP_LEN_PORT_XYZ                 (PN_DCP_LEN_PORT + PN_DCP_LEN_SNAME_XYZ)
#define PN_DCP_LEN_PORT_XYZ_ABCDE           (PN_DCP_LEN_PORT_XYZ + PN_DCP_LEN_SNAME_ABCDE)
#define PN_DCP_LEN_IP_PART                  3
#define PN_DCP_LEN_OPT_OFS                  2
#define PN_DCP_LEN_IP_PARAM                 12
#define PN_DCP_LEN_DEV_ID                   4
#define PN_DCP_LEN_DEV_ROLE                 2
#define PN_DCP_LEN_DEV_OPTS                 16
#define PN_DCP_OPT_INI_SUB_LEN              1

/* PN_DCP_LEN_VENDOR: to be compatible with LLDP, length is restricted to 16
 * instead of 240 bytes */
#define PN_DCP_LEN_VENDOR                   16

/* Count Defines */
#define PN_DCP_CNT_IP_PART                  4

/* Block Error */
#define PN_DCP_OK                           0x00
#define PN_DCP_ERR_OPT_UNSUPP               0x01
#define PN_DCP_ERR_SUB_UNSUPP               0x02
#define PN_DCP_ERR_SUB_SET                  0x03
#define PN_DCP_ERR_RESOURCE_ERR             0x04
#define PN_DCP_ERR_SET_NOT_POSSIBLE         0x05
#define PN_DCP_ERR_IN_OPER_SET_NOT_POS      0x06

/* Block Info */
#define PN_DCP_OPT_IP_NONE                  (0x00 << 0)
#define PN_DCP_OPT_IP_SET                   (0x01 << 0)
#define PN_DCP_OPT_IP_NO_CONFLICT           (0x00 << 7)


/* constant definitions
---------------------------------------------------------------------------*/
#define DCP_NUM_OPTS                       12
#define DCP_HDR_SIZE                       10

#define DCP_GET_REQ                        0x03
#define DCP_SET_REQ                        0x04
#define DCP_IDENTIFY_REQ                   0x05
#define DCP_HELLO_REQ                      0x06

#ifndef CONFIG_DCP_HOLD_TIME
#  define CONFIG_DCP_HOLD_TIME             3000
#endif

#define PN_DCP_HELLO_MASK                  0x0001
#define PN_DCP_HELLO_OFF                   0x0000
#define PN_DCP_HELLO_ON                    0x0001


/* public data types
---------------------------------------------------------------------------*/
typedef enum {
    DCPMCR_STATE_OPEN = 0,                  /**< wait for identify req */
    DCPMCR_STATE_WRSP = 1,                  /**< wait for send resp delay */
} DCPMCR_STATE_T;


/** DCP State Management Structure */
typedef struct {
    /** DCP Multicast Receiver (Identify) */
    struct {
        DCPMCR_STATE_T  state;              /**< DCPMCR state (Identify) */
        PN_TIMER_ID_T   timer;              /**< identify response delay */
        OAL_BUFFER_T   *pBuf;               /**< response frame buffer */
        IOD_CREP_T      ep;                 /**< endpoint data */
    } dcpmcr;

    /** DCP Unicast Receiver (Get/Set) */
    struct {
        char            sam[MAC_ADDR_LEN];  /**< source mac address */
        PN_TIMESTAMP_T  holdTimeout;        /**< hold timeout */
        Unsigned32      sxid;               /**< transaction ID */
        IOD_CREP_T      ep;                 /**< endpoint data */
        OAL_BUFFER_T   *pBuf;               /**< response frame buffer */
    } dcpucr;
} PN_DCP_STATE_T;


/** DCP Block Header */
typedef struct {
    Unsigned8   option;                     /**< option */
    Unsigned8   subOpt;                     /**< suboption */
    Unsigned16  blkLen;                     /**< block length */
} PN_DCP_BLOCK_T;


/* list of public functions
---------------------------------------------------------------------------*/
IOD_STATUS_T PN_dcpInit(void);
IOD_STATUS_T PN_dcpInitNvs(void);
void PN_dcpRecv(const MACADDR_T *, const MACADDR_T *, Unsigned8 *, unsigned int);
IOD_STATUS_T PN_dcpEthUp(void);
IOD_STATUS_T PN_dcpEthDown(void);


/* compiler check
---------------------------------------------------------------------------*/
#if (0 < CONFIG_MAX_DCP_HELLO)
#  error "DCP FSU/Hello is currently unavailable."
#endif


#endif /* PN_DCP_H */
