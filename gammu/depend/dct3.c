
#include "../../common/gsmstate.h"

#ifdef GSM_ENABLE_NOKIA_DCT3

#include <string.h>
#include <signal.h>

#include "../../common/misc/coding.h"
#include "../../common/gsmcomon.h"
#include "../../common/service/gsmpbk.h"
#include "../../common/phone/nokia/dct3/dct3func.h"
#include "../gammu.h"

extern GSM_Reply_Function UserReplyFunctions3[];

/* ------- some usefull functions ----------------------------------------- */

GSM_Error CheckDCT3Only()
{
	bool found = false;

	/* Checking if phone is DCT3 */
#ifdef GSM_ENABLE_NOKIA6110
	if (strstr(N6110Phone.models, s.Phone.Data.ModelInfo->model) != NULL) found = true;
#endif
#ifdef GSM_ENABLE_NOKIA7110
	if (strstr(N7110Phone.models, s.Phone.Data.ModelInfo->model) != NULL) found = true;
#endif
#ifdef GSM_ENABLE_NOKIA9210
	if (strstr(N9210Phone.models, s.Phone.Data.ModelInfo->model) != NULL) found = true;
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

static void CheckDCT3()
{
	GSM_Error error;

	error = CheckDCT3Only();
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

static GSM_Error DCT3_ReplyPhoneTests(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int i;

	for (i=0;i<msg.Buffer[3];i++) {
		switch (i) {
		case 0: printf("Unknown(%02i)             ",i);	break;
		case 1: printf("MCU ROM checksum        ");	break;
		case 2: printf("MCU RAM interface       ");	break;
		case 3: printf("MCU RAM component       ");	break;
		case 4: printf("MCU EEPROM interface    ");	break;
		case 5: printf("MCU EEPROM component    ");	break;
		case 6: printf("Real Time Clock battery ");	break;
		case 7: printf("CCONT interface         ");	break;
		case 8: printf("AD converter            ");	break;
		case 9: printf("SW Reset                ");	break;
		case 10:printf("Power Off               ");	break;
		case 11:printf("Security Data           ");	break;
		case 12:printf("EEPROM Tune checksum    ");	break;
		case 13:printf("PPM checksum            ");	break;
		case 14:printf("MCU download DSP        ");	break;
		case 15:printf("DSP alive               ");	break;
		case 16:printf("COBBA serial            ");	break;
		case 17:printf("COBBA paraller          ");	break;
		case 18:printf("EEPROM security checksum");	break;
		case 19:printf("PPM validity            ");	break;
		case 20:printf("Warranty state          ");	break;
		case 21:printf("Simlock check           ");	break;
		case 22:printf("IMEI check?             ");	break;/*from PC-Locals1.3.is OK?*/
		default:printf("Unknown(%02i)             ",i);	break;
		}
		switch (msg.Buffer[4+i]) {
		case 0:   printf(" : done, result OK");				break;
		case 0xff:printf(" : not done, result unknown");		break;
		case 254: printf(" : done, result NOT OK");			break;
		default:  printf(" : result unknown(%i)",msg.Buffer[4+i]);	break;
    		}
		printf("\n");
	}

	return GE_NONE;
}

void DCT3tests(int argc, char *argv[])
{
	unsigned char 	buffer[3]  = {0x00,0x01,0xcf};
	unsigned char 	buffer3[8] = {0x00,0x01,0xce,0x1d,0xfe,0x23,0x00,0x00};
	int		i;

	if (CheckDCT3Only()!=GE_NONE) return;

	error=DCT3_EnableSecurity (&s, 0x01);
	Print_Error(error);

	/* make almost all tests */
	error = s.Protocol.Functions->WriteMessage(&s, buffer3, 8, 0x40);
	Print_Error(error);
	
	GSM_Terminate();

	while (!false) {
		GSM_Init(false);
		if (error==GE_NONE) break;
		GSM_Terminate();
	}

	my_sleep(400);

	s.User.UserReplyFunctions=UserReplyFunctions3;

	for (i=0;i<10;i++) {
		error=GSM_WaitFor (&s, buffer, 3, 0x40, 4, ID_User1);	
		if (error == GE_NONE) break;
	}
}

void DCT3netmonitor(int argc, char *argv[])
{
	char value[100];

	GSM_Init(true);

        CheckDCT3();

	error=DCT3_Netmonitor(&s, atoi(argv[2]), value);
	Print_Error(error);

	printf("%s\n",value);
#ifdef GSM_ENABLE_BEEP
if (atoi(argv[2]) == 243) GSM_PhoneBeep();
#endif
	GSM_Terminate();
}

static GSM_Error DCT3_ReplyGetMSID(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int i;

	printf("MSID          : ");
	for (i=5;i<18;i++) printf("%02x",msg.Buffer[i]);
	printf("\n");
	return GE_NONE;
}

static GSM_Error DCT3_ReplyGetDSPROM(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	printf("DSP ROM       : %c\n",msg.Buffer[5]);
	return GE_NONE;
}

static GSM_Error DCT3_ReplySimlockInfo(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int	i, j;  
	char	uni[100];
	char	buffer[50];    

	j=0;
	for (i=0; i < 12; i++)
	{
		if (j<24) {
			uni[j]='0' + (msg.Buffer[9+i] >> 4);
			j++;
		}
		if (j!=15) {
			if (j<24) {
				uni[j]='0' + (msg.Buffer[9+i] & 0x0f);
				j++;
			}
		} else j++;
	}

	strncpy(buffer,uni,5);
	buffer[5]=0;
	printf("Simlock 1     : MCC+MNC %10s, %s, %s, counter %i\n",
		buffer,
		((msg.Buffer[6] & 1) == 1)==0?"opened":"CLOSED",
		((msg.Buffer[5] & 1) != 1)==0?"user   ":"factory",
		msg.Buffer[21]);

	strncpy(buffer,uni+16,4);
	buffer[4]=0;
	printf("Simlock 2     : GID1    %10s, %s, %s, counter %i\n",
		buffer,
		((msg.Buffer[6] & 4) == 4)==0?"opened":"CLOSED",
		((msg.Buffer[5] & 4) != 4)==0?"user   ":"factory",
		msg.Buffer[23]);

	strncpy(buffer,uni+20,4);
	buffer[4]=0;
	printf("Simlock 3     : GID2    %10s, %s, %s, counter %i\n",
		buffer,
		((msg.Buffer[6] & 8) == 8)==0?"opened":"CLOSED",
		((msg.Buffer[5] & 8) != 8)==0?"user   ":"factory",
		msg.Buffer[24]);

	strncpy(buffer,uni+5,10);
	buffer[10]=0;
	printf("Simlock 4     : MSIN    %10s, %s, %s, counter %i\n",
		buffer,
		((msg.Buffer[6] & 2) == 2)==0?"opened":"CLOSED",
		((msg.Buffer[5] & 2) != 2)==0?"user   ":"factory",
		msg.Buffer[22]);

	return GE_NONE;
}

static GSM_Error DCT3_ReplyGetMCUchkSum(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int i;
	
	if (msg.Buffer[3] == 0x12) printf("Language Pack: %c\n",msg.Buffer[5]);
	if (msg.Buffer[3] == 0x02) 
	{
		printf("MCU checksum  : ");
		for (i=5;i<9;i++) printf("%c",msg.Buffer[i]);
		printf("\n");
	}
	return GE_NONE;
}

void DCT3Info(int argc, char *argv[])
{
	unsigned char 		req[]  	= {0x00, 0x01, 0x8A, 0x00}; 	  /* Get simlock info */
	unsigned char 		req2[]  = {0x00, 0x01, 0xb4, 0x00, 0x00}; /* Get MSID */
	unsigned char 		req3[]  = {0x00, 0x01, 0xc8, 0x02}; 	  /* Get MCU chksum */
	unsigned char 		req4[]  = {0x00, 0x01, 0xc8, 0x09}; 	  /* Get DSP ROM */
	GSM_Protocol_Message 	msg;	

        if (CheckDCT3Only()!=GE_NONE) return;

	error=DCT3_EnableSecurity (&s, 0x01);
	Print_Error(error);

	msg = *(s.Phone.Data.RequestMsg);
	req2[3] = msg.Buffer[5];
	req2[4] = req2[2] + req2[3];

	s.User.UserReplyFunctions=UserReplyFunctions3;

	error=GSM_WaitFor (&s, req, 4, 0x40, 4, ID_User3);
	Print_Error(error);

 	error=GSM_WaitFor (&s, req2, 5, 0x40, 4, ID_User8);
	Print_Error(error);

	error=GSM_WaitFor (&s, req3, 4, 0x40, 4, ID_User9);
	Print_Error(error);

	error=GSM_WaitFor (&s, req4, 4, 0x40, 4, ID_User10);
	Print_Error(error);
}

static GSM_Error DCT3_ReplyResetTest36(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	printf("Netmonitor test 36 cleaned OK\n");
	return GE_NONE;
}

void DCT3ResetTest36(int argc, char *argv[])
{
	unsigned char req[]  = {0x00, 0x01, 0x65, 0x40, 0x00}; /* Reset test 36 in netmon */

	GSM_Init(true);

        CheckDCT3();

	error=DCT3_EnableSecurity (&s, 0x01);
	Print_Error(error);

	s.User.UserReplyFunctions=UserReplyFunctions3;

	error=GSM_WaitFor (&s, req, 5, 0x40, 4, ID_User2);
	Print_Error(error);

#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif
	GSM_Terminate();
}

static unsigned char PPS[32]; /* Product Profile Settings */

static GSM_Error DCT3_ReplyGetPPS(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	int i,j,z;

#ifdef DEBUG
	dprintf("Product Profile Settings received -");
	for (i=0;i<4;i++) dprintf(" %02x",msg.Buffer[3+i]);
	dprintf("\n");
#endif
	j=128;z=0;
	for (i=0;i<32;i++) {
		PPS[i]='0';
		if (msg.Buffer[z+3]&j) PPS[i]='1';
		if (j==1) {
			j=128;
			z++;
		} else j=j/2;
	}
#ifdef DEBUG
	dprintf("After decoding: ");
	for (i=0;i<32;i++) dprintf("%c",PPS[i]);
	dprintf("\n");
#endif
	return GE_NONE;
}

static GSM_Error DCT3_ReplySetPPS(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	printf("Setting done OK\n");
	return GE_NONE;
}

void DCT3SetPhoneMenus(int argc, char *argv[])
{
	char 		value[100];
	int 		i,j,z;
	unsigned char 	reqGet[] = {0x00, 0x01, 0x6a};
	unsigned char 	reqSet[] = {
		0x00, 0x01, 0x6b,
		0x00, 0x00, 0x00, 0x00 }; /* bytes with Product Profile Setings */

	if (CheckDCT3Only()!=GE_NONE) return;

	error=DCT3_EnableSecurity (&s, 0x01);
	Print_Error(error);

	s.User.UserReplyFunctions=UserReplyFunctions3;

	error=GSM_WaitFor (&s, reqGet, 3, 0x40, 4, ID_User4);
	Print_Error(error);

	printf("ALS : enabling menu\n");
	PPS[10] = '1';

	if (!strcmp(s.Phone.Data.ModelInfo->model,"6150")) {
		printf("6150: enabling WellMate menu\n");
		PPS[18] = '1';
	}
	/* FIXME */
	if (!strcmp(s.Phone.Data.ModelInfo->model,"3210")) {
		printf("3210: enabling vibra menu\n");
		PPS[24] = '1';
	}
	if (!strcmp(s.Phone.Data.ModelInfo->model,"3310") && s.Phone.Data.VerNum>5.13) {
		printf("3310: enabling 3315 features\n");
		PPS[25] = '1';
	}
	/* FIXME */
	if (!strcmp(s.Phone.Data.ModelInfo->model,"3210") && s.Phone.Data.VerNum>=5.31) {
		printf("3210: enabling React and Logic game\n");
		PPS[26] = '1';		
	}

#ifdef DEBUG
	dprintf("After settings: ");
	for (i=0;i<32;i++) dprintf("%c",PPS[i]);
	dprintf("\n");
#endif

	j=128;z=0;
	for (i=0;i<32;i++) {
		if (PPS[i]=='1') reqSet[z+3]=reqSet[z+3]+j;
		if (j==1) {
			j=128;
			z++;
		} else j=j/2;
	} 

//	reqSet[3]=0xe7;
//	reqSet[4]=0x25;
//	reqSet[5]=0x00;
//	reqSet[6]=0xe0;

	error=GSM_WaitFor (&s, reqSet, 7, 0x40, 4, ID_User4);
	Print_Error(error);

	printf("Enabling netmonitor\n");
	error=DCT3_Netmonitor(&s, 243, value);
	Print_Error(error);
}

static GSM_Error DCT3_Reply61GetSecurityCode(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	printf("Security Code is \"%s\"\n",msg.Buffer+5);
	return GE_NONE;
}

static GSM_Error DCT3_Reply7191GetSecurityCode(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	printf("Security Code is \"%s\"\n",msg.Buffer+6);
	return GE_NONE;
}

void DCT3GetSecurityCode(int argc, char *argv[])
{
	unsigned char req6110[] = {
		0x00, 0x01, 0x6e,
		0x01};			/* Type of the requested code. */
	unsigned char req71_91[] = {
		N7110_FRAME_HEADER, 0xee,
		0x1c};			/* Setting */

	if (CheckDCT3Only()!=GE_NONE) return;

	error=DCT3_EnableSecurity (&s, 0x01);
	Print_Error(error);

	s.User.UserReplyFunctions=UserReplyFunctions3;

#ifdef GSM_ENABLE_NOKIA6110
	if (strstr(N6110Phone.models, s.Phone.Data.ModelInfo->model) != NULL)
	{
		error=GSM_WaitFor (&s, req6110, 4, 0x40, 4, ID_User6);
	}
#endif
#ifdef GSM_ENABLE_NOKIA7110
	if (strstr(N7110Phone.models, s.Phone.Data.ModelInfo->model) != NULL)
	{
		error=GSM_WaitFor (&s, req71_91, 5, 0x7a, 4, ID_User6);
	}
#endif
#ifdef GSM_ENABLE_NOKIA9210
	if (strstr(N9210Phone.models, s.Phone.Data.ModelInfo->model) != NULL)
	{
		error=GSM_WaitFor (&s, req71_91, 5, 0x7a, 4, ID_User6);
	}
#endif
	Print_Error(error);
}

#ifdef GSM_ENABLE_NOKIA6110

static GSM_Error DCT3_ReplyGetOperatorName(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char buffer[10];

	NOKIA_DecodeNetworkCode(msg.Buffer+5, buffer);
	buffer[6] = 0;
	printf("Network           : %s (%s ",	buffer,DecodeUnicodeString(GSM_GetNetworkName(buffer)));
	printf(", %s)\n",			DecodeUnicodeString(GSM_GetCountryName(buffer)));
	printf("Name              : \"%s\"\n",msg.Buffer+8);

	return GE_NONE;
}

void DCT3GetOperatorName(int argc, char *argv[])
{
	unsigned char req[] = {0x00,0x01,0x8c,0x00};

	GSM_Init(true);

	if (strstr(N6110Phone.models, s.Phone.Data.ModelInfo->model) == NULL) Print_Error(GE_NOTSUPPORTED);
	CheckDCT3();

	error=DCT3_EnableSecurity (&s, 0x01);
	Print_Error(error);

	s.User.UserReplyFunctions=UserReplyFunctions3;

	error=GSM_WaitFor (&s, req, 4, 0x40, 4, ID_User5);
	Print_Error(error);

	GSM_Terminate();
}

static GSM_Error DCT3_ReplySetOperatorName(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	printf("Operator name set OK\n");
	return GE_NONE;
}

void DCT3SetOperatorName(int argc, char *argv[])
{
	int 		i = 0;
	unsigned char 	req[256] = {
		0x00,0x01,0x8b,0x00,
		0x00,0x00, /* MCC */
		0x00};     /* MNC */

	GSM_Init(true);

	if (strstr(N6110Phone.models, s.Phone.Data.ModelInfo->model) == NULL) Print_Error(GE_NOTSUPPORTED);
	CheckDCT3();

	error=DCT3_EnableSecurity (&s, 0x01);
	Print_Error(error);

	s.User.UserReplyFunctions=UserReplyFunctions3;

	switch (argc) {
	case 2:
	case 3:	NOKIA_EncodeNetworkCode(req+4,"000 00");
		req[7] = 0x00;
		i = 1;
		break;
	case 4: NOKIA_EncodeNetworkCode(req+4,argv[2]);
		strncpy(req+7,argv[3],200);
		i = strlen(argv[3]);
	}
    
	error=GSM_WaitFor (&s, req, 8+i, 0x40, 4, ID_User7);
	Print_Error(error);

	GSM_Terminate();
}

static GSM_Error DCT3_ReplyDisplayOutput(GSM_Protocol_Message msg, GSM_StateMachine *s)
{
	unsigned char buf[100];

	switch (msg.Buffer[3]) {
	case 0x50:
		dprintf("Display string received\n");
		memcpy(buf,msg.Buffer+8,msg.Buffer[7]*2);
		buf[msg.Buffer[7]*2]   = 0;
		buf[msg.Buffer[7]*2+1] = 0;
		printf("X=%i, Y=%i, Text=\"%s\"\n",msg.Buffer[6],msg.Buffer[5],DecodeUnicodeString(buf));
		return GE_NONE;
	case 0x54:
		dprintf("Display output set\n");
		return GE_NONE;
	}
	return GE_UNKNOWNRESPONSE;
}

void DCT3DisplayOutput(int argc, char *argv[])
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x53,
			       0x01}; //1 = enable, 2 = disable

	GSM_Init(true);

	if (strstr(N6110Phone.models, s.Phone.Data.ModelInfo->model) == NULL) Print_Error(GE_NOTSUPPORTED);
	CheckDCT3();

	s.User.UserReplyFunctions=UserReplyFunctions3;

	error=GSM_WaitFor (&s, req, 5, 0x0d, 4, ID_User7);
	Print_Error(error);

	/* We do not want to monitor serial line forever -
	 * press Ctrl+C to stop the monitoring mode.
	 */
	signal(SIGINT, interrupted);
	printf("If you want break, press Ctrl+C...\n");
	printf("Entering monitor mode...\n\n");

	/* Loop here indefinitely -
	 * allows you to see messages from GSM code in
	 * response to unknown messages etc.
	 * The loops ends after pressing the Ctrl+C.
	 */
	while (!bshutdown) {
		GSM_ReadDevice(&s,true);
		my_sleep(10);
	}

	req[4] = 0x02;
	error=GSM_WaitFor (&s, req, 5, 0x0d, 4, ID_User7);
	Print_Error(error);

	GSM_Terminate();
}
#endif

