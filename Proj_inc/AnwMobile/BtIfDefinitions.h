/////////////////////////////////////////////////////////////////////////////
//
//  Name        BtIfDefinitions.h
//  $Header:
//
//  Function    this file contains Widcomm SDK type definitions
//
//  Date                 Modification
//  ----------------------------------
//  12/17/2000    JF   Create
//
//  Copyright (c) 2000-2002, WIDCOMM Inc., All Rights Reserved.
//  Proprietary and confidential.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _BTIFDEFINITIONS_H 
#define _BTIFDEFINITIONS_H


// Ensure alignment across all builds
//
#ifdef _WIN32_WCE
#pragma pack (8)
#else
#pragma pack (1)
#endif

//
// Define return codes from the SDP service functions
//
typedef enum
{
    SDP_OK,
    SDP_COULD_NOT_ADD_RECORD,
    SDP_INVALID_RECORD,
    SDP_INVALID_PARAMETERS

} SDP_RETURN_CODE;

//
// Define SPP connection states
//
typedef enum
{
    SPP_CONNECTED,	        // port now connected
    SPP_DISCONNECTED	    // port now disconnected
} SPP_STATE_CODE;


// Maximum number of UUIDs in a service class ID list
//
#define MAX_UUIDS_PER_SEQUENCE   3

// Maximum service name length
//
#define BT_MAX_SERVICE_NAME_LEN  100

// Maximum protocol list elements in a record
//
#define MAX_PROTOCOL_LIST_ELEM   3

#define SDP_MAX_LIST_ELEMS      3
#define MAX_ELEM_IN_SEQ 10


//
// Define a structure to hold attribute values when
// read from the discovery database. Note that the
// attribute may be a sequence, in which case the
// number of elements will be greater than 1.
//
#define MAX_SEQ_ENTRIES     20
#define MAX_ATTR_LEN        256

typedef struct
{
    int     num_elem;

    struct
    {
        #define ATTR_TYPE_INT      0       // Attribute value is an integer
        #define ATTR_TYPE_TWO_COMP 1       // Attribute value is an 2's complement integer
        #define ATTR_TYPE_UUID     2       // Attribute value is a UUID
        #define ATTR_TYPE_BOOL     3       // Attribute value is a boolean
        #define ATTR_TYPE_ARRAY    4       // Attribute value is an array of bytes

        int     type;
        int     len;                        // Length of the attribute
        BOOL    start_of_seq;               // TRUE for each start of sequence
        union
        {
            unsigned char  u8;                      // 8-bit integer
            unsigned short u16;                     // 16-bit integer
            unsigned long  u32;                     // 32-bit integer
            BOOL           b;                       // Boolean
            unsigned char  array[MAX_ATTR_LEN];     // Variable length array
        } val;

    } elem [MAX_SEQ_ENTRIES];

} SDP_DISC_ATTTR_VAL;

///////////////////////////////////////////////////////////////////////////////
///// definitions used in FTP client apps to present remote server directory listings
//
#define MAX_NAME_SIZE   255
#define MAX_ENTRY_SIZE  1000
#define DATE_TIME_SIZE  15

typedef struct
{
    WCHAR    name[MAX_NAME_SIZE + 1];
    BOOL    is_folder;
    WCHAR    date_created[DATE_TIME_SIZE + 1];
    WCHAR    date_modified[DATE_TIME_SIZE + 1];
    WCHAR    date_accessed[DATE_TIME_SIZE + 1];
    ULONG    file_size;

#define FTP_READ_PERM      (0x01)
#define FTP_WRITE_PERM     (0x02)
#define FTP_DELETE_PERM    (0x04)

    WCHAR    user_perm;
    WCHAR    group_perm;
    WCHAR    other_perm;
} tFTP_FILE_ENTRY;



#ifndef BT_CONN_STATS_DEFINED
#define BT_CONN_STATS_DEFINED
typedef struct
{
    UINT32   bIsConnected;
    INT32    Rssi;
    UINT32   BytesSent;
    UINT32   BytesRcvd;
    UINT32   Duration;
} tBT_CONN_STATS;
#endif  //BT_CONN_STATS_DEFINED

/////////////////////////////////////////////////////////////////////////////////////////
//	Definitions for applications only
//

#if !defined (WIDCOMMSDK_EXPORTS)

//
// Define some types used by the Widcomm SDK
//
typedef unsigned char   UINT8;
typedef unsigned short  UINT16;
typedef unsigned int    UINT32;

//
// Common Bluetooth field definitions
//
#define BD_ADDR_LEN     6             		/* Device address length */
typedef UINT8 BD_ADDR[BD_ADDR_LEN];			/* Device address */
typedef UINT8 *BD_ADDR_PTR;					/* Pointer to Device Address */

#define LINK_KEY_LEN    16
typedef UINT8 LINK_KEY[LINK_KEY_LEN];       /* Link Key */

#define PIN_CODE_LEN    16
typedef UINT8 PIN_CODE[PIN_CODE_LEN];       /* Pin Code (upto 128 bits) MSB is 0 */
typedef UINT8 *PIN_CODE_PTR;				/* Pointer to Pin Code */

#define DEV_CLASS_LEN   3
typedef UINT8 DEV_CLASS[DEV_CLASS_LEN];     /* Device class */
typedef UINT8 *DEV_CLASS_PTR;				/* Pointer to Device class */

//
// Coding of the DEV_CLASS field. Refer to Bluetooth Assigned Numbers
// specification for further information on device class.
//
// First byte - Service class of the CoD, byte [0]
#define SERV_CLASS_NETWORKING               (1 << 1)
#define SERV_CLASS_RENDERING                (1 << 2)
#define SERV_CLASS_CAPTURING                (1 << 3)
#define SERV_CLASS_OBJECT_TRANSFER          (1 << 4)
#define SERV_CLASS_OBJECT_AUDIO             (1 << 5)
#define SERV_CLASS_OBJECT_TELEPHONY         (1 << 6)
#define SERV_CLASS_OBJECT_INFORMATION       (1 << 7)

// Second byte - high order 
#define SERV_CLASS_LIMITED_DISC_MODE        (0x20)

// Also 2nd byte - Major Device class - low 5 bits
#define MAJOR_DEV_CLASS_MASK                0x1F

#define MAJOR_DEV_CLASS_MISC                0x00
#define MAJOR_DEV_CLASS_COMPUTER            0x01
#define MAJOR_DEV_CLASS_PHONE               0x02
#define MAJOR_DEV_CLASS_LAN_ACCESS          0x03
#define MAJOR_DEV_CLASS_AUDIO               0x04
#define MAJOR_DEV_CLASS_PERIPHERAL          0x05
#define MAJOR_DEV_CLASS_IMAGING             0x06
#define MAJOR_DEV_CLASS_UNSPECIFIED         0x1F

// 3rd byte - Minor Device class, hi  6 bits
#define MINOR_DEV_CLASS_MASK                0xFC

