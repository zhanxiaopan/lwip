/*
 *            Copyright (c) 2002-2013 by Real Time Automation, Inc.
 *
 *  This software is copyrighted by and is the sole property of
 *  Real Time Automation, Inc. (RTA).  All rights, title, ownership,
 *  or other interests in the software remain the property of RTA.
 *  This software may only be used in accordance with the corresponding
 *  license agreement.  Any unauthorized use, duplication, transmission,
 *  distribution, or disclosure of this software is expressly forbidden.
 *
 *  This Copyright notice MAY NOT be removed or modified without prior
 *  written consent of RTA.
 *
 *  RTA reserves the right to modify this software without notice.
 *
 *  Real Time Automation
 *  150 S. Sunny Slope Road            USA 262.439.4999
 *  Suite 130                          www.rtaautomation.com
 *  Brookfield, WI 53005               software@rtaautomation.com
 *
 *************************************************************************
 *
 *    Version Date: 05NOV2013
 *         Version: 2.37
 *    Conformed To: EtherNet/IP Protocol Conformance Test A-10 (28-AUG-2012)
 *     Module Name: eips_struct.h
 *          Author: Jamin D. Wendorf (jwendorf@rtaautomation.com)
 *        Language: Ansi C
 * Compile Options: N/A
 * Compile defines: N/A
 *       Libraries: N/A
 *    Link Options: N/A
 *
 * Description.
 * =======================================================================
 * This file contains structure definitions.
 *
 */

#ifndef __EIPS_STRUCT_H__
#define __EIPS_STRUCT_H__

/* --------------------------------------------------------------- */
/*	STRUCTURE DEFINITIONS					   */
/* --------------------------------------------------------------- */
/* ====================================================================
SOCKET STRUCTURES

Socket structures are used to simplify TCP/IP processing.
======================================================================= */
typedef struct
{
   uint8 sock_type;
   EIPS_USERSYS_SOCKTYPE sock_id; /* used for TCP */
   uint32 sock_addr; /* used for UDP */
   uint16 sock_port; /* used for UDP */
}EIPS_SOCKET_STRUCT;

/* ====================================================================
CONNECTION DATA STRUCTURE

The Connection Data structure contains all of the data needed by the
connection class to manage a connection.  This data isn't needed over
the network, therefore not supported by the object model.
======================================================================= */
typedef struct
{
/* 0001 */  uint8  priority_and_tick;
/* 0002 */  uint8  connection_time_out_ticks;
/* 0006 */  uint32 O2T_CID;
/* 0010 */  uint32 T2O_CID;
/* 0012 */  uint16 connection_serial_number;
/* 0014 */  uint16 vendor_ID;
/* 0018 */  uint32 originator_serial_number;
/* 0019 */  uint8  connection_timeout_multiplier;
/* 0023 */  uint32 O2T_RPI;
/* 0025 */  uint16 O2T_cnxn_params; /* name change with lrg_fwd_open to help find all changes */
/* 0027 */  uint16 O2T_cnxn_size;   /* new field since lrg_fwd_open is 16 bit */
/* 0031 */  uint32 T2O_RPI;
/* 0033 */  uint16 T2O_cnxn_params; /* name change with lrg_fwd_open to help find all changes */
/* 0035 */  uint16 T2O_cnxn_size;   /* new field since lrg_fwd_open is 16 bit */
/* 0036 */  uint8  xport_type_and_trigger;
/* 0037 */  uint8  connection_path_size;
/* 0101 */  uint16 connection_path[205]; /* config data needs 400 bytes, plus encoding */
/* 0105 */  uint32 O2T_API;
/* 0109 */  uint32 T2O_API;

  /* Connection Variables (not in ForwardOpen) */
/* 0110 */  uint8  state;
/* 0114 */  uint32 SessionID;
/* 0116 */  uint16 O2T_TimerNum;
/* 0118 */  uint16 O2T_SeqNum;
/* 0120 */  uint16 T2O_TimerNum;
/* 0122 */  uint16 T2O_SeqNum;

/* 0119 */  uint16 unused_index; /* used to find a connection in the list easier */
}EIPS_CONNECTION_DATA_STRUCT;

