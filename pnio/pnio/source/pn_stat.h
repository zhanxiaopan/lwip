

#ifndef PN_STAT_H
#define PN_STAT_H


/**< statistic type definition */
typedef enum {
    PN_STAT_MEM_OUT_OF_MEM_NET,         /**< mem: out of net buffers */
    PN_STAT_CFG_BUF_OOM,                /**< cfg: config buffer out-of-memory */
    PN_STAT_CRT_INPUT_OOM,              /**< crt: input buffer out-of-memory*/
    PN_STAT_CRT_INPUT_TX_DROP,          /**< crt: input TX dropped */
    PN_STAT_CRT_OUTPUT_OOM,             /**< crt: output buffer out-of-memory*/
    PN_STAT_LLDP_BUF_OOM,               /**< lldp: buffer out-of-memory */
    PN_STAT_OAL_RX_OOM,                 /**< oal: RX buffer out-of-memory */

    PN_STAT_END_MARKER
} PN_STAT_TYPE_T;


/* prototypes */
void PN_statInit(void);
void PN_statInc(PN_STAT_TYPE_T);


#endif /* PN_STAT_H */
