/* (c) 2002-2004 by Marcin Wiacek and Michal Cihar */

#include "../../gsmstate.h"

#if defined(GSM_ENABLE_AT) || defined(GSM_ENABLE_BLUEAT) || defined(GSM_ENABLE_IRDAAT) || defined(GSM_ENABLE_DKU2AT)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../../gsmcomon.h"
#include "at.h"

static GSM_Error AT_WriteMessage (GSM_StateMachine *s, unsigned const char *buffer,
				     size_t length, int type)
{
	size_t sent=0, i=0;
	ssize_t write_data=0;

	GSM_DumpMessageText(s, buffer, length, type);
	GSM_DumpMessageBinary(s, buffer, length, type);

	if (s->Protocol.Data.AT.FastWrite) {
		while (sent != length) {
			write_data = s->Device.Functions->WriteDevice(s,buffer + sent, length - sent);

			if (write_data == 0 || write_data < 0) {
				return ERR_DEVICEWRITEERROR;
			}
			sent += write_data;
		}
	} else {
		for (i = 0; i < length; i++) {
			/* For some phones like Siemens M20 we need to wait a little
			 * after writing each char. Possible reason: these phones
			 * can't receive so fast chars or there is bug here in Gammu */
			write_data = s->Device.Functions->WriteDevice(s, buffer + i, 1);

			if (write_data != 1) {
				return ERR_DEVICEWRITEERROR;
			}
			usleep(1000);
		}
		usleep(400000);
	}
	return ERR_NONE;
}

typedef struct {
	const char	*text;
	int	lines;
	GSM_Phone_RequestID requestid;
} SpecialAnswersStruct;

