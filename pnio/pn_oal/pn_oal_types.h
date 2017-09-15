

#ifndef PN_OAL_TYPES_H
#define PN_OAL_TYPES_H

#include <inttypes.h> /* platform integer types (8, 16, 32 bit) */

#include "pn_oal_compiler.h"


/* printf defines for architecture specific integer types
 *
 * Each value must be defined without the %-sign, only e.g. i or li should be
 * defined as string.
 *
 *   FMT_d32     %d/%ld
 *   FMT_i32     %i/%li
 *   FMT_u32     %u/%lu
 *   FMT_x32     %x/%lx
 *   FMT_size_t  %u/%lu
 */
#define FMT_d32      PRId32
#define FMT_i32      PRIi32
#define FMT_u32      PRIu32
#define FMT_x32      PRIx32
#define FMT_size_t   "u"


/* length where UDP header ends and data starts
   14 - ethernet header (src MAC, dst MAC, type)
    4 - VLAN header
   20 - IPv4 header
    8 - UDP header
*/
#define ETH_IP_UDP_HDR (14 + 4 + 20 + 8)


/* Network and Frame Handling Defines
 *
 * CONFIG_TARGET_ETH_PORT_COUNT - number of available Ethernet ports
 * CONFIG_TARGET_ETH_BUF_RX - number of RX buffers
 * CONFIG_TARGET_ETH_BUF_TX - number of TX buffers
 */
#define CONFIG_TARGET_ETH_PORT_COUNT 1
#define CONFIG_TARGET_ETH_BUF_RX 16
#define CONFIG_TARGET_ETH_BUF_TX 2


/* Endianness Setting
 *
 * Available defines:
 *   CONFIG_TARGET_BIG_ENDIAN
 *   CONFIG_TARGET_LITTLE_ENDIAN
 *
 * If unsure, enable CONFIG_LOGGING and CONFIG_LOGGING_DETAILS_SYSINFO to let
 * the device detect the endianness on runtime.
 */
#define CONFIG_TARGET_LITTLE_ENDIAN 1


/* Memory Alignment
 *
 * This is used to align memory regions.
 */
#define OAL_MEM_ALIGN_CPU 4
#define OAL_MEM_ALIGN_NET 4


/* portable data type definitions */
typedef int8_t    Integer8;     /**< signed integer 8 bit */
typedef int16_t   Integer16;    /**< signed integer 16 bit */
typedef int32_t   Integer32;    /**< signed integer 32 bit */
typedef int64_t   Integer64;    /**< signed integer 64 bit */
typedef uint8_t   Unsigned8;    /**< unsigned integer 8 bit */
typedef uint16_t  Unsigned16;   /**< unsigned integer 16 bit */
typedef uint32_t  Unsigned32;   /**< unsigned integer 32 bit */
typedef uint64_t  Unsigned64;   /**< unsigned integer 64 bit */
typedef long      PtrCast;      /**< pointer casting helper */

typedef int OAL_UDPCHAN_T;      /**< UDP socket descriptor */
typedef int OAL_LOCK_BIN_T;     /**< binary lock handle */
typedef int OAL_LOCK_CNT_T;     /**< counting lock handle */


#endif /* PN_OAL_TYPES_H */
