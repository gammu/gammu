/* (c) 2002-2004 by Marcin Wiacek and Michal Cihar */




GSM_Error WINAPI Terminate()
{
	if(theApp.m_bCurrentObexMode)
	{
		return OBEX_Terminate();
	}
	return AT_Terminate();
}

int GSM_ReadDevice (bool wait)	   //by karl
{
	unsigned char	buff[300];
	int		res = 0, count;
	long  waitseconds1=1;
	clock_t waitTime1=0;

	if(wait)
			waitTime1 = clock () + waitseconds1 * CLK_TCK ;

	do 
	{
		res = theApp.m_DeviceAPI.DeviceAPI_ReadDevice(buff, 255);
		if (!wait) break;
		if (res > 0) break;
	//	my_sleep(5);
		Sleep(5);
	}
	while (clock() < waitTime1);

	for (count = 0; count < res; count++)
	{
		if(theApp.m_bCurrentObexMode)
			OBEX_StateMachine(buff[count]);
		else // modify for GX 18 AVFL command
		{
		  if ((theApp.m_pATMultiAnwser) &&
			  ( ( !strcmp(theApp.m_pATMultiAnwser->Specialtext,"%AVFL:") ) ||
			    ( !strcmp(theApp.m_pATMultiAnwser->Specialtext,"%ARF:") ) ) )
			AT_StateMachine2(buff[count]);
		  else
			AT_StateMachine(buff[count]);
		}
	}

	return res;
}

GSM_Error CAtApp::DispatchMessage(GSM_Protocol_Message *msg)
{
	GSM_Error		error	= ERR_UNKNOWNFRAME;

	int i;
	if(m_pDebugInfo)
	{
		if (m_pDebugInfo->dl==DL_TEXT || m_pDebugInfo->dl==DL_TEXTALL ||
			m_pDebugInfo->dl==DL_TEXTDATE || m_pDebugInfo->dl==DL_TEXTALLDATE)
		{
			smprintf(m_pDebugInfo, "RECEIVED frame ");
			smprintf(m_pDebugInfo, "type 0x%02X/length 0x%02X/%i", msg->Type, msg->Length, msg->Length);
	//		DumpMessage(s->di.use_global ? di.df : s->di.df, s->di.dl, msg.Buffer, msg.Length);
			if (msg->Length == 0) smprintf(m_pDebugInfo, "\n");
			fflush(m_pDebugInfo->df);
		}
		if (m_pDebugInfo->dl==DL_BINARY) {
			smprintf(m_pDebugInfo,"%c",0x02);	/* Receiving */
			smprintf(m_pDebugInfo,"%c",msg->Type);
			smprintf(m_pDebugInfo,"%c",msg->Length/256);
			smprintf(m_pDebugInfo,"%c",msg->Length%256);
			for (i=0;i<msg->Length;i++) {
				smprintf(m_pDebugInfo,"%c",msg->Buffer[i]);
			}
		}
	}
	if(m_pReplyCheckType)
	{
		error=CheckReplyFunctions(*m_pReplyCheckType,msg);
		if (error==ERR_NONE && CommandCallBackFun) 
		{
			error=CommandCallBackFun(*msg);
			if (error == ERR_NEEDANOTHERANSWER) 
				error = ERR_NONE;
			else if(error !=ERR_NEEDANOTHEDATA)
			{
				m_bFinishRequest=TRUE;
			}
		}
	}
	else
		error = ERR_NONE;	
/*
	if (strcmp(s->Phone.Functions->models,"NAUTO"))
	{
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE)
		{
			disp = true;
			switch (error) 
			{
			case ERR_UNKNOWNRESPONSE:
				smprintf(s, "\nUNKNOWN response");
				break;
			case ERR_UNKNOWNFRAME:
				smprintf(s, "\nUNKNOWN frame");
				break;
			case ERR_FRAMENOTREQUESTED:
				smprintf(s, "\nFrame not request now");
				break;
			default:
				disp = false;
			}
		}

		if (error == ERR_UNKNOWNFRAME || error == ERR_FRAMENOTREQUESTED)
		{
			if(Phone->RequestID==ID_None)
				error = ERR_NONE;
			else
				error = ERR_TIMEOUT;
		}
	}
*/
/*	if (disp) 
	{
		smprintf(s,". If you can, PLEASE report it (see readme.txt). THANK YOU\n");
		if (Phone->SentMsg != NULL) {
			smprintf(s,"LAST SENT frame ");
			smprintf(s, "type 0x%02X/length %i", Phone->SentMsg->Type, Phone->Sentmsg.Length);
			DumpMessage(s->di.use_global ? di.df : s->di.df, s->di.dl, Phone->Sentmsg.Buffer, Phone->Sentmsg.Length);
		}
		smprintf(s, "RECEIVED frame ");
		smprintf(s, "type 0x%02X/length 0x%02X/%i", msg->Type, msg.Length, msg.Length);
		DumpMessage(s->di.use_global ? di.df : s->di.df, s->di.dl, msg.Buffer, msg.Length);
		smprintf(s, "\n");
	}*/

	return error;
}
