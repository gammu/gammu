// PhoneT.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "PhoneT.h"

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
// CPhoneTApp

BEGIN_MESSAGE_MAP(CPhoneTApp, CWinApp)
	//{{AFX_MSG_MAP(CPhoneTApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPhoneTApp construction

CPhoneTApp::CPhoneTApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_pDebugInfo = NULL;
	m_bFinishRequest = FALSE;
	m_DispatchError = ERR_TIMEOUT;
	m_pATMultiAnwser = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CPhoneTApp object

CPhoneTApp theApp;
GSM_Error CPhoneTApp::DispatchMessage(GSM_Protocol_Message* msg)
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
GSM_Error WINAPI SetATProtocolData(bool EditMode,bool bFastWrite,DWORD dwFlag)
{
	return ERR_NONE;
}

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
	return PHONET_Initialise();
}
GSM_Error WINAPI Terminate()
{
	return PHONET_Terminate();
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
		my_sleep(5);
	}
	while (clock() < waitTime1);

	for (count = 0; count < res; count++)
	{
		PHONET_StateMachine(buff[count]);
	}

	return res;
}
GSM_Error WINAPI WriteCommand(unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg))
{
	theApp.m_bFinishRequest = FALSE;
	theApp.m_pReplyCheckType = ReplyCheckType;
	theApp.CommandCallBackFun = CallBackFun;

	
	GSM_Error error	= ERR_TIMEOUT;
	theApp.m_DispatchError = ERR_TIMEOUT;
	error = PHONET_WriteMessage (buffer, length, type);
	if (error!=ERR_NONE) return error;

	int				i;
	i=0;
	while (i<WaitTime)
	{
		if(i>1) my_sleep(20);

		if (GSM_ReadDevice(true)!=0) i=0;
		if (theApp.m_bFinishRequest == TRUE) 
			return theApp.m_DispatchError;

		i++;
	}
	
	if(WaitTime==0) return ERR_NONE;
	return ERR_TIMEOUT;
}
