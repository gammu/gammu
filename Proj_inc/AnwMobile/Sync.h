#ifndef _ANWSYNC_H
#define _ANWSYNC_H
#include "Todo.h"
#include "Calendar.h"
#include "mbglobals.h"

int WINAPI InitSync(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail,bool *bSyncML);
int WINAPI SyncGetNexUpdateToDo(ToDo_Data_Struct *Schedule_Data_Strc,CString &strOriIndex,int* nUpdateFlag,bool bStart);
int WINAPI SyncGetNexUpdateCalendar(Calendar_Data_Struct *Schedule_Data_Strc,CString &strOriIndex,int* nUpdateFlag,bool bStart);
int WINAPI SyncGetNextUpdatePhonbook(GSM_MemoryEntry* Entry,CString &strOriIndex,int* nUpdateFlag,bool bStart);
int WINAPI TerminateSync();
int WINAPI InitSyncGet(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail);

typedef int (WINAPI* SYNC_InitSyncProc)(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail,bool *bSyncML);
typedef int (WINAPI* SYNC_GetNexUpdateToDoProc)(ToDo_Data_Struct *Schedule_Data_Strc,char* strOriIndex,int* nUpdateFlag,bool bStart);
typedef int (WINAPI* SYNC_GetNexUpdateCalendarProc)(Calendar_Data_Struct *Schedule_Data_Strc,char*  strOriIndex,int* nUpdateFlag,bool bStart);
typedef int (WINAPI* SYNC_GetNextUpdatePhonbookProc)(GSM_MemoryEntry* Entry,char* strOriIndex,int* nUpdateFlag,bool bStart);
typedef int (WINAPI* SYNC_TerminateSyncProc)();
typedef int (WINAPI* SYNC_InitSyncGetProc)(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail,int (*pGetStatusfn)(int nCur,int nTotal));

extern SYNC_InitSyncProc		ANWInitSync;
extern SYNC_GetNexUpdateToDoProc		ANWGetNexUpdateToDo;
extern SYNC_GetNexUpdateCalendarProc		ANWGetNexUpdateCalendar;
extern SYNC_GetNextUpdatePhonbookProc		ANWGetNextUpdatePhonbook;
extern SYNC_TerminateSyncProc		ANWTerminateSync;
extern SYNC_InitSyncGetProc		ANWInitSyncGet;

#endif