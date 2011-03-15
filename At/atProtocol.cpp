/* (c) 2002-2004 by Marcin Wiacek and Michal Cihar */
#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "At.h"

#include "atprotocol.h"
extern CAtApp theApp;

GSM_Error AT_WriteMessage (unsigned char *buffer,
				     int length, unsigned char type)
{
	GSM_Protocol_ATData *d = &theApp.m_ATProtocolData;
	CDeviceAPI	*Device 	= &theApp.m_DeviceAPI;
	int i,sent = 0;
	d->Msg.Type = type;

	if (d->FastWrite)
	{
		while (sent != length) {
			if ((i = Device->DeviceAPI_WriteDevice(buffer + sent, length - sent)) == 0) {
				return ERR_DEVICEWRITEERROR;
			}
			sent += i;
		}
	} else {
		for (i=0;i<length;i++) {
			if (Device->DeviceAPI_WriteDevice(buffer+i,1)!=1) return ERR_DEVICEWRITEERROR;
			/* For some phones like Siemens M20 we need to wait a little
			 * after writing each char. Possible reason: these phones
			 * can't receive so fast chars or there is bug here in MBdrv */
			my_sleep(1);
		} 
		my_sleep(400);
	}

	return ERR_NONE;
}

typedef struct {
	char	*text;
	int	lines;
} SpecialAnswersStruct;

