/* (c) 2003 by Marcin Wiacek */

#include "../../gsmstate.h"

#if defined(GSM_ENABLE_MROUTERBLUE)

#include <stdio.h>
#include <string.h>

#include "../../gsmcomon.h"
#include "mrouter.h"

static GSM_Error MROUTER_WriteMessage (GSM_StateMachine *s, unsigned char *buffer,
				    int length, unsigned char type)
{
	unsigned char	*out_buffer; 
	int 		current=0,sent;
	
	out_buffer = (unsigned char *)malloc(length+1);

	memcpy(out_buffer,buffer,length);
	out_buffer[length]=0x7E;

	GSM_DumpMessageLevel2(s, out_buffer, length, type);
	GSM_DumpMessageLevel3(s, out_buffer, length, type);

	/* Send it out... */
	sent = s->Device.Functions->WriteDevice(s,out_buffer,length+1);

	free(out_buffer);

	if (sent!=current) return ERR_DEVICEWRITEERROR;
	return ERR_NONE;
}

static GSM_Error MROUTER_StateMachine(GSM_StateMachine *s, unsigned char rx_char)
{
	GSM_Phone_Functions 	*Phone	= s->Phone.Functions;
	GSM_Protocol_MROUTERData 	*d	= &s->Protocol.Data.MROUTER;

	switch (d->MsgRXState) {
	case RX_Sync:
		if (rx_char == 0x7E) {
			d->MsgRXState = RX_GetMessage;
			d->Msg.Count = 0;
			d->Msg.Length = 0;
		} else smprintf(s,"Sync error: %02x\n",rx_char);
		break;
	case RX_GetMessage:
		if (rx_char == 0x7E) {
			s->Phone.Data.RequestMsg	= &d->Msg;
			s->Phone.Data.DispatchError	= Phone->DispatchMessage(s);
			d->Msg.Count = 0;
			d->Msg.Length = 0;
		} else {
			d->Msg.BufferUsed 	= d->Msg.Length+1;
			d->Msg.Buffer 		= (unsigned char *)realloc(d->Msg.Buffer,d->Msg.BufferUsed);

			d->Msg.Buffer[d->Msg.Count] = rx_char;
			d->Msg.Count++;
			d->Msg.Length++;
		}
		break;
	}

	return ERR_NONE;
}

static GSM_Error MROUTER_Initialise(GSM_StateMachine *s)
{
	GSM_Protocol_MROUTERData *d = &s->Protocol.Data.MROUTER;
GSM_Error error;

	d->Msg.BufferUsed	= 0;
	d->Msg.Buffer 		= NULL;
	d->Msg.Length		= 0;

	d->MsgRXState 		= RX_Sync;

//	error=s->Device.Functions->DeviceSetDtrRts(s,false,false);
//    	if (error!=ERR_NONE) return error; 

	error=s->Device.Functions->DeviceSetSpeed(s,115200);
    	if (error!=ERR_NONE) return error; 


//	error=s->Device.Functions->DeviceSetSpeed(s,115200);
//    	if (error!=ERR_NONE) return error; 

	return ERR_NONE;
}

static GSM_Error MROUTER_Terminate(GSM_StateMachine *s)
{
	free(s->Protocol.Data.MROUTER.Msg.Buffer);
	return ERR_NONE;
}

GSM_Protocol_Functions MROUTERProtocol = {
	MROUTER_WriteMessage,
	MROUTER_StateMachine,
	MROUTER_Initialise,
	MROUTER_Terminate
};

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
