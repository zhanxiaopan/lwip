

#ifndef PN_CMSM_H
#define PN_CMSM_H


/*****************************************************************************/
/* Defines */
/*****************************************************************************/

/**< CMSM state machine */
typedef enum {
    PN_CMSM_STATE_IDLE,                         /**< idle */
    PN_CMSM_STATE_RUN                           /**< run */
} PN_CMSM_STATE_T;


/*****************************************************************************/
/* Prototypes */
/*****************************************************************************/

RET_T PN_cmsmRecRd(
    unsigned char *pData,                       /**< data ptr */
    Unsigned16 *pDataLen,                       /**< data length */
    Unsigned32 maxDataLen,                      /**< max data length */
    ERROR_STATUS_T *errorStatus,                /**< error status */
    AR_T *pAR,                                  /**< AR ptr */
    Unsigned32 api,                             /**< API */
    Unsigned16 slot,                            /**< slot */
    Unsigned16 subSlot,                         /**< subslot */
    Unsigned16 index                            /**< index */
);


#endif /* PN_CMSM_H */
