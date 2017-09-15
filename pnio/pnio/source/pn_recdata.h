

#ifndef PN_RECDATA_H
#define PN_RECDATA_H


/* public constant definitions
 ---------------------------------------------------------------------------*/
#define IODWriteReqHeader       0x0008U 
#define IODReadReqHeader        0x0009U
#define IM0Data             0x0020U
#define IM1Data             0x0021U
#define IM2Data             0x0022U
#define IM3Data             0x0023U
#define IM4Data             0x0024U
#define PDNCDataCheck           0x0230U
#define ARFSUDataAdjust         0x0609U
#define PDIFSUDataAdjust    0x0608U
#define FSHello             0x0600U
#define BLOCK_TYPE_EXP_IDENT_DATA       0x0012
#define BLOCK_TYPE_API_DATA     0x001A
#define PN_RECDATA_PORTID_LEN   255

/* record data block */
#define PN_REC_BLK_TYPE_DIAGNOSISDATA                               0x0010
#define PN_REC_BLK_TYPE_SUBSTITUTEVALUE                             0x0014
#define PN_REC_BLK_TYPE_RECINPDATAOBJELEM                           0x0015
#define PN_REC_BLK_TYPE_RECOUTDATAOBJELEM                           0x0016
#define PN_REC_BLK_TYPE_ARDATA                                      0x0018
#define PN_REC_BLK_TYPE_LOGBOOKDATA                                 0x0019
#define PN_REC_BLK_TYPE_PDPORTDATACHECK                             0x0200
#define PN_REC_BLK_TYPE_PDPORTDATAADJUST                            0x0202
#define PN_REC_BLK_TYPE_PDINTERFACEADJUST                           0x0250
#define PN_REC_BLK_TYPE_CHK_PEERS                                   0x020A
#define PN_REC_BLK_TYPE_CHK_MAUTYPE                                 0x020C
#define PN_REC_BLK_TYPE_CHK_LINKSTATE                               0x021C
#define PN_REC_BLK_TYPE_ADJ_MAUTYPE                                 0x020E
#define PN_REC_BLK_TYPE_ADJ_LINKSTATE                               0x021B
#define PN_REC_BLK_TYPE_ADJ_PEERTOPEER                              0x0224
#define PN_REC_BLK_TYPE_PDPORTSTATISTIC                             0x0251
#define PN_REC_BLK_TYPE_MUL_BLK_HDR                                 0x0400
#define PN_REC_BLK_TYPE_RES                                         0x8000
#define PN_REC_BLK_TYPE_WRITE_RES                                   0x8008
#define PN_REC_BLK_TYPE_READ_RES                                    0x8009
#define PN_REC_BLK_TYPE_ALARMCRBLOCK_RES                            0x8103
#define PN_REC_BLK_TYPE_ARSERVERBLK_RES                             0x8106

/* block versions */
#define PN_REC_BLK_VER_HIGH_1                                       1
#define PN_REC_BLK_VER_LOW_0                                        0
#define PN_REC_BLK_VER_LOW_1                                        1

/* record data indexes */
#define PN_REC_IDX_PDPORTDATACHECK_ONE_SUBSLOT                      0x802b
#define PN_REC_IDX_RECINPDATAOBJELEM_FOR_ONE_SUBSLOT                0x8028
#define PN_REC_IDX_RECOUTDATAOBJELEM_FOR_ONE_SUBSLOT                0x8029
#define PN_REC_IDX_PDPORTDATAADJUST_ONE_SUBSLOT                     0x802f
#define PN_REC_IDX_PDINTERFACEADJUST_FOR_ONE_SUBSLOT                0x8071
#define PN_REC_IDX_PDPORTSTATISTIC_FOR_ONE_SUBSLOT                  0x8072
#define PN_REC_IDX_PDINTERFACEDATAREAL_FOR_ONE_SUBSLOT              0x8080
#define PN_REC_IDX_REALIDENTIFICATIONDATA_FOR_ONE_AR                0xe001
#define PN_REC_IDX_WRITEMULTIPLE                                    0xe040
#define PN_REC_IDX_ARFSUDATAADJUST_FOR_ONE_AR                       0xe050
#define PN_REC_IDX_ARDATA_FOR_ONE_API                               0xf020
#define PN_REC_IDX_ARDATA                                           0xf820
#define PN_REC_IDX_LOGBOOKDATA                                      0xf830
#define PN_REC_IDX_PDEVDATA                                         0xf831
#define PN_REC_IDX_PDREALDATA                                       0xf841
#define PN_REC_IDX_PDEXPECTEDDATA                                   0xf842
#define PN_REC_IDX_AUTOCONFIGURATION                                0xf850
#define PN_REC_IDX_CMSM_TRIGGER                                     0xfbff