// Minor device class if major is computer
#define MINOR_DEV_CLASS_COMP_UNCLASSIFIED   0x00
#define MINOR_DEV_CLASS_COMP_WORKSTATION    (0x01 << 2)
#define MINOR_DEV_CLASS_COMP_SERVER         (0x02 << 2)
#define MINOR_DEV_CLASS_COMP_LAPTOP         (0x03 << 2)
#define MINOR_DEV_CLASS_COMP_HANDHELD       (0x04 << 2)
#define MINOR_DEV_CLASS_COMP_PALM           (0x05 << 2)

// Minor device class if major is phone
#define MINOR_DEV_CLASS_PHONE_UNCLASSIFIED  0x00
#define MINOR_DEV_CLASS_PHONE_CELLULAR      (0x01 << 2)
#define MINOR_DEV_CLASS_PHONE_CORDLESS      (0x02 << 2)
#define MINOR_DEV_CLASS_PHONE_SMART         (0x03 << 2)
#define MINOR_DEV_CLASS_PHONE_MODEM         (0x04 << 2)

// Minor device class if major is imaging
#define MINOR_DEV_CLASS_IMAGING_UNCLASSIFIED  0x00
#define MINOR_DEV_CLASS_IMAGING_DISPLAY       (0x04 << 2)
#define MINOR_DEV_CLASS_IMAGING_CAMERA        (0x08 << 2)
#define MINOR_DEV_CLASS_IMAGING_SCANNER       (0x10 << 2)
#define MINOR_DEV_CLASS_IMAGING_PRINTER       (0x20 << 2)

#define MINOR_DEV_CLASS_AUDIO_UNCLASSIFIED  0x00

// Minor device class if major is audio
#define MINOR_DEV_CLASS_AUDIO_UNCLASSIFIED  0x00
#define MINOR_DEV_CLASS_AUDIO_HEADSET       (0x01 << 2)


#define BD_NAME_LEN     248
typedef UINT8 BD_NAME[BD_NAME_LEN];         /* Device name */
typedef UINT8 *BD_NAME_PTR;					/* Pointer to Device name */

#define BD_MODEL_NAME	64
typedef UINT8 BD_MODEL[BD_MODEL_NAME];
typedef UINT8 *BD_MODEL_PTR;

// L2CAP Configuration QOS structure
//
#ifndef BT_TYPES_H
typedef struct
{
	UINT8				qos_flags;			/* TBD */

// Values for service_type
//
#define NO_TRAFFIC		0
#define BEST_EFFORT		1
#define GUARANTEED		2

    UINT8				service_type;		/* see below */
	UINT32				token_rate;			/* bytes/second */
	UINT32				token_bucket_size;	/* bytes */
	UINT32				peak_bandwidth;		/* bytes/second */
	UINT32				latency;			/* microseconds */
	UINT32				delay_variation;	/* microseconds */			

} FLOW_SPEC;
#endif


#define L2CAP_MIN_MTU                   48

//Zong, need to talk to ask to see if we can make them the same
#ifdef _WIN32_WCE
    #define L2CAP_MAX_MTU               1400
    #define L2CAP_DEFAULT_MTU           672
#else
    #define L2CAP_MAX_MTU               1696
    #define L2CAP_DEFAULT_MTU           1691
#endif

#define L2CAP_DEFAULT_FLUSH_TO          0xFFFF


// This structure is used to add protocol lists and find protocol elements
//
#ifndef SDP_MAX_PROTOCOL_PARAMS
#define SDP_MAX_PROTOCOL_PARAMS     1
#endif

typedef struct 
{
    UINT16      protocol_uuid;
    UINT16      num_params;
    UINT16      params[SDP_MAX_PROTOCOL_PARAMS];
} tSDP_PROTOCOL_ELEM;

typedef struct 
{
    UINT16              num_elems;
    tSDP_PROTOCOL_ELEM  list_elem[SDP_MAX_LIST_ELEMS];
} tSDP_PROTO_LIST_ELEM;

// Maximum UUID size - 16 bytes, and structure to hold any type of UUID.
//
#define MAX_UUID_SIZE              16

#ifndef BT_TYPES_H
typedef struct
{
#define LEN_UUID_16     2
#define LEN_UUID_32     4
#define LEN_UUID_128    16

    UINT16          len;
    UINT8           pad[14];

    union
    {
        UINT16      uuid16;
        UINT32      uuid32;
        UINT8       uuid128[MAX_UUID_SIZE];
    } uu;

} tBT_UUID;
#endif


// Define the service record attribute IDs.
//
#define  ATTR_ID_SERVICE_RECORD_HDL             0x0000
#define  ATTR_ID_SERVICE_CLASS_ID_LIST          0x0001
#define  ATTR_ID_SERVICE_RECORD_STATE           0x0002
#define  ATTR_ID_SERVICE_ID                     0x0003
#define  ATTR_ID_PROTOCOL_DESC_LIST             0x0004
#define  ATTR_ID_BROWSE_GROUP_LIST              0x0005
#define  ATTR_ID_LANGUAGE_BASE_ATTR_ID_LIST     0x0006
#define  ATTR_ID_SERVICE_INFO_TIME_TO_LIVE      0x0007
#define  ATTR_ID_SERVICE_AVAILABILITY           0x0008
#define  ATTR_ID_BT_PROFILE_DESC_LIST           0x0009
#define  ATTR_ID_DOCUMENTATION_URL              0x000A
#define  ATTR_ID_CLIENT_EXE_URL                 0x000B
#define  ATTR_ID_ICON_URL                       0x000C
#define  ATTR_ID_ADDITION_PROTO_DESC_LISTS      0x000D

#define  LANGUAGE_BASE_ID                       0x0100
#define  ATTR_ID_SERVICE_NAME                   LANGUAGE_BASE_ID + 0x0000
#define  ATTR_ID_SERVICE_DESCRIPTION            LANGUAGE_BASE_ID + 0x0001
#define  ATTR_ID_PROVIDER_NAME                  LANGUAGE_BASE_ID + 0x0002

#define  ATTR_ID_VERSION_OR_GROUP				0x0200
#define  ATTR_ID_VERSION_NUMBER_LIST            ATTR_ID_VERSION_OR_GROUP
#define  ATTR_ID_GROUP_ID                       ATTR_ID_VERSION_OR_GROUP
#define  ATTR_ID_SERVICE_DATABASE_STATE         0x0201
#define  ATTR_ID_DATA_STORES_OR_NETWORK         0x0301
#define  ATTR_ID_SUPPORTED_DATA_STORES          ATTR_ID_DATA_STORES_OR_NETWORK
#define  ATTR_ID_EXTERNAL_NETWORK               ATTR_ID_DATA_STORES_OR_NETWORK
#define  ATTR_ID_FAX_CLASS_1_OR_AUDIO_VOLUME	0x0302
#define  ATTR_ID_FAX_CLASS_1_SUPPORT            ATTR_ID_FAX_1_OR_AUD_VOL_OR_DEV_NAME
#define  ATTR_ID_REMOTE_AUDIO_VOLUME_CONTROL    ATTR_ID_FAX_1_OR_AUD_VOL_OR_DEV_NAME
#define  ATTR_ID_DEVICE_NAME                    ATTR_ID_FAX_1_OR_AUD_VOL_OR_DEV_NAME
#define  ATTR_ID_FORMATS_OR_FAX_2_0				0x0303
#define  ATTR_ID_SUPPORTED_FORMATS_LIST			ATTR_ID_FORMATS_OR_FAX_2_0
#define  ATTR_ID_FAX_CLASS_2_0_SUPPORT			ATTR_ID_FORMATS_OR_FAX_2_0
#define  ATTR_ID_FAX_CLASS_2_OR_FRIENDLY_NAME   0x0304
#define  ATTR_ID_FAX_CLASS_2_SUPPORT            ATTR_ID_FAX_CLASS_2_OR_FRIENDLY_NAME
#define  ATTR_ID_FRIENDLY_NAME                  ATTR_ID_FAX_CLASS_2_OR_FRIENDLY_NAME
#define  ATTR_ID_AUDIO_FEEDBACK_SUPPORT			0x0305

