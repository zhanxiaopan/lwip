

#ifndef PN_CRTDATA_H
#define PN_CRTDATA_H

/* endpoint queue sizes */
#define CRT_RX_QUEUE_NUM    1
#define CRT_TX_QUEUE_NUM    2

/* defines */
#define PN_CRT_TIME_BASE    3125
#define PN_CRT_TIME_DIV     100000


/* prototypes */
IOD_STATUS_T CD_init(void);
IOD_STATUS_T CD_openEP(AR_T *, const Unsigned8 *, const Unsigned8 *, IO_EP_T **, ERROR_STATUS_T *);
IOD_STATUS_T CD_closeEP(IO_EP_T *);
IOD_STATUS_T CD_activateEP(IO_EP_T *);
IOD_STATUS_T CD_getData(IO_EP_T *, Unsigned16, Unsigned8 *, Unsigned16);
IOD_STATUS_T CD_setData(IO_EP_T *, Unsigned16, const Unsigned8 *, unsigned int);
void CD_exchangeOutputBuf(IO_EP_T *, OAL_BUFFER_T **);
IOD_STATUS_T PN_crtGetApduStatus(Integer32 arId, PN_APDU_STATUS_T *pApduStatus);
void PN_crtSetStationProblemIndicator(PN_BOOL_T flag);


#endif /* PN_CRTDATA_H */
