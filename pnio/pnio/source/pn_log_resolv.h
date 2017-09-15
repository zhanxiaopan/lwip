

#ifndef PN_LOG_RESOLV_H
#define PN_LOG_RESOLV_H

#include <pn_includes.h>

#ifdef CONFIG_LOGGING
#ifdef CONFIG_LOGGING_NAME_RESOLVER

typedef enum {
    PN_LOG_RES_TYPE_PARAM = 0,
    PN_LOG_RES_TYPE_APP_CALLBACK,
    PN_LOG_RES_TYPE_MEM_QUEUES,
    PN_LOG_RES_TYPE_USAGE_TYPES,
} LOG_resType;


/**< generic convert function typedef */
typedef const char * (*LOG_convGeneric)(void *);

/**< ID and its string representation */
typedef struct {
    Unsigned32 id;
    const char *name;
    const LOG_convGeneric conv;
} LOG_MAP_ID_T;

#define LOG_MAP_ID_ELEM(id, conv) { id, #id, conv }

const char * LOG_resKeyVal(LOG_resType type, Unsigned32 keyId, void *val);
const char * LOG_convStr(void *str);
const char * LOG_convIPv4(void *ipAddr);
const char * LOG_convMAC(void *macAddr);
const char * LOG_convDataDir(DATA_DIR_T dataDir);
const char * LOG_convSlotState(SLOT_STATE_T state);
const char * LOG_convSubslotState(SUBSLOT_STATE_T state);
const char * LOG_convIoCrType(IO_CR_TYPE_T type);

#else /* ! CONFIG_LOGGING_NAME_RESOLVER */

const char * LOG_resIdToHexStr(Unsigned32 id);

#define LOG_resKeyVal(x, y, z) LOG_resIdToHexStr(y)
#define LOG_convStr(x) x
#define LOG_convIPv4(x) LOG_resIdToHexStr((Unsigned32) *x)
#define LOG_convDataDir(x) LOG_resIdToHexStr(x)
#define LOG_convSlotState(x) LOG_resIdToHexStr(x)
#define LOG_convSubslotState(x) LOG_resIdToHexStr(x)
#define LOG_convIoCrType(x) LOG_resIdToHexStr(x)

#endif /* CONFIG_LOGGING_NAME_RESOLVER */
#endif /* CONFIG_LOGGING */
#endif /* PN_LOG_RESOLV_H */
