
#include "../../gsmstate.h"

#if defined(GSM_ENABLE_ALCABUS)

#include <stdio.h>
#include <string.h>

#include "../../gsmcomon.h"
#include "alcabus.h"

static GSM_Error ALCABUS_WriteMessage (GSM_StateMachine *s, unsigned char *buffer,
				       int length, unsigned char type)
{
	int i;

	GSM_DumpMessageLevel2(s, buffer, length, type);
	GSM_DumpMessageLevel3(s, buffer, length, type);
	for (i=0;i<length;i++) {
		if (s->Device.Functions->WriteDevice(s,buffer+i,1)!=1) return GE_DEVICEWRITEERROR;
		/* For some phones like Siemens M20 we need to wait a little
		 * after writing each char. Possible reason: these phones
		 * can't receive so fast chars or there is bug here in Gammu */
		mili_sleep(1);
	}

	mili_sleep(400);

	return GE_NONE;
}

static GSM_Error ALCABUS_StateMachine(GSM_StateMachine *s, unsigned char rx_byte)
{
	GSM_Protocol_ALCABUSData 	*d = &s->Protocol.Data.ALCABUS;
	int				i;
	static char 			*StartStrings[] = {
		"OK"		  , "ERROR"	    , "+CME ERROR:"	,
		"+CMS ERROR:"	  , "RING"	    , "NO CARRIER"	,
		"NO ANSWER"	  , "AT+CREG=1"     , "\x0D\x0A_OSIGQ:" ,
		"_OSIGQ:"	  , "\x0D\x0A_OBS:" , "_OBS:"		,
		"\x0D\x0A+CGREG:" , "+CGREG:"	    , "\x0D\x0A+CMTI:"	,
		"CMIT:"		  , "\x0D\x0A+COLP" , "\x0D\x0A+CREG"};

	if (d->linestart == NULL) {
		d->Msg.Length = 0;
		d->linestart  = d->Msg.Buffer;
	}

	d->Msg.Buffer[d->Msg.Length++] = rx_byte;
	d->Msg.Buffer[d->Msg.Length  ] = 0;

	switch (rx_byte) {
	case 10:
	case 13:
		d->wascrlf = true;
		if (d->Msg.Length > 0 && rx_byte == 10 && d->Msg.Buffer[d->Msg.Length-2]==13) {
			for (i=0;i<18;i++) {
			    if (strncmp(StartStrings[i],d->linestart,strlen(StartStrings[i])) == 0) {
				s->Phone.Data.RequestMsg	= &d->Msg;
				s->Phone.Data.DispatchError	= s->Phone.Functions->DispatchMessage(s);
				d->linestart			= NULL;
				break;
			    }
			}
		}
		break;
	default:
		if (d->wascrlf) {
			d->linestart = d->Msg.Buffer + (d->Msg.Length - 1);
			d->wascrlf = false;
		}
	}
	if (d->Msg.Length == PROTOCOL_MAX_RECEIVE_LENGTH - 2) d->Msg.Length = 0;
	return GE_NONE;
}

static GSM_Error ALCABUS_Initialise(GSM_StateMachine *s)
{
	GSM_Error error	= GE_UNKNOWN;

	return error;
}

GSM_Protocol_Functions ALCABUSProtocol = {
	ALCABUS_WriteMessage,
	ALCABUS_StateMachine,
	ALCABUS_Initialise,
	NONEFUNCTION
};

#endif

