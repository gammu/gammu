#ifndef _LGGENFUNDEF_H
#define _LGGENFUNDEF_H
#include "mbglobals.h"
#include "commFun.h"
#include "AtGenFundef.h"
void WINAPI LG_DecryptText(unsigned char *buffer,unsigned char *ucharOutput,int OutputBufferSize);
void WINAPI LG_EncryptText(unsigned char *pSrc,unsigned char *ucharOutput,int OutputBufferSize);
void WINAPI LG_EncryptBuffer(unsigned char *pSrc,unsigned char *ucharOutput,int nInputSize,int OutputBufferSize);
int WINAPI LG_DecryptBuffer(unsigned char *buffer,unsigned char *ucharOutput,int nInputSize,int OutputBufferSize);
GSM_Error WINAPI LG_GetReplyStatue(GSM_Protocol_Message	*msg,GSM_ATReplayInfo* pReplynfo,GSM_Lines *pLines);
GSM_Error WINAPI LG_GetCommandInfo(CString  &strStartTxfCmd,CString &strEndTxfCmd,CString &strCSCSCmd, GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));

GSM_Error WINAPI LG_SetPBKCharset(bool PreferUnicode,ATCharsetInfo *pCharsetInfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI LG_LGFunInit(GSM_Error (*pSetATProtocolDatafn)(bool EditMode,bool bFastWrite,DWORD dwFlag));
GSM_Error WINAPI LG_GetFreeSize(int* nFreeSize, GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));
GSM_Error WINAPI LG_GetFreeSize7050(int* nFreeSize, GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));

GSM_Error WINAPI LG_SetTxfMode( GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),bool bCheckOld=true);
GSM_Error WINAPI LG_EndTxfMode( GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));
GSM_Error WINAPI LG_EndTxfMode_MobileCancel( GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));
GSM_Error WINAPI LG_EndTxfModeWithoutCheck( GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));
bool WINAPI LG_CheckInFileTxfMode();
void WINAPI LG_SetInFileTxfMode( bool bInFileTxfMode);

GSM_Error WINAPI LG_DeleteFile(GSM_File *ID,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));
GSM_Error WINAPI LG_GetFilePart(GSM_File *File,int (*pGetStatusfn)(int nCur,int nTotal),bool bDecrypt,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));

GSM_Error WINAPI LG_AddFilePart(GSM_File *File, int *Pos,int (*pGetStatusfn)(int nCur,int nTotal),int nAddIndex,bool bEncry,bool bExternal,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));
GSM_Error WINAPI LGC3_AddFilePart(GSM_File *File, int *Pos,int (*pGetStatusfn)(int nCur,int nTotal),int nAddIndex,bool bEncry,bool bExternal,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));

GSM_Error WINAPI LGG7050_AddFilePart(GSM_File *File, int *Pos,int (*pGetStatusfn)(int nCur,int nTotal),int nAddIndex,bool bEncry,bool bExternal,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));
GSM_Error WINAPI LGG7050_GetFilePart(GSM_File *File,int (*pGetStatusfn)(int nCur,int nTotal),bool bDecrypt,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)));

#endif