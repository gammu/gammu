/* (c) 2002-2004 by Marcin Wiacek */
/* MSID by Walek */

#include <gammu.h>

#ifdef GSM_ENABLE_NOKIA_DCT3

#include <string.h>
#include <signal.h>
#include <stdlib.h>

#include "../../../libgammu/gsmstate.h"
#include "../../../libgammu/phone/nokia/dct3/dct3func.h"
#include "../../../libgammu/phone/pfunc.h"
#include "../../../libgammu/service/gsmnet.h"
#include "../../gammu.h"
#include "../../../helper/string.h"

extern GSM_Reply_Function UserReplyFunctions3[];

/* ------- some usefull functions ----------------------------------------- */

GSM_Error CheckDCT3Only(void)
{
	gboolean found = FALSE;

/* Checking if phone is DCT3 */
#ifdef GSM_ENABLE_NOKIA6110
	if (strstr(N6110Phone.models, gsm->Phone.Data.ModelInfo->model) != NULL) found = TRUE;
#endif
#ifdef GSM_ENABLE_NOKIA7110
	if (strstr(N7110Phone.models, gsm->Phone.Data.ModelInfo->model) != NULL) found = TRUE;
#endif
#ifdef GSM_ENABLE_NOKIA9210
	if (strstr(N9210Phone.models, gsm->Phone.Data.ModelInfo->model) != NULL) found = TRUE;
#endif
	if (!found) return ERR_NOTSUPPORTED;

	if (gsm->ConnectionType!=GCT_MBUS2	    && gsm->ConnectionType!=GCT_FBUS2      &&
	    gsm->ConnectionType!=GCT_FBUS2DLR3 && gsm->ConnectionType!=GCT_FBUS2BLUE  &&
	    gsm->ConnectionType!=GCT_FBUS2IRDA && gsm->ConnectionType!=GCT_IRDAPHONET &&
	    gsm->ConnectionType!=GCT_BLUEFBUS2) {
		return ERR_OTHERCONNECTIONREQUIRED;
	}
	return ERR_NONE;
}

void CheckDCT3(void)
{
	GSM_Error error;

	error = CheckDCT3Only();
	switch (error) {
		case ERR_OTHERCONNECTIONREQUIRED:
			printf("Can't do it with current phone protocol\n");
			GSM_TerminateConnection(gsm);
			Terminate(3);
		case ERR_NONE:
			break;
		default:
			Print_Error(error);
			break;
	}
}

static gboolean answer_yes3(const char *text)
{
    	int         len;
    	char        ans[99];

	while (1) {
		printf("%s (yes/no) ? ",text);
		len=GetLine(stdin, ans, 99);
		if (len==-1) Terminate(3);
		if (strcasecmp(ans, "yes") == 0) return TRUE;
		if (strcasecmp(ans, "no" ) == 0) return FALSE;
	}
}

/* ------------------- functions ------------------------------------------- */

static FILE *DCT3T9File;

static GSM_Error DCT3_ReplyGetT9(GSM_Protocol_Message msg, GSM_StateMachine *sm UNUSED)
{
	size_t DCT3T9Size;
	size_t written;

	DCT3T9Size = msg.Length - 6;
	written = fwrite(msg.Buffer+6,1,DCT3T9Size,DCT3T9File);
	if (written == DCT3T9Size) return ERR_NONE;
	return ERR_UNKNOWN;
}

void DCT3GetT9(int argc, char *argv[])
{
	int	      i;
	GSM_Error error;
	unsigned char req[] = {0x00, 0x01, 0xAE, 0x02, 0x00,
			       0x00};	/* Part number */

/* "00 01 AE 00" gets some control values */

	CheckDCT3();

	DCT3T9File = fopen("T9", "w");
	if (DCT3T9File == NULL) return;

	gsm->User.UserReplyFunctions=UserReplyFunctions3;

	for (i=0;i<5;i++) {
		req[5] = i;
		error=GSM_WaitFor (gsm, req, 6, 0x40, 4, ID_User3);
		Print_Error(error);
	}

	fclose(DCT3T9File);
}