/* PeerToPeerBoundary coding */
#define PN_REC_P2P_LLDP_MASK                                        0x01

/* SubstitutionMode */
#define PN_REC_SUBMODE_ZERO                                         0x0000

/* Length Defines */
#define PN_REC_PEER_PORT_NAME_LEN                                   14
#define PN_REC_PEER_STATION_NAME_LEN                                255

/* macro for filling errorStatus structure */
#define SET_ERROR_STATUS( a, b, c, d) do { \
                errorStatus->errorCode = (a); \
                errorStatus->errorDecode = (b); \
                errorStatus->errorCode1 = (c); \
                errorStatus->errorCode2 = (d); \
} \
while(0)

/* public data types
 ---------------------------------------------------------------------------*/
/**
 * ARFSU payload types 
 */
typedef PN_CC_PACKED_PRE struct {
        Unsigned32 helloMode;
        Unsigned32 helloInterval;
        Unsigned32 helloRetry;
        Unsigned32 helloDelay;
} PN_CC_PACKED FS_HELLO_BLOCK_T;

typedef PN_CC_PACKED_PRE struct {
        Unsigned16 padding;
        Unsigned32 mode;
        Unsigned8 uuid[16];
} PN_CC_PACKED FS_PARAMETER_BLOCK_T;

typedef PN_CC_PACKED_PRE struct {
        Unsigned16 padding;
        Unsigned8 data[FLA_LENGTH];
} PN_CC_PACKED FASTSTARTUP_BLOCK_T;

/**
 * ARFSU Data Adjust block
 */
typedef PN_CC_PACKED_PRE struct {
        BLOCK_HEADER_T subHeader;
        PN_CC_PACKED_PRE union {
                FS_PARAMETER_BLOCK_T fsParamBlock;
                FASTSTARTUP_BLOCK_T fastStartupBlock;
        } PN_CC_PACKED subData;
} PN_CC_PACKED ARFSU_DATA_ADJUST_T;

typedef PN_CC_PACKED_PRE struct {
        Unsigned16 padding1;
        BLOCK_HEADER_T subHeader;
        Unsigned16 padding2;
        PN_CC_PACKED_PRE union {
                FASTSTARTUP_BLOCK_T fastStartupBlock;
                FS_HELLO_BLOCK_T    helloBlock;
        } PN_CC_PACKED subData;
} PN_CC_PACKED PDFSU_DATA_ADJUST_T;

/**
 * PDNC DataCheck block
 */
typedef PN_CC_PACKED_PRE struct {
        Unsigned32 mainReqDropBudget;
        Unsigned32 mainDemDropBudget;
        Unsigned32 errorDropBudget;
} PN_CC_PACKED PDNC_DATACHECK_T;

/**
 * IOD Read Response IM0FilterDataSubModul block
 */
typedef PN_CC_PACKED_PRE struct {
        BLOCK_HEADER_T blockHeader;
        Unsigned16 nrOfApis;
        Unsigned32 api;
        Unsigned16 nrOfModules;

        Unsigned16 slotNumber;
        Unsigned32 moduleIdentNumber;
        Unsigned16 numberOfSubModules;
        Unsigned16 subSlotNumber;
        Unsigned32 subModuleIdentNumber;
} PN_CC_PACKED IM0_FILTERDATA_T;

/**
 * IOD Read Response IMData
 */
