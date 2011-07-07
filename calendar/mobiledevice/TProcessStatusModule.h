#include "..\StdAfx.h"
#include "DllLoad.h"

#define		COMMCMD_STARTSYNC				1
#define		COMMCMD_ENDSYNC					2
#define		COMMCMD_STARTDOWNLOAD			3
#define		COMMCMD_ENDDOWNLOAD				4
#define		COMMCMD_STARTBACKUP				5
#define		COMMCMD_ENDBACKUP				6
#define		COMMCMD_STARTRESTORE			7
#define		COMMCMD_ENDRESTORE				8
#define		COMMCMD_STARTUPLOAD				9
#define		COMMCMD_ENDUPLOAD				10

//functions for backup and restore files 
typedef	void (WINAPI *TFSetBackupRestoreLog)(LPCTSTR lpszBackupLog,LPCTSTR lpszRestoreLog);
typedef	int (WINAPI *TFBackupData)(LPCTSTR lpszTempFile);
typedef	int (WINAPI *TFRestoreData)(LPCTSTR lpszRestoreFile);


//functions for upload and download files 
typedef	void (WINAPI * TFSetUploadLog)(LPCTSTR lpszUploadLog);
typedef	int (WINAPI * TFUploadFiles)(LPCTSTR lpszResuleFile);
typedef	void (WINAPI * TFSetDownloadLog)(LPCTSTR lpszDownloadLog);
typedef	int (WINAPI * TFDownloadFiles)(LPCTSTR lpszRSPFile);



//functions for sync  
typedef	BOOL (WINAPI *TFLockSyncSetting) (LPEXP_SYNCSETTING *ppSyncSetting,UINT& cbBytes);
typedef	BOOL (WINAPI *TFUnLockSyncSetting) (LPEXP_SYNCSETTING pSyncSetting);
typedef	BOOL (WINAPI *TFCustomizeAddressFieldMap)();
typedef	BOOL (WINAPI *TFSetPhonePSW)(LPCTSTR lpszPsw);
typedef	LPCTSTR (WINAPI *TFGetPhonePSW)();
typedef	int (WINAPI *TFGetOutlkDefaultFolder)(LPTSTR pBuffer,UINT cbBytes);;
typedef	int (WINAPI *TFBrowseOutlkFolder)(LPTSTR pBuffer,UINT cbBytes);
typedef	void (WINAPI *TFSetSyncLog)(LPCTSTR lpszLogFile);
typedef	int (WINAPI *TFSyncData)(LPCTSTR lpszUserName);


//general functions
typedef BOOL (WINAPI * TFSetCommCallBack)(DWORD dwCookie, void *lpfnCallBack);
typedef int (WINAPI * TFOnCommStart)(DWORD dwCommCmd,DWORD lParam);
typedef int (WINAPI * TFOnCommEnd)(DWORD dwCommCmd,DWORD lParam);
typedef BOOL (WINAPI * TFSetBackupTypes)(LPCTSTR lpszUniDatabase);
   


class TProcessStatusModule : public TDllModule {
private:

	TFSetBackupRestoreLog	FSetBackupRestoreLog;
	TFBackupData	FBackupData;
	TFRestoreData   FRestoreData;
	TFSetUploadLog FSetUploadLog;	
	TFUploadFiles FUploadFiles;
	TFSetDownloadLog FSetDownloadLog;	
	TFDownloadFiles FDownloadFiles;

	TFLockSyncSetting FLockSyncSetting;
	TFUnLockSyncSetting FUnLockSyncSetting;
	TFCustomizeAddressFieldMap FCustomizeAddressFieldMap;
	TFSetPhonePSW FSetPhonePSW;
	TFGetPhonePSW FGetPhonePSW;
	TFGetOutlkDefaultFolder FGetOutlkDefaultFolder;
	TFBrowseOutlkFolder FBrowseOutlkFolder;
	TFSetSyncLog FSetSyncLog;
	TFSyncData FSyncData;

	TFSetCommCallBack FSetCommCallBack;
	TFOnCommStart FOnCommStart;
	TFOnCommEnd FOnCommEnd;
	TFSetBackupTypes FSetBackupTypes;


private:
    void initAll(void);

public:
    TProcessStatusModule();
    virtual ~TProcessStatusModule();

    virtual BOOL Create(void);
    virtual void Destroy(void);
    
	//functions for backup and restore files
	void SetBackupRestoreLog(LPCTSTR lpszBackupLog,LPCTSTR lpszRestoreLog);
	int BackupData(LPCTSTR lpszTempFile);
	int RestoreData(LPCTSTR lpszRestoreFile);

	
	//functions for upload and download files 
	void SetUploadLog(LPCTSTR lpszUploadLog);
	int UploadFiles(LPCTSTR lpszResuleFile);
	void SetDownLoadLog(LPCTSTR lpszDownloadLog);
	int DownloadFiles(LPCTSTR lpszRSPFile);

	
	//functions for sync  
	BOOL LockSyncSetting (LPEXP_SYNCSETTING *ppSyncSetting,UINT& cbBytes);
	BOOL UnLockSyncSetting (LPEXP_SYNCSETTING pSyncSetting);
	BOOL CustomizeAddressFieldMap();
	BOOL SetPhonePSW (LPCTSTR lpszPsw);
	LPCTSTR GetPhonePSW ();
	int GetOutlkDefaultFolder(LPTSTR pBuffer,UINT cbBytes);;
	int BrowseOutlkFolder(LPTSTR pBuffer,UINT cbBytes);
	void SetSyncLog(LPCTSTR lpszLogFile);
	int SyncData(LPCTSTR lpszUserName);

	
	//general functions
	BOOL SetCommCallBack(DWORD dwCookie, void *lpfnCallBack);
	int OnCommStart(DWORD dwCommCmd,DWORD lParam);
	int OnCommEnd(DWORD dwCommCmd,DWORD lParam);
	BOOL SetBackupTypes(LPCTSTR lpszUniDatabase);


   
};