// Define for service attribute, all the 'Descriptor Type' values.
// These are also referred to as 'attribute type' values
#define  NULL_DESC_TYPE                     0
#define  UINT_DESC_TYPE                     1
#define  TWO_COMP_INT_DESC_TYPE             2
#define  UUID_DESC_TYPE                     3
#define  TEXT_STR_DESC_TYPE                 4
#define  BOOLEAN_DESC_TYPE                  5
#define  DATA_ELE_SEQ_DESC_TYPE             6
#define  DATA_ELE_ALT_DESC_TYPE             7
#define  URL_DESC_TYPE                      8


// Define common 16-bit protocol UUIDs
//
#define UUID_PROTOCOL_SDP                    0x0001
#define UUID_PROTOCOL_UDP                    0x0002
#define UUID_PROTOCOL_RFCOMM                 0x0003
#define UUID_PROTOCOL_TCP                    0x0004
#define UUID_PROTOCOL_TCS_BIN                0x0005
#define UUID_PROTOCOL_TCS_AT                 0x0006
#define UUID_PROTOCOL_OBEX                   0x0008
#define UUID_PROTOCOL_IP                     0x0009
#define UUID_PROTOCOL_FTP                    0x000A
#define UUID_PROTOCOL_HTTP                   0x000C
#define UUID_PROTOCOL_WSP                    0x000E
#define UUID_PROTOCOL_BNEP                   0x000F
#define UUID_PROTOCOL_UPNP                   0x0010
#define UUID_PROTOCOL_HIDP                   0x0011
#define UUID_PROTOCOL_HCRP_CTRL              0x0012
#define UUID_PROTOCOL_HCRP_DATA              0x0014
#define UUID_PROTOCOL_HCRP_NOTIF             0x0016
#define UUID_PROTOCOL_AVCTP                  0x0017
#define UUID_PROTOCOL_AVDTP                  0x0019
#define UUID_PROTOCOL_L2CAP                  0x0100

// Define common 16-bit service class UUIDs
//
#define UUID_SERVCLASS_SERVICE_DISCOVERY_SERVER 0X1000
#define UUID_SERVCLASS_BROWSE_GROUP_DESCRIPTOR  0X1001
#define UUID_SERVCLASS_PUBLIC_BROWSE_GROUP      0X1002
#define UUID_SERVCLASS_SERIAL_PORT              0X1101
#define UUID_SERVCLASS_LAN_ACCESS_USING_PPP     0X1102
#define UUID_SERVCLASS_DIALUP_NETWORKING        0X1103
#define UUID_SERVCLASS_IRMC_SYNC                0X1104
#define UUID_SERVCLASS_OBEX_OBJECT_PUSH         0X1105
#define UUID_SERVCLASS_OBEX_FILE_TRANSFER       0X1106
#define UUID_SERVCLASS_IRMC_SYNC_COMMAND        0X1107
#define UUID_SERVCLASS_HEADSET                  0X1108
#define UUID_SERVCLASS_CORDLESS_TELEPHONY       0X1109
#define UUID_SERVCLASS_INTERCOM                 0X1110
#define UUID_SERVCLASS_FAX                      0X1111
#define UUID_SERVCLASS_HEADSET_AUDIO_GATEWAY    0X1112
#define UUID_SERVCLASS_PNP_INFORMATION          0X1200
#define UUID_SERVCLASS_GENERIC_NETWORKING       0X1201
#define UUID_SERVCLASS_GENERIC_FILETRANSFER     0X1202
#define UUID_SERVCLASS_GENERIC_AUDIO            0X1203
#define UUID_SERVCLASS_GENERIC_TELEPHONY        0X1204


////////////////////////////////////////////////////////////////////////////////
// Definitions for RFCOMM and PORT functions
//

// 
// Define port settings structure send from the application in the 
// set settings request, or to the application in the set settings indication.
//
typedef struct 
{

#define PORT_BAUD_RATE_2400       0x00
#define PORT_BAUD_RATE_4800       0x01
#define PORT_BAUD_RATE_7200       0x02
#define PORT_BAUD_RATE_9600       0x03
#define PORT_BAUD_RATE_19200      0x04
#define PORT_BAUD_RATE_38400      0x05
#define PORT_BAUD_RATE_57600      0x06
#define PORT_BAUD_RATE_115200     0x07
#define PORT_BAUD_RATE_230400     0x08

    UINT8  baud_rate;

#define PORT_5_BITS               0x00
#define PORT_6_BITS               0x01
#define PORT_7_BITS               0x02
#define PORT_8_BITS               0x03

    UINT8  byte_size;

#define PORT_ONESTOPBIT           0x00
#define PORT_ONE5STOPBITS         0x01
    UINT8   stop_bits;

#define PORT_PARITY_NO            0x00
#define PORT_PARITY_YES           0x01
    UINT8   parity;

#define PORT_ODD_PARITY           0x00
#define PORT_EVEN_PARITY          0x01
#define PORT_MARK_PARITY          0x02
#define PORT_SPACE_PARITY         0x03

    UINT8   parity_type;

#define PORT_FC_OFF               0x00
#define PORT_FC_XONXOFF_ON_INPUT  0x01
#define PORT_FC_XONXOFF_ON_OUTPUT 0x02
#define PORT_FC_CTS_ON_INPUT      0x04
#define PORT_FC_CTS_ON_OUTPUT     0x08
#define PORT_FC_DSR_ON_INPUT      0x10
#define PORT_FC_DSR_ON_OUTPUT     0x20

    UINT8 fc_type;

    UINT8 rx_char1;

#define PORT_XON_DC1              0x11
    UINT8 xon_char;

#define PORT_XOFF_DC3             0x13
    UINT8 xoff_char;

} tPORT_STATE;

typedef struct
{

#define PORT_FLAG_CTS_HOLD  0x01    /* Tx is waiting for CTS signal */
#define PORT_FLAG_DSR_HOLD  0x02    /* Tx is waiting for DSR signal */
#define PORT_FLAG_RLSD_HOLD 0x04    /* Tx is waiting for RLSD signal */
    UINT16  flags;
    UINT16  in_queue_size;          /* Number of bytes in the input queue */
    UINT16  out_queue_size;         /* Number of bytes in the output queue */
} tPORT_STATUS;