GSM_Error AT_StateMachine(unsigned char rx_char)
{
	GSM_Protocol_Message 	Msg2;
	GSM_Protocol_ATData *d = &theApp.m_ATProtocolData;
	CDeviceAPI	*Device 	= &theApp.m_DeviceAPI;
	GSM_ATMultiAnwser *pATMultiAnwser=theApp.m_pATMultiAnwser;
	int			i;

	/* These are lines with end of "normal" answers */
	static char 		*StartStrings[] = {
		"OK"		, 	"ERROR"	 	,
		"+CME ERROR:"	,	"+CMS ERROR:"	,

		"+CPIN: "	,	/*A2D issue*/
		"ATE"	,	/*SL5C*/
		"CONNECT",
#ifdef _LG_
		"EXITEND",
#endif
		NULL};

	/* Some info from phone can be inside "normal" answers
	 * It starts with strings written here
	 */
	static SpecialAnswersStruct	SpecialAnswers[] = {
		{"_OSIGQ:"	,1},	{"_OBS:"	,1},
		{"^SCN:"	,1},	{"+CGREG:"	,1},
		{"+CBM:"	,1},	{"+CMT:"	,2},
		{"+CMTI:"	,1},	{"+CDS:"	,2},	
		{"RING"		,1},	{"NO CARRIER"	,1},
		{"NO ANSWER"	,1},	{"+COLP"	,1},
		{"+CLIP"	,1},

		{"SDNDCRC ="	,1},	/* Samsung binary transfer end */

		{NULL		,1}};

    	/* Ignore leading CR, LF and ESC */
    	if (d->Msg.Length == 0 && d->Msg.Type == 0x00) {
		if (rx_char == 10 || rx_char == 13 || rx_char == 27) return ERR_NONE;
		d->LineStart = d->Msg.Length;
	}

	if (d->Msg.BufferUsed < d->Msg.Length + 2) {
		d->Msg.BufferUsed	= d->Msg.Length + 2;
		d->Msg.Buffer 		= (unsigned char *)realloc(d->Msg.Buffer,d->Msg.BufferUsed);
	}
	d->Msg.Buffer[d->Msg.Length++] = rx_char;
	d->Msg.Buffer[d->Msg.Length  ] = 0;

	switch (rx_char) {
	case 0:
		break;
	case 10:
	case 13:
		if (!d->wascrlf) d->LineEnd = d->Msg.Length-1;
		d->wascrlf = true;
		if (d->Msg.Length > 0 && rx_char == 10 && d->Msg.Buffer[d->Msg.Length-2]==13) {
			i = 0;
			while (StartStrings[i] != NULL) {
				if (strncmp(StartStrings[i],(char*)d->Msg.Buffer+d->LineStart,strlen(StartStrings[i])) == 0) {

					if(theApp.m_bFinishRequest == false)
						theApp.m_DispatchError = theApp.DispatchMessage(&d->Msg);
					d->Msg.Length			= 0;
					break;
				}
				i++;
			}
			if (d->Msg.Length == 0) break;

			i = 0;
			while (SpecialAnswers[i].text != NULL) {
				if (strncmp(SpecialAnswers[i].text,(char*)d->Msg.Buffer+d->LineStart,strlen(SpecialAnswers[i].text)) == 0) {					
					/* We need something better here */

					d->SpecialAnswerStart 	= d->LineStart;
					d->SpecialAnswerLines	= SpecialAnswers[i].lines;
				}
				i++;
			}
			if(theApp.m_pATMultiAnwser && strlen(theApp.m_pATMultiAnwser->Specialtext)>0)
			{
				if (strncmp(theApp.m_pATMultiAnwser->Specialtext,(char*)d->Msg.Buffer+d->LineStart,strlen(theApp.m_pATMultiAnwser->Specialtext)) == 0) 
				{
					d->SpecialAnswerStart 	= d->LineStart;
					d->SpecialAnswerLines	= theApp.m_pATMultiAnwser->Anwserlines;
				}
			}


			if (d->SpecialAnswerLines == 1)
			{
				/* This is end of special answer. We copy it and send to phone module */
				Msg2.Buffer = (unsigned char *)malloc(d->LineEnd - d->SpecialAnswerStart + 3);
				memcpy(Msg2.Buffer,d->Msg.Buffer+d->SpecialAnswerStart,d->LineEnd - d->SpecialAnswerStart + 2);
				Msg2.Length = d->LineEnd - d->SpecialAnswerStart + 2;
				Msg2.Buffer[Msg2.Length] = 0;

				if(theApp.m_bFinishRequest == false)
					theApp.m_DispatchError = theApp.DispatchMessage(&d->Msg);
				free(Msg2.Buffer);
				if(theApp.m_DispatchError != ERR_NEEDANOTHEDATA) 
				{
					d->Msg.Length			= 0;
					d->SpecialAnswerLines	= 0;
					break;
				}

				/* We cut special answer from main buffer */
				d->Msg.Length			= d->SpecialAnswerStart;
				if (d->Msg.Length != 0) d->Msg.Length = d->Msg.Length - 2;

				/* We need to find earlier values of all variables */
				d->wascrlf 			= false;
				d->LineStart			= 0;
				for (i=0;i<d->Msg.Length;i++) 
				{
					switch(d->Msg.Buffer[i])
					{
					case 0:
						break;
					case 10:
					case 13:
						if (!d->wascrlf) d->LineEnd = d->Msg.Length-1;
						d->wascrlf = true;
						break;
					default:
						if (d->wascrlf) 
						{
							d->LineStart	= d->Msg.Length-1;
							d->wascrlf 	= false;
						}

					}
				}

				if(d->LineStart == 0)
				{
					if (d->Msg.Length != 0)
						d->Msg.Length+=2;
					d->LineStart	= d->Msg.Length;
				}

				d->Msg.Buffer[d->Msg.Length] = 0;
			}
			if (d->SpecialAnswerLines > 0) d->SpecialAnswerLines--;
		}
		break;
	case 'T':

	default:
		if (d->wascrlf) 
		{
			d->LineStart	= d->Msg.Length-1;
			d->wascrlf 	= false;
		}
		if (d->EditMode)
		{
			if (strlen((char*)d->Msg.Buffer+d->LineStart) == 2 && strncmp((char*)d->Msg.Buffer+d->LineStart,"> ",2)==0) 
			{

				if(theApp.m_bFinishRequest == false)
					theApp.m_DispatchError = theApp.DispatchMessage(&d->Msg);
			}
		}
	}
	return ERR_NONE;
}
GSM_Error AT_SetProtocolData(bool EditMode,bool bFastWrite,DWORD dwFlag)
{
	GSM_Protocol_ATData *d = &theApp.m_ATProtocolData;
	if(dwFlag & 0x01)
		d->EditMode		= EditMode;
	if(dwFlag & 0x02)
		d->FastWrite		= bFastWrite;
	return ERR_NONE;
}

GSM_Error AT_Initialise(int Speed)
{
	GSM_Protocol_ATData *d = &theApp.m_ATProtocolData;
	CDeviceAPI	*Device 	= &theApp.m_DeviceAPI;

	d->Msg.Buffer 		= NULL;
	d->Msg.BufferUsed	= 0;
	d->Msg.Length		= 0;
	d->Msg.Type		= 0;

	d->SpecialAnswerLines	= 0;
	d->LineStart		= -1;
	d->LineEnd		= -1;
	d->wascrlf 		= false;
	d->EditMode		= false;
	//d->FastWrite		= false;

	Device->DeviceAPI_DeviceSetDtrRts(true,true);

	return Device->DeviceAPI_DeviceSetSpeed(Speed);
}

GSM_Error AT_Terminate()
{
	if(theApp.m_ATProtocolData.Msg.Buffer)
		free(theApp.m_ATProtocolData.Msg.Buffer);
	return ERR_NONE;
}
/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
