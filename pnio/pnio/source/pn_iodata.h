

#ifndef PN_IODATA_H_
#define PN_IODATA_H_

/* public constant definitions
---------------------------------------------------------------------------*/
/* IO CR types */
#define IO_INPUT_CR             0x0001
#define IO_OUTPUT_CR            0x0002

/* IO CR Properties Bits */
#define IOCR_PROP_RESERVED      0xFFFFF7F0      /* reserved bits */
#define IOCR_PROP_MEDIA_RED     0x00000800      /* Media redundancy */
#define IOCR_PROP_RTCLASS       0x0000000F      /* Real time class */

#define IOCR_RES_LEN 8
#define IOPS_LEN 1
#define IOCS_LEN 1


/* public data types
---------------------------------------------------------------------------*/

/* list of public functions
---------------------------------------------------------------------------*/
void IO_processIOCRApiBlock(
    AR_T *pAR,
    Unsigned32 api,
    IOCRREQ_API_T *pApiBlock,
    IO_EP_T *pIoEp);

Unsigned8 IO_checkIOCRApiBlock(
    AR_T *pAR, Unsigned32 api,
    IOCRREQ_API_T *pApiBlock,
    Unsigned16 *pApiBlockLen);

IOD_STATUS_T IO_createIODataObj(
    AR_T      *pAR,
    Unsigned32 api,
    Unsigned16 slotNr,
    Unsigned16 subSlotNr,
    Unsigned16 frameOffset,
    IO_EP_T *pIOEP,
    IO_CR_TYPE_T ioCrType);

IOD_STATUS_T IO_createIOCS(
    AR_T      *pAR,
    Unsigned32 api,
    Unsigned16 slotNr,
    Unsigned16 subSlotNr,
    Unsigned16 frameOffset,
    IO_EP_T *pIOEP,
    IO_CR_TYPE_T ioCrType);

IOD_STATUS_T IO_processIOCRBlock(
    AR_T *pAR,
    const IOCR_BLOCK_REQ_T *pIOCRBlockReq,
    IOCR_BLOCK_RES_T *pIOCRBlockRes,
    ERROR_STATUS_T *pErrorResp,
    Unsigned16 *pRespLen);

void IO_startCyclicComm(AR_T *pAR);

IOD_STATUS_T PN_ioSetIoxsDirect(
    SUBSLOT_T *pSubSlot,                        /**< subslot ptr */
    IO_CR_TYPE_T type,                          /**< input/output type */
    PN_BOOL_T iocsFlag,                         /**< IOCS/IOPS flag */
    Unsigned8 value                             /**< value */
);

IOD_STATUS_T PN_ioGetIoxsDirect(
    SUBSLOT_T *pSubSlot,                        /**< subslot ptr */
    IO_CR_TYPE_T type,                          /**< input/output type */
    PN_BOOL_T iocsFlag,                         /**< IOCS/IOPS flag */
    Unsigned8 *pValue                           /**< value ref */
);


/* list of public variables
---------------------------------------------------------------------------*/

#endif /* PN_IODATA_H_ */
