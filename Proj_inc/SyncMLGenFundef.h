#ifndef _SYNCMLGENFUNDEF_H
#define _SYNCMLGENFUNDEF_H
#include "mbglobals.h"
#include "gsmcal.h"
#define SYNC_PHONEBOOK 0x00000001
#define SYNC_CALENDAR 0x00000002

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
void  SYNCMLGEN_Encode_WBXML_OPAQUE(unsigned char * buffer,int* nPos,int nLength);
void SYNCMLGEN_EncodeString(unsigned char * buffer,int* nPos,unsigned char ID,char* psz);
void SYNCMLGEN_EncodeString(unsigned char * buffer,int* nPos,unsigned char ID,int nIndex);
GSM_Error SYNCMLGEN_AddFilePart(GSM_File *File, int *Pos,unsigned char *szType,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SYNCMLGEN_GetFilePart(GSM_File *File,unsigned char *szType,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SYNCMLGEN_AddFile(char* pszFileName,unsigned char *szType,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);


BOOL WINAPI SYNCMLGEN_GetKeyValue(unsigned char*  pBuffer,int BufferSize,char *pszSectionKey, char* pszValaue);

BOOL WINAPI SYNCMLGEN_GetSpecialSection(unsigned char*  pBuffer,int BufferSize,char *szSectionKey,
								 unsigned char**  pSpecialSection,int *pnSectionSize,int* pnFindpos);
GSM_Error  SYNCMLGEN_EncodeResponse(unsigned char * buffer,int* nPos,int *pCmdID,
							//		char * pszAlertTarget,char * pszAlertSource,
							//		char * pszSyncTarget,char * pszSyncSource,char * pSyncMaxObjSize,
									GSM_File XMLfile,char *pServerPhkNext = NULL,char *pServerCalNext = NULL,bool* bPhkSlowSync = NULL,bool* bCalSlowSync = NULL);
void WINAPI SYNCMLGEN_EncodeGet(unsigned char * buffer,int* nPos,int nCmdID,char *pMetaType,char *pTarget);
//void SYNCMLGEN_EncodeSyncCmd(unsigned char * buffer,int* nPos,int nCmdID,char * pSource,char * pTarget,char * pszSyncMaxObjSize);
void WINAPI SYNCMLGEN_EncodeStatus(unsigned char * buffer,int* nPos, int nCmdID,int nMsgIDRef,int nCmdRef,char* pszCmd,
							char* szTargetRef,char* szSourceRef,int nData,char* pszAnchorLast,char* pszAnchorNext);
GSM_Error  SYNCMLGEN_SendResponse(GSM_File XMLfile,GSM_File* pGetFile,bool bSessionFinal);
GSM_Error  SYNCMLGEN_FinishSync(GSM_File* XMLfile,bool bFininshSession);
GSM_Error SYNCMLGEN_PaserServerRespone(GSM_File XMLfile);
GSM_Error SYNCMLGEN_PaserServerUpdateRespone(GSM_File XMLfile,char* pszSaveData2File_PreFileName);

//GSM_Error  SYNCMLGEN_GetServerData(GSM_File XMLfile,GSM_File *pDataFile,SYNCML_DATATYPE datatype);
void SYNCMLGEN_SaveToDBFile(SYNCML_DATATYPE nDataType);
GSM_Error SYNCMLGEN_AddNeworReplaceServerDataToDB(unsigned char *pDataBuffer,int nDataSize, char *pszSource,bool bAddtoUpdateList = false,bool bAdd = true);
GSM_Error SYNCMLGEN_AddNewClientDataToDB(char*pvcfFileName,int nLocation ,SYNCML_DATATYPE nDataType);
//int SYNCMLGEN_CheckDataInDB(unsigned char *pDataBuffer,int nDataSize, char *pszSource,SYNCML_DATATYPE nDataType,COMMAND command);
//void SYNCMLGEN_ClearDBUpdateInfo();
GSM_Error  SYNCMLGEN_SendAlertRequest(GSM_File XMLfile,GSM_File *pDataFile,int nAlertData,char* pLast,char* pNext,SYNCML_DATATYPE nDataType);
//GSM_Error  SYNCMLGEN_SyncData(GSM_File XMLfile,GSM_File *pDataFile);

void  SYNCMLGEN_UpdateDatabase(GSM_File XMLfile,CPtrList* pEntryList,SYNCML_DATATYPE nDataType);
void SYNCMLGEN_EncodeDeleteCmd(unsigned char * buffer,int* nPos,int nCmdID,char * pSource,char * pTarget,
							   char * pszSyncMaxObjSize,char * pszMediaType ,char* pServerID);
void SYNCMLGEN_EncodeAddCmd(unsigned char * buffer,int* nPos,int nCmdID,char * pSource,char * pTarget,
							   char * pszSyncMaxObjSize,char * pszMediaType ,char* pvcffile,char* pszServerID,char* pszClientID ,bool bAdd);

void WINAPI SYNCMLGEN_EncodeResultCmd(unsigned char * buffer,int* nPos, int nCmdID,int nMsgIDRef,int nCmdRef,char *pMetaType,char* szTargetRef);
//void SYNCMLGEN_FillClientUpdateInfo(GSM_SYNCMLUpdateInfo *UpdateInfo);

GSM_Error  SYNCMLGEN_SendRefreshRequest(GSM_File XMLfile,GSM_File *pDataFile);

GSM_Error SYNCMLGEN_InitSyncSession(GSM_File *pResponseFile,DWORD dwAlertSyncFlag);
void SYNCMLGEN_EncodeSyncHdr(unsigned char * buffer,int* nPos,char* pszVerDTD,char* pszVerProto,int nSessionID,int nMsgID,int nMaxMsgSize);
void SYNCMLGEN_EncodeAlert(unsigned char * buffer,int* nPos,int nCmdID,int nData,char *pszSource,char *pszTarget,
							   char *pMetaType,char *pAnchorNext,char *pAnchorLast);

GSM_Error WINAPI SYNCMLGEN_Initialise(char *pIMEI,char *pSourceName,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI SYNCMLGEN_InitPhonebookGet(int* nUsedCount,int (*pGetStatusfn)(int nCur,int nTotal));
GSM_Error WINAPI SYNCMLGEN_GetMemory(int nLocation ,char *pszvcfFileName);
GSM_Error WINAPI SYNCMLGEN_DoSync(int* nUsedCount);
GSM_Error WINAPI SYNCMLGEN_SetMemory(char* pszfilename,int* nLocation,int Control);
GSM_Error WINAPI SYNCMLGEN_DeleteMemory(GSM_MemoryEntry *entry,int Control);
GSM_Error WINAPI SYNCMLGEN_TerminateSession();
GSM_Error WINAPI SYNCMLGEN_DeleteMemoryEx(GSM_MemoryEntry *entry,int Control,char* pszOriIndex);
GSM_Error WINAPI SYNCMLGEN_PriDeleteMemory(char* szClientIndex,char* szServerIndex,int Control);

GSM_Error WINAPI SYNCMLGEN_InitCalendarGet(int* nCalendarUsedCount,int* nTodoUsedCount,int (*pGetStatusfn)(int nCur,int nTotal));
GSM_Error WINAPI SYNCMLGEN_GetCalendar(int nLocation ,char *pszvcardFileName);
GSM_Error WINAPI SYNCMLGEN_DelCalendar(GSM_CalendarEntry *Note,int Control);
GSM_Error WINAPI SYNCMLGEN_AddCalendar(char* pszfilename,int* nLocation,int Control);
GSM_Error WINAPI SYNCMLGEN_DelCalendarEx(GSM_CalendarEntry *Note,int Control,char* pszOriIndex);
GSM_Error WINAPI SYNCMLGEN_PriDelCalendar(char* szClientIndex,char* szServerIndex,int Control);

GSM_Error WINAPI SYNCMLGEN_GetToDo(int nLocation ,char *pszvtodoFileName);
GSM_Error WINAPI SYNCMLGEN_DelToDo(GSM_ToDoEntry *Todo,int Control);
GSM_Error WINAPI SYNCMLGEN_AddToDo(char* pszfilename,int* nLocation,int Control);
GSM_Error WINAPI SYNCMLGEN_DelToDoEx(GSM_ToDoEntry *Todo,int Control,char* pszOriIndex);
GSM_Error WINAPI SYNCMLGEN_PriDelToDo(char* szClientIndex,char* szServerIndex,int Control);

GSM_Error WINAPI SYNCMLGEN_GetCalendarOriIndex(int nIndex , CString &str);
GSM_Error WINAPI SYNCMLGEN_GetToDoOriIndex(char* pszIndex  , CString &str);
GSM_Error WINAPI SYNCMLGEN_GetMemoryOriIndex(int nIndex , CString &str);

GSM_Error  SYNCMLGEN_EncodeAlertResponse(unsigned char * buffer,int* nPos,int *pCmdID,
									GSM_File XMLfile,int nMsgIDRef,
									char* pszPHKServerNext,char* pszCalserverNext,bool* bSlowSync_phonebook,bool* bSlowSync_Calendar);
GSM_Error WINAPI SYNCMLGEN_InitSyncPIM(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail);
GSM_Error WINAPI SYNCMLGEN_InitSyncGetData(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail,int (*pGetStatusfn)(int nCur,int nTotal));
GSM_Error  WINAPI SYNCMLGEN_SendGetUpdateDataRequest(GSM_File XMLfile,GSM_File *pDataFile,DWORD dwGetFlag,
													 char* pszPHKServerNext,char* pszCalserverNext,bool* bSlowSync_phonebook,bool* bSlowSync_Calendar);
GSM_Error WINAPI  SYNCMLGEN_GetUpdateCalendar(int* nLocation ,char *pszvcardFileName,CString &strOriIndex,int* nUpdateFlag);
GSM_Error WINAPI SYNCMLGEN_GetUpdateToDo(int nIndex,char* pszIndex ,char *pszvcardFileName,CString &strOriIndex,int* nUpdateFlag);
GSM_Error WINAPI SYNCMLGEN_GetUpdateMemory(int* nLocation ,char *pszvcardFileName,CString &strOriIndex,int* nUpdateFlag);
GSM_Error WINAPI SYNCMLGEN_EncodeSyncResponse(unsigned char * buffer,int* nPos,int *pCmdID,
									GSM_File XMLfile,int nMsgIDRef);

void WINAPI SYNCMLGEN_EncodeChar(unsigned char * buffer,int* nPos,unsigned char ID,char* psz);
void WINAPI SYNCMLGEN_EncodeInt(unsigned char * buffer,int* nPos,unsigned char ID,int nIndex);

#endif