
#include "StdAfx.h"
#include "TProcessStatusModule.h"


TProcessStatusModule::TProcessStatusModule()
                        :TDllModule(_T("PSAPI.DLL"))   //Mention the dll name here
{
  initAll();
}

TProcessStatusModule::~TProcessStatusModule()
{
}

void TProcessStatusModule::initAll(void)
{
	FSetBackupRestoreLog=NULL;
	FBackupData=NULL;
	FRestoreData=NULL;
	FSetUploadLog=NULL;
	FUploadFiles=NULL;
	FSetDownloadLog=NULL;
	FDownloadFiles=NULL;

	FLockSyncSetting=NULL;
	FUnLockSyncSetting=NULL;
	FCustomizeAddressFieldMap=NULL;
	FSetPhonePSW=NULL;
	FGetPhonePSW=NULL;
	FGetOutlkDefaultFolder=NULL;
	FBrowseOutlkFolder=NULL;
	FSetSyncLog=NULL;

	FSetCommCallBack=NULL;
	FSetBackupTypes=NULL;
	FOnCommStart=NULL;
	FOnCommEnd=NULL;
	
}

BOOL TProcessStatusModule::Create(void)
{
  if (TDllModule::Create()) {
    // DLL handle is valid (DLL is loaded)
    #ifdef UNICODE
		//Emergency Functions
		FSetBackupRestoreLog = (TFSetBackupRestoreLog)::GetProcAddress(m_hHandle,_T("SetBackupRestoreLogW"));
		FBackupData = (TFBackupData)::GetProcAddress(m_hHandle,_T("BackupDataW"));
		FRestoreData = (TFRestoreData)::GetProcAddress(m_hHandle,_T("RestoreDataW"));

		//Transport Functions
		FSetUploadLog=(TFSetUploadLog)::GetProcAddress(m_hHandle,_T("SetUploadLogW"));
		FUploadFiles=(TFUploadFiles)::GetProcAddress(m_hHandle,_T("UploadFilesW"));

		FSetDownloadLog=(TFSetDownloadLog)::GetProcAddress(m_hHandle,_T("SetDownloadLogW"));
		FDownloadFiles=(TFDownloadFiles)::GetProcAddress(m_hHandle,_T("DownloadFilesW"));

		FLockSyncSetting=(TFLockSyncSetting)::GetProcAddress(m_hHandle,_T("LockSyncSettingw"));
		FUnLockSyncSetting=(TFUnLockSyncSetting)::GetProcAddress(m_hHandle,_T("UnLockSyncSettingW"));
		FCustomizeAddressFieldMap=(TFCustomizeAddressFieldMap)::GetProcAddress(m_hHandle,_T("CustomizeAddressFieldMapW"));
		FSetPhonePSW=(TFSetPhonePSW)::GetProcAddress(m_hHandle,_T("SetPhonePSWW"));
		FGetPhonePSW=(TFGetPhonePSW)::GetProcAddress(m_hHandle,_T("GetPhonePSWW"));
		FGetOutlkDefaultFolder=(TFGetOutlkDefaultFolder)::GetProcAddress(m_hHandle,_T("GetOutlkDefaultFolderW"));
		FBrowseOutlkFolder=(TFBrowseOutlkFolder)::GetProcAddress(m_hHandle,_T("BrowseOutlkFolderW"));
		FSetSyncLog=(TFSetSyncLog)::GetProcAddress(m_hHandle,_T("SetSyncLogW"));


		FSetCommCallBack=(TFSetCommCallBack)::GetProcAddress(m_hHandle,_T("SetCommCallBackW"));
		FOnCommStart=(TFOnCommStart)::GetProcAddress(m_hHandle,_T("OnCommStartW"));
		FOnCommEnd=(TFOnCommEnd)::GetProcAddress(m_hHandle,_T("OnCommEndW"));
		FSetBackupTypes=(TFSetBackupTypes)::GetProcAddress(m_hHandle,_T("SetBackupTypesW"));

    #else
		//Emergency Functions
		FSetBackupRestoreLog = (TFSetBackupRestoreLog)::GetProcAddress(m_hHandle,_T("SetBackupRestoreLogA"));
		FBackupData = (TFBackupData)::GetProcAddress(m_hHandle,_T("BackupDataA"));
		FRestoreData = (TFRestoreData)::GetProcAddress(m_hHandle,_T("RestoreDataA"));

		//Transport Functions
		FSetUploadLog=(TFSetUploadLog)::GetProcAddress(m_hHandle,_T("SetUploadLogA"));
		FUploadFiles=(TFUploadFiles)::GetProcAddress(m_hHandle,_T("UploadFilesA"));

		FSetDownloadLog=(TFSetDownloadLog)::GetProcAddress(m_hHandle,_T("SetDownloadLogA"));
		FDownloadFiles=(TFDownloadFiles)::GetProcAddress(m_hHandle,_T("DownloadFilesA"));

		FLockSyncSetting=(TFLockSyncSetting)::GetProcAddress(m_hHandle,_T("LockSyncSettingA"));
		FUnLockSyncSetting=(TFUnLockSyncSetting)::GetProcAddress(m_hHandle,_T("UnLockSyncSettingA"));
		FCustomizeAddressFieldMap=(TFCustomizeAddressFieldMap)::GetProcAddress(m_hHandle,_T("CustomizeAddressFieldMapA"));
		FSetPhonePSW=(TFSetPhonePSW)::GetProcAddress(m_hHandle,_T("SetPhonePSWA"));
		FGetPhonePSW=(TFGetPhonePSW)::GetProcAddress(m_hHandle,_T("GetPhonePSWA"));
		FGetOutlkDefaultFolder=(TFGetOutlkDefaultFolder)::GetProcAddress(m_hHandle,_T("GetOutlkDefaultFolderA"));
		FBrowseOutlkFolder=(TFBrowseOutlkFolder)::GetProcAddress(m_hHandle,_T("BrowseOutlkFolderA"));
		FSetSyncLog=(TFSetSyncLog)::GetProcAddress(m_hHandle,_T("SetSyncLogA"));


		FSetCommCallBack=(TFSetCommCallBack)::GetProcAddress(m_hHandle,_T("SetCommCallBackA"));
		FOnCommStart=(TFOnCommStart)::GetProcAddress(m_hHandle,_T("OnCommStartA"));
		FOnCommEnd=(TFOnCommEnd)::GetProcAddress(m_hHandle,_T("OnCommEndA"));
		FSetBackupTypes=(TFSetBackupTypes)::GetProcAddress(m_hHandle,_T("SetBackupTypesA"));

    #endif
    // Can also check if all pointers to functions are different from NULL.
    return TRUE;
  }
  return FALSE;
}

