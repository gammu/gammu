/* (c) 2002-2005 by Marcin Wiacek and Michal Cihar */
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
	// cables
	if (mystrncasecmp("mbus"	,connection,0)) s->ConnectionType = GCT_MBUS2;
	if (mystrncasecmp("fbus"	,connection,0)) s->ConnectionType = GCT_FBUS2;
	if (mystrncasecmp("fbuspl2303"	,connection,0)) s->ConnectionType = GCT_FBUS2PL2303;
	if (mystrncasecmp("dlr3"	,connection,0)) s->ConnectionType = GCT_FBUS2DLR3;
	if (mystrncasecmp("fbusdlr3"	,connection,0)) s->ConnectionType = GCT_FBUS2DLR3;
	if (mystrncasecmp("dku5"	,connection,0)) s->ConnectionType = GCT_DKU5FBUS2;
	if (mystrncasecmp("dku5fbus"	,connection,0)) s->ConnectionType = GCT_DKU5FBUS2;
	if (mystrncasecmp("dku2"	,connection,0)) s->ConnectionType = GCT_DKU2PHONET;
	if (mystrncasecmp("dku2phonet"	,connection,0)) s->ConnectionType = GCT_DKU2PHONET;
	if (mystrncasecmp("dku2at"	,connection,0)) s->ConnectionType = GCT_DKU2AT;

        // for serial ports assigned by bt stack
	if (mystrncasecmp("fbusblue"	,connection,0)) s->ConnectionType = GCT_FBUS2BLUE;
	if (mystrncasecmp("phonetblue"	,connection,0)) s->ConnectionType = GCT_PHONETBLUE;
	// bt
	if (mystrncasecmp("blueobex"	,connection,0)) s->ConnectionType = GCT_BLUEOBEX;
	if (mystrncasecmp("bluephonet"	,connection,0)) s->ConnectionType = GCT_BLUEPHONET;
	if (mystrncasecmp("blueat"	,connection,0)) s->ConnectionType = GCT_BLUEAT;
	if (mystrncasecmp("bluerfobex"	,connection,0)) s->ConnectionType = GCT_BLUEOBEX;
	if (mystrncasecmp("bluerffbus"	,connection,0)) s->ConnectionType = GCT_BLUEFBUS2;
	if (mystrncasecmp("bluerfphonet",connection,0)) s->ConnectionType = GCT_BLUEPHONET;
	if (mystrncasecmp("bluerfat"	,connection,0)) s->ConnectionType = GCT_BLUEAT;
	if (mystrncasecmp("bluerfgnapbus",connection,0)) s->ConnectionType = GCT_BLUEGNAPBUS;

	// old "serial" irda
	if (mystrncasecmp("infrared"	,connection,0)) s->ConnectionType = GCT_FBUS2IRDA;
	if (mystrncasecmp("fbusirda"	,connection,0)) s->ConnectionType = GCT_FBUS2IRDA;
	// socket irda
	if (mystrncasecmp("irda"	,connection,0)) s->ConnectionType = GCT_IRDAPHONET;
	if (mystrncasecmp("irdaphonet"	,connection,0)) s->ConnectionType = GCT_IRDAPHONET;
	if (mystrncasecmp("irdaat"	,connection,0)) s->ConnectionType = GCT_IRDAAT;
	if (mystrncasecmp("irdaobex"	,connection,0)) s->ConnectionType = GCT_IRDAOBEX;
	if (mystrncasecmp("irdagnapbus" ,connection,0)) s->ConnectionType = GCT_IRDAGNAPBUS;

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
	GSM_RegisterConnection(s, GCT_MBUS2, 	  &SerialDevice,  &MBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2
	GSM_RegisterConnection(s, GCT_FBUS2,	  &SerialDevice,  &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2DLR3
	GSM_RegisterConnection(s, GCT_FBUS2DLR3,  &SerialDevice,  &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_DKU5FBUS2
	GSM_RegisterConnection(s, GCT_DKU5FBUS2,  &SerialDevice,  &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2PL2303
	GSM_RegisterConnection(s, GCT_FBUS2PL2303,&SerialDevice,  &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2BLUE
	GSM_RegisterConnection(s, GCT_FBUS2BLUE,  &SerialDevice,  &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2IRDA
	GSM_RegisterConnection(s, GCT_FBUS2IRDA,  &SerialDevice,  &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_DKU2PHONET
	GSM_RegisterConnection(s, GCT_DKU2PHONET, &SerialDevice,  &PHONETProtocol);
#endif
#ifdef GSM_ENABLE_DKU2AT
	GSM_RegisterConnection(s, GCT_DKU2AT,     &SerialDevice,  &ATProtocol);
#endif
#ifdef GSM_ENABLE_AT
	GSM_RegisterConnection(s, GCT_AT, 	  &SerialDevice,  &ATProtocol);
#endif
#ifdef GSM_ENABLE_PHONETBLUE
	GSM_RegisterConnection(s, GCT_PHONETBLUE, &SerialDevice,  &PHONETProtocol);
#endif
#ifdef GSM_ENABLE_IRDAGNAPBUS
	GSM_RegisterConnection(s, GCT_IRDAGNAPBUS,&IrdaDevice,    &GNAPBUSProtocol);
#endif
#ifdef GSM_ENABLE_IRDAPHONET
	GSM_RegisterConnection(s, GCT_IRDAPHONET, &IrdaDevice, 	  &PHONETProtocol);
#endif
#ifdef GSM_ENABLE_IRDAAT
	GSM_RegisterConnection(s, GCT_IRDAAT, 	  &IrdaDevice,    &ATProtocol);
#endif
#ifdef GSM_ENABLE_IRDAOBEX
	GSM_RegisterConnection(s, GCT_IRDAOBEX,   &IrdaDevice,    &OBEXProtocol);
#endif
#ifdef GSM_ENABLE_BLUEGNAPBUS
	GSM_RegisterConnection(s, GCT_BLUEGNAPBUS,&BlueToothDevice,&GNAPBUSProtocol);
#endif
#ifdef GSM_ENABLE_BLUEFBUS2
	GSM_RegisterConnection(s, GCT_BLUEFBUS2,  &BlueToothDevice,&FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_BLUEPHONET
	GSM_RegisterConnection(s, GCT_BLUEPHONET, &BlueToothDevice,&PHONETProtocol);
#endif
#ifdef GSM_ENABLE_BLUEAT
	GSM_RegisterConnection(s, GCT_BLUEAT, 	  &BlueToothDevice,&ATProtocol);
#endif
#ifdef GSM_ENABLE_BLUEOBEX
	GSM_RegisterConnection(s, GCT_BLUEOBEX,   &BlueToothDevice,&OBEXProtocol);
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
#ifdef GSM_ENABLE_ATGEN
		/* With ATgen and auto model we can work with unknown models too */
		if (s->ConnectionType==GCT_AT || s->ConnectionType==GCT_BLUEAT || s->ConnectionType==GCT_IRDAAT || s->ConnectionType==GCT_DKU2AT) {
#ifdef GSM_ENABLE_ALCATEL
			/* If phone provides Alcatel specific functions, enable them */
			if (model->model[0] != 0 && IsPhoneFeatureAvailable(model, F_ALCATEL)) {
				smprintf(s,"[Module           - \"%s\"]\n",ALCATELPhone.models);
				s->Phone.Functions = &ALCATELPhone;
				return ERR_NONE;
			}
#endif
#ifdef GSM_ENABLE_SONYERICSSON
			/* If phone provides Sony-Ericsson specific functions, enable them */
			if (model->model[0] != 0 && IsPhoneFeatureAvailable(model, F_SONYERICSSON)) {
				smprintf(s,"[Module           - \"%s\"]\n",SONYERICSSONPhone.models);
				s->Phone.Functions = &SONYERICSSONPhone;
				return ERR_NONE;
			}
#endif
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
	if (s->ConnectionType==GCT_AT || s->ConnectionType==GCT_BLUEAT || s->ConnectionType==GCT_IRDAAT || s->ConnectionType==GCT_DKU2AT) {
		GSM_RegisterModule(s,&ATGENPhone);
		if (s->Phone.Functions!=NULL) return ERR_NONE;
	}
#endif
#ifdef GSM_ENABLE_OBEXGEN
	GSM_RegisterModule(s,&OBEXGENPhone);
#endif
#ifdef GSM_ENABLE_GNAPGEN
	GSM_RegisterModule(s,&GNAPGENPhone);
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
#ifdef GSM_ENABLE_SONYERICSSON
	GSM_RegisterModule(s,&SONYERICSSONPhone);
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
					case GCT_DKU2AT:
						s->Phone.Functions = &ATGENPhone;
						break;
#endif
#ifdef GSM_ENABLE_OBEXGEN
					case GCT_IRDAOBEX:
					case GCT_BLUEOBEX:
						s->Phone.Functions = &OBEXGENPhone;
						break;
#endif
#ifdef GSM_ENABLE_GNAPGEN
					case GCT_BLUEGNAPBUS:
					case GCT_IRDAGNAPBUS:
						s->Phone.Functions = &GNAPGENPhone;
						break;
#endif
#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
					case GCT_MBUS2:
					case GCT_FBUS2:
					case GCT_FBUS2DLR3:
					case GCT_FBUS2PL2303:
					case GCT_FBUS2BLUE:
					case GCT_FBUS2IRDA:
					case GCT_DKU5FBUS2:
					case GCT_DKU2PHONET:
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

//	return Phone->DispatchError;
	return ERR_TIMEOUT;
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
			    Data->RequestID    == ID_EachFrame) {
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
		DumpMessage(&s->di, msg->Buffer, msg->Length);
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
			DumpMessage(&s->di, Phone->SentMsg->Buffer, Phone->SentMsg->Length);
		}
		smprintf(s, "RECEIVED frame ");
		smprintf(s, "type 0x%02X/length 0x%02X/%i", msg->Type, msg->Length, msg->Length);
		DumpMessage(&s->di, msg->Buffer, msg->Length);
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

	free(cfg->Device);
	cfg->Device 	 = INI_GetValue(cfg_info, section, "port", 		false);
	if (!cfg->Device) {
		cfg->Device		 	 = strdup(DefaultPort);
		cfg->DefaultDevice		 = true;
	} else {
		cfg->Device			 = strdup(cfg->Device);
		cfg->DefaultDevice 		 = false;
	}

	free(cfg->Connection);
	cfg->Connection  = INI_GetValue(cfg_info, section, "connection", 	false);
	if (!cfg->Connection) {
		cfg->DefaultConnection		 = true;
		cfg->Connection	 		 = strdup(DefaultConnection);
	} else {
		cfg->Connection			 = strdup(cfg->Connection);
		cfg->DefaultConnection		 = false;
	}

	free(cfg->SyncTime);
	cfg->SyncTime 	 = INI_GetValue(cfg_info, section, "synchronizetime",	false);
	if (!cfg->SyncTime) {
		cfg->DefaultSyncTime		 = true;
		cfg->SyncTime		 	 = strdup(DefaultSynchronizeTime);
	} else {
		cfg->SyncTime			 = strdup(cfg->SyncTime);
		cfg->DefaultSyncTime		 = false;
	}

	free(cfg->DebugFile);
	cfg->DebugFile   = INI_GetValue(cfg_info, section, "logfile", 		false);
	if (!cfg->DebugFile) {
		cfg->DefaultDebugFile		 = true;
		cfg->DebugFile		 	 = strdup(DefaultDebugFile);
	} else {
		cfg->DebugFile			 = strdup(cfg->DebugFile);
		cfg->DefaultDebugFile 		 = false;
	}

	free(cfg->LockDevice);
	cfg->LockDevice  = INI_GetValue(cfg_info, section, "use_locking", 	false);
	if (!cfg->LockDevice) {
		cfg->DefaultLockDevice		 = true;
		cfg->LockDevice	 		 = strdup(DefaultLockDevice);
	} else {
		cfg->LockDevice			 = strdup(cfg->LockDevice);
		cfg->DefaultLockDevice		 = false;
	}

	Temp		 = INI_GetValue(cfg_info, section, "model", 		false);
	if (!Temp) {
		cfg->DefaultModel		 = true;
		strcpy(cfg->Model,DefaultModel);
	} else {
		cfg->DefaultModel 		 = false;
		strcpy(cfg->Model,Temp);
	}

	Temp		 = INI_GetValue(cfg_info, section, "logformat", 	false);
	if (!Temp) {
		cfg->DefaultDebugLevel		 = true;
		strcpy(cfg->DebugLevel,DefaultDebugLevel);
	} else {
		cfg->DefaultDebugLevel 		 = false;
		strcpy(cfg->DebugLevel,Temp);
	}

	free(cfg->StartInfo);
	cfg->StartInfo   = INI_GetValue(cfg_info, section, "startinfo", 	false);
	if (!cfg->StartInfo) {
		cfg->DefaultStartInfo		 = true;
		cfg->StartInfo	 		 = strdup(DefaultStartInfo);
	} else {
		cfg->StartInfo			 = strdup(cfg->StartInfo);
		cfg->DefaultStartInfo 		 = false;
	}

	Temp		 = INI_GetValue(cfg_info, section, "reminder", 		false);
	if (!Temp) {
		strcpy(cfg->TextReminder,"Reminder");
	} else {
		strcpy(cfg->TextReminder,Temp);
	}

	Temp		 = INI_GetValue(cfg_info, section, "meeting", 		false);
	if (!Temp) {
		strcpy(cfg->TextMeeting,"Meeting");
	} else {
		strcpy(cfg->TextMeeting,Temp);
	}

	Temp		 = INI_GetValue(cfg_info, section, "call", 		false);
	if (!Temp) {
		strcpy(cfg->TextCall,"Call");
	} else {
		strcpy(cfg->TextCall,Temp);
	}

	Temp		 = INI_GetValue(cfg_info, section, "birthday", 		false);
	if (!Temp) {
		strcpy(cfg->TextBirthday,"Birthday");
	} else {
		strcpy(cfg->TextBirthday,Temp);
	}

	Temp		 = INI_GetValue(cfg_info, section, "memo", 		false);
	if (!Temp) {
		strcpy(cfg->TextMemo,"Memo");
	} else {
		strcpy(cfg->TextMemo,Temp);
	}

	return true;
}

static OnePhoneModel allmodels[] = {
#ifdef GSM_ENABLE_NOKIA650
	{"0650" ,"THF-12","",           {0}},
#endif
#ifdef GSM_ENABLE_NOKIA6110
	{"2100" ,"NAM-2" ,"",           {F_NOWAP,F_NOCALLER,F_RING_SM,F_CAL33,F_POWER_BATT,F_PROFILES33,F_NOCALLINFO,F_NODTMF,0}},//quess
	{"3210" ,"NSE-8" ,"",           {F_NOWAP,F_NOCALLER,F_NOCALENDAR,F_NOPBKUNICODE,F_POWER_BATT,F_PROFILES51,F_NOPICTUREUNI,F_NOCALLINFO,F_NODTMF,0}},
	{"3210" ,"NSE-9" ,"",           {F_NOWAP,F_NOCALLER,F_NOCALENDAR,F_NOPBKUNICODE,F_POWER_BATT,F_PROFILES51,F_NOPICTUREUNI,F_NOCALLINFO,F_NODTMF,0}},
	{"3310" ,"NHM-5" ,"",           {F_NOWAP,F_NOCALLER,F_RING_SM,F_CAL33,F_POWER_BATT,F_PROFILES33,F_NOCALLINFO,F_NODTMF,0}},
	{"3330" ,"NHM-6" ,"",           {F_NOCALLER,F_RING_SM,F_CAL33,F_PROFILES33,F_NOPICTUREUNI,F_NOCALLINFO,F_NODTMF,0}},
	{"3390" ,"NPB-1" ,"",           {F_NOWAP,F_NOCALLER,F_RING_SM,F_CAL33,F_PROFILES33,F_NOPICTUREUNI,F_NOCALLINFO,F_NODTMF,0}},
	{"3410" ,"NHM-2" ,"",           {F_RING_SM,F_CAL33,F_PROFILES33,F_NOCALLINFO,F_NODTMF,0}},
	{"3610" ,"NAM-1" ,"",           {F_NOCALLER,F_RING_SM,F_CAL33,F_POWER_BATT,F_PROFILES33,F_NOCALLINFO,F_NODTMF,0}},//quess
	{"5510" ,"NPM-5" ,"",           {F_NOCALLER,F_PROFILES33,F_NOPICTUREUNI,0}},
	{"5110" ,"NSE-1" ,"",           {F_NOWAP,F_NOCALLER,F_NORING,F_NOPICTURE,F_NOSTARTUP,F_NOCALENDAR,F_NOPBKUNICODE,F_PROFILES51,F_MAGICBYTES,F_DISPSTATUS,0}},
	{"5110i","NSE-2" ,"",           {F_NOWAP,F_NOCALLER,F_NORING,F_NOPICTURE,F_NOSTARTUP,F_NOCALENDAR,F_NOPBKUNICODE,F_PROFILES51,F_MAGICBYTES,F_DISPSTATUS,0}},
	{"5130" ,"NSK-1" ,"",           {F_NOWAP,F_NOCALLER,F_NORING,F_NOPICTURE,F_NOSTARTUP,F_NOCALENDAR,F_NOPBKUNICODE,F_PROFILES51,F_MAGICBYTES,F_DISPSTATUS,0}},
	{"5190" ,"NSB-1" ,"",           {F_NOWAP,F_NOCALLER,F_NORING,F_NOPICTURE,F_NOSTARTUP,F_NOCALENDAR,F_NOPBKUNICODE,F_PROFILES51,F_MAGICBYTES,F_DISPSTATUS,0}},
	{"6110" ,"NSE-3" ,"",           {F_NOWAP,F_NOPICTURE,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DISPSTATUS,0}},
	{"6130" ,"NSK-3" ,"",           {F_NOWAP,F_NOPICTURE,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DISPSTATUS,0}},
	{"6150" ,"NSM-1" ,"",           {F_NOWAP,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DISPSTATUS,F_NOPICTUREUNI,0}},
	{"6190" ,"NSB-3" ,"",           {F_NOWAP,F_NOPICTURE,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DISPSTATUS,0}},
#endif
#if defined(GSM_ENABLE_NOKIA6110) || defined(GSM_ENABLE_ATGEN)
	{"5210" ,"NSM-5" ,"Nokia 5210", {F_CAL52,F_NOSTARTANI,F_NOPICTUREUNI,F_NODTMF,0}},
	{"8210" ,"NSM-3" ,"Nokia 8210", {F_NOWAP,F_NOSTARTANI,F_NOPBKUNICODE,F_NOPICTUREUNI,0}},
	{"8250" ,"NSM-3D","Nokia 8250", {F_NOWAP,F_NOSTARTANI,F_CAL82,F_NOPICTUREUNI,0}},
	{"8290" ,"NSB-7" ,"Nokia 8290", {F_NOWAP,F_NOSTARTANI,F_NOPBKUNICODE,F_NOPICTUREUNI,0}},
	{"8850" ,"NSM-2" ,"Nokia 8850", {0}},
	{"8855" ,"NSM-4" ,"Nokia 8855", {0}},
	{"8890" ,"NSB-6" ,"Nokia 8890", {0}},
#endif
#if defined(GSM_ENABLE_NOKIA7110) || defined(GSM_ENABLE_ATGEN)
	{"6210" ,"NPE-3" ,"Nokia 6210", {F_VOICETAGS,F_CAL62,0}},
	{"6250" ,"NHM-3" ,"Nokia 6250", {F_VOICETAGS,F_CAL62,0}},
	{"7110" ,"NSE-5" ,"Nokia 7110", {F_CAL62,0}},
	{"7190" ,"NSB-5" ,"Nokia 7190", {F_CAL62,0}},
#endif
#ifdef GSM_ENABLE_NOKIA6510
	{"1100" ,"RH-18" ,"",		{0}},
	{"1100a","RH-38" ,"",		{0}},
	{"1100b","RH-36" ,"",		{0}},
	{"2650", "RH-53" ,"",           {F_PBKTONEGAL,F_PBKSMSLIST,0}},//fixme
	{"3100" ,"RH-19" ,"",           {F_PBKTONEGAL,F_PBKSMSLIST,0}},//fixme
	{"3100b","RH-50" ,"",           {F_PBKTONEGAL,F_PBKSMSLIST,0}},//fixme
 	{"3105" ,"RH-48" ,"Nokia 3105",	{F_PBKTONEGAL,F_PBKSMSLIST,F_VOICETAGS,0}},//fixme
	{"3108", "RH-6",  "Nokia 3108",	{0}}, //does it have irda ?
	{"3200", "RH-30" ,"Nokia 3200",	{F_PBKTONEGAL,0}},//fixme
	{"3200a","RH-31" ,"Nokia 3200",	{F_PBKTONEGAL,0}},//fixme
	{"3300" ,"NEM-1" ,"Nokia 3300", {F_PBKTONEGAL,0}},//fixme
	{"3300" ,"NEM-2" ,"Nokia 3300", {F_PBKTONEGAL,0}},//fixme
	{"3510" ,"NHM-8" ,"",           {F_CAL35,F_PBK35,F_NOGPRSPOINT,F_VOICETAGS,0}},
	{"3510i","RH-9"  ,"",           {F_CAL35,F_PBK35,F_NOGPRSPOINT,F_VOICETAGS,F_PROFILES,0}},
	{"3530" ,"RH-9"  ,"",           {F_CAL35,F_PBK35,F_NOGPRSPOINT,F_VOICETAGS,0}},
	{"3589i","RH-44" ,"",		{F_VOICETAGS,0}},
	{"3590" ,"NPM-8" ,"",		{0}},//irda?
	{"3595" ,"NPM-10" ,"",		{0}},//irda?
#endif
#if defined(GSM_ENABLE_NOKIA6510) || defined(GSM_ENABLE_ATGEN)
	{"3220" ,"RH-37" ,"",           {F_PBKTONEGAL,F_TODO66,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,F_SYNCML,F_FILES2,F_3220_MMS,0}},
	{"3220" ,"RH-49" ,"",           {F_PBKTONEGAL,F_TODO66,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,F_SYNCML,F_FILES2,F_3220_MMS,0}},
	{"5100" ,"NPM-6" ,"Nokia 5100", {F_PBKTONEGAL,F_TODO66,F_RADIO,0}},
	{"5100" ,"NPM-6U","Nokia 5100", {F_PBKTONEGAL,F_TODO66,F_RADIO,0}},
	{"5100" ,"NPM-6X","Nokia 5100", {F_PBKTONEGAL,F_TODO66,F_RADIO,0}},
	{"5140" ,"NPL-4" ,"Nokia 5140", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKUSER,F_WAPMMSPROXY,F_CHAT, F_SYNCML,0}},
	{"5140" ,"NPL-5" ,"Nokia 5140", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKUSER,F_WAPMMSPROXY,F_CHAT, F_SYNCML,0}},
	{"5140i","RM-104","Nokia 5140i",{F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKUSER,F_WAPMMSPROXY,F_CHAT, F_SYNCML,0}},
	{"6020" ,"RM-30" ,"Nokia 6020", {F_VOICETAGS, F_FILES2, F_CHAT, F_WAPMMSPROXY, F_TODO66, F_PBKTONEGAL, F_NOTES,0}},//quess
	{"6021" ,"RM-94" ,"Nokia 6021", {F_PBKTONEGAL,F_TODO66,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,F_CHAT,F_SYNCML,F_FILES2,F_6230iWAP,F_6230iCALLER,0}},//quess
	{"6100" ,"NPL-2" ,"Nokia 6100", {F_PBKTONEGAL,F_TODO66,0}},
	{"6101" ,"RM-76" ,"Nokia 6101", {F_PBKTONEGAL,F_TODO66,0}},
	{"6103" ,"RM-161","Nokia 6103", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_NOFILE1,0}},//fixme
	{"6111" ,"RM-82" ,"Nokia 6111", {F_SERIES40_30,F_FILES2,F_TODO66,F_RADIO,F_NOTES,F_SMS_FILES,0}},
	{"6125" ,"RM-178","Nokia 6125", {F_SERIES40_30,F_FILES2,F_TODO66,F_RADIO,F_NOTES,F_SMS_FILES,0}},//fixme
	{"6131" ,"RM-115","Nokia 6131", {F_SERIES40_30,F_FILES2,F_TODO66,F_RADIO,F_NOTES,F_SMS_FILES,0}},//fixme
	{"6170" ,"RM-47" ,"Nokia 6170", {F_PBKTONEGAL,F_TODO66,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,F_CHAT,F_SYNCML,F_FILES2,0}},
	{"6170" ,"RM-48" ,"Nokia 6170", {F_PBKTONEGAL,F_TODO66,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,F_CHAT,F_SYNCML,F_FILES2,0}},
	{"6200" ,"NPL-3" ,"Nokia 6200", {F_PBKTONEGAL,0}},
	{"6220" ,"RH-20" ,"Nokia 6220", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,F_CHAT,F_SYNCML,0}},
	{"6230" ,"RH-12" ,"Nokia 6230", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,F_CHAT,F_SYNCML,F_FILES2,0}},
	{"6230" ,"RH-28" ,"Nokia 6230", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,F_CHAT,F_SYNCML,F_FILES2,0}},
	{"6230i","RM-72" ,"Nokia 6230i",{F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,F_CHAT,F_SYNCML,F_FILES2,F_6230iWAP,F_6230iCALLER,0}},
	{"6233" ,"RM-145","Nokia 6233", {F_SERIES40_30,F_FILES2,F_TODO66,F_RADIO,F_NOTES,F_SMS_FILES,0}},//fixme
	{"6234" ,"RM-123","Nokia 6234", {F_SERIES40_30,F_FILES2,F_TODO66,F_RADIO,F_NOTES,F_SMS_FILES,0}},//fixme
	{"6270" ,"RM-56","Nokia 6270", {F_SERIES40_30,F_FILES2,F_TODO66,F_RADIO,F_NOTES,F_SMS_FILES,0}},//fixme
	{"6280" ,"RM-78","Nokia 6280", {F_SERIES40_30,F_FILES2,F_TODO66,F_RADIO,F_NOTES,F_SMS_FILES,0}},//fixme
	{"6310" ,"NPE-4" ,"Nokia 6310", {F_TODO63,F_CAL65,F_NOMIDI,F_NOMMS,F_VOICETAGS,0}},
	{"6310i","NPL-1" ,"Nokia 6310i",{F_TODO63,F_CAL65,F_NOMIDI,F_BLUETOOTH,F_NOMMS,F_VOICETAGS,F_PROFILES,0}},
	{"6385" ,"NHP-2AX","Nokia 6385",{F_TODO63,F_CAL65,F_NOMIDI,F_NOMMS,F_VOICETAGS,0}},
	{"6510" ,"NPM-9" ,"Nokia 6510", {F_TODO63,F_CAL65,F_NOMIDI,F_RADIO,F_NOFILESYSTEM,F_NOMMS,F_VOICETAGS,0}},
	{"6610" ,"NHL-4U","Nokia 6610", {F_PBKTONEGAL,F_TODO66,F_RADIO,0}},
	{"6610i","RM-37" ,"Nokia 6610i",{F_PBKTONEGAL,F_TODO66,F_RADIO,F_SYNCML,0}},
	{"6800" ,"NSB-9" ,"Nokia 6800", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,0}},
	{"6800" ,"NHL-6" ,"Nokia 6800", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,0}},
	{"6810" ,"RM-2"  ,"Nokia 6810", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,F_NOTES,F_CHAT,F_SYNCML,0}},//quess
	{"6820" ,"NHL-9" ,"Nokia 6820", {F_PBKTONEGAL,F_TODO66,F_PBKSMSLIST,F_NOTES,F_CHAT,F_SYNCML,0}},//quess
	{"6822" ,"RM-68" ,"Nokia 6822", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,F_CHAT,F_SYNCML,F_FILES2,0}},
	{"7200" ,"RH-23" ,"Nokia 7200", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,F_PBKUSER,0}},//quess
	{"7210" ,"NHL-4" ,"Nokia 7210", {F_PBKTONEGAL,F_TODO66,F_RADIO,0}},
	{"7250" ,"NHL-4J","Nokia 7250", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKIMG,F_SYNCML,0}},
	{"7250i","NHL-4JX","Nokia 7250i",{F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKIMG,F_SYNCML,0}},
	{"7260" ,"RM-17" ,"Nokia 7260", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,F_CHAT,F_SYNCML,F_FILES2,0}},
	{"7270" ,"RM-8"  ,"Nokia 7270", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,F_CHAT,F_SYNCML,F_FILES2,0}},
	{"7360" ,"RM-127","Nokia 7360", {F_PBKTONEGAL,F_TODO66,F_RADIO,F_PBKSMSLIST,F_PBKUSER,F_WAPMMSPROXY,F_NOTES,F_CHAT,F_SYNCML,F_FILES2,0}},
	{"7370" ,"RM-70" ,"Nokia 7370", {F_SERIES40_30,F_FILES2,F_TODO66,F_RADIO,F_NOTES,F_SMS_FILES,0}},//fixme
	{"7600", "NMM-3", "Nokia 7600", {F_TODO66,0}},
	{"8310" ,"NHM-7" ,"Nokia 8310", {F_CAL62,F_NOMIDI,F_RADIO,F_NOFILESYSTEM,F_NOMMS,F_VOICETAGS,0}},
	{"8390" ,"NSB-8" ,"Nokia 8390", {F_CAL62,F_NOMIDI,F_RADIO,F_NOFILESYSTEM,F_NOMMS,F_VOICETAGS,0}},
	{"8910" ,"NHM-4" ,"Nokia 8910", {F_CAL62,F_NOMIDI,F_NOFILESYSTEM,F_NOMMS,0}},
	{"8910i","NHM-4" ,"Nokia 8910i",{F_CAL62,F_NOMIDI,F_NOFILESYSTEM,F_NOMMS,0}},
