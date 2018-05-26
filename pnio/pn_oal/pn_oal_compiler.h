

#ifndef PN_OAL_COMPILER_H
#define PN_OAL_COMPILER_H


/***************************************************************************/
/* helper macros */
/***************************************************************************/
#define STR_META(x) #x
#define STR(x) STR_META(x)


/***************************************************************************/
/* compiler specific defines */
/***************************************************************************/
/*
 *   CONFIG_CC_FORMAT_PRINTF_PRE - printf-format attribute before definition
 *   CONFIG_CC_FORMAT_PRINTF     - printf-format attribute after definition
 *   CONFIG_INLINE               - inline attribute
 *   CONFIG_NORETURN             - no-return attribute after definition
 *   CONFIG_NORETURN_PRE         - no-return attribute before definition
 *   PN_CC_PACKED                - packed attribute after definition
 *   PN_CC_PACKED_PRE            - packed attribute before definition
 *
 * hint: for new compiler defines see http://sourceforge.net/p/predef/wiki/Compilers
 */

/* GNU C Compiler */
#if defined(__GNUC__)
#  define CONFIG_CC_FORMAT_PRINTF       __attribute__((format (printf, 5, 6)))
#  define CONFIG_CC_FORMAT_PRINTF_PRE
#  define CONFIG_INLINE                 inline
#  define CONFIG_NORETURN               __attribute__((noreturn))
#  define CONFIG_NORETURN_PRE
#  define PN_CC_PACKED                  __attribute__((packed))
#  define PN_CC_PACKED_PRE
#  if defined(__GNUC_PATCHLEVEL__)
#    define PN_CC_VERSION               "GCC C/C++ V" STR(__GNUC__) "." STR(__GNUC_MINOR__) "." STR(__GNUC_PATCHLEVEL__)
#  else
#    define PN_CC_VERSION               "GCC C/C++ V" STR(__GNUC__) "." STR(__GNUC_MINOR__)
#  endif

/* IAR Compiler */
#elif defined(__IAR_SYSTEMS_ICC__)
#  define CONFIG_CC_FORMAT_PRINTF
#  define CONFIG_CC_FORMAT_PRINTF_PRE   _Pragma("__printf_args")
#  define CONFIG_INLINE                 inline
#  define CONFIG_NORETURN
#  define CONFIG_NORETURN_PRE           __noreturn
#  define PN_CC_PACKED
#  define PN_CC_PACKED_PRE              __packed
#  define PN_CC_VERSION                 "IAR C/C++ V" STR(__VER__)

/* Renesas RX Compiler */
#elif defined(__RX)
/* checkbox must be set in project properties (Compiler/CPU/Advanced) */
#  pragma packoption
#  define CONFIG_CC_FORMAT_PRINTF
#  define CONFIG_CC_FORMAT_PRINTF_PRE
#  define CONFIG_NORETURN
#  define CONFIG_NORETURN_PRE
#  define PN_CC_PACKED
#  define PN_CC_PACKED_PRE
#  define PN_CC_VERSION                 "Renesas RX Compiler"

/* Microsoft Visual C++ */
#elif defined(_MSC_VER)
#  define _CRT_SECURE_NO_WARNINGS       1
#  define CONFIG_CC_FORMAT_PRINTF
#  define CONFIG_CC_FORMAT_PRINTF_PRE
#  define CONFIG_INLINE                 _inline
#  define CONFIG_NORETURN
#  define CONFIG_NORETURN_PRE
#  define PN_CC_PACKED
#  define PN_CC_PACKED_PRE
#  define PN_CC_VERSION                 "Microsoft Visual C/C++ V" STR(_MSC_VER)

/* Texas Instruments C Compiler with GCC Extensions
 * See: Build/ARM Compiler/Advanced Options/Language Options
 *      [x] Enable support for GCC extensions
 */
#elif defined(__TI_COMPILER_VERSION__)
#  define CONFIG_CC_FORMAT_PRINTF       __attribute__((format (printf, 5, 6)))
#  define CONFIG_CC_FORMAT_PRINTF_PRE
#  define CONFIG_INLINE                 inline
#  define CONFIG_NORETURN               __attribute__((noreturn))
#  define CONFIG_NORETURN_PRE
#  define PN_CC_PACKED                  __attribute__((packed))
#  define PN_CC_PACKED_PRE
#  define PN_CC_VERSION                 "TI C V" STR(__TI_COMPILER_VERSION__)

#else
#  error "Unknown compiler type. Please add defines to pn_oal_compiler.h"
#endif

#endif /* PN_OAL_COMPILER_H */

