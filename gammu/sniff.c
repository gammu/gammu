/* (c) 2002-2003 by Marcin Wiacek */

#include "../libgammu/gsmstate.h"

#ifdef DEBUG

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../libgammu/protocol/nokia/mbus2.h"
#include "../libgammu/protocol/nokia/phonet.h"
#include "../libgammu/phone/nokia/nfunc.h"
#include "../libgammu/misc/misc.h"
#include "../libgammu/gsmcomon.h"
#include "../libgammu/service/gsmcal.h"
#include "gammu.h"

static GSM_Protocol_MBUS2Data	MBUS2Data;
static GSM_Protocol_PHONETData	PHONETData;

#define MBUS2_DEVICE_PC1     0x1D

static void DecodeInputMBUS2(unsigned char rx_byte)
{
	GSM_Protocol_MBUS2Data *d = &MBUS2Data;
	GSM_Debug_Info	ldi = {DL_TEXTALL, stdout, false, NULL, true, false, NULL, NULL};

	d->Msg.CheckSum[0] = d->Msg.CheckSum[1];
	d->Msg.CheckSum[1] ^= rx_byte;

	if (d->MsgRXState == RX_GetMessage) {
		d->Msg.Buffer[d->Msg.Count] = rx_byte;
		d->Msg.Count++;

		if (d->Msg.Count != d->Msg.Length+2) return;

		if (d->Msg.CheckSum[0] != rx_byte) {
			printf("[ERROR: checksum]\n");
			printf(" 0x%02x / 0x%04lx", d->Msg.Type, (long)d->Msg.Length);
			DumpMessage(&ldi, d->Msg.Buffer, d->Msg.Length);
			d->MsgRXState = RX_Sync;
			return;
		}

		if (d->Msg.Destination != MBUS2_DEVICE_PHONE) {
			printf("Received frame");
		} else {
			printf("Sending frame");
		}
		printf(" 0x%02x / 0x%04lx", d->Msg.Type, (long)d->Msg.Length);
		DumpMessage(&ldi, d->Msg.Buffer, d->Msg.Length);
		if (d->Msg.Destination != MBUS2_DEVICE_PHONE) {
			if (gsm->Phone.Functions != NULL) {
				gsm->Phone.Data.RequestMsg = &d->Msg;
				gsm->Phone.Functions->DispatchMessage(gsm);
			}
		}
		d->MsgRXState = RX_Sync;
		return;
	}
	if (d->MsgRXState == RX_GetLength2) {
		if (d->Msg.Type == MBUS2_ACK_BYTE) {
			d->MsgRXState = RX_Sync;
		} else {
			d->Msg.Length = d->Msg.Length + rx_byte;
			d->MsgRXState = RX_GetMessage;
		}
		return;
	}
	if (d->MsgRXState == RX_GetLength1) {
		d->Msg.Length = rx_byte * 256;
		d->MsgRXState = RX_GetLength2;
		return;
	}
	if (d->MsgRXState == RX_GetType) {
		d->Msg.Type 	= rx_byte;
		d->MsgRXState 	= RX_GetLength1;
		return;
	}
	if (d->MsgRXState == RX_GetSource) {
		if (rx_byte != MBUS2_DEVICE_PC && rx_byte != MBUS2_DEVICE_PHONE && rx_byte != MBUS2_DEVICE_PC1) {
			d->MsgRXState = RX_Sync;
			printf("[ERROR: incorrect char - %02x, not %02x and %02x and %02x]\n",
				rx_byte, MBUS2_DEVICE_PHONE, MBUS2_DEVICE_PC, MBUS2_DEVICE_PC1);
		} else {
			d->Msg.Source = rx_byte;
			d->MsgRXState = RX_GetType;
		}
		return;
	}
	if (d->MsgRXState == RX_GetDestination) {
		if (rx_byte != MBUS2_DEVICE_PC && rx_byte != MBUS2_DEVICE_PHONE && rx_byte != MBUS2_DEVICE_PC1) {
			d->MsgRXState = RX_Sync;
			printf("[ERROR: incorrect char - %02x, not %02x and %02x and %02x]\n",
				rx_byte, MBUS2_DEVICE_PHONE, MBUS2_DEVICE_PC, MBUS2_DEVICE_PC1);
		} else {
			d->Msg.Destination 	= rx_byte;
			d->MsgRXState 		= RX_GetSource;
		}
		return;
	}
	if (d->MsgRXState == RX_Sync) {
		if (rx_byte == MBUS2_FRAME_ID) {
			d->Msg.CheckSum[1] 	= MBUS2_FRAME_ID;
			d->Msg.Count 		= 0;
			d->MsgRXState 		= RX_GetDestination;
		} else {
			printf("[ERROR: incorrect char - %02x, not %02x]\n", rx_byte, MBUS2_FRAME_ID);
		}
	}
}

