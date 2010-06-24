// IRDA.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "IRDA.h"
#include "irda_w32.h"
#include "devfunc.h"

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
// CIRDAApp

BEGIN_MESSAGE_MAP(CIRDAApp, CWinApp)
	//{{AFX_MSG_MAP(CIRDAApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIRDAApp construction

CIRDAApp::CIRDAApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CIRDAApp object

CIRDAApp theApp;


GSM_Error WINAPI OpenDevice(GSM_DeviceData *s,Debug_Info *debugInfo)
{
	return irda_open(s,debugInfo);
}
GSM_Error WINAPI CloseDevice(GSM_DeviceData *s,Debug_Info *debugInfo)
{
	return irda_close(s, debugInfo);
}
GSM_Error WINAPI DeviceSetParity(GSM_DeviceData *s, bool parity,Debug_Info *debugInfo)
{
	return ERR_NONE;
}
GSM_Error WINAPI DeviceSetDtrRts(GSM_DeviceData *s, bool dtr, bool rts,Debug_Info *debugInfo)
{
	return ERR_NONE;
}
GSM_Error WINAPI DeviceSetSpeed(GSM_DeviceData *s, int speed,Debug_Info *debugInfo)
{
	return ERR_NONE;
}
int WINAPI ReadDevice(GSM_DeviceData *s, void *buf, size_t nbytes,Debug_Info *debugInfo)
{
	return socket_read(s, buf, nbytes, s->hSocketPhone);
}
int WINAPI WriteDevice(GSM_DeviceData *s, void *buf, size_t nbytes,Debug_Info *debugInfo)
{
	return socket_write(s, (unsigned char *)buf, nbytes, s->hSocketPhone,debugInfo);
}