GSM_Error AT_StateMachine(GSM_StateMachine *s, unsigned char rx_char)
{
	GSM_Protocol_Message 	Msg2;
	GSM_Protocol_ATData 	*d = &s->Protocol.Data.AT;
	size_t			i;

	/* These are lines with end of "normal" answers */
	static const char 		*StatusStrings[] = {
		/* Standard AT */
		"OK\r",
		"ERROR\r",

		/* AT with bad end of lines */
		"OK\n",
		"ERROR\n",

		/* Standard GSM */
		"+CME ERROR:",
		"+CMS ERROR:",

		/* Motorola A1200 */
		"MODEM ERROR:",

		/* Huawei */
		"COMMAND NOT SUPPORT",

		NULL};

	/* Some info from phone can be inside "normal" answers
	 * It starts with strings written here
	 */
	static const SpecialAnswersStruct SpecialAnswers[] = {
		/* Standard GSM */
		{"+CGREG:"	,1, ID_GetNetworkInfo},
		/* Following has 2 lines in PDU mode, 1 line in TEXT ... */
		{"+CBM:"	,2, ID_All},
		{"+CMT:"	,2, ID_All},
		{"+CMTI:"	,1, ID_All},
		{"+CDS:"	,2, ID_All},
		{"+CDSI:"	,1, ID_All},
		{"+CREG:"	,1, ID_GetNetworkInfo},
		{"+CUSD"	,1, ID_All},
		{"+COLP"	,1, ID_All},
		{"+CLIP"	,1, ID_All},
		{"+CRING"	,1, ID_All},
		{"+CCWA"	,1, ID_All},
		{"+CLCC"	,1, ID_All},

		/* Standard AT */
		{"RING"		,1, ID_All},
		{"NO CARRIER"	,1, ID_All},
		{"NO ANSWER"	,1, ID_All},

		/* GlobeTrotter */
		{"_OSIGQ:"	,1, ID_All},
		{"_OBS:"	,1, ID_All},

		{"^SCN:"	,1, ID_All},

		/* Sony-Ericsson */
		{"*EBCA"	,1, ID_All},

		/* Samsung binary transfer end */
		{"SDNDCRC ="	,1, ID_All},
		/* Samsung reply to SSHT in some cases */
		{"SAMSUNG PTS DG Test", 1, ID_All},

		/* Cross PD1101wi reply to almost anything */
		{"NOT FOND ^,NOT CUSTOM AT", 1, ID_All},

		/* Motorola banner */
		{"+MBAN:"	,1, ID_All},

		/* HSPA CORPORATION */
		{"+ZEND"	,1, ID_All},

		/* Huawei */
		{"^RSSI:"	,1, ID_All}, /* ^RSSI:18 */
		{"^HCSQ:"	,1, ID_All}, /* ^HCSQ:"WCDMA",39,29,45 */
		{"^DSFLOWRPT:"	,1, ID_All}, /* ^DSFLOWRPT:00000124,00000082,00000EA6,0000000000012325,000000000022771D,0000BB80,0001F400 */
		{"^BOOT:"	,1, ID_All}, /* ^BOOT:27710117,0,0,0,75 */
		{"^MODE:"	,1, ID_All}, /* ^MODE:3,3 */
		{"^CSNR:"	,1, ID_All}, /* ^CSNR:-93,-23 */
		{"^HCSQ:"	,1, ID_All}, /* ^HCSQ:"LTE",59,50,161,24 */
		{"^SRVST:"	,1, ID_All}, /* ^SRVST:0 */
		{"^SIMST:"	,1, ID_All}, /* ^SIMST:1 */
		{"^STIN:"	,1, ID_All}, /* ^STIN: 7, 0, 0 */

		/* D-Link */
		{"+SPNWNAME:"	,1, ID_All}, /* +SPNWNAME: "432", "11", "Mci", "Mci" */
		{"+PSBEARER:"	,1, ID_All}, /* +PSBEARER: 24, 0 */

		/* ONDA */
		{"+ZUSIMR:"	,1, ID_All}, /* +ZUSIMR:2 */

		/* Telit */
		{"#STN:"	,1, ID_All}, /* #STN: 150,1,"" */

		{NULL		,1, ID_All}};

	/* We're starting new message */
	if (d->Msg.Length == 0) {
		/* Ignore leading CR, LF and ESC */
		if (rx_char == 10 || rx_char == 13 || rx_char == 27) {
			return ERR_NONE;
		}
		d->LineStart = 0;
	}

	/* Allocate more memory if needed */
	if (d->Msg.BufferUsed < d->Msg.Length + 2) {
		d->Msg.BufferUsed	= d->Msg.Length + 200;
		d->Msg.Buffer 		= (unsigned char *)realloc(d->Msg.Buffer,d->Msg.BufferUsed);
		if (d->Msg.Buffer == NULL) {
			return ERR_MOREMEMORY;
		}
	}

	/* Store current char in the buffer */
	d->Msg.Buffer[d->Msg.Length++] = rx_char;
	d->Msg.Buffer[d->Msg.Length  ] = 0;

	/* Parse char */
	switch (rx_char) {
	case 0:
		break;
	case 10:
	case 13:
		/* Store line end (if we did not do it in last char */
		if (! d->wascrlf) {
			d->LineEnd = d->Msg.Length - 1;
		}
		d->wascrlf = TRUE;

		/* Process line after \r\n */
		if (d->Msg.Length > 0 && rx_char == 10 && d->Msg.Buffer[d->Msg.Length - 2] == 13) {
			/* Process standard responses */
			for (i = 0; StatusStrings[i] != NULL; i++) {
				if (strncmp(StatusStrings[i],
							d->Msg.Buffer + d->LineStart,
							strlen(StatusStrings[i])) == 0) {
					s->Phone.Data.RequestMsg	= &d->Msg;
					s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);
					d->Msg.Length			= 0;
					break;
				}
			}
			/* Generally hack for A2D */
			if (d->CPINNoOK) {
				if (strncmp("+CPIN: ",
						d->Msg.Buffer + d->LineStart,
						7) == 0) {
					s->Phone.Data.RequestMsg	= &d->Msg;
					s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);
					d->Msg.Length			= 0;
					break;
				}
			}

			/* Check for incoming frames */
			for (i = 0; SpecialAnswers[i].text != NULL; i++) {
				if (strncmp(SpecialAnswers[i].text,
							d->Msg.Buffer + d->LineStart,
							strlen(SpecialAnswers[i].text)) == 0) {
					/* We need something better here */
					if (s->Phone.Data.RequestID == SpecialAnswers[i].requestid) {
						i++;
						continue;
					}
					if ((s->Phone.Data.RequestID == ID_SetOBEX || s->Phone.Data.RequestID == ID_DialVoice)&&
							strcmp(SpecialAnswers[i].text, "NO CARRIER") == 0) {
						i++;
						continue;
					}
					d->SpecialAnswerStart 	= d->LineStart;
					d->SpecialAnswerLines	= SpecialAnswers[i].lines;
				}
			}

			/* Last line of incoming frame */
			if (d->SpecialAnswerLines == 1) {
				/* This is end of special answer. We copy it and send to phone module */
				Msg2.Buffer = (unsigned char *)malloc(d->LineEnd - d->SpecialAnswerStart + 3);
				memcpy(Msg2. Buffer, d->Msg.Buffer + d->SpecialAnswerStart, d->LineEnd - d->SpecialAnswerStart + 2);
				Msg2.Length = d->LineEnd - d->SpecialAnswerStart + 2;
				Msg2.Buffer[Msg2.Length] = '\0';
				Msg2.Type = 0;

				s->Phone.Data.RequestMsg	= &Msg2;
				s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);
				free(Msg2.Buffer);
				Msg2.Buffer = NULL;

				/* We cut special answer from main buffer */
				d->Msg.Length			= d->SpecialAnswerStart;

				/* We need to find earlier values of all variables */
				d->wascrlf 			= FALSE;
				d->LineStart			= 0;
				for (i = 0;i < d->Msg.Length; i++) {
					switch (d->Msg.Buffer[i]) {
					case 0:
						break;
					case 10:
					case 13:
						if (!d->wascrlf) {
							d->LineEnd = d->Msg.Length;
						}
						d->wascrlf = TRUE;
						break;
					default:
						if (d->wascrlf) {
							d->LineStart	= d->Msg.Length;
							d->wascrlf 	= FALSE;
						}
					}
				}
				d->Msg.Buffer[d->Msg.Length] = 0;
			}
			if (d->SpecialAnswerLines > 0) {
				d->SpecialAnswerLines--;
			}
		}
		break;
	case 'T':
		/* When CONNECT string received, we know there will not follow
		 * anything AT related, after CONNECT can follow ppp data, alcabus
		 * data and also other things.
		 */
		if (strncmp(d->Msg.Buffer + d->LineStart, "CONNECT", 7) == 0) {
			s->Phone.Data.RequestMsg   	= &d->Msg;
			s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);
			d->LineStart              	= -1;
			d->Msg.Length			= 0;
			break;
		}
		FALLTHROUGH
	default:
		if (d->wascrlf) {
			d->LineStart	= d->Msg.Length - 1;
			d->wascrlf 	= FALSE;
		}
		if (d->EditMode) {
			if (strlen(d->Msg.Buffer+d->LineStart) == 2 &&
					strncmp(d->Msg. Buffer + d->LineStart, "> ", 2) == 0) {
				s->Phone.Data.RequestMsg	= &d->Msg;
				s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);
			}
		}
	}
	return ERR_NONE;
}

