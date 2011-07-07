#ifndef _MSOTDLL_H
#define _MSOTDLL_H
#include "..\SYNC\MSOTFun.h"

class CMSOTDLL
{
public:
	CMSOTDLL();
	~CMSOTDLL();

	HINSTANCE m_hDll;
	BOOL LoadDll(CString strdllName);

	fnInitDLL			m_pfnInitDLL;
	fnTerminateDLL		m_pfnTerminateDLL;
	fnGetProfileList	m_pfnGetProfileList;
	fnBuildTree			m_pfnBuildTree;
	fnGetFolderFirstEntryID			m_pfnGetFolderFirstEntryID;
	fnGetProfileListWithoutFolderType	m_pfnGetProfileListWithoutFolderType;

	fnGetFolder			m_pfnGetFolder;
	fnGetContact		m_pfnGetContact;
	fnDeleteContact		m_pfnDeleteContact;
	fnAddContact		m_pfnAddContact;
	fnUpdateContact		m_pfnUpdateContact;
	fnCompareContactData		m_pfnCompareContactData;
	fnFillMSOTEntryType		m_pfnFillMSOTEntryType;
	fnUpdateContactToMobileSupport		m_pfnUpdateContactToMobileSupport;
	fnAddSIMContact		m_pfnAddSIMContact;

	fnGetCalendar		m_pfnGetCalendar;
	fnDeleteCalendar	m_pfnDeleteCalendar;
	fnAddCalendar		m_pfnAddCalendar;
	fnUpdateCalendar	m_pfnUpdateCalendar;
	fnCompareCalendarData m_pfnCompareCalendarData;
	fnGetCalendarUpdateFiled m_pfnGetCalendarUpdateFiled;
	fnUpdateCalendarToMobileSupport	m_pfnUpdateCalendarToMobileSupport;

	fnGetTask			m_pfnGetTask;
	fnDeleteTask		m_pfnDeleteTask;
	fnAddTask			m_pfnAddTask;
	fnUpdateTask		m_pfnUpdateTask;
	fnCompareTaskData		m_pfnCompareTaskData;
	fnGetTaskUpdateFiled m_pfnGetTaskUpdateFiled;
	fnUpdateTaskToMobileSupport m_pfnUpdateTaskToMobileSupport;

	fnGetFolderByName	m_pfnGetFolderByName;
	fnSetMobileName	m_pfnSetMobileName;
	fnGetFolderList m_pfnGetFolderList;

	DWORD MSOT_InitDLL(); //InitOleApp
	DWORD MSOT_TerminateDLL(); //ReleaseOleApp

	DWORD MSOT_GetProfileList(CPtrList* pList);
	//need delete Item data in every child, type is _OutlookFolder*
	DWORD MSOT_BuildTree(void* pOutlookTree,CString strFolderEntryID, int nType, LPCTSTR szSelEntry);	//Get outlook folder tree //BuildOutlookTree
	DWORD MSOT_GetFolderFirstEntryID(CString strFolderEntryID, int nType, CString szFirstEntry, CString szStoreID);	//Get outlook folder tree //BuildOutlookTree
	//return value is _OutlookFolder*
	DWORD MSOT_GetFolder(int nIndex);	//Get outlook folder //GetOutlookFolder
	LPDISPATCH MSOT_GetFolderByName(CString &strOTEntryID, LPCTSTR szStoreID, int nType);
	void MSOT_SetMobileName(TCHAR* pMobileModleName);
	DWORD MSOT_GetProfileListWithoutFolderType(CPtrList* pList);

	//Phonebook
	int  MSOT_GetContact(LPDISPATCH lpFolder,CPtrList *pContactsList,volatile BOOL* bStop, fnProgressCallback pFn,int nProgressMin,int nProgressMax);
	int  MSOT_DeleteContact(LPDISPATCH lpFolder,ContactData *pContactData);
	int  MSOT_AddContact(LPDISPATCH lpFolder,ContactData *pContactData);
	int  MSOT_UpdateContact(LPDISPATCH lpFolder,ContactData *pOldContactData,ContactData *pNewContactData);
	int  MSOT_CompareContactData(Contact_Data_Struct* pbInfo1,Contact_Data_Struct* pbInfo2);
	int  MSOT_FillMSOTEntryType(Contact_Data_Struct* pContactData);
	void MSOT_UpdateContactToMobileSupport(Contact_Data_Struct *pContactData );
	int  MSOT_AddSIMContact(LPDISPATCH lpFolder,ContactData *pContactData);
	//Calendar
	int  MSOT_GetCalendar(LPDISPATCH lpFolder,CPtrList *pCalendarList,volatile BOOL* bStop, fnProgressCallback pFn,int nProgressMin,int nProgressMax);
	int  MSOT_DeleteCalendar(LPDISPATCH lpFolder,CalendarData *pCalendarData );
	int  MSOT_AddCalendar(LPDISPATCH lpFolder,CalendarData *pCalendarData);
	int  MSOT_UpdateCalendar(LPDISPATCH lpFolder,CalendarData *pData_outlook,CalendarData *pData_New,DWORD dwFiled);
	BOOL MSOT_CompareCalendarData(Calendar_Data_Struct* pbInfo1,Calendar_Data_Struct* pbInfo2);
	DWORD MSOT_GetCalendarUpdateFiled(Calendar_Data_Struct* pbInfo1,Calendar_Data_Struct* pbInfo2);
	void MSOT_UpdateCalendarToMobileSupport(Calendar_Data_Struct* pbInfo1);
	//Task
	int  MSOT_GetTask(LPDISPATCH lpFolder,CPtrList *pTaskList,volatile BOOL* bStop, fnProgressCallback pFn,int nProgressMin,int nProgressMax);
	int  MSOT_DeleteTask(LPDISPATCH lpFolder,TaskData *pTaskData);
	int  MSOT_AddTask(LPDISPATCH lpFolder,TaskData *pTaskData);
	int  MSOT_UpdateTask(LPDISPATCH lpFolder,TaskData *pTaskData,TaskData *,DWORD dwFiled);
	BOOL MSOT_CompareTaskData(ToDo_Data_Struct* pbInfo1,ToDo_Data_Struct* pbInfo2);
	DWORD MSOT_GetTaskUpdateFiled(ToDo_Data_Struct* pbInfo1,ToDo_Data_Struct* pbInfo2);
	void MSOT_UpdateTaskToMobileSupport(ToDo_Data_Struct* pbInfo1);
	DWORD MSOT_GetFolderList(CString strFolderEntryID, int nType,CPtrList *pFolderList);


};

#endif


