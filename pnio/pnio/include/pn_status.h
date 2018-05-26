

#ifndef PN_STATUS_H
#define PN_STATUS_H


/*! IOD Status Definition */
typedef enum
{
    IOD_OK = 0,                     /*! successful */
    IOD_DCP_IDENT_OTHER_DEVICE_OK,  /*! DCP identity request was for an other device */
    IOD_INDEX_SUPPORTED_OK,         /*! index supported */
    IOD_RECORD_BUSY_OK,             /*! postpone record data handling */
    IOD_OK_FRAME_TYPE_NOT_PNIO,     /*! frame doesn't contain PROFINET data */
    IOD_OK_OAL_SHUTDOWN,            /*! OAL: shutdown stack */

    IOD_OK_END_MARKER,              /*! OK Status End Marker */

    IOD_GEN_ERR,                    /*! generic error */
    IOD_INIT_ERR,                   /*! initialization failed */
    IOD_FULL_ERR,                   /*! no internal resources */
    IOD_ALLOC_ERR,                  /*! allocation failed */
    IOD_IO_ERR,                     /*! input/output error */
    IOD_OVERFLOW_ERR,               /*! overflow error */
    IOD_MISMATCH_ERR,               /*! mismatch error */
    IOD_OAL_MUTEX_ERR,              /*! OAL mutex error */
    IOD_OAL_TIMER_ERR,              /*! OAL: timer setup error */
    IOD_OAL_ETH_INIT_FAILED,        /*! OAL: Ethernet init failed */
    IOD_OAL_ETH_OPEN_FAILED,        /*! OAL: Ethernet open failed */
    IOD_EPWRONGTYPE_ERR,            /*! endpoint has wrong type */
    IOD_EPWRONGSTATE_ERR,           /*! endpoint has wrong state */
    IOD_NOTFOUND_ERR,               /*! not-found error */
    IOD_DCP_INIT_ERR,               /*! DCP module not initialized */
    IOD_DCP_SERVTYPE_INVAL_ERR,     /*! invalid DCP ServiceType */
    IOD_DCP_RESP_TIMER_ERR,         /*! could not create DCP response timer */
    IOD_DCP_UNKNOWN_SERVICEID_ERR,  /*! unknown DCP ServiceID received */
    IOD_DCP_IDENT_HANDLER_ERR,      /*! error in DCP identity request handler */
    IOD_DCP_GET_HANDLER_ERR,        /*! error in DCP get request handler */
    IOD_DCP_SET_HANDLER_ERR,        /*! error in DCP set request handler */
    IOD_DCP_PARAMETER_MISMATCH_ERR, /*! DCP parameter mismatch error */
    IOD_DCP_INVALID_DEVICE_NAME,    /*! DCP: invalid device name */
    IOD_DCP_INVALID_DEVICE_NAME_LEN,/*! DCP: invalid device name length */
    IOD_PARAM_ERR,                  /*! parameter error */
    IOD_RPCREQ_ERR,                 /*! RPC request error */
    IOD_ARGSLEN_INVALID,            /*! invalid argument length */
    IOD_NOT_SUPPORTED,              /*! unsupported error */
    IOD_FRAME_ERR,                  /*! frame parsing error */
    IOD_AR_ERR,                     /*! AR not found */
    IOD_API_ERR,                    /*! wrong API */
    IOD_SLOT_ERR,                   /*! slot not supported */
    IOD_SUBSLOT_ERR,                /*! subslot not supported */
    IOD_PADDING_ERR,                /*! padding error */
    IOD_INDEX_ERR,                  /*! index not supported */
    IOD_ERROR_SET,                  /*! error status was set */
    IOD_SEQNUM_ERR,                 /*! incorrect sequence number */
    IOD_NO_RESOURCES,               /*! no resource available */
    IOD_ALTYPE_ERR,                 /*! alarm type error */
    IOD_ALFRAMETYPE_ERR,            /*! alarm frame type error */
    IOD_ALTIMEOUT_ERR,              /*! alarm timeout error */
    IOD_ALRETRY_ERR,                /*! alarm retry error */
    IOD_ALDATALEN_ERR,              /*! alarm data length error */
    IOD_ALVLANID_ERR,               /*! alarm VLAN ID error */
    IOD_AR_UUID_NOT_FOUND,          /*! AR UUID not found */
    IOD_INTERNAL_ERR,               /*! internal error */
    IOD_ALPRIO_ERR,                 /*! alarm priority error */
    IOD_NULLPTR_ERR,                /*! null pointer error */
    IOD_BLOCKTYPE_ERR,              /*! block type error */
    IOD_BLOCKLEN_ERR,               /*! block length error */
    IOD_BLOCKVERH_ERR,              /*! block version high error */
    IOD_BLOCKVERL_ERR,              /*! block version low error */
    IOD_WRONG_SERVICE,              /*! wrong service error */
    IOD_WRONG_STATE,                /*! wrong state error */
    IOD_EP_NOT_EXIST,               /*! endpoint does not exist error */
    IOD_MODULE_NOT_FOUND,           /*! module not found error */
    IOD_SUBMODULE_NOT_FOUND,        /*! submodule not found error */
    IOD_AL_EP_NOT_EXIST,            /*! alarm endpoint does not exist error */
    IOD_SLOT_NOT_EXIST,             /*! slot does not exist error */
    IOD_SUBSLOT_NOT_EXIST,          /*! subslot does not exist error */
    IOD_SLOT_ALREADY_EXIST,         /*! slot already exist error */
    IOD_SUBSLOT_ALREADY_EXIST,      /*! subslot already exist error */
    IOD_EXP_NO_FREE_AR,             /*! no unused AR location found in expected list */
    IOD_EXP_NO_FREE_API,            /*! no unused API location found in expected list */
    IOD_EXP_NO_FREE_SLOT,           /*! no unused Slot location found in expected list */
    IOD_EXP_NO_FREE_SUBSLOT,        /*! no unused SubSlot location found in expected list */
    IOD_EXP_AR_NOT_FOUND,           /*! AR not found in expected list */
    IOD_FRAME_IGNORED,              /*! frame didn't match specific criterias and was ignored */
    IOD_MM_ALLOC_OOM,               /*! MM: out of memory */
    IOD_MM_QUEUE_NOT_FOUND,         /*! MM: queue ID not found */
    IOD_MM_QUEUE_EMPTY,             /*! MM: empty queue */
    IOD_MM_QUEUE_FULL,              /*! MM: full queue */
    IOD_MM_MEM_ERROR,               /*! MM: error in memory management */
    IOD_FULL_AP_LIST,               /*! no free entry in AP list found */
    IOD_FULL_SLOT_LIST,             /*! no free entry in slot list found */
    IOD_FULL_SUBSLOT_LIST,          /*! no free entry in subslot list found */
    IOD_FULL_MODULE_LIST,           /*! no free entry in module list found */
    IOD_FULL_SUBMOD_LIST,           /*! no free entry in submodule list found */
    IOD_API_NOT_FOUND,              /*! API not found */
    IOD_SLOT_NOT_FOUND,             /*! slot not found */
    IOD_GET_MAC_ADDR_FAILED,        /*! could not read MAC address */
    IOD_CFG_INIT_FAILED,            /*! configuration init failed */
    IOD_CFG_MISSING_INIT,           /*! Config: unitialized module */
    IOD_ELEMENT_NOT_FOUND,          /*! element not found */
    IOD_NVS_READ_FAILED,            /*! NVS read failed */
    IOD_NVS_WRITE_FAILED,           /*! NVS write failed */
    IOD_UDP_OPEN_FAILED,            /*! UDP: channel open failed */
    IOD_UDP_REOPEN_FAILED,          /*! UDP: channel reopen failed */
    IOD_UDP_RECV_FAILED,            /*! UDP: channel receive failed */
    IOD_UDP_SEND_FAILED,            /*! UDP: channel send failed */
    IOD_UDP_CLOSE_FAILED,           /*! UDP: channel close failed */
    IOD_RAW_OPEN_FAILED,            /*! RAW channel open failed */
    IOD_RAW_CLOSE_FAILED,           /*! RAW channel close failed */
    IOD_DCP_IN_PROGRESS,            /*! DCP: already processing a frame (delay not expired) */
    IOD_LOCK_CREATE_FAILED,         /*! Lock: creating lock failed */
    IOD_LOCK_GET_TIMEOUT_FAIL,      /*! Lock: get timeout reached without aquiring lock */
    IOD_TIMER_ALREADY_STARTED,      /*! Timer: timer already started */
    IOD_TIMER_NOT_USED,             /*! Timer: handle not assigned */
    IOD_TIMER_NOT_SET,              /*! Timer: unconfigured timer */
    IOD_TIMER_OUT_OF_TIMERS,        /*! Timer: out of timers */
    IOD_TIMER_CREATE_FAILED,        /*! Timer: failed to create timer */
    IOD_TIMER_START_FAILED,         /*! Timer: failed to start timer */
    IOD_TIMER_STOP_FAILED,          /*! Timer: failed to stop timer */
    IOD_IP_SET_FAILED,              /*! TCP/IP: could not update IP address configuration */
    IOD_NET_IP_INVALID,             /*! Net: IP address invalid */
    IOD_NET_GATEWAY_INVALID,        /*! Net: gateway address invalid */
    IOD_NET_NETMASK_INVALID,        /*! Net: netmask invalid */
    IOD_NET_UDP_OPEN_FAILED,        /*! Net: opening UDP channel failed */
    IOD_NET_UDP_CLOSE_FAILED,       /*! Net: closing UDP channel failed */
    IOD_NET_UDP_REOPEN_FAILED,      /*! Net: reopening UDP channels failed */
    IOD_NET_UDP_SEND_FAILED,        /*! Net: sending UDP data failed */
    IOD_NET_UDP_RECV_FAILED,        /*! Net: reading UDP data failed */
    IOD_NET_FRAME_SEND_FAILED,      /*! Net: sending frame failed */
    IOD_THREAD_CREATE_FAILED,       /*! OAL: creating thread failed */
    IOD_THREAD_CLOSE_FAILED,        /*! OAL: closing thread failed */
    IOD_PDEV_PLUG_FAILED,           /*! PDEV: plugging (sub)module failed */
    IOD_SNMP_INIT_FAILED,           /*! SNMP: init failed */
    IOD_MULTICAST_ADD_FAILED,       /*! Net: failed to add multicast address */
    IOD_MULTICAST_DEL_FAILED,       /*! Net: failed to del multicast address */
    IOD_EP_OFFSET_OVERFLOW,         /*! CRT: endpoint offset length overflow */
    IOD_RPC_SESSION_NOT_AVAIL,      /*! RPC: no free session available */
    IOD_RPC_SESSION_NOT_FOUND,      /*! RPC: session not found */
    IOD_RPC_SESSION_INVALID,        /*! RPC: invalid session */
    IOD_RPC_SESSION_LOCK_FAILED,    /*! RPC: failed to lock session */
    IOD_RPC_SESSION_UNLOCK_FAILED,  /*! RPC_ failed to unlock session */
    IOD_DIAG_AVAIL,                 /*! diagnosis available */
    IOD_NOT_FOUND,                  /*! generic: not found */
    IOD_SUBSLOT_TYPE_NOT_INPUT,     /*! subslot hasn't type input */
    IOD_SUBSLOT_TYPE_NOT_OUTPUT,    /*! subslot hasn't type output */
    IOD_SIZE_OVERFLOW,              /*! data size too large */
} IOD_STATUS_T;


