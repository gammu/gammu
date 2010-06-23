
#include "../../common/gsmstate.h"

#ifdef GSM_ENABLE_NOKIA_DCT4

#include <string.h>

#include "../gammu.h"
#include "../../common/misc/coding.h"

extern GSM_Reply_Function UserReplyFunctions4[];

/* ------- some usefull functions ----------------------------------------- */

GSM_Error CheckDCT4Only()
{
	bool found = false;

	/* Checking if phone is DCT4 */
#ifdef GSM_ENABLE_NOKIA6510
	if (strstr(N6510Phone.models, GetModelData(NULL,s.Model,NULL)->model) != NULL) found = true;
#endif
	if (!found) return GE_NOTSUPPORTED;

	if (s.ConnectionType!=GCT_FBUS2	   && s.ConnectionType!=GCT_MBUS2  &&
	    s.ConnectionType!=GCT_INFRARED && s.ConnectionType!=GCT_DLR3AT &&
	    s.ConnectionType!=GCT_IRDA)
	{
		return GE_UNKNOWNCONNECTIONTYPESTRING;
	}
	return GE_NONE;
}

static void CheckDCT4()
{
	GSM_Error error;

	error = CheckDCT4Only();
	switch (error) {
	case GE_NOTSUPPORTED:
		Print_Error(GE_NOTSUPPORTED);
		break;
	case GE_UNKNOWNCONNECTIONTYPESTRING:
		printf("Can't do it with current phone protocol\n");
		GSM_TerminateConnection(&s);
		exit(-1);
	default:
		break;
	}
}

/* ------------------- functions ------------------------------------------- */

static GSM_Error DCT4_ReplySetPPS(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	printf("Setting done OK\n");
	return GE_NONE;
}

