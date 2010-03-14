
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "gsmcomon.h"
#include "gsmstate.h"
#include "misc/cfg.h"
#include "misc/coding.h"
#include "device/devfunc.h"

static void GSM_RegisterConnection(GSM_StateMachine *s, unsigned int connection,
		GSM_Device_Functions *device, GSM_Protocol_Functions *protocol)
{
	if ((unsigned int)s->ConnectionType == connection)
	{
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
	if (mystrncasecmp("fbusblue"	,connection,0)) s->ConnectionType = GCT_FBUS2BLUE;
	if (mystrncasecmp("fbusirda"	,connection,0)) s->ConnectionType = GCT_FBUS2IRDA;
	if (mystrncasecmp("phonetblue"	,connection,0)) s->ConnectionType = GCT_PHONETBLUE;
	if (mystrncasecmp("irdaphonet"	,connection,0)) s->ConnectionType = GCT_IRDAPHONET;
	if (mystrncasecmp("bluefbus"	,connection,0)) s->ConnectionType = GCT_BLUEFBUS2;
	if (mystrncasecmp("bluephonet"	,connection,0)) s->ConnectionType = GCT_BLUEPHONET;
	if (mystrncasecmp("blueat"	,connection,0)) s->ConnectionType = GCT_BLUEAT;

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
	if (s->ConnectionType==0) return GE_UNKNOWNCONNECTIONTYPESTRING;

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
#ifdef GSM_ENABLE_FBUS2BLUE
	GSM_RegisterConnection(s, GCT_FBUS2BLUE, &SerialDevice,   &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_FBUS2IRDA
	GSM_RegisterConnection(s, GCT_FBUS2IRDA, &SerialDevice,   &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_PHONETBLUE
	GSM_RegisterConnection(s, GCT_PHONETBLUE,&SerialDevice,	  &PHONETProtocol);
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
	if (s->Device.Functions==NULL || s->Protocol.Functions==NULL)
			return GE_SOURCENOTAVAILABLE;
	return GE_NONE;
}

static void GSM_RegisterModule(GSM_StateMachine *s,GSM_Phone_Functions *phone)
{
	/* Auto model */
	if (s->Config.Model[0] == 0) {
		if (strstr(phone->models,GetModelData(NULL,s->Phone.Data.Model,NULL)->model) != NULL) {
			smprintf(s,"[Module           - \"%s\"]\n",phone->models);
			s->Phone.Functions = phone;
		}
	} else {
		if (strstr(phone->models,s->Config.Model) != NULL) {
			smprintf(s,"[Module           - \"%s\"]\n",phone->models);
			s->Phone.Functions = phone;
		}
	}
}

GSM_Error GSM_RegisterAllPhoneModules(GSM_StateMachine *s)
{
	/* Auto model */
	if (s->Config.Model[0] == 0) {
#ifdef GSM_ENABLE_ATGEN
		/* With ATgen and auto model we can work with unknown models too */
		if (s->ConnectionType==GCT_AT || s->ConnectionType==GCT_BLUEAT) {
			smprintf(s,"[Module           - \"%s\"]\n",ATGENPhone.models);
			s->Phone.Functions = &ATGENPhone;
			return GE_NONE;
		}
#endif
		if (GetModelData(NULL,s->Phone.Data.Model,NULL)->model[0] == 0) return GE_UNKNOWNMODELSTRING;
	}
	s->Phone.Functions=NULL;
#ifdef GSM_ENABLE_ATGEN
	/* AT module can have the same models ID to "normal" Nokia modules */
	if (s->ConnectionType==GCT_AT || s->ConnectionType==GCT_BLUEAT) {
		GSM_RegisterModule(s,&ATGENPhone);
		if (s->Phone.Functions!=NULL) return GE_NONE;
	}
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
	if (s->Phone.Functions==NULL) return GE_UNKNOWNMODELSTRING;
	return GE_NONE;
}

GSM_Error GSM_InitConnection(GSM_StateMachine *s, int ReplyNum)
{
	GSM_Error	error;
	GSM_DateTime	time;

	s->Speed			  = 0;
	s->ReplyNum			  = ReplyNum;
	s->Phone.Data.ModelInfo		  = GetModelData("unknown",NULL,NULL);
	s->Phone.Data.Manufacturer[0]	  = 0;
	s->Phone.Data.Model[0]		  = 0;
	s->Phone.Data.Version[0]	  = 0;
	s->Phone.Data.VerDate[0]	  = 0;
	s->Phone.Data.VerNum		  = 0;
	s->Phone.Data.StartInfoCounter	  = 0;

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

	s->di 				  = di;
	s->di.use_global 		  = s->Config.UseGlobalDebugFile;
	GSM_SetDebugLevel(s->Config.DebugLevel, &s->di);
	error=GSM_SetDebugFile(s->Config.DebugFile, &s->di);
	if (error != GE_NONE) return error;

	if (s->di.dl == DL_TEXTALL || s->di.dl == DL_TEXT || s->di.dl == DL_TEXTERROR ||
	    s->di.dl == DL_TEXTALLDATE || s->di.dl == DL_TEXTDATE || s->di.dl == DL_TEXTERRORDATE)
	{
		smprintf(s,"[Gammu            - version %s built %s %s]\n",VERSION,__TIME__,__DATE__);
		smprintf(s,"[Connection       - \"%s\"]\n",s->Config.Connection);
		smprintf(s,"[Model type       - \"%s\"]\n",s->Config.Model);
		smprintf(s,"[Device           - \"%s\"]\n",s->Config.Device);
#ifdef WIN32
#  ifdef _MSC_VER
		smprintf(s,"[OS/compiler      - Windows %i.%i.%i, MS VC version %i]\n",_winmajor,_winminor,_osver,_MSC_VER);
#  else
		smprintf(s,"[OS/compiler      - win32]\n");
#  endif
#elif defined(DJGPP)
		smprintf(s,"[OS/compiler      - djgpp]\n");
#else
		smprintf(s,"[OS/compiler      - "
// Detect some Unix-like OSes:
#  if defined(linux) || defined(__linux) || defined(__linux__)
		"Linux"
#  elif defined(__FreeBSD__)
		"FreeBSD"
#  elif defined(__NetBSD__)
		"NetBSD"
#  elif defined(__OpenBSD__)
		"OpenBSD"
#  elif defined(__GNU__)
		"GNU/Hurd"
#  elif defined(sun) || defined(__sun) || defined(__sun__)
#    if defined(__SVR4)
		"Sun Solaris"
#    else
		"SunOS"
#    endif
#  elif defined(hpux) || defined(__hpux) || defined(__hpux__)
		"HP-UX"
#  elif defined(ultrix) || defined(__ultrix) || defined(__ultrix__)
		"DEC Ultrix"
#  elif defined(sgi) || defined(__sgi)
		"SGI Irix"
#  elif defined(__osf__)
		"OSF Unix"
#  elif defined(bsdi) || defined(__bsdi__)
		"BSDI Unix"
#  elif defined(_AIX)
		"AIX Unix"
#  elif defined(_UNIXWARE)
		"SCO Unixware"
#  elif defined(DGUX)
		"DG Unix"
#  elif defined(__QNX__)
		"QNX"
#  else
		"Unknown"
#endif
// Show info for some compilers:
#  if defined(__GNUC__)
		", gcc %i.%i]\n", __GNUC__, __GNUC_MINOR__
#  elif defined(__SUNPRO_CC)
		", Sun C++ %x]\n", __SUNPRO_CC
#  else
		"]\n"
#  endif
            	);
#endif
	}
	if (s->di.dl==DL_BINARY) {
		smprintf(s,"%c",((unsigned char)strlen(VERSION)));
		smprintf(s,"%s",VERSION);
	}

	error=GSM_RegisterAllConnections(s, s->Config.Connection);
	if (error!=GE_NONE) return error;

	/* Model auto */
	if (s->Config.Model[0]==0)
	{
		if (mystrncasecmp(s->Config.LockDevice,"yes",0)) {
			error = lock_device(s->Config.Device, &(s->LockFile));
			if (error != GE_NONE) return error;
		}

		/* Irda devices can set now model to some specific and
		 * we don't have to make auto detection later */
		error=s->Device.Functions->OpenDevice(s);
		if (error!=GE_NONE) return error;

		s->opened = true;

		error=s->Protocol.Functions->Initialise(s);
		if (error!=GE_NONE) return error;

		/* If still auto model, try to get model by asking phone for it */
		if (s->Phone.Data.Model[0]==0)
		{
			smprintf(s,"[Module           - \"auto\"]\n");
			switch (s->ConnectionType) {
#ifdef GSM_ENABLE_ATGEN
				case GCT_AT:
				case GCT_BLUEAT:
					s->Phone.Functions = &ATGENPhone;
					break;
#endif
#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
				case GCT_MBUS2:
				case GCT_FBUS2:
				case GCT_FBUS2DLR3:
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
			if (s->Phone.Functions == NULL) return GE_UNKNOWN;

			/* Please note, that AT module need to send first
			 * command for enabling echo
			 */
			error=s->Phone.Functions->Initialise(s);
			if (error!=GE_NONE) return error;
			
			error=s->Phone.Functions->GetModel(s);
			if (error!=GE_NONE) return error;
		}
	}

	/* Switching to "correct" module */
	error=GSM_RegisterAllPhoneModules(s);
	if (error!=GE_NONE) return error;

	/* We didn't open device earlier ? Make it now */
	if (!s->opened) {
		if (mystrncasecmp(s->Config.LockDevice,"yes",0)) {
			error = lock_device(s->Config.Device, &(s->LockFile));
			if (error != GE_NONE) return error;
		}

		error=s->Device.Functions->OpenDevice(s);
		if (error!=GE_NONE) return error;

		s->opened = true;

		error=s->Protocol.Functions->Initialise(s);
		if (error!=GE_NONE) return error;
	}

	error=s->Phone.Functions->Initialise(s);
	if (error!=GE_NONE) return error;

	if (mystrncasecmp(s->Config.StartInfo,"yes",0)) {
		s->Phone.Functions->ShowStartInfo(s,true);
		s->Phone.Data.StartInfoCounter = 30;
	}

	if (mystrncasecmp(s->Config.SyncTime,"yes",0)) {
		GSM_GetCurrentDateTime (&time);
		s->Phone.Functions->SetDateTime(s,&time);
	}

	/* For debug it's good to have firmware and real model version and manufacturer */
	error=s->Phone.Functions->GetManufacturer(s);
	if (error!=GE_NONE) return error;
	error=s->Phone.Functions->GetModel(s);
	if (error!=GE_NONE) return error;
	error=s->Phone.Functions->GetFirmware(s);
	if (error!=GE_NONE) return error;

	return GE_NONE;
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
	
	if (!s->opened) return GE_UNKNOWN;

	smprintf(s,"[Closing]\n");

	if (mystrncasecmp(s->Config.StartInfo,"yes",0)) {
		if (s->Phone.Data.StartInfoCounter > 0) s->Phone.Functions->ShowStartInfo(s,false);
	}

	if (s->Phone.Functions != NULL) {
		error=s->Phone.Functions->Terminate(s);
		if (error!=GE_NONE) return error;
	}

	error=s->Protocol.Functions->Terminate(s);	
	if (error!=GE_NONE) return error;

	error = s->Device.Functions->CloseDevice(s);
	if (error!=GE_NONE) return error;

	s->Phone.Data.ModelInfo		  = NULL;
	s->Phone.Data.Manufacturer[0]	  = 0;
	s->Phone.Data.Model[0]		  = 0;
	s->Phone.Data.Version[0]	  = 0;
	s->Phone.Data.VerDate[0]	  = 0;
	s->Phone.Data.VerNum		  = 0;

	if (s->LockFile!=NULL) unlock_device(&(s->LockFile));

	s->opened = false;

	if (!s->di.use_global && s->di.dl!=0 && s->di.df!=stdout) fclose(s->di.df);

	return GE_NONE;
}

GSM_Error GSM_WaitForOnce(GSM_StateMachine *s, unsigned char *buffer,
			  int length, unsigned char type, int time)
{
	GSM_Phone_Data			*Phone		= &s->Phone.Data;
	GSM_Protocol_Message 		*msg;
	int				i;

	i=0;
	do {
		/* Some data received. Reset timer */
		if (GSM_ReadDevice(s,true)!=0) i=0;

		msg = s->Phone.Data.RequestMsg;

		if (strcmp(s->Phone.Functions->models,"NAUTO")) {
			switch (Phone->DispatchError) {		
			case GE_UNKNOWNRESPONSE:
			case GE_UNKNOWNFRAME:
				if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR ||
				    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl==DL_TEXTERRORDATE)
				{
					if (Phone->DispatchError == GE_UNKNOWNFRAME) {
						smprintf(s, "UNKNOWN frame. If you want, PLEASE report it (see /readme.txt). Thank you\n");
					} else {
						smprintf(s, "UNKNOWN response. If you want, PLEASE report it (see /readme.txt). Thank you\n");
					}
					if (length != 0) {
						smprintf(s,"Last sent message ");
						smprintf(s, "0x%02x / 0x%04x", type, length);
						DumpMessage(s->di.df, buffer, length);
					}
					smprintf(s, "Received frame ");
					smprintf(s, "0x%02x / 0x%04x", msg->Type, msg->Length);
					DumpMessage(s->di.df, msg->Buffer, msg->Length);
					smprintf(s, "\n");
				}
				if (Phone->DispatchError == GE_UNKNOWNFRAME) Phone->DispatchError=GE_TIMEOUT;
				break;
			case GE_FRAMENOTREQUESTED:
				dprintf("[Frame not requested in this moment]\n");
				Phone->DispatchError=GE_TIMEOUT;
				break;
			default:
				break;
			}
		}
		/* Request completed */
		if (Phone->RequestID==ID_None) return Phone->DispatchError;

		i++;
	} while (i<time);

	return GE_TIMEOUT;
}

GSM_Error GSM_WaitFor (GSM_StateMachine *s, unsigned char *buffer,
		       int length, unsigned char type, int time,
		       GSM_Phone_RequestID request)
{
	GSM_Protocol		*Protocol	= &s->Protocol;
	GSM_Phone_Data		*Phone		= &s->Phone.Data;
	GSM_Error		error;
	int			reply;

	if (mystrncasecmp(s->Config.StartInfo,"yes",0)) {
		if (s->Phone.Data.StartInfoCounter > 0) {
			s->Phone.Data.StartInfoCounter--;
			if (s->Phone.Data.StartInfoCounter == 0) s->Phone.Functions->ShowStartInfo(s,false);
		}
	}

	Phone->RequestID	= request;
	Phone->DispatchError	= GE_TIMEOUT;

	for (reply=0;reply<s->ReplyNum;reply++) {
		if (reply!=0) {
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl == DL_TEXTERROR ||
			    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE || s->di.dl == DL_TEXTERRORDATE)
			{
			    smprintf(s, "[Retrying %i type 0x%02x]\n", reply, type);
			}
		}
		error = Protocol->Functions->WriteMessage(s, buffer, length, type);
		if (error!=GE_NONE) return error;

		error = GSM_WaitForOnce(s, buffer, length, type, time);
		if (error != GE_TIMEOUT) return error;
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
			if (strncmp(Reply[i].msgtype,msg->Buffer,strlen(Reply[i].msgtype))==0) {
				execute=true;
			}
		}

		if (execute) {
			*reply=i;
			if ((unsigned int)Reply[i].requestID == ID_IncomingFrame ||
			    (unsigned int)Reply[i].requestID == Data->RequestID ||
			    Data->RequestID	== ID_EachFrame) {
				return GE_NONE;
			}
			available=true;
		}
		i++;
	}

	if (available) {
		return GE_FRAMENOTREQUESTED;
	} else {
		return GE_UNKNOWNFRAME;
	}
}

GSM_Error GSM_DispatchMessage(GSM_StateMachine *s)
{
	GSM_Error		error	= GE_UNKNOWNFRAME;
	GSM_Reply_Function	*Reply;
	GSM_Protocol_Message	*msg 	= s->Phone.Data.RequestMsg;
	GSM_Phone_Data 		*Data	= &s->Phone.Data;
	int			reply, i;

	if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL ||
	    s->di.dl==DL_TEXTDATE || s->di.dl==DL_TEXTALLDATE) {
		smprintf(s, "Received frame ");
		smprintf(s, "0x%02x / 0x%04x", msg->Type, msg->Length);
		DumpMessage(s->di.df, msg->Buffer, msg->Length);
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

	if (error==GE_UNKNOWNFRAME) {
		Reply=s->Phone.Functions->ReplyFunctions;
		error=CheckReplyFunctions(s,Reply,&reply);
	}                                

	if (error==GE_NONE) {
		error=Reply[reply].Function(*msg, s);
		if ((unsigned int)Reply[reply].requestID==Data->RequestID) Data->RequestID=ID_None;
	}

	return error;
}

CFG_Header *CFG_FindGammuRC()
{
	CFG_Header	*cfg_info;
        char		*homedir;
        char		rcfile[200];

#if defined(WIN32) || defined(DJGPP)
        homedir = getenv("HOMEDRIVE");
        strncpy(rcfile, homedir ? homedir : "", 200);
        homedir = getenv("HOMEPATH");
        strncat(rcfile, homedir ? homedir : "", 200);
        strncat(rcfile, "\\gammurc", 200);
#else
        homedir = getenv("HOME");
        if (homedir) strncpy(rcfile, homedir, 200);
        strncat(rcfile, "/.gammurc", 200);
#endif

        if ((cfg_info = CFG_ReadFile(rcfile, false)) == NULL) {
#if defined(WIN32) || defined(DJGPP)
                if ((cfg_info = CFG_ReadFile("gammurc", false)) == NULL) {
//			dprintf("CFG file - No config files. Using defaults.\n");
                        return NULL;
                }
#else
                if ((cfg_info = CFG_ReadFile("/etc/gammurc", false)) == NULL) {
//			dprintf("CFG file - No config files. Using defaults.\n");
                        return NULL;
                }
#endif
        }

	return cfg_info;
}

void CFG_ReadConfig(CFG_Header *cfg_info, GSM_Config *cfg)
{
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

	if (cfg_info==NULL) return;
	
	cfg->Device 	 = CFG_Get(cfg_info, "gammu", "port", 		false);
	if (!cfg->Device) {
		cfg->Device		 	 = DefaultPort;
	} else {
		cfg->DefaultDevice 		 = false;
	}
	cfg->Connection  = CFG_Get(cfg_info, "gammu", "connection", 	false);
	if (!cfg->Connection) {
		cfg->Connection	 		 = DefaultConnection;
	} else {
		cfg->DefaultConnection		 = false;
	}
	cfg->SyncTime 	 = CFG_Get(cfg_info, "gammu", "synchronizetime",false);
	if (!cfg->SyncTime) {
		cfg->SyncTime		 	 = DefaultSynchronizeTime;
	} else {
		cfg->DefaultSyncTime		 = false;
	}
	cfg->DebugFile   = CFG_Get(cfg_info, "gammu", "logfile", 	false);
	if (!cfg->DebugFile) {
		cfg->DebugFile		 	 = DefaultDebugFile;
	} else {
		cfg->DefaultDebugFile 		 = false;
	}
	cfg->LockDevice  = CFG_Get(cfg_info, "gammu", "use_locking", 	false);
	if (!cfg->LockDevice) {
		cfg->LockDevice	 		 = DefaultLockDevice;
	} else {
		cfg->DefaultLockDevice		 = false;
	}
	Temp		 = CFG_Get(cfg_info, "gammu", "model", 		false);
	if (!Temp) {
		strcpy(cfg->Model,DefaultModel);
	} else {
		cfg->DefaultModel 		 = false;
		strcpy(cfg->Model,Temp);
	}
	Temp		 = CFG_Get(cfg_info, "gammu", "logformat", 	false);
	if (!Temp) {
		strcpy(cfg->DebugLevel,DefaultDebugLevel);
	} else {
		cfg->DefaultDebugLevel 		 = false;
		strcpy(cfg->DebugLevel,Temp);
	}
	cfg->StartInfo   = CFG_Get(cfg_info, "gammu", "startinfo", 	false);
	if (!cfg->StartInfo) {
		cfg->StartInfo	 		 = DefaultStartInfo;
	} else {
		cfg->DefaultStartInfo 		 = false;
	}
}

static OnePhoneModel allmodels[] = {
#ifdef GSM_ENABLE_NOKIA6110
	{"3210" ,"NSE-8" ,"",           {F_NOWAP,F_NOCALLER,F_NOCALENDAR,F_NOPBKUNICODE,F_POWER_BATT,F_PROFILES51,F_NOPICTUREUNI,F_NOCALLINFO,F_NODTMF,0}},
	{"3210" ,"NSE-9" ,"",           {F_NOWAP,F_NOCALLER,F_NOCALENDAR,F_NOPBKUNICODE,F_POWER_BATT,F_PROFILES51,F_NOPICTUREUNI,F_NOCALLINFO,F_NODTMF,0}},
	{"3310" ,"NHM-5" ,"",           {F_NOWAP,F_NOCALLER,F_RING_SM,F_CAL33,F_POWER_BATT,F_PROFILES33,F_NOCALLINFO,F_NODTMF,0}},
	{"3330" ,"NHM-6" ,"",           {F_NOCALLER,F_RING_SM,F_CAL33,F_PROFILES33,F_NOPICTUREUNI,F_NOCALLINFO,F_NODTMF,0}},
	{"3390" ,"NPB-1" ,"",           {F_NOWAP,F_NOCALLER,F_RING_SM,F_CAL33,F_PROFILES33,F_NOPICTUREUNI,F_NOCALLINFO,F_NODTMF,0}},
	{"3410" ,"NHM-2" ,"",           {F_RING_SM,F_CAL33,F_PROFILES33,F_NOCALLINFO,F_NODTMF,0}},
#endif
#ifdef GSM_ENABLE_NOKIA6510
	{"3510" ,"NHM-8" ,"",           {F_CAL35,F_NOTODO,F_PBK35,F_NOMMS,F_NOGPRSPOINT,0}},
	{"3510i","RH-9"   ,"",          {F_CAL35,F_NOTODO,F_PBK35,F_NOGPRSPOINT,0}},
	{"3530" ,"RH-9"   ,"",          {F_CAL35,F_NOTODO,F_PBK35,F_NOGPRSPOINT,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6510)
	{"3650" ,"NHL-8" ,"Nokia 3650", {F_RADIO,0}},
	{"5100" ,"NPM-6" ,"Nokia 5100", {F_RADIO,F_NOTODO,0}},
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
	{"6100" ,"NPL-2" ,"Nokia 6100", {F_RADIO,F_NOTODO,0}},
#endif
#ifdef GSM_ENABLE_NOKIA6110
	{"6110" ,"NSE-3" ,"",           {F_NOWAP,F_NOPICTURE,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DISPSTATUS,0}},
	{"6130" ,"NSK-3" ,"",           {F_NOWAP,F_NOPICTURE,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DISPSTATUS,0}},
	{"6150" ,"NSM-1" ,"",           {F_NOWAP,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DISPSTATUS,F_NOPICTUREUNI,0}},
	{"6190" ,"NSB-3" ,"",           {F_NOWAP,F_NOPICTURE,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DISPSTATUS,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA7110)
	{"6210" ,"NPE-3" ,"Nokia 6210", {0}},
	{"6250" ,"NHM-3" ,"Nokia 6250", {0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6510)
	{"6310" ,"NPE-4" ,"Nokia 6310", {F_NOMIDI,F_NOMMS,0}},
	{"6310i","NPL-1" ,"Nokia 6310i",{F_NOMIDI,F_BLUETOOTH,F_NOMMS,0}},
	{"6510" ,"NPM-9" ,"Nokia 6510", {F_NOMIDI,F_RADIO,F_NOFILESYSTEM,F_NOMMS,0}},
	{"6610" ,"NHL-4U","Nokia 6610", {F_RADIO,F_NOTODO,0}},
	{"6800" ,"NSB-9" ,"Nokia 6800", {F_RADIO,F_NOTODO,0}},
	{"6800" ,"NHL-6" ,"Nokia 6800", {F_RADIO,F_NOTODO,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA7110)
	{"7110" ,"NSE-5" ,"Nokia 7110", {0}},
	{"7190" ,"NSB-5" ,"Nokia 7190", {0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6510)
	{"7210" ,"NHL-4" ,"Nokia 7210", {F_RADIO,F_NOTODO,0}},
	{"7250" ,"NHL-4J","Nokia 7250", {F_RADIO,F_NOTODO,0}},
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
	{"8310" ,"NHM-7" ,"Nokia 8310", {F_NOMIDI,F_RADIO,F_NOTODO,F_NOFILESYSTEM,F_NOMMS,F_NONEWCALENDAR,0}},
	{"8390" ,"NSB-8" ,"Nokia 8390", {F_NOMIDI,F_RADIO,F_NOTODO,F_NOFILESYSTEM,F_NOMMS,F_NONEWCALENDAR,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6110)
	{"8850" ,"NSM-2" ,"Nokia 8850", {0}},
	{"8855" ,"NSM-4" ,"Nokia 8855", {0}},
	{"8890" ,"NSB-6" ,"Nokia 8890", {0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6510)
	{"8910" ,"NHM-4" ,"Nokia 8910", {F_NOMIDI,F_NOFILESYSTEM,F_NOMMS,F_NONEWCALENDAR,0}},
#endif
#ifdef GSM_ENABLE_NOKIA9210
	{"9210" ,"RAE-3" ,"",           {0}},
#endif
#ifdef GSM_ENABLE_ATGEN
	{"at"   ,	  "at",		  "",				   {0}},
	{"M20"  ,	  "M20",	  "",				   {F_M20SMS,0}},
	{"MC35" ,	  "MC35",	  "",				   {0}},
	{"C35i" ,	  "C35i",	  "",				   {0}},
	{"S35i" ,	  "S35i",	  "",				   {0}},
	{"M35i" ,	  "M35i",	  "",				   {0}},
	{"C45" ,	  "C45",	  "",				   {0}},
	{"S45" ,	  "S45",	  "",				   {0}},
	{"ME45" ,	  "ME45",	  "",				   {0}},
	{"SL45" ,	  "SL45",	  "",				   {0}},
	{"SL45i" ,	  "SL45i",	  "",				   {0}},
	{"M50" ,	  "M50",	  "",				   {0}},
	{"S45"	,	  "6618" ,	  "",				   {0}},
	{"ME45" ,	  "3618" ,	  "",				   {0}},
	{"R320s" ,	"1101201-BV R320s","",				   {0}},
	{"R380s",	"7100101-BVR380s" ,"",				   {0}},
	{"R520m",	"1130101-BVR520m" ,"",				   {0}},
	{"T39m",	"1130102-BVT39m" ,"",				   {0}},
	{"T65",		"1101901-BVT65" , "",				   {0}},
	{"T68",		"1130201-BVT68" , "",				   {0}},
	{"T68i",	"1130202-BVT68" , "",				   {0}},
	{"R600",	"102001-BVR600" , "",				   {0}},
	{"T200",	"1130501-BVT200" ,"",				   {0}},
	{"T300",	"1030601-BVT300" ,"",				   {0}},
	{"P800",	"7130501-BVP800" ,"",				   {0}},
	{"iPAQ" ,	  "iPAQ"  ,	  "" ,				   {0}},
	{"A2D"  ,	  "A2D"  ,	  "" ,				   {0}},
	{"9210" ,	  "RAE-3",	  "Nokia Communicator GSM900/1800",{0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_ALCATEL)
	{"BE5", 	  "ONE TOUCH 500","",				   {F_SMSONLYSENT,F_BROKENCPBS,0}},
	{"BF5", 	  "ONE TOUCH 715","",				   {F_SMSONLYSENT,F_BROKENCPBS,0}},
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

bool IsPhoneFeatureAvailable(OnePhoneModel *model, int feature)
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
		smprintf(s,"Sending frame ");
		smprintf(s,"0x%02x / 0x%04x", type, messagesize);
		DumpMessage(s->di.df, message, messagesize);
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

int smprintf(GSM_StateMachine *s, const char *format, ...)
{
	va_list		argp;
	int 		result=0;
	unsigned char	buffer[2000];

	va_start(argp, format);
	if (s == NULL) {
		if (di.dl != 0 && di.df) {
			result = vsprintf(buffer, format, argp);
			result = smfprintf(di.df, buffer);
		}
	} else {
		if (s->di.dl != 0 && s->di.df) {
			result = vsprintf(buffer, format, argp);
			result = smfprintf(s->di.df, buffer);
		}
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

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