#endif
#ifdef GSM_ENABLE_NOKIA3320
	{"3320" ,"NPC-1" ,"Nokia 3320", {F_CAL62,F_DAYMONTH,0}},//fixme
#endif
#if defined(GSM_ENABLE_NOKIA3650) || defined(GSM_ENABLE_ATGEN)
	{"3650" ,"NHL-8" ,"Nokia 3650", {0}},
	{"NGAGE","NEM-4" ,"",           {F_RADIO,0}},
#endif
#ifdef GSM_ENABLE_NOKIA9210
	{"9210" ,"RAE-3" ,"",           {0}},
	{"9210i","RAE-5" ,"",           {0}},
#endif
#ifdef GSM_ENABLE_ATGEN
	{"7650" ,"NHL-2" ,"Nokia 7650", {0}},
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
	{"ME45" ,	  "ME45", 	  "SIEMENS ME45",		   {0}},
	{"SL45" ,	  "SL45",	  "",				   {0}},
	{"SL45i" ,	  "SL45i",	  "",				   {0}},
	{"M50" ,	  "M50",	  "",				   {0}},
	{"S45"	,	  "6618" ,	  "",				   {0}},
	{"ME45" ,	  "3618" ,	  "",				   {0}},
	{"S55" ,	  "S55" ,	  "",				   {0}},
	{"S65" ,	  "S65" , "SIEMENS S65",				   {0}},
	/* Samsung */
	{"S100" ,	  "SGH-S100" ,    "",				   {0}},
	{"S200" ,	  "SGH-S200" ,    "",				   {0}},
	{"S300" ,	  "SGH-S300" ,    "",				   {0}},
	{"S500" ,	  "SGH-S500" ,    "",				   {0}},
	{"V200" ,	  "SGH-V200" ,    "",				   {0}},
	{"T100" ,	  "SGH-T100" ,    "",				   {0}},
	{"E700" ,	  "SGH-E700" ,    "",				   {0}},

	/* Ericsson/Sony Ericsson */
	{"T28s",	"1101101-BVT28s","",				   {F_SONYERICSSON, 0}},
	{"R320s" ,	"1101201-BV R320s","",				   {F_SONYERICSSON, 0}},
	{"R380s",	"7100101-BVR380s" ,"",				   {F_SONYERICSSON, 0}},
	{"R520m",	"1130101-BVR520m" ,"",				   {F_SONYERICSSON, 0}},
	{"T39m",	"1130102-BVT39m" ,"",				   {F_SONYERICSSON, 0}},
	{"T65",		"1101901-BVT65" , "",				   {F_SONYERICSSON, 0}},
	{"T68",		"1130201-BVT68" , "",				   {F_SONYERICSSON, 0}},
	{"T68i",	"1130202-BVT68" , "",				   {F_SONYERICSSON, 0}},
	{"R600",	"102001-BVR600" , "",				   {F_SONYERICSSON, 0}},
	{"T200",	"1130501-BVT200" ,"",				   {F_SONYERICSSON, 0}},
	{"T300",	"1130601-BVT300" ,"T300",			   {F_SONYERICSSON, 0}},
	{"T310",	"1130602-BVT310" ,"",			   	   {F_SONYERICSSON, 0}},
	{"P800",	"7130501-BVP800" ,"",				   {F_SONYERICSSON, 0}},

	/* Ericsson/Sony Ericsson till 2003, not verified */
	{"T106",	"1022101-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"SH888",	"1100801-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"R250s PRO",	"1100802-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"A1018s",	"1100901-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"A1018sc",	"1100901-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"A1028s",	"1100902-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"A1028sc",	"1100902-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T18s",	"1101001-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T18sc",	"1101001-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T10s",	"1101002-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T10sc",	"1101002-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T28s",	"1101101-BC" ,"",				   {F_SONYERICSSON, 0}},
	{"T28sc",	"1101101-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"R310s",	"1101102-BC" ,"",				   {F_SONYERICSSON, 0}},
	{"R310sc",	"1101102-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"R320s",	"1101201-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"R320sc",	"1101201-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T20e",	"1101401-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T20ec",	"1101401-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"A2618s",	"1101501-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"A2618sc",	"1101501-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"A2628s",	"1101502-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"A2628sc",	"1101502-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T29s",	"1101601-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T29sc",	"1101601-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"A3618s",	"1101801-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"A3618sc",	"1101801-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T65",		"1101901-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T65",		"1101901-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"R600",	"1102001-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"R602",	"1102001-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T100",	"1102101-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T102/T105",	"1102101-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"R520m",	"1130101-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"R520mc",	"1130101-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T39m",	"1130102-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T39mc",	"1130102-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T68",		"1130201-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T68",		"1130201-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T68i",	"1130202-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T68ie",	"1130202-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T66",		"1130401-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T66",		"1130401-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T600",	"1130402-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T602",	"1130402-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T200",	"1130501-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T202",	"1130501-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T300",	"1130601-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T302",	"1130601-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T310",	"1130602-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T312",	"1130602-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T28 WORLD",	"1141101-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"R190",	"1150101-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"R290",	"1180101-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T106",	"1222101-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T608",	"1250101-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T306",	"1260101-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T316",	"1260102-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T106 850",	"1272101-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GM12",	"6050102-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GM22",	"6100201-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GM25",	"6100202-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GM47",	"6100501-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GM27",	"6100502-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GM29",	"6100503-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GM41",	"6100504-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GR47",	"6100511-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GC75",	"6130201-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GC25",	"6150101-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GM48",	"6220501-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GM28",	"6220502-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GR48",	"6220511-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"R380s",	"7100101-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"R380sc",	"7100101-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"P800",	"7130501-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"P802",	"7130501-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"R380s world",	"7140101-BV" ,"",				   {F_SONYERICSSON, 0}},

/* Sony-Ericsson since 2003, partly not verified  */
	{"T230",	"AAA-1001012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T228",	"AAA-1001012-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T290i",	"AAA-1001013-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T290c",	"AAA-1001013-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"J300i",	"AAA-1001021-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"J300c",	"AAA-1001021-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"Z300i",	"AAA-1001031-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z300c",	"AAA-1001031-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"J230i",	"AAA-1002011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"J230c",	"AAA-1002011-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"J220i",	"AAA-1002012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"J220c",	"AAA-1002012-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"J100i",	"AAA-1002021-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"J100c",	"AAA-1002021-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T226",	"AAA-1041011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T226s/T237m",	"AAA-1041012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"J300a",	"AAA-1041021-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z300a",	"AAA-1041031-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"J230a",	"AAA-1042011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"J220a",	"AAA-1042012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"J100a",	"AAA-1042021-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T292a",	"AAA-1061011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T610",	"AAB-1021011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T618",	"AAB-1021011-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"T630",	"AAB-1021012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T628",	"AAB-1021012-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"Z200",	"AAB-1021021-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z208",	"AAB-1021021-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"Z600",	"AAB-1021031-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z608",	"AAB-1021031-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"K700i",	"AAB-1021041-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K700c",	"AAB-1021041-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"F500i",	"AAB-1021042-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K506c",	"AAB-1021042-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"K500i",	"AAB-1021043-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K500c",	"AAB-1021043-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"K508i",	"AAB-1021044-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K508c",	"AAB-1021044-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"S700i",	"AAB-1021051-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"S700c",	"AAB-1021051-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"J200i",	"AAB-1021061-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"J200c",	"AAB-1021061-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"J210i",	"AAB-1021062-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"J210c",	"AAB-1021062-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"K300i",	"AAB-1021071-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K300c",	"AAB-1021071-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"K750i",	"AAB-1022011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K750c",	"AAB-1022011-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"D750i/K758c",	"AAB-1022012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"W800i",	"AAB-1022013-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"W800c",	"AAB-1022013-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"W700i",	"AAB-1022014-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"W700c",	"AAB-1022014-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"Z550i",	"AAB-1022015-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z550c",	"AAB-1022015-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"Z558i",	"AAB-1022016-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z558c",	"AAB-1022016-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"W550i",	"AAB-1022021-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"W550c",	"AAB-1022021-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"Z530i",	"AAB-1022031-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z530c",	"AAB-1022031-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"K510i",	"AAB-1022041-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K510c",	"AAB-1022041-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"K310i",	"AAB-1022042-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K310c",	"AAB-1022042-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"K320i/K320c",	"AAB-1022043-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T616",	"AAB-1031011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"T637",	"AAB-1031012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K300a",	"AAB-1031021-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K510a",	"AAB-1032011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K310a",	"AAB-1032012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z520i",	"AAC-1052011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z520c",	"AAC-1052011-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"Z520a",	"AAC-1052012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z525i",	"AAC-1052013-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z525a",	"AAC-1052014-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z550a",	"AAC-1052031-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z1010",	"AAD-3011011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"V800/V802SE/Z800i",	"AAD-3021011-BV" ,"",			   {F_SONYERICSSON, 0}},
	{"K600i",	"AAD-3021021-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"V600i",	"AAD-3021022-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K608i",	"AAD-3021023-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"W900i",	"AAD-3022011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"W850i",	"AAD-3022021-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K800i/K800c",	"AAD-3022031-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K610i",	"AAD-3022041-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K618i/V630i",	"AAD-3022042-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K610im",	"AAD-3022043-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z610i",	"AAD-3022044-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K790i",	"AAF-1022011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K790c",	"AAF-1022011-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"Z500a",	"AAF-1031011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"S710a",	"AAF-1031021-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"K790a",	"AAF-1032011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"W712a",	"AAF-1042011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z712a",	"AAF-1042012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"W600a/W600c/W600i",	"AAF-1052021-BV" ,"",			   {F_SONYERICSSON, 0}},
	{"W300i",	"AAF-1052031-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"W300c",	"AAF-1052031-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"W710i/W710c",	"AAF-1052041-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"Z710i/Z710c",	"AAF-1052043-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"W810i",	"AAF-1052051-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"W810c",	"AAF-1052051-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"Z502a",	"AAF-1061011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GR64",	"BAC-1051012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GS64",	"BAC-1051022-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GA64",	"BAC-1051023-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"CM52",	"BAE-4001011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"CM52",	"BAE-4001012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GC82",	"FAA-1041011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"P900",	"FAB-1021011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"P908",	"FAB-1021011-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"P910i",	"FAB-1021012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"P910c",	"FAB-1021012-CN" ,"",				   {F_SONYERICSSON, 0}},
	{"M608c",	"FAB-1022011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"W958c",	"FAB-1022012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"P910a",	"FAB-1031012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"P990i",	"FAD-3022011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"M600i",	"FAD-3022012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"W950i",	"FAD-3022013-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GC99",	"FAD-3031012-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GC79",	"FAE-1021011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GC85",	"FAF-1021011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GC83",	"FAF-1031011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"VDC",		"FAF-1031021-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GC89",	"FAF-1051011-BV" ,"",				   {F_SONYERICSSON, 0}},
	{"GC86",	"FAF-1051012-BV" ,"",				   {F_SONYERICSSON, 0}},

	/* IrDA identifications for Sony-Ericsson */
	/* At least my K750i reports itself as "Sony Ericss", so detect it for IrDA */
	{"Generic SE",	"Generic SE", "Sony Ericss",			   {F_SONYERICSSON, 0}},
	{"K300 series",	"K300 series", "K300 series",			   {F_SONYERICSSON, 0}},

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

	while (model->features[i] != 0) {
		if (model->features[i] == feature) {
			return true;
		}
		i++;
	}
	return false;
}

void GSM_DumpMessageLevel2(GSM_StateMachine *s, unsigned char *message, int messagesize, int type)
{
	if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
	    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
		smprintf(s,"SENDING frame ");
		smprintf(s,"type 0x%02X/length 0x%02X/%i", type, messagesize, messagesize);
		DumpMessage(&s->di, message, messagesize);
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
	char		buffer[2000];

	va_start(argp, format);

	if ((s != NULL && s->di.df != 0) || (s == NULL && di.df != 0)) {
		result = vsprintf(buffer, format, argp);
		result = smfprintf((s == NULL) ? &di : &(s->di), "%s", buffer);
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