void DCT3VibraTest(int argc, char *argv[])
{
	unsigned char ans[200];
	unsigned char SetLevel[4] = {0x00, 0x01, 0xA3,
				     0xff};	/* Level */
	GSM_Error error;

	CheckDCT3();

	gsm->User.UserReplyFunctions=UserReplyFunctions3;

	error=DCT3_EnableSecurity (gsm, 0x01);
	Print_Error(error);

	error=GSM_WaitFor (gsm, SetLevel, 4, 0x40, 4, ID_User3);
	Print_Error(error);

	printf("Press any key to continue...\n");
	GetLine(stdin, ans, 99);

	SetLevel[3] = 0x00;
	error=GSM_WaitFor (gsm, SetLevel, 4, 0x40, 4, ID_User3);
}

static GSM_Error DCT3_ReplyPhoneTests(GSM_Protocol_Message msg, GSM_StateMachine *sm UNUSED)
{
	int i;

	for (i=0;i<msg.Buffer[3];i++) {
		switch (i) {
		case 0: printf("Unknown(%02i)                       ",i);break;
		case 1: printf("MCU ROM checksum         (startup)");	break;
		case 2: printf("MCU RAM interface        (startup)");	break;
		case 3: printf("MCU RAM component                 ");	break;
		case 4: printf("MCU EEPROM interface     (startup)");	break;
		case 5: printf("MCU EEPROM component              ");	break;
		case 6: printf("Real Time Clock battery  (startup)");	break;
		case 7: printf("CCONT interface          (startup)");	break;
		case 8: printf("AD converter             (startup)");	break;
		case 9: printf("SW Reset                          ");	break;
		case 10:printf("Power Off                         ");	break;
		case 11:printf("Security Data                     ");	break;
		case 12:printf("EEPROM Tune checksum     (startup)");	break;
		case 13:printf("PPM checksum             (startup)");	break;
		case 14:printf("MCU download DSP         (startup)");	break;
		case 15:printf("DSP alive                (startup)");	break;
		case 16:printf("COBBA serial             (startup)");	break;
		case 17:printf("COBBA paraller           (startup)");	break;
		case 18:printf("EEPROM security checksum (startup)");	break;
		case 19:printf("PPM validity             (startup)");	break;
		case 20:printf("Warranty state           (startup)");	break;
		case 21:printf("Simlock check/SW version (startup)");	break;
		case 22:printf("IMEI check?                       ");	break;/*from PC-Locals1.3.is OK?*/
		default:printf("Unknown(%02i)                       ",i);break;
		}
		switch (msg.Buffer[4+i]) {
		case 0:   printf(" : passed");					break;
		case 0xff:printf(" : not executed");				break;
		case 254: printf(" : fail");					break;
		default:  printf(" : result unknown(%i)",msg.Buffer[4+i]);	break;
    		}
		printf("\n");
	}

	return ERR_NONE;
}

void DCT3SelfTests(int argc, char *argv[])
{
	unsigned char 	buffer[3]  = {0x00,0x01,0xcf};
	unsigned char 	buffer3[8] = {0x00,0x01,0xce,0x1d,0xfe,0x23,0x00,0x00};
	int		i;
	GSM_Error error;

	CheckDCT3();

	error=DCT3_EnableSecurity (gsm, 0x01);
	Print_Error(error);

	if (answer_yes3("Run all tests now ?")) {
		/* make almost all tests */
		error = gsm->Protocol.Functions->WriteMessage(gsm, buffer3, 8, 0x40);
		Print_Error(error);

		GSM_Terminate();

		while (!FALSE) {
			GSM_Init(FALSE);
			if (error==ERR_NONE) break;
			GSM_Terminate();
		}

		usleep(400000);
	}

	gsm->User.UserReplyFunctions=UserReplyFunctions3;

	for (i=0;i<10;i++) {
		error=GSM_WaitFor (gsm, buffer, 3, 0x40, 4, ID_User1);
		if (error == ERR_NONE) break;
	}
}

