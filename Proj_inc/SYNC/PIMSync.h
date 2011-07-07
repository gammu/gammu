// PIMSync.h: 
//
//////////////////////////////////////////////////////////////////////
#if !defined(_PIMSYNC1000_H_)
#define _PIMSYNC1000_H_

#ifdef SYNC_DLL_EXPORTS
#define SYNC_DLL_API __declspec(dllexport)
#else
#define SYNC_DLL_API __declspec(dllimport)
#endif
#include "..\anwmobile\anwmobile.h"
#include <iterator>
#include <vector>
#include <string>
#include <map>
#include <afxtempl.h>
using namespace std;

#define MAILSYSTEM_MSOUTLOOK		0x00000001
#define MAILSYSTEM_OUTLOOKEXPRESS	0x00000002
#define MAILSYSTEM_LOTUSNOTES6		0x00000004
#define MAILSYSTEM_WINDOWSCONTACTS		0x00000008

enum	{	//use for nSyncMethod
	Sync_To_Mobile = 1,
	Sync_From_Mobile,
	Sync_Both
};

enum	{	//use for nConflictMethod
	Conflict_Add_All = 1,	//not use in new spec
	Conflict_Ignore,
	Conflict_Warring,
	Conflict_Replace_PC,
	Conflict_Replace_Mobile
};

enum	{	//use for nConflictMethod
	SyncErr_No_Err = 1,
	SyncErr_User_Cancel = -1,	//not use in new spec
	SyncErr_Outlook_Err = -2,
	SyncErr_Mobile_Err = -3,
	SyncErr_DB_Err = -4,
	SyncErr_Mobile_Full = -5,
	SyncErr_Mobile_Time = -6	//In Sync Calendar, can't get mobile time

};

//for GetOutlookProfileList() use.
class _OutlookFolder	{
public:
	_OutlookFolder() 
	{
		dwTypeFlag=0; 
	};
	_OutlookFolder(_OutlookFolder* theData)
	{
		sName=theData->sName;
		sEntryID=theData->sEntryID;
		sStoreID=theData->sStoreID;
		dwTypeFlag = theData->dwTypeFlag;
	};
/*	std::string	sName;
	std::string	sEntryID;
	std::string	sStoreID;*/
	CString sName;
	CString sEntryID;
	CString sStoreID;
	DWORD dwTypeFlag;	//FolderTypeFlag
};

//for GetOutlookProfileList() use.
typedef std::vector<_OutlookFolder>	OutlookFolderList;

typedef struct OutlookSetting_TAG
{
	int nSyncMethod;
	int	nConflictMethod;
	_OutlookFolder ofFolder;
	DATE	dtStart;
	DATE	dtEnd;
	int nMailSystem;
} SYNCSetting;// ContactSetting, NoteSetting;
/*
typedef struct CalendarSetting_TAG
{
	int nSyncMethod;
	int	nConflictMethod;
	_OutlookFolder ofFolder;
	DATE	dtStart;
	DATE	dtEnd;
} CalendarSetting;
*/
enum _QueryReturn	{	//for fnQueryCallback return
	Cft_Ignore,		//QueryConflict Answer
	Cft_Mobile,		//QueryConflict Answer
	Cft_Outlook,	//QueryConflict Answer
	Del_Restore,	//QueryDelMobile, QueryDelOutlook and QueryDelBoth Answer
	Del_Delete,		//QueryDelMobile, QueryDelOutlook and QueryDelBoth Answer
	Del_Cancel		//QueryDelMobile, QueryDelOutlook and QueryDelBoth Answer
	
};

enum QueryMode	{	//for fnQueryCallback Mode
	QueryConflict = 0,
	QueryDelMobile,
	QueryDelOutlook,
	QueryDelBoth
};

enum		{
	MailFolder = 0,
	CalendarFolder,
	ContactFolder,
	ToDoListFolder,
	WorkFolder,
	MemoFolder
};
typedef struct
{
	CString	sName;
	CString	sEntryID;
	CString	sStoreID;
	DWORD dwTypeFlag;	//FolderTypeFlag
}OutlookFolder, *POutlookFolder;

//for GetOutlookProfileList() use.
enum	FolderTypeFlag	{
	MailFolder_FLAG = 0x0001,
	CalendarFolder_FLAG = 0x0002,
	ContactFolder_FLAG = 0x0004,
	ToDoListFolder_FLAG = 0x0008,
	MemoFolder_FLAG = 0x0010,
	WorkFolder_FLAG = 0x0020
};
enum 
{
	OUTLOOK,

};
//QueryConflict: szFullname-NULL, otData-outlook fullname or subject, mbData-mobile fullname or subject
//QueryDelMobile: szFullname-NULL, otData-NULL, mbData-int(delete numbers of Mobile)
//QueryDelOutlook: szFullname-NULL, otData-int(delete numbers of Outlook), mbData-NULL
//QueryDelBoth: szFullname-NULL, otData-int(delete numbers of Outlook), mbData-int(delete numbers of Mobile)
typedef int(*fnQueryCallback)(LPCTSTR szFullname, void* otData=NULL, void* mbData=NULL, int nMode=QueryConflict);
typedef int(*fnProgressCallback)(int nPos);		//for progress & connection
typedef BOOL(*fnStartConnect)(fnProgressCallback pfnConnect);	

