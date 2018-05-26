

#ifndef PN_UTILS_H
#define PN_UTILS_H


/* list of public functions
---------------------------------------------------------------------------*/
IOD_STATUS_T PN_utilVerifyReqBlock(const BLOCK_HEADER_T *, Unsigned16, Unsigned16, ERROR_STATUS_T *);
void swapUUID(UUID_T *);
Unsigned32 PN_utilCrcFletcher32(Unsigned16 const *, Unsigned32);
void PN_utilPadWithBlanks(Unsigned8 *str, unsigned int strLen, unsigned int fillLen);
void PN_utilRandInit(Unsigned32 val);
Unsigned32 PN_utilRand(void);
void PN_utilRandStream(Unsigned32 *pArea, unsigned int len);
PN_TIMESTAMP_T PN_utilTs32(Unsigned32 ms);
PN_TIMESTAMP_T PN_utilTsAdd(PN_TIMESTAMP_T left, PN_TIMESTAMP_T right);
PN_TIMESTAMP_T PN_utilTsSub(PN_TIMESTAMP_T left, PN_TIMESTAMP_T right);
PN_BOOL_T PN_utilTsIsLE(PN_TIMESTAMP_T left, PN_TIMESTAMP_T right);
PN_BOOL_T PN_utilIsDigitRow(const char *str, unsigned int len);
PN_BOOL_T PN_utilIsVisibleString(const unsigned char *pStr, unsigned int len);
PN_BOOL_T PN_utilIsIm2DateTime(const unsigned char *pStr, unsigned int len);


/***************************************************************************/
/** Macro: Clear an error code.
 *
 * This macro clears all 4 elements of the errorStatus variable. It sets
 * errorCode, errorCode1, errorCode2 and errorDecode to zero.
 */
#define CLEAR_ERROR_CODE() \
    do { \
        errorStatus->errorCode = 0; \
        errorStatus->errorCode1 = 0; \
        errorStatus->errorCode2 = 0; \
        errorStatus->errorDecode = 0; \
    } while (0)


/***************************************************************************/
/** Macro: Check if an error code is set.
 *
 * This macro checks all 4 elements of the errorStatus variable if at least one
 * of them is non-zero. It checks errorCode, errorCode1, errorCode2 and
 * errorDecode.
 */
#define IS_ERROR_CODE_SET() \
    ((0 != errorStatus->errorCode) || \
     (0 != errorStatus->errorCode1) || \
     (0 != errorStatus->errorCode2) || \
     (0 != errorStatus->errorDecode))


/***************************************************************************/
/** Macro: Mark an element as unused.
 *
 * This macro works around the circumstance that not every comiler has the
 * ability to mark a function parameter as unused. It should have no runtime
 * impact, as normal compilers just optimize the macro away.
 */
#define UNUSEDARG(x) ((void)(x))


/***************************************************************************/
/** Macro: Set an error if a condition is true.
 *
 * This macro checks the given condition if the local variable errCode2 is
 * zero. If the condition is false, it sets the errCode2 variable to errorCode.
 */
#define CHECK_PAR(condition, errorCode) \
    if ((!errCode2) && (condition))  { \
        errCode2 = errorCode; \
        PN_logErr("CHECK_PAR failed. errCode2: %d", errCode2); \
    }


/***************************************************************************/
/** Macro: Halt the device if a pointer is zero
 *
 * This macro halts the device if a given pointer is zero.
 */
#define PN_HALT_IF_NULL(x) if (!x) { OAL_halt(); }


/***************************************************************************/
/** Macro: Halt the device if a structure isn't active
 *
 * This macro halts the device if the active flag of a given structure pointer
 * isn't true.
 */
#define PN_HALT_IF_INACTIVE(x) if (PN_TRUE != x->active) { OAL_halt(); }


/***************************************************************************/
/** Macro: Halt the device if a condition doesn't match
 *
 * This macro halts the device if condition doesnt match - like ASSERT.
 */
#define PN_HALT_IF_FALSE(x) if (!(x)) { OAL_halt(); }


#endif /* PN_UTILS_H */
