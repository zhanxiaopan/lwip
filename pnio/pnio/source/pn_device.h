

#ifndef PN_DEVICE_H
#define PN_DEVICE_H


/* Global Structures */
extern AP_LIST_T apList;                /**< application process list */
extern MODULE_LIST_T moduleList;        /**< module list */


/** Initialize the device slot/module/param structures.
 *
 * Set the device slot/module/param configuration to zero.
 *
 * @return IOD_OK for success, otherwise error
 */
IOD_STATUS_T DEV_init(void);

#endif /* PN_DEVICE_H */