typedef PN_CC_PACKED_PRE struct {
        BLOCK_HEADER_T blockHeader;
        Unsigned16 vendorId_be16;
        Unsigned8 orderID[PN_LEN_ORDER_ID];
        Unsigned8 imSerialNumber[16];
        Unsigned16 imHardwareRevision_be16;
        Unsigned8 imRevisionPrefix;
        Unsigned8 imSWRevisionFunctionalEnhancement;
        Unsigned8 imSWRevisionBugFix;
        Unsigned8 imSWRevisionInternalChange;
        Unsigned16 imRevisionCounter_be16;
        Unsigned16 imProfileID_be16;
        Unsigned16 imProfileSpecificType_be16;
        Unsigned8 imVersionMajor;
        Unsigned8 imVersionMinor;
        Unsigned16 imSupported;
} PN_CC_PACKED IM0_DATA_T;

typedef PN_CC_PACKED_PRE struct {
        BLOCK_HEADER_T blockHeader;
        Unsigned8 IM_TagFunction[32];
        Unsigned8 IM_TagLocation[22];
} PN_CC_PACKED IM1_DATA_T;

typedef PN_CC_PACKED_PRE struct {
        BLOCK_HEADER_T blockHeader;
        Unsigned8 IM_Date[16];
} PN_CC_PACKED IM2_DATA_T;

typedef PN_CC_PACKED_PRE struct {
        BLOCK_HEADER_T blockHeader;
        Unsigned8 IM_Descriptor[54];
} PN_CC_PACKED IM3_DATA_T;

typedef PN_CC_PACKED_PRE struct {
        BLOCK_HEADER_T blockHeader;
        Unsigned8 IM_Signature[54];
} PN_CC_PACKED IM4_DATA_T;

/**
 * common IOD header
 */
typedef PN_CC_PACKED_PRE struct {
        BLOCK_HEADER_T blockHeader;
        Unsigned16 seqNumber;
        Unsigned8 ARUUID[16];
        Unsigned32 api;
        Unsigned16 slot;
        Unsigned16 subSlot;
        Unsigned16 padding1;
        Unsigned16 index;
        Unsigned32 recordDataLength;
} PN_CC_PACKED IOD_HEADER_T;

typedef PN_CC_PACKED_PRE struct {
        IOD_HEADER_T header;
        Unsigned8 targetARUUID[16];
} PN_CC_PACKED IOD_READ_REQ_T;

/**
 * IOD Write Request block
 */
typedef PN_CC_PACKED_PRE struct {
        BLOCK_HEADER_T dataHeader;
        PN_CC_PACKED_PRE union {
                IM1_DATA_T imData;
                PDFSU_DATA_ADJUST_T  pdiFSUdataAdjust;
        } PN_CC_PACKED payload;
} PN_CC_PACKED IOD_WRITE_REQ_PAYLOAD_T;

typedef PN_CC_PACKED_PRE struct {
        PN_CC_PACKED_PRE union {
                IOD_WRITE_REQ_PAYLOAD_T body;
                Unsigned8 userSpecData[FLA_LENGTH];
        } PN_CC_PACKED data;
} PN_CC_PACKED IOD_WRITE_REQ_BODY_T;

typedef PN_CC_PACKED_PRE struct {
        IOD_HEADER_T header;
        Unsigned16 additionalVal1;
        Unsigned16 additionalVal2;
        ERROR_STATUS_T errorStatus;
        Unsigned8 padding[16];
} PN_CC_PACKED IOD_WRITE_RES_T;

/**
 * IOD Read Response RealIdentificationData
 */
typedef PN_CC_PACKED_PRE struct {
        BLOCK_HEADER_T blockHeader;
        Unsigned16 nrOfApis;
        Unsigned32 api;
        Unsigned16 numberOfSlots;
        Unsigned8 slotDescrData[FLA_LENGTH];
} PN_CC_PACKED REAL_IDENTIFICATION_T;


/**< RealIdentificationData Slot */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 slotNr_be16;                     /**< SlotNumber */
    Unsigned32 modIdentNr_be32;                 /**< ModuleIdentNumber */
    Unsigned16 nrOfSubSlots_be16;               /**< NumberOfSubslots */
} PN_CC_PACKED PN_REC_REAL_IDENTIFICATION_SLOT_T;


/**< RealIdentificationData Subslot */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 subSlotNr_be16;                  /**< SubslotNumber */
    Unsigned32 subModIdentNr_be32;              /**< SubmoduleIdentNumber */
} PN_CC_PACKED PN_REC_REAL_IDENTIFICATION_SUBSLOT_T;


