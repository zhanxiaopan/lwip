

#ifndef PN_RECDATA_LOG_H
#define PN_RECDATA_LOG_H


/*****************************************************************************/
/* Prototypes */
/*****************************************************************************/
RET_T PN_recRdLogBookData(
    const IOD_HEADER_T *pReadReqHdr,            /**< request header */
    IOD_READ_RES_T *pReadRes,                   /**< response data */
    Unsigned16 *pOutDataLen                     /**< response length */
);


#endif /* PN_RECDATA_LOG_H */
