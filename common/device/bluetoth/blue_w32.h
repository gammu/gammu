#ifndef __blue_w32_h
#define __blue_w32_h

#include <winsock2.h>
/* MS Platform SDK */

#include <pshpack1.h>		// Without it compiled code hangs up BT stack

typedef ULONGLONG 		BTH_ADDR, *PBTH_ADDR;

#define NAP_BIT_OFFSET          (8 * 4)
#define SAP_BIT_OFFSET          (0)

#ifdef __BORLANDC__
#define NAP_MASK                0xFFFF00000000LL
#define SAP_MASK                0x0000FFFFFFFFL

#define GET_NAP(_bth_addr)  	((ULONGLONG)  ((_bth_addr) >> NAP_BIT_OFFSET))
#define GET_SAP(_bth_addr)  	((ULONGLONG)  (((_bth_addr) & SAP_MASK) >> SAP_BIT_OFFSET))
#else
#define NAP_MASK                0xFFFF00000000LL
#define SAP_MASK                0x0000FFFFFFFFLL

#define GET_NAP(_bth_addr)  	((ULONGLONG)  (((_bth_addr) & NAP_MASK) >> NAP_BIT_OFFSET))
#define GET_SAP(_bth_addr)  	((ULONGLONG)  (((_bth_addr) & SAP_MASK) >> SAP_BIT_OFFSET))
#endif

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

#ifdef BLUETOOTH_RF_SEARCHING

#ifndef NS_BTH
#  define NS_BTH  16
#endif

#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
