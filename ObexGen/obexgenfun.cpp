/* (c) 2003 by Marcin Wiacek */
/* www.irda.org OBEX specs 1.3 */

/* Module connects to F9EC7BC4-953c-11d2-984E-525400DC9E09 UUID and in the
 * future there will required implementing reconnecting. See "ifdef xxxx"
 */

#include "stdafx.h"
#include <string.h>
#include <time.h>
#include "obexgenfun.h"
#include "ObexGen.h"
#include "commfun.h"
#include "coding.h"
#include "gsmcal.h"

#include "coding.h"
GSM_Reply_MsgType ObexReplymsgType;

extern CObexGenApp theApp;
void OBEXAddBlock(unsigned char *Buffer, int *Pos, unsigned char ID,unsigned char *AddBuffer, int AddLength)
{
	Buffer[(*Pos)++] = ID;
	Buffer[(*Pos)++] = (AddLength+3)/256;
	Buffer[(*Pos)++] = (AddLength+3)%256;
	if (AddBuffer != NULL) {
		memcpy(Buffer+(*Pos),AddBuffer,AddLength);
		(*Pos) += AddLength;
	}
}
void OBEXGEN_FindNextDir_UnicodePath(unsigned char *Path, unsigned int *Pos, unsigned char *Return)
{
	unsigned char temp1[10];
	unsigned char temp2[10];
	EncodeUnicode(temp1,(unsigned char*)"\\",2);
	EncodeUnicode(temp2,(unsigned char*)"/",2);

	Return[0] = 0;
	Return[1] = 0;
	while(1) 
	{
		if (Path[(*Pos)*2] == 0x00 && Path[(*Pos)*2+1] == 0x00) break;
		if ((Path[(*Pos)*2] == temp1[0] && Path[(*Pos)*2+1] == temp1[1] ) || 
			(Path[(*Pos)*2] == temp2[0] && Path[(*Pos)*2+1] == temp2[1]))
		{
			(*Pos)++;
			break;
		}
		int nLen = UnicodeLength(Return);
		Return[(nLen+1)*2] = 0;
		Return[(nLen+1)*2+1] = 0;
		nLen = UnicodeLength(Return);
		Return[nLen*2] =  Path[(*Pos)*2];
		Return[nLen*2+1] =  Path[(*Pos)*2+1];
		(*Pos)++;
	}
}
//static GSM_Error OBEXGEN_GetNextFileFolder(GSM_StateMachine *s, GSM_File *File, bool start);
void OBEXGEN_FindNextDir(unsigned char *Path, unsigned int *Pos, unsigned char *Return)
{
	unsigned char buff[800];

	buff[0] = 0;
	while(1) {
		if (Path[*Pos] == 0x00) break;
		if (Path[*Pos] == '\\'|| Path[*Pos] == '/') {
			(*Pos)++;
			break;
		}
		buff[strlen((char*)buff)+1] = 0;
		buff[strlen((char*)buff)]   = Path[(*Pos)];
		(*Pos)++;
	}
	EncodeUnicode(Return,buff,strlen((char*)buff));
}
static GSM_Error OBEXGEN_ReplyDeleteFilePart(GSM_Protocol_Message msg)
{
	int num=0;
	switch (msg.Type) {
	case 0x90:
		smprintf(theApp.m_pDebuginfo,"Last part of file added OK\n");
		return ERR_NONE;
	case 0xA0:
		smprintf(theApp.m_pDebuginfo,"Part of file added OK\n");
		return ERR_NONE;
	case 0xC0:
		smprintf(theApp.m_pDebuginfo,"Not understand. Probably not supported\n");
		return ERR_NOTSUPPORTED;
	}
	return ERR_UNKNOWNRESPONSE;
}


static GSM_Error OBEXGEN_ReplyAbortFilePart(GSM_Protocol_Message msg)
{
	int num=0;
	switch (msg.Type) {
	case 0x90:
		smprintf(theApp.m_pDebuginfo,"Last part of file added OK\n");
		return ERR_NONE;
	case 0xA0:
		smprintf(theApp.m_pDebuginfo,"Part of file added OK\n");
		return ERR_NONE;
	case 0xC0:
		smprintf(theApp.m_pDebuginfo,"Not understand. Probably not supported\n");
		return ERR_NOTSUPPORTED;
	}
	return ERR_UNKNOWNRESPONSE;
}


