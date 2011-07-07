#include "stdafx.h"
#include "TransportWrapper.h"
#include "..\\OKWAP\\Download\\inc\\flow.h"


CTransportWrapper::CTransportWrapper()
{
}

CTransportWrapper::~CTransportWrapper()
{
}

//This is the upload program from mobile's perspective
CAMMError CTransportWrapper::StartDownload(int nFileType,TCHAR	*csTargetDir,TCHAR *csLogFile, HWND hWnd, void *fptrCallback)
{	

	CString s;
	CString cus;


	s.Format("%s",csLogFile);
	cus = s.GetBuffer(1);

	SetUploadLog(csLogFile);
	
	s.Format("%d",nFileType);
	cus = s.GetBuffer(1);
	s.Format("%s",csTargetDir);
	cus = s.GetBuffer(1);


	s.Format("0x%x",hWnd);
	cus = s.GetBuffer(1);


	//Temporary stop detecting "TreadFunc" [Nono, 2004_0912]
	gpIEShellListCtrl->SetNotificationObject(false);
	int nRet =	UploadFiles(nFileType,"CAMM",(LPCTSTR)csTargetDir,(LPCTSTR)csLogFile,hWnd);
	gpIEShellListCtrl->Refresh();
	gpIEShellListCtrl->SetNotificationObject(true);
    //
	s.Format("%d",nRet);
	cus = s.GetBuffer(1);
	if(nRet)
	{
		return CAMM_ERROR_SUCCESS;
	}
	else
	{
		return CAMM_ERROR_FAILURE;
	}
		
}


CAMMError CTransportWrapper::StartUpload(CList<CString,CString> &filelist,CString csLogFileName, HWND handleWindow, int nConnectMode, void *fptrCallback)
{
//Sanjeev 271103
	HWND hWnd = NULL;
	if(handleWindow != NULL)
	{
		hWnd = handleWindow;
	}
	else
	{
		hWnd = AfxGetMainWnd()->m_hWnd;
	}
//Sanjeev 271103
	int nFileCount = filelist.GetCount();
	
	TCHAR *tcNameBuffer=new TCHAR[MAX_PATH * nFileCount];
	memset(tcNameBuffer,0x00,sizeof(TCHAR) * (MAX_PATH * nFileCount));

	POSITION posNext;
	posNext = filelist.GetHeadPosition();

	_tcscat(tcNameBuffer,"CAMM");
		_tcscat(tcNameBuffer,"\r\n");

	while(posNext)
	{
		_tcscat(tcNameBuffer,filelist.GetNext(posNext));
		_tcscat(tcNameBuffer,"\r\n");
	}

	//to upload flies from PC to Mobile the API is DownloadFiles
	CString s;
	CString cus;

	SetDownloadLog(csLogFileName);

	//Nono => 2004_0819
	//= Get Card-Info =//
/*
    	CString csCardInfo = GetCardInfo();
    	bool bLinkCard = true;
    	if ( csCardInfo.IsEmpty()) {
    		bLinkCard = false;
		}
    	//Switch between of USB-cable mode & Data-Cable.
    	if (!bLinkCard) {
		}else {
		}
	*/
  
	//nono, 2004_0910
	CString csCardInfo = GetCardInfo();
	if (1 == nConnectMode) {
    	//USB-cable mode
    	bool bLinkCard = true;
    	if ( csCardInfo.IsEmpty()) {
    		bLinkCard = false;
		}
    	if (bLinkCard) {
		}
	}
    //Nono <= 	

	


	s.Format("0x%x",hWnd);
	cus = s.GetBuffer(1);

//	HANDLE hDownloadEvent;
//	hDownloadEvent = CreateEvent(NULL,FALSE,FALSE,"Download");
//	WaitForSingleObject

/*
	//Nono => 2004_0819
	//Switch between of USB-cable mode & Data-Cable.
	int nRet = 0;
	if (!bLinkCard) {
    	nRet = DownloadFiles(tcNameBuffer,hWnd);
	}else {
		nRet = DownloadFilesToCard( tcNameBuffer, hWnd, csCardInfo); ;
	}
    //Nono <= 	
*/
	//Nono, 2004_0910
	int nRet = 0;
    if (0 == nConnectMode){
    	nRet = DownloadFiles(tcNameBuffer,hWnd);
	}else if (1 == nConnectMode) {
		nRet = DownloadFilesToCard( tcNameBuffer, hWnd, csCardInfo); ;
	}else {
		CString msg;
		msg.Format("None valid value of nConnectMode!? [&d]", nConnectMode);
		AfxMessageBox(msg);
	}
    
	
	s.Format("%d",nRet);
	cus = s.GetBuffer(1);
	if (0 == nConnectMode) {
	}else if (1 == nConnectMode) {
	}



	delete []tcNameBuffer;
	tcNameBuffer = NULL;
	filelist.RemoveAll();


	if(nRet)
	{
		return CAMM_ERROR_SUCCESS;
	}
	else
	{
		return CAMM_ERROR_FAILURE;
	}
}










