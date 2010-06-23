
/* MS Platform SDK */

#ifndef __AFIRDA__
#define __AFIRDA__

#define AF_IRDA 			26
#define SOL_IRLMP 			0x00FF 
#define IRLMP_ENUMDEVICES 		0x00000010
#define IRLMP_9WIRE_MODE 		0x00000016

typedef struct sockaddr_irda {
	unsigned short 			irdaAddressFamily;
	unsigned char 			irdaDeviceID[4];
	char 				irdaServiceName[25];
};

typedef struct irda_device_info {
	unsigned char 			irdaDeviceID[4];
	char 				irdaDeviceName[22];
	unsigned char 			irdaDeviceHints1;
	unsigned char 			irdaDeviceHints2;
	unsigned char 			irdaCharSet;
};

typedef struct irda_device_list {
	ULONG 				numDevice;
	struct irda_device_info 	Device[1];
};

#endif // __AFIRDA__

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