void DCT4SetPhoneMenus(int argc, char *argv[])
{
	int 		current = 10;
	unsigned char 	reqSet[200] = {
		N7110_FRAME_HEADER,0x04,0x00,0x01,0x47,0x48,0x02,
		0x00};		/* Number of changed features */

	if (CheckDCT4Only()!=GE_NONE) return;

	s.User.UserReplyFunctions=UserReplyFunctions4;

	printf("ALS : enabling menu\n");
	reqSet[9]++;			/* Number of features */
	reqSet[current++] = 1; 		/* Feature number */
	reqSet[current++] = 0x01; 	/* 0x01 = ON, 0x00 = OFF */

	if (!strcmp(GetModelData(NULL,s.Model,NULL)->model,"6310")  ||
	    !strcmp(GetModelData(NULL,s.Model,NULL)->model,"6310i") ||
	    !strcmp(GetModelData(NULL,s.Model,NULL)->model,"6510")  ||
	    !strcmp(GetModelData(NULL,s.Model,NULL)->model,"3510")) {
		printf("Enabling ciphering algorithm A52\n");
		reqSet[9]++;			/* Number of features */
		reqSet[current++] = 3; 		/* Feature number */
		reqSet[current++] = 0x01; 	/* 0x01 = ON, 0x00 = OFF */
	}

	printf("Enabling games WAP download\n");
	reqSet[9]++;			/* Number of features */
	reqSet[current++] = 7; 		/* Feature number */
	reqSet[current++] = 0x01; 	/* 0x01 = ON, 0x00 = OFF */

	printf("Enabling games WAP score send\n");
	reqSet[9]++;			/* Number of features */
	reqSet[current++] = 8; 		/* Feature number */
	reqSet[current++] = 0x01; 	/* 0x01 = ON, 0x00 = OFF */

	printf("Enabling games WAP URL check\n");
	reqSet[9]++;			/* Number of features */
	reqSet[current++] = 9;	 	/* Feature number */
	reqSet[current++] = 0x01; 	/* 0x01 = ON, 0x00 = OFF */

	/* Impossible, but my data says, that 6510 has Bluetooth at least
	 * in firmware. For checking
	 */
	if (!strcmp(GetModelData(NULL,s.Model,NULL)->model,"6310") ||
	    !strcmp(GetModelData(NULL,s.Model,NULL)->model,"6310i") ||
	    !strcmp(GetModelData(NULL,s.Model,NULL)->model,"6510")) {
		printf("Enabling Bluetooth menu\n");
		reqSet[9]++;			/* Number of features */
		reqSet[current++] = 10; 	/* Feature number */
		reqSet[current++] = 0x01; 	/* 0x01 = ON, 0x00 = OFF */
	}

	printf("Enabling GPRS PCCCH support\n");
	reqSet[9]++;			/* Number of features */
	reqSet[current++] = 13; 	/* Feature number */
	reqSet[current++] = 0x01; 	/* 0x01 = ON, 0x00 = OFF */

//	printf("Enabling GPRS automatic attach\n");
//	reqSet[9]++;			/* Number of features */
//	reqSet[current++] = 14; 	/* Feature number */
//	reqSet[current++] = 0x01; 	/* 0x01 = ON, 0x00 = OFF */

	if (!strcmp(GetModelData(NULL,s.Model,NULL)->model,"6310")  ||
	    !strcmp(GetModelData(NULL,s.Model,NULL)->model,"6310i") ||
	    !strcmp(GetModelData(NULL,s.Model,NULL)->model,"6510")  ||
	    !strcmp(GetModelData(NULL,s.Model,NULL)->model,"3510")) {
		printf("Enabling GEA1 support indication\n");
		reqSet[9]++;			/* Number of features */
		reqSet[current++] = 15; 	/* Feature number */
		reqSet[current++] = 0x01; 	/* 0x01 = ON, 0x00 = OFF */
	}

	if (!strcmp(GetModelData(NULL,s.Model,NULL)->model,"6310i")) {
		printf("Enabling EOTD support\n");
		reqSet[9]++;			/* Number of features */
		reqSet[current++] = 16; 	/* Feature number */
		reqSet[current++] = 0x01; 	/* 0x01 = ON, 0x00 = OFF */
	}

	if (!strcmp(GetModelData(NULL,s.Model,NULL)->model,"6310") ||
	    !strcmp(GetModelData(NULL,s.Model,NULL)->model,"8310") ||
	    !strcmp(GetModelData(NULL,s.Model,NULL)->model,"6510") ||
	    !strcmp(GetModelData(NULL,s.Model,NULL)->model,"3510")) {
		printf("Enabling GPRS always online\n");
		reqSet[9]++;			/* Number of features */
		reqSet[current++] = 18; 	/* Feature number */
		reqSet[current++] = 0x00; 	/* 0x00 = Context, 0x01 = Attach */
	}

	if (!strcmp(GetModelData(NULL,s.Model,NULL)->model,"6310i")) {
		printf("Enabling GPRS always online\n");
		reqSet[9]++;			/* Number of features */
		reqSet[current++] = 17; 	/* Feature number */
		reqSet[current++] = 0x00; 	/* 0x00 = Context, 0x01 = Attach */

		printf("Enabling Java games in Games menu\n");
		reqSet[9]++;			/* Number of features */
		reqSet[current++] = 18; 	/* Feature number */
		reqSet[current++] = 0x01; 	/* 0x01 = ON, 0x00 = OFF */

		printf("Enabling bookmarks menu in Services\n");
		reqSet[9]++;			/* Number of features */
		reqSet[current++] = 20; 	/* Feature number */
		reqSet[current++] = 0x01; 	/* 0x01 = ON, 0x00 = OFF */

		printf("Enabling Services setting menu\n");
		reqSet[9]++;			/* Number of features */
		reqSet[current++] = 21; 	/* Feature number */
		reqSet[current++] = 0x00; 	/* 0x01 = hidden, 0x00 = visible */

		printf("Enabling Service menu item (?)\n");
		reqSet[9]++;			/* Number of features */
		reqSet[current++] = 23; 	/* Feature number */
		reqSet[current++] = 0x01; 	/* 0x01 = ON, 0x00 = OFF */

		printf("Enabling WAP push on/off\n");
		reqSet[9]++;			/* Number of features */
		reqSet[current++] = 28; 	/* Feature number */
		reqSet[current++] = 0x01; 	/* 0x01 = ON, 0x00 = OFF */

		printf("Enabling GoTo in Services menu\n");
		reqSet[9]++;			/* Number of features */
		reqSet[current++] = 29; 	/* Feature number */
		reqSet[current++] = 0x00; 	/* 0x01 = hidden, 0x00 = visible */
	}

	reqSet[current++] = 0x00;
	reqSet[current++] = 0x00;

	error=GSM_WaitFor (&s, reqSet, current, 0x1b, 4, ID_User1);
	Print_Error(error);
}

