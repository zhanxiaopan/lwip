

#ifndef PN_DEF_H
#define PN_DEF_H

#define FLA_LENGTH 1 /**< global length of flexible array */
#define MAX_SUBMOD_DATA_LENGTH 1439
#define MAC_MC_FIRST_BYTE 0x01
#define RESPONDER_UDP_PORT 0x8892
#define LEN_UUID 16

#define AR_UUID_SIZE LEN_UUID  /* size of AR identifier */
#define AR_INIT_NAME_SIZE 100  /* max name size of AR intiator (Controller) */

#define LMPM_FRAME_ID_LEN                2


/* MAU Type - 6.2.12.7 Coding of the field MAUType */
#define PNIO_MAUTYPE_RESERVED           0x0000
#define PNIO_MAUTYPE_10TXHD             0x000A  /* 10Mb Half Duplex*/
#define PNIO_MAUTYPE_10TXFD             0x000B  /* 10Mb Full Duplex*/
#define PNIO_MAUTYPE_100TXHD            0x000F  /* 100Mb Half Duplex*/
#define PNIO_MAUTYPE_100TXFD            0x0010  /* 100Mb Full Duplex*/
#define PNIO_MAUTYPE_1000THD            0x001D  /* 1000Mb Half Duplex */
#define PNIO_MAUTYPE_1000TFD            0x001E  /* 1000Mb Full Duplex */


/* Link State - 6.2.12.16 Coding of the field LinkState */
#define PNIO_LINKSTATE_RESERVED         0x00
#define PNIO_LINKSTATE_UP               0x01 /* ready to pass packets */
#define PNIO_LINKSTATE_DOWN             0x02
#define PNIO_LINKSTATE_TESTING          0x03 /* in some test mode */
#define PNIO_LINKSTATE_UNKNOWN          0x04 /* status can not determined */
#define PNIO_LINKSTATE_DORMANT          0x05
#define PNIO_LINKSTATE_NOTPRESENT       0x06
#define PNIO_LINKSTATE_LOWERLAYERDOWN   0x07

/* IO Status */
#define PNIO_IOXS_GOOD                  0x80
#define PNIO_IOPS_SUBST_ACTIVE          0x0001
#define PNIO_IOXS_STATE_BY_SUBSLOT      0x00
#define PNIO_IOXS_STATE_BY_SLOT         0x20
#define PNIO_IOXS_STATE_BY_DEVICE       0x40
#define PNIO_IOXS_STATE_BY_CONTROLLER   0x60
#define PNIO_IOXS_STATE_BY_MASK         0x60

/* Broadcast/Multicast MAC addresses */
#define MAC_ADDR_DCP                    { 0x01, 0x0E, 0xCF, 0x00, 0x00, 0x00 }
#define MAC_ADDR_DCP_HELLO              { 0x01, 0x0E, 0xCF, 0x00, 0x00, 0x01 }
#define MAC_ADDR_LLDP                   { 0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E }

/* Ethernet Packet Length Defines */
#define ETH_HDR_LEN                     14
#define VLAN_TAG_LEN                    4
#define FRAME_TYPE_LEN                  2
#define FRAME_ID_LEN                    2
#define ETH_MTU_LEN                     1536

#endif
