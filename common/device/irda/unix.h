
/* 1. irda.h version 1.1 created by Dag Brattli <dagb@cs.uit.no>
 * 2. modified for OpenObex project (http://sourceforge.net/projects/openobex)
 * 3. modified for Gammu
 */

#ifndef IRDA_H
#define IRDA_H

#include <sys/types.h>
#include <sys/socket.h>

/* Hint bit positions for first hint byte */
#define HINT_PNP         0x01
#define HINT_PDA         0x02
#define HINT_COMPUTER    0x04
#define HINT_PRINTER     0x08
#define HINT_MODEM       0x10
#define HINT_FAX         0x20
#define HINT_LAN         0x40
#define HINT_EXTENSION   0x80

/* Hint bit positions for second hint byte (first extension byte) */
#define HINT_TELEPHONY   0x01
#define HINT_FILE_SERVER 0x02
#define HINT_COMM        0x04
#define HINT_MESSAGE     0x08
#define HINT_HTTP        0x10
#define HINT_OBEX        0x20

/* IrLMP character code values */
#define CS_ASCII         0x00
#define	CS_ISO_8859_1    0x01
#define	CS_ISO_8859_2    0x02
#define	CS_ISO_8859_3    0x03
#define	CS_ISO_8859_4    0x04
#define	CS_ISO_8859_5    0x05
#define	CS_ISO_8859_6    0x06
#define	CS_ISO_8859_7    0x07
#define	CS_ISO_8859_8    0x08
#define	CS_ISO_8859_9    0x09
#define CS_UNICODE       0xff

/* These are the currently known dongles */
typedef enum {
	IRDA_TEKRAM_DONGLE       = 0,
	IRDA_ESI_DONGLE          = 1,
	IRDA_ACTISYS_DONGLE      = 2,
	IRDA_ACTISYS_PLUS_DONGLE = 3,
	IRDA_GIRBIL_DONGLE       = 4,
	IRDA_LITELINK_DONGLE     = 5,
	IRDA_AIRPORT_DONGLE      = 6,
	IRDA_OLD_BELKIN_DONGLE   = 7,
} IRDA_DONGLE;

/* Protocol types to be used for SOCK_DGRAM */
enum {
	IRDAPROTO_UNITDATA = 0,
	IRDAPROTO_ULTRA    = 1,
	IRDAPROTO_MAX
};

#define SOL_IRLMP      266 /* Same as SOL_IRDA for now */
#define SOL_IRTTP      266 /* Same as SOL_IRDA for now */

#define IRLMP_ENUMDEVICES        1	/* Return discovery log */
#define IRLMP_IAS_SET            2	/* Set an attribute in local IAS */
#define IRLMP_IAS_QUERY          3	/* Query remote IAS for attribute */
#define IRLMP_HINTS_SET          4	/* Set hint bits advertised */
#define IRLMP_QOS_SET            5
#define IRLMP_QOS_GET            6
#define IRLMP_MAX_SDU_SIZE       7
#define IRLMP_IAS_GET            8	/* Get an attribute from local IAS */
#define IRLMP_IAS_DEL		 9	/* Remove attribute from local IAS */
#define IRLMP_HINT_MASK_SET	10	/* Set discovery filter */
#define IRLMP_WAITDEVICE	11	/* Wait for a new discovery */

#define IRTTP_MAX_SDU_SIZE IRLMP_MAX_SDU_SIZE /* Compatibility */

/* LM-IAS Limits */
#define IAS_MAX_STRING         256
#define IAS_MAX_OCTET_STRING  1024
#define IAS_MAX_CLASSNAME       64
#define IAS_MAX_ATTRIBNAME     256

/* LM-IAS Attribute types */
#define IAS_MISSING 0
#define IAS_INTEGER 1
#define IAS_OCT_SEQ 2
#define IAS_STRING  3

#define LSAP_ANY              0xff