/* ====================================================================
   I/O CONNECTION STRUCTURE
======================================================================= */
typedef struct
{
    uint8  state; /* used to determine if an instance is used */
    uint16 index; /* used to find a connection in the list easier */

#ifdef EIPS_QOS_USED
    uint8 dscp_type;
    uint8 dscp_value;
#endif

    EIPS_CONNECTION_DATA_STRUCT CnxnStruct;

    /* ***************** */
    /* Control Variables */
    /* ***************** */
    uint8   IO_Type;    /* Input Only, Listen Only, Exclusive Owner */
    uint32  RunIdleInfo;

    /* **************** */
    /* O_to_T Variables */
    /* **************** */
    uint32  O2T_IPAddr;
    uint16  O2T_Port;
    uint32  O2T_SeqNum;
    uint16  O2T_InstID;
    uint16  O2T_Size; /* in units (uint8, uint16 or uint32) */

#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
    uint32 * O2T_DataPtr;
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
    uint16 * O2T_DataPtr;
#else                                       /* BYTES */
    uint8 *  O2T_DataPtr;
#endif

    /* **************** */
    /* T_to_O Variables */
    /* **************** */
    uint32  T2O_IPAddr;
    uint16  T2O_Port;
    uint32  T2O_SeqNum;
    uint16  T2O_InstID;
    uint16  T2O_Size; /* in units (uint8, uint16 or uint32) */

#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
    uint32 * T2O_DataPtr;
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
    uint16 * T2O_DataPtr;
#else                                       /* BYTES */
    uint8 *  T2O_DataPtr;
#endif
}EIPS_IOCNXN_STRUCT;

/* ====================================================================
   SESSION STRUCTURE (CLIENT SPECIFIC DATA)
======================================================================= */
typedef struct
{
   uint32 SessionHandle;
   EIPS_USERSYS_SOCKTYPE    sock_id;
   EIPS_CONNECTION_DATA_STRUCT *CnxnPtrs[EIPS_USER_MAX_NUM_EM_CNXNS_PER_SESSION];
}EIPS_SESSION_STRUCT;

/* ====================================================================
   ENCAPSULATED MESSAGE STRUCTURE (USED FOR ENCAPSULATION PROCESSING)
======================================================================= */
typedef struct
{
  /*0x00*/ uint16 Command;
  /*0x02*/ uint16 Length;
  /*0x04*/ uint32 SessionHandle;
  /*0x08*/ uint32 Status;
  /*0x0C*/ uint8  SenderContext[8];
  /*0x0D*/ uint32 Options;
  /* I need a pointer to the rest of the data, this way I don't need to
     define the pointer when I use this structure as a pointer to overlay
     over the passed buffer's memory */
  /*0x11*/ uint8  *EncapsulatedData;
}EIPS_ENCAP_MSG_STRUCT;

typedef struct
{
   uint16 TypeId;
   uint16 Length;
   uint8  Data[EIPS_CPF_MAX_ITEM_DATA_LEN];
}EIPS_CPF_ITEMSTRUCT;

typedef struct
{
   uint8  service;	 /* 0 - 0x7F */
   uint8  path_size;	 /* 0 - EIPS_USER_MSGRTR_PATH_WORDS */
   uint16 path[EIPS_USER_MSGRTR_PATH_WORDS];
   uint16 req_data_size; /* 0 - EIPS_USER_MSGRTR_DATA_SIZE */
   uint8  req_data[EIPS_USER_MSGRTR_DATA_SIZE];
}EIPS_USER_MSGRTR_REQ_FMT;

typedef struct
{
   uint8  gen_stat;	  /* 0 = Success; 1-255 = Error        */
   uint8  ext_stat_size;  /* 0 - EIPS_USER_MSGRTR_EXTSTAT_SIZE */
   uint16 ext_stat[EIPS_USER_MSGRTR_EXTSTAT_SIZE];
   uint16 rsp_data_size;  /* 0 - EIPS_USER_MSGRTR_DATA_SIZE    */
   uint8  rsp_data[EIPS_USER_MSGRTR_DATA_SIZE];
}EIPS_USER_MSGRTR_RSP_FMT;