extern "C"
{
	//-----Gereral API-----
	SYNC_DLL_API BOOL  StartConnect(fnProgressCallback pfnConnect);	//Start connect
	//nType-use OutlookFolder value, 
	SYNC_DLL_API DWORD GetOutlookProfileList(CPtrList* pList,DWORD dwMailSystem);
	//need delete Item data in every child, type is _OutlookFolder*
	SYNC_DLL_API DWORD BuildOutlookTree(void* pOutlookTree,CString strFolderEntryID, int nType, LPCTSTR szSelEntry);	//Get outlook folder tree
	//return value is _OutlookFolder*
	SYNC_DLL_API DWORD GetOutlookFolder(int nIndex);	//Get outlook folder
	SYNC_DLL_API DWORD GetOutlookFolderFirstEntryID(CString strFolderEntryID, int nType, CString szFirstEntry, CString szStoreID);
	
	SYNC_DLL_API DWORD InitOleApp(DWORD dwMailSystem);
	SYNC_DLL_API DWORD ReleaseOleApp(DWORD dwMailSystem);

	SYNC_DLL_API DWORD GetOutlookFolderList(CString strFolderEntryID, int nType,CPtrList *pFolderList,DWORD dwMailSystem);

//	SYNC_DLL_API LPDISPATCH GetOutlookFolder(LPCTSTR szFolderPath, int nType);
	//-----Contact Sync API-----
	//return 1:sync sucess, others:failure.
	SYNC_DLL_API DWORD SyncContact(
			SYNCSetting* lpSetting, 
			volatile BOOL* bStop, 
			fnProgressCallback pfnProgress, 
			fnQueryCallback pfnConflict );

	//-----Calendar Sync API-----
	SYNC_DLL_API DWORD SyncCalendar(
			SYNCSetting* lpSetting, 
			volatile BOOL* bStop, 
			fnProgressCallback pFn,
			fnQueryCallback pfnConflict );
	SYNC_DLL_API DWORD SyncTask(
			SYNCSetting* lpSetting, 
			volatile BOOL* bStop, 
			fnProgressCallback pFn,
			fnQueryCallback pfnConflict );

	//-----Note Sync API-----
	//return 1:sync sucess, others:failure.
	SYNC_DLL_API DWORD SyncNote(
			SYNCSetting* lpSetting, 
			volatile BOOL* bStop, 
			fnProgressCallback pfnProgress, 
			fnQueryCallback pfnConflict );

	SYNC_DLL_API int GetMobileSupportFunction(Support_Mobile_Function *ColumnList);

	SYNC_DLL_API int InitSyncSession(SYNCSetting* lpPhonebookSetting,SYNCSetting* lpCalendarSetting,SYNCSetting* lpTaskSetting,SYNCSetting* lpMemoSetting,SYNCSetting* lpMailSetting);
	SYNC_DLL_API int TerminateSyncSession();
	SYNC_DLL_API int GetSyncInitData(int* PhonebookGetStatus , int *CalStatus,int* TaskStatus, int* MemoStatus,int* mailStatus,volatile BOOL* bStop);

	SYNC_DLL_API int InitSyncEngine(HMODULE hAnwMobileModule , DWORD& dwSupport);
	SYNC_DLL_API BOOL GetMailSytemDefaultDataBase(CString &strFolderID ,CString &cDatabaseName, int nType,DWORD dwMailSystem);


}
typedef DWORD (*fnGetOutlookFolderList)(CString strFolderEntryID, int nType,CPtrList *pFolderList,DWORD dwMailSystem);

typedef DWORD(*fnGetOutlookProfileList)(CPtrList* pList,DWORD dwMailSystem);
typedef DWORD(*fnBuildOutlookTree)(void* pOutlookTree,CString strFolderEntryID, int nType, LPCTSTR szSelEntry);
typedef DWORD(*fnGetOutlookFolder)(int nIndex);
typedef DWORD (*fnGetOutlookFolderFirstEntryID)(CString strFolderEntryID, int nType, CString szFirstEntry, CString szStoreID);

typedef	DWORD (*fnInitOleApp)(DWORD dwMailSystem);
typedef	DWORD (*fnReleaseOleApp)(DWORD dwMailSystem);

//Contact Sync API
typedef DWORD(*fnSyncContact)(SYNCSetting* lpSetting, volatile BOOL* bStop, fnProgressCallback pfnProgress, fnQueryCallback pfnConflict);
//pOutlookTree must be a CTreeCtrl class object
typedef DWORD(*fnSyncCalendar)(SYNCSetting* lpSetting, volatile BOOL* bStop, fnProgressCallback pfnProgress, fnQueryCallback pfnConflict);
//Contact Sync API
typedef DWORD(*fnSyncNote)(SYNCSetting* lpSetting, volatile BOOL* bStop, fnProgressCallback pfnProgress, fnQueryCallback pfnConflict);

typedef int(*fnGetMobileSupport)(Support_Mobile_Function *ColumnList);
typedef DWORD(*fnSyncTask)(SYNCSetting* lpSetting, volatile BOOL* bStop, fnProgressCallback pfnProgress, fnQueryCallback pfnConflict);

typedef int (*fnInitSyncSession)(SYNCSetting* lpPhonebookSetting,SYNCSetting* lpCalendarSetting,SYNCSetting* lpTaskSetting,SYNCSetting* lpMemoSetting,SYNCSetting* lpMailSetting);
typedef int (*fnTerminateSyncSession)();
typedef int (*fnGetSyncInitData)(int* PhonebookGetStatus , int *CalStatus,int* TaskStatus, int* MemoStatus,int* mailStatus,volatile BOOL* bStop);
typedef int (*fnInitSyncEngine)(HMODULE hAnwMobileModule , DWORD& dwSupport);
typedef BOOL (*fnGetMailSytemDefaultDataBase)(CString &strFolderID ,CString &cDatabaseName, int nType,DWORD dwMailSystem);

#endif // !defined(_PIMSYNC1000_H_)
