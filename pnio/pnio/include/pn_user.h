

#ifndef PN_USER_H_
#define PN_USER_H_

#include <pn_frames.h>


/** Application Main
 *
 * Build up the device structure and initialize the Profinet stack.
 */
void main_appl(void);


/** Get the current AR Count
 *
 * Info: Because of the stacks implementation, this function does not show the
 * correct values in IOD_newARInd and IOD_ReleaseInd. IOD_newARInd is called
 * before the AR is set to used (so the value has to be interpreted with +1)
 * and IOD_ReleaseInd is called before the AR is released, so value must be
 * decremented by 1.
 *
 * @return count of used ARs
 */
unsigned int IOD_getARCount(void);


/** Get Cyclic Output Data
 *
 * This function reads cyclic output data of IO Data Object identified by API
 * \c api, Slot \c slotNr and SubSlot \c subSlotNr.  \c length number of bytes
 * is read from cyclic data buffer and written to the buffer \c pValue. For
 * this the buffer \c pValue should have a minimum size of \c length.
 * Additionally the IO Provider Status is returned in \c pIoPS.
 *
 * Reading of Output data is only allowed if an IO Data Object was created
 * before for this API/Slot/SubSlot. This is usually done by the PROFINET IO
 * Stack when receiving Connect frame from IO Controller.This function can
 * only be used to read IO Data Objects which are configured inside an Output
 * Communication Relationship (IOCR type).
 *
 * @retval PN_OK successful
 * @retval other error
 */
RET_T IOD_GetOutputData(
    Unsigned32 api,             /**< Application process number */
    Unsigned16 slotNr,          /**< Slot number */
    Unsigned16 subSlotNr,       /**< SubSlot number */
    Unsigned8 *pValue,          /**< buffer to return IO data */
    Unsigned16 length,          /**< number of bytes to read */
    Unsigned8 *pIoPS            /**< pointer to return IOPS */
);


/** Set Cyclic Input Data
 *
 * This function sets cyclic input data of IO Data Object identified by API \c
 * api, Slot \c slotNr and SubSlot \c subSlotNr. The data to be set is given in
 * \c pValue buffer with length \c length. Additionally the IO Provider Status
 * has to be set in \c ioPS.
 *
 * Setting of Input data is only allowed if an IO Data Object was created
 * before for this API/Slot/SubSlot. This is usually done by the PROFINET IO
 * Stack when receiving Connect frame from IO Controller. This function can
 * only be used to set IO Data Objects which are configured inside an Input
 * Communication Relationship (IOCR type).
 *
 * @retval PN_OK successful
 * @retval other failed
 */
RET_T IOD_SetInputData(
    Unsigned32 api,             /**< Application process number */
    Unsigned16 slotNr,          /**< Slot number */
    Unsigned16 subSlotNr,       /**< SubSlot number */
    Unsigned8 *pValue,          /**< pointer to data to set */
    Unsigned16 length,          /**< size of data to set */
    Unsigned8 ioPS              /**< IOPS to set */
);


/** Get Cyclic Input Data (Debug-Function)
 *
 * This function reads cyclic input data of IO Data Object identified by API \c
 * api, Slot \c slotNr and SubSlot \c subSlotNr. The data to be read is written
 * to \c pValue buffer with length \c length. If the buffer size is smaller
 * than the input data, an error is returned. The IO Provider Status is written
 * to \c pioPS.
 *
 * Reading of Input data is only allowed if an IO Data Object was created
 * before for this API/Slot/SubSlot. This is usually done by the PROFINET IO
 * Stack when receiving Connect frame from IO Controller. This function can
 * only be used to read IO Data Objects which are configured inside an Input
 * Communication Relationship (IOCR type).
 *
 * @retval PN_OK successful
 * @retval other failed
 */
RET_T IOD_GetInputData(
    Unsigned32 api,             /**< Application process number */
    Unsigned16 slotNr,          /**< Slot number */
    Unsigned16 subSlotNr,       /**< SubSlot number */
    Unsigned8 *pValue,          /**< pointer of data buffer */
    Unsigned16 length,          /**< size of data buffer */
    Unsigned8 *pIoPS            /**< IOPS store pointer */
);


/** Pull Module (with Submodules) from a given Slot
 *
 * Pull module with all attached submodules out of a given slot and send an
 * alarm to the controller if a connection is established.
 *
 * @retval PN_OK successful
 * @retval other failed
 */
RET_T IOD_pullModule(
    Unsigned32 api,                             /**< API number */
    Unsigned16 slotNr                           /**< Slot number */
);