void DCT4tests(int argc, char *argv[])
{
	unsigned char buffer[6]  = {0x00,0x06,0x04,0x00,0x03,0x00};

#ifndef DEBUG
	return;
#endif

	if (CheckDCT4Only()!=GE_NONE) return;

	s.User.UserReplyFunctions=UserReplyFunctions4;

	error = s.Protocol.Functions->WriteMessage(&s, buffer, 6, 0x35);
}

static GSM_Error DCT4_ReplyVibra(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
#ifdef DEBUG
	switch (msg.Buffer[3]) {
		case 0x0D : dprintf("Vibra state set OK\n"); break;
		case 0x0F : dprintf("Vibra power set OK\n"); break;
	}
#endif
	return GE_NONE;
}

void DCT4SetVibraLevel(int argc, char *argv[])
{
	GSM_DateTime	Date;
	unsigned int	i,j;

	/* Set vibra level */
	unsigned char 	SetLevel[6] = {N7110_FRAME_HEADER,0x0E,
		0x64,		/* Vibra power (in percent) */
		0x00};
	/* Enables or disables vibra */
	unsigned char 	Control[6] = {N7110_FRAME_HEADER,0x0C,
		0x01,		/* 0x01 = On, 0x00 = Off */
		0x00};

	GSM_Init(true);

        CheckDCT4();

	s.User.UserReplyFunctions=UserReplyFunctions4;

	SetLevel[4] = atoi(argv[2]);
	error=GSM_WaitFor (&s, SetLevel, 6, 0x1C, 4, ID_User3);
	Print_Error(error);

	error=GSM_WaitFor (&s, Control, 6, 0x1C, 4, ID_User3);
	Print_Error(error);

	for (i=0;i<3;i++) {
		GSM_GetCurrentDateTime (&Date);
		j=Date.Second;
		while (j==Date.Second) {
			mili_sleep(10);
			GSM_GetCurrentDateTime(&Date);
		}
	}

	Control[4] = 0x00;
	error=GSM_WaitFor (&s, Control, 6, 0x1C, 4, ID_User3);
	Print_Error(error);

	GSM_Terminate();
}

static GSM_Error DCT4_ReplyResetSecurityCode(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	switch (msg.Buffer[3]) {
	case 0x05:
		printf("Security code set to \"12345\"\n");
		return GE_NONE;
	case 0x06:
		printf("Unknown reason. Can't reset your security code\n");
		return GE_NONE;
	}
	return GE_UNKNOWNRESPONSE;
}

void DCT4ResetSecurityCode(int argc, char *argv[])
{
	unsigned char ResetCode[22] = {0x00,0x06,0x03,0x04,0x01,
		'1','2','3','4','5','6','7','8','9','0',	/* Old code */
		0x00,
		'1','2','3','4','5',				/* New code */
		0x00};

	if (CheckDCT4Only()!=GE_NONE) return;

	s.User.UserReplyFunctions=UserReplyFunctions4;

	error=GSM_WaitFor (&s, ResetCode, 22, 0x08, 4, ID_User2);
	Print_Error(error);
}

static GSM_Error DCT4_ReplyGetVoiceRecord(GSM_Protocol_Message msg, GSM_Phone_Data *Data, GSM_User *User)
{
	int 		i=18,j;
	unsigned char	Buffer[100];

	switch (msg.Buffer[3]) {
	case 0x05:
		dprintf("Part of voice record received\n");
		if (msg.Length == 6) {
			dprintf("Empty\n");
			return GE_EMPTY;
		}
		*Data->VoiceRecord = 0;
		while (i<msg.Length) {
			Data->PhoneString[(*Data->VoiceRecord)++] = msg.Buffer[i+1];
			Data->PhoneString[(*Data->VoiceRecord)++] = msg.Buffer[i];
			i += 2;
		}
		return GE_NONE;
	case 0x0D:
		dprintf("Last part of voice record is %02x %02x\n",msg.Buffer[11],msg.Buffer[12]);
		dprintf("Token is %02x\n",msg.Buffer[13]);
		Data->PhoneString[0] = msg.Buffer[11];
		Data->PhoneString[1] = msg.Buffer[12];
		Data->PhoneString[2] = msg.Buffer[13];
		return GE_NONE;
		break;
	case 0x31:
		dprintf("Names of voice records received\n");
		j = 33;
		for (i=0;i<msg.Buffer[9];i++) {
			memcpy(Buffer,msg.Buffer+(j+1),msg.Buffer[j]);
			Buffer[msg.Buffer[j]] 	= 0;
			Buffer[msg.Buffer[j]+1] = 0;
			dprintf("%i. \"%s\"\n",i+1,DecodeUnicodeString(Buffer));	
			if (i==*Data->VoiceRecord) {
				sprintf(Data->PhoneString,"%s.wav",DecodeUnicodeString(Buffer));
				return GE_NONE;
			}
			if (i != msg.Buffer[9] - 1) {
				j+=msg.Buffer[j] + 1;
				if (msg.Buffer[j] == 0x00 && msg.Buffer[j+1]==0x00) j+=2;
				j+=23;
			}
		}
		return GE_EMPTY;
	}
	return GE_UNKNOWNRESPONSE;
}

