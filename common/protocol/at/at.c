
#include "../../gsmstate.h"

#if defined(GSM_ENABLE_AT) || defined(GSM_ENABLE_BLUEAT) || defined(GSM_ENABLE_IRDAAT)

#include <stdio.h>
#include <string.h>

#include "../../gsmcomon.h"
#include "at.h"

static GSM_Error AT_WriteMessage (GSM_StateMachine *s, unsigned char *buffer,
				     int length, unsigned char type)
{
	int i,sent = 0;

	GSM_DumpMessageLevel2(s, buffer, length, type);
	GSM_DumpMessageLevel3(s, buffer, length, type);
	if (s->Protocol.Data.AT.FastWrite) {
		while (sent != length) {
			if ((i = s->Device.Functions->WriteDevice(s,buffer + sent, length - sent)) == 0) {
				return GE_DEVICEWRITEERROR;
			}
			sent += i;
		}
	} else {
		for (i=0;i<length;i++) {
			if (s->Device.Functions->WriteDevice(s,buffer+i,1)!=1) return GE_DEVICEWRITEERROR;
			/* For some phones like Siemens M20 we need to wait a little
			 * after writing each char. Possible reason: these phones
			 * can't receive so fast chars or there is bug here in Gammu */
			my_sleep(1);
		} 
		my_sleep(400);
	}

	return GE_NONE;
}

static GSM_Error AT_StateMachine(GSM_StateMachine *s, unsigned char rx_byte)
{
	GSM_Protocol_ATData 	*d = &s->Protocol.Data.AT;
	int			i;
	static char 		*StartStrings[] = {
		"OK"		  , "ERROR"	    , "+CME ERROR:"	,
		"+CMS ERROR:"	  , "RING"	    , "NO CARRIER"	,
		"NO ANSWER"	  , "AT+CREG=1"     , "\x0D\x0A+COLP"   ,
		"+CPIN: "	  , "\x0D\x0A+CREG:",
		"\x0D\x0A_OSIGQ:" , "_OSIGQ:"	    ,
		"\x0D\x0A_OBS:"   , "_OBS:"	    ,
		"\x0D\x0A+CGREG:" , "+CGREG:"	    ,
		"\x0D\x0A+CMTI:"  , "CMIT:"	    ,
		"\x0D\x0A^SCN:"   , "^SCN:"};

    	/* Ignore leading CR, LF and ESC */
    	if (d->Msg.Length == 0 && (rx_byte == 10 || rx_byte == 13 || rx_byte == 27)) return GE_NONE;

	if (d->Msg.BufferUsed < d->Msg.Length + 2) {
		d->Msg.BufferUsed	= d->Msg.Length + 2;
		d->Msg.Buffer 		= (unsigned char *)realloc(d->Msg.Buffer,d->Msg.BufferUsed);
		if (d->linestart != NULL) {
			d->linestart = d->Msg.Buffer+d->linestartnum;	
		}
	}
	d->Msg.Buffer[d->Msg.Length++] = rx_byte;
	d->Msg.Buffer[d->Msg.Length  ] = 0;
	if (d->linestart == NULL) {
	    d->linestart 	= d->Msg.Buffer;
	    d->linestartnum 	= 0;
	}

	switch (rx_byte) {
	case 0:
		break;
	case 10:
	case 13:
		d->wascrlf = true;
		if (d->Msg.Length > 0 && rx_byte == 10 && d->Msg.Buffer[d->Msg.Length-2]==13) {
			for (i=0;i<21;i++) {
			    if (strncmp(StartStrings[i],d->linestart,strlen(StartStrings[i])) == 0) {
				s->Phone.Data.RequestMsg	= &d->Msg;
				s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);
				d->linestart			= NULL;
				d->Msg.Length			= 0;
				break;
			    }
			}
		}
		break;
	case 'T':
		/* When CONNECT string received, we know there will not follow
		 * anything AT related, after CONNECT can follow ppp data, alcabus
         	 * data and also other things.
         	 */
        	if (strncmp(d->linestart, "CONNECT", 7) == 0) {
            		s->Phone.Data.RequestMsg   	= &d->Msg;
           		s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);
            		d->linestart              	= NULL;
			d->Msg.Length			= 0;
            		break;
       		}
	default:
		if (d->wascrlf) {
			d->linestart 	= d->Msg.Buffer + (d->Msg.Length - 1);
			d->linestartnum	= d->Msg.Length - 1;
			d->wascrlf 	= false;
		}
		if (d->EditMode) {
			if (strlen(d->linestart) == 2 && strncmp(d->linestart,"> ",2)==0) {
				s->Phone.Data.RequestMsg	= &d->Msg;
				s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);
			}
		}
	}
	return GE_NONE;
}

static GSM_Error AT_Initialise(GSM_StateMachine *s)
{
	GSM_Protocol_ATData *d = &s->Protocol.Data.AT;

	d->Msg.BufferUsed	= 0;
	d->Msg.Buffer 		= NULL;
	d->Msg.Length		= 0;

	d->linestart 		= NULL;
	d->EditMode		= false;
	d->Msg.Type		= 0;
	d->FastWrite		= false;

	s->Device.Functions->DeviceSetDtrRts(s,true,true);

	return s->Device.Functions->DeviceSetSpeed(s,s->Speed);
}

static GSM_Error AT_Terminate(GSM_StateMachine *s)
{
	free(s->Protocol.Data.AT.Msg.Buffer);
	return GE_NONE;
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