/** Pull a Submodule from a given Subslot
 *
 * Pull a single submodule out of a given slot/subslot combination and send an
 * alarm to the controller if a connection is established.
 *
 * @retval PN_OK successful
 * @retval other failed
 */
RET_T IOD_pullSubModule(
    Unsigned32 api,                             /**< API number */
    Unsigned16 slotNr,                          /**< Slot number */
    Unsigned16 subSlotNr                        /**< Subslot number */
);


/** Plug a Module into a given Slot
 *
 * Plug a module into a given slot, but don't send an alarm.
 *
 * @retval PN_OK successful
 * @retval other failed
 */
RET_T IOD_plugModule(
    Unsigned32 api,                             /**< API number */
    Unsigned16 slotNr,                          /**< Slot number */
    Unsigned32 moduleIdentNr                    /**< Module ident number */
);


/** Plug a Submodule into a given Subslot
 *
 * Plug a submodule into a given slot/subslot combination and send an alarm if
 * a connection is established.
 *
 * @retval PN_OK successful
 * @retval other failed
 */
RET_T IOD_plugSubModule(
    Unsigned32 api,                             /**< API number */
    Unsigned16 slotNr,                          /**< Slot number */
    Unsigned16 subSlotNr,                       /**< Subslot number */
    Unsigned32 moduleIdentNr,                   /**< Module ident nr */
    Unsigned32 subModuleIdentNr,                /**< Submodule ident nr */
    Unsigned16 *subModulePlugHandle             /**< Submodule plug handle */
);


/** PROFINET IO Stack Initialization
 *
 * This function is the central stack initialization function. It has to be
 * called by user before any other stack function is used. It initializes all
 * lower and higher layer modules.
 *
 * @retval PN_OK successful
 * @retval other failed
 */
RET_T IOD_Init(
    PNA_CALLBACK_FUNC appCallback               /**< appl callback handler */
);


/** PROFINET IO Stack Shutdown
 *
 * Shutdown the components of the Profinet stack.
 */
void IOD_shutdown(
    void
);


/** PROFINET IO Stack Init Check
 *
 * Returns if the stack has been started.
 *
 * @retval PN_TRUE  stack has been started
 * @retval PN_FALSE stack was not started yet
 */
PN_BOOL_T IOD_isStackInitialized(
    void
);


/** Send Application Ready
 *
 * This function sends an application ready frame to IO Controller. The frame
 * is send as control service via RPC.
 *
 * The user has to provide the application relationship reference to identify
 * the application relationship the application relationship command belongs
 * to. Application relationship reference is provided in parameter end
 * callback.
 *
 * Info: Normally there is no need to call this function because the stack
 * automatically sends the application ready message.
 *
 * @retval PN_OK successful
 * @retval other failed
 */
RET_T IOD_sendApplReady(
    AR_T *pAR,              /**< Application relationship reference */
    Unsigned16 type,        /**< type of message (0 = normal, 1 = plug) */
    Unsigned16 plugHandle   /**< handle from plug request */
);


/** Set Input IOPS to Subslot Path
 *
 * Set input IOPS to subslot path.
 *
 * @retval IOD_OK ok
 * @retval other fail
 */
IOD_STATUS_T PN_ioSetIops(
    Unsigned32 api,                             /**< API */
    Unsigned16 slot,                            /**< slot */
    Unsigned16 subSlot,                         /**< subslot */
    Unsigned8 value                             /**< value */
);


/** Get Output IOPS from Subslot Path
 *
 * Set output IOPS from subslot path.
 *
 * @retval IOD_OK ok
 * @retval other fail
 */
IOD_STATUS_T PN_ioGetIops(
    Unsigned32 api,                             /**< API */
    Unsigned16 slot,                            /**< slot */
    Unsigned16 subSlot,                         /**< subslot */
    Unsigned8 *pValue                           /**< value ref */
);


/** Set Output IOCS to Subslot Path
 *
 * Set output IOCS to subslot path.
 *
 * @retval IOD_OK ok
 * @retval other fail
 */
IOD_STATUS_T PN_ioSetIocs(
    Unsigned32 api,                             /**< API */
    Unsigned16 slot,                            /**< slot */
    Unsigned16 subSlot,                         /**< subslot */
    Unsigned8 value                             /**< value */
);


/** Get Input IOCS from Subslot Path
 *
 * Set input IOCS from subslot path.
 *
 * @retval IOD_OK ok
 * @retval other fail
 */
