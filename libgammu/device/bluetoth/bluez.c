/* Based on some work from Bluez (www.bluez.org)
 * (C) 2000-2001 Qualcomm Incorporated
 * (C) 2002-2003 Maxim Krasnyansky <maxk@qualcomm.com>
 * (C) 2002-2004 Marcel Holtmann <marcel@holtmann.org>
 * GNU GPL version 2
 */
/* based on some Marcel Holtmann work from Gnokii (www.gnokii.org)
 * (C) 1999-2000 Hugh Blemings & Pavel Janik ml. (C) 2001-2004 Pawel Kot
 * GNU GPL version 2 or later
 */
/* Due to a problem in the source code management, the names of some of
 * the authors have unfortunately been lost. We do not mean to belittle
 * their efforts and hope they will contact us to see their names
 * properly added to the Copyright notice above.
 * Having published their contributions under the terms of the GNU
 * General Public License (GPL) [version 2], the Copyright of these
 * authors will remain respected by adhering to the license they chose
 * to publish their code under.
 */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_BLUETOOTHDEVICE
#ifdef BLUEZ_FOUND

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/hci_lib.h>

#include "../../gsmcomon.h"
#include "../devfunc.h"
#include "bluetoth.h"

GSM_Error bluetooth_connect(GSM_StateMachine *s, int port, char *device)
{
	GSM_Device_BlueToothData 	*d = &s->Device.Data.BlueTooth;
	struct sockaddr_rc 		laddr, raddr;
	bdaddr_t			bdaddr;
	int 				fd;

	memset(&laddr, 0, sizeof(laddr));
	memset(&raddr, 0, sizeof(raddr));

	smprintf(s, "Connecting to RF channel %i\n",port);

	fd = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if (fd < 0) {
		smprintf(s, "Can't create socket\n");
		return ERR_DEVICENODRIVER;
	}

	bacpy(&laddr.rc_bdaddr, BDADDR_ANY);
	laddr.rc_family 	= AF_BLUETOOTH;
	laddr.rc_channel 	= 0;

	if (bind(fd, (struct sockaddr *)&laddr, sizeof(laddr)) < 0) {
		smprintf(s, "Can't bind socket (%d, %s)\n", errno, strerror(errno));
		close(fd);
		return ERR_DEVICEOPENERROR;
	}

	str2ba(device, &bdaddr);
	bacpy(&raddr.rc_bdaddr, &bdaddr);
	raddr.rc_family 	= AF_BLUETOOTH;
	raddr.rc_channel 	= port;

	if (connect(fd, (struct sockaddr *)&raddr, sizeof(raddr)) < 0) {
		smprintf(s, "Can't connect (%d, %s)\n", errno, strerror(errno));
		close(fd);
		return ERR_DEVICEOPENERROR;
	}

	d->hPhone = fd;
    	return ERR_NONE;
}

#ifdef BLUETOOTH_RF_SEARCHING

