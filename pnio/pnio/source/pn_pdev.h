

#ifndef PN_PDEV_H
#define PN_PDEV_H


/***************************************************************************/
/* defines */
/***************************************************************************/

/* interface and port */
#define PDEV_SUBSLOT_IF                     0x8000
#define PDEV_SUBSLOT_PORT                   0x8001

/* SubmoduleState.AddInfo */
#define PN_PDEV_SUB_STATE_ADD_NONE          (0 << 0)
#define PN_PDEV_SUB_STATE_ADD_NO_TAKE       (1 << 0)

/* SubmoduleState.MaintenanceRequired */
#define PN_PDEV_SUB_STATE_MAINTREQ_NO       (0 << 4)
#define PN_PDEV_SUB_STATE_MAINTREQ_YES      (1 << 4)

/* SubmoduleState.MaintenanceDemanded */
#define PN_PDEV_SUB_STATE_MAINTDEM_NO       (0 << 5)
#define PN_PDEV_SUB_STATE_MAINTDEM_YES      (1 << 5)

/* SubmoduleState.Fault */
#define PN_PDEV_SUB_STATE_FAULT_NO          (0 << 6)
#define PN_PDEV_SUB_STATE_FAULT_YES         (1 << 6)

/* SubmoduleState.ARInfo */
#define PN_PDEV_SUB_STATE_AR_OWN            (0 << 7)
#define PN_PDEV_SUB_STATE_AR_ARP            (1 << 7)
#define PN_PDEV_SUB_STATE_AR_SO             (2 << 7)
#define PN_PDEV_SUB_STATE_AR_IOC            (3 << 7)
#define PN_PDEV_SUB_STATE_AR_IOS            (4 << 7)

/* SubmoduleState.IdentInfo */
#define PN_PDEV_SUB_STATE_IDENT_OK          (0 << 11)
#define PN_PDEV_SUB_STATE_IDENT_SUBST       (1 << 11)
#define PN_PDEV_SUB_STATE_IDENT_WRONG       (2 << 11)
#define PN_PDEV_SUB_STATE_IDENT_NOSUB       (3 << 11)

/* SubmoduleState.FormatIndicator */
#define PN_PDEV_SUB_STATE_FORM_IND          (1 << 15)


#define SUBMODULE_PROP_SHARED_INPUT         (0x0001 <<  2)

#define PN_IM_1_TAG_FUNC_LEN                32
#define PN_IM_1_TAG_LOC_LEN                 22
#define PN_IM_2_DATE_LEN                    16
#define PN_IM_3_DESC_LEN                    54
#define PN_IM_4_SIG_LEN                     54

#define PN_PDEV_MASK_IM_0                   0x01
#define PN_PDEV_MASK_IM_1                   0x02
#define PN_PDEV_MASK_IM_2                   0x04
#define PN_PDEV_MASK_IM_3                   0x08
#define PN_PDEV_MASK_IM_4                   0x10


/***************************************************************************/
/* exported variables */
/***************************************************************************/

extern const PNIO_VERSION_T pnio_version;       /**< Profinet Stack Version Information */
extern PNA_CALLBACK_FUNC PNA_callback;          /**< app callback function */
extern PN_BOOL_T IOD_InitDone;                  /**< Profinet Started Flag */
extern int PN_flagShutdown;                     /**< global shutdown flag */
extern PN_BOOL_T PN_pdevGlobalEnable;           /**< global stack enable */


/***************************************************************************/
/* datatypes */
/***************************************************************************/

/**< expected module/submodule block */
typedef struct {
    PN_BOOL_T used;                             /**< AR usage flag */
    Unsigned8 AR[AR_UUID_SIZE];                 /**< AR */
    struct {
        PN_BOOL_T used;                         /**< API usage flag */
        Unsigned32 number;                      /**< API */
        struct {
            PN_BOOL_T used;                     /**< slot usage flag */
            Unsigned16 number;                  /**< slot */
            Unsigned32 identNumber;             /**< module ident nr */
            struct {
                PN_BOOL_T used;                 /**< subslot usage flag */
                Unsigned16 number;              /**< subslot */
                Unsigned32 identNumber;         /**< subslot ident nr */
            } subSlot[CONFIG_MAX_SUBSLOT_COUNT];
        } slot[CONFIG_MAX_SLOT_COUNT];
    } api[CONFIG_MAX_AP_COUNT];
} EXP_AR_T;


/**< subhelper callback for PD_processExpSubModBlock_loop */
typedef void (*PN_PD_PESMB_CB_T)(
    int sel,                                    /**< stage: API (1), submod (2), desc (3) */
    AR_T *pAR,                                  /**< pointer to AR */
    const EXPSUB_API_T *pApi,                   /**< pointer to API block */
    const EXPSUB_SUBMOD_T *pSubMod,             /**< pointer to submodule block */
    const EXPSUB_DATADESC_T *pDataDesc,         /**< pointer to data descriptor */
    ERROR_STATUS_T *pErrorResp                  /**< pointer to error response */
);


/***************************************************************************/
/* prototypes */
/***************************************************************************/

IOD_STATUS_T PN_pdInitNVS(
    void
);