static GSM_Reply_Function UserReplyFunctions3[] = {
	{DCT3_ReplyDisplayOutput,	"\x0D",0x03,0x50,ID_IncomingFrame},
	{DCT3_ReplyDisplayOutput,	"\x0D",0x03,0x54,ID_User7	 },

	{DCT3_ReplyResetTest36,		"\x40",0x02,0x65,ID_User2 	 },
	{DCT3_ReplyGetPPS,		"\x40",0x02,0x6A,ID_User4 	 },
	{DCT3_ReplySetPPS,		"\x40",0x02,0x6B,ID_User4 	 },
	{DCT3_Reply61GetSecurityCode,	"\x40",0x02,0x6E,ID_User6 	 },
	{DCT3_ReplySimlockInfo,		"\x40",0x02,0x8A,ID_User3	 },
#ifdef GSM_ENABLE_NOKIA6110
	{DCT3_ReplySetOperatorName,	"\x40",0x02,0x8B,ID_User7	 },
	{DCT3_ReplyGetOperatorName,	"\x40",0x02,0x8C,ID_User5	 },
#endif
	{DCT3_ReplyGetMSID,		"\x40",0x02,0xb5,ID_User8	 },
	{DCT3_ReplyGetDSPROM,		"\x40",0x02,0xC8,ID_User10	 },
	{DCT3_ReplyGetMCUchkSum,	"\x40",0x02,0xC8,ID_User9	 },
	{DCT3_ReplyPhoneTests,		"\x40",0x02,0xCF,ID_User1	 },

	{DCT3_Reply7191GetSecurityCode,	"\x7a",0x04,0x1C,ID_User6	 },

	{NULL,				"\x00",0x00,0x00,ID_None	 }
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
