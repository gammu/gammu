/* (c) 2002-2004 by Marcin Wiacek */
/* Phones ID (c) partially by Walek */

#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "gsmcomon.h"
#include "gsmstate.h"
#include "misc/cfg.h"
#include "misc/coding/coding.h"
#include "device/devfunc.h"

static void GSM_RegisterConnection(GSM_StateMachine *s, unsigned int connection,
		GSM_Device_Functions *device, GSM_Protocol_Functions *protocol)
{
	if ((unsigned int)s->ConnectionType == connection) {
		s->Device.Functions	= device;	
		s->Protocol.Functions	= protocol;
	}
}

static GSM_Error GSM_RegisterAllConnections(GSM_StateMachine *s, char *connection)
{
	/* We check here is used connection string type is correct for ANY
	 * OS. If not, we return with error, that string is incorrect at all
	 */
	s->ConnectionType = 0;
	if (mystrncasecmp("mbus"	,connection,0)) s->ConnectionType = GCT_MBUS2;
	if (mystrncasecmp("fbus"	,connection,0)) s->ConnectionType = GCT_FBUS2;
	if (mystrncasecmp("fbusdlr3"	,connection,0)) s->ConnectionType = GCT_FBUS2DLR3;
	if (mystrncasecmp("fbusdku5"	,connection,0)) s->ConnectionType = GCT_FBUS2DKU5;
	if (mystrncasecmp("fbuspl2303"	,connection,0)) s->ConnectionType = GCT_FBUS2PL2303;
	if (mystrncasecmp("fbusblue"	,connection,0)) s->ConnectionType = GCT_FBUS2BLUE;
	if (mystrncasecmp("fbusirda"	,connection,0)) s->ConnectionType = GCT_FBUS2IRDA;
	if (mystrncasecmp("phonetblue"	,connection,0)) s->ConnectionType = GCT_PHONETBLUE;
	if (mystrncasecmp("mrouterblue"	,connection,0)) s->ConnectionType = GCT_MROUTERBLUE;
	if (mystrncasecmp("irdaphonet"	,connection,0)) s->ConnectionType = GCT_IRDAPHONET;
	if (mystrncasecmp("irdaat"	,connection,0)) s->ConnectionType = GCT_IRDAAT;
	if (mystrncasecmp("irdaobex"	,connection,0)) s->ConnectionType = GCT_IRDAOBEX;
	if (mystrncasecmp("blueobex"	,connection,0)) s->ConnectionType = GCT_BLUEOBEX;
	if (mystrncasecmp("bluefbus"	,connection,0)) s->ConnectionType = GCT_BLUEFBUS2;
	if (mystrncasecmp("bluephonet"	,connection,0)) s->ConnectionType = GCT_BLUEPHONET;
	if (mystrncasecmp("blueat"	,connection,0)) s->ConnectionType = GCT_BLUEAT;
	if (mystrncasecmp("bluerfobex"	,connection,0)) s->ConnectionType = GCT_BLUEOBEX;
	if (mystrncasecmp("bluerffbus"	,connection,0)) s->ConnectionType = GCT_BLUEFBUS2;
	if (mystrncasecmp("bluerfphonet",connection,0)) s->ConnectionType = GCT_BLUEPHONET;
	if (mystrncasecmp("bluerfat"	,connection,0)) s->ConnectionType = GCT_BLUEAT;

	/* These are for compatibility only */
	if (mystrncasecmp("atblue"	,connection,0)) s->ConnectionType = GCT_BLUEAT;
	if (mystrncasecmp("dlr3blue"	,connection,0)) s->ConnectionType = GCT_BLUEFBUS2;
	if (mystrncasecmp("irda"	,connection,0)) s->ConnectionType = GCT_IRDAPHONET;
	if (mystrncasecmp("dlr3"	,connection,0)) s->ConnectionType = GCT_FBUS2DLR3;
	if (mystrncasecmp("infrared"	,connection,0)) s->ConnectionType = GCT_FBUS2IRDA;

	if (mystrncasecmp("at"		,connection,2)) {
		/* Use some resonable default, when no speed defined */
		if (strlen(connection) == 2) {
			s->Speed = 19200;
		} else {
			s->Speed = FindSerialSpeed(connection+2);
		}
		if (s->Speed != 0) s->ConnectionType = GCT_AT;
	}
	if (s->ConnectionType==0) return ERR_UNKNOWNCONNECTIONTYPESTRING;

	/* We check now if user gave connection type compiled & available
	 * for used OS (if not, we return, that source not available)
	 */
	s->Device.Functions	= NULL;	
	s->Protocol.Functions	= NULL;
#ifdef GSM_ENABLE_MBUS2
	GSM_RegisterConnection(s, GCT_MBUS2, 	 &SerialDevice,   &MBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2
	GSM_RegisterConnection(s, GCT_FBUS2,	 &SerialDevice,   &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2DLR3
	GSM_RegisterConnection(s, GCT_FBUS2DLR3, &SerialDevice,   &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2DKU5
	GSM_RegisterConnection(s, GCT_FBUS2DKU5, &SerialDevice,   &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2PL2303
	GSM_RegisterConnection(s, GCT_FBUS2PL2303,&SerialDevice,   &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2BLUE
	GSM_RegisterConnection(s, GCT_FBUS2BLUE, &SerialDevice,   &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2IRDA
	GSM_RegisterConnection(s, GCT_FBUS2IRDA, &SerialDevice,   &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_PHONETBLUE
	GSM_RegisterConnection(s, GCT_PHONETBLUE,&SerialDevice,	  &PHONETProtocol);
#endif
#ifdef GSM_ENABLE_MROUTERBLUE
	GSM_RegisterConnection(s, GCT_MROUTERBLUE,&SerialDevice,  &MROUTERProtocol);
#endif
#ifdef GSM_ENABLE_IRDAPHONET
	GSM_RegisterConnection(s, GCT_IRDAPHONET,&IrdaDevice, 	  &PHONETProtocol);
#endif
#ifdef GSM_ENABLE_BLUEFBUS2
	GSM_RegisterConnection(s, GCT_BLUEFBUS2, &BlueToothDevice,&FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_BLUEPHONET
	GSM_RegisterConnection(s, GCT_BLUEPHONET,&BlueToothDevice,&PHONETProtocol);
#endif
#ifdef GSM_ENABLE_BLUEAT
	GSM_RegisterConnection(s, GCT_BLUEAT, 	 &BlueToothDevice,&ATProtocol);
#endif
#ifdef GSM_ENABLE_AT
	GSM_RegisterConnection(s, GCT_AT, 	 &SerialDevice,   &ATProtocol);
#endif
#ifdef GSM_ENABLE_IRDAAT
	GSM_RegisterConnection(s, GCT_IRDAAT, 	 &IrdaDevice,     &ATProtocol);
#endif
#ifdef GSM_ENABLE_IRDAOBEX
	GSM_RegisterConnection(s, GCT_IRDAOBEX,  &IrdaDevice,     &OBEXProtocol);
#endif
#ifdef GSM_ENABLE_BLUEOBEX
	GSM_RegisterConnection(s, GCT_BLUEOBEX,  &BlueToothDevice,&OBEXProtocol);
#endif
	if (s->Device.Functions==NULL || s->Protocol.Functions==NULL)
			return ERR_SOURCENOTAVAILABLE;
	return ERR_NONE;
}

static void GSM_RegisterModule(GSM_StateMachine *s,GSM_Phone_Functions *phone)
{
	/* Auto model */
	if (s->CurrentConfig->Model[0] == 0) {
		if (strstr(phone->models,GetModelData(NULL,s->Phone.Data.Model,NULL)->model) != NULL) {
			smprintf(s,"[Module           - \"%s\"]\n",phone->models);
			s->Phone.Functions = phone;
		}
	} else {
		if (strstr(phone->models,s->CurrentConfig->Model) != NULL) {
			smprintf(s,"[Module           - \"%s\"]\n",phone->models);
			s->Phone.Functions = phone;
		}
	}
}

GSM_Error GSM_RegisterAllPhoneModules(GSM_StateMachine *s)
{
	OnePhoneModel *model;
	
	/* Auto model */
	if (s->CurrentConfig->Model[0] == 0) {
		model = GetModelData(NULL,s->Phone.Data.Model,NULL);
#ifdef GSM_ENABLE_ALCATEL
		if (model->model[0] != 0 && IsPhoneFeatureAvailable(model, F_ALCATEL)) {
			smprintf(s,"[Module           - \"%s\"]\n",ALCATELPhone.models);
			s->Phone.Functions = &ALCATELPhone;
			return ERR_NONE;
		}
#endif
#ifdef GSM_ENABLE_ATGEN
		/* With ATgen and auto model we can work with unknown models too */
		if (s->ConnectionType==GCT_AT || s->ConnectionType==GCT_BLUEAT || s->ConnectionType==GCT_IRDAAT) {
			smprintf(s,"[Module           - \"%s\"]\n",ATGENPhone.models);
			s->Phone.Functions = &ATGENPhone;
			return ERR_NONE;
		}
#endif
		if (model->model[0] == 0) return ERR_UNKNOWNMODELSTRING;
	}
	s->Phone.Functions=NULL;
#ifdef GSM_ENABLE_ATGEN
	/* AT module can have the same models ID to "normal" Nokia modules */
	if (s->ConnectionType==GCT_AT || s->ConnectionType==GCT_BLUEAT || s->ConnectionType==GCT_IRDAAT) {
		GSM_RegisterModule(s,&ATGENPhone);
		if (s->Phone.Functions!=NULL) return ERR_NONE;
	}
#endif
#ifdef GSM_ENABLE_OBEXGEN
	GSM_RegisterModule(s,&OBEXGENPhone);
#endif
#ifdef GSM_ENABLE_MROUTERGEN
	GSM_RegisterModule(s,&MROUTERGENPhone);
#endif
#ifdef GSM_ENABLE_NOKIA3320
	GSM_RegisterModule(s,&N3320Phone);
#endif
#ifdef GSM_ENABLE_NOKIA3650
	GSM_RegisterModule(s,&N3650Phone);
#endif
#ifdef GSM_ENABLE_NOKIA650
	GSM_RegisterModule(s,&N650Phone);
#endif
#ifdef GSM_ENABLE_NOKIA6110
	GSM_RegisterModule(s,&N6110Phone);
#endif
#ifdef GSM_ENABLE_NOKIA6510
	GSM_RegisterModule(s,&N6510Phone);
#endif
#ifdef GSM_ENABLE_NOKIA7110
	GSM_RegisterModule(s,&N7110Phone);
#endif
#ifdef GSM_ENABLE_NOKIA9210
	GSM_RegisterModule(s,&N9210Phone);
#endif
#ifdef GSM_ENABLE_ALCATEL
	GSM_RegisterModule(s,&ALCATELPhone);
#endif
	if (s->Phone.Functions==NULL) return ERR_UNKNOWNMODELSTRING;
	return ERR_NONE;
}

GSM_Error GSM_InitConnection(GSM_StateMachine *s, int ReplyNum)
{
	GSM_Error	error;
	GSM_DateTime	time;
	int		i;

	for (i=0;i<s->ConfigNum;i++) {
		s->CurrentConfig		  = &s->Config[i];

		s->Speed			  = 0;
		s->ReplyNum			  = ReplyNum;
		s->Phone.Data.ModelInfo		  = GetModelData("unknown",NULL,NULL);
		s->Phone.Data.Manufacturer[0]	  = 0;
		s->Phone.Data.Model[0]		  = 0;
		s->Phone.Data.Version[0]	  = 0;
		s->Phone.Data.VerDate[0]	  = 0;
		s->Phone.Data.VerNum		  = 0;
		s->Phone.Data.StartInfoCounter	  = 0;
		s->Phone.Data.SentMsg		  = NULL;

		s->Phone.Data.HardwareCache[0]	  = 0;
		s->Phone.Data.ProductCodeCache[0] = 0;
		s->Phone.Data.EnableIncomingCall  = false;
		s->Phone.Data.EnableIncomingSMS	  = false;
		s->Phone.Data.EnableIncomingCB	  = false;
		s->Phone.Data.EnableIncomingUSSD  = false;
		s->User.UserReplyFunctions	  = NULL;
		s->User.IncomingCall		  = NULL;
		s->User.IncomingSMS		  = NULL;
		s->User.IncomingCB		  = NULL;
		s->User.IncomingUSSD		  = NULL;
		s->User.SendSMSStatus		  = NULL;
		s->LockFile			  = NULL;
		s->opened			  = false;
		s->Phone.Functions		  = NULL;

		s->di 				  = di;
		s->di.use_global 		  = s->CurrentConfig->UseGlobalDebugFile;
		GSM_SetDebugLevel(s->CurrentConfig->DebugLevel, &s->di);
		error=GSM_SetDebugFile(s->CurrentConfig->DebugFile, &s->di);
		if (error != ERR_NONE) return error;

		if (s->di.dl == DL_TEXTALL || s->di.dl == DL_TEXT || s->di.dl == DL_TEXTERROR ||
	    	    s->di.dl == DL_TEXTALLDATE || s->di.dl == DL_TEXTDATE || s->di.dl == DL_TEXTERRORDATE) {
			smprintf(s,"[Gammu            - %s built %s %s",VERSION,__TIME__,__DATE__);
			if (strlen(GetCompiler()) != 0) {
				smprintf(s," in %s",GetCompiler());
			}
			smprintf(s,"]\n[Connection       - \"%s\"]\n",s->CurrentConfig->Connection);
			smprintf(s,"[Model type       - \"%s\"]\n",s->CurrentConfig->Model);
			smprintf(s,"[Device           - \"%s\"]\n",s->CurrentConfig->Device);
			if (strlen(GetOS()) != 0) {
				smprintf(s,"[Run on           - %s]\n",GetOS());
			}
		}
		if (s->di.dl==DL_BINARY) {
			smprintf(s,"%c",((unsigned char)strlen(VERSION)));
			smprintf(s,"%s",VERSION);
		}

		error=GSM_RegisterAllConnections(s, s->CurrentConfig->Connection);
		if (error!=ERR_NONE) return error;

		/* Model auto */
		if (s->CurrentConfig->Model[0]==0) {
			if (mystrncasecmp(s->CurrentConfig->LockDevice,"yes",0)) {
				error = lock_device(s->CurrentConfig->Device, &(s->LockFile));
				if (error != ERR_NONE) return error;
			}

			/* Irda devices can set now model to some specific and
			 * we don't have to make auto detection later */
			error=s->Device.Functions->OpenDevice(s);
			if (i != s->ConfigNum - 1) {
				if (error == ERR_DEVICEOPENERROR) 	continue;
				if (error == ERR_DEVICELOCKED) 	 	continue;
				if (error == ERR_DEVICENOTEXIST)  	continue;
				if (error == ERR_DEVICEBUSY) 		continue;
				if (error == ERR_DEVICENOPERMISSION) 	continue;
				if (error == ERR_DEVICENODRIVER) 	continue;
				if (error == ERR_DEVICENOTWORK) 	continue;
			}
 			if (error!=ERR_NONE) {
 				if (s->LockFile!=NULL) unlock_device(&(s->LockFile));
 				return error;
 			}

			s->opened = true;

			error=s->Protocol.Functions->Initialise(s);
			if (error!=ERR_NONE) return error;

			/* If still auto model, try to get model by asking phone for it */
			if (s->Phone.Data.Model[0]==0) {      
				smprintf(s,"[Module           - \"auto\"]\n");
				switch (s->ConnectionType) {
#ifdef GSM_ENABLE_ATGEN
					case GCT_AT:
					case GCT_BLUEAT:
					case GCT_IRDAAT:
						s->Phone.Functions = &ATGENPhone;
						break;
#endif
#ifdef GSM_ENABLE_OBEXGEN
					case GCT_IRDAOBEX:
					case GCT_BLUEOBEX:
						s->Phone.Functions = &OBEXGENPhone;
						break;
#endif
#ifdef GSM_ENABLE_MROUTERGEN
					case GCT_MROUTERBLUE:
						s->Phone.Functions = &MROUTERGENPhone;
						break;
#endif
#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
					case GCT_MBUS2:
					case GCT_FBUS2:
					case GCT_FBUS2DLR3:
					case GCT_FBUS2DKU5:
					case GCT_FBUS2PL2303:
					case GCT_FBUS2BLUE:
					case GCT_FBUS2IRDA:
					case GCT_PHONETBLUE:
					case GCT_IRDAPHONET:
					case GCT_BLUEFBUS2:
					case GCT_BLUEPHONET:
						s->Phone.Functions = &NAUTOPhone;
						break;
#endif
					default:
						s->Phone.Functions = NULL;
				}
				if (s->Phone.Functions == NULL) return ERR_UNKNOWN;

				/* Please note, that AT module need to send first
				 * command for enabling echo
				 */
				error=s->Phone.Functions->Initialise(s);
				if (error == ERR_TIMEOUT && i != s->ConfigNum - 1) continue;
				if (error != ERR_NONE) return error;
			
				error=s->Phone.Functions->GetModel(s);
				if (error == ERR_TIMEOUT && i != s->ConfigNum - 1) continue;
				if (error != ERR_NONE) return error;
			}
		}

		/* Switching to "correct" module */
		error=GSM_RegisterAllPhoneModules(s);
		if (error!=ERR_NONE) return error;

		/* We didn't open device earlier ? Make it now */
		if (!s->opened) {
			if (mystrncasecmp(s->CurrentConfig->LockDevice,"yes",0)) {
				error = lock_device(s->CurrentConfig->Device, &(s->LockFile));
				if (error != ERR_NONE) return error;
			}

			error=s->Device.Functions->OpenDevice(s);
			if (i != s->ConfigNum - 1) {
				if (error == ERR_DEVICEOPENERROR) 	continue;
				if (error == ERR_DEVICELOCKED) 	 	continue;
				if (error == ERR_DEVICENOTEXIST)  	continue;
				if (error == ERR_DEVICEBUSY) 		continue;
				if (error == ERR_DEVICENOPERMISSION) 	continue;
				if (error == ERR_DEVICENODRIVER) 	continue;
				if (error == ERR_DEVICENOTWORK) 	continue;
			}
			if (error!=ERR_NONE) {
				if (s->LockFile!=NULL) unlock_device(&(s->LockFile));
				return error;
			}

			s->opened = true;

			error=s->Protocol.Functions->Initialise(s);
			if (error!=ERR_NONE) return error;
		}

		error=s->Phone.Functions->Initialise(s);
		if (error == ERR_TIMEOUT && i != s->ConfigNum - 1) continue;
		if (error != ERR_NONE) return error;

		if (mystrncasecmp(s->CurrentConfig->StartInfo,"yes",0)) {
			s->Phone.Functions->ShowStartInfo(s,true);
			s->Phone.Data.StartInfoCounter = 30;
		}

		if (mystrncasecmp(s->CurrentConfig->SyncTime,"yes",0)) {
			GSM_GetCurrentDateTime (&time);
			s->Phone.Functions->SetDateTime(s,&time);
		}

		/* For debug it's good to have firmware and real model version and manufacturer */
		error=s->Phone.Functions->GetManufacturer(s);
		if (error == ERR_TIMEOUT && i != s->ConfigNum - 1) continue;
		if (error != ERR_NONE) return error;
		error=s->Phone.Functions->GetModel(s);
		if (error != ERR_NONE) return error;
		error=s->Phone.Functions->GetFirmware(s);
		if (error != ERR_NONE) return error;
		return ERR_NONE;
	}
	return ERR_UNKNOWN;
}

int GSM_ReadDevice (GSM_StateMachine *s, bool wait)
{
	unsigned char	buff[255];
	int		res = 0, count;

	unsigned int	i;
	GSM_DateTime	Date;

	GSM_GetCurrentDateTime (&Date);
	i=Date.Second;
	while (i==Date.Second) {
		res = s->Device.Functions->ReadDevice(s, buff, 255);
		if (!wait) break;
		if (res > 0) break;
		my_sleep(5);
		GSM_GetCurrentDateTime(&Date);
	}

	for (count = 0; count < res; count++)
		s->Protocol.Functions->StateMachine(s,buff[count]);

	return res;
}

GSM_Error GSM_TerminateConnection(GSM_StateMachine *s)
{
	GSM_Error error;
	
	if (!s->opened) return ERR_UNKNOWN;

	smprintf(s,"[Closing]\n");

	if (mystrncasecmp(s->CurrentConfig->StartInfo,"yes",0)) {
		if (s->Phone.Data.StartInfoCounter > 0) s->Phone.Functions->ShowStartInfo(s,false);
	}

	if (s->Phone.Functions != NULL) {
		error=s->Phone.Functions->Terminate(s);
		if (error!=ERR_NONE) return error;
	}

	error=s->Protocol.Functions->Terminate(s);	
	if (error!=ERR_NONE) return error;

	error = s->Device.Functions->CloseDevice(s);
	if (error!=ERR_NONE) return error;

	s->Phone.Data.ModelInfo		  = NULL;
	s->Phone.Data.Manufacturer[0]	  = 0;
	s->Phone.Data.Model[0]		  = 0;
	s->Phone.Data.Version[0]	  = 0;
	s->Phone.Data.VerDate[0]	  = 0;
	s->Phone.Data.VerNum		  = 0;

	if (s->LockFile!=NULL) unlock_device(&(s->LockFile));

	if (!s->di.use_global && s->di.dl!=0 && fileno(s->di.df) != 1 && fileno(s->di.df) != 2) fclose(s->di.df);

	s->opened = false;

	return ERR_NONE;
}

GSM_Error GSM_WaitForOnce(GSM_StateMachine *s, unsigned char *buffer,
			  int length, unsigned char type, int time)
{
	GSM_Phone_Data			*Phone = &s->Phone.Data;
	GSM_Protocol_Message 		sentmsg;
	int				i;

	i=0;
	do {
		if (length != 0) {
			sentmsg.Length 	= length;
			sentmsg.Type	= type;
			sentmsg.Buffer 	= (unsigned char *)malloc(length);
			memcpy(sentmsg.Buffer,buffer,length);
			Phone->SentMsg  = &sentmsg;
		}

		/* Some data received. Reset timer */
		if (GSM_ReadDevice(s,true)!=0) i=0;

		if (length != 0) {
			free (sentmsg.Buffer);
			Phone->SentMsg  = NULL;
		}

		/* Request completed */
		if (Phone->RequestID==ID_None) return Phone->DispatchError;

		i++;
	} while (i<time);

	return ERR_TIMEOUT;
}

GSM_Error GSM_WaitFor (GSM_StateMachine *s, unsigned char *buffer,
		       int length, unsigned char type, int time,
		       GSM_Phone_RequestID request)
{
	GSM_Phone_Data		*Phone = &s->Phone.Data;
	GSM_Error		error;
	int			reply;

	if (mystrncasecmp(s->CurrentConfig->StartInfo,"yes",0)) {
		if (Phone->StartInfoCounter > 0) {
			Phone->StartInfoCounter--;
			if (Phone->StartInfoCounter == 0) s->Phone.Functions->ShowStartInfo(s,false);
		}
	}

	Phone->RequestID	= request;
	Phone->DispatchError	= ERR_TIMEOUT;

	for (reply=0;reply<s->ReplyNum;reply++) {
		if (reply!=0) {
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl == DL_TEXTERROR ||
			    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl == DL_TEXTERRORDATE)
			{
				smprintf(s, "[Retrying %i type 0x%02X]\n", reply, type);
			}
		}
		error = s->Protocol.Functions->WriteMessage(s, buffer, length, type);
		if (error!=ERR_NONE) return error;

		error = GSM_WaitForOnce(s, buffer, length, type, time);
		if (error != ERR_TIMEOUT) return error;
        }

	return Phone->DispatchError;
}

static GSM_Error CheckReplyFunctions(GSM_StateMachine *s, GSM_Reply_Function *Reply, int *reply)
{
	GSM_Phone_Data			*Data	  = &s->Phone.Data;
	GSM_Protocol_Message		*msg	  = s->Phone.Data.RequestMsg;
	bool				execute;
	bool				available = false;
	int				i	  = 0;
//	int 				j;

	while (Reply[i].requestID!=ID_None) {
		execute=false;
		/* Binary frames like in Nokia */
		if (strlen(Reply[i].msgtype) < 2) {
			if (Reply[i].msgtype[0]==msg->Type) {
				if (Reply[i].subtypechar!=0) {
					if (Reply[i].subtypechar<=msg->Length) {
						if (msg->Buffer[Reply[i].subtypechar]==Reply[i].subtype)
							execute=true;
					}			
				} else execute=true;
			}
		} else {
//			printf("msg length %i %i\n",strlen(Reply[i].msgtype),msg->Length);
			if ((int)strlen(Reply[i].msgtype)<msg->Length) {
//				printf("Comparing \"%s\" and \"",Reply[i].msgtype);
//				for (j=0;j<strlen(Reply[i].msgtype);j++) {
//					if (msg->Buffer[j]!=13 && msg->Buffer[j]!=10) {
//						printf("%c",msg->Buffer[j]);
//					}
//				}
//				printf("\"\n");
				if (strncmp(Reply[i].msgtype,msg->Buffer,strlen(Reply[i].msgtype))==0) {
					execute=true;
				}
			}
		}

		if (execute) {
			*reply=i;
			if (Reply[i].requestID == ID_IncomingFrame ||
			    Reply[i].requestID == Data->RequestID ||
			    Data->RequestID	== ID_EachFrame) {
				return ERR_NONE;
			}
			available=true;
		}
		i++;
	}

	if (available) {
		return ERR_FRAMENOTREQUESTED;
	} else {
		return ERR_UNKNOWNFRAME;
	}
}

GSM_Error GSM_DispatchMessage(GSM_StateMachine *s)
{
	GSM_Error		error	= ERR_UNKNOWNFRAME;
	GSM_Protocol_Message	*msg 	= s->Phone.Data.RequestMsg;
	GSM_Phone_Data 		*Phone	= &s->Phone.Data;
	bool			disp    = false;
	GSM_Reply_Function	*Reply;
	int			reply, i;

	if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
	    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
		smprintf(s, "RECEIVED frame ");
		smprintf(s, "type 0x%02X/length 0x%02X/%i", msg->Type, msg->Length, msg->Length);
		DumpMessage(s->di.use_global ? di.df : s->di.df, s->di.dl, msg->Buffer, msg->Length);
		if (msg->Length == 0) smprintf(s, "\n");
		fflush(s->di.df);
	}
	if (s->di.dl==DL_BINARY) {
		smprintf(s,"%c",0x02);	/* Receiving */
		smprintf(s,"%c",msg->Type);
		smprintf(s,"%c",msg->Length/256);
		smprintf(s,"%c",msg->Length%256);
		for (i=0;i<msg->Length;i++) {
			smprintf(s,"%c",msg->Buffer[i]);
		}
	}

	Reply=s->User.UserReplyFunctions;
	if (Reply!=NULL) error=CheckReplyFunctions(s,Reply,&reply);

	if (error==ERR_UNKNOWNFRAME) {
		Reply=s->Phone.Functions->ReplyFunctions;
		error=CheckReplyFunctions(s,Reply,&reply);
	}

	if (error==ERR_NONE) {
		error=Reply[reply].Function(*msg, s);
		if (Reply[reply].requestID==Phone->RequestID) {
			if (error == ERR_NEEDANOTHERANSWER) {
				error = ERR_NONE;
			} else {
				Phone->RequestID=ID_None;
			}
		}
	}

	if (strcmp(s->Phone.Functions->models,"NAUTO")) {
		if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
		    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE) {
			disp = true;
			switch (error) {
			case ERR_UNKNOWNRESPONSE:
				smprintf(s, "\nUNKNOWN response");
				break;
			case ERR_UNKNOWNFRAME:
				smprintf(s, "\nUNKNOWN frame");
				break;
			case ERR_FRAMENOTREQUESTED:
				smprintf(s, "\nFrame not request now");
				break;
			default:
				disp = false;
			}
		}

		if (error == ERR_UNKNOWNFRAME || error == ERR_FRAMENOTREQUESTED) {
			error = ERR_TIMEOUT;
		}
	}

	if (disp) {
		smprintf(s,". If you can, PLEASE report it (see readme.txt). THANK YOU\n");
		if (Phone->SentMsg != NULL) {
			smprintf(s,"LAST SENT frame ");
			smprintf(s, "type 0x%02X/length %i", Phone->SentMsg->Type, Phone->SentMsg->Length);
			DumpMessage(s->di.use_global ? di.df : s->di.df, s->di.dl, Phone->SentMsg->Buffer, Phone->SentMsg->Length);
		}
		smprintf(s, "RECEIVED frame ");
		smprintf(s, "type 0x%02X/length 0x%02X/%i", msg->Type, msg->Length, msg->Length);
		DumpMessage(s->di.use_global ? di.df : s->di.df, s->di.dl, msg->Buffer, msg->Length);
		smprintf(s, "\n");
	}

	return error;
}

INI_Section *GSM_FindGammuRC(void)
{
	INI_Section	*ini_file;
        char		*HomeDrive,*HomePath,*FileName=malloc(1);
	int		FileNameUsed=1;

	FileName[0] = 0;
#if defined(WIN32) || defined(DJGPP)
        HomeDrive = getenv("HOMEDRIVE");
	if (HomeDrive) {
		FileName 	=  realloc(FileName,FileNameUsed+strlen(HomeDrive)+1);
		FileName 	=  strcat(FileName, HomeDrive);
		FileNameUsed	+= strlen(HomeDrive)+1;
	}
        HomePath = getenv("HOMEPATH");
        if (HomePath) {
		FileName 	=  realloc(FileName,FileNameUsed+strlen(HomePath)+1);
		FileName 	=  strcat(FileName, HomePath);
		FileNameUsed	+= strlen(HomePath)+1;
	}
	FileName = realloc(FileName,FileNameUsed+8+1);
        strcat(FileName, "\\gammurc");
#else
	HomeDrive = NULL;
        HomePath  = getenv("HOME");
        if (HomePath) {
		FileName 	=  realloc(FileName,FileNameUsed+strlen(HomePath)+1);
		FileName 	=  strcat(FileName, HomePath);
		FileNameUsed	+= strlen(HomePath)+1;
	}
	FileName = realloc(FileName,FileNameUsed+9+1);
        strcat(FileName, "/.gammurc");
#endif
//	dbgprintf("\"%s\"\n",FileName);

	ini_file = INI_ReadFile(FileName, false);
	free(FileName);
        if (ini_file == NULL) {
#if defined(WIN32) || defined(DJGPP)
		ini_file = INI_ReadFile("gammurc", false);
                if (ini_file == NULL) return NULL;
#else
		ini_file = INI_ReadFile("/etc/gammurc", false);
                if (ini_file == NULL) return NULL;
#endif
        }

	return ini_file;
}

bool GSM_ReadConfig(INI_Section *cfg_info, GSM_Config *cfg, int num)
{
	INI_Section 	*h;
	unsigned char 	section[50];
	bool		found = false;

#if defined(WIN32) || defined(DJGPP)
        char *DefaultPort		= "com2:";
#else
        char *DefaultPort		= "/dev/ttyS1";
#endif
        char *DefaultModel		= "";
        char *DefaultConnection		= "fbus";
	char *DefaultSynchronizeTime	= "no";
	char *DefaultDebugFile		= "";
	char *DefaultDebugLevel		= "";
	char *DefaultLockDevice		= "no";
	char *DefaultStartInfo		= "no";
	char *Temp;

	/* By default all debug output will go to one filedescriptor */
	bool DefaultUseGlobalDebugFile 	= true;

        cfg->Device		 = DefaultPort;
        cfg->Connection	 	 = DefaultConnection;
        cfg->SyncTime 	 	 = DefaultSynchronizeTime;
	cfg->DebugFile	 	 = DefaultDebugFile;
        strcpy(cfg->Model,DefaultModel);
	strcpy(cfg->DebugLevel,DefaultDebugLevel);
	cfg->LockDevice	 	 = DefaultLockDevice;
	cfg->StartInfo		 = DefaultStartInfo;
	cfg->DefaultDevice	 = true;
	cfg->DefaultModel	 = true;
	cfg->DefaultConnection	 = true;
	cfg->DefaultSyncTime	 = true;
	cfg->DefaultDebugFile	 = true;
	cfg->DefaultDebugLevel	 = true;
	cfg->DefaultLockDevice	 = true;
	cfg->DefaultStartInfo	 = true;

	cfg->UseGlobalDebugFile	 = DefaultUseGlobalDebugFile;

	if (cfg_info==NULL) return false;

	if (num == 0) {
		sprintf(section,"gammu");
	} else {
		sprintf(section,"gammu%i",num);
	}
        for (h = cfg_info; h != NULL; h = h->Next) {
                if (mystrncasecmp(section, h->SectionName, strlen(section))) {
			found = true;
			break;
		}
        }
	if (!found) return false;

	cfg->Device 	 = INI_GetValue(cfg_info, section, "port", 		false);
	if (!cfg->Device) {
		free(cfg->Device);
		cfg->Device		 	 = strdup(DefaultPort);
	} else {
		cfg->DefaultDevice 		 = false;
	}
	cfg->Connection  = INI_GetValue(cfg_info, section, "connection", 	false);
	if (!cfg->Connection) {
		free(cfg->Connection);
		cfg->Connection	 		 = strdup(DefaultConnection);
	} else {
		cfg->DefaultConnection		 = false;
	}
	cfg->SyncTime 	 = INI_GetValue(cfg_info, section, "synchronizetime",	false);
	if (!cfg->SyncTime) {
		free(cfg->SyncTime);
		cfg->SyncTime		 	 = strdup(DefaultSynchronizeTime);
	} else {
		cfg->DefaultSyncTime		 = false;
	}
	cfg->DebugFile   = INI_GetValue(cfg_info, section, "logfile", 		false);
	if (!cfg->DebugFile) {
		free(cfg->DebugFile);
		cfg->DebugFile		 	 = strdup(DefaultDebugFile);
	} else {
		cfg->DefaultDebugFile 		 = false;
	}
	cfg->LockDevice  = INI_GetValue(cfg_info, section, "use_locking", 	false);
	if (!cfg->LockDevice) {
		free(cfg->LockDevice);
		cfg->LockDevice	 		 = strdup(DefaultLockDevice);
	} else {
		cfg->DefaultLockDevice		 = false;
	}
	Temp		 = INI_GetValue(cfg_info, section, "model", 		false);
	if (!Temp) {
		strcpy(cfg->Model,DefaultModel);
	} else {
		cfg->DefaultModel 		 = false;
		strcpy(cfg->Model,Temp);
	}
	Temp		 = INI_GetValue(cfg_info, section, "logformat", 	false);
	if (!Temp) {
		strcpy(cfg->DebugLevel,DefaultDebugLevel);
	} else {
		cfg->DefaultDebugLevel 		 = false;
		strcpy(cfg->DebugLevel,Temp);
	}
	cfg->StartInfo   = INI_GetValue(cfg_info, section, "startinfo", 	false);
	if (!cfg->StartInfo) {
		free(cfg->StartInfo);
		cfg->StartInfo	 		 = strdup(DefaultStartInfo);
	} else {
		cfg->DefaultStartInfo 		 = false;
	}
	return true;
}

static OnePhoneModel allmodels[] = {
#ifdef GSM_ENABLE_NOKIA650
	{"0650" ,"THF-12","",           {0}},
#endif
#ifdef GSM_ENABLE_NOKIA6510
	{"1100" ,"RH-18" ,"",		{0}},
	{"1100a","RH-38" ,"",		{0}},
	{"1100b","RH-36" ,"",		{0}},
#endif
#ifdef GSM_ENABLE_NOKIA6110
	{"2100" ,"NAM-2" ,"",           {F_NOWAP,F_NOCALLER,F_RING_SM,F_CAL33,F_POWER_BATT,F_PROFILES33,F_NOCALLINFO,F_NODTMF,0}},//quess
#endif
#ifdef GSM_ENABLE_NOKIA6510
	{"3100" ,"RH-19" ,"",           {F_PBKTONEGAL,F_PBKSMSLIST,0}},//fixme
	{"3100b","RH-50" ,"",           {F_PBKTONEGAL,F_PBKSMSLIST,0}},//fixme
 	{"3105" ,"RH-48" ,"Nokia 3105",	{F_PBKTONEGAL,F_PBKSMSLIST,F_VOICETAGS,0}},//fixme
	{"3108", "RH-6",  "Nokia 3108",	{0}}, //does it have irda ?
	{"3200", "RH-30" ,"Nokia 3200",	{F_PBKTONEGAL,0}},//fixme
	{"3200a","RH-31" ,"Nokia 3200",	{F_PBKTONEGAL,0}},//fixme
#endif
#ifdef GSM_ENABLE_NOKIA6110
	{"3210" ,"NSE-8" ,"",           {F_NOWAP,F_NOCALLER,F_NOCALENDAR,F_NOPBKUNICODE,F_POWER_BATT,F_PROFILES51,F_NOPICTUREUNI,F_NOCALLINFO,F_NODTMF,0}},
	{"3210" ,"NSE-9" ,"",           {F_NOWAP,F_NOCALLER,F_NOCALENDAR,F_NOPBKUNICODE,F_POWER_BATT,F_PROFILES51,F_NOPICTUREUNI,F_NOCALLINFO,F_NODTMF,0}},
#endif
#ifdef GSM_ENABLE_NOKIA6510
	{"3300" ,"NEM-1" ,"Nokia 3300", {F_PBKTONEGAL,0}},//fixme
	{"3300" ,"NEM-2" ,"Nokia 3300", {F_PBKTONEGAL,0}},//fixme
#endif
#ifdef GSM_ENABLE_NOKIA6110
	{"3310" ,"NHM-5" ,"",           {F_NOWAP,F_NOCALLER,F_RING_SM,F_CAL33,F_POWER_BATT,F_PROFILES33,F_NOCALLINFO,F_NODTMF,0}},
#endif
#ifdef GSM_ENABLE_NOKIA3320
	{"3320" ,"NPC-1" ,"Nokia 3320", {F_CAL62,F_DAYMONTH,0}},//fixme
#endif
#ifdef GSM_ENABLE_NOKIA6110
	{"3330" ,"NHM-6" ,"",           {F_NOCALLER,F_RING_SM,F_CAL33,F_PROFILES33,F_NOPICTUREUNI,F_NOCALLINFO,F_NODTMF,0}},
	{"3390" ,"NPB-1" ,"",           {F_NOWAP,F_NOCALLER,F_RING_SM,F_CAL33,F_PROFILES33,F_NOPICTUREUNI,F_NOCALLINFO,F_NODTMF,0}},
	{"3410" ,"NHM-2" ,"",           {F_RING_SM,F_CAL33,F_PROFILES33,F_NOCALLINFO,F_NODTMF,0}},
#endif
#ifdef GSM_ENABLE_NOKIA6510
	{"3510" ,"NHM-8" ,"",           {F_CAL35,F_PBK35,F_NOGPRSPOINT,F_VOICETAGS,0}},
	{"3510i","RH-9"  ,"",           {F_CAL35,F_PBK35,F_NOGPRSPOINT,F_VOICETAGS,0}},
	{"3530" ,"RH-9"  ,"",           {F_CAL35,F_PBK35,F_NOGPRSPOINT,F_VOICETAGS,0}},
	{"3589i","RH-44" ,"",		{F_VOICETAGS,0}},
	{"3590" ,"NPM-8" ,"",		{0}},//irda?
	{"3595" ,"NPM-10" ,"",		{0}},//irda?
#endif
#ifdef GSM_ENABLE_NOKIA6110
	{"3610" ,"NAM-1" ,"",           {F_NOCALLER,F_RING_SM,F_CAL33,F_POWER_BATT,F_PROFILES33,F_NOCALLINFO,F_NODTMF,0}},//quess
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA3650)
	{"3650" ,"NHL-8" ,"Nokia 3650", {0}},
	{"NGAGE","NEM-4" ,"",           {F_RADIO,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6510)
	{"5100" ,"NPM-6" ,"Nokia 5100", {F_PBKTONEGAL,F_TODO66,F_RADIO,0}},
	{"5100" ,"NPM-6U","Nokia 5100", {F_PBKTONEGAL,F_TODO66,F_RADIO,0}},
	{"5100" ,"NPM-6X","Nokia 5100", {F_PBKTONEGAL,F_TODO66,F_RADIO,0}},
	{"5140" ,"NPL-4" ,"Nokia 5140", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKUSER,F_WAPMMSPROXY,0}},
	{"5140" ,"NPL-5" ,"Nokia 5140", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKUSER,F_WAPMMSPROXY,0}},
#endif
#ifdef GSM_ENABLE_NOKIA6110
	{"5110" ,"NSE-1" ,"",           {F_NOWAP,F_NOCALLER,F_NORING,F_NOPICTURE,F_NOSTARTUP,F_NOCALENDAR,F_NOPBKUNICODE,F_PROFILES51,F_MAGICBYTES,F_DISPSTATUS,0}},
	{"5110i","NSE-2" ,"",           {F_NOWAP,F_NOCALLER,F_NORING,F_NOPICTURE,F_NOSTARTUP,F_NOCALENDAR,F_NOPBKUNICODE,F_PROFILES51,F_MAGICBYTES,F_DISPSTATUS,0}},
	{"5130" ,"NSK-1" ,"",           {F_NOWAP,F_NOCALLER,F_NORING,F_NOPICTURE,F_NOSTARTUP,F_NOCALENDAR,F_NOPBKUNICODE,F_PROFILES51,F_MAGICBYTES,F_DISPSTATUS,0}},
	{"5190" ,"NSB-1" ,"",           {F_NOWAP,F_NOCALLER,F_NORING,F_NOPICTURE,F_NOSTARTUP,F_NOCALENDAR,F_NOPBKUNICODE,F_PROFILES51,F_MAGICBYTES,F_DISPSTATUS,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6110)
	{"5210" ,"NSM-5" ,"Nokia 5210", {F_CAL52,F_NOSTARTANI,F_NOPICTUREUNI,F_NODTMF,0}},
#endif
#ifdef GSM_ENABLE_NOKIA6110
	{"5510" ,"NPM-5" ,"",           {F_NOCALLER,F_PROFILES33,F_NOPICTUREUNI,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6510)
	{"6100" ,"NPL-2" ,"Nokia 6100", {F_PBKTONEGAL,F_TODO66,0}},
#endif
#ifdef GSM_ENABLE_NOKIA6110
	{"6110" ,"NSE-3" ,"",           {F_NOWAP,F_NOPICTURE,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DISPSTATUS,0}},
	{"6130" ,"NSK-3" ,"",           {F_NOWAP,F_NOPICTURE,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DISPSTATUS,0}},
	{"6150" ,"NSM-1" ,"",           {F_NOWAP,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DISPSTATUS,F_NOPICTUREUNI,0}},
	{"6190" ,"NSB-3" ,"",           {F_NOWAP,F_NOPICTURE,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DISPSTATUS,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6510)
	{"6200" ,"NPL-3" ,"Nokia 6200", {F_PBKTONEGAL,0}},
	{"6220" ,"RH-20" ,"Nokia 6220", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA7110)
	{"6210" ,"NPE-3" ,"Nokia 6210", {F_VOICETAGS,F_CAL62,0}},
	{"6250" ,"NHM-3" ,"Nokia 6250", {F_VOICETAGS,F_CAL62,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6510)
	{"6230" ,"RH-12" ,"Nokia 6230", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,0}},
	{"6310" ,"NPE-4" ,"Nokia 6310", {F_TODO63,F_CAL65,F_NOMIDI,F_NOMMS,F_VOICETAGS,0}},
	{"6310i","NPL-1" ,"Nokia 6310i",{F_TODO63,F_CAL65,F_NOMIDI,F_BLUETOOTH,F_NOMMS,F_VOICETAGS,0}},
	{"6385" ,"NHP-2AX","Nokia 6385",{F_TODO63,F_CAL65,F_NOMIDI,F_NOMMS,F_VOICETAGS,0}},
	{"6510" ,"NPM-9" ,"Nokia 6510", {F_TODO63,F_CAL65,F_NOMIDI,F_RADIO,F_NOFILESYSTEM,F_NOMMS,F_VOICETAGS,0}},
	{"6610" ,"NHL-4U","Nokia 6610", {F_PBKTONEGAL,F_TODO66,F_RADIO,0}},
	{"6610i","RM-37" ,"Nokia 6610i",{F_PBKTONEGAL,F_TODO66,F_RADIO,0}},
	{"6800" ,"NSB-9" ,"Nokia 6800", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,0}},
	{"6800" ,"NHL-6" ,"Nokia 6800", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,0}},
	{"6810" ,"RM-2"  ,"Nokia 6810", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,F_NOTES,0}},//quess
	{"6820" ,"NHL-9" ,"Nokia 6820", {F_PBKTONEGAL,F_TODO66,F_PBKSMSLIST,F_NOTES,0}},//quess
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA7110)
	{"7110" ,"NSE-5" ,"Nokia 7110", {F_CAL62,0}},
	{"7190" ,"NSB-5" ,"Nokia 7190", {F_CAL62,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6510)
	{"7200" ,"RH-23" ,"Nokia 7200", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,F_PBKUSER,0}},//quess
	{"7210" ,"NHL-4" ,"Nokia 7210", {F_PBKTONEGAL,F_TODO66,F_RADIO,0}},
	{"7250" ,"NHL-4J","Nokia 7250", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKIMG,0}},
	{"7250i","NHL-4JX","Nokia 7250i",{F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKIMG,0}},
	{"7600", "NMM-3", "Nokia 7600", {F_TODO66,0}},
#endif
#if defined(GSM_ENABLE_ATGEN)
	{"7650" ,"NHL-2" ,"Nokia 7650", {0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6110)
	{"8210" ,"NSM-3" ,"Nokia 8210", {F_NOWAP,F_NOSTARTANI,F_NOPBKUNICODE,F_NOPICTUREUNI,0}},
	{"8250" ,"NSM-3D","Nokia 8250", {F_NOWAP,F_NOSTARTANI,F_CAL82,F_NOPICTUREUNI,0}},
	{"8290" ,"NSB-7" ,"Nokia 8290", {F_NOWAP,F_NOSTARTANI,F_NOPBKUNICODE,F_NOPICTUREUNI,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6510)
	{"8310" ,"NHM-7" ,"Nokia 8310", {F_CAL62,F_NOMIDI,F_RADIO,F_NOFILESYSTEM,F_NOMMS,F_VOICETAGS,0}},
	{"8390" ,"NSB-8" ,"Nokia 8390", {F_CAL62,F_NOMIDI,F_RADIO,F_NOFILESYSTEM,F_NOMMS,F_VOICETAGS,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6110)
	{"8850" ,"NSM-2" ,"Nokia 8850", {0}},
	{"8855" ,"NSM-4" ,"Nokia 8855", {0}},
	{"8890" ,"NSB-6" ,"Nokia 8890", {0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6510)
	{"8910" ,"NHM-4" ,"Nokia 8910", {F_CAL62,F_NOMIDI,F_NOFILESYSTEM,F_NOMMS,0}},
	{"8910i","NHM-4" ,"Nokia 8910i",{F_CAL62,F_NOMIDI,F_NOFILESYSTEM,F_NOMMS,0}},
#endif
#ifdef GSM_ENABLE_NOKIA9210
	{"9210" ,"RAE-3" ,"",           {0}},
	{"9210i","RAE-5" ,"",           {0}},
#endif
#ifdef GSM_ENABLE_ATGEN
	{"at"   ,	  "at",		  "",				   {0}},
	/* Siemens */
	{"M20"  ,	  "M20",	  "",				   {F_M20SMS,F_SLOWWRITE,0}},
	{"MC35" ,	  "MC35",	  "",				   {0}},
	{"TC35" ,	  "TC35",	  "",				   {0}},
	{"S25",		  "S25",  	  "SIEMENS S25",		   {0}},
	{"C35i" ,	  "C35i",	  "",				   {0}},
	{"S35i" ,	  "S35i",	  "",				   {0}},
	{"M35i" ,	  "M35i",	  "",				   {0}},
	{"S40" ,	  "Siemens S40",  "",			   	   {0}},	
	{"C45" ,	  "C45",	  "",				   {0}},
	{"S45" ,	  "S45",	  "",				   {0}},
	{"ME45" ,	  "ME45",	  "",				   {0}},
	{"SL45" ,	  "SL45",	  "",				   {0}},
	{"SL45i" ,	  "SL45i",	  "",				   {0}},
	{"M50" ,	  "M50",	  "",				   {0}},
	{"S45"	,	  "6618" ,	  "",				   {0}},
	{"ME45" ,	  "3618" ,	  "",				   {0}},
	{"S55" ,	  "S55" ,	  "",				   {0}},
	/* Samsung */
	{"S100" ,	  "SGH-S100" ,    "",				   {0}},
	{"S200" ,	  "SGH-S200" ,    "",				   {0}},
	{"S300" ,	  "SGH-S300" ,    "",				   {0}},
	{"S500" ,	  "SGH-S500" ,    "",				   {0}},
	{"V200" ,	  "SGH-V200" ,    "",				   {0}},
	{"T100" ,	  "SGH-T100" ,    "",				   {0}},
	{"E700" ,	  "SGH-E700" ,    "",				   {0}},
	/* Ericsson/Sony Ericsson */
	{"T28s",	"1101101-BVT28s","",				   {0}},
	{"R320s" ,	"1101201-BV R320s","",				   {0}},
	{"R380s",	"7100101-BVR380s" ,"",				   {0}},
	{"R520m",	"1130101-BVR520m" ,"",				   {0}},
	{"T39m",	"1130102-BVT39m" ,"",				   {0}},
	{"T65",		"1101901-BVT65" , "",				   {0}},
	{"T68",		"1130201-BVT68" , "",				   {0}},
	{"T68i",	"1130202-BVT68" , "",				   {0}},
	{"R600",	"102001-BVR600" , "",				   {0}},
	{"T200",	"1130501-BVT200" ,"",				   {0}},
	{"T300",	"1130601-BVT300" ,"T300",			   {0}},
	{"T310",	"1130602-BVT310" ,"",			   	   {0}},	
	{"P800",	"7130501-BVP800" ,"",				   {0}},
	/* Other */
	{"iPAQ" ,	  "iPAQ"  ,	  "" ,				   {0}},
	{"A2D"  ,	  "A2D"  ,	  "" ,				   {0}},
	{"9210" ,	  "RAE-3",	  "Nokia Communicator GSM900/1800",{0}},
	{"myV-65",	"myV-65 GPRS",	  "",				   {F_SMSME900,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_ALCATEL)
	{"BE5", 	  "ONE TOUCH 500","",				   {F_ALCATEL,F_SMSONLYSENT,F_BROKENCPBS,0}},
	{"BH4",		  "ONE TOUCH 535","ALCATEL OT535",		   {F_ALCATEL,F_SMSONLYSENT,0}},
	{"BF5", 	  "ONE TOUCH 715","ALCATEL OT715",		   {F_ALCATEL,F_SMSONLYSENT,F_BROKENCPBS,0}},
#endif
	{"unknown",	  ""      ,"",           {0}}
};

OnePhoneModel *GetModelData(char *model, char *number, char *irdamodel)
{
	int i = 0;

	while (strcmp(allmodels[i].number,"") != 0) {
		if (model !=NULL) {
			if (strcmp (model, allmodels[i].model) == 0) {
				return (&allmodels[i]);
			}
		}
		if (number !=NULL) {
			if (strcmp (number, allmodels[i].number) == 0) {
				return (&allmodels[i]);
			}
		}
		if (irdamodel !=NULL) {
			if (strcmp (irdamodel, allmodels[i].irdamodel) == 0) {
				return (&allmodels[i]);
			}
		}
		i++;
	}
	return (&allmodels[i]);
}

bool IsPhoneFeatureAvailable(OnePhoneModel *model, Feature feature)
{
	int	i	= 0;
	bool	retval  = false;

	while (model->features[i] != 0) {
		if (model->features[i] == feature) {
			retval = true;
			break;
		}
		i++;
	}
	return retval;
}

void GSM_DumpMessageLevel2(GSM_StateMachine *s, unsigned char *message, int messagesize, int type)
{
	if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
	    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
		smprintf(s,"SENDING frame ");
		smprintf(s,"type 0x%02X/length 0x%02X/%i", type, messagesize, messagesize);
		DumpMessage(s->di.use_global ? di.df : s->di.df, s->di.dl, message, messagesize);
		if (messagesize == 0) smprintf(s,"\n");
		if (s->di.df) fflush(s->di.df);
	}
}

void GSM_DumpMessageLevel3(GSM_StateMachine *s, unsigned char *message, int messagesize, int type)
{
	int i;

	if (s->di.dl==DL_BINARY) {
		smprintf(s,"%c",0x01);	/* Sending */
		smprintf(s,"%c",type);
		smprintf(s,"%c",messagesize/256);
		smprintf(s,"%c",messagesize%256);
		for (i=0;i<messagesize;i++) smprintf(s,"%c",message[i]);
	}
}

#ifdef __GNUC__
__attribute__((format(printf, 2, 3)))
#endif
int smprintf(GSM_StateMachine *s, const char *format, ...)
{
	va_list		argp;
	int 		result=0;
	unsigned char	buffer[2000];
	Debug_Level	dl;
	FILE		*df;

	va_start(argp, format);
	if (s == NULL) {
		dl = di.dl;
		df = di.df;
	} else {
		dl = s->di.dl;
		if (s->di.use_global) {
			df = di.df;
		} else {
			df = s->di.df;
		}
	}

	if (dl != 0) {
		result = vsprintf(buffer, format, argp);
		result = smfprintf(df, dl, "%s", buffer);
	}

	va_end(argp);
	return result;
}

void GSM_OSErrorInfo(GSM_StateMachine *s, char *description)
{
#ifdef WIN32
	int 		i;
	unsigned char 	*lpMsgBuf;

	/* We don't use errno in win32 - GetLastError gives better info */
	if (GetLastError()!=-1) {
		if (s->di.dl == DL_TEXTERROR || s->di.dl == DL_TEXT || s->di.dl == DL_TEXTALL ||
		    s->di.dl == DL_TEXTERRORDATE || s->di.dl == DL_TEXTDATE || s->di.dl == DL_TEXTALLDATE) {
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);
			for (i=0;i<(int)strlen(lpMsgBuf);i++) {
				if (lpMsgBuf[i] == 13 || lpMsgBuf[i] == 10) {
					lpMsgBuf[i] = ' ';
				}
			}
			smprintf(s,"[System error     - %s, %i, \"%s\"]\n",description,GetLastError(),(LPCTSTR)lpMsgBuf);
			LocalFree(lpMsgBuf);
		}
	}
	return;
#endif

	if (errno!=-1) {
		if (s->di.dl == DL_TEXTERROR || s->di.dl == DL_TEXT || s->di.dl == DL_TEXTALL ||
		    s->di.dl == DL_TEXTERRORDATE || s->di.dl == DL_TEXTDATE || s->di.dl == DL_TEXTALLDATE) {
			smprintf(s,"[System error     - %s, %i, \"%s\"]\n",description,errno,strerror(errno));
		}
	}
}

#ifdef GSM_ENABLE_BACKUP

void GSM_GetPhoneFeaturesForBackup(GSM_StateMachine *s, GSM_Backup_Info *info)
{
	GSM_Error 		error;
	GSM_MemoryStatus	MemStatus;
	GSM_ToDoStatus		ToDoStatus;
	GSM_CalendarEntry       Note;
	GSM_WAPBookmark		Bookmark;
	GSM_MultiWAPSettings	WAPSettings;
 	GSM_FMStation		FMStation;
 	GSM_GPRSAccessPoint	GPRSPoint;
//	GSM_Profile		Profile;

	if (info->PhonePhonebook) {
		MemStatus.MemoryType = MEM_ME;
                error=s->Phone.Functions->GetMemoryStatus(s, &MemStatus);
		if (error==ERR_NONE && MemStatus.MemoryUsed != 0) {
		} else {
			info->PhonePhonebook = false;
		}
	}
	if (info->SIMPhonebook) {
		MemStatus.MemoryType = MEM_SM;
                error=s->Phone.Functions->GetMemoryStatus(s, &MemStatus);
		if (error==ERR_NONE && MemStatus.MemoryUsed != 0) {
		} else {
			info->SIMPhonebook = false;
		}
	}
	if (info->Calendar) {
		error=s->Phone.Functions->GetNextCalendar(s,&Note,true);
		if (error!=ERR_NONE) info->Calendar = false;
	}
	if (info->ToDo) {
		error=s->Phone.Functions->GetToDoStatus(s,&ToDoStatus);
		if (error == ERR_NONE && ToDoStatus.Used != 0) {
		} else {
			info->ToDo = false;
		}
	}
	if (info->WAPBookmark) {
		Bookmark.Location = 1;
		error=s->Phone.Functions->GetWAPBookmark(s,&Bookmark);
		if (error == ERR_NONE) {
		} else {
			info->WAPBookmark = false;
		}
	}
	if (info->WAPSettings) {
		WAPSettings.Location = 1;
		error=s->Phone.Functions->GetWAPSettings(s,&WAPSettings);
		if (error == ERR_NONE) {
		} else {
			info->WAPSettings = false;
		}
	}
	if (info->MMSSettings) {
		WAPSettings.Location = 1;
		error=s->Phone.Functions->GetMMSSettings(s,&WAPSettings);
		if (error == ERR_NONE) {
		} else {
			info->WAPSettings = false;
		}
	}
	if (info->FMStation) {
 		FMStation.Location = 1;
 		error = s->Phone.Functions->GetFMStation(s,&FMStation);
 	        if (error == ERR_NONE || error == ERR_EMPTY) {
		} else {
			info->FMStation = false;
		}
	}
	if (info->GPRSPoint) {
 		GPRSPoint.Location = 1;
 		error = s->Phone.Functions->GetGPRSAccessPoint(s,&GPRSPoint);
 	        if (error == ERR_NONE || error == ERR_EMPTY) {
		} else {
			info->GPRSPoint = false;
		}
	}
}

#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