AP_T * PD_getAPI(
    Unsigned32 api                              /**< API number */
);

RET_T PD_getIMDescr(
    Unsigned16 rec,                             /**< I&M index (0 - 4) */
    void * pImData                              /**< ptr to store I&M data */
);

RET_T PD_setIMDescr(
    Unsigned16 rec,                             /**< I&M index */
    const void * pImData                        /**< ptr to get I&M data */
);

RET_T PD_getSlot(
    Unsigned32 api,                             /**< Number of API */
    Unsigned16 slotNr,                          /**< Slot number */
    SLOT_T **ppSlot                             /**< Pointer to Slot */
);

Unsigned32 PD_getSlotIdentNumber(
    Unsigned32 api,                             /**< API */
    Unsigned16 slot                             /**< slot */
);

Unsigned32 PD_getSubSlotIdentNumber(
    Unsigned32 api,                             /**< API */
    Unsigned16 slot,                            /**< slot */
    Unsigned16 subSlot                          /**< subslot */
);

Unsigned8 PD_checkLocation(
    Unsigned32 api,                             /**< API */
    Unsigned16 slot,                            /**< slot */
    Unsigned16 subSlot                          /**< subslot */
);

RET_T PD_getPlugModule(
    const Unsigned32 api,                       /**< API */
    const Unsigned16 slotNr,                    /**< slot */
    MODULE_T **ppModule                         /**< ptr to store module */
);

RET_T PD_getSubSlot(
    const Unsigned32 api,                       /**< API */
    const Unsigned16 slotNr,                    /**< slot */
    const Unsigned16 subSlotNr,                 /**< subslot */
    SUBSLOT_T **ppSubSlot                       /**< ptr to store subslot */
);

RET_T PD_getSlotSubSlot(
    const Unsigned32 api,                       /**< API */
    const Unsigned16 slotNr,                    /**< slot */
    const Unsigned16 subSlotNr,                 /**< subslot */
    SLOT_T **ppSlot,                            /**< ptr to store slot */
    SUBSLOT_T **ppSubSlot                       /**< ptr to store subslot */
);

RET_T PD_getApiSlotSubSlot(
    Unsigned32 api,                             /**< API */
    Unsigned16 slotNr,                          /**< slot */
    Unsigned16 subSlotNr,                       /**< subslot */
    AP_T **ppAP,                                /**< ptr to store API */
    SLOT_T **ppSlot,                            /**< ptr to store slot */
    SUBSLOT_T **ppSubSlot                       /**< ptr to store subslot */
);

IOD_STATUS_T PD_expRemoveAR(
    AR_T *pAR                                   /**< AR pointer */
);

PN_BOOL_T PD_expIsElementLocked(
    AR_T *pAR,                                  /**< own AR, will be ignored */
    Unsigned32 iApi,                            /**< API */
    Unsigned16 iSlot,                           /**< slot */
    Unsigned16 iSubSlot,                        /**< subslot */
    int iLevel                                  /**< deepness level (0 = AR .. 3 = SubSlot) */
);

Unsigned16 PD_createModDiffBlock(
    AR_T *pAR,                                  /**< AR */
    MOD_DIFF_BLOCK_T *pModDiffBlock,            /**< buffer to store ModuleDiffBlock */
    ERROR_STATUS_T *pErrorStatus                /**< ErrorStatus */
);

Unsigned16 PD_createExpIdentDataBlock(
    AR_T *pAR,                                  /**< AR */
    Unsigned32 api,                             /**< API */
    Unsigned16 slot,                            /**< slot */
    Unsigned16 subSlot,                         /**< subslot */
    Unsigned16 index,                           /**< index */
    EXP_IDENT_DATA_T *pExpIdentDataBlock        /**< ptr to store block */
);

IOD_STATUS_T PD_processExpSubModBlock(
    AR_T *pAR,                                  /**< pointer to AR */
    const EXP_SUBMOD_BLOCK_T *expSubModBlock,   /**< expected submodule block */
    ERROR_STATUS_T *errorResp                   /**< error response */
);

IOD_STATUS_T PD_getModuleByIdentNr(
    const Unsigned32 moduleIdentNr,             /**< module ident nr */
    MODULE_T **ppModule                         /**< ptr to store module */
);

IOD_STATUS_T PD_getSubModuleByIdentNr(
    const Unsigned32 moduleIdentNr,             /**< module ident nr */
    const Unsigned32 subModuleIdentNr,          /**< submodule ident nr */
    SUBMOD_T **ppSubModule                      /**< ptr to store submodule */
);

IOD_STATUS_T PD_updateSlotSubslotState(
    Unsigned32 api,                             /**< API */
    Unsigned16 slotNr                           /**< slot */
);

IOD_STATUS_T PD_clearAR(
    AR_T *pAR                                   /**< pointer to AR */
);

#if defined(CONFIG_IM_1_4_SUPPORT) && !defined(CONFIG_IM_1_4_CALLBACK)
void PN_pdClearIM(
    void
);
#endif

void PN_pnioLoop(
    void
);

void PN_pdevStart(
    void
);

void PN_pdevStop(
    void
);


#endif /* PN_DEV_H */
