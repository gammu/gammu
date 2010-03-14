
#include "../../common/gsmstate.h"

#ifdef GSM_ENABLE_NOKIA_DCT4

#include <string.h>

#include "dct4.h"
#include "../gammu.h"
#include "../../common/phone/nokia/nfunc.h"
#include "../../common/misc/coding.h"

extern GSM_Reply_Function UserReplyFunctions4[];

/* ------- some usefull functions ----------------------------------------- */

GSM_Error CheckDCT4Only()
{
	bool found = false;

	/* Checking if phone is DCT4 */
#ifdef GSM_ENABLE_NOKIA6510
 	if (strstr(N6510Phone.models, s.Phone.Data.ModelInfo->model) != NULL) found = true;
#endif
	if (!found) return GE_NOTSUPPORTED;

	if (s.ConnectionType!=GCT_MBUS2	     && s.ConnectionType!=GCT_FBUS2      &&
	    s.ConnectionType!=GCT_FBUS2DLR3  && s.ConnectionType!=GCT_PHONETBLUE &&
	    s.ConnectionType!=GCT_IRDAPHONET && s.ConnectionType!=GCT_BLUEPHONET)
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

static DCT4_Feature DCT4Features[] = {
	{DCT4_ALS,		 	 "Alternate Line Service (ALS)", {{1,"on"},{0,"off"},{0,""}}},
	{DCT4_A52,		 	 "Ciphering alghoritm A52", 	 {{1,"on"},{0,"off"},{0,""}}},
	{DCT4_CSP,		 	 "Customer Service Profile",	 {{0,"off"},{1,"on"},{0,""}}},
	{DCT4_DISPLAY_PHONE_NAME,	 "Display both number and name for incoming calls",{{1,"on"},{0,"off"},{0,""}}},
	{DCT4_DISPLAY_WAP_PROFILE,	 "Display selected WAP profile name instead of Home option menu in Services",{{1,"on"},{0,"off"},{0,""}}},
	{DCT4_USE_PREF_SIM_NET,		 "Use SIM preffered network list",{{1,"on"},{0,"off"},{0,""}}},
	{DCT4_WAP_PUSH,			 "WAP push",			 {{1,"on"},{0,"off"},{0,""}}},

	{DCT4_GPRS_PCCH,	 	 "PCCH support for GPRS",	 {{1,"on"},{0,"off"},{0,""}}},
	{DCT4_ALWAYS_ONLINE,     	 "GPRS Always Online",		 {{0,"on (Context)"},{1,"off (Attach)"},{0,""}}},///??
	{DCT4_GEA1,			 "GEA1 support indication",	 {{1,"on"},{0,"off"},{0,""}}},
	{DCT4_EOTD,			 "EOTD support",		 {{1,"on"},{0,"off"},{0,""}}},

	{DCT4_GAMES_WAP_DOWNLOAD,	 "Games WAP download",		 {{1,"on"},{0,"off"},{0,""}}},
	{DCT4_GAMES_SCORE_SEND,		 "Games WAP score send",	 {{1,"on"},{0,"off"},{0,""}}},
	{DCT4_GAMES_URL_CHECK,		 "Games URL check",		 {{1,"on"},{0,"off"},{0,""}}},

	{DCT4_BLUETOOTH_MENU,	 	 "Bluetooth menu",		 {{1,"on"},{0,"off"},{0,""}}},
	{DCT4_WAP_BOOKMARKS_MENU,	 "Bookmarks menu in Services", 	 {{1,"on"},{0,"off"},{0,""}}},
	{DCT4_WAP_GOTO_MENU,	 	 "GoTo menu in Services",	 {{0,"on"},{1,"off"},{0,""}}},
	{DCT4_WAP_SETTINGS_MENU,	 "Profiles menu in Services",	 {{0,"on"},{1,"off"},{0,""}}},
	{DCT4_SERVICES_GAMES_APP_GALLERY,"Services menu in Games/Apps/Gallery",{{1,"on"},{0,"off"},{0,""}}},
	{DCT4_JAVA_GAMES_MENU,		 "Java games menu in Games",	 {{1,"on"},{0,"off"},{0,""}}},
	{DCT4_SAT_CONFIRM_MENU,		 "Can use confirming SIM service actions", {{1,"on"},{0,"off"},{0,""}}},

#ifdef DEBUG
	{DCT4_TEST,"",{{1,"1"},{0,"0"}}},
#endif

	{0,			 	 "",				 {{0,""}}}
};

static DCT4_Phone_Features DCT4PhoneFeatures[] = {
/*NHM-8*/ {"3510",	{{DCT4_ALS,1},{DCT4_A52,3},{DCT4_CSP,6},
			 {DCT4_GAMES_WAP_DOWNLOAD,7},{DCT4_GAMES_SCORE_SEND,8},
			 {DCT4_GAMES_URL_CHECK,9},{DCT4_GPRS_PCCH,13},
			 {DCT4_GEA1,15},{DCT4_ALWAYS_ONLINE,18},{0,0}}},
/*RH-9*/  {"3510i",	{{DCT4_ALS,1},{DCT4_A52,3},{DCT4_CSP,4},{DCT4_GPRS_PCCH,9},
 			 {DCT4_DISPLAY_PHONE_NAME,14},{DCT4_WAP_GOTO_MENU,15},
			 {DCT4_WAP_SETTINGS_MENU,16},{DCT4_SERVICES_GAMES_APP_GALLERY,19},
			 {DCT4_DISPLAY_WAP_PROFILE,25},{0,0}}},
/*NPM-6*/ {"5100",	{{DCT4_ALS,1},{DCT4_CSP,4},{DCT4_GAMES_URL_CHECK,5},{DCT4_GPRS_PCCH,8},
			 {DCT4_GEA1,9},{DCT4_ALWAYS_ONLINE,11},{DCT4_EOTD,12},
			 {DCT4_DISPLAY_PHONE_NAME,17},{DCT4_WAP_GOTO_MENU,18},
			 {DCT4_WAP_SETTINGS_MENU,19},{DCT4_SERVICES_GAMES_APP_GALLERY,22},
			 {DCT4_DISPLAY_WAP_PROFILE,26},{DCT4_SAT_CONFIRM_MENU,27},
			 {0,0}}},
/*NPL-2*/ {"6100",	{{DCT4_ALS,1},{DCT4_CSP,4},{DCT4_GAMES_URL_CHECK,5},{DCT4_GPRS_PCCH,8},
			 {DCT4_GEA1,9},{DCT4_ALWAYS_ONLINE,11},{DCT4_EOTD,12},
			 {DCT4_DISPLAY_PHONE_NAME,17},{DCT4_WAP_GOTO_MENU,18},
			 {DCT4_WAP_SETTINGS_MENU,19},{DCT4_SERVICES_GAMES_APP_GALLERY,22},
			 {DCT4_DISPLAY_WAP_PROFILE,26},{DCT4_SAT_CONFIRM_MENU,27},
			 {0,0}}},
/*NPE-4*/ {"6310",	{{DCT4_ALS,1},{DCT4_A52,3},{DCT4_CSP,6},{DCT4_GAMES_WAP_DOWNLOAD,7},
			 {DCT4_GAMES_SCORE_SEND,8},{DCT4_GAMES_URL_CHECK,9},{DCT4_BLUETOOTH_MENU,10},
			 {DCT4_GPRS_PCCH,13},{DCT4_GEA1,15},{DCT4_ALWAYS_ONLINE,18},{0,0}}},
/*NPL-1*/ {"6310i",	{{DCT4_ALS,1},{DCT4_A52,3},{DCT4_CSP,6},{DCT4_GAMES_WAP_DOWNLOAD,7},
			 {DCT4_GAMES_SCORE_SEND,8},{DCT4_GAMES_URL_CHECK,9},
			 {DCT4_BLUETOOTH_MENU,10},{DCT4_USE_PREF_SIM_NET,11},
			 {DCT4_GPRS_PCCH,13},{DCT4_GEA1,15},{DCT4_EOTD,16},
			 {DCT4_ALWAYS_ONLINE,17},{DCT4_JAVA_GAMES_MENU,18},
      			 {DCT4_WAP_BOOKMARKS_MENU,20},{DCT4_WAP_SETTINGS_MENU,21},
			 {DCT4_WAP_PUSH,28},{DCT4_WAP_GOTO_MENU,29},{0,0}}},
/*NPM-9*/ {"6510",	{{DCT4_ALS,1},{DCT4_A52,3},{DCT4_CSP,6},{DCT4_GAMES_WAP_DOWNLOAD,7},
			 {DCT4_GAMES_SCORE_SEND,8},{DCT4_GAMES_URL_CHECK,9},
			 {DCT4_GPRS_PCCH,13},{DCT4_GEA1,15},{DCT4_ALWAYS_ONLINE,18},{0,0}}},
/*NHL-4U*/{"6610",	{{DCT4_ALS,1},{DCT4_CSP,4},{DCT4_GAMES_URL_CHECK,5},{DCT4_GPRS_PCCH,8},
			 {DCT4_GEA1,9},{DCT4_ALWAYS_ONLINE,11},{DCT4_EOTD,12},
			 {DCT4_DISPLAY_PHONE_NAME,17},{DCT4_WAP_GOTO_MENU,18},
			 {DCT4_WAP_SETTINGS_MENU,19},{DCT4_SERVICES_GAMES_APP_GALLERY,22},
			 {DCT4_DISPLAY_WAP_PROFILE,26},{DCT4_SAT_CONFIRM_MENU,27},
			 {0,0}}},
/*NHL-4*/ {"7210",	{{DCT4_ALS,1},{DCT4_CSP,4},{DCT4_GAMES_URL_CHECK,5},{DCT4_GPRS_PCCH,8},
			 {DCT4_GEA1,9},{DCT4_ALWAYS_ONLINE,11},{DCT4_EOTD,12},
			 {DCT4_DISPLAY_PHONE_NAME,17},{DCT4_WAP_GOTO_MENU,18},
			 {DCT4_WAP_SETTINGS_MENU,19},{DCT4_SERVICES_GAMES_APP_GALLERY,22},
			 {DCT4_DISPLAY_WAP_PROFILE,26},{DCT4_SAT_CONFIRM_MENU,27},
			 {0,0}}},
/*NHM-7*/ {"8310",	{{DCT4_ALS,1},{DCT4_CSP,6},{DCT4_GAMES_WAP_DOWNLOAD,7},
			 {DCT4_GAMES_SCORE_SEND,8},{DCT4_GAMES_URL_CHECK,9},{DCT4_GPRS_PCCH,13},
			 {DCT4_ALWAYS_ONLINE,18},{0,0}}},
	{"",		{{0,0}}}
};

static GSM_Error DCT4_ReplySetPPS(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	printf("Setting done OK\n");
	return GE_NONE;
}

void DCT4SetPhoneMenus(int argc, char *argv[])
{
	int 		current = 10,i=0,j,z;
	unsigned char 	reqSet[200] = {
		N7110_FRAME_HEADER,0x04,0x00,0x01,0x47,0x48,0x02,
		0x00};		/* Number of changed features */

	if (CheckDCT4Only()!=GE_NONE) return;

	s.User.UserReplyFunctions=UserReplyFunctions4;

	while (DCT4PhoneFeatures[i].Model[0] != 0x00) {
		if (!strcmp(DCT4PhoneFeatures[i].Model,s.Phone.Data.ModelInfo->model)) {
			j = 0;
			while (DCT4PhoneFeatures[i].Features[j].Name != 0x00) {
				z = 0;
				while (DCT4Features[z].Name != 0x00) {
					if (DCT4Features[z].Name == DCT4PhoneFeatures[i].Features[j].Name) {
						printf("%s : %s\n",DCT4Features[z].Text,DCT4Features[z].Values[0].Text);
						reqSet[9]++;							/* Number of features */
						reqSet[current++] = DCT4PhoneFeatures[i].Features[j].Number; 	/* Feature number */
						reqSet[current++] = DCT4Features[z].Values[0].Value;		/* Value */
						break;
					}
					z++;
				}
				j++;
			}
		}
		i++;
	}

	if (current == 10) {
		printf("Sorry, but configuration matrix for this model is not added yet. Please report\n");
		return;
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

static GSM_Error DCT4_ReplyVibra(GSM_Protocol_Message msg, GSM_StateMachine *s)
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
			my_sleep(10);
			GSM_GetCurrentDateTime(&Date);
		}
	}

	Control[4] = 0x00;
	error=GSM_WaitFor (&s, Control, 6, 0x1C, 4, ID_User3);
	Print_Error(error);

	GSM_Terminate();
}