struct sockaddr_irda {
	sa_family_t	irdaAddressFamily;	/* AF_IRDA */
	u_int8_t	sir_lsap_sel; 		/* LSAP selector */
	u_int32_t	irdaDeviceID;		/* Device address */
	char		irdaServiceName[25]; 	/* Usually <service>:IrDA:TinyTP */
};

struct irda_device_info {
	u_int32_t	saddr; 		  	/* Address of local interface */
	u_int32_t	irdaDeviceID;		/* Address of remote device */
	char		irdaDeviceName[22]; 	/* Description */
	u_int8_t	charset; 		/* Charset used for description */
	u_int8_t	hints[2]; 		/* Hint bits */
};

struct irda_device_list {
       u_int32_t 		numDevice;
       struct irda_device_info 	Device[1];
};

struct irda_ias_set {
	char irda_class_name[IAS_MAX_CLASSNAME];
	char irda_attrib_name[IAS_MAX_ATTRIBNAME];
	unsigned int irda_attrib_type;
	union {
		unsigned int irda_attrib_int;
		struct {
			unsigned short len;
			u_int8_t octet_seq[IAS_MAX_OCTET_STRING];
		} irda_attrib_octet_seq;
		struct {
			u_int8_t len;
			u_int8_t charset;
			u_int8_t string[IAS_MAX_STRING];
		} irda_attrib_string;
	} attribute;
	u_int32_t       daddr;    /* Address of device (for IAS query only) */
};

/* Some private IOCTL's (max 16) */
#define SIOCSDONGLE    (SIOCDEVPRIVATE + 0)
#define SIOCGDONGLE    (SIOCDEVPRIVATE + 1)
#define SIOCSBANDWIDTH (SIOCDEVPRIVATE + 2)
#define SIOCSMEDIABUSY (SIOCDEVPRIVATE + 3)
#define SIOCGMEDIABUSY (SIOCDEVPRIVATE + 4)
#define SIOCGRECEIVING (SIOCDEVPRIVATE + 5)
#define SIOCSMODE      (SIOCDEVPRIVATE + 6)
#define SIOCGMODE      (SIOCDEVPRIVATE + 7)
#define SIOCSDTRRTS    (SIOCDEVPRIVATE + 8)
#define SIOCGQOS       (SIOCDEVPRIVATE + 9)

/* No reason to include <linux/if.h> just because of this one ;-) */
#define IRNAMSIZ 16 

/* IrDA quality of service information (must not exceed 16 bytes) */
struct if_irda_qos {
	u_int32_t baudrate;
	u_int16_t data_size;
	u_int16_t window_size;
	u_int16_t min_turn_time;
	u_int16_t max_turn_time;
	u_int8_t  add_bofs;
	u_int8_t  link_disc;
};

/* For setting RTS and DTR lines of a dongle */
struct if_irda_line {
	u_int8_t dtr;
	u_int8_t rts;
};

/* IrDA interface configuration (data part must not exceed 16 bytes) */
struct if_irda_req {
	union {
		char ifrn_name[IRNAMSIZ];  /* if name, e.g. "irda0" */
	} ifr_ifrn;
	
	/* Data part */
	union {
		struct if_irda_line ifru_line;
		struct if_irda_qos  ifru_qos;
		unsigned short      ifru_flags;
		unsigned int        ifru_receiving;
		unsigned int        ifru_mode;
		unsigned int        ifru_dongle;
	} ifr_ifru;
};

#define ifr_baudrate  ifr_ifru.ifru_qos.baudrate
#define ifr_receiving ifr_ifru.ifru_receiving 
#define ifr_dongle    ifr_ifru.ifru_dongle
#define ifr_mode      ifr_ifru.ifru_mode
#define ifr_dtr       ifr_ifru.ifru_line.dtr
#define ifr_rts       ifr_ifru.ifru_line.rts

#endif /* IRDA_H */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
