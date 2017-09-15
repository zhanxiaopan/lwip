

#ifndef PN_LOG_H
#define PN_LOG_H

#include <pn_includes.h>

/* macro to initialize module logging ID */
#ifdef CONFIG_LOGGING
#  define PN_LOG_SET_ID(id) \
static const PN_LOG_ID_T PN_LOG_id = id;
#else
#  define PN_LOG_SET_ID(id)
#endif /* CONFIG_LOGGING */


/* check if logging is configured correctly */
#if defined(CONFIG_LOGGING)
#  if !defined(CONFIG_LOGGING_TARGET_RAW) && \
      !defined(CONFIG_LOGGING_TARGET_SYSLOG)
#    error "Logging enabled but no logging target set (CONFIG_LOGGING_TARGET_*)."
#  endif

#  if !defined(CONFIG_LOGGING_BUFFER_SIZE)
#    error "Logging enabled but not log string size set (CONFIG_LOGGING_BUFFER_SIZE)."
#  endif
#endif

/* define where the message buffer resides */
#if defined(CONFIG_LOGGING)
#  if defined(CONFIG_LOGGING_TARGET_SYSLOG)
#    define CONFIG_LOGGING_BUF syslog_data->data
#  else
#    define CONFIG_LOGGING_BUF pn_log_data
#  endif
#endif

/* defines for ModuleDbgFlag */
#define PN_DBG_NONE                           0x00
#define PN_DBG_ERR                            0x01
#define PN_DBG_WARN                           0x02
#define PN_DBG_INFO                           0x04
#define PN_DBG_DBG                            0x08
#define PN_DBG_ALL                            ((PN_DBG_DBG * 2) - 1)
#define PN_DBG_DEFAULT                        (PN_DBG_INFO | PN_DBG_WARN | PN_DBG_ERR)


/* logging functions */

#  define PN_logDbg(...)
#  define PN_logInfo(...)
#  define PN_logWarn(...)
#  define PN_logErr(...)

#  define PN_logInitEarly()
#  define uuidToString(uuid) ""


#endif /* PN_LOG_H */
