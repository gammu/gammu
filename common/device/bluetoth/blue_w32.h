#ifndef __blue_w32_h
#define __blue_w32_h

GSM_Error bluetooth_connect(GSM_StateMachine *s, int port, char *device);
GSM_Error bluetooth_findchannel(GSM_StateMachine *s);

#include <winsock2.h>
/* MS Platform SDK */

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

#ifdef BLUETOOTH_RF_SEARCHING

#ifndef NS_BTH
#  define NS_BTH  16
#endif

#define LUP_CONTAINERS          0x0002
#define LUP_RETURN_NAME         0x0010
#define LUP_RETURN_TYPE         0x0020
#define LUP_RETURN_COMMENT      0x0080
#define LUP_RETURN_ADDR         0x0100
#define LUP_RETURN_BLOB         0x0200
#define LUP_FLUSHCACHE       	0x1000
#define LUP_RES_SERVICE         0x8000

#ifndef WINSOCK_API_LINKAGE
#ifdef DECLSPEC_IMPORT
#define WINSOCK_API_LINKAGE DECLSPEC_IMPORT
#else
#define WINSOCK_API_LINKAGE
#endif
#endif

WINSOCK_API_LINKAGE INT WSAAPI
WSALookupServiceBeginA(
    IN  LPWSAQUERYSETA lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    );

#define WSALookupServiceBegin  WSALookupServiceBeginA

WINSOCK_API_LINKAGE INT WSAAPI
WSALookupServiceNextA(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETA   lpqsResults
    );

#define WSALookupServiceNext  WSALookupServiceNextA

WINSOCK_API_LINKAGE INT WSAAPI WSALookupServiceEnd(IN HANDLE hLookup);

#define MAX_PROTOCOL_CHAIN 7

#define SO_PROTOCOL_INFOA 0x2004      /* WSAPROTOCOL_INFOA structure */
#define SO_PROTOCOL_INFO  SO_PROTOCOL_INFOA

WINSOCK_API_LINKAGE INT WSAAPI
WSAAddressToStringA(
    IN     LPSOCKADDR          	lpsaAddress,
    IN     DWORD               	dwAddressLength,
    IN     LPWSAPROTOCOL_INFOA 	lpProtocolInfo,
    IN OUT LPSTR             	lpszAddressString,
    IN OUT LPDWORD             	lpdwAddressStringLength
    );

#define WSAAddressToString  WSAAddressToStringA

#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
