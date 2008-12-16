/**
 * Simple GSM frame decoder
 * wumpus 2003 -- www.blacksphere.tk
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <gammu.h>

#include "wmx-util.h"
#include "wmx-gsm.h"

/* Administrative */
GSMDecoder *GSMDecoder_new(void)
{
	GSMDecoder *self;

	self 		= malloc(sizeof(GSMDecoder));
	self->xmlout 	= NULL;

	return self;
}

/**
 * Set 'sniffer' XML output stream
 */
void GSMDecoder_xmlout(GSMDecoder *self, FILE *f)
{
	self->xmlout = f;
	if(self->xmlout) {
		/* Open XML doc */
		fprintf(self->xmlout,"<?xml version=\"1.0\"?>\n");
		fprintf(self->xmlout,"<dump>\n");
	}
}

void GSMDecoder_free(GSMDecoder *self)
{
	if(self->xmlout) {
		fprintf(self->xmlout,"</dump>\n");
		fclose(self->xmlout);
	}
	free(self);
}

const char *GSM_protocols[]={
/*0*/ "Group call control",
/*1*/ "Broadcast call control",
/*2*/ "PDSS1",
/*3*/ "Call Control; call related SS messages (TS 24.008)",
/*4*/ "PDSS2",
/*5*/ "Mobility Management messages (TS 24.008)",
/*6*/ "Radio Resources management messages (GSM 04.18)",
/*7*/ "RFU",
/*8*/ "GPRS mobility management messages",
/*9*/ "SMS messages (GSM 04.11)",
/*A*/ "GPRS session management messages",
/*B*/ "Non call related SS messages (GSM 04.80)",
/*C*/ "Location services",
/*D*/ "RFU",
/*E*/ "Reserved for extension of the PD to one octet length",
/*F*/ "Reserved for tests procedures described in GSM 11.10"
};

typedef struct GSMpackettypestruct {
	int 	id;
	const char 	*section;
	const char 	*description;
} GSMpackettype;

/* Include the data */
GSMpackettype GSM_RR_packettypes[] = {
#include "type-rr.h"
};
GSMpackettype GSM_MM_packettypes[] = {
#include "type-mm.h"
};
GSMpackettype GSM_CC_packettypes[] = {
#include "type-cc.h"
};
GSMpackettype GSM_SS_packettypes[] = {
#include "type-ss.h"
};
GSMpackettype GSM_SMS_packettypes[] = {
#include "type-sms.h"
};

GSMpackettype *GSMpackettype_lookup(GSMpackettype list[], int id)
{
	GSMpackettype *ptr = list;

	while(ptr->id != -1 && ptr->id != id) ptr++;
	if(ptr->id == -1) {
		return NULL;
	} else {
		return ptr;
	}
}

/* disassemble L3 packet -- GSM 04.06
tx=0   Network to MS
tx=1   MS to Network
*/
void GSMDecoder_L3packet(GSMDecoder *self, GSMDecoder_l1l2data *l1, unsigned char *buffer, size_t length)
{
	int 		proto,type;
	GSMpackettype 	*ptype;

	dumpraw("Inform", buffer, length);
	printf(" ");
	if(length < 2) return;

	/* Attempt at decoding first byte
	   -- protocol discriminator
	 */
	proto = buffer[0]&0xF;
	printf("\n  %s ", GSM_protocols[proto]);

	switch(proto) {
	case 0x3: /* CC */
		type  = buffer[1]&0x3F;
		ptype = GSMpackettype_lookup(GSM_CC_packettypes, type);
		if (ptype == NULL) {
			printf("\n  Unknown %02X", type);
		} else {
			printf("\n  %s (%s)", ptype->description, ptype->section);
		}
		break;
	case 0x5: /* MM */
		type  = buffer[1]&0x3F;
		ptype = GSMpackettype_lookup(GSM_MM_packettypes, type);
		if (ptype == NULL) {
			printf("\n  Unknown %02X", type);
		} else {
			printf("\n  %s (%s)", ptype->description, ptype->section);
		}
		break;
	case 0x6: /* RR */
		if (buffer[0]>>4) {
			printf("\n  RR:Invalid Skip Indicator");
			return;
		}
		type  = buffer[1];
		ptype = GSMpackettype_lookup(GSM_RR_packettypes, type);
		if (ptype == NULL) {
			printf("\n  Unknown %02X", type);
		} else {
			printf("\n  %s (%s)", ptype->description, ptype->section);
		}
		break;
	case 0x9: /* SMS */
		type  = buffer[1]&0x3F;
		ptype = GSMpackettype_lookup(GSM_SMS_packettypes, type);
		if (ptype == NULL) {
			printf("\n  Unknown %02X", type);
		} else {
			printf("\n  %s (%s)", ptype->description, ptype->section);
		}
		break;
	case 0xB: /* SS */
		if (buffer[1]&0x80) {
			printf("\n  SS:Invalid Message Type");
			return;
		}
		type  = buffer[1]&0x3F;
		ptype = GSMpackettype_lookup(GSM_SS_packettypes, type);
		if (ptype == NULL) {
			printf("\n  Unknown %02X", type);
		} else {
			printf("\n  %s (%s)", ptype->description, ptype->section);
		}
		break;
	}
}