//
// Define RFCOMM Port events that registered application can receive
// in the callback
//
#define PORT_EV_RXCHAR  0x00000001   /* Any Character received */
#define PORT_EV_RXFLAG  0x00000002   /* Received certain character */
#define PORT_EV_TXEMPTY 0x00000004   /* Transmitt Queue Empty */
#define PORT_EV_CTS     0x00000008   /* CTS changed state */
#define PORT_EV_DSR     0x00000010   /* DSR changed state */
#define PORT_EV_RLSD    0x00000020   /* RLSD changed state */
#define PORT_EV_BREAK   0x00000040   /* BREAK received */
#define PORT_EV_ERR     0x00000080   /* Line status error occurred */
#define PORT_EV_RING    0x00000100   /* Ring signal detected */
#define PORT_EV_CTSS    0x00000400   /* CTS state */
#define PORT_EV_DSRS    0x00000800   /* DSR state */
#define PORT_EV_RLSDS   0x00001000   /* RLSD state */
#define PORT_EV_OVERRUN 0x00002000   /* receiver buffer overrun */
#define PORT_EV_TXCHAR  0x00004000   /* Any character transmitted */

#define PORT_EV_CONNECTED    0x00000200  /* RFCOMM connection established */
#define PORT_EV_CONNECT_ERR  0x00008000  /* Was not able to establish connection */
                                     /* or disconnected */
#define PORT_EV_FC      0x00010000   /* flow control enabled flag changed by remote */
#define PORT_EV_FCS     0x00020000   /* flow control status true = enabled */

//
// To register for RFCOMM events application should provide bitmask with 
// corresponding bit set
//
#define PORT_MASK_ALL             (PORT_EV_RXCHAR | PORT_EV_TXEMPTY | PORT_EV_CTS | \
                                   PORT_EV_DSR | PORT_EV_RLSD | PORT_EV_BREAK | \
                                   PORT_EV_ERR | PORT_EV_RING | PORT_EV_CONNECT_ERR | \
                                   PORT_EV_DSRS | PORT_EV_CTSS | PORT_EV_RLSDS | \
                                   PORT_EV_RXFLAG | PORT_EV_TXCHAR | PORT_EV_OVERRUN | \
                                   PORT_EV_CONNECTED | PORT_EV_FC | PORT_EV_FCS)



//
// Definitions used by RFCOMM to set control leads
//
#define PORT_SET_DTRDSR         0x01
#define PORT_CLR_DTRDSR         0x02
#define PORT_SET_CTSRTS         0x03
#define PORT_CLR_CTSRTS         0x04
#define PORT_SET_RI             0x05        /* DCE only */
#define PORT_CLR_RI             0x06        /* DCE only */
#define PORT_SET_DCD            0x07        /* DCE only */
#define PORT_CLR_DCD            0x08        /* DCE only */
#define PORT_SET_BREAK          0x09
#define PORT_CLR_BREAK          0x0A

//
// RFCOMM Port modem control leads
//
#define PORT_DTRDSR_ON          0x01
#define PORT_CTSRTS_ON          0x02
#define PORT_RING_ON            0x04
#define PORT_DCD_ON             0x08


//
// RFCOMM Port errors
//
#define PORT_ERR_BREAK      0x01    /* Break condition occured on the peer device */
#define PORT_ERR_OVERRUN    0x02    /* Overrun is reported by peer device */
#define PORT_ERR_FRAME      0x04    /* Framing error reported by peer device */
#define PORT_ERR_RXOVER     0x08    /* Input queue overflow occured */
#define PORT_ERR_TXFULL     0x10    /* Output queue overflow occured */

//
// Flags used in the RFCOMM Port purge function
//
#define PORT_PURGE_TXCLEAR  0x01
#define PORT_PURGE_RXCLEAR  0x02

//
// RFCOMM default MTU size
//
#define RFCOMM_DEFAULT_MTU              127


//
// Valid Security Service Levels 
//
#define BTM_SEC_NONE               0x00 /* Nothing required */
#define BTM_SEC_IN_AUTHORIZE       0x01 /* Inbound call requires authorization */
#define BTM_SEC_IN_AUTHENTICATE    0x02 /* Inbound call requires authentication */
#define BTM_SEC_IN_ENCRYPT         0x04 /* Inbound call requires encryption */
#define BTM_SEC_OUT_AUTHORIZE      0x08 /* Outbound call requires authorization */
#define BTM_SEC_OUT_AUTHENTICATE   0x10 /* Outbound call requires authentication */
#define BTM_SEC_OUT_ENCRYPT        0x20 /* Outbound call requires encryption */
#define BTM_SEC_BOND               0x40 /* Bonding */

//
// definitions for security
//

// 
// Predefined security services 
//

#define BTM_SEC_SERVICE_SDP_SERVER  0
#define BTM_SEC_SERVICE_SERIAL_PORT 1
#define BTM_SEC_SERVICE_LAN_ACCESS  2
#define BTM_SEC_SERVICE_DUN         3
#define BTM_SEC_SERVICE_IRMCSYBC    4
#define BTM_SEC_SERVICE_OBEX_PUSH   5
#define BTM_SEC_SERVICE_OBEX_FTP    6
#define BTM_SEC_SERVICE_IRMCSYNCCMD 7
#define BTM_SEC_SERVICE_HEADSET     8
#define BTM_SEC_SERVICE_CORDLESS    9
#define BTM_SEC_SERVICE_INTERCOM    10
#define BTM_SEC_SERVICE_FAX         11
#define BTM_SEC_SERVICE_HEADSET_AG  12
#define BTM_SEC_SERVICE_PNP_INFO    13
#define BTM_SEC_SERVICE_GEN_NET     14
#define BTM_SEC_SERVICE_GEN_FILE    15
#define BTM_SEC_SERVICE_GEN_AUDIO   16
#define BTM_SEC_SERVICE_GEN_TEL     17

#define BTM_SEC_SERVICE_FIRST_EMPTY 18

// Number of services that can be registered with security manager
#define BTM_SEC_MAX_SERVICE_RECORDS     32

// Following bits can be provided by host in the trusted_mask field
#define BTM_SEC_TRUST_SDP_SERVER    (1 << BTM_SEC_SERVICE_SDP_SERVER)
#define BTM_SEC_TRUST_SERIAL_PORT   (1 << BTM_SEC_SERVICE_SERIAL_PORT)
#define BTM_SEC_TRUST_LAN_ACCESS    (1 << BTM_SEC_SERVICE_LAN_ACCESS)
#define BTM_SEC_TRUST_DUN           (1 << BTM_SEC_SERVICE_DUN)
#define BTM_SEC_TRUST_IRMCSYBC      (1 << BTM_SEC_SERVICE_IRMCSYBC)
#define BTM_SEC_TRUST_OBEX_PUSH     (1 << BTM_SEC_SERVICE_OBEX_PUSH)
#define BTM_SEC_TRUST_OBEX_FTP      (1 << BTM_SEC_SERVICE_OBEX_FTP)
#define BTM_SEC_TRUST_IRMCSYNCCMD   (1 << BTM_SEC_SERVICE_IRMCSYNCCMD)
#define BTM_SEC_TRUST_HEADSET       (1 << BTM_SEC_SERVICE_HEADSET)
#define BTM_SEC_TRUST_CORDLESS      (1 << BTM_SEC_SERVICE_CORDLESS)
#define BTM_SEC_TRUST_IINTERCOM     (1 << BTM_SEC_SERVICE_IINTERCOM)
#define BTM_SEC_TRUST_FAX           (1 << BTM_SEC_SERVICE_FAX)
#define BTM_SEC_TRUST_HEADSET_AG    (1 << BTM_SEC_SERVICE_HEADSET_AG)
#define BTM_SEC_TRUST_PNP_INFO      (1 << BTM_SEC_SERVICE_PNP_INFO)
#define BTM_SEC_TRUST_GEN_NET       (1 << BTM_SEC_SERVICE_GEN_NET)
#define BTM_SEC_TRUST_GEN_FILE      (1 << BTM_SEC_SERVICE_GEN_FILE)
#define BTM_SEC_TRUST_GEN_AUDIO     (1 << BTM_SEC_SERVICE_GEN_AUDIO)
#define BTM_SEC_TRUST_GEN_TEL       (1 << BTM_SEC_SERVICE_GEN_TEL)

