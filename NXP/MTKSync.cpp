#include "stdafx.h"
#include "MTK.h"
#include "commfun.h"
#include "MTKfundef.h"
extern CMTKApp theApp;
GSM_Error WINAPI InitSyncPIM(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail,bool *bSyncML)
{
	*bSyncML = false;
	return ERR_NONE;
}
GSM_Error WINAPI SyncGetNexUpdateToDoData(GSM_ToDoEntry *Schedule_Data_Strc,CString &strOriIndex,int* nUpdateFlag,bool bStart)
{
	return ERR_NOTSUPPORTED;
}
GSM_Error WINAPI SyncGetNexUpdateCalendarData(GSM_CalendarEntry *Schedule_Data_Strc,CString &strOriIndex,int* nUpdateFlag,bool bStart)
{
	return ERR_NOTSUPPORTED;
}
GSM_Error WINAPI SyncGetNextUpdatePhonbookData(GSM_MemoryEntry* Entry,CString &strOriIndex,int* nUpdateFlag,bool bStart)
{
	return ERR_NOTSUPPORTED;
}
GSM_Error WINAPI TerminateSyncPIM()
{
	return ERR_NONE;
}
GSM_Error WINAPI InitSyncGetData(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail,int (*pGetStatusfn)(int nCur,int nTotal))
{
	if(pPhoneBook) pPhoneBook->nCount = -1;
	if(pCalendar) pCalendar->nCount = -1;
	if(pToDo) pToDo->nCount = -1;
	if(pMemo) pMemo->nCount = -1;
	if(pMail) pMail->nCount = -1;
	return ERR_NONE;
}
