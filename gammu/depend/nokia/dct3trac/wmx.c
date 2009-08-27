/**
 * Nokia DCT3 Firmware Debug Trace Monitor
 * wumpus 2003 -- www.blacksphere.tk
 * SIM stuff by The Monty
 *
 * Command line arguments:
 * gammu --nokiadebug v00-0F,20,21
 * (v=verbose)
 */

#include "../../../../libgammu/gsmstate.h"

#ifdef GSM_ENABLE_NOKIA_DCT3

#include <string.h>
#include <signal.h>

#include "../../../../libgammu/misc/coding/coding.h"
#include "../../../../libgammu/gsmcomon.h"
#include "../../../../libgammu/gsmstate.h"
#include "../../../../libgammu/service/gsmpbk.h"
#include "../../../../libgammu/phone/nokia/dct3/dct3func.h"
#include "../../../gammu.h"
#include "../dct3.h"
#include "wmx.h"
#include "wmx-util.h"
#include "wmx-gsm.h"
#include "wmx-sim.h"
#include "wmx-list.h"

extern GSM_Reply_Function UserReplyFunctionsX[];

/* Global variables suck */
GSMDecoder 		*gsmdec;
struct wmx_tracestruct 	*traces;

static GSM_Error DCT3_ReplySwitchDebug(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	switch(msg.Buffer[2]) {
	case 0x70:
		printf("Debug Trace Enabled\n");
		break;
	case 0x71:
		printf("Debug Trace Disabled\n");
		break;
	}
	return ERR_NONE;
}

/**
 * RPC confirmation/reply
 */
static GSM_Error DCT3_ReplyRPC(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	printf("RPC Reply ");
	printf("call=%02x rettype=%02x data=", msg.Buffer[2], msg.Buffer[3]);
	if(msg.Buffer[3] == 3) {
		/* string */
		printf("%s", &msg.Buffer[4]);
	} else {
		dumpraw("RPC Reply data", &msg.Buffer[4], msg.Length-4);
	}
	printf("\n");
	return ERR_NONE;
}

/* disassemble mdisnd (0x18xx) packet */
static void mdisnd_data(unsigned char type, unsigned char *buffer, size_t length)
{
	GSMDecoder_l1l2data 	dat;
	size_t 			x;
	int 			ch;

	if(type==0x1B && length>2) {
		/* channel packet */
		ch 	= buffer[1];
		dat.tx 	= GSMDECODER_SEND;
		dat.ch 	= ch;
		printf("%02X ch=%02X ",buffer[0],ch);
		if (ch == 0x80 || ch == 0xB0) {
			printf("\n");
			GSMDecoder_L2packet(gsmdec, &dat, &buffer[2], length-2);
		} else if (ch == 0x70) {
			dumpraw("MDI send ch70 prefix", &buffer[2], 2);
			printf("\n");
			GSMDecoder_L2packet(gsmdec, &dat, &buffer[4], length-4);
		} else {
			dumpraw("MDI recv 1B packet", &buffer[2], length-2);
		}
	} else {
		/* hex */
		for(x=0; x<length; x++) {
			printf("%02x ",buffer[x]&0xFF);
		}
	}
}

/* disassemble mdircv (0x19xx) packet */
static void mdircv_data(unsigned char type, unsigned char *buffer, size_t length)
{
	size_t 			x;
	int 			ch;
	GSMDecoder_l1l2data 	dat;

	if (type==0x80 && length>1) {
		// buffer[0]    channel
		// buffer[1]    flag1
		// buffer[2]    flag2
		// buffer[3..5] timestamp
		// buffer[6..7] unknown_hw1
		// buffer[8..9] unknown_hw2
		ch 	 	= buffer[0];
		dat.tx 	 	= GSMDECODER_RECEIVE;
		dat.ch 	 	= ch;
		dat.bsic 	= buffer[1];
		dat.err  	= buffer[2];
		dat.seq  	= (buffer[3]<<16)|(buffer[4]<<8)|(buffer[5]);
		dat.arfcn 	= (buffer[6]<<8)|buffer[7];
		dat.timeshift 	= (buffer[8]<<8)|buffer[9];

		printf("ch=%02X bsic=%i err=%i t=%06X arfcn=%i shift=%i",
			ch, buffer[1], buffer[2],
			dat.seq, dat.arfcn, dat.timeshift
		);

		//dumpraw("MDI recv 80 header", &buffer[6], 4);
		printf(" ");
		if(buffer[2] == 0) { /* unencrypted */
			if(ch == 0x70) {
				/* Normal header + 2b prefix */
				dumpraw("MDI recv ch70 prefix", &buffer[10], 2);
				printf("\n");
				GSMDecoder_L2packet(gsmdec, &dat, &buffer[12], length-12);
 			} else if (ch == 0x80 || ch == 0xB0) {
				/* Normal header */
				printf("\n");
				GSMDecoder_L2packet(gsmdec, &dat, &buffer[10], length-10);
			} else if (ch == 0x50 || ch == 0x60) {
				/* Short header */

				printf("\n");
				GSMDecoder_L2short_packet(gsmdec, &dat, &buffer[10], length-10);
			} else {
				dumpraw("MDI send 80 packet", &buffer[10], length-10);
			}
		} else {
			/* Encrypted (?) */
			dumpraw("MDI send err 80", &buffer[10], length-10);
		}
	} else {
		/* hex */
		for(x=0; x<length; x++) {
			printf("%02x ",buffer[x]&0xFF);
		}
	}
}