static bool answer_yes2(char *text)
{
    	int         len;
    	char        ans[99];

	while (1) {
		printf("%s (yes/no) ? ",text);
		len=GetLine(stdin, ans, 99);
		if (len==-1) exit(-1);
		if (mystrncasecmp(ans, "yes",0)) return true;
		if (mystrncasecmp(ans, "no" ,0)) return false;
	}
}

static GSM_Error DCT4_ReplyResetSecurityCode(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	switch (msg.Buffer[3]) {
	case 0x05:
		printf("Security code set to \"12345\"\n");
		return GE_NONE;
	case 0x06:
		printf("Unknown reason. Can't reset your security code\n");
		return GE_UNKNOWN;
	}
	return GE_UNKNOWNRESPONSE;
}

void DCT4ResetSecurityCode(int argc, char *argv[])
{
	unsigned int	i;
	unsigned char 	ResetCode[30] = {0x00,0x06,0x03,0x04,0x01,
		'1','2','3','4','5','6','7','8','9','0',	/* Old code */
		0x00,
		'1','2','3','4','5',0x00,0x00,0x00,0x00,0x00,   /* New code */
		0x00};

	if (CheckDCT4Only()!=GE_NONE) return;

	s.User.UserReplyFunctions=UserReplyFunctions4;

	error=GSM_WaitFor (&s, ResetCode, 27, 0x08, 4, ID_User2);
	if (error == GE_UNKNOWN) {
		if (answer_yes2("Try brutal force ?")) {
			for (i=10000;i<9999999;i++) {
				printf("Trying %i\n",i);
				memset(ResetCode+6,0,22);
				sprintf(ResetCode+5,"%i",i);
				sprintf(ResetCode+16,"12345");
				error=GSM_WaitFor (&s, ResetCode, 27, 0x08, 4, ID_User2);
				if (error == GE_NONE) break;
			}
		}
	} else Print_Error(error);
}