void TProcessStatusModule::Destroy(void)
{
  TDllModule::Destroy();
  initAll();
}




//Start of Emergency functions

void TProcessStatusModule::SetBackupRestoreLog(LPCTSTR lpszBackupLog,LPCTSTR lpszRestoreLog)
{
//	ASSERT(FSetBackupRestoreLog != NULL);
	if (FSetBackupRestoreLog) 
      FSetBackupRestoreLog(lpszBackupLog,lpszRestoreLog);
}
int TProcessStatusModule::BackupData(LPCTSTR lpszTempFile)
{
//	ASSERT(FBackupData != NULL);
	if (FBackupData) 
      return FBackupData(lpszTempFile);
	return 0;
}
int TProcessStatusModule::RestoreData(LPCTSTR lpszRestoreFile)
{
//	ASSERT(FRestoreData != NULL);
	if (FRestoreData) 
      return FRestoreData(lpszRestoreFile);
	return 0;
}

   
//End of Emergency functions



//Start of Transport functions

void TProcessStatusModule::SetUploadLog(LPCTSTR lpszUploadLog)
{
//	ASSERT(FSetUploadLog != NULL);
	if (FSetUploadLog) 
      FSetUploadLog(lpszUploadLog);
}


int TProcessStatusModule::UploadFiles(LPCTSTR lpszResuleFile)
{
//	ASSERT(FUploadFiles != NULL);
	if (FUploadFiles) 
      FUploadFiles(lpszResuleFile);

	return 0;
}
   


