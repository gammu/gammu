/*-
 * Copyright (c) 2009 Iain Hibbert
 * All rights reserved.
 *
 * Licensend under GNU GPL 2 or later.
 */

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_BLUETOOTHDEVICE
#ifdef BSD_BLUE_FOUND

#include <sys/socket.h>
#include <sys/time.h>

#include <bluetooth.h>
#include <errno.h>
#include <netdb.h>
#include <sdp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../../gsmcomon.h"
#include "../devfunc.h"
#include "bluetoth.h"

/*
 * Handle FreeBSD compatibility
 */
#ifndef BTPROTO_RFCOMM
#define BTPROTO_RFCOMM		BLUETOOTH_PROTO_RFCOMM
#define BDADDR_ANY		NG_HCI_BDADDR_ANY
#define sockaddr_bt		sockaddr_rfcomm
#define bt_len			rfcomm_len
#define bt_family		rfcomm_family
#define bt_channel		rfcomm_channel
#define bt_bdaddr		rfcomm_bdaddr
#define bdaddr_copy(d, s)	memcpy((d), (s), sizeof(bdaddr_t))
#endif

static GSM_Error bluetooth_open(GSM_StateMachine *s, bdaddr_t *bdaddr, int channel)
{
	GSM_Device_BlueToothData	*d = &s->Device.Data.BlueTooth;
	struct sockaddr_bt 		sa;
	int 				fd;

	memset(&sa, 0, sizeof(sa));
	sa.bt_len = sizeof(sa);
	sa.bt_family = AF_BLUETOOTH;

	smprintf(s, "Connecting to RF channel %i\n", channel);

	fd = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if (fd < 0) {
		smprintf(s, "Can't create socket\n");
		return ERR_DEVICENODRIVER;
	}

	bdaddr_copy(&sa.bt_bdaddr, BDADDR_ANY);

	if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
		smprintf(s, "Can't bind socket: %s\n", strerror(errno));
		close(fd);
		return ERR_DEVICEOPENERROR;
	}

	sa.bt_channel = channel;
	bdaddr_copy(&sa.bt_bdaddr, bdaddr);

	if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
		smprintf(s, "Can't connect to %s: %s\n", bt_ntoa(bdaddr, NULL), strerror(errno));
		close(fd);
		return ERR_DEVICEOPENERROR;
	}

	d->hPhone = fd;
    	return ERR_NONE;
}

GSM_Error bluetooth_connect(GSM_StateMachine *s, int port, char *device)
{
	bdaddr_t	bdaddr;
	struct hostent *he = NULL;

	if (!bt_aton(device, &bdaddr)) {
		if ((he = bt_gethostbyname(device)) == NULL) {
			smprintf(s, "%s: %s\n", device, hstrerror(h_errno));
			return ERR_UNKNOWN;
		}

		bdaddr_copy(&bdaddr, (bdaddr_t *)he->h_addr);
	}

	return bluetooth_open(s, &bdaddr, port);
}

#ifdef BLUETOOTH_RF_SEARCHING

static int bluetooth_channel(sdp_data_t *value)
{
	sdp_data_t	pdl, seq;
	uintmax_t	channel;

	sdp_get_alt(value, value);	/* strip any alt container */

	while (sdp_get_seq(value, &pdl)) {
		if (sdp_get_seq(&pdl, &seq)
		    && sdp_match_uuid16(&seq, SDP_UUID_PROTOCOL_L2CAP)
		    && sdp_get_seq(&pdl, &seq)
		    && sdp_match_uuid16(&seq, SDP_UUID_PROTOCOL_RFCOMM)
		    && sdp_get_uint(&seq, &channel)
		    && channel >= 1 && channel <= 30)
			return channel;
	}

	return -1;
}

static char *bluetooth_service(sdp_data_t *value)
{
	char		*str;
	size_t		len;

	if (!sdp_get_str(value, &str, &len))
		return NULL;

	return strndup(str, len);
}

static GSM_Error bluetooth_search(GSM_StateMachine *s, bdaddr_t *bdaddr)
{
	sdp_data_t	rec, rsp, ssp, value;
	uint8_t		buf[3];
	uint16_t	attr;
	sdp_session_t	ss;
	int		ch, channel, sc, score;
	char		*sv;

	smprintf(s, "Searching for services on %s\n", bt_ntoa(bdaddr, NULL));

	ss = sdp_open(NULL, bdaddr);
	if (ss == NULL) {
		smprintf(s, "SDP Connection failed: %s\n", strerror(errno));
		return ERR_TIMEOUT;
	}

	ssp.next = buf;
	ssp.end = buf + sizeof(buf);
	sdp_put_uuid16(&ssp, SDP_UUID_PROTOCOL_RFCOMM);
	ssp.end = ssp.next;
	ssp.next = buf;

	if (!sdp_service_search_attribute(ss, &ssp, NULL, &rsp)) {
		smprintf(s, "SDP Service Search Attribute failed: %s\n", strerror(errno));
		sdp_close(ss);
		return ERR_UNKNOWN;
	}

	channel = -1;
	score = 0;

	while (sdp_get_seq(&rsp, &rec)) {
		ch = -1;
		sv = NULL;

		while (sdp_get_attr(&rec, &attr, &value)) {
			switch (attr) {
			case SDP_ATTR_PROTOCOL_DESCRIPTOR_LIST:
				ch = bluetooth_channel(&value);
				break;

			case SDP_ATTR_PRIMARY_LANGUAGE_BASE_ID + SDP_ATTR_SERVICE_NAME_OFFSET:
				sv = bluetooth_service(&value);
				break;

			default:
				break;
			}
		}

		if (ch != -1) {
			smprintf(s, "   Channel %i", ch);
			if (sv != NULL) {
				sc = bluetooth_checkservicename(s, sv);
				smprintf(s, " - \"%s\" (score=%d)", sv, sc);
				if (sc > score) {
					score = sc;
					channel = ch;
				}
			}
			smprintf(s, "\n");
		}

		free(sv);
	}

	sdp_close(ss);

	if (channel == -1) {
		smprintf(s, "No suitable service found!\n");
		return ERR_NOTSUPPORTED;
	}

	return bluetooth_open(s, bdaddr, channel);
}

GSM_Error bluetooth_findchannel(GSM_StateMachine *s)
{
	char			*device = s->CurrentConfig->Device;
	bdaddr_t		bdaddr;
	struct hostent		*he;
	struct bt_devinquiry	*ii;
	int			count, n;

	if (bt_aton(device, &bdaddr))
		return bluetooth_search(s, &bdaddr);

	if ((he = bt_gethostbyname(device)) != NULL)
		return bluetooth_search(s, (bdaddr_t *)he->h_addr);

	smprintf(s, "Device \"%s\" unknown. Starting inquiry..\n", device);

	if ((count = bt_devinquiry(NULL, 10, 20, &ii)) == -1) {
		smprintf(s, "Inquiry failed: %s\n", strerror(errno));
		return ERR_UNKNOWN;
	}

	smprintf(s, "Found %d device%s.\n", count, (count == 1 ? "" : "s"));

	for (n = 0; n < count; n++) {
		if (bluetooth_search(s, &ii[n].bdaddr) == ERR_NONE) {
			free(ii);
			return ERR_NONE;
		}
	}

	free(ii);
	return ERR_UNKNOWN;
}

#endif
#endif
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
