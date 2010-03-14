
/* part of irda.h available in Linux kernel source */

#ifndef IRDA_H
#define IRDA_H

#include <sys/types.h>
#include <sys/socket.h>

#define SOL_IRLMP      		266	 		/* Same as SOL_IRDA for now */
#define IRLMP_ENUMDEVICES       1	 		/* Return discovery log */
#define LSAP_ANY                0xff

struct sockaddr_irda {
	sa_family_t		irdaAddressFamily;	/* AF_IRDA */
	u_int8_t		sir_lsap_sel; 		/* LSAP selector */
	u_int32_t		irdaDeviceID;		/* Device address */
	char			irdaServiceName[25]; 	/* Usually <service>:IrDA:TinyTP */
};

struct irda_device_info {
	u_int32_t		saddr; 		  	/* Address of local interface */
	u_int32_t		irdaDeviceID;		/* Address of remote device */
	char			irdaDeviceName[22]; 	/* Description */
	u_int8_t		charset; 		/* Charset used for description */
	u_int8_t		hints[2]; 		/* Hint bits */
};

struct irda_device_list {
       u_int32_t 		numDevice;
       struct irda_device_info 	Device[1];
};

#endif /* IRDA_H */

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