void TProcessStatusModule::SetDownLoadLog(LPCTSTR lpszDownloadLog)
{
//	ASSERT(FSetDownloadLog != NULL);
	if (FSetDownloadLog) 
      FSetDownloadLog(lpszDownloadLog);
}


int TProcessStatusModule::DownloadFiles(LPCTSTR lpszRSPFile)
{
//	ASSERT(FDownloadFiles != NULL);
	if (FDownloadFiles) 
      FDownloadFiles(lpszRSPFile);

	return 0;
}
   
//End of Transport functions

//start of sync functions


BOOL TProcessStatusModule::LockSyncSetting (LPEXP_SYNCSETTING *ppSyncSetting,UINT& cbBytes)
{
	//	ASSERT(FLockSyncSetting != NULL);
	if (FLockSyncSetting) 
      FLockSyncSetting(ppSyncSetting,cbBytes);
	return 0;
}
BOOL TProcessStatusModule::UnLockSyncSetting (LPEXP_SYNCSETTING pSyncSetting)
{
	//	ASSERT(FUnLockSyncSetting != NULL);
	if (FUnLockSyncSetting) 
      FUnLockSyncSetting(pSyncSetting);
	return 0;
}
BOOL TProcessStatusModule::CustomizeAddressFieldMap()
{
	//	ASSERT(FCustomizeAddressFieldMap != NULL);
	if (FCustomizeAddressFieldMap) 
      FCustomizeAddressFieldMap();
	return 0;
}
BOOL TProcessStatusModule::SetPhonePSW (LPCTSTR lpszPsw)
{
	//	ASSERT(FSetPhonePSW != NULL);
	if (FSetPhonePSW) 
      FSetPhonePSW(lpszPsw);
	return 0;
}
LPCTSTR TProcessStatusModule::GetPhonePSW ()
{
	//	ASSERT(FGetPhonePSW != NULL);
	if (FGetPhonePSW) 
      FGetPhonePSW();
	return _T("");
}
int TProcessStatusModule::GetOutlkDefaultFolder(LPTSTR pBuffer,UINT cbBytes)
{
	//	ASSERT(FGetOutlkDefaultFolder != NULL);
	if (FGetOutlkDefaultFolder) 
      FGetOutlkDefaultFolder(pBuffer,cbBytes);
	return 0;
}
int TProcessStatusModule::BrowseOutlkFolder(LPTSTR pBuffer,UINT cbBytes)
{
	//	ASSERT(FBrowseOutlkFolder != NULL);
	if (FBrowseOutlkFolder) 
      FBrowseOutlkFolder(pBuffer,cbBytes);
	return 0;
}
void TProcessStatusModule::SetSyncLog(LPCTSTR lpszLogFile)
{
	//	ASSERT(FSetSyncLog != NULL);
	if (FSetSyncLog) 
      FSetSyncLog(lpszLogFile);
}
int TProcessStatusModule::SyncData(LPCTSTR lpszUserName)
{
	//	ASSERT(FSyncData != NULL);
	if (FSyncData) 
      FSyncData(lpszUserName);
	return 0;
}

//end of sync functions


//Start of general functions
BOOL TProcessStatusModule::SetCommCallBack(DWORD dwCookie, void *lpfnCallBack)
{
	//	ASSERT(FSetCommCallBack != NULL);
	if (FSetCommCallBack) 
      FSetCommCallBack(dwCookie,lpfnCallBack);

	return TRUE;
}


int TProcessStatusModule::OnCommStart(DWORD dwCommCmd,DWORD lParam)
{
	//	ASSERT(FOnCommStart != NULL);
	if (FOnCommStart) 
      FOnCommStart(dwCommCmd,lParam);

	return 0;
}


int TProcessStatusModule::OnCommEnd(DWORD dwCommCmd,DWORD lParam)
{
	//	ASSERT(FOnCommEnd != NULL);
	if (FOnCommEnd) 
      OnCommEnd(dwCommCmd,lParam);

	return 0;
}


BOOL TProcessStatusModule::SetBackupTypes(LPCTSTR lpszUniDatabase)
{
	//	ASSERT(FSetBackupTypes != NULL);
	if (FSetBackupTypes) 
      FSetBackupTypes(lpszUniDatabase);
	return 0;
}

//End of general functions