static GSM_Error DCT3_ReplyDebugTrace(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	size_t 			x;
	int 			id,timestamp,number;
	size_t			length;
	struct wmx_tracetype 	*wmx_minor;
	const char 			*desc;

	//printf("Debug Trace Received\n");
	/* parse frame
	   Debug trace packet:
	   packet type 0x00
	   source subsystem 0x01 (LOCAL)
	   verder formaat zie notebook
	   0x08 ID (payload=offset 0x02 here)
	   0x0A timestamp
	   0x0C seq nr
	   0x0D .. parameters
	*/
	id 		= ((msg.Buffer[2]&0xFF)<<8)|(msg.Buffer[3]&0xFF);
	timestamp 	= ((msg.Buffer[4]&0xFF)<<8)|(msg.Buffer[5]&0xFF);
	number 		= msg.Buffer[6]&0xFF;
	length 		= msg.Buffer[7]&0xFF;

	/* filter */
	//if((id&0xFF00)==0x1900 && id != 0x1980)
	//	return GE_NONE;
	//printf("%02x\n",msg.Buffer[10]);
	//if(msg.Buffer[10]!=0x40)
	//	return GE_NONE;
	/* Query trace type name */
	desc = "Unknown";
	if(traces != NULL) {
		wmx_minor = wmx_tracestruct_queryminor(traces, id);
		if(wmx_minor != NULL) desc = wmx_minor->desc;
	}
	printf("<%04X> %s\n", id, desc);
	printf("t=%04x nr=%02x: ", timestamp, number);

	/* TODO -- decode debug types on phone type */
	switch(id>>8) {
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x37:
	case 0x38:
	case 0x39:
	case 0x3A:
	case 0x3B:
	case 0x3C:
	case 0x5F:
		/* text */
		/* skip length byte */
		printf("\"");
		for(x=8; x<msg.Length; x++) {
			printf("%c",msg.Buffer[x]&0xFF);
		}
		printf("\"");
		break;
		/*
	case 0x6801:
		for(x=8; x<msg.Length; x++) {
			printf("%02x%c ",msg.Buffer[x]&0xFF,msg.Buffer[x]&0xFF);
		}
		break;
		*/
	case 0x18: /* MDISND */

		/* skip these:
		  +00 length
		  +01 type (also xx in 0x18xx)
		*/
		if(msg.Length<10 || msg.Buffer[9]!=(id&0xFF)) {
			printf("C %02X: param:%02x", id&0xFF, msg.Buffer[8]);
		} else {
			//printf("D %02X: ", id&0xFF);
			printf("D %02X: ", id&0xFF);
			mdisnd_data((unsigned char)(id&0xFF), (unsigned char*)&msg.Buffer[10], msg.Length-10);
		}
		break;
	case 0x19: /* MDIRCV */
		if(msg.Length<10 || msg.Buffer[9]!=(id&0xFF)) {
			printf("C %02X: param:%02x", id&0xFF, msg.Buffer[8]);
		} else {
			printf("D %02X: ", id&0xFF);
			mdircv_data((unsigned char)(id&0xFF), (unsigned char*)&msg.Buffer[10], msg.Length-10);
			//dumpraw((unsigned char*)&msg.Buffer[10], msg.Length-10);
		}
		break;
 	case 0x20: /* 0x25 SIM commands */
 		/*
		for(x=8;x<msg.Length;x++)
 		    printf("%02x ", msg.Buffer[x]&0xFF);
 		*/
 		printf("SIM command ");
 		if(msg.Buffer[8]==0xa0) { // check if valid (class=a0)
			simCommand_data(msg.Buffer[9], (unsigned char)(id&0xFF), (unsigned char*)&msg.Buffer[10], msg.Length-10);
 			// TODO: pass the msg.Buffer[9] and skip 1rst arg
 		} else {
			printf("Unknown 0x25 packet (NOT SIM cmd): ");
			for(x=8;x<msg.Length;x++) printf("%02x ", msg.Buffer[x]&0xFF);
			printf("\n");
 		}
 		break;
 	case 0x22: /* 0x27 SIM answer to command (error/ok/etc..) */
 		if(msg.Length < 10 || msg.Length < length + 8) {
 		    	// Unknown response
 		    	for(x=0;x<msg.Length-10;x++) printf("%02x ", msg.Buffer[x]&0xFF);
			printf(" (Unknown 0x27 packet ? ? )\n");
 		} else {
 		    	simAnswer_Process((unsigned char)(id&0xFF), (unsigned char*)&msg.Buffer[8], length);
 		}
 		break;
 	case 0x23:  /* 0x28 SIM response data to commands */
 		if(msg.Length < 10 || msg.Length < length + 8) {
 		    	// Unknown response
 		    	for(x=0;x<msg.Length-10;x++) printf("%02x ", msg.Buffer[x]&0xFF);
 		    	printf(" (Unknown 0x28 packet)\n");
 		} else {
 		    	simResponse_Process((unsigned char)(id&0xFF), (unsigned char*)&msg.Buffer[8], length);
 		}
 		break;
	default:
		/* hex */
		for(x=8; x<msg.Length; x++) {
			printf("%02x ",msg.Buffer[x]&0xFF);
		}
		break;
	}
	printf("\n");
	return ERR_NONE;
}


