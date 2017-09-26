

#ifndef PN_OAL_H
#define PN_OAL_H

#include <string.h>

#include <stdbool.h>
#include <ctype.h>
#include "psnprintf.h"


/* C abstraction defines
---------------------------------------------------------------------------*/
#define OAL_MEMCPY              memcpy
#define OAL_MEMSET              memset
#define OAL_MEMCMP              memcmp
#define OAL_STRLEN              strlen
#define OAL_STRNLEN             pstrnlen
#define OAL_STRNCPY             strncpy
#define OAL_ASSERT(x)           { if (!(x)) { PN_logErr("Assert: %s", #x); OAL_halt(); } }
#define OAL_SNPRINTF            psnprintf
#define OAL_VSNPRINTF           pvsnprintf
#define OAL_ISALNUM             isalnum
#define OAL_ISDIGIT             isdigit



#endif /* PN_OAL_H */
