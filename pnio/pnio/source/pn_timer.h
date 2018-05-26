

#ifndef PN_TIMER_H
#define PN_TIMER_H


/*****************************************************************************/
/* defines */
/*****************************************************************************/
#define CONFIG_TIMER_MAX                (CONFIG_MAX_TIMERS_LOW + CONFIG_MAX_TIMERS_HIGH)


/*****************************************************************************/
/* enums */
/*****************************************************************************/

typedef enum {
    PN_TIMER_STATE_FREE = 0,
    PN_TIMER_STATE_USED,
    PN_TIMER_STATE_SETUP,
    PN_TIMER_STATE_ACTIVE,
    PN_TIMER_STATE_RUN
} PN_TIMER_STATE_T;


/*****************************************************************************/
/* structures */
/*****************************************************************************/

/**< timer description */
typedef struct {
    PN_TIMER_STATE_T  state;            /**< timer state */
    PN_TIMER_STATE_T  stateNext;        /**< timer state next */
    PN_TIMER_CB_T     pFunc;            /**< timer function */
    void             *pArg;             /**< timer function argument */
    PN_TIMESTAMP_T    expire;           /**< timeout */
    PN_TIMESTAMP_T    period;           /**< period */
    PN_TIMER_TYPE_T   type;             /**< type */
} PN_TIMER_T;


/*****************************************************************************/
/* prototypes */
/*****************************************************************************/
IOD_STATUS_T PN_timerInit(void);
IOD_STATUS_T PN_timerCreate(PN_TIMER_ID_T *pId, PN_TIMER_PRIO_T prio);
IOD_STATUS_T PN_timerSetup(PN_TIMER_ID_T id, PN_TIMER_TYPE_T type, Unsigned32 period, PN_TIMER_CB_T pFunc, void *pArg, PN_BOOL_T startFlag);
IOD_STATUS_T PN_timerStart(PN_TIMER_ID_T id);
IOD_STATUS_T PN_timerStop(PN_TIMER_ID_T id);
void PN_timerRun(void);


#endif /* PN_TIMER_H */