/**< DiagnosisData */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T blockHeader;                 /**< header */
    Unsigned32 api;                             /**< API */
} PN_CC_PACKED DIAGNOSIS_DATA_T;


/**< DiagnosisData Sub */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 slot;                            /**< slot */
    Unsigned16 subSlot;                         /**< subslot */
    Unsigned16 channelNumber;                   /**< channel number */
    Unsigned16 channelProperties;               /**< channel properties */
    Unsigned16 usi;                             /**< user structure identifier */
} PN_CC_PACKED DIAGNOSIS_DATA_SUB_T;


/********************************************
 * ARData Overview
 *
 * PN_RPC_AR_DATA_T
 * {
 *   PN_RPC_ARDATA_ARINFO_T
 *   CMInitiatorStationName
 *   PN_RPC_ARDATA_IOCR_HDR_T
 *   {
 *     PN_RPC_ARDATA_IOCR_T
 *   }
 *   PN_RPC_ARDATA_ALARM_T
 *   ParameterServerStationName
 *   PN_RPC_ARDATA_API_T
 *   {
 *     APIs
 *   }
 * }
 *******************************************/

/**< ARData */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T blkHdr;                      /**< header */
    Unsigned16 nrOfArs_be16;                    /**< NumberOfARs */
} PN_CC_PACKED PN_RPC_ARDATA_T;


/**< ARData entry: AR information */
typedef PN_CC_PACKED_PRE struct {
    UUID_T arUuid;                              /**< ARUUID */
    Unsigned16 arType_be16;                     /**< ARType */
    Unsigned32 arProps_be32;                    /**< ARProperties */
    UUID_T cmInitObjUuid;                       /**< CMInitiatorObjectUUID */
    Unsigned16 stationNameLen_be16;             /**< StationNameLength */
                                                /*   CMInitiatorStationName */
} PN_CC_PACKED PN_RPC_ARDATA_ARINFO_T;


/**< ARData entry: IOCRs header */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 nrOfIocrs_be16;                  /**< NumberOfIOCRs */
} PN_CC_PACKED PN_RPC_ARDATA_IOCR_HDR_T;


/**< ARData entry: IOCR entry */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 iocrType_be16;                   /**< IOCRType */
    Unsigned32 iocrProps_be32;                  /**< IOCRProperties */
    Unsigned16 frameId_be16;                    /**< FrameID */
    PN_APDU_STATUS_T apduStatus;                /**< APDU_Status */
    Unsigned16 initUdpRtPort_be16;              /**< InitiatorUDPRTPort */
    Unsigned16 respUdpRtPort_be16;              /**< ResponderUDPRTPort */
} PN_CC_PACKED PN_RPC_ARDATA_IOCR_T;


/**< ARData entry: alarm info */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 alarmCrType_be16;                /**< AlarmCRType */
    Unsigned16 locAlarmRef_be16;                /**< LocalAlarmReference */
    Unsigned16 remAlarmRef_be16;                /**< RemoteAlarmReference */
    UUID_T paramSrvObjUuid;                     /**< ParameterServiceObjectUUID */
    Unsigned16 stationNameLen_be16;             /**< StationNameLength */
                                                /*   ParameterServerStationName */
} PN_CC_PACKED PN_RPC_ARDATA_ALARM_T;


/**< ARData entry: API data */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 nrOfApis;                        /**< NumberOfAPIs */
} PN_CC_PACKED PN_RPC_ARDATA_API_T;


/**< LogBookData */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T hdr;                     /**< header */
    Unsigned64 actualLocTs_be64;            /**< ActualLocalTimeStamp */
    Unsigned16 nrOfLogEntries_be16;         /**< NumberOfLogEntries */
} PN_CC_PACKED PN_REC_LOG_DATA_T;


/**< LogBookData Entry */
typedef PN_CC_PACKED_PRE struct {
    Unsigned64 LocalTimeStamp_be64;         /**< LocalTimeStamp */
    UUID_T arUUID;                          /**< ARUUID */
    Unsigned32 pnioStatus;                  /**< PNIOStatus */
    Unsigned32 entryDetail;                 /**< EntryDetail */
} PN_CC_PACKED LOG_DATA_ENTRY_T;


/**
 * API Data Block
 */
