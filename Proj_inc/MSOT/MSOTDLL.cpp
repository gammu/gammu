#include "stdafx.h"
#include "MSOTDll.h"

CMSOTDLL::CMSOTDLL()
{
	m_hDll = NULL;
}
CMSOTDLL::~CMSOTDLL()
{
	if(m_hDll)
		FreeLibrary(m_hDll);
}
BOOL CMSOTDLL::LoadDll(CString strDllName)
{
	if(strDllName.IsEmpty()) return false;

	m_hDll = ::LoadLibrary(strDllName);
	if(!m_hDll)
		return FALSE;

	m_pfnInitDLL = (fnInitDLL)GetProcAddress(m_hDll,"InitDLL");
	m_pfnTerminateDLL = (fnTerminateDLL)GetProcAddress(m_hDll,"TerminateDLL");
	m_pfnGetProfileList = (fnGetProfileList)GetProcAddress(m_hDll,"GetProfileList");
	m_pfnBuildTree = (fnBuildTree)GetProcAddress(m_hDll,"BuildTree");
	m_pfnGetFolder = (fnGetFolder)GetProcAddress(m_hDll,"GetFolder");
	m_pfnGetFolderByName = (fnGetFolderByName)GetProcAddress(m_hDll,"GetFolderByName");
	m_pfnSetMobileName = (fnSetMobileName)GetProcAddress(m_hDll,"SetMobileName");
	m_pfnGetFolderFirstEntryID = (fnGetFolderFirstEntryID)GetProcAddress(m_hDll,"GetFolderFirstEntryID");
	m_pfnGetProfileListWithoutFolderType = (fnGetProfileListWithoutFolderType)GetProcAddress(m_hDll,"GetProfileListWithoutFolderType");

	m_pfnGetContact = (fnGetContact)GetProcAddress(m_hDll,"GetContact");
	m_pfnDeleteContact = (fnDeleteContact)GetProcAddress(m_hDll,"DeleteContact");
	m_pfnAddContact = (fnAddContact)GetProcAddress(m_hDll,"AddContact");
	m_pfnUpdateContact = (fnUpdateContact)GetProcAddress(m_hDll,"UpdateContact");
	m_pfnCompareContactData = (fnCompareContactData)GetProcAddress(m_hDll,"CompareContactData");
	m_pfnFillMSOTEntryType = (fnFillMSOTEntryType)GetProcAddress(m_hDll,"FillMSOTEntryType");
	m_pfnUpdateContactToMobileSupport = (fnUpdateContactToMobileSupport)GetProcAddress(m_hDll,"UpdateContactToMobileSupport");

	m_pfnGetCalendar = (fnGetCalendar)GetProcAddress(m_hDll,"GetCalendar");
	m_pfnDeleteCalendar = (fnDeleteCalendar)GetProcAddress(m_hDll,"DeleteCalendar");
	m_pfnAddCalendar = (fnAddCalendar)GetProcAddress(m_hDll,"AddCalendar");
	m_pfnUpdateCalendar = (fnUpdateCalendar)GetProcAddress(m_hDll,"UpdateCalendar");
	m_pfnCompareCalendarData =(fnCompareCalendarData)GetProcAddress(m_hDll,"CompareCalendarData");
	m_pfnGetCalendarUpdateFiled =(fnGetCalendarUpdateFiled)GetProcAddress(m_hDll,"GetCalendarUpdateFiled");
	m_pfnUpdateCalendarToMobileSupport =(fnUpdateCalendarToMobileSupport)GetProcAddress(m_hDll,"UpdateCalendarToMobileSupport");

	m_pfnGetTask = (fnGetTask)GetProcAddress(m_hDll,"GetTask");
	m_pfnDeleteTask = (fnDeleteTask)GetProcAddress(m_hDll,"DeleteTask");
	m_pfnAddTask = (fnAddTask)GetProcAddress(m_hDll,"AddTask");
	m_pfnUpdateTask = (fnUpdateTask)GetProcAddress(m_hDll,"UpdateTask");
	m_pfnCompareTaskData = (fnCompareTaskData)GetProcAddress(m_hDll,"CompareTaskData");
	m_pfnGetTaskUpdateFiled = (fnGetTaskUpdateFiled)GetProcAddress(m_hDll,"GetTaskUpdateFiled");
	m_pfnUpdateTaskToMobileSupport = (fnUpdateTaskToMobileSupport)GetProcAddress(m_hDll,"UpdateTaskToMobileSupport");

	m_pfnGetFolderList = (fnGetFolderList)GetProcAddress(m_hDll,"GetFolderList");
	m_pfnAddSIMContact = (fnAddSIMContact)GetProcAddress(m_hDll,"AddSIMContact");
	return true;
}

