// Serial.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "Serial.h"
#include "ser_w32.h"

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
// CSerialApp

BEGIN_MESSAGE_MAP(CSerialApp, CWinApp)
	//{{AFX_MSG_MAP(CSerialApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSerialApp construction

CSerialApp::CSerialApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSerialApp object

CSerialApp theApp;


GSM_Error WINAPI OpenDevice(GSM_DeviceData *s,Debug_Info *debugInfo)
{
	return serial_open(s,debugInfo);

}
GSM_Error WINAPI CloseDevice(GSM_DeviceData *s,Debug_Info *debugInfo)
{
	return serial_close(s,debugInfo);
}
GSM_Error WINAPI DeviceSetParity(GSM_DeviceData *s, bool parity,Debug_Info *debugInfo)
{
	return serial_setparity(s,parity,debugInfo);
}
GSM_Error WINAPI DeviceSetDtrRts(GSM_DeviceData *s, bool dtr, bool rts,Debug_Info *debugInfo)
{
	return serial_setdtrrts(s,  dtr, rts,debugInfo);
}
GSM_Error WINAPI DeviceSetSpeed(GSM_DeviceData *s, int speed,Debug_Info *debugInfo)
{
	return serial_setspeed(s,speed,debugInfo);
}
int WINAPI ReadDevice(GSM_DeviceData *s, void *buf, size_t nbytes,Debug_Info *debugInfo)
{
	return serial_read(s, buf, nbytes,debugInfo);
}
int WINAPI WriteDevice(GSM_DeviceData *s, void *buf, size_t nbytes,Debug_Info *debugInfo)
{
	return serial_write(s, buf,  nbytes,debugInfo);
}