GSM_Error OBEXGEN_ReplyGetFilePart(GSM_Protocol_Message msg)
{
	int old,Pos=0;
	int tol=0;

	switch (msg.Type) {
	case 0xA0:
		//for T610
		while(1) 
		{
			if (Pos >= msg.Length-3) break;
			switch (msg.Buffer[Pos]) {
			{
				case 0xC3:
						old=0;
					//	tol=256*msg.Buffer[Pos+3] + msg.Buffer[Pos+4];
						tol=256*256*256*msg.Buffer[Pos+1] + 256*256*msg.Buffer[Pos+2]+256*msg.Buffer[Pos+3] + msg.Buffer[Pos+4];

						theApp.m_File->Used=0;
						if(theApp.m_File->Buffer!=NULL)
						{
						  theApp.m_File->Buffer=NULL;
						}
						theApp.m_File->Buffer = (unsigned char *)realloc(theApp.m_File->Buffer,tol+2);
						memset(theApp.m_File->Buffer,0,tol);
			//			memcpy(File->Buffer+old,msg.Buffer+8,msg.Length-8);
			//			Pos=Pos+msg.Length-8;
			//			File->Used=msg.Length-8;
			//			return ERR_NONE;
						Pos=Pos+5;
						continue;
					break;
				case 0x48:
				case 0x49:
				case 0xCB:
				{
					while( 1 )
					{
						if (Pos >= msg.Length) break;
						if( msg.Buffer[Pos] == 0x48 || msg.Buffer[Pos] == 0x49)
							break;
						if( msg.Buffer[Pos] == 0xC3 || msg.Buffer[Pos] == 0xCB)
							Pos+=5;
						else
							Pos+=msg.Buffer[Pos+1]*256+msg.Buffer[Pos+2];
					}
					if (Pos >= msg.Length) return ERR_NONE;;
					smprintf(theApp.m_pDebuginfo,"File part received\n");
					old = theApp.m_File->Used;
					//tol = old +	msg.BufferUsed;
					tol = old +	msg.Length;
					theApp.m_File->Used = tol-Pos-3;

					theApp.m_File->Buffer = (unsigned char *)realloc(theApp.m_File->Buffer,tol+5);
			//		memcpy(theApp.m_File->Buffer+Pos+old,msg.Buffer+3,msg.BufferUsed-3); //peggy -
					//memcpy(theApp.m_File->Buffer+Pos+old,msg.Buffer+Pos+3,msg.Length-Pos-3); //peggy -
					memcpy(theApp.m_File->Buffer+old,msg.Buffer+Pos+3,msg.Length-3); //peggy -
					theApp.m_File->Buffer[Pos+old+msg.Length-Pos-3] ='\0';//peggy
					theApp.m_FileLastPart = true;
					return ERR_NONE;
				}

			}
			default:
				break;
			}
			//Pos+=msg.BufferUsed-3;
			Pos+=msg.Buffer[Pos+1]*256+msg.Buffer[Pos+2];// roger Add
		}
		old=0;
		Pos=0;
		//----------------
		theApp.m_FileLastPart = true;
		return ERR_NONE;
	case 0x90:
//		if (msg.Length < 11) return ERR_NONE;
		if (msg.Type == 0x90) smprintf(theApp.m_pDebuginfo,"Last file part received\n");
		while(1) 
		{
			if (Pos >= msg.Length) break;
			switch (msg.Buffer[Pos]) 
			{
			case 0xC3:
					old=0;
				//	tol=256*msg.Buffer[Pos+3] + msg.Buffer[Pos+4];
					tol=256*256*256*msg.Buffer[Pos+1] + 256*256*msg.Buffer[Pos+2]+256*msg.Buffer[Pos+3] + msg.Buffer[Pos+4];

					theApp.m_File->Used=0;
					if(theApp.m_File->Buffer!=NULL)
					{
					  theApp.m_File->Buffer=NULL;
					}
					theApp.m_File->Buffer = (unsigned char *)realloc(theApp.m_File->Buffer,tol+2);
					memset(theApp.m_File->Buffer,0,tol);
		//			memcpy(File->Buffer+old,msg.Buffer+8,msg.Length-8);
		//			Pos=Pos+msg.Length-8;
		//			File->Used=msg.Length-8;
		//			return ERR_NONE;
					Pos=Pos+5;
					continue;
				break;
			case 0x48:
			case 0x49:
					smprintf(theApp.m_pDebuginfo,"File part received\n");
					old = theApp.m_File->Used;
					//tol = old +	msg.BufferUsed;
					tol = old +	msg.Length;
					theApp.m_File->Used = tol-3;

					theApp.m_File->Buffer = (unsigned char *)realloc(theApp.m_File->Buffer,tol);
					//memcpy(theApp.m_File->Buffer+Pos+old,msg.Buffer+3,msg.BufferUsed-3);
				//	memcpy(theApp.m_File->Buffer+old,msg.Buffer+3,msg.BufferUsed-3); 
			//		Pos+=msg.BufferUsed-3;//peggy -
					memcpy(theApp.m_File->Buffer+old,msg.Buffer+3,msg.Length-3); 
					Pos+=msg.Length-3; //peggy test
				return ERR_NONE;
			// Sony t610
			case 0xCB:
				{
				/*	while( 1 )
					{
						if( msg.Buffer[Pos] == 0x48 )
							break;
						Pos++;
					}	*/
					while(1)
					{
						if (Pos >= msg.Length) break;
						if( msg.Buffer[Pos] == 0x48 )
							break;
						if( msg.Buffer[Pos] == 0xC3 || msg.Buffer[Pos] == 0xCB)
							Pos+=5;
						else
							Pos+=msg.Buffer[Pos+1]*256+msg.Buffer[Pos+2];
					}
					if (Pos >= msg.Length) return ERR_NONE;

					smprintf(theApp.m_pDebuginfo,"File part received\n");
					old = theApp.m_File->Used;
					theApp.m_File->Used += msg.Buffer[Pos+1]*256+msg.Buffer[Pos+2]-3;
					smprintf(theApp.m_pDebuginfo,"Length of file part: %i\n",
									msg.Buffer[Pos+1]*256+msg.Buffer[Pos+2]-3);	
					//if(!old || theApp.m_File->Buffer==NULL)
							theApp.m_File->Buffer = (unsigned char *)realloc(theApp.m_File->Buffer,theApp.m_File->Used+2);
					memcpy(theApp.m_File->Buffer+old,msg.Buffer+Pos+3,theApp.m_File->Used-old);
					//msg.Buffer+=3+Pos;
					//memcpy(theApp.m_File->Buffer+old,msg.Buffer,msg.Length);	
					return ERR_NONE;
				}
		//------------
			default:
				break;
			}
			Pos+=msg.Buffer[Pos+1]*256+msg.Buffer[Pos+2];
		}
		return ERR_NONE;
	case 0xC3:
		return ERR_NOTSUPPORTED;
	case 0xC4:
		smprintf(theApp.m_pDebuginfo,"Not found\n");
		return ERR_SECURITYERROR;
	}
	return ERR_UNKNOWNRESPONSE;
}
GSM_Error OBEXGEN_ReplyConnect(GSM_Protocol_Message msg)
{ 
	
    int nPos = 0;
	int nMaxSize = 0;
	theApp.m_ConnectID[0]=0x00;
	theApp.m_ConnectID[1]=0x00;
	theApp.m_ConnectID[2]=0x00;
	theApp.m_ConnectID[3]=0x01;
	
	switch (msg.Type) {
	case 0xA0:
		smprintf(theApp.m_pDebuginfo,"Connected/disconnected OK\n");
		if (msg.Length != 0) {
			nMaxSize = msg.Buffer[2]*256+msg.Buffer[3];
			if(nMaxSize <512 )
				theApp.m_FrameSize = nMaxSize;
			else if(theApp.m_pMobileInfo && _stricmp(theApp.m_pMobileInfo->Manufacturer,"Nokia")== 0)
				theApp.m_FrameSize = nMaxSize;
			else
				theApp.m_FrameSize =nMaxSize;// 512;//msg.Buffer[2]*256+msg.Buffer[3];//for sony ericsson

			if (theApp.m_pMobileInfo && IsPhoneFeatureAvailable(theApp.m_pMobileInfo, F_SLOWOBEX))
			{
				if(theApp.m_FrameSize > 512)
					theApp.m_FrameSize = 512;
			}

			smprintf(theApp.m_pDebuginfo,"Maximal size of frame is %i 0x%x\n",theApp.m_FrameSize,theApp.m_FrameSize);
	
	
			nPos = 4;
			while(nPos< msg.Length)
			{
				if(msg.Buffer[nPos] == 0xCB)
				{
					memcpy(theApp.m_ConnectID,msg.Buffer+nPos+1,4);
					break;
				}
				nPos +=msg.Buffer[nPos+1]*256+msg.Buffer[nPos+2];
			}
	
		}
		return ERR_NONE;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error OBEXGEN_Disconnect(GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error error;
	int		Current=0;
	unsigned char 	req[200] ;
	smprintf(theApp.m_pDebuginfo, "Disconnecting\n");
	if(theApp.m_pMobileInfo==NULL || (theApp.m_pMobileInfo && strncmp(theApp.m_pMobileInfo->Manufacturer, "SHARP", 5)!=0))
//	if(theApp.m_pMobileInfo==NULL)
	{
		req[Current++] = 0xCB; // ID
		req[Current++] = theApp.m_ConnectID[0];
		req[Current++] = theApp.m_ConnectID[1];
		req[Current++] = theApp.m_ConnectID[2];
		req[Current++] = theApp.m_ConnectID[3];
	}

//	return GSM_WaitFor (s, NULL, 0, 0x81, 2, ID_Initialise);
//	error = GSM_WaitFor (s, req, Current, 0x81, 2, ID_Initialise);
	error = pWriteCommandfn (req, Current, 0x81, 12,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyConnect);
	if(error == ERR_NONE){
		theApp.m_Service = OBEX_Init;
	}
	if(theApp.m_pMobileInfo!=NULL && strncmp(theApp.m_pMobileInfo->Manufacturer, "SHARP", 5)==0){
		theApp.m_Service = OBEX_Init;
	}

	
	Sleep( 500 );

	return error;
}
GSM_Error OBEXGEN_Connect2(unsigned char *Buffer,int BufferSize,OBEX_Service service,OBEX_ConnectInfo *pConnectInfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error error;
	if (service == theApp.m_Service) return ERR_NONE;
	theApp.m_Service = service;
	
	error = pWriteCommandfn (Buffer, BufferSize, 0x80, 18,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyConnect);
	if(pConnectInfo)
	{
		pConnectInfo->m_ConnectID[0] = theApp.m_ConnectID[0];
		pConnectInfo->m_ConnectID[1] = theApp.m_ConnectID[1];
		pConnectInfo->m_ConnectID[2] = theApp.m_ConnectID[2];
		pConnectInfo->m_ConnectID[3] = theApp.m_ConnectID[3];
		pConnectInfo->m_FrameSize = theApp.m_FrameSize;
	}
	return error;
}
GSM_Error OBEXGEN_Connect(/*char* pszManufacturer,*/OBEX_Service service,OBEX_ConnectInfo *pConnectInfo,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	GSM_Error error;
	int		Current=4;
	unsigned char 	req2[200];
	unsigned char 	req[200] = {
		0x10,			/* Version 1.0 			*/
		0x00,			/* no flags 			*/
//		0x05,0x00};		/* 0x500 max size of packet 	*/
		0x40,0x00};		/* 0x500 max size of packet 	*/

	if (service == theApp.m_Service) return ERR_NONE;
	
	if(theApp.m_pMobileInfo && _stricmp(theApp.m_pMobileInfo->Manufacturer,"Nokia")== 0)
	{
		req[2] = 0x27;
	}

	switch (service) {
	case OBEX_None:
		break;
	case OBEX_BrowsingFolders:
		/* Server ID */

/*		if(mystrncasecmp((unsigned char *)pszManufacturer,(unsigned char *)"Siemens",0))
		{
			req2[0] = 0x6B; req2[1] = 0x01; req2[2] = 0xCB;
			req2[3] = 0x31; req2[4] = 0x41; req2[5] = 0x06;
			req2[6] = 0x11; req2[7] = 0xD4; req2[8] = 0x9A;
			req2[9] = 0x77; req2[10]= 0x00; req2[11]= 0x50;
			req2[12]= 0xDA; req2[13]= 0x3F; req2[14]= 0x47;
			req2[15]= 0x1F;
		}
		else*/
		{

			req2[0] = 0xF9; req2[1] = 0xEC; req2[2] = 0x7B;
			req2[3] = 0xC4; req2[4] = 0x95; req2[5] = 0x3C;
			req2[6] = 0x11; req2[7] = 0xD2; req2[8] = 0x98;
			req2[9] = 0x4E; req2[10]= 0x52; req2[11]= 0x54;
			req2[12]= 0x00; req2[13]= 0xDC; req2[14]= 0x9E;
			req2[15]= 0x09;
		}

		/* Target block */
		OBEXAddBlock(req, &Current, 0x46, req2, 16);
		break;
	case 	OBEX_SYNCML:
			req2[0] = 0x53; req2[1] = 0x59; req2[2] = 0x4E;
			req2[3] = 0x43; req2[4] = 0x4D; req2[5] = 0x4C;
			req2[6] = 0x2D; req2[7] = 0x53; req2[8] = 0x59;
			req2[9] = 0x4E; req2[10]= 0x43;// req2[11]= 0xC7;

		OBEXAddBlock(req, &Current, 0x46, req2, 11);
		break;


	}

#ifndef xxxx
	//disconnect old service
#else
	if (theApp.m_Service != 0) return ERR_NONE;
#endif


	smprintf(theApp.m_pDebuginfo, "Connecting\n");
//	return GSM_WaitFor (s, req, Current, 0x80, 6, ID_Initialise);
	error =  pWriteCommandfn (req, Current, 0x80, 18,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyConnect);
	
	if(pConnectInfo)
	{
		pConnectInfo->m_ConnectID[0] = theApp.m_ConnectID[0];
		pConnectInfo->m_ConnectID[1] = theApp.m_ConnectID[1];
		pConnectInfo->m_ConnectID[2] = theApp.m_ConnectID[2];
		pConnectInfo->m_ConnectID[3] = theApp.m_ConnectID[3];
		pConnectInfo->m_FrameSize = theApp.m_FrameSize;
	}
	if(error == ERR_NONE) theApp.m_Service = service;
	return error;
}

GSM_Error OBEXGEN_CheckConnection(GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	GSM_Error error;
	int		Current=4;
	unsigned char 	req[200] = {
		0x10,			/* Version 1.0 			*/
		0x00,			/* no flags 			*/
		0x05,0x00};		/* 0x500 max size of packet 	*/

//	error =  GSM_WaitFor (s, req, Current, 0x80, 6, ID_Initialise);
	error = pWriteCommandfn (req, Current, 0x80, 12,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyConnect);
	if(error == ERR_NONE)
	{
		theApp.m_Service = OBEX_None;
		return error;
	}
	if(error != ERR_TIMEOUT && error !=ERR_DEVICEWRITEERROR)
		error =  ERR_NONE;
	return error;
}

GSM_Error OBEXGEN_Initialise(Debug_Info	*pDebuginfo)
{

	theApp.m_Service = OBEX_Init;
	ObexReplymsgType.nCount = 39;
	ObexReplymsgType.CheckInfo[0].msgtype[0] = 0x90;
	ObexReplymsgType.CheckInfo[0].subtypechar = 0;
	ObexReplymsgType.CheckInfo[0].subtype = 0x00;

	ObexReplymsgType.CheckInfo[1].msgtype[0] = 0xA0;
	ObexReplymsgType.CheckInfo[1].subtypechar = 0;
	ObexReplymsgType.CheckInfo[1].subtype = 0x00;

	ObexReplymsgType.CheckInfo[2].msgtype[0] = 0xA1;
	ObexReplymsgType.CheckInfo[2].subtypechar = 0;
	ObexReplymsgType.CheckInfo[2].subtype = 0x00;

	ObexReplymsgType.CheckInfo[3].msgtype[0] = 0xA2;
	ObexReplymsgType.CheckInfo[3].subtypechar = 0;
	ObexReplymsgType.CheckInfo[3].subtype = 0x00;

	ObexReplymsgType.CheckInfo[4].msgtype[0] = 0xA3;
	ObexReplymsgType.CheckInfo[4].subtypechar = 0;
	ObexReplymsgType.CheckInfo[4].subtype = 0x00;

	ObexReplymsgType.CheckInfo[5].msgtype[0] = 0xA4;
	ObexReplymsgType.CheckInfo[5].subtypechar = 0;
	ObexReplymsgType.CheckInfo[5].subtype = 0x00;

	ObexReplymsgType.CheckInfo[6].msgtype[0] = 0xA5;
	ObexReplymsgType.CheckInfo[6].subtypechar = 0;
	ObexReplymsgType.CheckInfo[6].subtype = 0x00;

	ObexReplymsgType.CheckInfo[7].msgtype[0] = 0xA6;
	ObexReplymsgType.CheckInfo[7].subtypechar = 0;
	ObexReplymsgType.CheckInfo[7].subtype = 0x00;

	ObexReplymsgType.CheckInfo[8].msgtype[0] = 0xB0;
	ObexReplymsgType.CheckInfo[8].subtypechar = 0;
	ObexReplymsgType.CheckInfo[8].subtype = 0x00;

	ObexReplymsgType.CheckInfo[9].msgtype[0] = 0xB1;
	ObexReplymsgType.CheckInfo[9].subtypechar = 0;
	ObexReplymsgType.CheckInfo[9].subtype = 0x00;

	ObexReplymsgType.CheckInfo[10].msgtype[0] = 0xB2;
	ObexReplymsgType.CheckInfo[10].subtypechar = 0;
	ObexReplymsgType.CheckInfo[10].subtype = 0x00;

	ObexReplymsgType.CheckInfo[11].msgtype[0] = 0xB3;
	ObexReplymsgType.CheckInfo[11].subtypechar = 0;
	ObexReplymsgType.CheckInfo[11].subtype = 0x00;

	ObexReplymsgType.CheckInfo[12].msgtype[0] = 0xB4;
	ObexReplymsgType.CheckInfo[12].subtypechar = 0;
	ObexReplymsgType.CheckInfo[12].subtype = 0x00;

	ObexReplymsgType.CheckInfo[13].msgtype[0] = 0xB5;
	ObexReplymsgType.CheckInfo[13].subtypechar = 0;
	ObexReplymsgType.CheckInfo[13].subtype = 0x00;

	ObexReplymsgType.CheckInfo[14].msgtype[0] = 0xB6;
	ObexReplymsgType.CheckInfo[14].subtypechar = 0;
	ObexReplymsgType.CheckInfo[14].subtype = 0x00;

	ObexReplymsgType.CheckInfo[15].msgtype[0] = 0xC0;
	ObexReplymsgType.CheckInfo[15].subtypechar = 0;
	ObexReplymsgType.CheckInfo[15].subtype = 0x00;

	ObexReplymsgType.CheckInfo[16].msgtype[0] = 0xC1;
	ObexReplymsgType.CheckInfo[16].subtypechar = 0;
	ObexReplymsgType.CheckInfo[16].subtype = 0x00;

	ObexReplymsgType.CheckInfo[17].msgtype[0] = 0xC2;
	ObexReplymsgType.CheckInfo[17].subtypechar = 0;
	ObexReplymsgType.CheckInfo[17].subtype = 0x00;

	ObexReplymsgType.CheckInfo[18].msgtype[0] = 0xC3;
	ObexReplymsgType.CheckInfo[18].subtypechar = 0;
	ObexReplymsgType.CheckInfo[18].subtype = 0x00;

	ObexReplymsgType.CheckInfo[19].msgtype[0] = 0xC4;
	ObexReplymsgType.CheckInfo[19].subtypechar = 0;
	ObexReplymsgType.CheckInfo[19].subtype = 0x00;

	ObexReplymsgType.CheckInfo[20].msgtype[0] = 0xC5;
	ObexReplymsgType.CheckInfo[20].subtypechar = 0;
	ObexReplymsgType.CheckInfo[20].subtype = 0x00;

	ObexReplymsgType.CheckInfo[21].msgtype[0] = 0xC6;
	ObexReplymsgType.CheckInfo[21].subtypechar = 0;
	ObexReplymsgType.CheckInfo[21].subtype = 0x00;

	ObexReplymsgType.CheckInfo[22].msgtype[0] = 0xC7;
	ObexReplymsgType.CheckInfo[22].subtypechar = 0;
	ObexReplymsgType.CheckInfo[22].subtype = 0x00;

	ObexReplymsgType.CheckInfo[23].msgtype[0] = 0xC8;
	ObexReplymsgType.CheckInfo[23].subtypechar = 0;
	ObexReplymsgType.CheckInfo[23].subtype = 0x00;

	ObexReplymsgType.CheckInfo[24].msgtype[0] = 0xC9;
	ObexReplymsgType.CheckInfo[24].subtypechar = 0;
	ObexReplymsgType.CheckInfo[24].subtype = 0x00;

	ObexReplymsgType.CheckInfo[25].msgtype[0] = 0xCA;
	ObexReplymsgType.CheckInfo[25].subtypechar = 0;
	ObexReplymsgType.CheckInfo[25].subtype = 0x00;

	ObexReplymsgType.CheckInfo[26].msgtype[0] = 0xCB;
	ObexReplymsgType.CheckInfo[26].subtypechar = 0;
	ObexReplymsgType.CheckInfo[26].subtype = 0x00;

	ObexReplymsgType.CheckInfo[27].msgtype[0] = 0xCC;
	ObexReplymsgType.CheckInfo[27].subtypechar = 0;
	ObexReplymsgType.CheckInfo[27].subtype = 0x00;

	ObexReplymsgType.CheckInfo[28].msgtype[0] = 0xCD;
	ObexReplymsgType.CheckInfo[28].subtypechar = 0;
	ObexReplymsgType.CheckInfo[28].subtype = 0x00;

	ObexReplymsgType.CheckInfo[29].msgtype[0] = 0xCE;
	ObexReplymsgType.CheckInfo[29].subtypechar = 0;
	ObexReplymsgType.CheckInfo[29].subtype = 0x00;

	ObexReplymsgType.CheckInfo[30].msgtype[0] = 0xCF;
	ObexReplymsgType.CheckInfo[30].subtypechar = 0;
	ObexReplymsgType.CheckInfo[30].subtype = 0x00;

	ObexReplymsgType.CheckInfo[31].msgtype[0] = 0xD0;
	ObexReplymsgType.CheckInfo[31].subtypechar = 0;
	ObexReplymsgType.CheckInfo[31].subtype = 0x00;

	ObexReplymsgType.CheckInfo[32].msgtype[0] = 0xD1;
	ObexReplymsgType.CheckInfo[32].subtypechar = 0;
	ObexReplymsgType.CheckInfo[32].subtype = 0x00;

	ObexReplymsgType.CheckInfo[33].msgtype[0] = 0xD2;
	ObexReplymsgType.CheckInfo[33].subtypechar = 0;
	ObexReplymsgType.CheckInfo[33].subtype = 0x00;

	ObexReplymsgType.CheckInfo[34].msgtype[0] = 0xD3;
	ObexReplymsgType.CheckInfo[34].subtypechar = 0;
	ObexReplymsgType.CheckInfo[34].subtype = 0x00;

	ObexReplymsgType.CheckInfo[35].msgtype[0] = 0xD4;
	ObexReplymsgType.CheckInfo[35].subtypechar = 0;
	ObexReplymsgType.CheckInfo[35].subtype = 0x00;

	ObexReplymsgType.CheckInfo[36].msgtype[0] = 0xD5;
	ObexReplymsgType.CheckInfo[36].subtypechar = 0;
	ObexReplymsgType.CheckInfo[36].subtype = 0x00;

	ObexReplymsgType.CheckInfo[37].msgtype[0] = 0xE0;
	ObexReplymsgType.CheckInfo[37].subtypechar = 0;
	ObexReplymsgType.CheckInfo[37].subtype = 0x00;


	ObexReplymsgType.CheckInfo[38].msgtype[0] = 0xE1;
	ObexReplymsgType.CheckInfo[38].subtypechar = 0;
	ObexReplymsgType.CheckInfo[38].subtype = 0x00;
	return ERR_NONE;
}

//=================================================================================
//---ChangePath------------------------
static GSM_Error OBEXGEN_ReplyChangePath(GSM_Protocol_Message msg)
{                
	switch (msg.Type) {
	case 0xA0:
		smprintf(theApp.m_pDebuginfo,"Path set OK\n");
		return ERR_NONE;
	case 0xA1:
		smprintf(theApp.m_pDebuginfo,"Folder created\n");
		return ERR_NONE;
	case 0xC3:
		smprintf(theApp.m_pDebuginfo,"Security error\n");
		return ERR_SECURITYERROR;
	}
	return ERR_UNKNOWNRESPONSE;
}

GSM_Error OBEXGEN_ChangePath(char *Name, unsigned char Flag1,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	unsigned char 	req[800];
	int		Current = 2;

	/* Flags */
	req[0] = Flag1;
	req[1] = 0x00;

	req[Current++] = 0xCB; // ID
//	req[Current++] = 0x00; req[Current++] = 0x00;
//	req[Current++] = 0x00; req[Current++] = 0x01;
	req[Current++] = theApp.m_ConnectID[0];
	req[Current++] = theApp.m_ConnectID[1];
	req[Current++] = theApp.m_ConnectID[2];
	req[Current++] = theApp.m_ConnectID[3];

	/* Name block */
	if (Name != NULL && UnicodeLength((unsigned char *)Name) != 0) {
		OBEXAddBlock(req, &Current, 0x01, (unsigned char *)Name, UnicodeLength((unsigned char *)Name)*2+2);
	} else {
		OBEXAddBlock(req, &Current, 0x01, NULL, 0);
	}


	/* connection ID block */
/*	req[Current++] = 0xCB; // ID
//	req[Current++] = 0x00; req[Current++] = 0x00;
//	req[Current++] = 0x00; req[Current++] = 0x01;
	req[Current++] = theApp.m_ConnectID[0];
	req[Current++] = theApp.m_ConnectID[1];
	req[Current++] = theApp.m_ConnectID[2];
	req[Current++] = theApp.m_ConnectID[3];
*/
//	return GSM_WaitFor (s, req, Current, 0x85, 4, ID_SetPath);
	return  pWriteCommandfn (req, Current, 0x85, 8,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyChangePath);
}

//=================================================================================
//---GetFileFolderInfo------------------------
static GSM_Error OBEXGEN_ReplyGetFileInfo(GSM_Protocol_Message msg)
{
	static int old=0,Pos=0;
	int free=0,used=0,tol=0;
	GSM_File	 	*File = theApp.m_File;
	old =0;
	Pos = 0;
	switch (msg.Type) {
	case 0x83:
		smprintf(theApp.m_pDebuginfo,"Not available ?\n");
		return ERR_NONE;
		//For Sony T610
	case 0x90:
		while(1) {
			if (Pos >= msg.Length) break;
			switch (msg.Buffer[Pos]) 
			{
				case 0xC3:
						old=0;
					//	tol=256*msg.Buffer[Pos+3] + msg.Buffer[Pos+4];
						tol=256*256*256*msg.Buffer[Pos+1] + 256*256*msg.Buffer[Pos+2]+256*msg.Buffer[Pos+3] + msg.Buffer[Pos+4];

						File->Used=0;
						if(File->Buffer!=NULL)
						{
						  File->Buffer=NULL;
						}
						File->FileSize = tol;
						File->Buffer = (unsigned char *)realloc(File->Buffer,tol+2);
						memset(File->Buffer,0,tol);
			//			memcpy(File->Buffer+old,msg.Buffer+8,msg.Length-8);
			//			Pos=Pos+msg.Length-8;
			//			File->Used=msg.Length-8;
			//			return ERR_NONE;
						Pos=Pos+5;
						continue;
					break;
				case 0x48:
						old=File->Used;
						tol=old+256*msg.Buffer[Pos+1] + msg.Buffer[Pos+2];
						File->Used=tol-3;

						File->Buffer = (unsigned char *)realloc(File->Buffer,File->Used);

						memcpy(File->Buffer+old,msg.Buffer+Pos+3,msg.Length-3-Pos);
						
						return ERR_NONE;			
					break;
				case 0x90:
						free=256*msg.Buffer[3] + msg.Buffer[4];
						used=256*msg.Buffer[5] + msg.Buffer[6];
						memcpy(File->Buffer+Pos,msg.Buffer+8,msg.BufferUsed-8);
						Pos=Pos+msg.BufferUsed-8;
						File->Used=Pos;
						return ERR_NONE;
					break;
				case 0xA0:
				case 0xC0:
						return ERR_EMPTY;
					break;
					//T610 above
				case 0xCB:
						while( 1 )
						{
							if( msg.Buffer[Pos] == 0x48|| msg.Buffer[Pos] == 0x49 )
								break;
							if( msg.Buffer[Pos] == 0xC3 || msg.Buffer[Pos] == 0xCB)
								Pos+=5;
							else
								Pos+=msg.Buffer[Pos+1]*256+msg.Buffer[Pos+2];
							if(Pos >= msg.Length)
								break;
					//		Pos++;
						}	
						if(Pos >= msg.Length)
							break;
						if( msg.Buffer[Pos] == 0x49 || msg.Buffer[Pos] == 0x48 )
						{
							old=0;
							tol=256*msg.Buffer[Pos+1] + msg.Buffer[Pos+2];
							File->Used=0;
							if(File->Buffer!=NULL)
							{
							  File->Buffer=NULL;
							}
							File->Buffer = (unsigned char *)realloc(File->Buffer,tol
								+2);
							memset(File->Buffer,0,tol);
							memcpy(File->Buffer+old,msg.Buffer+(Pos+3),msg.Length-(Pos+3));
							//Pos=Pos+msg.Length-(Pos+3);
							Pos=msg.Length-(Pos+3);
							File->Used=Pos;//msg.Length-(8);
							return ERR_NONE;
					break;
						}
				/*default:
						return ERR_NONE;
					break;*/
			}
			Pos+=msg.Buffer[Pos+1]*256+msg.Buffer[Pos+2];
		}

		smprintf(theApp.m_pDebuginfo,"Last part of file info received\n");
		return ERR_NONE;
	case 0xA0:
		while(1) {
			if (Pos >= msg.Length) break;
			switch (msg.Buffer[Pos]) {
			case 0x48:
			case 0x49:
				/* SE T310 */
				smprintf(theApp.m_pDebuginfo,"File part received\n");
				old = theApp.m_File->Used;
				theApp.m_File->Used += msg.Buffer[Pos+1]*256+msg.Buffer[Pos+2]-3;
				smprintf(theApp.m_pDebuginfo,"Length of file part: %i\n",
						msg.Buffer[Pos+1]*256+msg.Buffer[Pos+2]-3);	
				theApp.m_File->Buffer = (unsigned char *)realloc(theApp.m_File->Buffer,theApp.m_File->Used+2);
				memcpy(theApp.m_File->Buffer+old,msg.Buffer+Pos+3,theApp.m_File->Used-old);
				theApp.m_FileLastPart = true;
				return ERR_EMPTY;
				//T610 above
			case 0xCB:
			case 0xC3:
				while( 1 )
				{
					if( msg.Buffer[Pos] == 0x48|| msg.Buffer[Pos] == 0x49 )
						break;
					if( msg.Buffer[Pos] == 0xC3 || msg.Buffer[Pos] == 0xCB)
						Pos+=5;
					else
						Pos+=msg.Buffer[Pos+1]*256+msg.Buffer[Pos+2];
			//		Pos++;
					if(Pos >= msg.Length)
						break;
				}	
				if(Pos >= msg.Length)
					break;
				if( msg.Buffer[Pos] == 0x49 || msg.Buffer[Pos] == 0x48 )
					{
						old=0;
						tol=256*msg.Buffer[Pos+1] + msg.Buffer[Pos+2];
						File->Used=0;
						if(File->Buffer!=NULL)
						{
						  File->Buffer=NULL;
						}
						File->Buffer = (unsigned char *)realloc(File->Buffer,tol+2);
						memset(File->Buffer,0,tol);
						memcpy(File->Buffer+old,msg.Buffer+(Pos+3),msg.Length-(Pos+3));
						File->Used=msg.Length-(Pos+3);
						Pos=Pos+msg.Length-(Pos+3);
						theApp.m_FileLastPart = true;
						return ERR_NONE;
					}

			default:
				theApp.m_FileLastPart = true;
				break;
			}
			Pos+=msg.Buffer[Pos+1]*256+msg.Buffer[Pos+2];
		}
		return ERR_UNKNOWNRESPONSE;
	}
	return ERR_UNKNOWNRESPONSE;
}


GSM_Error OBEXGEN_GetFileFolderInfo(GSM_File *File,int Request,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
	int 		Current = 0;
	unsigned int Pos;
	GSM_Error		error;
	unsigned char 		req[2000], req2[800];
//	unsigned char		*name,*name1;
	int name,name1;
	name=name1 = 0;

	theApp.m_File 	= File;
	File->ReadOnly 		= false;
	File->Protected 	= false;
	File->Hidden		= false;
	File->System		= false;

	req[Current++] = 0xCB; // ID
//	req[Current++] = 0x00; req[Current++] = 0x00;
//	req[Current++] = 0x00; req[Current++] = 0x01;
	req[Current++] = theApp.m_ConnectID[0];
	req[Current++] = theApp.m_ConnectID[1];
	req[Current++] = theApp.m_ConnectID[2];
	req[Current++] = theApp.m_ConnectID[3];

	if (File->Used == 0x00) 
	{
		if (File->Folder) 
		{
			//theApp.m_Service = OBEX_BrowsingFolders;
			/* Type block */
			strcpy((char*)req2,"x-obex/folder-listing");
			OBEXAddBlock(req, &Current, 0x42, req2, strlen((char*)req2)+1);
	
			/* Name block */
			if (UnicodeLength(File->Name) == 0x00) {
				OBEXAddBlock(req, &Current, 0x01, NULL, 0);
			} 
			else 
			{
				CopyUnicodeString(req2,File->Name);
				OBEXAddBlock(req, &Current, 0x01, req2, UnicodeLength(req2)*2+2);
			}
		} 
		else 
		{
			File->Folder = false;

			if (UnicodeLength(File->ID_FullName) == 0) 
			{
#ifndef xxxx
				error = OBEXGEN_Connect(OBEX_None,NULL,pWriteCommandfn,pDebuginfo);
#else
				error = OBEXGEN_Connect(OBEX_BrowsingFolders,NULL,pWriteCommandfn,pDebuginfo);
#endif
				if (error != ERR_NONE) return error;

				EncodeUnicode(File->Name,(unsigned char *)"one",3);

			//	if (strcmp(s->CurrentConfig->Connection,"seobex")) 
				{	
					strcpy((char*)req2,"x-obex/capability");
//					strcpy(req2,"x-obex/object-profile");

					/* Type block */
					OBEXAddBlock(req, &Current, 0x42, req2, strlen((char*)req2)+1);
				} 
			/*	else 
				{
					EncodeUnicode(req2,"telecom/devinfo.txt",19);

					/* Name block */
			/**		OBEXAddBlock(req, &Current, 0x01, req2, UnicodeLength(req2)*2+2);
				}*/
			} 
			else 
			{
//				error = OBEXGEN_Connect(s,OBEX_None);
				error = OBEXGEN_Connect(OBEX_BrowsingFolders,NULL,pWriteCommandfn,pDebuginfo);
				if (error != ERR_NONE) return error;

			//	if (mystrncasecmp(s->CurrentConfig->Connection,"seobex",0)) 
//				name = strstr(File->ID_FullName,"\\");
//				name1 = strstr(File->ID_FullName,"/");
				name = wstrstr(File->ID_FullName,(unsigned char *)"\\");
				name1 = wstrstr(File->ID_FullName,(unsigned char *)"/");
				if (name || name1)
				{
					smprintf(theApp.m_pDebuginfo,"Changing to root\n");
					error = OBEXGEN_ChangePath(NULL, 2,pWriteCommandfn,pDebuginfo);
					if (error != ERR_NONE) return error;

					Pos = 0;
					do 
					{
						OBEXGEN_FindNextDir_UnicodePath(File->ID_FullName, &Pos, req2);
						smprintf(theApp.m_pDebuginfo,"%s %i %i\n",DecodeUnicodeString(req2),Pos,strlen((char*)File->ID_FullName));
						if (Pos == (int)UnicodeLength(File->ID_FullName)) break;
						smprintf(theApp.m_pDebuginfo,"Changing path down\n");
						error=OBEXGEN_ChangePath((char*)req2, 2,pWriteCommandfn,pDebuginfo);
						if (error != ERR_NONE) return error;
					} while (1);
				} 
				else 
				{
					CopyUnicodeString(req2,File->ID_FullName);
				}
				CopyUnicodeString(File->Name,req2);

				theApp.m_File = File;
	
				Current = 0;
				/* Name block */
				OBEXAddBlock(req, &Current, 0x01, req2, UnicodeLength(req2)*2+2);
			}
		}
	}


	/* connection ID block */
/*	req[Current++] = 0xCB; // ID
//	req[Current++] = 0x00; req[Current++] = 0x00;
//	req[Current++] = 0x00; req[Current++] = 0x01;
	req[Current++] = theApp.m_ConnectID[0];
	req[Current++] = theApp.m_ConnectID[1];
	req[Current++] = theApp.m_ConnectID[2];
	req[Current++] = theApp.m_ConnectID[3];
*/
	smprintf(theApp.m_pDebuginfo, "Getting file info from filesystem\n");
//	error=GSM_WaitFor (s, req, Current, 0x03, 7, ID_GetFileInfo);
	error=pWriteCommandfn (req, Current, 0x03, 14,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyGetFileInfo);
	if (error != ERR_NONE) 
		return error;

	theApp.m_FileLastPart = false;

	while (!theApp.m_FileLastPart) 
	{
		Current = 0;

		/* connection ID block */
		req[Current++] = 0xCB; // ID
//		req[Current++] = 0x00; req[Current++] = 0x00;
//		req[Current++] = 0x00; req[Current++] = 0x01;
		req[Current++] = theApp.m_ConnectID[0];
		req[Current++] = theApp.m_ConnectID[1];
		req[Current++] = theApp.m_ConnectID[2];
		req[Current++] = theApp.m_ConnectID[3];

		smprintf(theApp.m_pDebuginfo, "Getting file part from filesystem\n");
//		error=GSM_WaitFor (s, req, Current, 0x83, 4, ID_GetFile);
		error=pWriteCommandfn (req, Current, 0x83, 20,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyGetFilePart);
		if (error != ERR_NONE) return error;
	}
	return ERR_EMPTY;

}


//=================================================================================
//---GetFilePart------------------------



GSM_Error OBEXGEN_PrivGetFilePart(GSM_File *File, bool FolderList,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo,
						  int (*pGetStatusfn)(int nCur,int nTotal))
{
	unsigned int 		 Pos;
	int 		Current = 0;
	GSM_Error		error;
	unsigned char 		req[2000], req2[1000];
//	unsigned char		*name,*name1;
	int name,name1;
	name=name1 = 0;

	theApp.m_File 	= File;
	File->ReadOnly 		= false;
	File->Protected 	= false;
	File->Hidden		= false;
	File->System		= false;
	File->FileSize = -1;

	req[Current++] = 0xCB; // ID
//	req[Current++] = 0x00; req[Current++] = 0x00;
//	req[Current++] = 0x00; req[Current++] = 0x01;
	req[Current++] = theApp.m_ConnectID[0];
	req[Current++] = theApp.m_ConnectID[1];
	req[Current++] = theApp.m_ConnectID[2];
	req[Current++] = theApp.m_ConnectID[3];

	if (File->Used == 0x00) 
	{
		if (FolderList) 
		{
			theApp.m_Service = OBEX_BrowsingFolders;
			/* Type block */
			strcpy((char*)req2,"x-obex/folder-listing");
			OBEXAddBlock(req, &Current, 0x42, req2, strlen((char*)req2)+1);
	
			/* Name block */
		/*	if (UnicodeLength(File->Name) == 0x00) {
				OBEXAddBlock(req, &Current, 0x01, NULL, 0);
			} 
			else 
			{
				CopyUnicodeString(req2,File->Name);
				OBEXAddBlock(req, &Current, 0x01, req2, UnicodeLength(req2)*2+2);
			}*/
		} 
		else 
		{
			File->Folder = false;

			if (UnicodeLength(File->ID_FullName) == 0) 
			{
/*#ifndef xxxx
				error = OBEXGEN_Connect(OBEX_None,NULL,pWriteCommandfn,pDebuginfo);
#else*/
				error = OBEXGEN_Connect(OBEX_BrowsingFolders,NULL,pWriteCommandfn,pDebuginfo);
//#endif
				if (error != ERR_NONE) return error;

				EncodeUnicode(File->Name,(unsigned char *)"one",3);

//				if (mystrncasecmp(s->CurrentConfig->Connection,"seobex",0)) 
				{	
					strcpy((char*)req2,"x-obex/capability");
//					strcpy(req2,"x-obex/object-profile");

					/* Type block */
					OBEXAddBlock(req, &Current, 0x42, req2, strlen((char*)req2)+1);
				} 
		/*		else 
				{
					EncodeUnicode(req2,"telecom/devinfo.txt",19);

					/* Name block */
			/*		OBEXAddBlock(req, &Current, 0x01, req2, UnicodeLength(req2)*2+2);
				}*/
			} 
			else 
			{
//				error = OBEXGEN_Connect(s,OBEX_None);
				error = OBEXGEN_Connect(OBEX_BrowsingFolders,NULL,pWriteCommandfn,pDebuginfo);
				if (error != ERR_NONE) return error;

			//	if (mystrncasecmp(s->CurrentConfig->Connection,"seobex",0)) 
//				name = strstr(File->ID_FullName,"\\");
//				name1 = strstr(File->ID_FullName,"/");
				name = wstrstr(File->ID_FullName,(unsigned char *)"\\");
				name1 = wstrstr(File->ID_FullName,(unsigned char *)"/");
				if (name || name1)
				{
					smprintf(theApp.m_pDebuginfo,"Changing to root\n");
					error = OBEXGEN_ChangePath(NULL, 2,pWriteCommandfn,pDebuginfo);
					if (error != ERR_NONE) return error;

					Pos = 0;
					do 
					{
						OBEXGEN_FindNextDir_UnicodePath(File->ID_FullName, &Pos, req2);
						smprintf(theApp.m_pDebuginfo,"%s %i %i\n",DecodeUnicodeString(req2),Pos,strlen((char*)File->ID_FullName));
						if (Pos == (int)UnicodeLength(File->ID_FullName)) break;
						smprintf(theApp.m_pDebuginfo,"Changing path down\n");
						error=OBEXGEN_ChangePath((char*)req2, 2,pWriteCommandfn,pDebuginfo);
						if (error != ERR_NONE) return error;
					} while (1);
				} 
				else 
				{
					int nLen = UnicodeLength(File->wFileFullPathName);
					if(nLen<=0) 
						CopyUnicodeString(req2,File->ID_FullName);
					else
					{
						GetFileName_Unicode(File->wFileFullPathName,req2);
					}
				//	EncodeUnicode(req2,File->ID_FullName,strlen((char*)File->ID_FullName));
				}
				CopyUnicodeString(File->Name,req2);
			
				theApp.m_File = File;
	
		//		Current = 0;
				/* Name block */
				OBEXAddBlock(req, &Current, 0x01, req2, UnicodeLength(req2)*2+2);
			}
		}
	}
	/* connection ID block */
/*	req[Current++] = 0xCB; // ID
//	req[Current++] = 0x00; req[Current++] = 0x00;
//	req[Current++] = 0x00; req[Current++] = 0x01;
	req[Current++] = theApp.m_ConnectID[0];
	req[Current++] = theApp.m_ConnectID[1];
	req[Current++] = theApp.m_ConnectID[2];
	req[Current++] = theApp.m_ConnectID[3];
*/

	theApp.m_FileLastPart = false;

	smprintf(theApp.m_pDebuginfo, "Getting file info from filesystem\n");
//	error=GSM_WaitFor (s, req, Current, 0x03, 7, ID_GetFileInfo);
	error=pWriteCommandfn (req, Current, 0x03, 14,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyGetFileInfo);
	if (error != ERR_NONE) 
		return error;


	int bContiGet = true;
	while (!theApp.m_FileLastPart && bContiGet) 
	{
		Current = 0;

		/* connection ID block */
		req[Current++] = 0xCB; // ID
//		req[Current++] = 0x00; req[Current++] = 0x00;
//		req[Current++] = 0x00; req[Current++] = 0x01;
		req[Current++] = theApp.m_ConnectID[0];
		req[Current++] = theApp.m_ConnectID[1];
		req[Current++] = theApp.m_ConnectID[2];
		req[Current++] = theApp.m_ConnectID[3];

		smprintf(theApp.m_pDebuginfo, "Getting file part from filesystem\n");
//		error=GSM_WaitFor (s, req, Current, 0x83, 4, ID_GetFile);
		error=pWriteCommandfn ( req, Current, 0x83, 30,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyGetFilePart);

		if (error != ERR_NONE) return error;
		if(pGetStatusfn)
			bContiGet = pGetStatusfn(File->Used,File->FileSize);
	}
	if(!bContiGet)
	{
		//Terminate OBEX Get File ..... // need add
		
		error=pWriteCommandfn ( req, Current, 0xFF, 8,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyAbortFilePart);
		return ERR_CANCELED;
	}
	return ERR_EMPTY;
}

GSM_Error OBEXGEN_GetFilePart(GSM_File *File,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo,
						  int (*pGetStatusfn)(int nCur,int nTotal))
{
	return OBEXGEN_PrivGetFilePart(File,false,pWriteCommandfn,pDebuginfo,pGetStatusfn);
}
void OBEXGEN_ParseXML(GSM_File *File,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)

{
//	GSM_Phone_OBEXGENData	*Priv = &theApp.m_Priv.OBEXGEN;

	GSM_Error		error;
	unsigned char		Line[2000],Line2[2000],*name,*size;
	int			Pos,i,j,num,pos2,z;
//	char			*strtemp;
	num = 0;
	Pos = 0;
	unsigned char temp[10];
	EncodeUnicode (temp, ( unsigned char *) "\\", 2);
	while (1) 
	{
		MyGetLine(File->Buffer, &Pos, Line, File->Used);
	//	if (strlen(Line) == 0) break;
		if (strlen((char*)Line) == 0)
		{
			Pos++;
			if(Pos >=File->Used)
				break;
		}
		name =(unsigned char *) strstr((char*)Line,"folder name=\"");
		if (name != NULL) 
		{
			name += 13;
			j = 0;
			while(1) 
			{
				if (name[j] == '"') break;
				j++;
			}
			name[j] = 0;

			if (strcmp((char*)name,".") && strlen((char*)name) >0) num++;
		}
		name =(unsigned char *) strstr((char*)Line,"file name=\"");
		if (name != NULL) num++;
	}
	if (num != 0) 
	{
		i = theApp.m_FilesLocationsUsed-1;
		while (1) 
		{
			if (i==theApp.m_FilesLocationsCurrent-1) break;
			memcpy(&theApp.m_Files[i+num],&theApp.m_Files[i],sizeof(GSM_File));
			i--;
		}
	}
	Pos 	= 0;
	pos2 	= 0;
	while (1) 
	{
		MyGetLine(File->Buffer, &Pos, Line, File->Used);
	//	if (strlen(Line) == 0) break;
		if (strlen((char*)Line) == 0)
		{
			Pos++;
			if(Pos >=File->Used)
				break;
		}
		strcpy((char*)Line2,(char*)Line);
		name = (unsigned char *)strstr((char*)Line2,"folder name=\"");
		if (name != NULL) 
		{
			name += 13;
			j = 0;
			while(1) 
			{
				if (name[j] == '"') break;
				j++;
			}
			name[j] = 0;
			if (strcmp((char*)name,".") && strlen((char*)name) >0) 
			{
			//	dbgprintf("copying folder %s to %i parent %i\n",name,theApp.m_FilesLocationsCurrent+pos2,theApp.m_FilesLocationsCurrent);
	
				theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Level  = File->Level+1;
				theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Folder = true;

				DecodeUTF8ToUnicode(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Name,name,strlen((char*)name)	);		
				UnicodeReplace(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Name,"&amp;","&");
				UnicodeReplace(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Name,"&apos;","'");

				CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].ID_FullName,File->ID_FullName);
				UnicodeCat(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].ID_FullName,temp);
				UnicodeCat(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].ID_FullName,theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Name);
		//////////////
				CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].wFileFullPathName,File->wFileFullPathName);
				UnicodeCat(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].wFileFullPathName,temp);
				UnicodeCat(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].wFileFullPathName,theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Name); 