DWORD CMSOTDLL::MSOT_InitDLL()
{
	if(m_pfnInitDLL)
		return m_pfnInitDLL();
	return 0;
}
DWORD CMSOTDLL::MSOT_TerminateDLL()
{
	if(m_pfnTerminateDLL)
		return m_pfnTerminateDLL();
	return 0;
}
DWORD CMSOTDLL::MSOT_GetProfileListWithoutFolderType(CPtrList* pList)
{
	if(m_pfnGetProfileListWithoutFolderType)
		return m_pfnGetProfileListWithoutFolderType(pList);
	return 0;
}

DWORD CMSOTDLL::MSOT_GetProfileList(CPtrList* pList)
{
	if(m_pfnGetProfileList)
		return m_pfnGetProfileList(pList);
	return 0;
}
//need delete Item data in every child, type is _OutlookFolder*
DWORD CMSOTDLL::MSOT_BuildTree(void* pOutlookTree,CString strFolderEntryID, int nType, LPCTSTR szSelEntry)
{
	if(m_pfnBuildTree)
		return m_pfnBuildTree(pOutlookTree,strFolderEntryID,  nType,  szSelEntry);
	return 0;
}	

DWORD CMSOTDLL::MSOT_GetFolderList(CString strFolderEntryID, int nType,CPtrList *pFolderList)
{
	if(m_pfnGetFolderList)
		return m_pfnGetFolderList(strFolderEntryID,  nType,  pFolderList);
	return 0;
}
DWORD CMSOTDLL::MSOT_GetFolderFirstEntryID(CString strFolderEntryID, int nType, CString szFirstEntry, CString szStoreID)
{
	if(m_pfnGetFolderFirstEntryID)
		return m_pfnGetFolderFirstEntryID(strFolderEntryID,  nType,  szFirstEntry,szStoreID);
	return 0;
}
//return value is _OutlookFolder*
DWORD CMSOTDLL::MSOT_GetFolder(int nIndex)
{
	if(m_pfnGetFolder)
		return m_pfnGetFolder(nIndex);
	return 0;
}
LPDISPATCH CMSOTDLL::MSOT_GetFolderByName(CString &strOTEntryID, LPCTSTR szStoreID, int nType)
{
	if(m_pfnGetFolderByName)
		return m_pfnGetFolderByName(strOTEntryID,  szStoreID,  nType);
	return 0;
}
void CMSOTDLL::MSOT_SetMobileName(TCHAR* pMobileModelName)
{
	if(m_pfnSetMobileName)
		 m_pfnSetMobileName(pMobileModelName);
}
//Phonebook
int  CMSOTDLL::MSOT_GetContact(LPDISPATCH lpFolder,CPtrList *pContactsList,volatile BOOL* bStop, fnProgressCallback pFn,int nProgressMin,int nProgressMax)
{
	if(m_pfnGetContact)
		return m_pfnGetContact(lpFolder,pContactsList,bStop,pFn,nProgressMin, nProgressMax);
	return 0;
}
int  CMSOTDLL::MSOT_DeleteContact(LPDISPATCH lpFolder,ContactData *pContactData)
{
	if(m_pfnDeleteContact)
		return m_pfnDeleteContact(lpFolder,pContactData);
	return 0;
}
int  CMSOTDLL::MSOT_AddContact(LPDISPATCH lpFolder,ContactData *pContactData)
{
	if(m_pfnAddContact)
		return m_pfnAddContact(lpFolder,pContactData);
	return 0;
}
int  CMSOTDLL::MSOT_AddSIMContact(LPDISPATCH lpFolder,ContactData *pContactData)
{
	if(m_pfnAddSIMContact)
		return m_pfnAddSIMContact(lpFolder,pContactData);
	return 0;
}
int  CMSOTDLL::MSOT_UpdateContact(LPDISPATCH lpFolder,ContactData *pOldContactData,ContactData *pNewContactData)
{
	if(m_pfnUpdateContact)
		return m_pfnUpdateContact(lpFolder,pOldContactData,pNewContactData);
	return 0;
}
int  CMSOTDLL::MSOT_CompareContactData(Contact_Data_Struct* pbInfo1,Contact_Data_Struct* pbInfo2)
{
	if(m_pfnCompareContactData)
		return m_pfnCompareContactData(pbInfo1,pbInfo2);
	return 0;
}
int  CMSOTDLL::MSOT_FillMSOTEntryType(Contact_Data_Struct* pContactData)
{
	if(m_pfnFillMSOTEntryType)
		return m_pfnFillMSOTEntryType(pContactData);
	return 0;
}
//Calendar
int  CMSOTDLL::MSOT_GetCalendar(LPDISPATCH lpFolder,CPtrList *pCalendarList,volatile BOOL* bStop,
								fnProgressCallback pFn,int nProgressMin,int nProgressMax)
{
	if(m_pfnGetCalendar)
		return m_pfnGetCalendar(lpFolder,pCalendarList,bStop,pFn,nProgressMin,nProgressMax);
	return 0;
}
int  CMSOTDLL::MSOT_DeleteCalendar(LPDISPATCH lpFolder,CalendarData *pCalendarData )
{
	if(m_pfnDeleteCalendar)
		return m_pfnDeleteCalendar(lpFolder,pCalendarData );
	return 0;
}
int  CMSOTDLL::MSOT_AddCalendar(LPDISPATCH lpFolder,CalendarData *pCalendarData)
{
	if(m_pfnAddCalendar)
		return m_pfnAddCalendar(lpFolder,pCalendarData );
	return 0;
}
int  CMSOTDLL::MSOT_UpdateCalendar(LPDISPATCH lpFolder,CalendarData *pData_outlook,CalendarData *pData_New,DWORD dwFiled)
{
	if(m_pfnUpdateCalendar)
		return m_pfnUpdateCalendar(lpFolder,pData_outlook ,pData_New,dwFiled);
	return 0;
}
BOOL CMSOTDLL::MSOT_CompareCalendarData(Calendar_Data_Struct* pbInfo1,Calendar_Data_Struct* pbInfo2)
{
	if(m_pfnCompareCalendarData)
		return m_pfnCompareCalendarData(pbInfo1,pbInfo2);
	return 0;
}
DWORD CMSOTDLL::MSOT_GetCalendarUpdateFiled(Calendar_Data_Struct* pbInfo1,Calendar_Data_Struct* pbInfo2)
{
	if(m_pfnGetCalendarUpdateFiled)
		return m_pfnGetCalendarUpdateFiled(pbInfo1,pbInfo2);
	return 0;
}
void CMSOTDLL::MSOT_UpdateCalendarToMobileSupport(Calendar_Data_Struct* pbInfo1)
{
	if(m_pfnUpdateCalendarToMobileSupport)
		 m_pfnUpdateCalendarToMobileSupport(pbInfo1);
}
//Task
int  CMSOTDLL::MSOT_GetTask(LPDISPATCH lpFolder,CPtrList *pTaskList,volatile BOOL* bStop, fnProgressCallback pFn,int nProgressMin,int nProgressMax)
{
	if(m_pfnGetTask)
		return m_pfnGetTask(lpFolder,pTaskList, bStop,  pFn,nProgressMin, nProgressMax);
	return 0;
}
int  CMSOTDLL::MSOT_DeleteTask(LPDISPATCH lpFolder,TaskData *pTaskData)
{
	if(m_pfnDeleteTask)
		return m_pfnDeleteTask(lpFolder, pTaskData);
	return 0;
}
int  CMSOTDLL::MSOT_AddTask(LPDISPATCH lpFolder,TaskData *pTaskData)
{
	if(m_pfnAddTask)
		return m_pfnAddTask(lpFolder, pTaskData);
	return 0;
}
int  CMSOTDLL::MSOT_UpdateTask(LPDISPATCH lpFolder,TaskData *pTaskData,TaskData *pTaskDataNew,DWORD dwFiled)
{
	if(m_pfnUpdateTask)
		return m_pfnUpdateTask(lpFolder, pTaskData,pTaskDataNew,dwFiled);
	return 0;
}
BOOL CMSOTDLL::MSOT_CompareTaskData(ToDo_Data_Struct* pbInfo1,ToDo_Data_Struct* pbInfo2)
{
	if(m_pfnCompareTaskData)
		return m_pfnCompareTaskData(pbInfo1, pbInfo2);
	return 0;

}
DWORD CMSOTDLL::MSOT_GetTaskUpdateFiled(ToDo_Data_Struct* pbInfo1,ToDo_Data_Struct* pbInfo2)
{
	if(m_pfnGetTaskUpdateFiled)
		return m_pfnGetTaskUpdateFiled(pbInfo1, pbInfo2);
	return 0;

}
void CMSOTDLL::MSOT_UpdateTaskToMobileSupport(ToDo_Data_Struct* pbInfo1)
{
	if(m_pfnUpdateTaskToMobileSupport)
		 m_pfnUpdateTaskToMobileSupport(pbInfo1);

}
void CMSOTDLL::MSOT_UpdateContactToMobileSupport(Contact_Data_Struct* pbInfo1)
{
	if(m_pfnUpdateContactToMobileSupport)
		 m_pfnUpdateContactToMobileSupport(pbInfo1);

}