void DCT4GetVoiceRecord(int argc, char *argv[])
{
	/* Voice records names */
	unsigned char 	ReqNames[200] = {
		N7110_FRAME_HEADER,
		0x30,0x01,0x55,0x00,0x00,0xFF,0xFF,0x01,0x01,0x55,0x55};
	/* Voice record token */
	unsigned char 	ReqToken[200] = {
		N7110_FRAME_HEADER,0x0C,0x00,0x44,0x00,
		0x00,		/* Location: 0, 1, ... */
		0x55,0x55};
	/* Voice record part */
	unsigned char 	ReqGet[200] = {
		N7110_FRAME_HEADER,0x04,0x00,0x44,
		0x00,0x00,	/* Location: 0, 1, ...  */
		0x55,0x55,0x00,
		0x00,0x00,	/* Part Location	*/
		0x00,0x00,0x00,
		0x04,		/* ???			*/
		0x00};		/* Token		*/

	/* WAV file headers */
	unsigned char 	WAV_Header[] = {
			'R','I','F','F',
			0x00,0x00,0x00,0x00,	/* Length */
			'W','A','V','E'};
	unsigned char 	FMT_Header[] = {'f','m','t',' ',
			0x14,0x00,0x00,0x00,0x31,0x00,0x01,0x00,0x40,0x1f,
			0x00,0x00,0x59,0x06,0x00,0x00,0x41,0x00,0x00,0x00,
			0x02,0x00,0x40,0x01,'f', 'a', 'c', 't', 0x04,0x00,
			0x00,0x00,
			0x00,0x73,0x00,0x00};	/* Seems to be some length */
	unsigned char 	DATA_Header[] = {
			'd','a','t','a',
			0x00,0x00,0x00,0x00};	/* Length */

	long		wavfilesize=0;
	unsigned char	FileName[100], Buffer[10000], Token;
	unsigned int 	Location, size=0, CurrentLocation = 0, TokenLocation;
	int		i;
	FILE		*WAVFile;

	Location = atoi(argv[2]);
	if (Location == 0x00) {
		printf("Please numerate locations from 1\n");
		return;
	}
	Location--;

	GSM_Init(true);

        CheckDCT4();

	s.User.UserReplyFunctions=UserReplyFunctions4;

	s.Phone.Data.VoiceRecord 	= &Location;
	s.Phone.Data.PhoneString 	= FileName;
	dprintf("Getting voice record name\n");
	error=GSM_WaitFor (&s, ReqNames, 14, 0x4A, 4, ID_User4);
	Print_Error(error);
	
	s.Phone.Data.PhoneString 	= Buffer;
	ReqToken[7] 			= Location;
	dprintf("Getting voice record token\n");
	error=GSM_WaitFor (&s, ReqToken, 18, 0x23, 4, ID_User4);
	Print_Error(error);
	TokenLocation 			= Buffer[0] * 256 + Buffer[1];
	Token				= Buffer[2];

	WAVFile = fopen(FileName, "wb");      

	fwrite(&WAV_Header,	1, sizeof(WAV_Header),	WAVFile);
	fwrite(&FMT_Header,	1, sizeof(FMT_Header),	WAVFile);
	fwrite(&DATA_Header,	1, sizeof(DATA_Header),	WAVFile);

	s.Phone.Data.VoiceRecord 	= &size;
	s.Phone.Data.PhoneString 	= Buffer;
	ReqGet[7]			= Location;
	fprintf(stderr,"Getting voice record and saving to \"%s\": ",FileName);
	while (1) {
		dprintf("Getting next part of voice record\n");
		fprintf(stderr,".");
		error=GSM_WaitFor (&s, ReqGet, 18, 0x23, 4, ID_User4);
		if (error == GE_NONE) {
			wavfilesize += size;
			fwrite(Buffer,1,size,WAVFile);
		}
		if (error == GE_EMPTY) break;
		Print_Error(error);
		CurrentLocation += 4;
		ReqGet[11] = CurrentLocation / 256;
		ReqGet[12] = CurrentLocation % 256;
		if (CurrentLocation+4 > TokenLocation) break;
	}
	dprintf("Getting first part in last sequence of voice record\n");
	for (i=255;i>=0;i--) {
		ReqGet[16] = i;
		ReqGet[17] = Token;
		fprintf(stderr,".");
		error=GSM_WaitFor (&s, ReqGet, 18, 0x23, 4, ID_User4);
		if (error == GE_NONE) {
			wavfilesize += size;
			fwrite(Buffer,1,size,WAVFile);
			break;
		}
		if (error != GE_EMPTY) Print_Error(error);
	}
	while (1) {
		dprintf("Getting next part of last sequence in voice record\n");
		CurrentLocation += 4;
		ReqGet[11] = CurrentLocation / 256;
		ReqGet[12] = CurrentLocation % 256;
		fprintf(stderr,".");
		error=GSM_WaitFor (&s, ReqGet, 18, 0x23, 4, ID_User4);
		if (error == GE_NONE) {
			wavfilesize += size;
			fwrite(Buffer,1,size,WAVFile);
		}
		if (error == GE_EMPTY) break;
		Print_Error(error);
	}
	fprintf(stderr,"\n");

	wavfilesize 	+= sizeof(WAV_Header) + sizeof(FMT_Header) + sizeof(DATA_Header);
	WAV_Header[4] 	= wavfilesize % 256;
	WAV_Header[5] 	= wavfilesize / 256;
	WAV_Header[6] 	= wavfilesize / (256*256);
	WAV_Header[7] 	= wavfilesize / (256*256*256);

	/* FIXME */
	FMT_Header[36]	= ((wavfilesize - 238) * 5 ) % 256;
	FMT_Header[37]	= ((wavfilesize - 238) * 5 ) / 256;
	FMT_Header[38]	= ((wavfilesize - 238) * 5 ) / (256*256);
	FMT_Header[39]	= ((wavfilesize - 238) * 5 ) / (256*256*256);

	wavfilesize 	= wavfilesize - 54 - 6;
	DATA_Header[4] 	= wavfilesize % 256;
	DATA_Header[5] 	= wavfilesize / 256;
	DATA_Header[6] 	= wavfilesize / (256*256);
	DATA_Header[7] 	= wavfilesize / (256*256*256);

	fseek( WAVFile, 0, SEEK_SET);
	fwrite(&WAV_Header,	1, sizeof(WAV_Header),	WAVFile);
	fwrite(&FMT_Header,	1, sizeof(FMT_Header),	WAVFile);
	fwrite(&DATA_Header,	1, sizeof(DATA_Header),	WAVFile);

	fclose(WAVFile);

	GSM_Terminate();
}

static GSM_Reply_Function UserReplyFunctions4[] = {

	{DCT4_ReplyResetSecurityCode,	"\x08",0x03,0x05,ID_User2	},
	{DCT4_ReplyResetSecurityCode,	"\x08",0x03,0x06,ID_User2	},

	{DCT4_ReplySetPPS,		"\x1b",0x03,0x05,ID_User1	},

	{DCT4_ReplyVibra,		"\x1C",0x03,0x0D,ID_User3	},
	{DCT4_ReplyVibra,		"\x1C",0x03,0x0F,ID_User3	},

	{DCT4_ReplyGetVoiceRecord,	"\x23",0x03,0x05,ID_User4	},
	{DCT4_ReplyGetVoiceRecord,	"\x23",0x03,0x0D,ID_User4	},

	{DCT4_ReplyGetVoiceRecord,	"\x4A",0x03,0x31,ID_User4	},

	{NULL,				"\x00",0x00,0x00,ID_None	}
};

#endif