GSM_Error AT_Initialise(GSM_StateMachine *s)
{
	GSM_Protocol_ATData *d = &s->Protocol.Data.AT;
	GSM_Error		error;

	d->Msg.Buffer 		= NULL;
	d->Msg.BufferUsed	= 0;
	d->Msg.Length		= 0;
	d->Msg.Type		= 0;

	d->SpecialAnswerLines	= 0;
	d->LineStart		= -1;
	d->LineEnd		= -1;
	d->wascrlf 		= FALSE;
	d->EditMode		= FALSE;
	/* Slow write makes sense only on cable for some phones */
	d->FastWrite		= (s->ConnectionType != GCT_AT);
	d->CPINNoOK		= FALSE;

	error = s->Device.Functions->DeviceSetParity(s, FALSE);
	if (error != ERR_NONE) return error;

	error = s->Device.Functions->DeviceSetDtrRts(s, TRUE, TRUE);
	if (error != ERR_NONE) return error;

	return s->Device.Functions->DeviceSetSpeed(s, s->Speed);
}

static GSM_Error AT_Terminate(GSM_StateMachine *s)
{
	free(s->Protocol.Data.AT.Msg.Buffer);
	s->Protocol.Data.AT.Msg.Buffer = NULL;
	return ERR_NONE;
}

GSM_Protocol_Functions ATProtocol = {
	AT_WriteMessage,
	AT_StateMachine,
	AT_Initialise,
	AT_Terminate
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