#define PHONETIRDA_DEVICE_PC1     0x10

static void DecodeInputIRDA(unsigned char rx_byte)
{
	GSM_Protocol_PHONETData *d = &PHONETData;
	GSM_Debug_Info		ldi = {DL_TEXTALL, stdout, false, NULL, true, false, NULL, NULL};

	if (d->MsgRXState == RX_GetMessage) {
		d->Msg.Buffer[d->Msg.Count] = rx_byte;
		d->Msg.Count++;

		if (d->Msg.Count != d->Msg.Length) return;

		if (d->Msg.Destination != PHONET_DEVICE_PHONE) {
			printf("Received frame");
		} else {
			printf("Sending frame");
		}
		printf(" 0x%02x / 0x%04lx", d->Msg.Type, (long)d->Msg.Length);
		DumpMessage(&ldi, d->Msg.Buffer, d->Msg.Length);
		if (d->Msg.Destination != PHONET_DEVICE_PHONE) {
			if (gsm->Phone.Functions != NULL) {
				gsm->Phone.Data.RequestMsg = &d->Msg;
				gsm->Phone.Functions->DispatchMessage(gsm);
			}
		}
		d->MsgRXState = RX_Sync;
		return;
	}
	if (d->MsgRXState == RX_GetLength2) {
		d->Msg.Length = d->Msg.Length + rx_byte;
		d->MsgRXState = RX_GetMessage;
		return;
	}
	if (d->MsgRXState == RX_GetLength1) {
		d->Msg.Length = rx_byte * 256;
		d->MsgRXState = RX_GetLength2;
		return;
	}
	if (d->MsgRXState == RX_GetType) {
		d->Msg.Type 	= rx_byte;
		d->MsgRXState 	= RX_GetLength1;
		return;
	}
	if (d->MsgRXState == RX_GetSource) {
		if (rx_byte != PHONET_DEVICE_PHONE && rx_byte != PHONETIRDA_DEVICE_PC1) {
			d->MsgRXState = RX_Sync;
		} else {
			d->Msg.Source = rx_byte;
			d->MsgRXState = RX_GetType;
		}
		return;
	}
	if (d->MsgRXState == RX_GetDestination) {
		if (rx_byte != PHONETIRDA_DEVICE_PC1 && rx_byte != PHONET_DEVICE_PHONE) {
			d->MsgRXState = RX_Sync;
		} else {
			d->Msg.Destination 	= rx_byte;
			d->MsgRXState 		= RX_GetSource;
		}
		return;
	}
	if (d->MsgRXState == RX_Sync) {
		if (rx_byte == PHONET_FRAME_ID) {
			d->Msg.Count = 0;
			d->MsgRXState = RX_GetDestination;
		}
	}
}

static char				IMEI[50];
static GSM_DateTime			DateTime;
static GSM_Alarm			Alarm;
static GSM_MemoryEntry			Memory;
static GSM_MemoryStatus			MemoryStatus;
static GSM_SMSC				SMSC;
static GSM_MultiSMSMessage		GetSMSMessage;
static GSM_SMSMessage			SaveSMSMessage;
static GSM_SMSMemoryStatus		SMSStatus;
static GSM_SMSFolders			SMSFolders;
static GSM_SignalQuality		SignalQuality;
static GSM_BatteryCharge		BatteryCharge;
static GSM_NetworkInfo			NetworkInfo;
static GSM_Ringtone			Ringtone;
static GSM_CalendarEntry		Calendar;
static char				SecurityCode;
static GSM_WAPBookmark			WAPBookmark;
static GSM_Bitmap			Bitmap;
static char				PhoneString[500];

static char				Model[50];
static char				Version[50];
static double				VersionNum;

