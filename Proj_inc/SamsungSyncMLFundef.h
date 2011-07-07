#ifndef _SYNCMLGENFUNDEF_H
#define _SYNCMLGENFUNDEF_H
#include "mbglobals.h"
#include "gsmcal.h"
#define SYNC_PHONEBOOK 0x00000001
#define SYNC_CALENDAR 0x00000002
#define SYNC_TASK 0x00000004

typedef enum 
{
	SYNCML_DATA_VCARD,
	SYNCML_DATA_VCALENDAR,
	SYNCML_DATA_VTODO

}SYNCML_DATATYPE;

typedef struct
{
	char szServerID[MAX_PATH];
	char szClientID[MAX_PATH];
//	char szServerUpdatefilename[MAX_PATH];
//	char szClientUpdatefilename[MAX_PATH];
}SYNCML_DATAMAPINFO;
//void DeleteTempFile(char* pUpdateFilename , char *pszClienID);
void RemoveMemoryList();
void RemoveCalendarList();

void FreeGSMFile(GSM_File *file);
void  SamsungSyncML_Encode_WBXML_OPAQUE(unsigned char * buffer,int* nPos,int nLength);
void SamsungSyncML_EncodeString(unsigned char * buffer,int* nPos,unsigned char ID,char* psz);
void SamsungSyncML_EncodeString(unsigned char * buffer,int* nPos,unsigned char ID,int nIndex);
GSM_Error SamsungSyncML_AddFilePart(GSM_File *File, int *Pos,unsigned char *szType,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SamsungSyncML_GetFilePart(GSM_File *File,unsigned char *szType,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SamsungSyncML_AddFile(char* pszFileName,unsigned char *szType,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);


BOOL SamsungSyncML_GetKeyValue(unsigned char*  pBuffer,int BufferSize,char *pszSectionKey, char* pszValaue);

BOOL SamsungSyncML_GetSpecialSection(unsigned char*  pBuffer,int BufferSize,char *szSectionKey,
								 unsigned char**  pSpecialSection,int *pnSectionSize,int* pnFindpos);
GSM_Error  SamsungSyncML_EncodeResponse(unsigned char * buffer,int* nPos,int *pCmdID,
							//		char * pszAlertTarget,char * pszAlertSource,
							//		char * pszSyncTarget,char * pszSyncSource,char * pSyncMaxObjSize,
									GSM_File XMLfile,char *pServerPhkNext = NULL,char *pServerCalNext = NULL,bool* bPhkSlowSync = NULL,bool* bCalSlowSync = NULL);
void SamsungSyncML_EncodeGet(unsigned char * buffer,int* nPos,int nCmdID,char *pMetaType,char *pTarget);
//void SamsungSyncML_EncodeSyncCmd(unsigned char * buffer,int* nPos,int nCmdID,char * pSource,char * pTarget,char * pszSyncMaxObjSize);
void SamsungSyncML_EncodeStatus(unsigned char * buffer,int* nPos, int nCmdID,int nMsgIDRef,int nCmdRef,char* pszCmd,
							char* szTargetRef,char* szSourceRef,int nData,char* pszAnchorLast,char* pszAnchorNext);
GSM_Error  SamsungSyncML_SendResponse(GSM_File XMLfile,GSM_File* pGetFile,bool bSessionFinal);
GSM_Error  SamsungSyncML_FinishSync(GSM_File* XMLfile,bool bFininshSession);
GSM_Error SamsungSyncML_PaserServerRespone(GSM_File XMLfile);
GSM_Error SamsungSyncML_PaserServerUpdateRespone(GSM_File XMLfile,char* pszSaveData2File_PreFileName);

//GSM_Error  SamsungSyncML_GetServerData(GSM_File XMLfile,GSM_File *pDataFile,SYNCML_DATATYPE datatype);
void SamsungSyncML_SaveToDBFile(SYNCML_DATATYPE nDataType);
GSM_Error SamsungSyncML_AddNeworReplaceServerDataToDB(unsigned char *pDataBuffer,int nDataSize, char *pszSource,bool bAddtoUpdateList = false,bool bAdd = true);
GSM_Error SamsungSyncML_AddNewClientDataToDB(char*pvcfFileName,int nLocation ,SYNCML_DATATYPE nDataType);
//int SamsungSyncML_CheckDataInDB(unsigned char *pDataBuffer,int nDataSize, char *pszSource,SYNCML_DATATYPE nDataType,COMMAND command);
//void SamsungSyncML_ClearDBUpdateInfo();
GSM_Error  SamsungSyncML_SendAlertRequest(GSM_File XMLfile,GSM_File *pDataFile,int nAlertData,char* pLast,char* pNext,SYNCML_DATATYPE nDataType);
//GSM_Error  SamsungSyncML_SyncData(GSM_File XMLfile,GSM_File *pDataFile);

void  SamsungSyncML_UpdateDatabase(GSM_File XMLfile,CPtrList* pEntryList,SYNCML_DATATYPE nDataType);
void SamsungSyncML_EncodeDeleteCmd(unsigned char * buffer,int* nPos,int* nCmdID,char * pSource,char * pTarget,
							   char * pszSyncMaxObjSize,char * pszMediaType ,char* pServerID);
void SamsungSyncML_EncodeAddCmd(unsigned char * buffer,int* nPos,int* nCmdID,char * pSource,char * pTarget,
							   char * pszSyncMaxObjSize,char * pszMediaType ,char* pvcffile,char* pszLocation ,bool bAdd);

void SamsungSyncML_EncodeResultCmd(unsigned char * buffer,int* nPos, int nCmdID,int nMsgIDRef,int nCmdRef,char *pMetaType,char* szTargetRef);
//void SamsungSyncML_FillClientUpdateInfo(GSM_SYNCMLUpdateInfo *UpdateInfo);

GSM_Error  SamsungSyncML_SendRefreshRequest(GSM_File XMLfile,GSM_File *pDataFile);

GSM_Error SamsungSyncML_InitSyncSession(GSM_File *pResponseFile,DWORD dwAlertSyncFlag);
void SamsungSyncML_EncodeSyncHdr(unsigned char * buffer,int* nPos,char* pszVerDTD,char* pszVerProto,int nSessionID,int nMsgID,int nMaxMsgSize);
void SamsungSyncML_EncodeAlert(unsigned char * buffer,int* nPos,int nCmdID,int nData,char *pszSource,char *pszTarget,
							   char *pMetaType,char *pAnchorNext,char *pAnchorLast);

GSM_Error WINAPI SamsungSyncML_Initialise(char *pIMEI,char *pSourceName,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI SamsungSyncML_InitPhonebookGet(int* nUsedCount,int (*pGetStatusfn)(int nCur,int nTotal));
GSM_Error WINAPI SamsungSyncML_GetMemory(int nLocation ,char *pszvcfFileName);
GSM_Error WINAPI SamsungSyncML_DoSync(int* nUsedCount);
GSM_Error WINAPI SamsungSyncML_SetMemory(char* pszfilename,int* nLocation,int Control);
GSM_Error WINAPI SamsungSyncML_DeleteMemory(GSM_MemoryEntry *entry,int Control);
GSM_Error WINAPI SamsungSyncML_TerminateSession();
GSM_Error WINAPI SamsungSyncML_DeleteMemoryEx(GSM_MemoryEntry *entry,int Control,char* pszOriIndex);
GSM_Error WINAPI SamsungSyncML_PriDeleteMemory(char* szClientIndex,char* szServerIndex,int Control);

GSM_Error WINAPI SamsungSyncML_InitCalendarGet(int* nCalendarUsedCount,int* nTodoUsedCount,int (*pGetStatusfn)(int nCur,int nTotal));
GSM_Error WINAPI SamsungSyncML_GetCalendar(int nLocation ,char *pszvcardFileName);
GSM_Error WINAPI SamsungSyncML_DelCalendar(GSM_CalendarEntry *Note,int Control);
GSM_Error WINAPI SamsungSyncML_AddCalendar(char* pszfilename,int* nLocation,int Control);
GSM_Error WINAPI SamsungSyncML_DelCalendarEx(GSM_CalendarEntry *Note,int Control,char* pszOriIndex);
GSM_Error WINAPI SamsungSyncML_PriDelCalendar(char* szClientIndex,char* szServerIndex,int Control);

GSM_Error WINAPI SamsungSyncML_GetToDo(int nLocation ,char *pszvtodoFileName);
GSM_Error WINAPI SamsungSyncML_DelToDo(GSM_ToDoEntry *Todo,int Control);
GSM_Error WINAPI SamsungSyncML_AddToDo(char* pszfilename,int* nLocation,int Control);
GSM_Error WINAPI SamsungSyncML_DelToDoEx(GSM_ToDoEntry *Todo,int Control,char* pszOriIndex);
GSM_Error WINAPI SamsungSyncML_PriDelToDo(char* szClientIndex,char* szServerIndex,int Control);

GSM_Error WINAPI SamsungSyncML_GetCalendarOriIndex(int nIndex , CString &str);
GSM_Error WINAPI SamsungSyncML_GetToDoOriIndex(char* pszIndex  , CString &str);
GSM_Error WINAPI SamsungSyncML_GetMemoryOriIndex(int nIndex , CString &str);

GSM_Error  SamsungSyncML_EncodeAlertResponse(unsigned char * buffer,int* nPos,int *pCmdID,
									GSM_File XMLfile,int nMsgIDRef,
									char* pszPHKServerNext,char* pszCalserverNext,bool* bSlowSync_phonebook,bool* bSlowSync_Calendar);
GSM_Error WINAPI SamsungSyncML_InitSyncPIM(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail,
										   int *PhonebookTotoal,int *CalendarTotoal,int *TaskTotoal,int *MemoTotoal,int *MailTotoal);
GSM_Error WINAPI SamsungZSyncML_InitSyncPIM(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail,
										   int *PhonebookTotoal,int *CalendarTotoal,int *TaskTotoal,int *MemoTotoal,int *MailTotoal);
GSM_Error WINAPI SamsungZSyncML_InitSyncPhoneCalendar(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo);	//Overloaded
GSM_Error WINAPI SamsungSyncML_ContinueSyncPIM(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail,
										   int *PhonebookTotoal,int *CalendarTotoal,int *TaskTotoal,int *MemoTotoal,int *MailTotoal);
GSM_Error WINAPI SamsungZSyncML_ContinueSyncPIM(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail,
										   int *PhonebookTotoal,int *CalendarTotoal,int *TaskTotoal,int *MemoTotoal,int *MailTotoal);
GSM_Error WINAPI SamsungSyncML_InitSyncGetData(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail,int (*pGetStatusfn)(int nCur,int nTotal));
GSM_Error  WINAPI SamsungSyncML_SendGetUpdateDataRequest(GSM_File XMLfile,GSM_File *pDataFile,DWORD dwGetFlag,
													 char* pszPHKServerNext,char* pszCalserverNext,bool* bSlowSync_phonebook,bool* bSlowSync_Calendar);
GSM_Error WINAPI  SamsungSyncML_GetUpdateCalendar(int* nLocation ,char *pszvcardFileName,CString &strOriIndex,int* nUpdateFlag);
GSM_Error WINAPI SamsungSyncML_GetUpdateToDo(int nIndex,char* pszIndex ,char *pszvcardFileName,CString &strOriIndex,int* nUpdateFlag);
GSM_Error WINAPI SamsungSyncML_GetUpdateMemory(int* nLocation ,char *pszvcardFileName,CString &strOriIndex,int* nUpdateFlag);
#endif