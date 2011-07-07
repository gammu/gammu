#include "..\stdafx.h"

class CTransportWrapper
{
	
// The actual APIs are accessed through wrapper classes,the advantage is that 
// if we need to support other vendors the underlaying APIs in the wrapper will change
// thus the real API and program code will not change
public:
	CString csBackupDataType;
public:
	CTransportWrapper();
	~CTransportWrapper();
	CAMMError StartUpload(CList<CString,CString> &list,CString csLogFileName, HWND hWnd=NULL, int nConnectMode=0, void *fptrCallback=NULL);
	CAMMError StartDownload(int nFileType,TCHAR	*csTargetDir,TCHAR *csLogFile,HWND hWnd, void *fptrCallback);
	
	
};