static GSM_Error DCT4_ReplyGetVoiceRecord(GSM_Protocol_Message msg, GSM_StateMachine *s)
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
 		*s->Phone.Data.VoiceRecord = 0;
		while (i<msg.Length) {
 			s->Phone.Data.PhoneString[(*s->Phone.Data.VoiceRecord)++] = msg.Buffer[i+1];
 			s->Phone.Data.PhoneString[(*s->Phone.Data.VoiceRecord)++] = msg.Buffer[i];
			i += 2;
		}
		return GE_NONE;
	case 0x0D:
		dprintf("Last part of voice record is %02x %02x\n",msg.Buffer[11],msg.Buffer[12]);
		dprintf("Token is %02x\n",msg.Buffer[13]);
 		s->Phone.Data.PhoneString[0] = msg.Buffer[11];
 		s->Phone.Data.PhoneString[1] = msg.Buffer[12];
 		s->Phone.Data.PhoneString[2] = msg.Buffer[13];
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
 			if (i==*s->Phone.Data.VoiceRecord) {
 				sprintf(s->Phone.Data.PhoneString,"%s.wav",DecodeUnicodeString(Buffer));
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
	WAV_Header[4] 	= ((unsigned char)wavfilesize % 256);
	WAV_Header[5] 	= ((unsigned char)wavfilesize / 256);
	WAV_Header[6] 	= ((unsigned char)wavfilesize / (256*256));
	WAV_Header[7] 	= ((unsigned char)wavfilesize / (256*256*256));

	/* FIXME */
	FMT_Header[36]	= ((unsigned char)((wavfilesize - 238) * 5 ) % 256);
	FMT_Header[37]	= ((unsigned char)((wavfilesize - 238) * 5 ) / 256);
	FMT_Header[38]	= ((unsigned char)((wavfilesize - 238) * 5 ) / (256*256));
	FMT_Header[39]	= ((unsigned char)((wavfilesize - 238) * 5 ) / (256*256*256));

	wavfilesize 	= ((unsigned char)wavfilesize - 54 - 6);
	DATA_Header[4] 	= ((unsigned char)wavfilesize % 256);
	DATA_Header[5] 	= ((unsigned char)wavfilesize / 256);
	DATA_Header[6] 	= ((unsigned char)wavfilesize / (256*256));
	DATA_Header[7] 	= ((unsigned char)wavfilesize / (256*256*256));

	fseek( WAVFile, 0, SEEK_SET);
	fwrite(&WAV_Header,	1, sizeof(WAV_Header),	WAVFile);
	fwrite(&FMT_Header,	1, sizeof(FMT_Header),	WAVFile);
	fwrite(&DATA_Header,	1, sizeof(DATA_Header),	WAVFile);

	fclose(WAVFile);

	GSM_Terminate();
}