typedef struct
{
   EIPS_USER_MSGRTR_REQ_FMT	user_struct;
}EXPLICIT_REQUEST_STRUCT;

typedef struct
{
   uint8 service;
   uint8 reserved;
   EIPS_USER_MSGRTR_RSP_FMT	user_struct;
}EXPLICIT_RESPONSE_STRUCT;

typedef struct
{
    EIPS_USERSYS_SOCKTYPE   sock_id;    /* needed for I/O Forward Open */

    uint8   *encap_data;        /* local pointer to encap data */
    int16   bytes_used;         /* validate enough data for items */
    uint32  InterfaceHandle;    /* should be 0 */
    uint16  Timeout;            /* Timeout >0 in seconds */
    uint16  ItemCount;          /* store the number of items */
    uint32  ConnectionID;       /* Used with connected messages */
    uint16  SequenceNumber;     /* Used with connected messages */
    EIPS_ENCAP_MSG_STRUCT       *encap; /* Pointer to encapsulated message */
    EIPS_CPF_ITEMSTRUCT         Items[EIPS_CPF_MAX_ITEM_CNT]; /* store the items */
    EXPLICIT_REQUEST_STRUCT     *emreq; /* EM Request Message Pointer */
    EXPLICIT_RESPONSE_STRUCT    *emrsp; /* EM Response Message Pointer */
}CPF_MESSAGE_STRUCT;

typedef struct
{
/*  1 Byte  */ uint8  SrvCdMsk;
/*  1 Byte  */ uint8  ClassID;
/*  1 Byte  */ uint8  InstID;
/*  1 Byte  */ uint8  AttrID;
/*  1 Byte  */ uint8  Type;
/*  3 Bytew */ void   *DataPtr;
/*  3 Bytes */ void   (*FnPtr) (CPF_MESSAGE_STRUCT *cpf);
/* ======== */
/* 11 Bytes Total Per Entry */
}EIPS_GETSETATTR_TABLE_STRUCT;

/* ====================================================================
0x01 IDENTITY OBJECT ATTRIBUTES

The Identity Attributes structure contains the list of Identity
attributes. It is used to make these attibutes global to the system.
======================================================================= */
#define EIPS_STRUCT_MAX_PROD_NAME_SIZE  32
typedef struct
{
  struct
  {
/* ATTR 01 */ uint16  ClassRev;
  }Class;

  struct
  {
/* ATTR 01 */   uint16  Vendor;
/* ATTR 02 */   uint16  DeviceType;
/* ATTR 03 */   uint16  ProductCode;
/* ATTR 04 */   uint16  Revision;
/* ATTR 05 */   uint16  Status;
/* ATTR 06 */   uint32  SerialNum;
/* ATTR 07 */   uint8   ProductName[32];  /* user defined */
                uint8   SizOfName;        /* user defined */
/* ATTR 08 */   uint8   State;
  }Inst;
}EIPS_IDENTITY_OBJ_STRUCT;

/* ====================================================================
0x04 ASSEMBLY OBJECT ATTRIBUTES

The Assembly Attributes structure contains the list of Assembly
Instances and sizes.
======================================================================= */
typedef struct
{
   uint16   Inst;           /* Instance ID */
    uint8   RunIdle32Used;  /* set to TRUE if the RUN/IDLE Header is used */
   uint16   DataSize;       /* in units (uint8, uint16, or uint32) */
    uint8   Owned;          /* Outputs are owned */

#if EIPS_USEROBJ_ASM_ELEMENT_SIZE == 4      /* DWORDS */
    uint32 * DataPtr;
#elif EIPS_USEROBJ_ASM_ELEMENT_SIZE == 2    /* WORDS */
    uint16 * DataPtr;
#else                                       /* BYTES */
    uint8  * DataPtr;
#endif
}EIPS_ASSEMBLY_INST_STRUCT;

