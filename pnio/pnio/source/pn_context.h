

#ifndef PN_CONTEXT_H
#define PN_CONTEXT_H

#include <pn_rpc_types.h>

/* public constant definitions
---------------------------------------------------------------------------*/
/* defines for CM BlockType request */
#define CM_AR_BLOCK_REQ                 0x0101
#define CM_IOCR_BLOCK_REQ               0x0102
#define CM_ALARM_CR_BLOCK_REQ           0x0103
#define CM_EXPSUB_BLOCK_REQ             0x0104
#define CM_MCR_BLOCK_REQ                0x0106
#define CM_IOD_CTRLBLK_CONN_REQ         0x0110  /* IOD Control Block Connect */
#define CM_IOD_CTRLBLK_PLUG_REQ         0x0111  /* IOD Control Block Plug */
#define CM_IOX_CTRLBLK_CONN_REQ         0x0112  /* IOX Control Block Connect */
#define CM_IOX_CTRLBLK_PLUG_REQ         0x0113  /* IOX Control Block Plug */
#define CM_IOD_RELEASE_BLK_REQ          0x0114  /* IOD Release BLock */

/* defines for CM BlockType response */
#define CM_AR_BLOCK_RES 0x8101
#define CM_IOCR_BLOCK_RES       0x8102
#define CM_MODDIFF_BLOCK        0x8104
#define CM_IOD_CTRLBLK_CONN_RES         0x8110  /* IOD Control Block Connect */
#define CM_IOD_CTRLBLK_PLUG_RES         0x8111  /* IOD Control Block Plug */
#define CM_IOX_CTRLBLK_APP_READY_RES    0x8112  /* IOX Control Block Application Ready Response */
#define CM_IOX_CTRLBLK_PLUG_RES         0x8113  /* IOX Control Block Plug */
#define CM_IOD_RELEASE_BLK_RES          0x8114  /* IOD Release BLock */

/* Control command bits */
#define CM_CNTRLCMD_PRM_END     0x0001  /* parameter end */
#define CM_CNTRLCMD_APPL_READY 0x0002   /* Application ready */
#define CM_CNTRLCMD_RELEASE     0x0004  /* Release */
#define CM_CNTRLCMD_DONE        0x0008  /* Done */

/* Context manager data types
---------------------------------------------------------------------------*/

/* list of public functions
---------------------------------------------------------------------------*/

IOD_STATUS_T CM_processServReqPDU(
    Unsigned8 serviceNum,                       /**< RPC service number */
    RPC_SESSION_INFO_T *sessionId,              /**< RPC session identifier */
    const Unsigned8 *pInData,                   /**< buffer with request frame */
    Unsigned16 inDataLen,                       /**< length of request frame */
    Unsigned8 *pOutData,                        /**< buffer to store response frame */
    Unsigned16 *pOutDataLen,                    /**< pointer to store response length */
    ERROR_STATUS_T *pErrorStatus                /**< pointer to store error status response */
);

IOD_STATUS_T CM_init(void);

/* list of public variables
---------------------------------------------------------------------------*/
extern Unsigned8 activeServNum;


#endif
