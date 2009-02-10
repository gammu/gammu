/* part of irda.h available in Linux kernel source (www.kernel.org) */

/*********************************************************************
 *
 * Filename:      irda.h
 * Version:
 * Description:
 * Status:        Experimental.
 * Author:        Dag Brattli <dagb@cs.uit.no>
 * Created at:    Mon Mar  8 14:06:12 1999
 * Modified at:   Sat Dec 25 16:06:42 1999
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

#ifndef __irda_unx_h
#define __irda_unx_h

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

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