IOD_STATUS_T PN_ioGetIocs(
    Unsigned32 api,                             /**< API */
    Unsigned16 slot,                            /**< slot */
    Unsigned16 subSlot,                         /**< subslot */
    Unsigned8 *pValue                           /**< value ref */
);


/** Set Device Name
 *
 * @retval PN_OK successful
 * @retval other failed
 */
IOD_STATUS_T PN_devSetDeviceName(
    const char* name,               /**< device name */
    unsigned int length,            /**< length of device name */
    PN_BOOL_T permFlag              /**< temporary or permanent */
);


/** DCP Set Vendor Name
 *
 * Set the DCP vendor name.
 */
IOD_STATUS_T PN_dcpSetVendorName(
    char *vendorName,               /**< vendor name */
    unsigned int len                /**< name length */
);


/** LLDP Set System Description
 *
 * Set the LLDP system description.
 */
IOD_STATUS_T PN_lldpSetSystemDesc(
    char *sysDesc,                  /**< system description */
    unsigned int len                /**< desc length */
);


/** LLDP Set Port Description
 *
 * Set the LLDP port description.
 */
IOD_STATUS_T PN_lldpSetPortDesc(
    char *portDesc,                 /**< port description */
    unsigned int len                /**< desc length */
);


/** Finish Write Record
 *
 * This function finishes the record write.
 */
RET_T IOD_finishWriteRecord(
    int handle,                     /**< record data busy index */
    ERROR_STATUS_T errorStatus      /**< error status */
);


/** Finish Read Record
 *
 * This function finishes the record read.
 */
RET_T IOD_finishReadRecord(
    int handle,                     /**< record data busy index */
    ERROR_STATUS_T errorStatus,     /**< error status */
    Unsigned8 *pRecordData,         /**< user supplied record data */
    Unsigned16 recordLength         /**< user supplied data length */
);


/** Create a new application process.
 *
 * Creates the first access level to the devices structures, the application
 * process (AP).
 *
 * @retval IOD_OK successful
 * @retval other failed
 */
IOD_STATUS_T IOD_devNewAP(
    Unsigned32 apNr                 /**< API number */
);


/** Create a slot in an AP.
 *
 * Creates a slot which is assigned to the APs slot list. If the genFlag is set
 * to AUTO_GEN and the AP apNr doesn't exist, is is created automatically.
 *
 * @retval IOD_OK successful
 * @retval other failed
 */
IOD_STATUS_T IOD_devNewSlot(
    Unsigned32 apNr,                /**< AP number */
    Unsigned16 slotNr,              /**< Slot number */
    DEV_GEN_FLAG_T genFlag          /**< auto-gen flag */
);


/** Create a subslot in a slot.
 *
 * Creates a subslot which is assigned to the slots subslot list. If the
 * genFlag is set to AUTO_GEN and the slot slotNr doesn't exist, it is created
 * automatically.
 *
 * @retval IOD_OK successful
 * @retval other failed
 */
IOD_STATUS_T IOD_devNewSubSlot(
    Unsigned32 apNr,                /**< AP number */
    Unsigned16 slotNr,              /**< slot number */
    Unsigned16 subSlotNr,           /**< subslot number */
    DEV_GEN_FLAG_T genFlag          /**< auto-gen flag */
);


/** Create a module in the module list.
 *
 * Creates a module in the global module list.
 *
 * @retval IOD_OK successful
 * @retval other failed
 */
IOD_STATUS_T IOD_devNewModule(
    Unsigned32 id                   /**< module id */
);


/** Create a submodule in a submodulelist.
 *
 * Creates a submodule which is assigned to the modules submodule list. If the
 * genFlag is set to AUTO_GET and the module modId doesn't exist, it is created
 * automatically.
 *
 * @retval IOD_OK successful
 * @retval other failed
 */
IOD_STATUS_T IOD_devNewSubModule(
    Unsigned32 modId,               /**< module id */
    Unsigned32 subModId,            /**< submodule id */
    DATA_DIR_T dataDir,             /**< data direction */
    Unsigned16 inLen,               /**< input length */
    Unsigned16 outLen,              /**< output length */
    DEV_GEN_FLAG_T genFlag          /**< auto-gen flag */
);


/** Copy NVS data element to data pointer
 *
 * @retval IOD_OK successful
 * @retval other failed
 */
IOD_STATUS_T IOD_cfgGet(
    unsigned int id,                /**< config key ID */
    void *data                      /**< data pointer */
);


/** Copy Element and Provide Length
 *
 * @retval IOD_OK successful
 * @retval other failed
 */
IOD_STATUS_T IOD_cfgGetValLen(
    unsigned int id,                        /**< config key ID */
    void *data,                             /**< data pointer */
    Unsigned32 *pLen                        /**< data length */
);