/* ====================================================================
0x48 QoS OBJECT ATTRIBUTES

The QoS Attributes structure contains the list of QoS Attributes.
It is used to make these attibutes global to the system.
======================================================================= */
#ifdef EIPS_QOS_USED
#define EIPS_QOS_DFLT_DSCP_URGENT      55
#define EIPS_QOS_DFLT_DSCP_SCHEDULED   47
#define EIPS_QOS_DFLT_DSCP_HIGH        43
#define EIPS_QOS_DFLT_DSCP_LOW         31
#define EIPS_QOS_DFLT_DSCP_EXPLICIT    27

#define EIPS_QOS_DSCP_TYPE_NOTUSED      0   /* don't mark the DSCP field */
#define EIPS_QOS_DSCP_TYPE_URGENT       1
#define EIPS_QOS_DSCP_TYPE_SCHEDULED    2
#define EIPS_QOS_DSCP_TYPE_HIGH         3
#define EIPS_QOS_DSCP_TYPE_LOW          4
#define EIPS_QOS_DSCP_TYPE_EXPLICIT     5
typedef struct
{
  struct
  {
/* ATTR 01 */ uint16  ClassRev;
  }Class;

  struct
  {
/* ATTR 04 */   uint8  DSCP_Urgent;
/* ATTR 05 */   uint8  DSCP_Scheduled;
/* ATTR 06 */   uint8  DSCP_High;
/* ATTR 07 */   uint8  DSCP_Low;
/* ATTR 08 */   uint8  DSCP_Explicit;
  }Inst;
}EIPS_QOS_OBJ_STRUCT;
#endif

/* ====================================================================
0xF5 TCP/IP INTERFACE OBJECT ATTRIBUTES

The TCP/IP Interface Attributes structure contains the list of TCP/IP
Interface Attributes. It is used to make these attibutes global to the
system.
======================================================================= */
typedef struct
{
  uint16  Bufsiz;
  uint8   Buf[64];
}EIPS_TCPOBJ_BUF_STRUCT;

typedef struct
{
  uint16  PathSiz;
  uint8   Path[12];
}EIPS_PHY_LINK_STRUCT;

typedef struct
{
  uint32 IPAddress;
  uint32 NtwkMask;
  uint32 GtwyIPAddress;
  uint32 NameServer;
  uint32 NameServer2;
  EIPS_TCPOBJ_BUF_STRUCT DomainName;
}EIPS_IPADDR_STRUCT;

typedef struct
{
    uint8   alloc_ctrl;
    uint16  num_mcast;
    uint32  mcast_start_addr;
}EIPS_TCPOBJ_MCASTCFG_STRUCT;

typedef struct
{
    uint8   acd_activity;
    uint8   remote_mac[6];
    uint8   arp_pdu28[28];
}EIPS_TCPOBJ_LAST_ACD_CONFLICT_STRUCT;

typedef struct
{
  struct
  {
     uint16  ClassRev;
  }Class;

/* pass all instance values from the user */
}EIPS_TCP_OBJ_STRUCT;

/* ====================================================================
0xF6 ETHERNET LINK OBJECT ATTRIBUTES

The Ethernet Link Attributes structure contains the list of Ethernet
Link Attributes. It is used to make these attibutes global to the
system.
======================================================================= */
typedef struct
{
    uint32 InOctets;
    uint32 InUcastPackets;
    uint32 InNUcastPackets;
    uint32 InDiscards;
    uint32 InErrors;
    uint32 InUnknownProtos;
    uint32 OutOctets;
    uint32 OutUcastPackets;
    uint32 OutNUcastPackets;
    uint32 OutDiscards;
    uint32 OutErrors;
}EIPS_ELINKOBJ_INTFCNTS_STRUCT;

typedef struct
{
    uint32 AlignmentErrors;
    uint32 FCSErrors;
    uint32 SingleCollisions;
    uint32 MultipleCollisions;
    uint32 SQETestErrors;
    uint32 DeferredTransmissions;
    uint32 LateCollisions;
    uint32 ExcessiveCollisions;
    uint32 MACTransmitErrors;
    uint32 CarrierSenseErrors;
    uint32 FrameTooLarge;
    uint32 MACReceiveErrors;
}EIPS_ELINKOBJ_MEDIACNTS_STRUCT;

