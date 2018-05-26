

#ifndef PN_OAL_LOCK_INT_H
#define PN_OAL_LOCK_INT_H


#ifdef CONFIG_DEBUG_CONTEXT_LOCKING

/**< OAL context definition */
typedef enum {
    PN_OAL_CTX_NORMAL = 0,              /**< normal/user context */
    PN_OAL_CTX_TIMER = 1,               /**< IRQ: timer context */
    PN_OAL_CTX_ETHERNET = 2,            /**< IRQ: ethernet context */
} PN_OAL_CONTEXT_T;

extern volatile PN_OAL_CONTEXT_T gLockContext;  /**< global setable lock context */

#define PN_OAL_LOCK_CTX_ENTER(ctx) \
    PN_OAL_CONTEXT_T lockContext; \
    lockContext = gLockContext; \
    gLockContext = ctx
#define PN_OAL_LOCK_CTX_LEAVE() \
    gLockContext = lockContext

#else /* !CONFIG_DEBUG_CONTEXT_LOCKING */
#  define PN_OAL_LOCK_CTX_ENTER(ctx)
#  define PN_OAL_LOCK_CTX_LEAVE()
#endif


#endif /* PN_OAL_LOCK_INT_H */