static GSM_Error DCT3_ReplyMyPacket(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	size_t x;

	printf("MyPacket ");
	for(x=0; x<msg.Length; x++) {
		printf("%02x ",msg.Buffer[x]&0xFF);
	}
	printf("\n");
	return ERR_NONE;
}

#define ID_DebugTrace   0x666
#define ID_DebugSwitch  0x667
#define ID_RPC		0x668

void DCT3SetDebug(int argc, char *argv[])
{
	int		x,count;
	unsigned int y;
	unsigned char 	reqDisable[] = {0x01, 0x01, 0x71};
	GSM_Error error;
//	unsigned char 	reqTest[] = {0x01, 0x01, 0x96, 0xFF, 0xFF};

	/* RPC testing packets: */

	/* RPC: Get version */
	//unsigned char 	reqTest2[] = {0x01, 0x01, 0x00, 0x03, 0x00};
	/* RPC: read I/O 0x6D mask 0xFF */
	//unsigned char 	reqTest2[] = {0x01, 0x01, 0x02, 0x01, 0x02, 0x6D, 0xFF}; /*  */
	/* RPC: write I/O 0x03 mask 0xFF value 0x31 */
	//unsigned char 	reqTest2[] = {0x01, 0x01, 0x01, 0x01, 0x07, 0x03, 0xFF, 0x31}; /* write I/O */

	/* RPC: write forged FBUS packet to MDISND */
//	unsigned char 	reqTest2[] = {0x01, 0x01, 0x16, 0x01, 0x06,
//	0x14, // R0 -- length
//	0x05, // R1 -- MDI type identifier 0x05(FBUS)
//	0x1e, 0x0c, 0x00, 0x66,
//	0x00, 0x0e, 0x01, 0x01,
//	0x66, 0x55, 0x44, 0x33,
//	0x0d, 0x01, 0x01, 0x01,
//	0x1b, 0x58, 0x01, 0x44};
//	1805 t=cb37 nr=e2 :D 05:

	/* debug enable packet */
	unsigned char reqEnable[] = {
		0x00, 0x01, 0x70,
		/* Debug bits
		   byte[bit>>3]&(1<<(7-(bit&7)))
		*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x00 */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x40 */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x80 */
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xC0 */
		/* Debug verbose bits
		   byte[bit>>3]&(1<<(7-(bit&7)))
		*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		};
	FILE *xout;

	#define ENABLE_BIT(bit,verbose) reqEnable[3 + (bit>>3)] |= 1<<(7-(bit&7)); if(verbose){reqEnable[3 + 32 + (bit>>3)] |= 1<<(7-(bit&7));}

	/* Enable some bit
	   TODO command line or GUI interface
	*/
	//ENABLE_BIT(0x18, 1);	/* Enable MDISND debugging */
	//ENABLE_BIT(0x19, 1);	/* Enable MDIRCV debugging */
	//ENABLE_BIT(0x31, 1);

	gsmdec = GSMDecoder_new();
	/* Open XML file .. needs to be argument */
	xout = fopen("out.xml", "w");
	if (xout == NULL) {
		return;
	}
	GSMDecoder_xmlout(gsmdec, xout);

	printf("Debug Trace Mode -- wumpus 2003\n");
	traces = wmx_tracestruct_load(argv[2]);
	if(traces == NULL)
		printf("Warning: could not load trace description file %s\n", argv[2]);
	printf("Activating ranges:\n");
	count = 0;
	for(x=3; x<argc; x++) {
		char 		*ptr = argv[x];
		unsigned 	from,to,verbose;

		while(*ptr) {
		 	verbose = 0;
			if(*ptr == 'v') {
				verbose = 1;
				ptr++;
			}
			to = from = strtol(ptr, &ptr, 16);
			if(*ptr == '-') {
				ptr ++;
				to = strtol(ptr, &ptr, 16);
			}
			if(*ptr != ',' && *ptr != 0) {
				printf("Invalid parameter '%s'\n", argv[x]);
				return;
			}
			if(*ptr == ',')
				ptr++;
			if(from > 0xFF) from=0xFF;
			if(to > 0xFF) to=0xFF;
			printf("  %02x-%02x verbose=%i\n",from,to,verbose);
			for(y=from; y<=to; y++) {
				ENABLE_BIT(y, verbose);
				count++;
			}
		}
	}
	if(count == 0) {
		printf("Nothing activated -- bailing out\n");
		return;
	}
	//ENABLE_BIT(0x20, 1);  /* SIM commands (literal) */
	//ENABLE_BIT(0x21, 1);  /* SIML2 commands (literal) */
	//ENABLE_BIT(0x22, 1);  /* SIM commands (literal) */
	//ENABLE_BIT(0x3B, 1);	/* PHCTRL state */

	GSM_Init(TRUE);

	/* We Need DCT3 */
	CheckDCT3();

	error=DCT3_EnableSecurity (gsm, 0x01);
	Print_Error(error);

	gsm->User.UserReplyFunctions=UserReplyFunctionsX;

	//error=GSM_WaitFor (gsm, reqTest, sizeof(reqTest), 0x40, 1, ID_DebugSwitch);

	//error=GSM_WaitFor (gsm, reqTest2, sizeof(reqTest2), 0xD1, 4, ID_RPC);

	/* Enable Debug Mode */
	error=GSM_WaitFor (gsm, reqEnable, sizeof(reqEnable), 0x40, 4, ID_DebugSwitch);

	Print_Error(error);
	signal(SIGINT, interrupt);
	printf("Press Ctrl+C to interrupt...\n");
	x=0;

	/*
	while(x<100) {
		//printf(": %02x\n",x);
		gsm->Phone.Data.RequestID	= ID_DebugTrace;
		res = gsm->Device.Functions->ReadDevice(gsm, buff, 255);
		if(res) {
			printf("%02x\n",x);
			for(y=0;y<res;y++) {
				//printf("%02x\n",x,buff[y]&0xFF);
				gsm->Protocol.Functions->StateMachine(gsm,buff[y]);
				x++;
			}
		}
	}
	*/
	;

	/* todo: wait and dump for some time */
	while (!gshutdown) {
		GSM_ReadDevice(gsm,TRUE);
		usleep(10000);
	}
	signal(SIGINT, SIG_DFL);
	printf("Disabling\n");
	error=GSM_WaitFor (gsm, reqDisable, sizeof(reqDisable), 0x40, 10, ID_DebugSwitch);
	Print_Error(error);

	GSMDecoder_free(gsmdec);
}

GSM_Reply_Function UserReplyFunctionsX[] = {
	{DCT3_ReplySwitchDebug,		"\x40",0x02,0x70,ID_DebugSwitch		},
	{DCT3_ReplySwitchDebug,		"\x40",0x02,0x71,ID_DebugSwitch		},
	{DCT3_ReplyDebugTrace,		"\x00",0x00,0x00,ID_IncomingFrame	},
	{DCT3_ReplyMyPacket,		"\x40",0x00,0x00,ID_IncomingFrame	},

	{DCT3_ReplyRPC,			"\xD2",0x00,0x00,ID_RPC			},

	{NULL,				"\x00",0x00,0x00,ID_None 		}
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