typedef PN_CC_PACKED_PRE struct {
        BLOCK_HEADER_T blockHeader;
        Unsigned16 nrOfApis;
        Unsigned32 api;
} PN_CC_PACKED API_DATA_T;


/**< SubstituteDataObjectElement */
typedef PN_CC_PACKED_PRE struct {
                                            /*   Data */
    Unsigned8 subDataVal;                   /**< SubstituteDataValid */
} PN_CC_PACKED PN_REC_SUBSTITUTE_DATAOBJELEM_T;


/**< SubstituteDataItem */
typedef PN_CC_PACKED_PRE struct {
    Unsigned8 iocs;                         /**< IOCS */
                                            /**< SubstituteDataObjectElement */
} PN_CC_PACKED PN_REC_SUBSTITUTE_DATAITEM_T;


/**< SubstituteValue */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T blkHdr;                  /**< header */
    Unsigned16 subMode;                     /**< SubstitutionMode */
                                            /*   SubstituteDataItem */
} PN_CC_PACKED PN_REC_SUBSTITUTE_VALUE_T;


/**< RecordInputDataObjectElement (0x8028) */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T blkHdr;                  /**< header */
    Unsigned8 lengthIocs;                   /**< LengthIOCS */
    Unsigned8 iocs;                         /**< IOCS */
    Unsigned8 lengthIops;                   /**< LengthIOPS */
    Unsigned8 iops;                         /**< IOPS */
    Unsigned16 lengthData;                  /**< LengthData */
                                            /*   Data */
} PN_CC_PACKED PN_REC_RECINPDATAOBJ_T;


/**< RecordOutputDataObjectElement (0x8029) */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T blkHdr;                  /**< header */
    Unsigned16 subActFlag;                  /**< SubstituteActiveFlag */
    Unsigned8 lengthIocs;                   /**< LengthIOCS */
    Unsigned8 lengthIops;                   /**< LengthIOPS */
    Unsigned16 lengthData;                  /**< LengthData */
    Unsigned8 iocs;                         /**< IOCS */
                                            /*   Data */
                                            /*   IOPS */
                                            /*   SubstituteValue */
} PN_CC_PACKED PN_REC_RECOUTDATAOBJ_T;


/**< RecordOutputDataObjectElement IOPS */
typedef PN_CC_PACKED_PRE struct {
    Unsigned8 iops;                         /**< IOPS */
} PN_CC_PACKED PN_REC_RECOUTDATAOBJ_IOPS_T;


/**
 * Port data block
 */
typedef PN_CC_PACKED_PRE struct {
        BLOCK_HEADER_T blockHeader;
        Unsigned16 padding;
        Unsigned16 slot;
        Unsigned16 subSlot;
} PN_CC_PACKED PORT_DATA_T;

/**
 * IOD Read Response block
 */
typedef PN_CC_PACKED_PRE struct {
        IOD_HEADER_T header;
        Unsigned16 additionalVal1;
        Unsigned16 additionalVal2;
        Unsigned8 padding[20];
        PN_CC_PACKED_PRE union {
                unsigned char data;
                IM0_FILTERDATA_T im0FilterData;
                IM0_DATA_T imData;
                REAL_IDENTIFICATION_T realIdentData;
                PDFSU_DATA_ADJUST_T pdfsuData;
                Unsigned8 userSpecData[FLA_LENGTH];
                PN_REC_LOG_DATA_T logData;
                EXP_IDENT_DATA_T expIdentData;
                API_DATA_T apiData;
                PORT_DATA_T portData;
        } PN_CC_PACKED data;
} PN_CC_PACKED IOD_READ_RES_T;

/**< MultipleBlockHeader */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T pBlkHdr;                     /**< BlockHeader */
    Unsigned16 padding;                         /**< padding */
    Unsigned32 api_be32;                        /**< API */
    Unsigned16 slot_be16;                       /**< slot */
    Unsigned16 subSlot_be16;                    /**< subslot */
} PN_CC_PACKED PN_REC_MULTIPLE_BLK_HDR_T;

/**< Generic Read Handler Function */
typedef RET_T (*PN_REC_TABLE_READ_FUNC_T)(
        unsigned char *data,
        Unsigned16 *pDataLen,
        Unsigned32 maxDataLen,
        ERROR_STATUS_T *errorStatus,
        AR_T *pAR,
        Unsigned32 api,
        Unsigned16 slot,
        Unsigned16 subSlot,
        Unsigned16 index
);