typedef struct
{
    uint16 ControlBits;
    uint16 ForcedInterfaceSpeed;
}EIPS_ELINKOBJ_INTFCTRL_STRUCT;

typedef struct
{
  struct
  {
/* ATTR 01 */ uint16  ClassRev;
  }Class;

#if 0 /* pass all values from the user */
  struct
  {
/* ATTR 01 */ uint32    InterfaceSpeed;
/* ATTR 02 */ uint32    InterfaceFlags;
/* ATTR 03 */ uint8     PhysicalAddress[6];
/* ATTR 04 */ EIPS_ELINKOBJ_INTFCNTS_STRUCT     InterfaceCnts;
/* ATTR 05 */ EIPS_ELINKOBJ_MEDIACNTS_STRUCT    MediaCnts;
/* ATTR 06 */ EIPS_ELINKOBJ_INTFCTRL_STRUCT     InterfaceControl;
  }Inst;
#endif
}EIPS_ELINK_OBJ_STRUCT;

#ifdef EIPS_TACL_OBJ_USED

/* TaCL Connection Point groupings */
typedef struct
{
    uint8 o2t_inst;
    uint8 t2o_inst;
    uint8 cfg_inst;
    char  name[33];
}EIPS_TACL_GROUPING_STRUCT;

/* instance types */
#define EIPS_TACL_OBJ_INST_TYPE_EXOWNER         1
#define EIPS_TACL_OBJ_INST_TYPE_INONLY          2
#define EIPS_TACL_OBJ_INST_TYPE_LISTENONLY      3
#define EIPS_TACL_OBJ_INST_TYPE_OUTONLY         4
#define EIPS_TACL_OBJ_INST_TYPE_DATAEXCHANGE    5

/* number of instances */
#define EIPS_TACL_OBJ_MAX_NUM_INST  (EIPS_TACL_OBJ_MAX_NUM_EXOWNER_CNXNS + \
                                     EIPS_TACL_OBJ_MAX_NUM_INONLY_CNXNS + \
                                     EIPS_TACL_OBJ_MAX_NUM_LISTENONLY_CNXNS + \
                                     EIPS_TACL_OBJ_MAX_NUM_DATAEXCHANGE_CNXNS + \
                                     EIPS_TACL_OBJ_MAX_NUM_OUTONLY_CNXNS)
/* Class/Instance structure */
typedef struct
{
  struct
  {
/* ATTR 01 */ uint16  ClassRev;
/* ATTR 02 */ uint16  MaxInst;
  }ClassLevel;

  struct
  {
/* ATTR 01 */

      /* not addressable */
      uint16 InstID;
      uint8  InstType;
  }InstLevel[EIPS_TACL_OBJ_MAX_NUM_INST];

  /* grouping of o2t, t2o and config instance */
  EIPS_TACL_GROUPING_STRUCT group[EIPS_TACL_OBJ_MAX_NUM_INST];
}EIPS_TACL_OBJ_STRUCT;
#endif

/* ====================================================================
   TIMER STRUCTURE (count down)
======================================================================= */
typedef struct
{
    uint32 current_value;
    uint32 reload_value;
    uint16 *timeout_flag_ptr;
    void   *custom_func_arg;

    /* standard timeout function, passes just the timer id */
    void (*timeout_func_ptr) (uint16 timer_id);
    /* allow the user to pass a function specific data set (like the struct that had the timer) */
    void (*tmo_custom_func_ptr) (uint16 timer_id, void *ptr);

#ifdef EIPS_TIMER_NAME_USED
   char   timer_name[32];
#endif
}EIPS_TIMER_DATA_STRUCT;

typedef struct
{
   uint8  state;
   uint8  owned;
   EIPS_TIMER_DATA_STRUCT timer_data;
}EIPS_TIMER_STRUCT;

#ifdef EIPS_PCCC_USED
#define EIPS_PCCC_MAX_STRING_LEN    82
typedef struct
{
    uint16  string_size;
    uint8   string_data[EIPS_PCCC_MAX_STRING_LEN];
}EIPS_PCCC_STRING_TYPE;
#endif

#endif /* __EIPS_STRUCT_H__ */