#define BTM_SEC_TRUST_ALL			0xFFFFFFFF



// BtIfObexHeaders.cpp : Definitions for the OBEX headers objects.
//

#define OBEX_MINIMUM_MTU            (255)
#define OBEX_MAX_CLIENTS            4
#define OBEX_MAX_SERVERS            4


#ifdef _WIN32_WCE
    #define OBEX_MAX_CSESSIONS          1
    #define OBEX_MAX_SSESSIONS          1           /* Current version only allows 1 session per server */
#else
    #define OBEX_MAX_CSESSIONS          4
    #define OBEX_MAX_SSESSIONS          7
#endif

#define OBEX_MAX_TARGET             3
#define OBEX_MAX_AUTH_CHALLENGE     3
#define OBEX_MAX_AUTH_RESPONSE      3
#define OBEX_MAX_HTTP               3
#define OBEX_MAX_APP_PARAM          3
#define OBEX_MAX_USER_HDR           4
#define OBEX_TIMER_SEED             60

#define OBEX_TIME_LOCAL		0x01
#define OBEX_TIME_UTC		0x02
#define OBEX_API 
#define OBEX_CLIENT_INCLUDED        (TRUE)
#define OBEX_SERVER_INCLUDED        (TRUE)

#ifndef BT_TYPES_H
#define GKI_BUF3_SIZE           700
#endif
#define OBEX_DATA_POOL_SIZE         (GKI_BUF3_SIZE)

#define OBEX_DATA_BUFFER_OVERHEAD   (41)
// for now, limit MTU to 640 - overhead 
#ifndef BT_TYPES_H
#define OBEX_DESIRED_MTU            (640 - OBEX_DATA_BUFFER_OVERHEAD)
#endif
#if (OBEX_DESIRED_MTU + OBEX_DATA_BUFFER_OVERHEAD > OBEX_DATA_POOL_SIZE)
#define OBEX_DEFAULT_MTU            (OBEX_DATA_POOL_SIZE - OBEX_DATA_BUFFER_OVERHEAD)
#else
#define OBEX_DEFAULT_MTU            (OBEX_DESIRED_MTU)
#endif

/*
** Define OBEX Request Codes
*/
#define OBEX_REQ_CONNECT                0x00
#define OBEX_REQ_DISCONNECT             0x01
#define OBEX_REQ_PUT                    0x02
#define OBEX_REQ_GET                    0x03
#define OBEX_REQ_SETPATH                0x05
#define OBEX_REQ_ABORT                  0x7f
#define OBEX_FINAL                      0x80
#define OBEX_UNREGISTER                 0xff

/*
** Define well-known OBEX Client data types
*/
typedef UINT32  tOBEX_CLIENT_HANDLE;    /* Application Handle */
typedef UINT32  tOBEX_CSESSION_HANDLE;  /* Session Handle */

/*
** Define well-known OBEX Server data types
*/
typedef UINT32   tOBEX_SERVER_HANDLE;    /* Application Handle */
typedef UINT32   tOBEX_SSESSION_HANDLE;  /* Session Handle */


/*
** Define OBEX error codes returned by OBEX API Functions and Application
** Callback Functions
*/
typedef enum
{
    OBEX_SUCCESS = 0,       /* Operation was successful or accepted */
    OBEX_FAIL,              /* Operation failed or was rejected */
    OBEX_ERROR,             /* Internal OBEX error */
    OBEX_ERR_RESOURCES,     /* Insufficient resources */
    OBEX_ERR_NO_CB,         /* Calback for request is missing */
    OBEX_ERR_DUP_SERVER,    /* Server for 'Target' already register with OBEX */
    OBEX_ERR_RESPONSE,      /* Peer rejected request */
    OBEX_ERR_UNK_APP,       /* Unknown Application Handle (unregistered?) */
    OBEX_ERR_PARAM,         /* Invalid or missing parameter value */
    OBEX_ERR_CLOSED,        /* Session is closed */
    OBEX_ERR_ABORTED,       /* Operation was aborted */
    OBEX_ERR_STATE,         /* Request is invalid for current state */
    OBEX_ERR_NA,            /* API call not allowed at this time */
    OBEX_ERR_HEADER,        /* Invalid data in tOBEX_HEADERS */
    OBEX_ERR_TOO_BIG,       /* The data presented in the tOBEX_HEADERS */
                            /* structure is larger than the maximum */
                            /* size allowed for the request */
    OBEX_ERR_TIMEOUT        /* Timeout */
} tOBEX_ERRORS;
#define OBEX_MAX_ERROR OBEX_ERR_TIMEOUT

