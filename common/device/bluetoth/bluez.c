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

#include "../../gsmstate.h"

#ifdef GSM_ENABLE_BLUETOOTHDEVICE
#ifdef GSM_ENABLE_BLUEZ

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

#include "../../gsmcomon.h"
#include "../devfunc.h"
#include "bluetoth.h"

GSM_Error bluetooth_connect(GSM_StateMachine *s, int port, char *device)
{
	GSM_Device_BlueToothData 	*d = &s->Device.Data.BlueTooth;
	struct sockaddr_rc 		laddr, raddr;
	bdaddr_t			bdaddr;
	int 				fd;

	smprintf(s, "Connecting to RF channel %i\n",port);

	fd = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if (fd < 0) {
		dbgprintf("Can't create socket\n");
		return ERR_DEVICENODRIVER;
	}

	bacpy(&laddr.rc_bdaddr, BDADDR_ANY);
	laddr.rc_family 	= AF_BLUETOOTH;
	laddr.rc_channel 	= 0;

	if (bind(fd, (struct sockaddr *)&laddr, sizeof(laddr)) < 0) {
		dbgprintf("Can't bind socket\n");
		close(fd);
		return ERR_DEVICEOPENERROR;
	}

	str2ba(device, &bdaddr);
	bacpy(&raddr.rc_bdaddr, &bdaddr);
	raddr.rc_family 	= AF_BLUETOOTH;
	raddr.rc_channel 	= port;

	if (connect(fd, (struct sockaddr *)&raddr, sizeof(raddr)) < 0) {
		dbgprintf("Can't connect\n");
		close(fd);
		return ERR_DEVICEOPENERROR;
	}

	d->hPhone = fd;
    	return ERR_NONE;
}

#ifdef BLUETOOTH_RF_SEARCHING

struct search_context {
	char				*svc;
	uuid_t				group;
	int				tree;
	uint32_t			handle;
};

static void print_service_desc(void *value, void *user) 
{ 
	sdp_data_t 	*p = (sdp_data_t *)value; 
	int 		i = 0, proto = 0, *channel = (int *)user; 

	for (; p; p = p->next, i++) { 
		switch (p->dtd) { 
		case SDP_UUID16:
		case SDP_UUID32:
		case SDP_UUID128:
			proto = sdp_uuid_to_proto(&p->val.uuid);
			break;
		case SDP_UINT8: 
			if (proto == RFCOMM_UUID) {
				(*channel) = p->val.uint8;
				return;
			}
			break; 
		} 
	} 
}

void print_access_protos(value, user)
{
	sdp_list_t 	*protDescSeq = (sdp_list_t *)value;
	int		*channel = (int *)user;
	
	sdp_list_foreach(protDescSeq,print_service_desc,channel);
}

static GSM_Error bluetooth_checkdevice(GSM_StateMachine *s, bdaddr_t *bdaddr, struct search_context *context)
{
	sdp_session_t			*sess;
	sdp_record_t 			*rec;
	sdp_list_t			*attrid, *search, *seq, *next, *proto = 0;
	sdp_data_t			*d;
	bdaddr_t			interface;
	uint32_t			range = 0x0000ffff;
	struct search_context 		subcontext;
	char				str[20];
	int				channel,channel2;

	bacpy(&interface,BDADDR_ANY);

	ba2str(bdaddr, str);
	smprintf(s,"%s\n", str);

	sess = sdp_connect(&interface, bdaddr, SDP_RETRY_IF_BUSY);
	if (!sess) {
		dbgprintf("Failed to connect to SDP server on %s: %s\n", str, strerror(errno));
		return ERR_UNKNOWN;
	}

	attrid = sdp_list_append(0, &range);
	search = sdp_list_append(0, &context->group);
	if (sdp_service_search_attr_req(sess, search, SDP_ATTR_REQ_RANGE, attrid, &seq)) {
		dbgprintf("Service Search failed: %s\n", strerror(errno));
		sdp_close(sess);
		return ERR_UNKNOWN;
	}
	sdp_list_free(attrid, 0);
	sdp_list_free(search, 0);

	channel2 = -1;
	for (; seq; seq = next) {
		rec 	= (sdp_record_t *) seq->data;
		
		if (channel2 == -1) {		
			if (!context->tree) {
				d = sdp_data_get(rec,SDP_ATTR_SVCNAME_PRIMARY);
			
				if (sdp_get_access_protos(rec,&proto) == 0) {
					channel = -1;
					sdp_list_foreach(proto,print_access_protos,&channel);
					sdp_list_free(proto,(sdp_free_func_t)sdp_data_free);
				}
				smprintf(s,"Channel %i",channel);
				if (d) smprintf(s," - \"%s\"",d->val.str);
				smprintf(s,"\n");
				if (channel2 == -1 && bluetooth_checkservicename(s, d->val.str) == ERR_NONE) {
					channel2 = channel;			
				}			
			}
    			if (sdp_get_group_id(rec,&subcontext.group) != -1) {
				memcpy(&subcontext, context, sizeof(struct search_context));
				if (subcontext.group.value.uuid16 != context->group.value.uuid16) bluetooth_checkdevice(s,bdaddr,&subcontext);
			}
		}

		next = seq->next;
		free(seq);
		sdp_record_free(rec);
	}
	sdp_close(sess);
	
	if (channel2 != -1) return bluetooth_connect(s, channel2, str);
	
	return ERR_NOTSUPPORTED;
}

GSM_Error bluetooth_findchannel(GSM_StateMachine *s)
{
	inquiry_info			ii[20];
	uint8_t				count = 0;
	int				i;
	struct search_context 		context;
	GSM_Error			error = ERR_NOTSUPPORTED;

	memset(&context, '\0', sizeof(struct search_context));
	sdp_uuid16_create(&(context.group),PUBLIC_BROWSE_GROUP);

	if (!strcmp(s->CurrentConfig->Device,"/dev/ttyS1")) {
		dbgprintf("Searching for devices\n");
		if (sdp_general_inquiry(ii, 20, 8, &count) < 0) {
			return ERR_UNKNOWN;
		}
	} else {
		count = 1;
		str2ba(s->CurrentConfig->Device,&ii[0].bdaddr);
	}
	for (i=0;i<count;i++) {
		error = bluetooth_checkdevice(s,&ii[i].bdaddr,&context);
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