///////////////////
				theApp.m_FilesLocationsUsed++;
				pos2++;
			}
		}
		strcpy((char*)Line2,(char*)Line);
		name = (unsigned char *)strstr((char*)Line2,"file name=\"");
		if (name != NULL) 
		{
			name += 11;
			j = 0;
			while(1) 
			{
				if (name[j] == '"') break;
				j++;
			}
			name[j] = 0;

			dbgprintf("copying file %s to %i\n",name,theApp.m_FilesLocationsCurrent+pos2);
			theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Level	= File->Level+1;
			theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Folder 	= false;

			DecodeUTF8ToUnicode(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Name,name,strlen((char*)name)	);		
			UnicodeReplace(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Name,"&amp;","&");
			UnicodeReplace(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Name,"&apos;","'");

			CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].ID_FullName,File->ID_FullName);
			UnicodeCat(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].ID_FullName,temp);
			UnicodeCat(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].ID_FullName,theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Name);
	//////////////
			CopyUnicodeString(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].wFileFullPathName,File->wFileFullPathName);
			UnicodeCat(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].wFileFullPathName,temp);
			UnicodeCat(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].wFileFullPathName,theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Name); 


			theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Used 	= 0;
			strcpy((char*)Line2,(char*)Line);
			size =(unsigned char *) strstr((char*)Line2,"size=\"");
			if (size != NULL)
			{
				theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Used = atoi((char*)size+6);
				theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].FileSize = strtoul((char *)(size+6), NULL, 10);
			}
			else theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].FileSize = -1;

			theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].ModifiedEmpty = true;
			strcpy((char*)Line2,(char*)Line);
			size = (unsigned char *)strstr((char*)Line2,"modified=\"");
			if (size != NULL) 
			{
				theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].ModifiedEmpty = false;
				ReadVCALDateTime((char *)size+10, &theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Modified);
			}
			else
				theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Modified.Year = 0;
