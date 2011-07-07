#ifndef _MSOTFUN_H
#define _MSOTFUN_H

#ifdef MSOT_DLL_EXPORTS
#define MSOT_DLL_API __declspec(dllexport)
#else
#define MSOT_DLL_API __declspec(dllimport)
#endif
#include <afxtempl.h>
#include "ConvertEngine.h"
#include "PIMsync.h"

#define UPDATE_ALL 0xFFFFFFFF

#define UPDATE_STARTDATE 0x00000001
#define UPDATE_STARTTIME 0x00000002
#define UPDATE_ENDDATE 0x00000004
#define UPDATE_ENDTIME 0x00000008
#define UPDATE_ALARMDATETIME 0x00000010
#define UPDATE_HAVETONE 0x00000020
#define UPDATE_LOCATION 0x00000040
#define UPDATE_TEXT 0x00000080
#define UPDATE_RECURRENCETYPE 0x00000100
#define UPDATE_RECURRENCEFREQ 0x00000200
#define UPDATE_DAYOFWEEKMASK 0x00000400
#define UPDATE_DESCRIPTION 0x00000800
#define UPDATE_PRIORITY		        0x00001000
#define UPDATE_COMPLETED		    0x00002000
#define UPDATE_COMPLETEDDATE		0x00004000
#define UPDATE_COMPLETEDTIME		0x00008000
#define UPDATE_REPEATENDDATE		0x00010000
typedef enum {
	OT_PBK_NONEDEFINED =-1 ,
	OT_PBK_NONE =0 ,
	OT_PBK_Number_Assistant =1 ,
	OT_PBK_Number_Business ,
	OT_PBK_Number_Business2 ,
	OT_PBK_Number_Fax_Business,
	OT_PBK_Number_Callback,
	OT_PBK_Number_Car,
	OT_PBK_Number_Company,
	OT_PBK_Number_Home,
	OT_PBK_Number_Home2,
	OT_PBK_Number_Fax_Home,
	OT_PBK_Number_ISDN,
	OT_PBK_Number_Mobile,
	OT_PBK_Number_Other,
	OT_PBK_Number_Fax_Other,
	OT_PBK_Number_Pager,
	OT_PBK_Number_Primary,
	OT_PBK_Number_Radio,
	OT_PBK_Number_Telix,
	OT_PBK_Number_TTYTDD,
	OT_PBK_Text_LastName,
	OT_PBK_Text_FirstName,
	OT_PBK_Text_Company,
	OT_PBK_Text_Department,
	OT_PBK_Text_JobTitle,
	OT_PBK_Text_Name,
	OT_PBK_Text_Address_Business,
	OT_PBK_Text_Address_Home,
	OT_PBK_Text_Address_Other,
	OT_PBK_Text_Email1,
	OT_PBK_Text_Email2,
	OT_PBK_Text_Email3,
	OT_PBK_Text_WebAddress,
	OT_PBK_Text_Office,
	OT_PBK_Text_Profession,
	OT_PBK_Text_Manager_Name,
	OT_PBK_Text_Assistant_Name,
	OT_PBK_Text_NickName,
	OT_PBK_Text_Title,
	OT_PBK_Text_Suffix,
	OT_PBK_Text_Spouse_Name,
	OT_PBK_Date_Birthday,
	OT_PBK_Date_Anniversary,
	OT_PBK_Text_Directory_Server,
	OT_PBK_Text_Email_alias,
	OT_PBK_Text_Internet_Address,

	OT_PBK_Text_Address_Business_Street,
	OT_PBK_Text_Address_Home_Street,
	OT_PBK_Text_Address_Other_Street,
	OT_PBK_Text_Address_Business_City,
	OT_PBK_Text_Address_Home_City,
	OT_PBK_Text_Address_Other_City,
	OT_PBK_Text_Address_Business_State,
	OT_PBK_Text_Address_Home_State,
	OT_PBK_Text_Address_Other_State,
	OT_PBK_Text_Address_Business_ZIPCode,
	OT_PBK_Text_Address_Home_ZIPCode,
	OT_PBK_Text_Address_Other_ZIPCode,
	OT_PBK_Text_Address_Business_Country,
	OT_PBK_Text_Address_Home_Country,
	OT_PBK_Text_Address_Other_Country,
	OT_PBK_Text_IMID,
	OT_PBK_Text_NOTES,
	OT_PBK_Text_CHILDREN

} MSOT_EntryType;

typedef struct
{
	GSM_EntryType			MobileEntryType;
	MSOT_EntryType			MSOTEntryType;
	int nMobileEntrytypeIndex; //home 1 ,home2
//	int nUpdateFalg;//SYNCDATA_UPDATE_NONE = 0,SYNCDATA_UPDATE_ADD,SYNCDATA_UPDATE_DELETE,	SYNCDATA_UPDATE_MODIFY
	CString			strText;
	GSM_DateTime	    	Date;
	int					  	Number;
}ContactEntry;