/*
** Define OBEX Response Code values used in OBEX Responses sent by
** the OBEX Server to the OBEX Client.  These codes are taken 
** from the 'IrDA Object Exchange Protocol (IROBEX)' specification
** with the high bit (final bit) removed.  The OBEX Core internally
** manages the 'final bit'.
*/
typedef enum
{
    OBEX_RSP_DEFAULT                        = 0x00,
    OBEX_RSP_CONTINUE                       = 0x10,
    OBEX_RSP_OK                             = 0x20,
    OBEX_RSP_CREATED                        = 0x21,
    OBEX_RSP_ACCEPTED                       = 0x22,
    OBEX_RSP_NON_AUTHORITATIVE_INFO         = 0x23,
    OBEX_RSP_NO_CONTENT                     = 0x24,
    OBEX_RSP_RESET_CONTENT                  = 0x25,
    OBEX_RSP_PARTIAL_CONTENT                = 0x26,
    OBEX_RSP_MULTIPLE_CHOICES               = 0x30,
    OBEX_RSP_MOVED_PERMANENTLY              = 0x31,
    OBEX_RSP_MOVED_TEMPORARILY              = 0x32,
    OBEX_RSP_SEE_OTHER                      = 0x33,
    OBEX_RSP_NOT_MODIFIED                   = 0x34,
    OBEX_RSP_USE_PROXY                      = 0x35,
    OBEX_RSP_BAD_REQUEST                    = 0x40,
    OBEX_RSP_UNAUTHORIZED                   = 0x41,
    OBEX_RSP_PAYMENT_REQUIRED               = 0x42,
    OBEX_RSP_FORBIDDEN                      = 0x43,
    OBEX_RSP_NOT_FOUND                      = 0x44,
    OBEX_RSP_METHOD_NOT_ALLOWED             = 0x45,
    OBEX_RSP_NOT_ACCEPTABLE                 = 0x46,
    OBEX_RSP_PROXY_AUTHENTICATION_REQUIRED  = 0x47,
    OBEX_RSP_REQUEST_TIME_OUT               = 0x48,
    OBEX_RSP_CONFLICT                       = 0x49,
    OBEX_RSP_GONE                           = 0x4A,
    OBEX_RSP_LENGTH_REQUIRED                = 0x4B,
    OBEX_RSP_PRECONDITION_FAILED            = 0x4C,
    OBEX_RSP_REQUESTED_ENTITY_TOO_LARGE     = 0x4D,
    OBEX_RSP_REQUEST_URL_TOO_LARGE          = 0x4E,
    OBEX_RSP_UNSUPPORTED_MEDIA_TYPE         = 0x4F,
    OBEX_RSP_INTERNAL_SERVER_ERROR          = 0x50,
    OBEX_RSP_NOT_IMPLEMENTED                = 0x51,
    OBEX_RSP_BAD_GATEWAY                    = 0x52,
    OBEX_RSP_SERVICE_UNAVAILABLE            = 0x53,
    OBEX_RSP_GATEWAY_TIMEOUT                = 0x54,
    OBEX_RSP_HTTP_VERSION_NOT_SUPPORTED     = 0x55,
    OBEX_RSP_DATABASE_FULL                  = 0x60,
    OBEX_RSP_DATABASE_LOCKED                = 0x61
} tOBEX_RESPONSE_CODE;

/*
** The tOBEX_CLIENT_HANDLE, tOBEX_SERVER_HANDLE, tOBEX_CSESSION_HANDLE 
** and tOBEX_SSESSION_HANDLE each consist of two 16 bit values.  The high 
** order 16 bits contain an arbitrary ordinal assigned when the client
** or server registers or when a session is opened.  The low order 16
** bits contain the tOBEX_GLOBAL_CLIENT.client_ctrl, 
** tOBEX_GLOBAL_SERVER.server_ctrl, tOBEX_CLIENT_CTRL.cses_ctrl or
** tOBEX_SERVER_CTRL.sses_ctrl index value for the client, server or 
** session.  The following macros encode an ordinal and index into a
** tOBEX_CLIENT_HANDLE, tOBEX_SERVER_HANDLE, tOBEX_CSESSION_HANDLE or
** tOBEX_SSESSION_HANDLE and decode an ordinal and index from a
** handle.
*/
#define OBEX_ENCODE_HANDLE(handle,ordinal,index) (handle = ((ordinal << 16) | (index & 0xFFFF)))
#define OBEX_DECODE_HANDLE(handle,ordinal,index) (ordinal = (handle >> 16), index = (handle & 0xFFFF))

/*
* The following defines and describes the tOBEX_HEADERS structure
* and the other structures that are embedded in the tOBEX_HEADERS
* structure.
*
* The tOBEX_HEADERS structure is used by many calls to OBEX API 
* and Application Callback Functions.  It contains the information 
* found in the OBEX Headers supported by this implementation of 
* OBEX as defined in the 'IrDA Object Exchange Protocol (IrOBEX)' 
* specification.  
*
* When an application wants to send information to the peer 
* application it creates an instance of the tOBEX_HEADERS structure
* and passes a pointer to the structure to the appropriate OBEX API
* Function.  OBEX will process the information found in the 
* structure and return to the application.  If the application has
* registered with OBEX as being 'well behaved' (see OBEX_ClientRegister
* and OBEX_ServerRegister functions), OBEX will assume that the 
* tOBEX_HEADERS pointer points to a GKI buffer (without the BT_HDR
* prefix) and will assume ownership of the buffer.  If the application
* has not registered as 'well behaved', OBEX will assume that the 
* tOBEX_HEADERS pointer points to static memory and will not alter
* the memory pointed to by the pointer.  For best performance, it
* is recommended that applications be well behaved.
*
* When OBEX receives information from the peer OBEX that it needs to
* deliver to the application, it creates an instance of the 
* tOBEX_HEADERS structure and passes a pointer to it to the appropriate
* Application Callback Function.  The tOBEX_HEADERS structure resides
* in a GKI buffer (without the BT_HDR prefix).  If the application has
* registered as 'well behaved', OBEX assumes that the application will
* assume ownership of the buffer and release it as appropriate.  If the
* application has not registered as 'well behaved', OBEX assumes that 
* the application treats the buffer as static memory and once the
* Application Callback Function returns to OBEX, OBEX will dispose of
* the buffer appropriately.
*/


/*
* The tOBEX_UNI_HDR structure is embedded in the tOBEX_HEADERS 
* structure.  It is used to hold the contents of an OBEX 
* 'UNICODE' type header.  These headers contain null-terminated
* UNICODE text strings.  The struct contains a 'p_string' member
* that points to the null-terminated UNICODE string and a 'str_len'
* member that indicates the number of characters in the UNICODE string.
* NOTE: The 'str_len' member includes the terminating NULL
* character.  Therefore, by example, if a Name header contains
* the UNICODE text string "JUMAR.TXT", the 'p_string' member of the
* struct will point to a data area containing
* the hex values (displayed in Big-endian format):
* "00 4A 00 55 00 4D 00 41 00 52 00 2E 00 54 00 58 00 54 00 00"
* and the 'str_len' member will contain the value 10.
*
* If a 'UNICODE' type header present in an OBEX request or
* response, the contents of that header will be stored in a 
* tOBEX_UNI_HDR struct and the 'flag' member of the tOBEX_HEADERS
* struct will indicate that the header is present.  Some headers may
* be present but contain no data.  In such a case, the 'flag' member
* of the tOBEX_HEADERS struct will indicate that the header is 
* present but the 'str_len' member and 'p_string' member of the 
* associated header struct will be zero.
*
* If the application has registered with OBEX as being 'well behaved',
* it is assumed that each 'p_string' member in use points to a GKI
* buffer and ownership of each buffer will follow the rules specified
* above for the tOBEX_HEADERS structure.  If the application has not
* registered with OBEX as being 'well behaved', the OBEX API functions
* assume that each 'p_string' member points to static memory it does
* not alter that memory.  OBEX also assumes that the Application
* Callback Functions will treat the memory pointed to by each 'p_string'
* member as static and will not alter that memory.
*/

/* OBEX UNICODE Type Header - null terminated UNICODE string */
typedef struct
{ 
    UINT32 str_len; 
    WCHAR  *p_string; 
} tOBEX_UNI_HDR; 


