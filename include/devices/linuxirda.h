/*********************************************************************
 *                
 * Filename:      irda.h
 * Version:       1.0
 * Description:   IrDA header file to be used by IrDA applications
 * Status:        Experimental.
 * Author:        Dag Brattli <dagb@cs.uit.no>
 * Created at:    Mon Mar  8 14:06:12 1999
 * Modified at:   Sat Dec 25 16:07:54 1999
 * Modified by:   Dag Brattli <dagb@cs.uit.no>
 * 
 *     Copyright (c) 1999 Dag Brattli, All Rights Reserved.
 *      
 *     This program is free software; you can redistribute it and/or 
 *     modify it under the terms of the GNU General Public License as 
 *     published by the Free Software Foundation; either version 2 of 
 *     the License, or (at your option) any later version.
 *  
 *     Neither Dag Brattli nor University of Tromsø admit liability nor
 *     provide warranty for any of this software. This material is 
 *     provided "AS-IS" and at no charge.
 *
 ********************************************************************/

/*
 * $Id: linuxirda.h,v 1.3 2001/07/05 10:54:53 pkot Exp $
 *
 */

#ifndef IRDA_H
#define IRDA_H

#include <sys/types.h>
#include <inttypes.h>

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

#define IRLMP_ENUMDEVICES        1
#define IRLMP_IAS_SET            2
#define IRLMP_IAS_QUERY          3
#define IRLMP_HINTS_SET          4
#define IRLMP_QOS_SET            5
#define IRLMP_QOS_GET            6
#define IRLMP_MAX_SDU_SIZE       7
#define IRLMP_IAS_GET            8

#define IRTTP_MAX_SDU_SIZE IRLMP_MAX_SDU_SIZE /* Compatibility */

/* LM-IAS Limits */
#define IAS_MAX_STRING        256
#define IAS_MAX_OCTET_STRING 1024
#define IAS_MAX_CLASSNAME      64
#define IAS_MAX_ATTRIBNAME    256

/* LM-IAS Attribute types */
#define IAS_MISSING 0
#define IAS_INTEGER 1
#define IAS_OCT_SEQ 2
#define IAS_STRING  3

#define LSAP_ANY              0xff
#define DEV_ADDR_ANY  0xffffffff

struct sockaddr_irda {
	sa_family_t   sir_family;   /* AF_IRDA */
	uint8_t      sir_lsap_sel; /* LSAP selector */
	uint32_t     sir_addr;     /* Device address */
	char          sir_name[25]; /* Usually <service>:IrDA:TinyTP */
};

struct irda_device_info {
	uint32_t     saddr;    /* Address of local interface */
	uint32_t     daddr;    /* Address of remote device */
	char          info[22]; /* Description */
	uint8_t      charset;  /* Charset used for description */
	uint8_t      hints[2]; /* Hint bits */
};

struct irda_device_list {
	uint32_t len;
	struct irda_device_info dev[1];
};

struct irda_ias_set {
	char irda_class_name[IAS_MAX_CLASSNAME];
	char irda_attrib_name[IAS_MAX_ATTRIBNAME];
	unsigned int irda_attrib_type;
	union {
		unsigned int irda_attrib_int;
		struct {
			unsigned short len;
			u_char octet_seq[IAS_MAX_OCTET_STRING];
		} irda_attrib_octet_seq;
		struct {
			unsigned char len;
			unsigned char charset;
			unsigned char string[IAS_MAX_STRING];
		} irda_attrib_string;
	} attribute;
        uint32_t       daddr;

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
	uint32_t baudrate;
	uint16_t data_size;
	uint16_t window_size;
	uint16_t min_turn_time;
	uint16_t max_turn_time;
	uint8_t  add_bofs;
	uint8_t  link_disc;
};

/* For setting RTS and DTR lines of a dongle */
struct if_irda_line {
	unsigned char dtr;
	unsigned char rts;
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