static GSM_Error bluetooth_checkdevice(GSM_StateMachine *s, bdaddr_t *bdaddr, uuid_t *group)
{
	sdp_session_t			*sess = 0;
	sdp_record_t 			*rec;
	sdp_list_t			*attrid, *search, *seq, *next, *proto;
	sdp_data_t			*d;
	bdaddr_t			interface;
	uint32_t			range = 0x0000ffff;
	char				str[20];
	int				channel,dd;
	char 				name[1000];
	int				score, bestscore = 0;
	int				found = -1;
	int retries = 0;
	uuid_t subgroup;

	if (s->ConnectionType == GCT_BLUEPHONET) {
		smprintf(s, "Looking for suitable channel for PHONET\n");
	} else if (s->ConnectionType == GCT_BLUEOBEX) {
		smprintf(s, "Looking for suitable channel for OBEX\n");
	} else if (s->ConnectionType == GCT_BLUEAT) {
		smprintf(s, "Looking for suitable channel for AT\n");
	}

	bacpy(&interface,BDADDR_ANY);

	ba2str(bdaddr, str);
	smprintf(s,"Device %s", str);

	/* Try to read name */
	dd = hci_open_dev(0);
	if (dd < 0) return ERR_UNKNOWN;
	memset(name, 0, sizeof(name));
	if (hci_read_remote_name(dd, bdaddr, sizeof(name), name, 100000) >= 0) {
	    smprintf(s, " (\"%s\")", name);
	}
	close(dd);
	smprintf(s,"\n");

	/*
	 * Need to sleep for some slow devices, otherwise we get
	 * "Operation already in progress" error.
	 */
	while (!sess) {
		/* Try to connect to device */
		sess = sdp_connect(&interface, bdaddr, SDP_RETRY_IF_BUSY);
		if (sess) break;

		if (errno == EALREADY && retries < 5) {
			smprintf(s, "Operation already in progress, retrying.\n");
			sleep(1);
			continue;
		}
		/* Failure */
		break;
	}

	/* Did we connect? */
	if (!sess) {
		smprintf(s, "Failed to connect to SDP server on %s: %s\n", str, strerror(errno));
		return ERR_TIMEOUT;
	}

	/* List available channels */
	attrid = sdp_list_append(0, &range);
	search = sdp_list_append(0, group);
	if (sdp_service_search_attr_req(sess, search, SDP_ATTR_REQ_RANGE, attrid, &seq)) {
		smprintf(s, "Service Search failed: %s\n", strerror(errno));
		sdp_close(sess);
		return ERR_UNKNOWN;
	}
	sdp_list_free(attrid, 0);
	sdp_list_free(search, 0);

	for (; seq; seq = next) {
		rec 	= (sdp_record_t *) seq->data;

		/* Get channel info */
		if (sdp_get_access_protos(rec, &proto) == 0) {
			channel = sdp_get_proto_port(proto, RFCOMM_UUID);
			sdp_list_foreach(proto, (sdp_list_func_t)sdp_list_free, 0);
			sdp_list_free(proto, 0);
		} else {
			goto next_seq;
		}
		smprintf(s, "   Channel %i", channel);

		/* Get service name and check it */
		d = sdp_data_get(rec, SDP_ATTR_SVCNAME_PRIMARY);
		if (d) {
			score = bluetooth_checkservicename(s, d->val.str);
			smprintf(s," - \"%s\" (score=%d)\n", d->val.str, score);
			if (score > bestscore) {
				found = channel;
				bestscore = score;
			}
		} else {
			smprintf(s,"\n");
		}

		/* Descent to subroups */
		memset(&subgroup, 0, sizeof(subgroup));
    		if (sdp_get_group_id(rec, &subgroup) != -1) {
			if (subgroup.value.uuid16 != group->value.uuid16) {
				bluetooth_checkdevice(s, bdaddr, &subgroup);
			}
		}

next_seq:
		next = seq->next;
		free(seq);
		sdp_record_free(rec);
	}
	sdp_close(sess);

	if (found != -1) {
		return bluetooth_connect(s, found, str);
	}

	smprintf(s, "No suitable bluetooth channel found!\n");
	return ERR_NOTSUPPORTED;
}

GSM_Error bluetooth_findchannel(GSM_StateMachine *s)
{
	inquiry_info			ii[20];
	uint8_t				count = 0;
	int				i;
	GSM_Error			error = ERR_TIMEOUT;
	struct hci_dev_info		hci_info;
	uuid_t group;

	memset(&group, 0, sizeof(group));
	/* We're looking only for rfcomm channels */
	sdp_uuid16_create(&group, RFCOMM_UUID);

	if (hci_devinfo(0, &hci_info) < 0) return ERR_DEVICENOTWORK;

	if (strcmp(s->CurrentConfig->Device, "/dev/ttyS1") == 0) {
		smprintf(s, "Searching for devices\n");
		if (sdp_general_inquiry(ii, 20, 8, &count) < 0) {
			return ERR_UNKNOWN;
		}
	} else {
		count = 1;
		str2ba(s->CurrentConfig->Device,&ii[0].bdaddr);
	}
	for (i=0;i<count;i++) {
		error = bluetooth_checkdevice(s,&ii[i].bdaddr,&group);
		if (error == ERR_NONE) return error;
	}
	return error;
}

#endif
#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