/*
* The tOBEX_OCTET_HDR structure is embedded in the tOBEX_HEADERS 
* structure.  It is used to hold the contents of an OBEX 
* 'octet-array' type header.  These headers contain unstructured octet 
* arrays.  The struct contains a 'p_array' member that points to an 
* array of octets and a 'length' member that indicates the length of 
* the array.
*
* If a header of the 'octet-array' type is present in an OBEX request or
* response, the contents of that header will be stored in its 
* corresponding struct and the 'flag' member of the tOBEX_HEADERS
* struct will indicate that the header is present.  Some headers may
* be present but contain no data.  In such a case, the 'flag' member
* of the tOBEX_HEADERS struct will indicate that the header is 
* present but the 'length' member and 'p_array' member of the 
* associated header struct will be zero.
*
* Some header types may occur multiple times in a request or response.
* For these types of headers, the tOBEX_HEADERS structure contains
* an array of embedded structures for that header type and a counter
* indicating the number of those headers that are present.  The 
* 'flag' member of the tOBEX_HEADERS struct will indicate that at 
* least one instance of the header is present in the request/response
* and the corresponding 'num_...' member of the tOBEX_HEADERS struct
* will indicate the actual number of instances included in the array
* of structures.
*
* If the application has registered with OBEX as being 'well behaved',
* it is assumed that each 'p_array' member in use points to a GKI
* buffer and ownership of each buffer will follow the rules specified
* above for the tOBEX_HEADERS structure.  If the application has not
* registered with OBEX as being 'well behaved', the OBEX API functions
* assume that each 'p_array' member points to static memory it does
* not alter that memory.  OBEX also assumes that the Application
* Callback Functions will treat the memory pointed to by each 'p_array'
* member as static and will not alter that memory.
*/

/* OBEX Octet-Array Type Header - unstructured octet array */
typedef struct
{ 
    UINT32 length; 
    UINT8 *p_array; 
} tOBEX_OCTET_HDR; 



/*
* The tOBEX_HEADERS structure contains three arrays of the 
* following tOBEX_HDR_TRIPLET structure to hold information 
* found in the OBEX Application Request-Response Parameters 
* Header, the Authenticate Challenge Header and the 
* Authenticate Response Header.  Each of these headers contain
* one or more 'Triplets' and each triplet is stored in an
* entry in its associated tOBEX_HDR_TRIPLET array.  The 
* tOBEX_HEADERS structure contains a 'num_app_params' member
* that indicates the number of triplets contained in the 
* Application Request-Response Parameters Header, a 
* 'num_auth_challenge' member that indicates the number of
* triplets contained in the Authenticate Challenge Header and
* a 'num_auth_response' member that indicates the number of
* triplets contains in the Authenticate Response Header.
*
* If the application has registered with OBEX as being 'well behaved',
* it is assumed that each 'p_array' member in use points to a GKI
* buffer and ownership of each buffer will follow the rules specified
* above for the tOBEX_HEADERS structure.  If the application has not
* registered with OBEX as being 'well behaved', the OBEX API functions
* assume that each 'p_array' member points to static memory it does
* not alter that memory.  OBEX also assumes that the Application
* Callback Functions will treat the memory pointed to by each 'p_array'
* member as static and will not alter that memory.
*/
typedef struct obex_hdr_triplet_s 
{ 
    UINT8   tag;
    UINT8   length;
    UINT8   *p_array;
} tOBEX_HDR_TRIPLET;

/*
* The tOBEX_HEADERS structure contains an array of the 
* following tOBEX_USER_HDR structure to hold information 
* found in the OBEX User Defined Headers.  The OBEX User
* Defined Header is formatted according to the 'IrDA Object 
* Exchange Protocol (IrOBEX)' specification.  The specification
* requires that the two high-order bits of the first octet of 
* the header indicate the format and, by implication, the size 
* of the data contained in the header and the remaining bits of 
* the first octet be within the range of 0x30 to 0x3F.
*
* By performing a logical AND on the 'id' member of the 
* tOBEX_USER_HDR structure and the OBEX_USER_TYPE_MASK constant
* and comparing the results with each of the other 
* 'OBEX_USER_TYPE_....' constants, the User Defined Header 
* format can be determined.  The appropriate member of the 
* 'value' union can then be accessed and the 'length' member
* can be interpreted as follows:
*
*   ---------------------- ----------- -------------------
*  |  If the high-order   |           |                   |
*  |  2 bits of id are    |  Then     |  Contains         |
*  |----------------------|-----------|-------------------|
*  | OBEX_USER_TYPE_UNI   | p_string  | Pointer to null   |
*  |                      |           | terminated        |
*  |                      |           | UNICODE text      |
*  |                      |           | string            |
*  |                      |-----------|-------------------|
*  |                      | length    | Number of         |
*  |                      |           | characters in     |
*  |                      |           | the UNICODE text  |
*  |                      |           | string (including |
*  |                      |           | the terminating   |
*  |                      |           | NULL character)   |
*  |----------------------|-----------|-------------------|
*  | OBEX_USER_TYPE_ARRAY | p_array   | Pointer to        |
*  |                      |           | unstructured      |
*  |                      |           | octet array       |
*  |                      |-----------|-------------------|
*  |                      | length    | Number octets of  |
*  |                      |           | data in the array |
*  |----------------------|-----------|-------------------|
*  | OBEX_USER_TYPE_BYTE  | user_byte | One byte of       |
*  |                      |           | user data         |
*  |                      |-----------|-------------------|
*  |                      | length    | N/A               |
*  |----------------------|-----------|-------------------|
*  | OBEX_USER_TYPE_INT   | user_int  | One 32-bit        |
*  |                      |           | integer of user   |
*  |                      |           | data              |
*  |                      |-----------|-------------------|
*  |                      | length    | N/A               |
*   ---------------------- ----------- -------------------
*
* If the application has registered with OBEX as being 'well behaved',
* it is assumed that each 'p_array' and 'p_string' member in use 
* points to a GKI buffer and ownership of each buffer will follow the 
* rules specified above for the tOBEX_HEADERS structure.  If the 
* application has not registered with OBEX as being 'well behaved', 
* the OBEX API functions assume that each 'p_array' member points to 
* static memory it does not alter that memory.  OBEX also assumes 
* that the Application Callback Functions will treat the memory 
* pointed to by each 'p_array' member as static and will not alter 
* that memory.
*/
typedef struct 
{ 
    UINT8   id;
#define OBEX_USER_TYPE_MASK  0xC0
#define OBEX_USER_TYPE_UNI   0x00 /* Null terminated ASCII text */
#define OBEX_USER_TYPE_ARRAY 0x40 /* Unstructured octet array */
#define OBEX_USER_TYPE_BYTE  0x80 /* Single byte */
#define OBEX_USER_TYPE_INT   0xC0 /* 32 bit integer */
    UINT16  length;
    union
    {
        WCHAR   *p_string;
        UINT8   *p_array;
        UINT8   user_byte;
        UINT32  user_int;
    } value;
} tOBEX_USER_HDR;