static void prepareStateMachine(void)
{
	GSM_Phone_Data	*Phone = &(gsm->Phone.Data);

	strcpy(Phone->IMEI,		IMEI);
	strcpy(Phone->Model,		Model);
	strcpy(Phone->Version,		Version);
	Phone->DateTime			= &DateTime;
	Phone->Alarm			= &Alarm;
	Phone->Memory			= &Memory;
	Phone->Memory->MemoryType	= MEM7110_CG;
	Phone->MemoryStatus		= &MemoryStatus;
	Phone->SMSC			= &SMSC;
	Phone->GetSMSMessage		= &GetSMSMessage;
	Phone->SaveSMSMessage		= &SaveSMSMessage;
	Phone->SMSStatus		= &SMSStatus;
	Phone->SMSFolders		= &SMSFolders;
	Phone->SignalQuality		= &SignalQuality;
	Phone->BatteryCharge		= &BatteryCharge;
	Phone->NetworkInfo		= &NetworkInfo;
	Phone->Ringtone			= &Ringtone;
	Phone->Ringtone->Format		= RING_NOKIABINARY;
	Phone->Cal			= &Calendar;
	Phone->SecurityCode		= &SecurityCode;
	Phone->WAPBookmark		= &WAPBookmark;
	Phone->Bitmap			= &Bitmap;
	Phone->PhoneString		= PhoneString;
	Phone->StartPhoneString 	= 0;

	Phone->EnableIncomingSMS 	= false;
	Phone->EnableIncomingCB 	= false;
	Model[0]			= 0;
	Phone->VerNum			= VersionNum;
	Version[0]			= 0;
	VersionNum			= 0;

	gsm->Phone.Functions		= NULL;
	gsm->User.UserReplyFunctions	= NULL;
	Phone->RequestID		= ID_EachFrame;
}

void decodesniff(int argc, char *argv[])
{
	GSM_ConnectionType	Protocol = GCT_MBUS2;
	unsigned char 		Buffer[50000];
	unsigned char 		Buffer2[50000];
	FILE			*file;
	int			len, len2, pos, state, i;
	unsigned char		mybyte1 = 0,mybyte2;
	GSM_Error error;

	if (!strcmp(argv[2],"MBUS2")) {
		Protocol = GCT_MBUS2;
    	} else if (!strcmp(argv[2],"IRDA")) {
		Protocol = GCT_IRDAPHONET;
	} else {
		printf("What protocol (\"%s\") ?\n",argv[2]);
		Terminate(3);
	}
	file = fopen(argv[3], "rb");
	if (file == NULL) {
		printf("Can not open file \"%s\"\n",argv[3]);
		Terminate(3);
	}
	prepareStateMachine();
	if (argc > 4) {
		strcpy(gsm->CurrentConfig->Model,argv[4]);
		error = GSM_RegisterAllPhoneModules(gsm);
		if (error!=ERR_NONE) Print_Error(error);
	}
	/* Irda uses simple "raw" format */
	if (Protocol == GCT_IRDAPHONET) {
		PHONETData.MsgRXState=RX_Sync;
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
	GSM_Debug_Info		ldi = {DL_TEXTALL, stdout, false, NULL, true, false, NULL, NULL};
	GSM_Error error;

	prepareStateMachine();
	if (argc > 3) {
		strcpy(gsm->CurrentConfig->Model,argv[3]);
		error = GSM_RegisterAllPhoneModules(gsm);
		if (error!=ERR_NONE) Print_Error(error);
	}
	file = fopen(argv[2], "rb");
	if (file == NULL) {
		printf("Can not open file \"%s\"\n",argv[2]);
		Terminate(3);
	}
	len2=30000;
	msg.Buffer = NULL;
	while (len2==30000) {
		len2=fread(Buffer, 1, 30000, file);
		i=0;
		while (i!=len2) {
			if (Buffer[i++]==0x01) {
				smprintf(gsm, "Sending frame ");
				sent = true;
			} else {
				smprintf(gsm, "Receiving frame ");
				sent = false;
			}
			type 	= Buffer[i++];
			len 	= Buffer[i++] * 256;
			len 	= len + Buffer[i++];
			smprintf(gsm, "0x%02x / 0x%04x", type, len);
			DumpMessage(&ldi, Buffer+i, len);
			fflush(stdout);
			if (gsm->Phone.Functions != NULL && !sent) {
				msg.Buffer = (unsigned char *)realloc(msg.Buffer,len);
				memcpy(msg.Buffer,Buffer+i,len);
				msg.Type		= type;
				msg.Length		= len;
				gsm->Phone.Data.RequestMsg = &msg;
				gsm->Phone.Functions->DispatchMessage(gsm);
			}
			i = i + len;
		}
	}

}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
