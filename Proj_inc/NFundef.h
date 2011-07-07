/* (c) 2002-2005 by Marcin Wiacek */
#ifndef _NFUNDEF_H
#define _NFUNDEF_H
#include "commfun.h"
#include "gsmsms.h"		
#include "gsmmulti.h"		
#include "gsmcal.h"	
#include "gsmlogo.h"		
#include "gsmnet.h"		
	
#define N6110_FRAME_HEADER 0x00, 0x01, 0x00
#define N7110_FRAME_HEADER 0x00, 0x01, 0x01
#define PHONE_MAXSMSINFOLDER		200
#define SM30_OTA	  2
#define GSM_MAXCALENDARTODONOTES	2000  // by karl 

typedef enum {
	/* DCT3 and DCT4 */
	N7110_PBK_ERROR	 = 0, /* Speed dial on SIM			*/
	N7110_PBK_SIM_SPEEDDIAL	 = 0x04, /* Speed dial on SIM			*/
	N7110_PBK_NAME		 = 0x07, /* Text: name (always the only one) 	*/
	N7110_PBK_EMAIL		 = 0x08, /* Text: email adress			*/
	N7110_PBK_POSTAL	 = 0x09, /* Text: postal address 		*/
	N7110_PBK_NOTE		 = 0x0A, /* Text: note		 		*/
	N7110_PBK_NUMBER 	 = 0x0B, /* Phone number 			*/
	N7110_PBK_RINGTONE_ID	 = 0x0C, /* Ringtone ID 			*/
	N7110_PBK_DATETIME    	 = 0x13, /* Call register: date and time	*/
	N7110_PBK_UNKNOWN1	 = 0x19, /* Call register: with missed calls	*/
	N7110_PBK_SPEEDDIAL	 = 0x1A, /* Speed dial 				*/
	N7110_PBK_GROUPLOGO	 = 0x1B, /* Caller group: logo 			*/
	N7110_PBK_LOGOON	 = 0x1C, /* Caller group: is logo on ?		*/
	N7110_PBK_GROUP		 = 0x1E, /* Caller group number in pbk entry	*/

	/* DCT4 only */
	N6510_PBK_URL		 = 0x2C, /* Text: URL address 			*/
	N6510_PBK_SMSLIST_ID	 = 0x2E, /* SMS list assigment			*/
	N6510_PBK_VOICETAG_ID	 = 0x2F, /* Voice tag assigment			*/
	N6510_PBK_PICTURE_ID	 = 0x33, /* Picture ID assigment		*/
	N6510_PBK_RINGTONEFILE_ID= 0x37, /* Ringtone ID from filesystem/internal*/
	N6510_PBK_USER_ID        = 0x38,  /* Text: user ID  */
    N6510_PBK_PUSH_TALK      = 0x3F  /*Compare : Text: PUSH TALK , Added By Mingfa 0127 */ 
} GSM_71_65_Phonebook_Entries_Types;
typedef enum {
	N7110_PBK_NUMBER_HOME	 = 0x02,
	N7110_PBK_NUMBER_MOBILE	 = 0x03,
	N7110_PBK_NUMBER_FAX	 = 0x04,
	N7110_PBK_NUMBER_WORK	 = 0x06,
	N7110_PBK_NUMBER_GENERAL = 0x0A
} GSM_71_65_Phonebook_Number_Types;
/*
typedef enum {
	MEM7110_CG		 = 0xf0, /* Caller groups memory 		*/
/**	MEM7110_SP		 = 0xf1	 /* Speed dial memory 			*/
//} GSM_71_65MemoryType;

