#ifndef SHARPFUNFUNDEF_H
#define SHARPFUNFUNDEF_H

#include "mbglobals.h"
#include "commfun.h"
#include "gsmsms.h"		
#include "gsmmulti.h"		
#include "ATGenFunDef.h"

typedef struct {
	char			m_ConnectID[4]; 
	int				m_FrameSize;
} OBEX_ConnectInfo2;

/////////////////////////////////////////////////////////////////////////////
// OBEX Header

//--- type
#define SHARP_OBEXHEADER_TYPE_VCARD		"text/x-vCard"
#define SHARP_OBEXHEADER_TYPE_VCAL		"text/x-vCalendar"

/////////////////////////////////////////////////////////////////////////////
// obex

GSM_Error SHARPOBEX_Initialise(OnePhoneModel *pMobileInfo,Debug_Info	*pDebuginfo);
GSM_Error SHARPOBEX_Connect(unsigned char *Buffer,int BufferSize, char* szTarget,OBEX_ConnectInfo2 *pConnectInfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SHARPOBEX_Disconnect(OBEX_ConnectInfo2 *pConnectInfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error SHARPOBEX_SetFolderPath(GSM_File *File, unsigned char RootFlag, unsigned char ChildFalg,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SHARPOBEX_GetNextFileFolder(GSM_File *File, bool start, unsigned char RootFlag, unsigned char ChildFlag,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SHARPOBEX_GetNextFileFolder2(GSM_File *File, bool start, unsigned char RootFlag, unsigned char ChildFlag,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SHARPOBEX_AddFolder(GSM_File *File, unsigned char RootFlag, unsigned char ChildFlag,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SHARPOBEX_GetOBEXFolderListCount(int  *nFileNo);
int SHARPOBEX_GetOBEXSubList(GSM_File *filelist[]);


GSM_Error SHARPOBEX_GetFilePart(GSM_File *File, unsigned char RootFlag, unsigned char ChildFlag,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo,
						  int (*pGetStatusfn)(int nCur,int nTotal));						  
GSM_Error SHARPOBEX_PrivGetFilePart(GSM_File *File, bool FolderList, unsigned char RootFlag, unsigned char ChildFlag ,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo,
						  int (*pGetStatusfn)(int nCur,int nTotal));						  
void SHARPOBEX_ParseXML(GSM_File *File, unsigned char Flag,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
					  
			  
GSM_Error SHARPOBEX_AddFilePart(GSM_File *File, int *Pos, char* szObexType, unsigned char RootFlag, unsigned char ChildFlag,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo,
							  int (*pGetStatusfn)(int nCur,int nTotal));
							  
GSM_Error SHARPOBEX_DeleteFile(GSM_File *ID, char* szObexType, unsigned char RootFlag, unsigned char ChildFlag,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

//---
void OBEXAddBlock(unsigned char *Buffer, int *Pos, unsigned char ID,unsigned char *AddBuffer, int AddLength);
//---
void OBEXHEADER_ConnectionID(unsigned char* request, int* pos, char* Id);
void OBEXHEADER_Length(unsigned char* request, int* pos, int Length);
void OBEXHEADER_Name(unsigned char* request, int* pos, unsigned char* szName);
void OBEXHEADER_Type(unsigned char* request, int* pos, char* szType);


/////////////////////////////////////////////////////////////////////////////
// VCard

void SHARPVCARD_ParseName(unsigned char* Line,unsigned char* buffer, unsigned char* name, unsigned char* first_name, unsigned char* last_name);
void SHARPVCARD_ParseDateTime(unsigned char* buffer, GSM_DateTime *dt);


/////////////////////////////////////////////////////////////////////////////
// VCalendar

bool SHARPVCAL_ParseRepeat(unsigned char* buffer, int* nType, int* nTimes);

// append the repeat times
void SHARPVCAL_RepeatCat(unsigned char* Buffer, int* Length, int nRepeatType, int nRepeatTimes, GSM_DateTime* Date);

GSM_CalendarNoteType SHARPVCAL_CategoryTypeID(char* szType);
unsigned char* SHARPVCAL_CategoryTypeString(GSM_CalendarNoteType nType);

// append the reminder time
void SHARPVCAL_AlarmCat(char* Buffer, int* Length, GSM_DateTime* StartTime , GSM_DateTime* AlarmTime, char* start);

/////////////////////////////////////////////////////////////////////////////
// Common

GSM_Error SHARPGSM_DecodeCalendarInfo(unsigned char *Buffer, int *Pos, int* Used, int* Total);
void SHARPGSM_LimitName4SIMCard(unsigned char* Name);
void SHARPGSM_PhonebookFindDefaultNameNumberGroup(GSM_MemoryEntry *entry, int *Name, int *Number, int *Group,int *LastName , int *FirstName);
void SHARPGSM_CalendarFindDefaultTextTimeAlarmPhoneRecurrance(GSM_CalendarEntry *entry, int *Text, int *Time, int *Alarm, int *Phone, int *Recurrance, int *RecurranceFreq, int *EndTime, int *Location, int *Description);
GSM_Error SHARPGSM_DecodeSharpPBInfo(unsigned char *Buffer, int *Pos, GSM_MemoryStatus *Status);
void WINAPI SHARPGSM_DecodeSharpDevInfo(unsigned char* Buffer, char* pszManufacturer, char* pszModel, char* pszIMEI);

/////////////////////////////////////////////////////////////////////////////
// SMS
GSM_Error SHARPAT_GetNextSMS(OnePhoneModel *pMobileInfo, GSM_MultiSMSMessage *sms, bool start,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SHARPAT_GetSMSStatus( GSM_SMSMemoryStatus *status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SHARPAT_GetSMS(OnePhoneModel *pMobileInfo, GSM_MultiSMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SHARPAT_AddSMS(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SHARPAT_DeleteSMS(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SHARPAT_SendSMS( GSM_SMSMessage *sms,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error SHARPAT_Initialise(OnePhoneModel *pMobileInfo,GSM_Error (*pSetATProtocolDatafn)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);


GSM_Error SHARPAT_GetReplyStatue(GSM_Protocol_Message	*msg,GSM_ATReplayInfo* pReplynfo);
GSM_Error SHARPAT_HandleCMSError(GSM_ATReplayInfo Replynfo);
GSM_Error SHARPAT_HandleCMEError(GSM_ATReplayInfo Replynfo);
void SHARPAT_DecodeDateTime(GSM_DateTime *dt, unsigned char *input);
GSM_Error SHARPAT_GenericReply(GSM_Protocol_Message msg);
GSM_Error SHARPAT_SetSMSMode(bool bPDUMode,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
void SHARPOBEX_ParseXML2(GSM_File *File, unsigned char Flag,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

#endif