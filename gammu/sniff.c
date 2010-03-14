
#include "../common/gsmstate.h"

#ifdef DEBUG

#include <string.h>
#include <stdio.h>

#include "../common/protocol/nokia/mbus2.h"
#include "../common/protocol/nokia/fbusirda.h"
#include "../common/phone/nokia/nfunc.h"
#include "../common/misc/misc.h"
#include "../common/gsmcomon.h"
#include "gammu.h"

static GSM_Protocol_MBUS2Data		MBUS2Data;
static GSM_Protocol_FBUS2IRDAData	IRDAData;

#define MBUS2_DEVICE_PC1     0x1D

static void DecodeInputMBUS2(unsigned char rx_byte)
{
	GSM_Protocol_MBUS2Data *d = &MBUS2Data;

	d->Msg.CheckSum[0] = d->Msg.CheckSum[1];
	d->Msg.CheckSum[1] ^= rx_byte;

	switch (d->MsgRXState) {

	case RX_Sync:

	if (rx_byte == MBUS2_FRAME_ID) {
		d->Msg.CheckSum[1] = MBUS2_FRAME_ID;
		d->Msg.Count = 0;
		d->MsgRXState = RX_GetDestination;	
	} else {
		printf("[ERROR: incorrect char - %02x, not %02x]\n", rx_byte, MBUS2_FRAME_ID);
	}
	break;

	case RX_GetDestination:

	if (rx_byte != MBUS2_DEVICE_PC && rx_byte != MBUS2_DEVICE_PHONE && rx_byte != MBUS2_DEVICE_PC1) {
		d->MsgRXState = RX_Sync;
		printf("[ERROR: incorrect char - %02x, not %02x and %02x and %02x]\n",
			rx_byte, MBUS2_DEVICE_PHONE, MBUS2_DEVICE_PC, MBUS2_DEVICE_PC1);
	} else {
		d->Msg.Destination = rx_byte;
		d->MsgRXState = RX_GetSource;
	}
	break;

	case RX_GetSource:

	if (rx_byte != MBUS2_DEVICE_PC && rx_byte != MBUS2_DEVICE_PHONE && rx_byte != MBUS2_DEVICE_PC1) {
		d->MsgRXState = RX_Sync;
		printf("[ERROR: incorrect char - %02x, not %02x and %02x and %02x]\n",
			rx_byte, MBUS2_DEVICE_PHONE, MBUS2_DEVICE_PC, MBUS2_DEVICE_PC1);
	} else {
		d->Msg.Source = rx_byte;
		d->MsgRXState = RX_GetType;
	}
	break;

	case RX_GetType:

	d->Msg.Type = rx_byte;
	d->MsgRXState = RX_GetLength1;
	break;

	case RX_GetLength1:

	d->Msg.Length = rx_byte * 256;
	d->MsgRXState = RX_GetLength2;
	break;
    
	case RX_GetLength2:

	if (d->Msg.Type == MBUS2_ACK_BYTE)
	{
		d->MsgRXState = RX_Sync;
	} else {
		d->Msg.Length = d->Msg.Length + rx_byte;
		d->MsgRXState = RX_GetMessage;
	}    
	break;
    
	case RX_GetMessage:

	d->Msg.Buffer[d->Msg.Count] = rx_byte;
	d->Msg.Count++;

	if (d->Msg.Count == d->Msg.Length+2) {
		/* If this is the last byte, it's the checksum. */
		/* Is the checksum correct? */        
		if (d->Msg.CheckSum[0] == rx_byte) {
			if (d->Msg.Destination != MBUS2_DEVICE_PHONE) {
				printf("Received frame");
			} else {
				printf("Sending frame");
			}
			printf(" 0x%02x / 0x%04x", d->Msg.Type, d->Msg.Length);
			DumpMessage(stdout, d->Msg.Buffer, d->Msg.Length);
			if (d->Msg.Destination != MBUS2_DEVICE_PHONE) {
				if (s.Phone.Functions != NULL) {
					s.Phone.Data.RequestMsg = &d->Msg;
					s.Phone.Functions->DispatchMessage(&s);
				}
			}
		} else {
			printf("[ERROR: checksum]\n");
			printf(" 0x%02x / 0x%04x", d->Msg.Type, d->Msg.Length);
			DumpMessage(stdout, d->Msg.Buffer, d->Msg.Length);
		}
		d->MsgRXState = RX_Sync;
	}
	break;

	}
}

#define FBUS2IRDA_DEVICE_PC1     0x10

