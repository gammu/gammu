/* (c) 2003 by Marcin Wiacek */
/* www.irda.org OBEX specs 1.3 */




GSM_Error OBEX_WriteMessage (unsigned char *buffer,
				    int length, unsigned char type)
{
	if(length==0 && type == 0x00) return ERR_NONE;
	unsigned char	*out_buffer; 
	int 		current=0,sent;
	CDeviceAPI	*Device 	= &theApp.m_DeviceAPI;
	
	out_buffer = (unsigned char *)malloc(length + 3);

	OBEXAddBlock(out_buffer, &current, type, buffer, length);

//	GSM_DumpMessageLevel2(s, out_buffer+3, length, type);
//	GSM_DumpMessageLevel3(s, out_buffer+3, length, type);

	/* Send it out... */
//	sent = Device->DeviceAPI_WriteDevice(out_buffer,current);
	sent = 0;
	int i =0;
	while (sent != current) 
	{
		if ((i = Device->DeviceAPI_WriteDevice(out_buffer + sent, current - sent)) == 0)
			return ERR_DEVICEWRITEERROR;
		sent += i;
	}

	free(out_buffer);

	if (sent!=current) 
		return ERR_DEVICEWRITEERROR;
	return ERR_NONE;
}

GSM_Error OBEX_StateMachine(unsigned char rx_char)
{
//	GSM_Phone_Functions 	*Phone	= s->Phone.Functions;
	GSM_Protocol_OBEXData *d = &theApp.m_ObexProtocolData;

	switch (d->MsgRXState) {
	case RX_Sync:
		d->Msg.Type   = rx_char;
		d->MsgRXState = RX_GetLength1;
		break;
	case RX_GetLength1:
		d->Msg.Length = rx_char * 256;
		d->MsgRXState = RX_GetLength2;
		break;    
	case RX_GetLength2:
		d->Msg.Length = d->Msg.Length + rx_char - 3;
		d->Msg.Count  = 0;
		if (d->Msg.Count == d->Msg.Length) {
		//	s->Phone.Data.RequestMsg	= &d->Msg;
		//	s->Phone.Data.DispatchError	= Phone->DispatchMessage(s);
			if(theApp.m_bFinishRequest == false)
				theApp.m_DispatchError = theApp.DispatchMessage(&d->Msg);
			d->MsgRXState = RX_Sync;
		} else {
			if (d->Msg.BufferUsed < d->Msg.Length) {
				d->Msg.BufferUsed 	= d->Msg.Length;
				d->Msg.Buffer 		= (unsigned char *)realloc(d->Msg.Buffer,d->Msg.BufferUsed);
			}
			d->MsgRXState 	= RX_GetMessage;
		}
		break;
	case RX_GetMessage:
		d->Msg.Buffer[d->Msg.Count] = rx_char;
		d->Msg.Count++;
		if (d->Msg.Count == d->Msg.Length) {
		//	s->Phone.Data.RequestMsg	= &d->Msg;
		//	s->Phone.Data.DispatchError	= Phone->DispatchMessage(s);
			if(theApp.m_bFinishRequest == false)
				theApp.m_DispatchError = theApp.DispatchMessage(&d->Msg);
			d->MsgRXState = RX_Sync;
		}
		break;
	}

	return ERR_NONE;
}

GSM_Error OBEX_Initialise()
{
	GSM_Protocol_OBEXData *d = &theApp.m_ObexProtocolData;

	d->Msg.BufferUsed	= 0;
	d->Msg.Buffer 		= NULL;
	d->Msg.Length		= 0;

	d->MsgRXState 		= RX_Sync;

	return ERR_NONE;
}

GSM_Error OBEX_Terminate()
{
	if(theApp.m_ObexProtocolData.Msg.Buffer)
		free(theApp.m_ObexProtocolData.Msg.Buffer);
	return ERR_NONE;
}


void OBEXAddBlock(unsigned char *Buffer, int *Pos, unsigned char ID, unsigned char *AddBuffer, int AddLength)
{
	Buffer[(*Pos)++] = ID;
	Buffer[(*Pos)++] = (AddLength+3)/256;
	Buffer[(*Pos)++] = (AddLength+3)%256;
	if (AddBuffer != NULL) {
		memcpy(Buffer+(*Pos),AddBuffer,AddLength);
		(*Pos) += AddLength;
	}
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
