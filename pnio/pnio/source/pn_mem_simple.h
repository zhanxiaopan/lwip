

#ifndef PN_MEM_SIMPLE_H
#define PN_MEM_SIMPLE_H

#ifndef CONFIG_MEM_MGMT_SIZE
#  define PN_CONFIG_MEM_MGMT_SIZE   65536
#else
#  define PN_CONFIG_MEM_MGMT_SIZE   CONFIG_MEM_MGMT_SIZE
#endif

#ifdef CONFIG_DEBUG_MEM_FENCES
#  define PN_MEM_MGMT_WATCH_LEN     sizeof(Unsigned32)      /**< length of watch entry */
#  define PN_MEM_MGMT_WATCH_CONT    0x9B71DCAB              /**< watch entry content */
#else
#  define PN_MEM_MGMT_WATCH_LEN     0
#endif
#define PN_MEM_MGMT_SIZE_LEN        sizeof(Unsigned32)      /**< length of size entry */


/**< global static memory structure */
typedef struct {
#ifdef CONFIG_DEBUG_MEM_FENCES
    Unsigned32 watchPre;            /**< pre-fence */
#endif

    volatile Unsigned32 used;       /**< global static memory usage index */
    volatile Unsigned32 cnt;        /**< allocation count */
    PN_LOCK_T mtx;                  /**< area mutex */
    char area[PN_CONFIG_MEM_MGMT_SIZE];

#ifdef CONFIG_DEBUG_MEM_FENCES
    Unsigned32 watchPost;           /**< post-fence */
#endif
} PN_MEM_AREA_T;


/* Function Prototypes */
IOD_STATUS_T PN_memInit(void);
IOD_STATUS_T PN_memAllocAlign(char **ppMem, Unsigned32 memLen, Unsigned32 align);
IOD_STATUS_T PN_memAlloc(char **ppMem, Unsigned32 memLen);
IOD_STATUS_T PN_memCalloc(char **ppMem, Unsigned32 memLen);
IOD_STATUS_T PN_memCallocAlign(char **ppMem, Unsigned32 memLen, Unsigned32 align);
IOD_STATUS_T PN_memCheck(void);
void PN_memDump(void);


#endif /* PN_MEM_SIMPLE_H */