/*! PROFINET stack generic return type */
typedef enum {
    /* Return value OK */
    PN_OK,                              /*! successful */
    PN_OK_MODULE_ALREADY_PLUGGED,       /*! no error - module already plugged */
    PN_OK_SUBMODULE_ALREADY_PLUGGED,    /*! no error - submodule already plugged */
    PN_OK_MODULE_ALREADY_PULLED,        /*! no error - module already pulled */
    PN_OK_SUBMODULE_ALREADY_PULLED,     /*! no error - submodule already pulled */
    PN_OK_END_MARKER,                   /*! no error - end marker for no-error codes */

    /* Return value ERROR */
    PN_E_API_NOT_EXIST,                 /*! error - API does not exist */
    PN_E_SLOT_NOT_EXIST,                /*! error - slot does not exist */
    PN_E_SUBSLOT_NOT_EXIST,             /*! error - subslot does not exist */
    PN_E_INDEX_NOT_EXIST,               /*! error - index does not exist */
    PN_E_EP_NOT_EXIST,                  /*! error - endpoint does not exist */
    PN_E_SIZE_MISMATCH,                 /*! error - size mismatch */
    PN_E_WRONG_TYPE,                    /*! error - wrong type */
    PN_E_OAL_INIT_ERROR,                /*! error - OAL initialization failed */
    PN_E_LMPM_INIT_ERROR,               /*! error - LMPM initialization failed */
    PN_E_LLDP_INIT_ERROR,               /*! error - LLDP initialization failed */
    PN_E_DCP_INIT_ERROR,                /*! error - DCP initialization failed */
    PN_E_CRT_INIT_ERROR,                /*! error - CRT initialization failed */
    PN_E_ALARM_INIT_ERROR,              /*! error - Alarm initialization failed */
    PN_E_TIMER_INIT_ERROR,              /*! error - Timer initialization failed */
    PN_E_NET_INIT_ERROR,                /*! error - Network initialization failed */
    PN_E_SNMP_INIT_ERROR,               /*! error - SNMP initialization failed */
    PN_E_AR_NOT_FOUND,                  /*! error - application relation not found */
    PN_E_AR_INIT_ERROR,                 /*! error - application relation init failed */
    PN_E_RPC_ERROR,                     /*! error - RPC failed */
    PN_E_STATE_CONFLICT,                /*! error - state conflict */
    PN_E_INTERNAL,                      /*! error - internal error */
    PN_E_STARTUP_ERROR,                 /*! error - startup failed */
    PN_E_SLOT_NOT_EMPTY,                /*! error - slot is not empty */
    PN_E_SUBSLOT_NOT_EMPTY,             /*! error - subslot is not empty */
    PN_E_MODULE_NOT_FOUND,              /*! error - module not found */
    PN_E_SUBMODULE_NOT_FOUND,           /*! error - submodule not found */
    PN_E_MODULE_IDENT_NOT_MATCH,        /*! error - module identification does not match */
    PN_E_SUBMODULE_IDENT_NOT_MATCH,     /*! error - submodule identification does not match */
    PN_E_MODULE_NOT_SET,                /*! error - module not set */
    PN_E_SUBMODULE_NOT_SET,             /*! error - submodule not set */
    PN_E_ALARM_ERROR,                   /*! error - alarm error */
    PN_E_STATE_UPDATE_FAILED,           /*! error - state update failed */
    PN_E_INDEX_ERROR,                   /*! error - index error */
    PN_E_CD_INIT_ERROR,                 /*! error - CD initialization failed */
    PN_E_IP_SET,                        /*! error - IP configuration could not be set */
    PN_E_OPTION_NOT_FOUND,              /*! error - option could not be found */
    PN_E_WRONG_LENGTH,                  /*! error - length mismatch */
    PN_E_RECORD_DATA_BUSY_ERR,          /*! error - while handling busy record data */
    PN_E_PULL_SUBMODULE_ERROR,          /*! error - while pulling submodule */
    PN_E_READ_IMPLICIT_WITH_AR_NOT_NIL, /*! error - read implicit with AR not set to zero */
    PN_E_MM_INIT_ERROR,                 /*! error - could not init memory management */
    PN_E_DEV_INIT_ERROR,                /*! error - could not init device structures (slot/mod/params) */
    PN_E_CFG_INIT_ERROR,                /*! error - could not init configuration storage */
    PN_E_DIAG_INIT_ERROR,               /*! error - could not init diagnosis module */

    /* Return values for User Indication functions! */
    PN_IND_BUSY,                        /*! info - device is busy, request was queued */
    PN_IND_READ_ERROR,                  /*! error - read failed */
    PN_IND_WRITE_ERROR,                 /*! error - write failed */
    PN_IND_ERROR                        /*! error - unspecified error occured */

} RET_T;

#endif /* PN_STATUS_H */