static void GSMDecoder_l1xml_open(GSMDecoder *self, GSMDecoder_l1l2data *l1, unsigned char *buffer, size_t length)
{
	unsigned int x;

	if(self->xmlout) {
		fprintf(self->xmlout, "<l1 ");
		fprintf(self->xmlout, "direction=\"%s\" ", l1->tx==GSMDECODER_SEND?"up":"down");
		fprintf(self->xmlout, "logicalchannel=\"%i\" ", l1->ch);
		if(l1->tx == GSMDECODER_RECEIVE) {
			fprintf(self->xmlout, "physicalchannel=\"%i\" ", l1->arfcn);
			fprintf(self->xmlout, "sequence=\"%u\" ", l1->seq);
			fprintf(self->xmlout, "error=\"%i\" ", l1->err);
			fprintf(self->xmlout, "timeshift=\"%i\" ", l1->timeshift);
			fprintf(self->xmlout, "bsic=\"%i\" ", l1->bsic);
			fprintf(self->xmlout, "data=\"");
			for(x=0; x<length; x++) {
				fprintf(self->xmlout, "%02X", buffer[x]);
			}
			fprintf(self->xmlout, "\" ");
		}
		fprintf(self->xmlout, ">\n");
	}
}

static void GSMDecoder_l1xml_close(GSMDecoder *self)
{
	if(self->xmlout) fprintf(self->xmlout, "</l1>\n");
}

/* disassemble L2 pseudo length header packet -- GSM 04.06
   tx=0   Network to MS
   tx=1   MS to Network
   write XML stream
*/
void GSMDecoder_L2short_packet(GSMDecoder *self, GSMDecoder_l1l2data *l1, unsigned char *buffer, size_t length)
{
	size_t ptr,usedlength, x;

	GSMDecoder_l1xml_open(self, l1, buffer, length);

	if(length<1) {
		/* too small to contain 1 header byte */
		printf("L2 packet too small\n");
		GSMDecoder_l1xml_close(self);
		return;
	}
	ptr = 0;
	//printf("L2 packet\n");

	/* dump Length Indicator field */
	usedlength = buffer[ptr]>>2;
	if((buffer[ptr]&3) != 1) {
		dumpraw("ErrPacket", buffer, length);
		printf(" Invalid pseudo length byte\n");
		GSMDecoder_l1xml_close(self);
		return;
	}
	printf("Length : used=" SIZE_T_FORMAT " ID=%i\n",
		usedlength,
		(buffer[ptr]>>0)&3);
	ptr++;
	/* dump Information */
	printf("Inform : ");
	if((ptr+usedlength) > length) {
		printf("Invalid used length\n");
	}
	if(self->xmlout) {
		fprintf(self->xmlout, "<l2 ");
		fprintf(self->xmlout, "data=\"");
		for(x=0; x<usedlength; x++) fprintf(self->xmlout, "%02X", buffer[ptr+x]);
		fprintf(self->xmlout, "\" ");
		/* Rest octets */
		fprintf(self->xmlout, "rest=\"");
		for(x=(ptr+usedlength); x<length; x++) fprintf(self->xmlout, "%02X", buffer[x]);
		fprintf(self->xmlout, "\" ");
		fprintf(self->xmlout, ">\n");
	}
	GSMDecoder_L3packet(self, l1, &buffer[ptr], usedlength);
	if(self->xmlout) fprintf(self->xmlout, "</l2>\n");
	//dumpraw("Inform", &buffer[ptr], usedlength);
	//printf("\n");
	GSMDecoder_l1xml_close(self);
}

#define TYPE_I 0x01 /* Information */
#define TYPE_S 0x02 /* Supervisory */
#define TYPE_U 0x03 /* Unnumbered */