//			if(theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Modified.Year >=3000 || theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Modified.Year<1900)
	//			theApp.m_Files[theApp.m_FilesLocationsCurrent+pos2].Modified.Year = 0;
			theApp.m_FilesLocationsUsed++;
			pos2++;
		}
	}

	z = theApp.m_FilesLocationsCurrent;
	if (z != 1) 
	{
		while (1) 
		{
			if (z == theApp.m_FilesLocationsUsed) break;
			if (theApp.m_Files[z].Folder) 
			{
				if (theApp.m_Files[z].Level > File->Level) 
				{
					smprintf(theApp.m_pDebuginfo,"Changing path down\n");
					error=OBEXGEN_ChangePath((char *)File->Name, 2,pWriteCommandfn,pDebuginfo);
					//if (error != ERR_NONE) return error;
					return;
				}
				break;
			} 
			z++;
		}
	}

	theApp.m_FileLev = File->Level;
	free(File->Buffer);
}
GSM_Error OBEXGEN_GetNextFileFolder2(GSM_File *File, bool start,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error error;
	unsigned char 		Name[300],FullName[400];
	int Pos = 0,i=0;
	if((!start && File->Folder) && (theApp.m_FilesLocationsUsed==1 && UnicodeLength(File->ID_FullName) != 0 ))
    {
		error = OBEXGEN_Connect(OBEX_BrowsingFolders,NULL,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;

		smprintf(pDebuginfo,"Changing path down\n");
		error=OBEXGEN_ChangePath(NULL, 2,pWriteCommandfn,pDebuginfo);

	//	strcpy((char*)FullName, (char*)File->ID_FullName);
		Pos = 0;
		do {
			OBEXGEN_FindNextDir_UnicodePath(File->ID_FullName, (unsigned int *)&Pos, Name);
	//		smprintf(pDebuginfo,"%s %i %i\n",DecodeUnicodeString(Name),Pos,strlen((char*)FullName));
			smprintf(pDebuginfo,"Changing path down\n");
			error=OBEXGEN_ChangePath((char*)Name, 2,pWriteCommandfn,pDebuginfo);
			if (error != ERR_NONE) return error;
			if (Pos == (int)UnicodeLength(File->ID_FullName)) break;
		} while (1);

		CopyUnicodeString(File->Name,Name);
		CopyUnicodeString(File->ID_FullName,Name);
	//	sprintf( (char*)File->Name,"%s", DecodeUnicodeString(Name) );
	//	sprintf((char*) File->ID_FullName,"%s", DecodeUnicodeString(Name) );
		memcpy(FullName,theApp.m_Files[0].ID_FullName,400);
		memcpy(Name,theApp.m_Files[0].Name,300);
		memset(theApp.m_Files[0].ID_FullName,0,400);
		memset(theApp.m_Files[0].Name,0,300);
		error =OBEXGEN_GetNextFileFolder( File,start,pWriteCommandfn,pDebuginfo);
		memcpy(File->ID_FullName,FullName,400);
		memcpy(File->Name,Name,300);
		for(i=1;i<theApp.m_FilesLocationsUsed;i++)
		{
			memset(FullName,0,400);
			memcpy(FullName,theApp.m_Files[i].ID_FullName,400);
			//DecodeUTF2String(FullName,FullName,strlen(FullName));//UTF8		
			//DecodeUTF2String(pathName,File->ID_FullName,strlen(File->ID_FullName));//UTF8		
			//sprintf(s->Phone.Data.Priv.OBEXGEN.Files[i].ID_FullName,"%s/%s",pathName,FullName);
//			sprintf((char*)theApp.m_Files[i].ID_FullName,"%s%s",File->ID_FullName,FullName);
			CopyUnicodeString(theApp.m_Files[i].ID_FullName,File->ID_FullName);
			UnicodeCat(theApp.m_Files[i].ID_FullName,FullName);
		}
	}				   
	else
	{
		error =OBEXGEN_GetNextFileFolder(File,start,pWriteCommandfn,pDebuginfo);
	}
	return error;
}
GSM_Error OBEXGEN_GetNextFileFolder(GSM_File *File, bool start,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
//	GSM_Phone_OBEXGENData	*Priv = &theApp.m_Priv.OBEXGEN;
	GSM_Error		error;
	int			i,Current;

	if (start) 
	{
//		if (!strcmp(s->CurrentConfig->Model,"seobex")) return ERR_NOTSUPPORTED;

		theApp.m_Files[0].Folder		= true;
		theApp.m_Files[0].Level		= 1;
		theApp.m_Files[0].Name[0]		= 0;
		theApp.m_Files[0].Name[1]		= 0;
		theApp.m_Files[0].ID_FullName[0]	= 0;
		theApp.m_Files[0].ID_FullName[1]	= 0;

		theApp.m_FilesLocationsUsed 	= 1;
		theApp.m_FilesLocationsCurrent 	= 0;
		theApp.m_FileLev			= 1;
		
		error = OBEXGEN_Connect(OBEX_BrowsingFolders,NULL,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;

		smprintf(theApp.m_pDebuginfo,"Changing to root\n");
		error = OBEXGEN_ChangePath(NULL, 2,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;

		Current = 0;
	}

	while (1) 
	{
		if (theApp.m_FilesLocationsCurrent == theApp.m_FilesLocationsUsed) 
		{
			dbgprintf("Last file\n");
			return ERR_EMPTY;
		}

//		strcpy((char*)File->ID_FullName,(char*)theApp.m_Files[theApp.m_FilesLocationsCurrent].ID_FullName);
		CopyUnicodeString(File->ID_FullName,theApp.m_Files[theApp.m_FilesLocationsCurrent].ID_FullName);
		File->Level	= theApp.m_Files[theApp.m_FilesLocationsCurrent].Level;
		File->Folder	= theApp.m_Files[theApp.m_FilesLocationsCurrent].Folder;
		CopyUnicodeString(File->Name,theApp.m_Files[theApp.m_FilesLocationsCurrent].Name);
		theApp.m_FilesLocationsCurrent++;

		if (File->Folder) 
		{
			if (File->Level < theApp.m_FileLev) 
			{
				for (i=0;i<File->Level;i++) 
				{
					smprintf(theApp.m_pDebuginfo,"Changing path up\n");
					error=OBEXGEN_ChangePath(NULL, 2,pWriteCommandfn,pDebuginfo);
					if (error != ERR_NONE) return error;
				}
			}
	
			smprintf(theApp.m_pDebuginfo,"Level %i %i\n",File->Level,theApp.m_FileLev);
	
//			File->Buffer 	= NULL;		
			File->Used 	= 0;
			OBEXGEN_PrivGetFilePart(File,true,pWriteCommandfn,pDebuginfo,NULL);
			//OBEXGEN_PrivGetFilePart(s, File,start);
			OBEXGEN_ParseXML(File,pWriteCommandfn,theApp.m_pDebuginfo);

		} 
		else 
		{
			File->Used 	    	= theApp.m_Files[theApp.m_FilesLocationsCurrent-1].Used;
			File->ModifiedEmpty 	= theApp.m_Files[theApp.m_FilesLocationsCurrent-1].ModifiedEmpty;
			if (!File->ModifiedEmpty) 
			{
				memcpy(&File->Modified,&theApp.m_Files[theApp.m_FilesLocationsCurrent-1].Modified,sizeof(GSM_DateTime));
			}
			File->ReadOnly 		= false;
			File->Protected 	= false;
			File->Hidden		= false;
			File->System		= false;

		}
		return ERR_NONE;
	}
}


//=================================================================================
//---DeleteFile------------------------

GSM_Error OBEXGEN_DeleteFile(GSM_File *ID,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error		error;
	int		Current = 0;
	unsigned int Pos;
	unsigned char		req[1000],req2[1000];
//	unsigned char		*name,*name1;
	int name,name1;
	name=name1 = 0;

//	if (!strcmp(s->CurrentConfig->Model,"seobex")) return ERR_NOTSUPPORTED;


	error = OBEXGEN_Connect(OBEX_BrowsingFolders,NULL,pWriteCommandfn,pDebuginfo);


	if (error != ERR_NONE) return error;

	Pos = 0;
//	name = strstr(ID,"\\");
//	name1 = strstr(ID,"/");
	name = wstrstr(ID->ID_FullName,(unsigned char *)"\\");
	name1 = wstrstr(ID->ID_FullName,(unsigned char *)"/");
	if (name || name1)
	{
		smprintf(theApp.m_pDebuginfo,"Changing to root\n");
		error = OBEXGEN_ChangePath(NULL, 2,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;

		do {
			OBEXGEN_FindNextDir_UnicodePath(ID->ID_FullName, &Pos, req2);
			//smprintf(theApp.m_pDebuginfo,"%s %i %i\n",DecodeUnicodeString(req2),Pos,strlen((char*)ID));
			if (Pos == (int)UnicodeLength(ID->ID_FullName)) break;
			smprintf(theApp.m_pDebuginfo,"Changing path down\n");
			error=OBEXGEN_ChangePath((char*)req2, 2,pWriteCommandfn,pDebuginfo);
			if (error != ERR_NONE) return error;
		} while (1);
	}
	else
	{
		int nLen = UnicodeLength(ID->wFileFullPathName);
		if(nLen<=0) 
			CopyUnicodeString(req2,ID->ID_FullName);
		else
			GetFileName_Unicode(ID->wFileFullPathName,req2);
	}


	req[Current++] = 0xCB; // ID
//	req[Current++] = 0x00; req[Current++] = 0x00;
//	req[Current++] = 0x00; req[Current++] = 0x01;
	req[Current++] = theApp.m_ConnectID[0];
	req[Current++] = theApp.m_ConnectID[1];
	req[Current++] = theApp.m_ConnectID[2];
	req[Current++] = theApp.m_ConnectID[3];
	/* Name block */
	OBEXAddBlock(req, &Current, 0x01, req2, UnicodeLength(req2)*2+2);

	/* connection ID block */
/*	req[Current++] = 0xCB; // ID
//	req[Current++] = 0x00; req[Current++] = 0x00;
//	req[Current++] = 0x00; req[Current++] = 0x01;
	req[Current++] = theApp.m_ConnectID[0];
	req[Current++] = theApp.m_ConnectID[1];
	req[Current++] = theApp.m_ConnectID[2];
	req[Current++] = theApp.m_ConnectID[3];
*/	

//	return GSM_WaitFor (s, req, Current, 0x82, 12, ID_DeleteFile);
	return pWriteCommandfn ( req, Current, 0x82,24,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyDeleteFilePart);
}

//=================================================================================
//---AddFilePart------------------------

static GSM_Error OBEXGEN_ReplyAddFilePart(GSM_Protocol_Message msg)
{
	int num=0;
	switch (msg.Type) {
	case 0x90:
		smprintf(theApp.m_pDebuginfo,"Last part of file added OK\n");
		return ERR_NONE;
	case 0xA0:
		smprintf(theApp.m_pDebuginfo,"Part of file added OK\n");
	/*	if(msg.Length >4)
		{
			num=msg.Buffer[4];
			if(msg.Length >= num +5)
			{
				memcpy(theApp.m_File->ID_FullName , msg.Buffer+5 ,num); 
				memset(theApp.m_File->ID_FullName+num,0,2);
			}
		}*/
		return ERR_NONE;
	case 0xC0:
		smprintf(theApp.m_pDebuginfo,"Not understand. Probably not supported\n");
		return ERR_NOTSUPPORTED;
	}
	return ERR_UNKNOWNRESPONSE;
}
//---AddFilePart------------------------


GSM_Error OBEXGEN_AddFilePart(GSM_File *File, int *Pos,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
							  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo,
							  int (*pGetStatusfn)(int nCur,int nTotal))
{
	GSM_Error		error;
	int			j;
	int Current = 0;
	unsigned int		Pos2;
	unsigned char 		req[67000],req2[800], filename[200];
	unsigned char		Time[100];
//	unsigned char		*name,*name1;
	int name,name1;
	name=name1 = 0;

	theApp.m_File = File;
	
	File->FileSize = -1; 
	int bContiGet = true;

	if (*Pos == 0) 
	{
		if (UnicodeLength(File->ID_FullName) == 0)
		{

#ifndef xxxx
			error = OBEXGEN_Connect(OBEX_None,NULL,pWriteCommandfn,pDebuginfo);
#else
			error = OBEXGEN_Connect(OBEX_BrowsingFolders,NULL,pWriteCommandfn,pDebuginfo);
#endif	

			if (error != ERR_NONE) return error;
		} 
		else
		{

			error = OBEXGEN_Connect(OBEX_BrowsingFolders,NULL,pWriteCommandfn,pDebuginfo);

			if (error != ERR_NONE) return error;

			//if (strcmp(s->CurrentConfig->Connection,"seobex")) {
	//		if (strcmp(s->CurrentConfig->Model,"seobex"))
//			name = strstr(File->ID_FullName,"\\");
//			name1 = strstr(File->ID_FullName,"/");
			name = wstrstr(File->ID_FullName,(unsigned char *)"\\");
			name1 = wstrstr(File->ID_FullName,(unsigned char *)"/");
			if (name || name1)
			{
				smprintf(theApp.m_pDebuginfo,"Changing to root\n");
				error = OBEXGEN_ChangePath(NULL, 2,pWriteCommandfn,pDebuginfo);
				if (error != ERR_NONE) return error;

				Pos2 = 0;
				do {
					OBEXGEN_FindNextDir_UnicodePath(File->ID_FullName, &Pos2, req2);
					smprintf(theApp.m_pDebuginfo,"%s %i %i\n",DecodeUnicodeString(req2),Pos2,strlen((char*)File->ID_FullName));
					smprintf(theApp.m_pDebuginfo,"Changing path down\n");
					if (Pos2 == (int)UnicodeLength(File->ID_FullName)) break;
					error=OBEXGEN_ChangePath((char*)req2, 2,pWriteCommandfn,pDebuginfo);
					if (error != ERR_NONE) return error;
				} while (1);
			}
			else
			{
				CopyUnicodeString(req2,File->ID_FullName);
			}

			//////////////
/*			req[Current++] = 0xCB; // ID
		//	req[Current++] = 0x00; req[Current++] = 0x00;
		//	req[Current++] = 0x00; req[Current++] = 0x01;
			req[Current++] = theApp.m_ConnectID[0];
			req[Current++] = theApp.m_ConnectID[1];
			req[Current++] = theApp.m_ConnectID[2];
			req[Current++] = theApp.m_ConnectID[3];

			OBEXAddBlock(req, &Current, 0x01, req2, UnicodeLength(req2)*2+2);
			error = GSM_WaitFor (s, req, Current, 0x82, 4, ID_AddFile);
			if (error != ERR_NONE) return error;
			Current = 0;*/
		}

		/* Name block */
		//OBEXAddBlock(req, &Current, 0x01, File->Name, UnicodeLength(File->Name)*2+2);//Add name Change
		memset( filename, 0, 200 );

		/* connection ID block */
		req[Current++] = 0xCB; // ID
	//	req[Current++] = 0x00; req[Current++] = 0x00;
	//	req[Current++] = 0x00; req[Current++] = 0x01;
		req[Current++] = theApp.m_ConnectID[0];
		req[Current++] = theApp.m_ConnectID[1];
		req[Current++] = theApp.m_ConnectID[2];
		req[Current++] = theApp.m_ConnectID[3];

		OBEXAddBlock(req, &Current, 0x01, req2, UnicodeLength(req2)*2+2);
		//---------------------
		if(File->Modified.Year>0)
		{
			wsprintf((char*) Time,"%04d%02d%02dT%02d%02d%02d",File->Modified.Year,File->Modified.Month,File->Modified.Day,
				File->Modified.Hour,File->Modified.Minute,File->Modified.Second);
			OBEXAddBlock(req, (int*)&Current, 0x44, Time, 15);
		}

		/* File size block */
/*		req[Current++] = 0xC3; // ID
		req[Current++] = 0;
		req[Current++] = 0;
		req[Current++] = File->Used / 256;
		req[Current++] = File->Used % 256;*/
		req[Current++] = 0xC3; // ID
		req[Current++] = File->Used / (256*256*256);
		req[Current++] = ( File->Used % (256*256*256))/(256*256);
		req[Current++] = ( File->Used % (256*256))/(256);
		req[Current++] = File->Used % 256;


		error=pWriteCommandfn (req, Current, 0x02, 20,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyAddFilePart);
		if (error != ERR_NONE)
			return error;
		Current = 0;
		req[Current++] = 0xCB; // ID
	//	req[Current++] = 0x00; req[Current++] = 0x00;
	//	req[Current++] = 0x00; req[Current++] = 0x01;
		req[Current++] = theApp.m_ConnectID[0];
		req[Current++] = theApp.m_ConnectID[1];
		req[Current++] = theApp.m_ConnectID[2];
		req[Current++] = theApp.m_ConnectID[3];

	}
	else
	{

		/* connection ID block */
		req[Current++] = 0xCB; // ID
	//	req[Current++] = 0x00; req[Current++] = 0x00;
	//	req[Current++] = 0x00; req[Current++] = 0x01;
		req[Current++] = theApp.m_ConnectID[0];
		req[Current++] = theApp.m_ConnectID[1];
		req[Current++] = theApp.m_ConnectID[2];
		req[Current++] = theApp.m_ConnectID[3];
	}

	j = theApp.m_FrameSize - Current - 20;
	if (j > 65535) j = 65535;

	if (File->Used - *Pos < j) {
		j = File->Used - *Pos;
		/* End of file body block */
		OBEXAddBlock(req, &Current, 0x49, File->Buffer+(*Pos), j);

		smprintf(theApp.m_pDebuginfo, "Adding file part %i %i\n",*Pos,j);
		*Pos = *Pos + j;
//		error = GSM_WaitFor (s, req, Current, 0x82, 10, ID_AddFile);
		error=pWriteCommandfn (req, Current, 0x82, 40,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyAddFilePart);
		if (error != ERR_NONE)
			return error;
		if(pGetStatusfn)
		{
			pGetStatusfn(File->Used,File->Used);
		}
		return ERR_EMPTY;
	} else {
		/* File body block */
		OBEXAddBlock(req, &Current, 0x48, File->Buffer+(*Pos), j);
		smprintf(theApp.m_pDebuginfo, "Adding file part %i %i\n",*Pos,j);
		*Pos = *Pos + j;
//		error=GSM_WaitFor (s, req, Current, 0x02, 10, ID_AddFile);
		error=pWriteCommandfn (req, Current, 0x02, 40,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyAddFilePart);
        
		if (error != ERR_NONE) 
			return error;
		if(pGetStatusfn)
		{
  		 bContiGet = pGetStatusfn(*Pos,File->FileSize);
		}	
	}

   
	if(!bContiGet)
	{
		//Terminate OBEX Add File ..... // need add
		
		  Current = 0;
		  req[Current++] = 0xCB; // ID

	   	  req[Current++] = theApp.m_ConnectID[0];
		  req[Current++] = theApp.m_ConnectID[1];
		  req[Current++] = theApp.m_ConnectID[2];
		  req[Current++] = theApp.m_ConnectID[3];
		error=pWriteCommandfn ( req, Current, 0xFF, 8,true,NULL, &ObexReplymsgType,OBEXGEN_ReplyAbortFilePart);

		return ERR_CANCELED;
	}



	return error;
}

//=================================================================================
//---AddFolder------------------------

GSM_Error OBEXGEN_AddFolder(GSM_File *File,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error		error;
	unsigned char		req2[800];
	unsigned int		Pos;

//	if (!strcmp(s->CurrentConfig->Model,"seobex")) return ERR_NOTSUPPORTED;

	error = OBEXGEN_Connect(OBEX_BrowsingFolders,NULL,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

	smprintf(theApp.m_pDebuginfo,"Changing to root\n");
	error = OBEXGEN_ChangePath(NULL, 2,pWriteCommandfn,pDebuginfo);
	if (error != ERR_NONE) return error;

	Pos = 0;
	do {
		OBEXGEN_FindNextDir_UnicodePath(File->ID_FullName, &Pos, req2);
		if (Pos == (int)UnicodeLength(File->ID_FullName)) break;
		smprintf(theApp.m_pDebuginfo,"%s %i %i\n",DecodeUnicodeString(req2),Pos,strlen((char*)File->ID_FullName));
		smprintf(theApp.m_pDebuginfo,"Changing path down\n");
		error=OBEXGEN_ChangePath((char*)req2, 2,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;
	} while (1);

	smprintf(theApp.m_pDebuginfo,"Adding directory\n");
	return OBEXGEN_ChangePath((char*)req2, 0,pWriteCommandfn,pDebuginfo);
}
void GetFilePath(char* pFileFullName,char *pFilePath)
{
/*	char*   pdest = strrchr( pFileFullName, '\\' );

	wsprintf(pFilePath,"%s",pFileFullName);
	if(pdest != NULL) 
		pFilePath[strlen(pFilePath)-strlen(pdest)] = '\0';*/
	char filename[MAX_PATH];
	char drive[_MAX_PATH * 2], dir[_MAX_PATH * 2], fname[_MAX_PATH * 2], ext[_MAX_PATH * 2];
	_splitpath((const char *)pFileFullName, drive, dir, fname, ext);
	wsprintf(filename,"%s%s",fname, ext);

	wsprintf(pFilePath,"%s",pFileFullName);
	pFilePath[strlen(pFilePath)-strlen(filename)] = '\0';


}




/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
int WINAPI OBEXGEN_GetOBEXSubList(GSM_File *filelist[])
{
    int nfile =0;
	int i=0;

	if(theApp.m_FilesLocationsUsed)
	{
		nfile=theApp.m_FilesLocationsUsed -1;
		for(i=0 ;i<nfile ;i++)
		{
			filelist[i]=&theApp.m_Files[i+1];
		}

	}
	return nfile;
}
GSM_Error WINAPI OBEXGEN_GetOBEXFolderListCount(int  *nFileNo)
{
	*nFileNo = 0;

	if(theApp.m_FilesLocationsUsed)
	{
		*nFileNo=theApp.m_FilesLocationsUsed -1;
	}
	return ERR_NONE;
}
GSM_Error WINAPI OBEXGEN_SetFolderPath(GSM_File *File,GSM_Error (*pWriteCommandfn) (unsigned char *buffer,int length, unsigned char type, int WaitTime,bool ObexMode,
						  GSM_ATMultiAnwser *pATMultiAnwser,GSM_Reply_MsgType* ReplyCheckType, GSM_Error (*CallBackFun)    (GSM_Protocol_Message msg)),Debug_Info	*pDebuginfo)
{
	GSM_Error		error = ERR_NONE;
	unsigned char 		Name[800],FullName[800];
	unsigned char 		wFullName[800];
	unsigned char      gsmFileFullPathName[800];
	int Pos = 0;
	if(UnicodeLength(File->wFileFullPathName) == 0)
		CopyUnicodeString(gsmFileFullPathName,File->ID_FullName);
	else
		CopyUnicodeString(gsmFileFullPathName,File->wFileFullPathName);

	if(File->Folder == false)
		GetFilePath_Unicode(gsmFileFullPathName,wFullName);
	else
		CopyUnicodeString(wFullName,gsmFileFullPathName);
	Pos = 0;
	do {
		OBEXGEN_FindNextDir_UnicodePath(wFullName,(unsigned int *) &Pos, Name);
		if(UnicodeLength((unsigned char*)Name) == 0)
		{
			error=OBEXGEN_ChangePath(NULL, 2,pWriteCommandfn,pDebuginfo);
			theApp.m_Files[0].Folder		= true;
			theApp.m_Files[0].Level		= 1;
			theApp.m_Files[0].Name[0]		= 0;
			theApp.m_Files[0].Name[1]		= 0;
			theApp.m_Files[0].ID_FullName[0]	= 0;
			theApp.m_Files[0].ID_FullName[1]	= 0;

			theApp.m_FilesLocationsUsed 	= 1;
			theApp.m_FilesLocationsCurrent 	= 1;
			theApp.m_FileLev			= 1;
		}
		else 
			error=OBEXGEN_ChangePath((char*)Name, 2,pWriteCommandfn,pDebuginfo);
		if (error != ERR_NONE) return error;
		if (Pos == UnicodeLength(wFullName)) break;
	} while (1);
	return ERR_NONE;
}

GSM_Error WINAPI OBEXGEN_Initialise2(OnePhoneModel *pMobileInfo, Debug_Info *pDebuginfo)
{
	theApp.m_pMobileInfo=pMobileInfo;
	return OBEXGEN_Initialise(pDebuginfo);
}
void WINAPI OBEXGEN_ClearFolderListInfo()
{
	theApp.m_FilesLocationsUsed 	= 1;
	theApp.m_FilesLocationsCurrent 	= 1;
}

/////////////////////////////////////////////////////////////////////////////
//
void OBEXHEADER_ConnectionID(unsigned char* request, int* pos, char* Id)
{
	request[(*pos)++] = 0xCB; // ID
	request[(*pos)++] = Id[0];
	request[(*pos)++] = Id[1];
	request[(*pos)++] = Id[2];
	request[(*pos)++] = Id[3];
}
void OBEXHEADER_Length(unsigned char* request, int* pos, int Length)
{
	request[(*pos)++] = 0xC3; // ID
	request[(*pos)++] = Length / (256*256*256);
	request[(*pos)++] = ( Length % (256*256*256))/(256*256);
	request[(*pos)++] = ( Length % (256*256))/(256);
	request[(*pos)++] = Length % 256;
}
void OBEXHEADER_Name(unsigned char* request, int* pos, unsigned char* szName)
{
//	unsigned char name[2000];

	if(szName==NULL){
		OBEXAddBlock(request, pos, 0x01, NULL, 0);
	}else{
//		EncodeUnicode(name, (unsigned char*)szName, strlen(szName));
		OBEXAddBlock(request, pos, 0x01, szName, UnicodeLength(szName)*2+2);
	}
}
void OBEXHEADER_Type(unsigned char* request, int* pos, char* szType)
{
	if(szType==NULL) return;
	OBEXAddBlock(request, pos, 0x42, (unsigned char*)szType, strlen(szType)+1);
}
n