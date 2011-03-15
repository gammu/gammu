#ifndef _ATGERFUNDEF_H
#define _ATGERFUNDEF_H
#include "commfun.h"
#include "gsmsms.h"		
#include "gsmmulti.h"		
#include "gsmcal.h"	
#include "gsmlogo.h"		
#include "gsmnet.h"		
#define AT_PBK_MAX_MEMORIES	200
#define PHONE_MAXSMSINFOLDER		200
#define SAMSUNGE_MAXSMSINFOLDER		201

typedef struct {
	int     Number;
	char    Text[60];
} ATErrorCode;

typedef enum {
	AT_Reply_OK = 1,
	AT_Reply_Connect,
	AT_Reply_Error,
	AT_Reply_Unknown,
	AT_Reply_CMSError,
	AT_Reply_CMEError,
	AT_Reply_SMSEdit,
	AT_Reply_Continue,
	AT_Reply_Entered,
} GSM_AT_Reply_State;

typedef enum {
	AT_SMSMemory_ERROR= 0,
	AT_AVAILABLE = 1,
	AT_NOTAVAILABLE
} GSM_AT_SMSMemory;

typedef enum {
	AT_SBNR_ERROR =0 ,
	AT_SBNR_AVAILABLE = 1,
	AT_SBNR_NOTAVAILABLE
} GSM_AT_SBNR;

typedef enum {
	AT_PBK_ERROR = 0,
	AT_PBK_HEX = 1,
	AT_PBK_GSM,
	AT_PBK_UCS2,
	AT_PBK_PCCP437,
	AT_PBK_UTF8
} GSM_AT_PBK_Charset;
typedef struct {
	GSM_AT_PBK_Charset m_PBKCharset;
	bool			m_UCS2CharsetFailed;	/* Whether setting of UCS2 charset has already failed 		*/
	bool			m_NonUCS2CharsetFailed;	/* Whether setting of non-UCS2 charset has already failed 	*/
} ATCharsetInfo;

typedef struct {
	int FirstMemoryEntry;
	int MemorySize;
	int TextLength;
	int NumberLength;

} GSM_MemoryInfo;
typedef struct {
	GSM_AT_SMSMemory	m_PhoneSMSMemory;	  	/* Is phone SMS memory available ? 				*/
	GSM_AT_SMSMemory	m_SIMSMSMemory;	  	/* Is SIM SMS memory available ? 				*/
	bool			m_CanSaveSMS;

} GSM_SMSMemoryInfo;

typedef struct {
	GSM_AT_Reply_State	ReplyState;	  	/* What response type - error, OK, etc. 			*/
	int			ErrorCode;	  	/* Error codes from responses					*/
    char			ErrorText[MAX_PATH];    	  	/* Error description 						*/
} GSM_ATReplayInfo;

typedef enum {
	AT_Nokia = 1,
	AT_Alcatel,
	AT_Siemens,
	AT_HP,
	AT_Falcom,
	AT_Ericsson,
	AT_Sagem,
	AT_Samsung,
	AT_Motorola,
	AT_Sharp,
	AT_Panasonic,
	AT_LG,
	AT_SAGEM,
	AT_Toshiba,
	AT_MTK,
	AT_Unknown
} GSM_AT_Manufacturer;
typedef enum {
	AT_Status,
	AT_NextEmpty,
	AT_Total,
	AT_First,
	AT_Sizes
} GSM_AT_NeededMemoryInfo;
typedef enum {
	SMS_AT_ERROR = 0,
	SMS_AT_PDU = 1,
	SMS_AT_TXT
} GSM_AT_SMS_Modes;

typedef struct 
{
	int		     	Number;
	int				nIndex[3000];
} SMSUsedLocation;

