/* (c) 2002-2004 by Marcin Wiacek and Michal Cihar */
// At.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "At.h"
#include "atprotocol.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CAtApp

BEGIN_MESSAGE_MAP(CAtApp, CWinApp)
	//{{AFX_MSG_MAP(CAtApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAtApp construction

CAtApp::CAtApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_pDebugInfo = NULL;
	m_bFinishRequest = FALSE;
	m_DispatchError = ERR_TIMEOUT;
	m_pATMultiAnwser = NULL;
	m_bCurrentObexMode = FALSE;
	m_Speed =115200;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CAtApp object

CAtApp theApp;
GSM_Error WINAPI InitialiseLib(char* pszDeviceDllName,Debug_Info *debugInfo,GSM_Config ConfigInfo)
{
	GSM_Error error = ERR_NONE;
	theApp.m_pDebugInfo = debugInfo;
	error = theApp.m_DeviceAPI.InitDeviceData(theApp.m_pDebugInfo,ConfigInfo);
	if(error != ERR_NONE)
		return error;

	if(theApp.m_DeviceAPI.LoadDeviceApiLibrary(pszDeviceDllName)==FALSE)
		return ERR_BUG;
	return ERR_NONE;
}

GSM_Error WINAPI ProtocolInitialise(GSM_Config ConfigInfo)
{
	GSM_Error error = ERR_NONE;
	char szConnectType[MAX_PATH];


	wsprintf(szConnectType,"%s",ConfigInfo.Connection);


	if(theApp.m_DeviceAPI.m_DeviceData.ConnectionType == GCT_AT)
	{
		if (strlen(szConnectType) > 2) 
		{
			theApp.m_Speed= theApp.m_DeviceAPI.FindSerialSpeed(szConnectType+2);
			if(theApp.m_Speed == 0) theApp.m_Speed =115200;
		}
	}
	else if(theApp.m_DeviceAPI.m_DeviceData.ConnectionType == GCT_PHONEAT)
	{
		if (strlen(szConnectType) > 7) 
		{
			theApp.m_Speed= theApp.m_DeviceAPI.FindSerialSpeed(szConnectType+7);
			if(theApp.m_Speed == 0) theApp.m_Speed =115200;
		}
	}
	else if(theApp.m_DeviceAPI.m_DeviceData.ConnectionType == GCT_SYNCMLAT)
	{
		if (strlen(szConnectType) > 8) 
		{
			theApp.m_Speed= theApp.m_DeviceAPI.FindSerialSpeed(szConnectType+8);
			if(theApp.m_Speed == 0) theApp.m_Speed =115200;
		}
	}
	else if(theApp.m_DeviceAPI.m_DeviceData.ConnectionType == GCT_CABLEAT)
	{
		if (strlen(szConnectType) > 7) 
		{
			theApp.m_Speed= theApp.m_DeviceAPI.FindSerialSpeed(szConnectType+7);
			if(theApp.m_Speed == 0) theApp.m_Speed =115200;
		}
	}
	else if(theApp.m_DeviceAPI.m_DeviceData.ConnectionType == GCT_CABLEPHONEAT)
	{
		if (strlen(szConnectType) > 12) 
		{
			theApp.m_Speed= theApp.m_DeviceAPI.FindSerialSpeed(szConnectType+12);
			if(theApp.m_Speed == 0) theApp.m_Speed =115200;
		}
	}
	else if(theApp.m_DeviceAPI.m_DeviceData.ConnectionType == GCT_FTPAT)
	{
		if (strlen(szConnectType) > 5) 
		{
			theApp.m_Speed= theApp.m_DeviceAPI.FindSerialSpeed(szConnectType+5);
			if(theApp.m_Speed == 0) theApp.m_Speed =115200;
		}
	}

	if(theApp.m_bCurrentObexMode)
	{
		return OBEX_Initialise();
	}
	return AT_Initialise(theApp.m_Speed);

}
GSM_Error WINAPI Terminate()
{
	if(theApp.m_bCurrentObexMode)
	{
		return OBEX_Terminate();
	}
	return AT_Terminate();
}
GSM_Error WINAPI Open_Device()
{
	return theApp.m_DeviceAPI.DeviceAPI_OpenDevice();
}
GSM_Error WINAPI Close_Device()
{
	return theApp.m_DeviceAPI.DeviceAPI_CloseDevice();
}

int GSM_ReadDevice (bool wait)
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
		else
			AT_StateMachine(buff[count]);
	}

	return res;
}
GSM_Error WINAPI WriteCommand(unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg))
{
	theApp.m_bFinishRequest = FALSE;
	theApp.m_pReplyCheckType = ReplyCheckType;
	theApp.CommandCallBackFun = CallBackFun;
	theApp.m_pATMultiAnwser = pATMultiAnwser;
	
	GSM_Error error	= ERR_TIMEOUT;
	theApp.m_DispatchError = ERR_TIMEOUT;

	if(theApp.m_bCurrentObexMode != ObexMode)
	{
		Terminate();
		theApp.m_bCurrentObexMode = ObexMode;
		if(theApp.m_bCurrentObexMode)
			 OBEX_Initialise();
		else
			AT_Initialise(theApp.m_Speed);
		}

	if(theApp.m_bCurrentObexMode)
		error = OBEX_WriteMessage (buffer, length, type);
	else
		error = AT_WriteMessage (buffer, length, type);
	if (error!=ERR_NONE) return error;

	int				i;
	i=0;
#ifdef _SIEMENS_
	if(!theApp.m_bCurrentObexMode && theApp.m_DeviceAPI.m_DeviceData.ConnectionType == GCT_CABLEAT)
	{
		long  waitseconds1=1;
		clock_t waitTime1=0;

		waitTime1 = clock () + (WaitTime +2) * CLK_TCK ;
		while (i<WaitTime)
		{
			if(i>1) my_sleep(20);

			if (GSM_ReadDevice(true)!=0) i=0;
			if (theApp.m_bFinishRequest == TRUE) 
				return theApp.m_DispatchError;
			if(clock() > waitTime1)
				break;
			i++;
		} 
	}
	else
#endif
	{
#ifdef _LG_
		if(WaitTime==0 && !theApp.m_bCurrentObexMode)
		{
			Sleep(5);
			unsigned char	buff[300];
			ZeroMemory(buff,300);
			int		res = 0, count;
			res = theApp.m_DeviceAPI.DeviceAPI_ReadDevice(buff, 255);
			Sleep(5);
			if(res >0 && (strstr((char*)buff,"EXITEND") || strstr((char*)buff,"CME ERROR")))
				return ERR_CANCELED;

			for (count = 0; count < res; count++)
				AT_StateMachine(buff[count]);
		}
#endif

		while (i<WaitTime)
		{
			if(i>1) my_sleep(20);

			if (GSM_ReadDevice(true)!=0) i=0;
			if (theApp.m_bFinishRequest == TRUE) 
				return theApp.m_DispatchError;

			i++;
		} 
	}
	
	if(WaitTime==0 || ReplyCheckType == NULL) return ERR_NONE;
	return ERR_TIMEOUT;
}

GSM_Error WINAPI SetATProtocolData(bool EditMode,bool bFastWrite,DWORD dwFlag)
{
	return AT_SetProtocolData(EditMode, bFastWrite,dwFlag);
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

	return error;
}
