

#ifndef PN_RECDATA_DIAG_H
#define PN_RECDATA_DIAG_H

/* public constant definitions
---------------------------------------------------------------------------*/

/* public data types
---------------------------------------------------------------------------*/

/* list of public functions
---------------------------------------------------------------------------*/
RET_T genReadRespDiagForAR(
    const IOD_HEADER_T *pReadReqHdr,
    Unsigned32 api,
    Unsigned16 slot,
    Unsigned16 subSlot,
    IOD_READ_RES_T *pReadRes,
    Unsigned16 *pOutDataLen);

RET_T genReadRespDiagForDevice(
    const IOD_HEADER_T *pReadReqHdr,
    Unsigned32 api,
    Unsigned16 slot,
    Unsigned16 subSlot,
    IOD_READ_RES_T *pReadRes,
    Unsigned16 *pOutDataLen);

RET_T genReadRespPDevData(
    const IOD_HEADER_T *pReadReqHdr,
    Unsigned32 api,
    Unsigned16 slot,
    Unsigned16 subSlot,
    IOD_READ_RES_T *pReadRes,
    Unsigned16 *pOutDataLen);

RET_T PN_recGenReadRespDiagMaintQualStatus(
    const IOD_HEADER_T *pReadReqHdr,            /**< read request hdr */
    Unsigned32 api,                             /**< API */
    Unsigned16 slot,                            /**< slot */
    Unsigned16 subSlot,                         /**< subslot */
    IOD_READ_RES_T *pReadRes,                   /**< target frame */
    Unsigned16 *pOutDataLen                     /**< output data len ptr */
);


#endif /* PN_RECDATA_DIAG_H */
