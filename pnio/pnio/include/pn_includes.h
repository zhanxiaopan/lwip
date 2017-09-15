

#ifndef PN_INCLUDES_H
#define PN_INCLUDES_H

/* enable calling from cpp */
#ifdef __cplusplus
extern "C" {
#endif

/* standard libraries */
#include <string.h>

/* application configuration */
#include <pnio_conf.h>
#include <pnio_conf_test.h>

/* independent types */
#include <pn_types_indep.h>
#include <pn_status.h>

/* test if all mandatory defines are set in pnio_conf.h or at compiler level */
#if defined(PN_CC_PACKED)
#  if !defined(PN_CC_PACKED_PRE)
#    define PN_CC_PACKED_PRE
#  endif
#endif

#if defined(PN_CC_PACKED_PRE)
#  if !defined(PN_CC_PACKED)
#    define PN_CC_PACKED
#  endif
#endif

#if !defined(PN_CC_PACKED) || !defined(PN_CC_PACKED_PRE)
#  error "PN_CC_PACKED or PN_CC_PACKED_PRE not defined. If structures are " \
         "automatically packed, set one of this defines to an empty value."
#endif

#if !defined(CONFIG_CC_FORMAT_PRINTF)
#  error "CONFIG_CC_FORMAT_PRINTF not defined. If your compiler doesn't " \
         "support printf-format checking, please set this define to an " \
         "empty value."
#endif


/* no TRACE */
#define TRACE(x, v)
#define TRACE_IMP(x)
#define TRACE_INC(x)
#define TRACE_DEC(x)


/* stack includes */
#include <pn_def.h>
#include <pn_oal.h>
#include <pn_oal_api.h>
#include <pn_types.h>

#include <pn_lock.h>

#include <pn_version.h>
#include <pn_frames.h>
#include <pn_config.h>
#include <pn_pdev.h>

#include <pn_mem_simple.h>
#include <pn_mem.h>

#include <pn_context.h>
#include <pn_lmpm.h>
#include <pn_lldp.h>
#include <pn_alarm.h>
#include <pn_crtdata.h>
#include <pn_dcp.h>
#include <pn_ar.h>
#include <pn_utils.h>
#include <pn_user.h>
#include <pn_diag.h>
#include <pn_recdata.h>
#include <pn_recdata_diag.h>
#include <pn_recdata_log.h>
#include <pn_rpc.h>
#include <pn_rpc_types.h>
#include <pn_iodata.h>
#include <pn_user.h>

#include <pn_log.h>

#include <pn_log_resolv.h>
#include <pn_device.h>
#include <pn_net.h>
#include <pn_stat.h>
#include <pn_snmp.h>

#include <pn_timer.h>

#include <pn_cmsm.h>

#ifdef CONFIG_TARGET_LOCK_INT_ENABLE
#  include <pn_oal_lock_int.h>
#endif

#ifdef __cplusplus
}
#endif

#endif /* PN_INCLUDES_H */