/**< Generic Read Table Entry */
typedef struct {
    Unsigned16 index;                           /**< record index */
    PN_BOOL_T targetArUuid;                     /**< TargetARUUID must-be-set flag */
    PN_REC_TABLE_READ_FUNC_T func;              /**< record handler */
} PN_REC_TABLE_READ_T;

/**
 * Store busy request data
 */
typedef struct {
        PN_BOOL_T used;
        RPC_SESSION_INFO_T *sessionId;
        AR_T *pAR;
        Unsigned8 *pResponse;
        ERROR_STATUS_T *pErrStatus;
        Unsigned16 seqNumber;
        Unsigned32 api;
        Unsigned16 slot;
        Unsigned16 subSlot;
        Unsigned16 index;
        Unsigned32 maxDataSize;
} BUSY_RECORD_REQ_T;

/**< CheckPeers Structure */
typedef PN_CC_PACKED_PRE struct {
    Unsigned8 numOfPeers;                       /**< number of peers */
} PN_CC_PACKED PN_REC_CHECKPEERS_T;

/**< CheckMAUType Structure */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 mauType_be16;                    /**< MAU type */
} PN_CC_PACKED PN_REC_CHECKMAUTYPE_T;

/**< CheckLinkState Structure */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 linkState_be16;                  /**< link state */
} PN_CC_PACKED PN_REC_CHECKLINKSTATE_T;

/**< AdjustMAUType Structure */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 padding;                         /**< padding */
    Unsigned16 mauType_be16;                    /**< MAU type */
    Unsigned16 adjProps_be16;                   /**< adjust properties */
} PN_CC_PACKED PN_REC_ADJMAUTYPE_T;

/**< AdjustLinkState Structure */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 padding;                         /**< padding */
    Unsigned16 linkState_be16;                  /**< link state */
    Unsigned16 adjProps_be16;                   /**< adjust properties */
} PN_CC_PACKED PN_REC_ADJLINKSTATE_T;

/**< AdjustPeerToPeerBoundary Structure */
typedef PN_CC_PACKED_PRE struct {
    Unsigned16 padding;                         /**< padding */
    Unsigned32 peerToPeer_be32;                 /**< PeerToPeerBoundary */
    Unsigned16 adjProps_be16;                   /**< adjust properties */
} PN_CC_PACKED PN_REC_ADJPEERTOPEER_T;

/**< PDPortData* Structure */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T header;                      /**< header */
    Unsigned16 padding;                         /**< padding */
    Unsigned16 slot_be16;                       /**< slot */
    Unsigned16 subSlot_be16;                    /**< subslot */
} PN_CC_PACKED PN_REC_PDPORTDATA_T;


/**< PDPortDataCheck/Adjust Settings Interface Helper Structure */
typedef PN_CC_PACKED_PRE struct {
    PN_BOOL_T flag;                             /**< usage flag */
    Unsigned16 val;                             /**< value */
} PN_CC_PACKED PN_REC_PDPORTDATA_CFG_SUB_T;


/**< PDPortDataCheck/Adjust Settings Interface Structure */
typedef PN_CC_PACKED_PRE struct {
    PN_REC_PDPORTDATA_CFG_SUB_T checkMauType;   /**< CheckMAUType */
    PN_REC_PDPORTDATA_CFG_SUB_T checkLinkState; /**< CheckLinkState */

    PN_CC_PACKED_PRE struct {
        PN_BOOL_T flag;                         /**< usage flag */
        Unsigned8 numOfPeers;                   /**< NumberOfPeers */
        Unsigned8 peerPortNameLen;              /**< PeerPortName length */
        Unsigned8 peerPortName[PN_REC_PEER_PORT_NAME_LEN]; /**< PeerPortName */
        Unsigned8 peerStationNameLen;           /**< PeerStationName length */
        Unsigned8 peerStationName[PN_REC_PEER_STATION_NAME_LEN]; /**< PeerStationName */
    } PN_CC_PACKED checkPeers;

    PN_REC_PDPORTDATA_CFG_SUB_T adjustMauType;  /**< AdjustMAUType */
    PN_REC_PDPORTDATA_CFG_SUB_T adjustLinkState;/**< AdjustLinkState */
    PN_REC_PDPORTDATA_CFG_SUB_T adjustPeerToPeer; /**< AdjustLinkState */
} PN_CC_PACKED PN_REC_PDPORTDATA_CFG_PORT_T;