static void DecodeInputIRDA(unsigned char rx_byte)
{
	GSM_Protocol_FBUS2IRDAData *d = &IRDAData;

	switch (d->MsgRXState) {

	case RX_Sync:

	if (rx_byte == FBUS2IRDA_FRAME_ID) {
		d->Msg.Count = 0;
		d->MsgRXState = RX_GetDestination;	
	}
//	} else {
//		printf("[ERROR: incorrect char - %02x, not %02x]\n", rx_byte, FBUS2IRDA_FRAME_ID);
//	}
	break;

	case RX_GetDestination:

	if (rx_byte != FBUS2IRDA_DEVICE_PC1 && rx_byte != FBUS2IRDA_DEVICE_PHONE) {
		d->MsgRXState = RX_Sync;
//	    	printf("[ERROR: incorrect char - %02x, not %02x and %02x]\n", rx_byte, FBUS2IRDA_DEVICE_PC1, FBUS2IRDA_DEVICE_PHONE);
	} else {
		d->Msg.Destination = rx_byte;
		d->MsgRXState = RX_GetSource;
	}
	break;

	case RX_GetSource:

	if (rx_byte != FBUS2IRDA_DEVICE_PHONE && rx_byte != FBUS2IRDA_DEVICE_PC1) {
		d->MsgRXState = RX_Sync;
//		printf("[ERROR: incorrect char - %02x, not %02x and %02x]\n", rx_byte, FBUS2IRDA_DEVICE_PHONE, FBUS2IRDA_DEVICE_PC1);
	} else {
		d->Msg.Source = rx_byte;
		d->MsgRXState = RX_GetType;
	}
	break;

	case RX_GetType:

	d->Msg.Type = rx_byte;
	d->MsgRXState = RX_GetLength1;
	break;

	case RX_GetLength1:

	d->Msg.Length = rx_byte * 256;
	d->MsgRXState = RX_GetLength2;
	break;
    
	case RX_GetLength2:

	d->Msg.Length = d->Msg.Length + rx_byte;
	d->MsgRXState = RX_GetMessage;
	break;
    
	case RX_GetMessage:

	d->Msg.Buffer[d->Msg.Count] = rx_byte;
	d->Msg.Count++;

	if (d->Msg.Count == d->Msg.Length) {
		if (d->Msg.Destination != FBUS2IRDA_DEVICE_PHONE) {
			printf("Received frame");
		} else {
			printf("Sending frame");
		}
		printf(" 0x%02x / 0x%04x", d->Msg.Type, d->Msg.Length);
		DumpMessage(stdout, d->Msg.Buffer, d->Msg.Length);
		if (d->Msg.Destination != FBUS2IRDA_DEVICE_PHONE) {
			if (s.Phone.Functions != NULL) {
				s.Phone.Data.RequestMsg = &d->Msg;
				s.Phone.Functions->DispatchMessage(&s);
			}
		}
		d->MsgRXState = RX_Sync;
	}
	break;

	}
}

static char				IMEI[50];
static GSM_DateTime			DateTime;
static GSM_DateTime			Alarm;
static GSM_PhonebookEntry		Memory;
static GSM_MemoryStatus			MemoryStatus;
static GSM_SMSC				SMSC;
static GSM_MultiSMSMessage		GetSMSMessage;
static GSM_SMSMessage			SaveSMSMessage;
static GSM_SMSMemoryStatus		SMSStatus;
static GSM_SMSFolders			SMSFolders;
static int				NetworkLevel;
static int				BatteryLevel;
static GSM_NetworkInfo			NetworkInfo;
static GSM_Ringtone			Ringtone;
static GSM_CalendarNote			Calendar;
static char				SecurityCode;
static GSM_WAPBookmark			WAPBookmark;
static GSM_Bitmap			Bitmap;
static char				PhoneString[500];

static char				Model[50];
static char				Version[50];
static double				VersionNum;

static void prepareStateMachine()
{
	GSM_Phone_Data	*Phone = &s.Phone.Data;

	Phone->IMEI			= IMEI;
	Phone->DateTime			= &DateTime;
	Phone->Alarm			= &Alarm;
	Phone->Memory			= &Memory;
	Phone->Memory->MemoryType	= GMT7110_CG;
	Phone->MemoryStatus		= &MemoryStatus;
	Phone->SMSC			= &SMSC;
	Phone->GetSMSMessage		= &GetSMSMessage;
	Phone->SaveSMSMessage		= &SaveSMSMessage;
	Phone->SMSStatus		= &SMSStatus;
	Phone->SMSFolders		= &SMSFolders;
	Phone->NetworkLevel		= &NetworkLevel;
	Phone->BatteryLevel		= &BatteryLevel;
	Phone->NetworkInfo		= &NetworkInfo;
	Phone->Ringtone			= &Ringtone;
	Phone->Ringtone->Format		= RING_NOKIABINARY;
	Phone->Calendar			= &Calendar;
	Phone->SecurityCode		= &SecurityCode;
	Phone->WAPBookmark		= &WAPBookmark;
	Phone->Bitmap			= &Bitmap;
	Phone->PhoneString		= PhoneString;
	Phone->StartPhoneString 	= 0;

	Phone->EnableIncomingSMS 	= false;
	Phone->EnableIncomingCB 	= false;
	Phone->Model			= Model;
	Model[0]			= 0;
	Phone->Version			= Version;
	Version[0]			= 0;
	Phone->VersionNum		= &VersionNum;
	VersionNum			= 0;

	s.Phone.Functions		= NULL;
	s.User.UserReplyFunctions	= NULL;
	Phone->RequestID		= ID_EachFrame;
}

