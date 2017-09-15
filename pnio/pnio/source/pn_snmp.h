

#ifndef PN_SNMP_H
#define PN_SNMP_H
#ifdef CONFIG_SNMP_INTERNAL


/* SNMP defines
-------------------------------------------------------------------------- */
#define PN_SNMP_SERVER_PORT             161


/* SNMP mapping macros
-------------------------------------------------------------------------- */
#define PN_SNMP_FUNC_INIT()             PN_snmpInit()
#define PN_SNMP_FUNC_SHUTDOWN()         PN_snmpShutdown()

/* prototypes */
IOD_STATUS_T PN_snmpInit(void);
void PN_snmpShutdown(void);


#else

#  if defined(CONFIG_SNMP_SUPPORT)
#    if !defined(PN_SNMP_FUNC_INIT) || \
        !defined(PN_SNMP_FUNC_SHUTDOWN)
#      error "Internal SNMP not used and SNMP macros not defined."
#    endif
#  else /* CONFIG_SNMP_SUPPORT */
#    define PN_SNMP_FUNC_INIT()             IOD_OK
#    define PN_SNMP_FUNC_SHUTDOWN()
#  endif /* CONFIG_SNMP_SUPPORT */


#endif /* CONFIG_SNMP_INTERNAL */
#endif /* PN_SNMP_H */