GSM_Error ATGEN_GetModel2(char* pszModel,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GenericReply(GSM_Protocol_Message msg);
GSM_Error ATGEN_GetReplyStatue(GSM_Protocol_Message	*msg,GSM_ATReplayInfo* pReplynfo);
GSM_Error ATGEN_HandleCMEError(GSM_ATReplayInfo Replynfo);
GSM_Error ATGEN_HandleCMSError(GSM_ATReplayInfo Replynfo);

void ATGEN_DecodeDateTime(GSM_DateTime *dt, unsigned char *input);
GSM_Error ATGEN_GetManufacturer(char* pszManufacturer,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_SetPBKCharset(bool PreferUnicode,ATCharsetInfo *pCharsetInfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_Initialise(OnePhoneModel *pMobileInfo,GSM_Error (*pSetATProtocolDatafn)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_Terminate(GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetFirmware(char* pszFirmwareVer,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetModel(char* pszModel,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetIMEI (char* pszIMEI,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetDateTime( GSM_DateTime *date_time,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_SetDateTime(GSM_DateTime *date_time,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_Reset( bool hard,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error ATGEN_GetNetworkInfo(GSM_NetworkInfo *netinfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetSignalQuality(GSM_SignalQuality *sig,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetBatteryCharge(GSM_BatteryCharge *bat,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);



GSM_Error ATGEN_GetMemoryInfo(GSM_MemoryStatus *Status,GSM_MemoryInfo* pMemoryInfo, GSM_AT_NeededMemoryInfo NeededInfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetMemoryStatus(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_PrivGetMemory (GSM_MemoryEntry *entry, int endlocation,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetMemory (GSM_MemoryEntry *entry,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_PrivSetMemory(GSM_MemoryEntry *entry,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_SetMemory(GSM_MemoryEntry *entry,int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_DeleteMemory(GSM_MemoryEntry *entry , int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_InitPBKGet(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetSMSMemories(GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_SetSMSMemory(bool SIM,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetSMSStatus( GSM_SMSMemoryStatus *status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetSMSLocation(GSM_SMSMessage *sms, unsigned char *folderid, int *location,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetSMSMode(GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error ATGEN_GetSMS( OnePhoneModel *pMobileInfo,GSM_MultiSMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetNextSMS(OnePhoneModel *pMobileInfo, GSM_MultiSMSMessage *sms, bool start,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_SetSMSC(GSM_SMSC *smsc,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_MakeSMSFrame(GSM_SMSMessage *message, unsigned char *hexreq, int *current, int *length2,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_AddSMS( OnePhoneModel *pMobileInfo,GSM_SMSMessage *sms,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_DeleteSMS( OnePhoneModel *pMobileInfo,GSM_SMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_SendSMS( GSM_SMSMessage *sms,GSM_Error (* pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
unsigned char *ATGEN_EncodePHKString (const unsigned char *src,int* len);
GSM_Error ATGEN_SetPBKUCS2Charset(ATCharsetInfo *pCharsetInfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
void ATGEN_DecodePHKString (unsigned char *dest, unsigned char *buffer);
GSM_Error ATGEN_GetSMSMMGL(OnePhoneModel *pMobileInfo,GSM_MultiSMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetNextSMSMMGL(OnePhoneModel *pMobileInfo,GSM_MultiSMSMessage *sms, bool start,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_SendSMSMMGL(GSM_SMSMessage *sms,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_AddSMSMMGL(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_DeleteSMSMMGL(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error ATGEN_SetSMSMode(bool bPDUMode,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_SetPBKMemory(GSM_MemoryType MemType,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_ReplySetMemory(GSM_Protocol_Message msg);

GSM_Error ATGEN_SetPBKMemoryEx(GSM_MemoryType MemType,bool bCheckOldType,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);


GSM_Error ATGEN_InitPBKGetEx(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo,
							  int (*GetStatus)(int nCur,int nTotal));
GSM_Error ATGEN_InitPBKGetEx2(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo,
							  int (*GetStatus)(int nCur,int nTotal),int nMax);

GSM_Error ATGEN_GetNextSMS4Sharp(OnePhoneModel *pMobileInfo, GSM_MultiSMSMessage *sms, bool start,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_InitPBKGet4Sharp(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_GetMemoryStatus4Sharp(GSM_MemoryStatus *Status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error ATGEN_SetMemoryEx(GSM_MemoryEntry *entry,bool bWithSpace,bool bWithIndex,int Control,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_PrivSetMemoryEx(GSM_MemoryEntry *entry,bool bWithSpace,bool bWithIndex,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_InitParameter(OnePhoneModel *pMobileInfo,Debug_Info	*pDebuginfo);

GSM_Error ATGEN_CDMA_GetModel(char* pszModel,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_CDMA_GetIMEI (char* pszIMEI,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_CDMA_GetManufacturer(char* pszManufacturer,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error ATGEN_CDMA_GetFirmware(char* pszFirmwareVer,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);


GSM_Error ATGEN_AddSMSForTextMode(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,bool bSetSMSMode,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);


GSM_Error ATGEN_SendSMSForTextMode( OnePhoneModel *pMobileInfo,GSM_SMSMessage *sms,bool bSetSMSMode,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error ATGEN_DecodeNokiaSMSFile(GSM_File *pfile,GSM_MultiSMSMessage *pSMS);

GSM_Error ATGEN_MakeNokiaSMSFrame(GSM_SMSMessage *message, unsigned char *hexreq,int *length2,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);


GSM_Error ATGEN_GetSMSStatus( GSM_SMSMemoryStatus *status,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error ATGEN_GetNextSMSEX(OnePhoneModel *pMobileInfo,GSM_MultiSMSMessage *sms, bool start, int startIndex,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_AddSMSEX(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,GSM_Error (*pSetATProtocolData)(bool EditMode,bool bFastWrite,DWORD dwFlag),GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error ATGEN_DeleteSMSEX(OnePhoneModel *pMobileInfo, GSM_SMSMessage *sms,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

#endif