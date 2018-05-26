

#ifndef PN_LMPM_H
#define PN_LMPM_H

/* OAL_BUFFER_T */
#ifndef PORT_GMBH_GOAL
#  include <pn_mem.h>
#endif


/* public constant definitions
---------------------------------------------------------------------------*/
#define LMPM_PRIQ_7                        7
#define LMPM_PRIQ_6                        6
#define LMPM_PRIQ_5                        5
#define LMPM_PRIQ_4                        4
#define LMPM_PRIQ_3                        3
#define LMPM_PRIQ_2                        2
#define LMPM_PRIQ_1                        1
#define LMPM_PRIQ_0                        0

#if !defined(ETH_HDR_LEN)
#  error "ETH_HDR_LEN not defined"
#elif !defined(VLAN_TAG_LEN)
#  error "VLAN_TAG_LEN not defined"
#endif
#define PNIO_DATA_OFF                      (ETH_HDR_LEN+VLAN_TAG_LEN)

#define LMPM_MAX_INPUTS                    8
#define LMPM_MAX_OUTPUTS                   8


/* list of public functions
---------------------------------------------------------------------------*/
IOD_STATUS_T LM_init(void);
void LMPM_shutdown(void);
IOD_STATUS_T LM_schedule(IO_EP_T *pEP);
IOD_STATUS_T LM_unschedule(const IO_EP_T *pEP);
IOD_STATUS_T LM_submitART(const IOD_CREP_T *pEP, OAL_BUFFER_T **pBuf);
Unsigned8    LM_fillMacHdr(const IOD_CREP_T *pEP, Unsigned8  *pBuf);
IOD_STATUS_T LM_handleCyclic(OAL_BUFFER_T **pBuf);
IOD_STATUS_T LM_handleNonCyclic(OAL_BUFFER_T *pBuf);

#endif