/*
* The following is the tOBEX_HEADERS structure.
*
* The 'rsp_code' member contains the OBEX Response Code used in the
* OBEX Response Packet.  It is stored without the high-order (final) 
* bit set.  The OBEX Core manages the 'final bit' in Response 
* Packets internally.  When the Server Application calls an OBEX 
* Server API Function it indicates the success or failure of the request
* by passing a tOBEX_ERRORS enumerator to the appropriate API.  If the
* Server Application wants to be more specific in the type of error 
* returned to the Client Application, it can set the specific OBEX 
* Response Code desired in the 'rsp_code' member of the tOBEX_HEADERS
* structure.  If the Server Application does not set the 'rsp_code'
* member to a valid tOBEX_ERRORS enumerator, the OBEX Core will
* generate an appropriate OBEX Response Code based on the tOBEX_ERRORS
* enumerator passed as a parameter to the Confirmation API Function and
* the current operation being performed.
*
* The 'flag' member indicates which headers are present.
*
* If the 'flag' member indicates that the Count Header is 
* present, the 'count' member of the tOBEX_HEADERS structure
* will contain the value found in the Count Header.
*
* If the 'flag' member indicates that the Name Header, Type Header,
* Description Header, Target Header, HTTP Header, Body (or 
* End-of-Body) Header, Who Header, Application Request-Response Header, 
* Authentication Challenge Header, Authentication Response Header, 
* Object Class Header or User Defined Header is present, the 
* corresponding data will be present in the tOBEX_HEADERS structure 
* as specified above.
*
* If the 'flag' member indicates that the Length Header is
* present, the 'hint_of_length' member of the tOBEX_HEADERS 
* structure will contain the value found in the Length Header.
*
* THE FOLLOWING DESCRIPTION APPLIES ONLY TO THE OBEX NON_LEGACY API:    
* If the 'flag' member indicates that the Time Header is present, 
* the 'time' member contains the contents of the Time Header
* as seconds since midnight, January 1, 1970, UTC ('Universal
* Coordinated Time' also known as GMT or 'Greenwich Mean Time') or
* Local Time.  The 'time_qualifier' member of the tOBEX_HEADERS 
* structure indicates whether the time in the header is UTC or Local 
* Time.  The 'IrDA Object Exchange Protocol (IrOBEX)' specification 
* recommends that the Time Header be formatted according to ISO 8601.
* The OBEX core performs the conversion between the ISO 8601 format
* and the tOBEX_HEADERS structure format as appropriate.
* END OF OBEX NON-LEGACY API DESCRIPTION
*
* NOTE!!!!!
*   The bit positions for each header represented in the 'flag' member
*   must not be altered because the OBEX Core relies on them being
*   assigned as defined here.
* 
*/
typedef struct
{
    UINT32              flag;
#define OBEX_FLAG_INTERNAL1         0x00000001  /* Used internally by OBEX Core */
#define OBEX_FLAG_COUNT             0x00000002 
#define OBEX_FLAG_NAME              0x00000004 
#define OBEX_FLAG_TYPE              0x00000008 
#define OBEX_FLAG_LENGTH            0x00000010 
#define OBEX_FLAG_OBJECT_CLASS      0x00000020 
#define OBEX_FLAG_TARGET            0x00000040 
#define OBEX_FLAG_WHO               0x00000080 
#define OBEX_FLAG_TIME              0x00000100 
#define OBEX_FLAG_INTERNAL2         0x00000200  /* Used internally by OBEX Core */
#define OBEX_FLAG_DESCRIPTION       0x00000400 
#define OBEX_FLAG_AUTH_CHALLENGE    0x00000800 
#define OBEX_FLAG_AUTH_RESPONSE     0x00001000 
#define OBEX_FLAG_HTTP              0x00002000 
#define OBEX_FLAG_APPL_REQ_RSP      0x00004000 
#define OBEX_FLAG_USER_DEFINED      0x00008000
#define OBEX_FLAG_BODY              0x00010000
#define OBEX_FLAG_BODY_END          0x00020000
/* flag bits legal for applications to set */
#define OBEX_FLAG_LEGAL_BITS \
       (OBEX_FLAG_COUNT + \
        OBEX_FLAG_NAME + \
        OBEX_FLAG_TYPE + \
        OBEX_FLAG_LENGTH + \
        OBEX_FLAG_OBJECT_CLASS + \
        OBEX_FLAG_TARGET + \
        OBEX_FLAG_WHO + \
        OBEX_FLAG_TIME + \
        OBEX_FLAG_DESCRIPTION + \
        OBEX_FLAG_AUTH_CHALLENGE + \
        OBEX_FLAG_AUTH_RESPONSE + \
        OBEX_FLAG_HTTP + \
        OBEX_FLAG_APPL_REQ_RSP + \
        OBEX_FLAG_USER_DEFINED + \
        OBEX_FLAG_BODY + \
        OBEX_FLAG_BODY_END)
    UINT32              internal1;  /* Used internally by OBEX Core */
    UINT32              count;
    tOBEX_UNI_HDR       uni_name;
    tOBEX_OCTET_HDR     type;
    UINT32              hint_of_length;
    tOBEX_OCTET_HDR     object_class;
    UINT32              num_target;
    tOBEX_OCTET_HDR     target[OBEX_MAX_TARGET];
    tOBEX_OCTET_HDR     who;
    UINT32              time_value;
    UINT8               time_qualifier;
#define OBEX_TIME_LOCAL     0x01
#define OBEX_TIME_UTC       0x02
    tOBEX_UNI_HDR       uni_description;
    UINT32              num_auth_challenge;
    tOBEX_HDR_TRIPLET   auth_challenge[OBEX_MAX_AUTH_CHALLENGE];
    UINT32              num_auth_response;
    tOBEX_HDR_TRIPLET   auth_response[OBEX_MAX_AUTH_RESPONSE];
    UINT32              num_http;
    tOBEX_OCTET_HDR     http[OBEX_MAX_HTTP];
    UINT32              num_app_param;
    tOBEX_HDR_TRIPLET   app_param[OBEX_MAX_APP_PARAM];
    UINT32              num_user;
    tOBEX_USER_HDR      user[OBEX_MAX_USER_HDR];
    tOBEX_OCTET_HDR     body;
} tOBEX_HEADERS;


#endif // !defined (WIDCOMMSDK_EXPORTS)

/* Define the L2CAP connection result codes
*/
#define L2CAP_CONN_OK                0
#define L2CAP_CONN_PENDING           1
#define L2CAP_CONN_NO_PSM            2
#define L2CAP_CONN_SECURITY_BLOCK    3
#define L2CAP_CONN_NO_RESOURCES      4
#define L2CAP_CONN_TIMEOUT           0xEEEE

#define L2CAP_CONN_NO_LINK           255        /* Add a couple of our own for internal use */


// Define a structure to hold the configuration parameters. Since the
// parameters are optional, for each parameter there is a boolean to
// use to signify its presence or absemce.
//
// NOTE:  This structure is used externally from (or above) the SDK
//        and is mapped to the tL2CAP_CFG_INFO structure when used
//        in the CL2CapConn::Reconfigure method.  This is done so we
//        only expose BOOL types externally.
//
typedef struct
{
    UINT16      result;                 // Only used in confirm messages
    BOOL        mtu_present;
    UINT16      mtu;
    BOOL        qos_present;
    FLOW_SPEC   qos;
    BOOL        flush_to_present;
    UINT16      flush_to;

    UINT16      flags;                  // Internally used by L2CAP

} tL2CAP_CONFIG_INFO;

#pragma pack ()

#endif // !defined(AFX_WIDCOMMTYPES_H__1F5ED990_6FC6_4B0D_882C_8D7C98C16A06__INCLUDED_)