typedef struct
{
	char szIndex[100];
	CPtrList Datalist;
}Contact_Data_Struct;
typedef struct
{
	CString strEntryID;
	char szOriIndex[MAX_PATH];
	Contact_Data_Struct Data;
}ContactData, *PContactData;

typedef struct
{
	CString strEntryID;
	char szOriIndex[MAX_PATH];
	Calendar_Data_Struct Data;
}CalendarData, *PCalendarData;

typedef struct
{
	CString strEntryID;
	char szOriIndex[MAX_PATH];
	ToDo_Data_Struct Data;
}TaskData, *PTaskData;



extern "C"
{
	//-----Gereral API-----
	MSOT_DLL_API DWORD InitDLL(); //InitOleApp
	MSOT_DLL_API DWORD TerminateDLL(); //ReleaseOleApp

	MSOT_DLL_API DWORD GetProfileList(CPtrList* pList);
	//need delete Item data in every child, type is _OutlookFolder*
	MSOT_DLL_API DWORD BuildTree(void* pOutlookTree,CString strFolderEntryID, int nType, LPCTSTR szSelEntry);	//Get outlook folder tree //BuildOutlookTree
	//return value is _OutlookFolder*
	MSOT_DLL_API DWORD GetFolder(int nIndex);	//Get outlook folder //GetOutlookFolder
	MSOT_DLL_API LPDISPATCH GetFolderByName(CString &strOTEntryID, LPCTSTR szStoreID, int nType);
	MSOT_DLL_API void SetMobileName(TCHAR* pMobileModleName);
	MSOT_DLL_API DWORD GetFolderFirstEntryID(CString strFolderEntryID, int nType, CString szdefaultEntry, CString szStoreID);
	MSOT_DLL_API DWORD GetProfileListWithoutFolderType(CPtrList* pList);

	//Phonebook
	MSOT_DLL_API int  GetContact(LPDISPATCH lpFolder,CPtrList *pContactsList,volatile BOOL* bStop, fnProgressCallback pFn,int nProgressMin,int nProgressMax);
	MSOT_DLL_API int  DeleteContact(LPDISPATCH lpFolder,ContactData *pContactData);
	MSOT_DLL_API int  AddContact(LPDISPATCH lpFolder,ContactData *pContactData);
	MSOT_DLL_API int  UpdateContact(LPDISPATCH lpFolder,ContactData *pOldContactData,ContactData *pNewContactData);
	MSOT_DLL_API int  CompareContactData(Contact_Data_Struct* pbInfo1,Contact_Data_Struct* pbInfo2);
	MSOT_DLL_API  int FillMSOTEntryType(Contact_Data_Struct* pContactData);
	MSOT_DLL_API void  UpdateContactToMobileSupport(Contact_Data_Struct *pContactData );
	MSOT_DLL_API int  AddSIMContact(LPDISPATCH lpFolder,ContactData *pContactData);
	//Calendar
	MSOT_DLL_API int  GetCalendar(LPDISPATCH lpFolder,CPtrList *pCalendarList,volatile BOOL* bStop, fnProgressCallback pFn,int nProgressMin,int nProgressMax);
	MSOT_DLL_API int  DeleteCalendar(LPDISPATCH lpFolder,CalendarData *pCalendarData );
	MSOT_DLL_API int  AddCalendar(LPDISPATCH lpFolder,CalendarData *pCalendarData);
	MSOT_DLL_API int  UpdateCalendar(LPDISPATCH lpFolder,CalendarData *pData_outlook,CalendarData *pData_New,DWORD dwFiled);
	MSOT_DLL_API BOOL CompareCalendarData(Calendar_Data_Struct* pbInfo1,Calendar_Data_Struct* pbInfo2);
	MSOT_DLL_API DWORD GetCalendarUpdateFiled(Calendar_Data_Struct* pbInfo1,Calendar_Data_Struct* pbInfo2);
	MSOT_DLL_API void  UpdateCalendarToMobileSupport(Calendar_Data_Struct *pCalendarData );
	//Task
	MSOT_DLL_API int  GetTask(LPDISPATCH lpFolder,CPtrList *pTaskList,volatile BOOL* bStop, fnProgressCallback pFn,int nProgressMin,int nProgressMax);
	MSOT_DLL_API int  DeleteTask(LPDISPATCH lpFolder,TaskData *pTaskData);
	MSOT_DLL_API int  AddTask(LPDISPATCH lpFolder,TaskData *pTaskData);
	MSOT_DLL_API int  UpdateTask(LPDISPATCH lpFolder,TaskData *pTaskData,TaskData *pTaskDataNew,DWORD dwFiled);
	MSOT_DLL_API BOOL CompareTaskData(ToDo_Data_Struct* pbInfo1,ToDo_Data_Struct* pbInfo2);
	MSOT_DLL_API DWORD GetTaskUpdateFiled(ToDo_Data_Struct* pbInfo1,ToDo_Data_Struct* pbInfo2);
	MSOT_DLL_API void  UpdateTaskToMobileSupport(ToDo_Data_Struct *pCalendarData );

	MSOT_DLL_API DWORD GetFolderList(CString strFolderEntryID, int nType,CPtrList *pFolderList);

	MSOT_DLL_API BOOL SelectDatabase(CString &strOTEntryID, int nType);
	MSOT_DLL_API BOOL GetDefaultDataBase(CString &strFolderID ,CString &cDatabaseName, int nType);

}