/** Get Const Ptr and Length of Element
 *
 * @retval IOD_OK successful
 * @retval other failed
 */
IOD_STATUS_T IOD_cfgGetLenConst(
    unsigned int id,                        /**< config key ID */
    const void **data,                      /**< data ptr ref */
    Unsigned32 *len                         /**< len ref */
);


/** Set Config Entry and Length
 *
 * @retval IOD_OK successful
 * @retval other failed
 */
IOD_STATUS_T IOD_cfgSetLen(
    unsigned int id,                        /**< config key ID */
    const void *data,                       /**< data pointer */
    unsigned int len                        /**< data length */
);


/** Set Config Entry
 *
 * @retval IOD_OK successful
 * @retval other failed
 */
IOD_STATUS_T IOD_cfgSet(
    unsigned int id,                        /**< config key ID */
    const void *data                        /**< data pointer */
);


/** Clear an NVS data element
 *
 * @retval IOD_OK successful
 * @retval other failed
 */
IOD_STATUS_T IOD_cfgClr(
    unsigned int id                 /**< config key ID */
);


/** Clear an NVS data element with a given pattern
 *
 * Acts like memset.
 *
 * @retval IOD_OK successful
 * @retval other failed
 */
IOD_STATUS_T IOD_cfgClrPat(
    unsigned int id,                /**< config key ID */
    unsigned char pattern           /**< pattern to clear key value */
);


/** Write current configuration to NVS
 *
 * @retval IOD_OK successful
 * @retval other failed
 */
IOD_STATUS_T IOD_cfgWrite(void);


/** Match NVS data element versus a data pointer
 *
 * @retval PN_TRUE matches
 * @retval PN_FALSE differs
 */
PN_BOOL_T IOD_cfgMatch(
    unsigned int id,                /**< config key ID */
    void *data,                     /**< data pointer */
    unsigned int len                /**< data length */
);


/** Copy Config Value
 *
 * Copies a configuration value to another.
 */
IOD_STATUS_T IOD_cfgCopy(
    unsigned int idTarget,          /**< target ID */
    unsigned int idSource           /**< source ID */
);


/** Set the global logging mask.
 *
 */
void PN_logSetGlobal(
    Unsigned8 mask                  /**< logging mask */
);


/** Set the module specific logging mask.
 *
 */
void PN_logSetMask(
    const PN_LOG_ID_T id,           /**< module id */
    Unsigned8 mask                  /**< logging mask */
);


/** Set Vendor ID
 *
 * Set the vendor ID.
 *
 * @retval IOD_STATUS_T status
 */
IOD_STATUS_T PN_devSetVendorId(
    Unsigned16 vendorId             /**< vendor ID */
);


/** Set Device ID
 *
 * Set the device ID.
 *
 * @retval IOD_STATUS_T status
 */
IOD_STATUS_T PN_devSetDeviceId(
    Unsigned16 deviceId             /**< device ID */
);


/** Set Hardware Revision
 *
 * Set the hardware revision.
 *
 * @retval IOD_STATUS_T status
 */
IOD_STATUS_T PN_devSetHwRev(
    Unsigned16 hwRev                /**< hardware revision */
);


/** Set Software Revision
 *
 * Set the software revision.
 *
 * @retval IOD_STATUS_T status
 */
IOD_STATUS_T PN_devSetSwRev(
    char prefix,                    /**< prefix */
    Unsigned8 funcEnh,              /**< functional enhancement */
    Unsigned8 bugfix,               /**< bugfix */
    Unsigned8 intChange,            /**< internal change */
    Unsigned16 revCnt               /**< revision counter */
);


/** Set Profile ID
 *
 * Set the profile ID.
 *
 * @retval IOD_STATUS_T status
 */
IOD_STATUS_T PN_devSetProfileId(
    Unsigned16 profileId,           /**< profile ID */
    Unsigned16 specType             /**< profile specific type */
);


/** Set Order ID
 *
 * Set the order ID.
 *
 * @retval IOD_STATUS_T status
 */
IOD_STATUS_T PN_devSetOrderId(
    char *orderId,                  /**< order ID */
    unsigned int len                /**< order ID len */
);


/** Set Serial Nr
 *
 * Set the serial nr.
 *
 * @retval IOD_STATUS_T status
 */
IOD_STATUS_T PN_devSetSerialNr(
    char *serialNr,                 /**< serial nr */
    unsigned int len                /**< serial nr len */
);


#endif /* PN_USER_H_ */