static GSM_Error DCT4_ReplyGetBTInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	printf("device address %02x%02x%02x%02x%02x%02x\n",
		msg.Buffer[9],msg.Buffer[10],msg.Buffer[11],
		msg.Buffer[12],msg.Buffer[13],msg.Buffer[14]);
	return GE_NONE;
}

static GSM_Error DCT4_ReplyGetSimlock(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int i;

	switch (msg.Buffer[3]) {
	case 0x0D:
		dprintf("Simlock info received\n");
		dprintf("Config_Data: ");
		for (i=14;i<22;i++) {
			dprintf("%02x",msg.Buffer[i]);
		}
		dprintf("\n");
		dprintf("Profile_Bits: ");
		for (i=22;i<30;i++) {
			dprintf("%02x",msg.Buffer[i]);
		}
		dprintf("\n");		
		return GE_NONE;
	case 0x13:
		dprintf("Simlock info received\n");
		if (msg.Buffer[58] == 0x05 && msg.Buffer[59] == 0x02) {
			dprintf("SIM_PATH: ");
			for (i=44;i<52;i++) {
				dprintf("%02x",msg.Buffer[i]);
			}
			dprintf("\n");
			printf("Simlock data  : ");
			for (i=60;i<63;i++) {
				printf("%02x",msg.Buffer[i]);
			}
			printf("\n");
		}
		return GE_NONE;
	}
	return GE_UNKNOWNRESPONSE;
}