typedef	DWORD (*fnInitDLL)();
typedef	DWORD (*fnTerminateDLL)();
typedef DWORD(*fnGetProfileList)(CPtrList* pList);
typedef DWORD(*fnBuildTree)(void* pOutlookTree,CString strFolderEntryID, int nType, LPCTSTR szSelEntry);
typedef DWORD(*fnGetFolder)(int nIndex);
typedef LPDISPATCH (*fnGetFolderByName)(CString &strOTEntryID, LPCTSTR szStoreID, int nType);
typedef BOOL (*fnSelectDatabase)(CString &strOTEntryID, int nType);
typedef BOOL (*fnGetDefaultDataBase)(CString &strFolderID ,CString &cDatabaseName, int nType);

typedef void (*fnSetMobileName)(TCHAR* pMobileModleName);
typedef DWORD (*fnGetFolderFirstEntryID)(CString strFolderEntryID, int nType, CString szdefaultEntry, CString szStoreID);
typedef DWORD(*fnGetProfileListWithoutFolderType)(CPtrList* pList);

typedef int(*fnGetContact)(LPDISPATCH lpFolder,CPtrList *pContactsList,volatile BOOL* bStop, fnProgressCallback pFn,int nProgressMin,int nProgressMax);
typedef int(*fnDeleteContact)(LPDISPATCH lpFolder,ContactData *pContactData);
typedef int(*fnAddContact)(LPDISPATCH lpFolder,ContactData *pContactData);
typedef int(*fnUpdateContact)(LPDISPATCH lpFolder,ContactData *pOldContactData,ContactData *pNewContactData);
typedef int(*fnCompareContactData)(Contact_Data_Struct* pbInfo1,Contact_Data_Struct* pbInfo2);
typedef int(*fnFillMSOTEntryType)(Contact_Data_Struct* pContactData);
typedef void  (*fnUpdateContactToMobileSupport)(Contact_Data_Struct *pContactData );
typedef int(*fnAddSIMContact)(LPDISPATCH lpFolder,ContactData *pContactData);

typedef int(*fnGetCalendar)(LPDISPATCH lpFolder,CPtrList *pCalendarList,volatile BOOL* bStop, fnProgressCallback pFn,int nProgressMin,int nProgressMax);
typedef int(*fnDeleteCalendar)(LPDISPATCH lpFolder,CalendarData *pCalendarData );
typedef int(*fnAddCalendar)(LPDISPATCH lpFolder,CalendarData *pCalendarData);
typedef int(*fnUpdateCalendar)(LPDISPATCH lpFolder,CalendarData *pData_outlook,CalendarData *pData_New,DWORD dwFiled);
typedef BOOL (*fnCompareCalendarData)(Calendar_Data_Struct* pbInfo1,Calendar_Data_Struct* pbInfo2);
typedef DWORD (*fnGetCalendarUpdateFiled)(Calendar_Data_Struct* pbInfo1,Calendar_Data_Struct* pbInfo2);
typedef void  (*fnUpdateCalendarToMobileSupport)(Calendar_Data_Struct *pCalendarData );


typedef int(*fnGetTask)(LPDISPATCH lpFolder,CPtrList *pTaskList,volatile BOOL* bStop, fnProgressCallback pFn,int nProgressMin,int nProgressMax);
typedef int(*fnDeleteTask)(LPDISPATCH lpFolder,TaskData *pTaskData);
typedef int(*fnAddTask)(LPDISPATCH lpFolder,TaskData *pTaskData);
typedef int(*fnUpdateTask)(LPDISPATCH lpFolder,TaskData *pTaskData,TaskData *pTaskDataNew,DWORD dwFiled);
typedef BOOL (*fnCompareTaskData)(ToDo_Data_Struct* pbInfo1,ToDo_Data_Struct* pbInfo2);
typedef DWORD (*fnGetTaskUpdateFiled)(ToDo_Data_Struct* pbInfo1,ToDo_Data_Struct* pbInfo2);
typedef void (*fnUpdateTaskToMobileSupport)(ToDo_Data_Struct* pbInfo1);
typedef DWORD (*fnGetFolderList)(CString strFolderEntryID, int nType,CPtrList *pFolderList);

#endif