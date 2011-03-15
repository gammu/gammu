/* (c) 2003 by Marcin Wiacek */

#ifndef obexgen_h
#define obexgen_h
#include "mbglobals.h"
#include "commfun.h"

typedef struct {
	char			m_ConnectID[4]; 
	int				m_FrameSize;
} OBEX_ConnectInfo;


GSM_Error WINAPI OBEXGEN_Initialise2(OnePhoneModel *pMobileInfo, Debug_Info *pDebuginfo);


GSM_Error OBEXGEN_Disconnect(GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error OBEXGEN_DisconnectEx(bool bHaveConnectID,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error OBEXGEN_Initialise(Debug_Info	*pDebuginfo);


GSM_Error OBEXGEN_GetNextFileFolder(GSM_File *File, bool start,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error OBEXGEN_DeleteFile(GSM_File *ID,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error OBEXGEN_AddFolder(GSM_File *File,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error OBEXGEN_GetFileFolderInfo(GSM_File *File,int Request,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

GSM_Error OBEXGEN_ChangePath(char *Name, unsigned char Flag1,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error OBEXGEN_Connect(OBEX_Service service,OBEX_ConnectInfo *pConnectInf,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error OBEXGEN_PrivGetFilePart(GSM_File *File, bool FolderList,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo,
						  int (*pGetStatusfn)(int nCur,int nTotal));
GSM_Error OBEXGEN_GetFilePart(GSM_File *File,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo,
						  int (*pGetStatusfn)(int nCur,int nTotal));

GSM_Error OBEXGEN_AddFilePart(GSM_File *File, int *Pos,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo,
							  int (*pGetStatusfn)(int nCur,int nTotal));
void OBEXGEN_FindNextDir_UnicodePath(unsigned char *Path, unsigned int *Pos, unsigned char *Return);

void OBEXGEN_FindNextDir(unsigned char *Path, unsigned int *Pos, unsigned char *Return);
void OBEXGEN_ParseXML(GSM_File *File,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI OBEXGEN_GetOBEXFolderListCount(int  *nFileNo);
int WINAPI OBEXGEN_GetOBEXSubList(GSM_File *filelist[]);
GSM_Error OBEXGEN_Connect2(unsigned char *Buffer,int BufferSize,OBEX_Service service,OBEX_ConnectInfo *pConnectInf,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error OBEXGEN_GetNextFileFolder2(GSM_File *File, bool start,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
GSM_Error WINAPI OBEXGEN_SetFolderPath(GSM_File *File,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);
void OBEXAddBlock(unsigned char *Buffer, int *Pos, unsigned char ID,unsigned char *AddBuffer, int AddLength);
GSM_Error OBEXGEN_CheckConnection(GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo);

void WINAPI OBEXGEN_ClearFolderListInfo();
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