struct DCT3ADCInfo {
	const char 	*name;
	const char 	*unit;
	int 	x;
	int	pos1;
	int	pos2;
};

static struct DCT3ADCInfo DCT3ADC[] = {
	{"Battery voltage:",			"mV",  1,  3,  2},
/* 	{"Charger voltage:",			"mV",  1, -1,  7}, */
/* 	{"Charger current:",			"mA",  1, -1,  5}, */
	{"Battery type:",			"mAh", 1,  4,  3},
	{"Battery temperature:",		"mK", 10,  5,  4},
/* 	{"Accessory detection:",		"mV",  1, -1, -1}, */
	{"RSSI:",				"",    1,  2, -1},
/* 	{"VCXO temperature:",			"mV",  1, -1, -1}, */
/* 	{"Hook information:",			"mV",  1, -1, -1}, */

	{"", "", 1, -1, -1}
};

unsigned char 	DCT3ADCBuf[200];
int		DCT3ADCInt;

static GSM_Error DCT3_ReplyGetADC(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	switch (msg.Buffer[2]) {
	case 0x68:
		memcpy(DCT3ADCBuf,msg.Buffer+4,msg.Length-4);
		return ERR_NONE;
	case 0x91:
		DCT3ADCInt = msg.Buffer[4]*256+msg.Buffer[5];
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

void DCT3GetADC(int argc, char *argv[])
{
	int		i = 0;
	GSM_Error error;
	unsigned char	GetRaw[] = {0x00, 0x01, 0x68};
	unsigned char	GetUnit[] = {0x00, 0x01, 0x91,
				     0x02};		/* Test number */

	CheckDCT3();

	gsm->User.UserReplyFunctions=UserReplyFunctions3;

	error=DCT3_EnableSecurity (gsm, 0x02);
	Print_Error(error);

	error=GSM_WaitFor (gsm, GetRaw, 3, 0x40, 6, ID_User3);
	Print_Error(error);

	while (1) {
		printf(" %30s ",DCT3ADC[i].name);
		if (DCT3ADC[i].pos1 != -1) {
			printf("raw ");
			printf("%10i ",
				DCT3ADCBuf[(DCT3ADC[i].pos1-1)*2]*256+
				DCT3ADCBuf[(DCT3ADC[i].pos1-1)*2+1]);
		}
		if (DCT3ADC[i].pos2 != -1) {
			printf("unit result ");
			GetUnit[3] = DCT3ADC[i].pos2;
			error=GSM_WaitFor (gsm, GetUnit, 6, 0x40, 4, ID_User3);
			Print_Error(error);
			printf("%10i ",DCT3ADCInt*DCT3ADC[i].x);
			printf("%s\n",DCT3ADC[i].unit);
		}
		i++;
		if (DCT3ADC[i].name[0] == 0x00) break;
	}

	error=DCT3_EnableSecurity (gsm, 0x01);
	Print_Error(error);
}

void DCT3DisplayTest(int argc, char *argv[])
{
	unsigned char ans[200];
	GSM_Error error;
	unsigned char req[] =  {0x00, 0x01, 0xD3,
				0x03,          	/* 3=set, 2=clear */
				0x03};	 	/* test number */

	CheckDCT3();

	if (atoi(argv[2]) != 1 && atoi(argv[2]) != 2) {
		printf("Give 1 or 2 as test number\n");
	}

	gsm->User.UserReplyFunctions=UserReplyFunctions3;

	req[4] = atoi(argv[2]);
	gsm->Protocol.Functions->WriteMessage(gsm, req, 5, 0x40);

	printf("Press any key to continue...\n");
	GetLine(stdin, ans, 99);

	req[3] = 0x02;
	req[4] = 0x03;
	gsm->Protocol.Functions->WriteMessage(gsm, req, 5, 0x40);

	error=DCT3_EnableSecurity (gsm, 0x03);
	Print_Error(error);
}

void DCT3netmonitor(int argc, char *argv[])
{
	GSM_Error error;
	char value[100];

	GSM_Init(TRUE);

        CheckDCT3();

	error=DCT3_Netmonitor(gsm, atoi(argv[2]), value);
	Print_Error(error);

	printf("%s\n",value);
#ifdef GSM_ENABLE_BEEP
	if (atoi(argv[2]) == 243) GSM_PhoneBeep();
#endif
	GSM_Terminate();
}

static GSM_Error DCT3_ReplyGetMSID(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	int i;

	printf("MSID          : ");
	for (i=5;i<18;i++) printf("%02x",msg.Buffer[i]);
	printf("\n");
	return ERR_NONE;
}

static GSM_Error DCT3_ReplyGetDSPROM(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	printf("DSP ROM       : %c\n",msg.Buffer[5]);
	return ERR_NONE;
}

static GSM_Error DCT3_ReplySimlockInfo(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	int	i, j;
	char	uni[100], buffer[50];

	j=0;
	for (i=0; i < 12; i++) {
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

	return ERR_NONE;
}

static GSM_Error DCT3_ReplyGetMCUchkSum(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	int i;

	if (msg.Buffer[3] == 0x12) printf("Language Pack: %c\n",msg.Buffer[5]);
	if (msg.Buffer[3] == 0x02) {
		printf("MCU checksum  : ");
		for (i=5;i<9;i++) printf("%c",msg.Buffer[i]);
		printf("\n");
	}
	return ERR_NONE;
}

static unsigned char MSID1;

GSM_Error DCT3_ReplyEnableSecurity2(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	smprintf(sm, "State of security commands set\n");
	MSID1 = msg.Buffer[5];
	return ERR_NONE;
}

GSM_Error DCT3Info(void)
{
	unsigned char 		req[]  	= {0x00, 0x01, 0x8A, 0x00}; 	  /* Get simlock info */
	unsigned char 		req2[]  = {0x00, 0x01, 0xb4, 0x00, 0x00}; /* Get MSID */
	unsigned char 		req3[]  = {0x00, 0x01, 0xc8, 0x02}; 	  /* Get MCU chksum */
	unsigned char 		req4[]  = {0x00, 0x01, 0xc8, 0x09}; 	  /* Get DSP ROM */
	GSM_Error error;

	error = CheckDCT3Only();
	if (error != ERR_NONE) return error;

	gsm->User.UserReplyFunctions=UserReplyFunctions3;

	error=DCT3_EnableSecurity (gsm, 0x01);
	Print_Error(error);

	error=GSM_WaitFor (gsm, req, 4, 0x40, 4, ID_User3);
	Print_Error(error);

	req2[3] = MSID1;
	req2[4] = req2[2] + req2[3];
 	error=GSM_WaitFor (gsm, req2, 5, 0x40, 4, ID_User8);
	Print_Error(error);

	error=GSM_WaitFor (gsm, req3, 4, 0x40, 4, ID_User9);
	Print_Error(error);

	error=GSM_WaitFor (gsm, req4, 4, 0x40, 4, ID_User10);
	Print_Error(error);
	return ERR_NONE;
}

static GSM_Error DCT3_ReplyResetTest36(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	printf("Netmonitor test 36 cleaned OK\n");
	return ERR_NONE;
}

void DCT3ResetTest36(int argc, char *argv[])
{
	unsigned char req[]  = {0x00, 0x01, 0x65, 0x40, 0x00}; /* Reset test 36 in netmon */
	GSM_Error error;

	GSM_Init(TRUE);

        CheckDCT3();

	error=DCT3_EnableSecurity (gsm, 0x01);
	Print_Error(error);

	gsm->User.UserReplyFunctions=UserReplyFunctions3;

	error=GSM_WaitFor (gsm, req, 5, 0x40, 4, ID_User2);
	Print_Error(error);

#ifdef GSM_ENABLE_BEEP
	GSM_PhoneBeep();
#endif
	GSM_Terminate();
}

static unsigned char PPS[32]; /* Product Profile Settings */

static GSM_Error DCT3_ReplyGetPPS(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	int i,j,z;

#ifdef DEBUG
	smprintf(sm, "Product Profile Settings received -");
	for (i=0;i<4;i++) smprintf(sm, " %02x",msg.Buffer[3+i]);
	smprintf(sm, "\n");
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
	smprintf(sm, "After decoding: ");
	for (i=0;i<32;i++) smprintf(sm, "%c",PPS[i]);
	smprintf(sm, "\n");
#endif
	return ERR_NONE;
}

static GSM_Error DCT3_ReplySetPPS(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	printf("Setting done OK\n");
	return ERR_NONE;
}

void DCT3SetPhoneMenus(int argc, char *argv[])
{
	char 		value[100];
	GSM_Error error;
	int 		i,j,z;
	unsigned char 	reqGet[] = {0x00, 0x01, 0x6a};
	unsigned char 	reqSet[] = {
		0x00, 0x01, 0x6b,
		0x00, 0x00, 0x00, 0x00 }; /* bytes with Product Profile Setings */

	CheckDCT3();

	error=DCT3_EnableSecurity (gsm, 0x01);
	Print_Error(error);

	gsm->User.UserReplyFunctions=UserReplyFunctions3;

	error=GSM_WaitFor (gsm, reqGet, 3, 0x40, 4, ID_User4);
	Print_Error(error);

	printf("ALS : enabling menu\n");
	PPS[10] = '1';

	if (!strcmp(gsm->Phone.Data.ModelInfo->model,"3310") && gsm->Phone.Data.VerNum>5.87) {
		printf("3310: enabling control of SMS charsets\n");
		PPS[11] = '0';/* 0 = ON, 1 = OFF */
	}
	if (!strcmp(gsm->Phone.Data.ModelInfo->model,"6150")) {
		printf("6150: enabling WellMate menu\n");
		PPS[18] = '1';
	}
	/* FIXME */
	if (!strcmp(gsm->Phone.Data.ModelInfo->model,"3210")) {
		printf("3210: enabling vibra menu\n");
		PPS[24] = '1';
	}
	if (!strcmp(gsm->Phone.Data.ModelInfo->model,"3310") && gsm->Phone.Data.VerNum>5.13) {
		printf("3310: enabling 3315 features\n");
		PPS[25] = '1';
	}
	/* FIXME */
	if (!strcmp(gsm->Phone.Data.ModelInfo->model,"3210") && gsm->Phone.Data.VerNum>=5.31) {
		printf("3210: enabling React and Logic game\n");
		PPS[26] = '1';
	}

#ifdef DEBUG
	smprintf(gsm, "After settings: ");
	for (i=0;i<32;i++) smprintf(gsm, "%c",PPS[i]);
	smprintf(gsm, "\n");
#endif

	j=128;z=0;
	for (i=0;i<32;i++) {
		if (PPS[i]=='1') reqSet[z+3]=reqSet[z+3]+j;
		if (j==1) {
			j=128;
			z++;
		} else j=j/2;
	}

/* 	reqSet[3]=0xe7; */
/* 	reqSet[4]=0x25; */
/* 	reqSet[5]=0x00; */
/* 	reqSet[6]=0xe0; */

	error=GSM_WaitFor (gsm, reqSet, 7, 0x40, 4, ID_User4);
	Print_Error(error);

	printf("Enabling netmonitor\n");
	error=DCT3_Netmonitor(gsm, 243, value);
	Print_Error(error);
}

static GSM_Error DCT3_Reply61GetSecurityCode(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	printf("Security Code is \"%s\"\n",msg.Buffer+5);
	return ERR_NONE;
}

static GSM_Error DCT3_Reply7191GetSecurityCode(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	printf("Security Code is \"%s\"\n",msg.Buffer+6);
	return ERR_NONE;
}

void DCT3GetSecurityCode(int argc, char *argv[])
{
#ifdef GSM_ENABLE_NOKIA6110
	unsigned char req6110[] = {0x00, 0x01, 0x6e,
				   0x01};	/* Code type */
#endif
#if defined(GSM_ENABLE_NOKIA7110) || defined(GSM_ENABLE_NOKIA9210)
	unsigned char req71_91[] = {N7110_FRAME_HEADER, 0xee,
				    0x1c};	/* Setting */
#endif
	GSM_Error error;

	CheckDCT3();

	error=DCT3_EnableSecurity (gsm, 0x01);
	Print_Error(error);

	gsm->User.UserReplyFunctions=UserReplyFunctions3;

#ifdef GSM_ENABLE_NOKIA6110
	if (strstr(N6110Phone.models, gsm->Phone.Data.ModelInfo->model) != NULL) {
		error=GSM_WaitFor (gsm, req6110, 4, 0x40, 4, ID_User6);
	}
#endif
#ifdef GSM_ENABLE_NOKIA7110
	if (strstr(N7110Phone.models, gsm->Phone.Data.ModelInfo->model) != NULL) {
		error=GSM_WaitFor (gsm, req71_91, 5, 0x7a, 4, ID_User6);
	}
#endif
#ifdef GSM_ENABLE_NOKIA9210
	if (strstr(N9210Phone.models, gsm->Phone.Data.ModelInfo->model) != NULL) {
		error=GSM_WaitFor (gsm, req71_91, 5, 0x7a, 4, ID_User6);
	}
#endif
	Print_Error(error);
}

#ifdef GSM_ENABLE_NOKIA6110

static GSM_Error DCT3_ReplyGetOperatorName(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	unsigned char buffer[10];

	NOKIA_DecodeNetworkCode(msg.Buffer+5, buffer);
	buffer[6] = 0;
	printf("Network           : %s (%s ",	buffer,DecodeUnicodeString(GSM_GetNetworkName(buffer)));
	printf(", %s)\n",			DecodeUnicodeString(GSM_GetCountryName(buffer)));
	printf("Name              : \"%s\"\n",msg.Buffer+8);

	return ERR_NONE;
}

void DCT3GetOperatorName(int argc, char *argv[])
{
	unsigned char req[] = {0x00,0x01,0x8c,0x00};
	GSM_Error error;

	GSM_Init(TRUE);

	if (strstr(N6110Phone.models, gsm->Phone.Data.ModelInfo->model) == NULL) Print_Error(ERR_NOTSUPPORTED);
	CheckDCT3();

	error=DCT3_EnableSecurity (gsm, 0x01);
	Print_Error(error);

	gsm->User.UserReplyFunctions=UserReplyFunctions3;

	error=GSM_WaitFor (gsm, req, 4, 0x40, 4, ID_User5);
	Print_Error(error);

	GSM_Terminate();
}

static GSM_Error DCT3_ReplySetOperatorName(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	printf("Operator name set OK\n");
	return ERR_NONE;
}

void DCT3SetOperatorName(int argc, char *argv[])
{
	int 		i = 0;
	GSM_Error error;
	unsigned char 	req[256] = {0x00,0x01,0x8b,0x00,
				    0x00,0x00, /* MCC */
				    0x00};     /* MNC */

	GSM_Init(TRUE);

	if (strstr(N6110Phone.models, gsm->Phone.Data.ModelInfo->model) == NULL) Print_Error(ERR_NOTSUPPORTED);
	CheckDCT3();

	error=DCT3_EnableSecurity (gsm, 0x01);
	Print_Error(error);

	gsm->User.UserReplyFunctions=UserReplyFunctions3;

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

	error=GSM_WaitFor (gsm, req, 8+i, 0x40, 4, ID_User7);
	Print_Error(error);

	GSM_Terminate();
}

static GSM_Error DCT3_ReplyDisplayOutput(GSM_Protocol_Message msg, GSM_StateMachine *sm)
{
	unsigned char buf[100];

	switch (msg.Buffer[3]) {
	case 0x50:
		smprintf(sm, "Display string received\n");
		memcpy(buf,msg.Buffer+8,msg.Buffer[7]*2);
		buf[msg.Buffer[7]*2]   = 0;
		buf[msg.Buffer[7]*2+1] = 0;
		printf("X=%i, Y=%i, Text=\"%s\"\n",msg.Buffer[6],msg.Buffer[5],DecodeUnicodeString(buf));
		return ERR_NONE;
	case 0x54:
		smprintf(sm, "Display output set\n");
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

void DCT3DisplayOutput(int argc, char *argv[])
{
	unsigned char req[] = {N6110_FRAME_HEADER, 0x53,
			       0x01}; /* 1 = enable, 2 = disable */
	GSM_Error error;

	GSM_Init(TRUE);

	if (strstr(N6110Phone.models, gsm->Phone.Data.ModelInfo->model) == NULL) Print_Error(ERR_NOTSUPPORTED);
	CheckDCT3();

	gsm->User.UserReplyFunctions=UserReplyFunctions3;

	error=GSM_WaitFor (gsm, req, 5, 0x0d, 4, ID_User7);
	Print_Error(error);

	signal(SIGINT, interrupt);
	printf("Press Ctrl+C to break...\n");
	printf("Entering monitor mode...\n\n");

	while (!gshutdown) {
		GSM_ReadDevice(gsm,TRUE);
		usleep(10000);
	}

	req[4] = 0x02;
	error=GSM_WaitFor (gsm, req, 5, 0x0d, 4, ID_User7);
	Print_Error(error);

	GSM_Terminate();
}
#endif

GSM_Reply_Function UserReplyFunctions3[] = {
#ifdef GSM_ENABLE_NOKIA6110
	{DCT3_ReplyDisplayOutput,	"\x0D",0x03,0x50,ID_IncomingFrame	},
	{DCT3_ReplyDisplayOutput,	"\x0D",0x03,0x54,ID_User7	 	},
#endif

	{DCT3_ReplyEnableSecurity2,	"\x40",0x02,0x64,ID_EnableSecurity	},
	{DCT3_ReplyResetTest36,		"\x40",0x02,0x65,ID_User2 	 	},
	{DCT3_ReplyGetADC,		"\x40",0x02,0x68,ID_User3 	 	},
	{DCT3_ReplyGetPPS,		"\x40",0x02,0x6A,ID_User4 	 	},
	{DCT3_ReplySetPPS,		"\x40",0x02,0x6B,ID_User4 	 	},
	{DCT3_Reply61GetSecurityCode,	"\x40",0x02,0x6E,ID_User6 	 	},
	{DCT3_ReplySimlockInfo,		"\x40",0x02,0x8A,ID_User3	 	},
#ifdef GSM_ENABLE_NOKIA6110
	{DCT3_ReplySetOperatorName,	"\x40",0x02,0x8B,ID_User7	 	},
	{DCT3_ReplyGetOperatorName,	"\x40",0x02,0x8C,ID_User5	 	},
#endif
	{DCT3_ReplyGetADC,		"\x40",0x02,0x91,ID_User3 	 	},
	{NoneReply,			"\x40",0x02,0xA3,ID_User3		},
	{DCT3_ReplyGetT9,		"\x40",0x02,0xAE,ID_User3		},
	{DCT3_ReplyGetMSID,		"\x40",0x02,0xb5,ID_User8	 	},
	{DCT3_ReplyGetDSPROM,		"\x40",0x02,0xC8,ID_User10	 	},
	{DCT3_ReplyGetMCUchkSum,	"\x40",0x02,0xC8,ID_User9	 	},
	{DCT3_ReplyPhoneTests,		"\x40",0x02,0xCF,ID_User1	 	},

	{DCT3_Reply7191GetSecurityCode,	"\x7a",0x04,0x1C,ID_User6	 	},

	{NULL,				"\x00",0x00,0x00,ID_None	 	}
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