/* disassemble L2 packet -- GSM 04.06
tx=0   Network to MS
tx=1   MS to Network
*/
void GSMDecoder_L2packet(GSMDecoder *self, GSMDecoder_l1l2data *l1, unsigned char *buffer, size_t length)
{
	size_t 	ptr,usedlength,x;
	int 	more;
	int 	type;
	char 	temp[80];

	GSMDecoder_l1xml_open(self, l1, buffer, length);

	if(length<5) {
		/* too small to contain 3 header bytes plus 20 data bytes */
		printf("L2 packet too small\n");
		GSMDecoder_l1xml_close(self);
		return;
	}
	ptr = 0;
	//printf("L2 packet\n");
	/* dump Address field */
	printf("Address: LPD=%i SAPI=%i C/R=%i EA=%i\n",
		(buffer[ptr]>>5)&3,
		(buffer[ptr]>>2)&7,
		(buffer[ptr]>>1)&1,
		(buffer[ptr]>>0)&1);
	/* skip next address field bytes */
	while((buffer[ptr]&1)==0) ptr++;
	ptr++;
	/* dump Control field */

	type = TYPE_I;
	// if(!(buffer[ptr]&1)) type = TYPE_I;
	if((buffer[ptr]&1) && !(buffer[ptr]&2)) type = TYPE_S;
	if((buffer[ptr]&1) && (buffer[ptr]&2)) type = TYPE_U;
	switch(type) {
	case TYPE_I:
		printf("Control: I TXSeq=%i P=%i RXSeq=%i\n",
			(buffer[ptr]>>5)&7,
			(buffer[ptr]>>4)&1,
			(buffer[ptr]>>1)&7);
		break;
	case TYPE_S: {
		const char *desc = "Unknown";
		x = (buffer[ptr]>>2)&3;
		switch(x) {
		case 0: /* 00 */ desc="RR (Receive Ready)"; break; /* ALSO acknowledgement */
		case 1: /* 01 */ desc="RNR (Receive Not Ready)"; break;
		case 2: /* 10 */ desc="REJ (Reject)"; break;
		}
		printf("Control: S TXSeq=%i P/F=%i S=" SIZE_T_FORMAT " %s\n",
			(buffer[ptr]>>5)&7,
			(buffer[ptr]>>4)&1,
			x, desc);
		} break;
	case TYPE_U: {
		const char *desc = "Unknown";
		x = (((buffer[ptr]>>5)&7)<<2) | ((buffer[ptr]>>2)&3);
		binstr(temp, x, 5);
		switch(x) {
		case 0: /* 00000 */ desc = "UI (Unnumbered information)"; break;
		case 3: /* 00011 */ desc = "DM (Disconnect mode)"; break;
		case 7: /* 00111 */ desc = "SABM (Set asynchronous balanced mode)"; break;
		case 8: /* 01000 */ desc = "DISC (Disconnect)"; break;
		case 12:/* 01100 */ desc = "UA (Unnumbered acknowledge)"; break;
		}

		printf("Control: U U=%s %s P/F=%i\n",
			temp, desc,
			(buffer[ptr]>>4)&1);
		} break;
	}
	ptr++;
	/* dump Length Indicator field */
	usedlength = buffer[ptr]>>2;
	more = (buffer[ptr]>>1)&1; /* more to go */
	printf("Length : used=" SIZE_T_FORMAT " M=%i EL=%i\n",
		usedlength, more,
		(buffer[ptr]>>0)&1);
	while((buffer[ptr]&1)==0) ptr++;
	ptr++;
	/* dump Information */
	printf("Inform : ");
	if((ptr+usedlength) > length) {
		printf("Invalid used length\n");
	}
	/*
	for(x=0; x<usedlength; x++) {
		printf("%02x ",buffer[ptr]);
		ptr++;
	}
	*/
	if(self->xmlout) {
		const char *desc;
		fprintf(self->xmlout, "<l2 ");
		switch(type) {
		case TYPE_I:
			fprintf(self->xmlout, "type=\"I\" ");
			fprintf(self->xmlout, "txseq=\"%i\" p=\"%i\" rxseq=\"%i\" ",
				(buffer[ptr]>>5)&7,
				(buffer[ptr]>>4)&1,
				(buffer[ptr]>>1)&7);
			break;
		case TYPE_S:
			fprintf(self->xmlout, "type=\"S\" ");
			desc = "Unknown";
			x = (buffer[ptr]>>2)&3;
			switch(x) {
			case 0: /* 00 */ desc="RR"; break; /* ALSO acknowledgement */
			case 1: /* 01 */ desc="RNR"; break;
			case 2: /* 10 */ desc="REJ"; break;
			}
			fprintf(self->xmlout, "subtype=\"%s\" rxseq=\"%i\" p=\"%i\" ",
				desc,
				(buffer[ptr]>>5)&7,
				(buffer[ptr]>>4)&1);
			break;
		case TYPE_U:
			fprintf(self->xmlout, "type=\"U\" ");
			desc = "Unknown";
			x = (((buffer[ptr]>>5)&7)<<2) | ((buffer[ptr]>>2)&3);
			switch(x) {
			case 0: /* 00000 */ desc = "UI"; break;
			case 3: /* 00011 */ desc = "DM"; break;
			case 7: /* 00111 */ desc = "SABM"; break;
			case 8: /* 01000 */ desc = "DISC"; break;
			case 12:/* 01100 */ desc = "UA"; break;
			}

			fprintf(self->xmlout, "subtype=\"%s\" p=\"%i\" ",
				desc,
				(buffer[ptr]>>4)&1);
			break;
		default:
			fprintf(self->xmlout, "type=\"Unknown\" ");
		}
		fprintf(self->xmlout, "data=\"");
		for(x=0; x<usedlength; x++) fprintf(self->xmlout, "%02X", buffer[ptr+x]);
		fprintf(self->xmlout, "\" ");
		fprintf(self->xmlout, ">\n");
	}
	GSMDecoder_L3packet(self, l1, &buffer[ptr], usedlength);
	if(self->xmlout) fprintf(self->xmlout, "</l2>\n");
	GSMDecoder_l1xml_close(self);
	//dumpraw("Inform", );
	//printf("\n");
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