/**< PDPortDataAdjust Settings Structure */
typedef PN_CC_PACKED_PRE struct {
    PN_REC_PDPORTDATA_CFG_PORT_T ports[CONFIG_TARGET_ETH_PORT_COUNT];
} PN_CC_PACKED PN_REC_PDPORTDATA_CFG_T;

/**< PDInterfaceAdjust Structure */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T header;                      /**< header */
    Unsigned16 padding;                         /**< padding */
    Unsigned32 mulIfaceMode;                    /**< MultipleInterfaceMode */
                                                /*   padding */
} PN_CC_PACKED PN_REC_PDINTERFACEADJUST_T;

/**< PDPortStatistic Structure (0x8072) */
typedef PN_CC_PACKED_PRE struct {
    BLOCK_HEADER_T blkHdr;                  /**< header */
    Unsigned16 padding;                     /**< padding */
    Unsigned32 ifInOctets_be32;             /**< ifInOctets */
    Unsigned32 ifOutOctets_be32;            /**< ifOutOctets */
    Unsigned32 ifInDiscards_be32;           /**< ifInDiscards */
    Unsigned32 ifOutDiscards_be32;          /**< ifOutDiscards */
    Unsigned32 ifInErrors_be32;             /**< ifInErrors */
    Unsigned32 ifOutErrors_be32;            /**< ifOutErrors */
                                            /*   padding to Unsigned32 align */
} PN_CC_PACKED PN_REC_PDPORTSTATISTIC;


/* global variables
 ---------------------------------------------------------------------------*/
extern PN_REC_PDPORTDATA_CFG_T PN_recPDPortDataCfg; /**< PDPortDataAdjust settings */


/* list of public functions
 ---------------------------------------------------------------------------*/

IOD_STATUS_T RD_processRecWrite(
    const IOD_HEADER_T *pWriteReqHdr,
    const IOD_WRITE_REQ_BODY_T *pWriteReqBody,
    IOD_WRITE_RES_T *pWriteRes,
    ERROR_STATUS_T *errorStatus,
    Unsigned16 *pOutDataLen,
    RPC_SESSION_INFO_T *rpcSessionId
);

IOD_STATUS_T RD_processRecImplicitRead(
    const IOD_HEADER_T *pReadReqHdr,
    IOD_READ_RES_T *pReadRes,
    ERROR_STATUS_T *errorStatus,
    Unsigned16 *pOutDataLen,
    RPC_SESSION_INFO_T *rpcSessionId
);

IOD_STATUS_T RD_processRecExplicitRead(
    const IOD_HEADER_T *pReadReqHdr,
    IOD_READ_RES_T *pReadRes,
    ERROR_STATUS_T *errorStatus,
    Unsigned16 *pOutDataLen,
    RPC_SESSION_INFO_T *rpcSessionId
);

void RD_init(void);
IOD_STATUS_T PN_recPDPortDataReset(void);
void PN_recPDPortDataParamStart(void);
void PN_recPDPortDataParamEnd(void);
void PN_recHeaderPrepare(BLOCK_HEADER_T *pHdr, Unsigned16 type, Unsigned8 verHigh, Unsigned8 verLow);
void PN_recHeaderLength(BLOCK_HEADER_T *pHdr, Unsigned16 size);
unsigned int PN_recMultipleBlockHeaderPrepare(PN_REC_MULTIPLE_BLK_HDR_T *pHdr, Unsigned32 api, Unsigned16 slot, Unsigned16 subSlot);
void PN_recMultipleBlockHeaderLength(PN_REC_MULTIPLE_BLK_HDR_T *pHdr, Unsigned16 size);

#ifdef CONFIG_RECORD_DATA_BUSY
IOD_STATUS_T PN_recBusyGet(
    int index,                                  /**< busy record index */
    BUSY_RECORD_REQ_T **rec                     /**< busy record ptr */
);
#endif /* CONFIG_RECORD_DATA_BUSY */


#endif /* PN_RECDATA_H */