typedef struct {
	unsigned char	Location[PHONE_MAXSMSINFOLDER]; /* locations of SMS messages in that folder */
	int		Number;				/* number of SMS messages in that folder */
} GSM_NOKIASMSFolder;
typedef struct {
	int		Location[GSM_MAXCALENDARTODONOTES];
	int		Number;	
} GSM_NOKIACalToDoLocations;
void WINAPI NOKIA_DecodeDateTime(unsigned char* buffer, GSM_DateTime *datetime);
void WINAPI NOKIA_EncodeDateTime(unsigned char* buffer, GSM_DateTime *datetime);
void NOBEXGEN_FindNextDir_UnicodePath(unsigned char *Path, unsigned int *Pos, unsigned char *Return);
GSM_Error WINAPI N_EnableFunctions(char *buff,int len,GSM_Error (*WriteCommand) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI N_Initialise(GSM_Error (*WriteCommand) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI N_Terminate (GSM_ConnectionType ConnectionType,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI N_GetManufacturer(char* pManufacturer);
GSM_Error WINAPI N_GetModel(char* pszModel,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI N_GetFirmware(char* pszFirmwareVer,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI N_GetIMEI (char* pszIMEI,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error WINAPI N_GetDateTime(GSM_DateTime *date_time,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI N_SetDateTime(GSM_DateTime *date_time,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error  WINAPI N_Reset(bool hard,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error  WINAPI N_CheckConnectStatus(GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);


GSM_Error WINAPI N_GetNetworkInfo(GSM_NetworkInfo *netinfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI N_GetBatteryCharge(GSM_BatteryCharge *bat,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI N_GetSignalQuality(GSM_SignalQuality *sig,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
//// phonebook
unsigned char  WINAPI NOKIA_GetMemoryType( GSM_MemoryType memory_type, unsigned char *ID);
GSM_Error WINAPI N_GetMemoryStatus(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_GetMemory (GSM_MemoryEntry *entry,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_SetMemory(OnePhoneModel *pMobileInfo,GSM_MemoryEntry *entry,int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_DeleteMemory(GSM_MemoryEntry *entry, int control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_CheckSIMCard(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error N_GetBitmap(OnePhoneModel *pMobileInfo,GSM_Bitmap *Bitmap,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
//SMS
GSM_Error N_GetSMSStatus(GSM_SMSMemoryStatus *status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error N_GetSMSMessage(GSM_MultiSMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_GetNextSMSMessage(GSM_MultiSMSMessage *sms, bool start,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error N_SetSMS(GSM_SMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_AddSMS(GSM_SMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_DeleteSMSMessage(GSM_SMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_SendSMSMessage(GSM_SMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error N_GetNextSMSMessageBitmap(GSM_MultiSMSMessage *sms, bool start, GSM_Bitmap *bitmap,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
//Calendar
GSM_Error N_GetCalendarStatus(OnePhoneModel *pMobileInfo,GSM_CalendarStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error N_GetNextCalendar(OnePhoneModel *pMobileInfo,GSM_CalendarEntry *Note, bool start,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error N_AddCalendar(OnePhoneModel *pMobileInfo, GSM_CalendarEntry *Note, int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error N_DelCalendar(GSM_CalendarEntry *Note, int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
//file
GSM_Error N_GetNextFileFolder(OnePhoneModel *pMobileInfo,GSM_File *File, bool start,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_GetFileFolderInfo(OnePhoneModel *pMobileInfo,GSM_File *File, int request,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error N_GetFilePart(OnePhoneModel *pMobileInfo, GSM_File *File,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_AddFilePart(OnePhoneModel *pMobileInfo, GSM_File *File, int *Pos,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_GetFileSystemStatus(OnePhoneModel *pMobileInfo, GSM_FileSystemStatus *status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_DeleteFile(OnePhoneModel *pMobileInfo,unsigned char *ID,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_AddFolder(OnePhoneModel *pMobileInfo, GSM_File *File,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
//obex

GSM_Error NObex40_SetATMode(GSM_ConnectionType ConnectionType,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error NObex40_SetMode(GSM_ConnectionType ConnectionType,bool bObexMode,OBEX_Service service,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error NObex40_DeleteFile(GSM_ConnectionType ConnectionType, GSM_File *ID,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error NObex40_AddFilePart(GSM_ConnectionType ConnectionType,GSM_File *File, int *Pos,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo,
							  int (*pGetStatusfn)(int nCur,int nTotal));
GSM_Error NObex40_GetFilePart(GSM_ConnectionType ConnectionType, GSM_File *File,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo
							  ,int (*pGetStatusfn)(int nCur,int nTotal));
GSM_Error NObex40_GetNextFileFolder(GSM_ConnectionType ConnectionType,GSM_File *File, bool start,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
void NOBEXGEN_ParseXML(GSM_File *File,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error NObex40_GetObexFileList(GSM_ConnectionType ConnectionType, GSM_File *File,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error NObex40_SetFolderPath(GSM_ConnectionType ConnectionType, GSM_File *File,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);



GSM_Error WINAPI N_SetFilesLocations(int Locations);
int WINAPI N_GetObjectSubList(int argv[]);
int WINAPI NObex40_GetOBEXSubList(GSM_File *filelist[]);
GSM_Error WINAPI NObex40_GetOBEXFolderListCount(int  *nFileNo);
//Todo

GSM_Error N_ReplyGetToDo(GSM_Protocol_Message msg);
GSM_Error N_GetCalendarInfo3(GSM_NOKIACalToDoLocations *Last, bool Calendar,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));
GSM_Error N_GetToDoStatus1(OnePhoneModel *pMobileInfo, GSM_ToDoStatus *status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_GetToDoStatus2(OnePhoneModel *pMobileInfo, GSM_ToDoStatus *status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_FindCalendarIconID3(OnePhoneModel *pMobileInfo,GSM_CalendarEntry *Entry, unsigned char *ID,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));
GSM_Error N71_65_DelCalendar(GSM_CalendarEntry *Note, int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));

GSM_Error N_GetToDoStatus(OnePhoneModel *pMobileInfo, GSM_ToDoStatus *status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_AddToDo(OnePhoneModel *pMobileInfo, GSM_ToDoEntry *ToDo, int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_GetNextToDo(OnePhoneModel *pMobileInfo, GSM_ToDoEntry *ToDo, bool refresh,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_DeleteToDo2(OnePhoneModel *pMobileInfo, GSM_ToDoEntry *ToDo, int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_ReplyDeleteCalendarNote(GSM_Protocol_Message msg);
GSM_Error N_GetCalendarNotePos3(GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));

GSM_Error N_GetMemory_S40 (GSM_MemoryEntry *entry,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error N_SetMemory_S40(OnePhoneModel *pMobileInfo,GSM_MemoryEntry *entry,int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error WINAPI N_DeleteMMSFile(OnePhoneModel *pMobileInfo,GSM_File *file,int ControlType,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI N_AddMMSFilePart(OnePhoneModel *pMobileInfo,GSM_File *ParentFile,GSM_File *File,int (*pGetStatusfn)(int nCur,int nTotal),int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI N_GetMMSFilePart(OnePhoneModel *pMobileInfo,GSM_File *File,int (*pGetStatusfn)(int nCur,int nTotal),int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI N_GMMSFileList(OnePhoneModel *pMobileInfo,GSM_File *ParentFile, GSM_File *childFile,bool bStart,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI N_MMSInit(GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI N_MMSTerminate(GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

int N_GetFolderSubList(OnePhoneModel *pMobileInfo,GSM_File *File,int argv[],GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
#endif