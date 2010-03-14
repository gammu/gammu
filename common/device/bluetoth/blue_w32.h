
/* MS Platform SDK */

#ifndef __blue_w32_h
#define __blue_w32_h

#include <pshpack1.h>		// Without it compiled code hangs up BT stack

typedef ULONGLONG 		BTH_ADDR, *PBTH_ADDR;

#define NAP_MASK                ((ULONGLONG) 0xFFFF00000000)
#define SAP_MASK                ((ULONGLONG) 0x0000FFFFFFFF)

#define NAP_BIT_OFFSET          (8 * 4)
#define SAP_BIT_OFFSET          (0)

#define GET_NAP(_bth_addr)  	((USHORT) (((_bth_addr) & NAP_MASK) >> NAP_BIT_OFFSET))
#define GET_SAP(_bth_addr)  	((ULONG)  (((_bth_addr) & SAP_MASK) >> SAP_BIT_OFFSET))

#ifndef AF_BTH
#define AF_BTH  		32
#endif

typedef struct _SOCKADDR_BTH {
	USHORT      		addressFamily;  // Always AF_BTH
	BTH_ADDR    		btAddr;         // Bluetooth device address
	GUID        		serviceClassId; // [OPTIONAL] system will query SDP for port
	ULONG       		port;           // RFCOMM channel or L2CAP PSM
} SOCKADDR_BTH, *PSOCKADDR_BTH;

#define BTHPROTO_RFCOMM  	0x0003

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