void DCT4Info(int argc, char *argv[])
{
	unsigned char GetBTAddress[8] = {N6110_FRAME_HEADER, 0x09, 0x19, 0x01, 0x03, 0x06};
	unsigned char GetSimlock[5] = {N6110_FRAME_HEADER, 0x12, 0x0D};
	unsigned char value[10];

        if (CheckDCT4Only()!=GE_NONE) return;

	s.User.UserReplyFunctions=UserReplyFunctions4;

	if (IsPhoneFeatureAvailable(s.Phone.Data.ModelInfo, F_BLUETOOTH)) {
		printf("Bluetooth     : ");

		error=GSM_WaitFor (&s, GetBTAddress, 8, 0xD7, 4, ID_User6);
		Print_Error(error);
	}

	error=GSM_WaitFor (&s, GetSimlock, 5, 0x53, 4, ID_User6);
	Print_Error(error);
	GetSimlock[4] = 0x0E;
	error=GSM_WaitFor (&s, GetSimlock, 5, 0x53, 4, ID_User6);
	Print_Error(error);
	GetSimlock[3] = 0x0C;
	error=GSM_WaitFor (&s, GetSimlock, 4, 0x53, 4, ID_User6);
	Print_Error(error);
	error=NOKIA_GetPhoneString(&s,"\x00\x03\x02\x07\x00\x08",6,0x1b,value,ID_User6,10);
	Print_Error(error);
	printf("UEM           : %s\n",value);
}

