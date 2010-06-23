
/* Taken from irda.h available in Linux kernel source */

#ifndef IRDA_H
#define IRDA_H

#include <sys/types.h>
#include <sys/socket.h>

/* Same as SOL_IRDA for now */
#define SOL_IRLMP      		266
/* Return discovery log */
#define IRLMP_ENUMDEVICES       1
#define LSAP_ANY                0xff

struct sockaddr_irda {
	/* AF_IRDA */
	sa_family_t		irdaAddressFamily;
	/* LSAP selector */
	u_int8_t		sir_lsap_sel;
	/* Device address */
	u_int32_t		irdaDeviceID;
	/* Usually <service>:IrDA:TinyTP */
	char			irdaServiceName[25];
};

struct irda_device_info {
	/* Address of local interface */
	u_int32_t		saddr;
	/* Address of remote device */
	u_int32_t		irdaDeviceID;
	/* Description */
	char			irdaDeviceName[22];
	/* Charset used for description */
	u_int8_t		charset;
	/* Hint bits */
	u_int8_t		hints[2];
};

struct irda_device_list {
        u_int32_t 		numDevice;
	struct irda_device_info Device[1];
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