void decodesniff(int argc, char *argv[])
{
	GSM_ConnectionType	Protocol	= GCT_MBUS2;
	unsigned char 		Buffer[50000];
	unsigned char 		Buffer2[50000];
	FILE			*file;
	int			len, len2, pos, state, i;
	unsigned char		mybyte1 = 0,mybyte2;

	if (!strcmp(argv[2],"MBUS2")) {
		Protocol = GCT_MBUS2;
    	} else if (!strcmp(argv[2],"IRDA")) {
		Protocol = GCT_IRDA;
	} else {
		printf("What protocol (\"%s\") ?\n",argv[2]);
		exit(-1);
	}
	file = fopen(argv[3], "rb");
	if (!file) {
		printf("Can not open file \"%s\"\n",argv[3]);
		exit(-1);
	}
	prepareStateMachine();
	if (argc > 4) {
		strcpy(s.CFGModel,argv[4]);
		error=GSM_RegisterAllPhoneModules(&s);
		if (error!=GE_NONE) Print_Error(error);
	}
	/* Irda uses simple "raw" format */
	if (Protocol == GCT_IRDA) {
		IRDAData.MsgRXState=RX_Sync;
		len2=30000;
		while (len2==30000) {	
			len2=fread(Buffer, 1, 30000, file);
			for (i=0;i<len2;i++) {
				DecodeInputIRDA(Buffer[i]);
			}
		}
	}
	/* MBUS2 uses PortMon format */
	if (Protocol == GCT_MBUS2) {
		MBUS2Data.MsgRXState=RX_Sync;
		len2=30000;
		state=0;
		while (len2==30000) {	
			len2=fread(Buffer, 1, 30000, file);
			pos=0;
			len=0;
			while (pos!=len2) {
				switch (state) {
				case 0:
					if (Buffer[pos]==' ') state = 1;
					break;
				case 1:
					state = 2;
					if (Buffer[pos]=='0') state = 0;
					break;
				case 2:
					if (Buffer[pos]>='0' && Buffer[pos]<='9') {
						state = 2;
					} else {
						if (Buffer[pos]==':') state = 3;
					}
					break;
				case 3:
					if (Buffer[pos]==' ') state = 4;
					break;
				case 4:
					if (Buffer[pos]==13) {
						state = 0;
						break;
					}
					mybyte1=Buffer[pos]-'0';
					if (Buffer[pos]>'9') mybyte1=Buffer[pos]-'A'+10;
					state = 5;
					break;
				case 5:
					mybyte2=Buffer[pos]-'0';
					if (Buffer[pos]>'9') mybyte2=Buffer[pos]-'A'+10;
					Buffer2[len++]=mybyte1*16+mybyte2;
					state = 6;
					break;
				case 6:
					state = 4;
					if (Buffer[pos]!=' ') state = 0;
					break;
				}
				pos++;		
			}	
			for (i=0;i<len;i++) {
				DecodeInputMBUS2(Buffer2[i]);
			}
		}
	}
	fclose(file);
}

void decodebinarydump(int argc, char *argv[])
{
	unsigned char 		Buffer[50000];
	FILE			*file;
	int			len, len2, i;
	unsigned char		type;
	bool			sent;
	GSM_Protocol_Message	msg;

	prepareStateMachine();
	if (argc > 3) {
		strcpy(s.CFGModel,argv[3]);
		error=GSM_RegisterAllPhoneModules(&s);
		if (error!=GE_NONE) Print_Error(error);
	}
	file = fopen(argv[2], "rb");
	if (!file) {
		printf("Can not open file \"%s\"\n",argv[2]);
		exit(-1);
	}
	len2=fread(Buffer, 1, 1, file);
	len2=Buffer[0];
	len =fread(Buffer, 1, len2, file);
	Buffer[len2]=0;
	dprintf("[Gammu            - version %s]\n",Buffer);
	len2=30000;
	while (len2==30000) {	
		len2=fread(Buffer, 1, 30000, file);
		i=0;
		while (i!=len2) {
			if (Buffer[i++]==0x01) {
				dprintf("Sending frame ");
				sent = true;
			} else {
				dprintf("Receiving frame ");
				sent = false;
			}
			type 	= Buffer[i++];
			len 	= Buffer[i++] * 256;
			len 	= len + Buffer[i++];
			dprintf("0x%02x / 0x%04x", type, len);
			DumpMessage(stdout, Buffer+i, len);
			fflush(stdout);
			if (s.Phone.Functions != NULL && !sent) {
				memcpy(msg.Buffer,Buffer+i,len);
				msg.Type		= type;
				msg.Length		= len;
				s.Phone.Data.RequestMsg = &msg;
				s.Phone.Functions->DispatchMessage(&s);
			}
			i = i + len;
		}
	}

}

#endif
