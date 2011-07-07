#ifndef MTKFUNDEF_H
#define MTKFUNDEF_H
#include "gsmlogo.h"		
#include "gsmsms.h"		
#include "gsmcal.h"		
GSM_Error MTK_GetReplyStatue(GSM_Protocol_Message	*msg,GSM_ATReplayInfo* pReplynfo);
GSM_Error MTK_SetCharset(bool PreferUnicode);

GSM_Error WINAPI SetConnectparameter(GSM_ConnectionType *ConnectionType,OnePhoneModel *pMobileInfo,	GSM_Error (* pSetATProtocolDatafn)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI Initialise();
GSM_Error WINAPI CheckConnectStatus();
GSM_Error WINAPI Terminate ();
GSM_Error WINAPI GetManufacturer(char* pszManufacturer);
GSM_Error WINAPI GetModel(char* pszModel);
GSM_Error WINAPI GetFirmware(char* pszFirmwareVer);
GSM_Error WINAPI GetIMEI(char* pszIMEI);
GSM_Error WINAPI GetPhoneDateTime(GSM_DateTime *date_time);
GSM_Error WINAPI SetPhoneDateTime(GSM_DateTime *date_time);
GSM_Error WINAPI PhoneReset(bool hard);
GSM_Error WINAPI GetNetworkInfo(GSM_NetworkInfo *netinfo);
GSM_Error WINAPI GetSignalQuality(GSM_SignalQuality *sig);
GSM_Error WINAPI GetBatteryCharge(GSM_BatteryCharge *bat);


GSM_Error WINAPI GetMemoryStatus(GSM_MemoryStatus *Status);
GSM_Error WINAPI GetMemory (GSM_MemoryEntry *entry);
GSM_Error WINAPI SetMemory (GSM_MemoryEntry *entry,int Control);
GSM_Error WINAPI DeleteMemory (GSM_MemoryEntry *entry,int Control);
GSM_Error WINAPI CheckSIMCard(GSM_MemoryStatus *Status);
GSM_Error WINAPI InitPHKGet(GSM_MemoryStatus *Status);
GSM_Error WINAPI GetPhoneBitmap(GSM_Bitmap *Bitmap);

GSM_Error WINAPI GetSMSStatus(GSM_SMSMemoryStatus *status);
GSM_Error WINAPI GetSMSMessage(GSM_MultiSMSMessage *sms);
GSM_Error WINAPI GetNextSMSMessage(GSM_MultiSMSMessage *sms, bool start);
GSM_Error WINAPI SetSMS(GSM_SMSMessage *sms);
GSM_Error WINAPI AddSMS(GSM_SMSMessage *sms);
GSM_Error WINAPI DeleteSMSMessage(GSM_SMSMessage *sms);
GSM_Error WINAPI SendSMSMessage(GSM_SMSMessage *sms);

GSM_Error WINAPI GetCalendarStatus(GSM_CalendarStatus *Status);
GSM_Error WINAPI GetNextCalendar(GSM_CalendarEntry *Note, bool start);
GSM_Error WINAPI  AddCalendar(GSM_CalendarEntry *Note, int Control);
GSM_Error WINAPI InitCalendarGet(GSM_CalendarStatus *Status,int *totalnum);
GSM_Error WINAPI DelCalendar(GSM_CalendarEntry *Note, int Control);

GSM_Error WINAPI GetNextFileFolder(GSM_File *File, bool start);
GSM_Error WINAPI GetFilePart( GSM_File *File);
GSM_Error WINAPI AddFilePart(GSM_File *File, int *Pos);
GSM_Error WINAPI GetFileSystemStatus( GSM_FileSystemStatus *status);
GSM_Error WINAPI DeleteFilePart(unsigned char *ID);
GSM_Error WINAPI AddFolder(GSM_File *File);

GSM_Error WINAPI OBEX_SetFolderPath(GSM_File *File);
GSM_Error WINAPI OBEX_GetObexFileList(GSM_File *File);
GSM_Error WINAPI OBEX_GetNextFileFolder(GSM_File *File, bool start);
GSM_Error WINAPI OBEX_GetFilePart(GSM_File *File,int (*pGetStatusfn)(int nCur,int nTotal));
GSM_Error WINAPI OBEX_AddFilePart(GSM_File *File, int *Pos,int (*pGetStatusfn)(int nCur,int nTotal));
GSM_Error WINAPI OBEX_DeleteFile(GSM_File *ID);
GSM_Error WINAPI OBEX_SetMode(bool bObexMode,OBEX_Service service);

GSM_Error WINAPI OBEX_GetOBEXFolderListCount(int  *nFileNo);
int WINAPI OBEX_GetOBEXSubList(GSM_File *filelist[]);
GSM_Error WINAPI SetFilesLocations(int Locations);
int WINAPI GetObjectSubList(int argv[]);

GSM_Error WINAPI InitSyncPIM(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail,bool *bSyncML);
GSM_Error WINAPI SyncGetNexUpdateToDoData(GSM_ToDoEntry *Schedule_Data_Strc,CString &strOriIndex,int* nUpdateFlag,bool bStart);
GSM_Error WINAPI SyncGetNexUpdateCalendarData(GSM_CalendarEntry *Schedule_Data_Strc,CString &strOriIndex,int* nUpdateFlag,bool bStart);
GSM_Error WINAPI SyncGetNextUpdatePhonbookData(GSM_MemoryEntry* Entry,CString &strOriIndex,int* nUpdateFlag,bool bStart);
GSM_Error WINAPI TerminateSyncPIM();
GSM_Error WINAPI InitSyncGetData(SyncML_SyncInfo* pPhoneBook,SyncML_SyncInfo* pCalendar,SyncML_SyncInfo* pToDo,SyncML_SyncInfo* pMemo,SyncML_SyncInfo* pMail,int (*pGetStatusfn)(int nCur,int nTotal));

GSM_Error WINAPI InitPHKGetEx(GSM_MemoryStatus *Status,int (*GetStatus)(int nCur,int nTotal));

GSM_Error WINAPI DeleteMMSFile(GSM_File *file,int ControlType);
GSM_Error WINAPI AddMMSFilePart(GSM_File *ParentFile,GSM_File *File,int (*pGetStatusfn)(int nCur,int nTotal),int Control);
GSM_Error WINAPI GetMMSFilePart(GSM_File *File,int (*pGetStatusfn)(int nCur,int nTotal),int Control);
GSM_Error WINAPI GMMSFileList(GSM_File *ParentFile, GSM_File *childFile,bool bStart);
GSM_Error WINAPI MMSInit();
GSM_Error WINAPI MMSTerminate();
GSM_Error WINAPI Calendar_NeedCreateIndexTable(BOOL &bNeedCreate);

#endif