static FILE 	*T9File;
int 		T9Size;
int 		T9FullSize;

static GSM_Error DCT4_ReplyGetT9(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	T9FullSize 	= msg.Buffer[18] * 256 + msg.Buffer[19];
	T9Size 		= msg.Length - 18;
	fwrite(msg.Buffer+18,1,T9Size,T9File);
	return GE_NONE;
}

void DCT4GetT9(int argc, char *argv[])
{
	int	      i,T9Dictionary=0;
	unsigned char req[] = {N7110_FRAME_HEADER, 0x04, 0x00, 0x5B,
			       0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			       0x00, 0x00,	/* Start position */
			       0x00, 0x00,
			       0x02, 0xBC};	/* How many bytes to read */

	T9File = fopen("T9", "w");      
	if (T9File == NULL) return;

	GSM_Init(true);

        CheckDCT4();

	s.User.UserReplyFunctions=UserReplyFunctions4;

	i = 0;
	while (1) {
		req[12] = i / 256;
		req[13] = i % 256;
		if (i != 0) {
			if (T9Dictionary - i < req[16]*256+req[17]) {
				req[16] = (T9Dictionary - i) / 256;
				req[17] = (T9Dictionary - i) % 256;
			}
			if (T9Dictionary - i == 0) break;
		}
		error=GSM_WaitFor (&s, req, 18, 0x23, 4, ID_User3);		
		Print_Error(error);
		if (i==0) {
			T9Dictionary = T9FullSize;
			dprintf("T9 dictionary size is %i\n",T9Dictionary);
		}
		i+=T9Size;
	}

	GSM_Terminate();
	fclose(T9File);
}

static GSM_Reply_Function UserReplyFunctions4[] = {

	{DCT4_ReplyResetSecurityCode,	"\x08",0x03,0x05,ID_User2	},
	{DCT4_ReplyResetSecurityCode,	"\x08",0x03,0x06,ID_User2	},

	{DCT4_ReplySetPPS,		"\x1b",0x03,0x05,ID_User1	},
	{NOKIA_ReplyGetPhoneString,	"\x1B",0x03,0x08,ID_User6	},

	{DCT4_ReplyVibra,		"\x1C",0x03,0x0D,ID_User3	},
	{DCT4_ReplyVibra,		"\x1C",0x03,0x0F,ID_User3	},

	{DCT4_ReplyGetT9,		"\x23",0x03,0x05,ID_User3	},
	{DCT4_ReplyGetVoiceRecord,	"\x23",0x03,0x05,ID_User4	},
	{DCT4_ReplyGetVoiceRecord,	"\x23",0x03,0x0D,ID_User4	},

	{DCT4_ReplyGetVoiceRecord,	"\x4A",0x03,0x31,ID_User4	},

	{DCT4_ReplyGetSimlock,		"\x53",0x03,0x0D,ID_User6	},
	{DCT4_ReplyGetSimlock,		"\x53",0x03,0x13,ID_User6	},

	{DCT4_ReplyGetBTInfo,		"\xD7",0x03,0x0A,ID_User6	},

	{NULL,				"\x00",0x00,0x00,ID_None	}
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
