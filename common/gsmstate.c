
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "gsmcomon.h"
#include "gsmstate.h"
#include "misc/cfg.h"

static void GSM_RegisterConnection(GSM_StateMachine *s, int connection,
		GSM_Device_Functions *device, GSM_Protocol_Functions *protocol)
{
	if (s->connectiontype == connection)
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
	s->connectiontype=0;
	if (!strcmp("fbus"	,connection)) s->connectiontype = GCT_FBUS2;
	if (!strcmp("mbus"	,connection)) s->connectiontype = GCT_MBUS2;
	if (!strcmp("dlr3"	,connection)) s->connectiontype = GCT_DLR3AT;
	if (!strcmp("irda"	,connection)) s->connectiontype = GCT_IRDA;
	if (!strcmp("infrared"	,connection)) s->connectiontype = GCT_INFRARED;
	if (!strcmp("at19200"	,connection)) s->connectiontype = GCT_AT19200;
	if (!strcmp("at115200"	,connection)) s->connectiontype = GCT_AT115200;
	if (!strcmp("atblue"	,connection)) s->connectiontype = GCT_ATBLUE;
	if (!strcmp("dlr3blue"	,connection)) s->connectiontype = GCT_DLR3BLUE;
	if (!strcmp("alcabus"	,connection)) s->connectiontype = GCT_ALCABUS;
	if (s->connectiontype==0) return GE_UNKNOWNCONNECTIONTYPESTRING;

	/* We check now if user gave connection type compiled & available
	 * for used OS (if not, we return, that source not available)
	 */
	s->Device.Functions	= NULL;	
	s->Protocol.Functions	= NULL;
#ifdef GSM_ENABLE_MBUS2
	GSM_RegisterConnection(s, GCT_MBUS2, 	&SerialDevice, 	 &MBUS2Protocol);
#endif
#ifdef GSM_ENABLE_DLR3AT
	GSM_RegisterConnection(s, GCT_DLR3AT, 	&SerialDevice, 	 &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_INFRARED
	GSM_RegisterConnection(s, GCT_INFRARED, &SerialDevice, 	 &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_IRDA
	GSM_RegisterConnection(s, GCT_IRDA, 	&IrdaDevice, 	 &FBUS2IRDAProtocol);
#endif
#ifdef GSM_ENABLE_FBUS2
	GSM_RegisterConnection(s, GCT_FBUS2,	&SerialDevice, 	 &FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_DLR3BLUETOOTH
	GSM_RegisterConnection(s, GCT_DLR3BLUE,	&BlueToothDevice,&FBUS2Protocol);
#endif
#ifdef GSM_ENABLE_ATBLUETOOTH
	GSM_RegisterConnection(s, GCT_ATBLUE, 	&BlueToothDevice,&ATProtocol);
#endif
#ifdef GSM_ENABLE_ALCABUS
	GSM_RegisterConnection(s, GCT_ALCABUS, 	&SerialDevice,   &ALCABUSProtocol);
#endif
#ifdef GSM_ENABLE_AT
	GSM_RegisterConnection(s, GCT_AT19200, 	&SerialDevice, 	 &ATProtocol);
	GSM_RegisterConnection(s, GCT_AT115200, &SerialDevice, 	 &ATProtocol);
#endif
	if (s->Device.Functions==NULL || s->Protocol.Functions==NULL)
			return GE_SOURCENOTAVAILABLE;
	return GE_NONE;
}

static void GSM_RegisterModule(GSM_StateMachine *s,GSM_Phone_Functions *phone)
{
	/* Auto model */
	if (s->CFGModel[0] == 0) {
		if (strstr(phone->models,GetModelData(NULL,s->Model,NULL)->model) != NULL) s->Phone.Functions = phone;
	} else {
		if (strstr(phone->models,s->CFGModel) != NULL) s->Phone.Functions = phone;
	}
}

GSM_Error GSM_RegisterAllPhoneModules(GSM_StateMachine *s)
{
	/* Auto model */
	if (s->CFGModel[0] == 0) {
		if (GetModelData(NULL,s->Model,NULL)->model[0] == 0) return GE_UNKNOWNMODELSTRING;
	}
	s->Phone.Functions=NULL;
#ifdef GSM_ENABLE_ATGEN
	/* AT module can have the same models ID to "normal" Nokia modules */
	if (s->connectiontype==GCT_AT19200 || s->connectiontype==GCT_AT115200 || s->connectiontype==GCT_ATBLUE) {
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
#ifdef GSM_ENABLE_NOKIA9110
	GSM_RegisterModule(s,&N9110Phone);
#endif
#ifdef GSM_ENABLE_ALCATEL
	GSM_RegisterModule(s,&AlcatelPhone);
#endif
	if (s->Phone.Functions==NULL) return GE_UNKNOWNMODELSTRING;
	return GE_NONE;
}

GSM_Error GSM_InitConnection(GSM_StateMachine *s, int ReplyNum)
{
	GSM_Error	error;
	bool		opened = false;
	GSM_DateTime	time;
	char		buffer[100];

	s->ReplyNum			  = ReplyNum;
	s->Model[0]			  = 0;
	s->Ver[0]			  = 0;
	s->VerDate[0]			  = 0;
	s->VerNum			  = 0;

	s->Phone.Data.Model		  = s->Model;
	s->Phone.Data.Version		  = s->Ver;
	s->Phone.Data.VersionDate	  = s->VerDate;
	s->Phone.Data.VersionNum	  = &s->VerNum;
	s->Phone.Data.Device		  = s->CFGDevice;

	s->Phone.Data.IMEICache[0]	  = 0;
	s->Phone.Data.HardwareCache[0]	  = 0;
	s->Phone.Data.ProductCodeCache[0] = 0;
	s->Phone.Data.EnableIncomingSMS	  = false;
	s->Phone.Data.EnableIncomingCB	  = false;
	s->User.UserReplyFunctions	  = NULL;
	s->User.IncomingCall		  = NULL;
	s->User.IncomingSMS		  = NULL;
	s->User.IncomingCB		  = NULL;
	s->User.SendSMSStatus		  = NULL;
	s->lockfile			  = NULL;

	s->di=di;
	GSM_SetDebugLevel(s->CFGDebugLevel, &s->di);
	error=GSM_SetDebugFile(s->CFGDebugFile, &s->di);
	if (error != GE_NONE) return error;

	if (s->di.dl == DL_TEXTALL || s->di.dl == DL_TEXT || s->di.dl == DL_TEXTERROR) {
		smprintf(s,"[Gammu            - version %s built %s %s]\n",VERSION,__TIME__,__DATE__);
		smprintf(s,"[Connection       - \"%s\"]\n",s->CFGConnection);
		smprintf(s,"[Model type       - \"%s\"]\n",s->CFGModel);
		smprintf(s,"[Device           - \"%s\"]\n",s->CFGDevice);
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
#  endif
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

	error=GSM_RegisterAllConnections(s, s->CFGConnection);
	if (error!=GE_NONE) return error;

	/* Model auto */
	if (s->CFGModel[0]==0)
	{
		if (!strcmp(s->CFGLockDevice,"yes")) {
			error = lock_device(s->CFGDevice, &(s->lockfile));
			if (error != GE_NONE) return error;
		}

		/* Irda devices can set now model to some specific and
		 * we don't have to make auto detection later */
		error=s->Device.Functions->OpenDevice(s);
		if (error!=GE_NONE) return error;

		error=s->Protocol.Functions->Initialise(s);
		if (error!=GE_NONE) return error;

		opened = true;

		/* If still auto model, try to get model by asking phone for it */
		if (s->Model[0]==0)
		{
			switch (s->connectiontype) {
#ifdef GSM_ENABLE_ATGEN
				case GCT_AT19200:
				case GCT_AT115200:
				case GCT_ATBLUE:
					s->Phone.Functions = &ATGENPhone;
					break;
#endif
#if defined(GSM_ENABLE_NOKIA_DCT3) || defined(GSM_ENABLE_NOKIA_DCT4)
				case GCT_FBUS2:
				case GCT_MBUS2:
				case GCT_INFRARED:
				case GCT_DLR3AT:
				case GCT_IRDA:
				case GCT_DLR3BLUE:
					s->Phone.Functions = &NAUTOPhone;
					break;
#endif
#ifdef ALCATEL
				case ALCABUS:
					s->Phone.Functions = &AlcatelPhone;
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
	if (!opened) {
		if (!strcmp(s->CFGLockDevice,"yes")) {
			error = lock_device(s->CFGDevice, &(s->lockfile));
			if (error != GE_NONE) return error;
		}

		error=s->Device.Functions->OpenDevice(s);
		if (error!=GE_NONE) return error;

		error=s->Protocol.Functions->Initialise(s);
		if (error!=GE_NONE) return error;
	}

	error=s->Phone.Functions->Initialise(s);
	if (error!=GE_NONE) return error;

	if (!strcmp(s->CFGSyncTime,"yes")) {
		GSM_GetCurrentDateTime (&time);
		s->Phone.Functions->SetDateTime(s,&time);
	}

	/* For debug it's good to have firmware and real model version and manufacturer */
	error=s->Phone.Functions->GetManufacturer(s,buffer);
	if (error!=GE_NONE) return error;
	error=s->Phone.Functions->GetModel(s);
	if (error!=GE_NONE) return error;
	error=s->Phone.Functions->GetFirmware(s);
	if (error!=GE_NONE) return error;

	s->opened=true;

	return GE_NONE;
}

int GSM_ReadDevice (GSM_StateMachine *s)
{
	unsigned char	buff[255];
	int		res = 0, count, i;
	GSM_DateTime	Date;

	GSM_GetCurrentDateTime (&Date);
	i=Date.Second;
	while (i==Date.Second) {
		res = s->Device.Functions->ReadDevice(s, buff, 255);
		if (res > 0) break;
		mili_sleep(10);
		GSM_GetCurrentDateTime(&Date);
	}

	for (count = 0; count < res; count++)
		s->Protocol.Functions->StateMachine(s,buff[count]);

	return res;
}

GSM_Error GSM_TerminateConnection(GSM_StateMachine *s)
{
	GSM_Error	error;

	s->opened	= false;
	s->Model[0]	= 0;
	s->Ver[0]	= 0;
	s->VerNum	= 0;
	
	error=s->Phone.Functions->Terminate(s);
	if (error!=GE_NONE) return error;

	error=s->Protocol.Functions->Terminate(s);	
	if (error!=GE_NONE) return error;

	if (s->di.dl!=0 && s->di.df!=stdout) fclose(s->di.df);

	error = s->Device.Functions->CloseDevice(s);
	if (error!=GE_NONE) return error;
	
	if (s->lockfile!=NULL) unlock_device(s->lockfile);

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
		if (GSM_ReadDevice(s)!=0) i=0;

		msg = s->Phone.Data.RequestMsg;

		switch (Phone->DispatchError) {		
		case GE_UNKNOWNRESPONSE:
		case GE_UNKNOWNFRAME:
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR) {
				if (Phone->DispatchError == GE_UNKNOWNFRAME) {
					smprintf(s, "\nUNKNOWN frame. If you want, PLEASE report it (see /readme). Thank you\n");
				} else {
					smprintf(s, "\nUNKNOWN response. If you want, PLEASE report it (see /readme). Thank you\n");
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

	Phone->RequestID	= request;
	Phone->DispatchError	= GE_TIMEOUT;

	for (reply=0;reply<s->ReplyNum;reply++) {
		if (reply!=0) {
			if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL || s->di.dl==DL_TEXTERROR) {
			    smprintf(s, "[Retrying %i]\n", reply+1);
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
			if (Reply[i].requestID	== ID_IncomingFrame ||
			    Reply[i].requestID	== Data->RequestID ||
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

	if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL) {
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
		error=Reply[reply].Function(*msg,Data,&s->User);
		if (Reply[reply].requestID==Data->RequestID) Data->RequestID=ID_None;
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

        if ((cfg_info = CFG_ReadFile(rcfile)) == NULL) {
#if defined(WIN32) || defined(DJGPP)
                if ((cfg_info = CFG_ReadFile("gammurc")) == NULL) {
//			dprintf("CFG file - No config files. Using defaults.\n");
                        return NULL;
                }
#else
                if ((cfg_info = CFG_ReadFile("/etc/gammurc")) == NULL) {
//			dprintf("CFG file - No config files. Using defaults.\n");
                        return NULL;
                }
#endif
        }

	return cfg_info;
}

void CFG_ReadConfig(CFG_Header *cfg_info, char **model, char **port,
		char **connection, char **synchronizetime, char **debugfile,
		char **debuglevel, char **lockdevice)
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

        (char *)*port		 = DefaultPort;
        (char *)*model		 = DefaultModel;
        (char *)*connection	 = DefaultConnection;
        (char *)*synchronizetime = DefaultSynchronizeTime;
	(char *)*debugfile	 = DefaultDebugFile;
	(char *)*debuglevel	 = DefaultDebugLevel;
	(char *)*lockdevice	 = DefaultLockDevice;

	if (cfg_info==NULL) return;
	
        (char *)*port = CFG_Get(cfg_info, "gammu", "port");
        if (!*port)		(char *)*port		 = DefaultPort;

        (char *)*model = CFG_Get(cfg_info, "gammu", "model");
        if (!*model)		(char *)*model		 = DefaultModel;

        (char *)*connection = CFG_Get(cfg_info, "gammu", "connection");
        if (!*connection)	(char *)*connection	 = DefaultConnection;

        (char *)*synchronizetime = CFG_Get(cfg_info, "gammu", "synchronizetime");
        if (!*synchronizetime) 	(char *)*synchronizetime = DefaultSynchronizeTime;

        (char *)*debugfile = CFG_Get(cfg_info, "gammu", "logfile");
        if (!*debugfile)	(char *)*debugfile	 = DefaultDebugFile;

        (char *)*debuglevel = CFG_Get(cfg_info, "gammu", "logformat");
        if (!*debuglevel)	(char *)*debuglevel	 = DefaultDebugLevel;

        (char *)*lockdevice = CFG_Get(cfg_info, "gammu", "use_locking");
        if (!*lockdevice)	(char *)*lockdevice	 = DefaultLockDevice;
}

static OnePhoneModel allmodels[] = {
#ifdef GSM_ENABLE_NOKIA6110
	{"3210" ,"NSE-8" ,"",           {F_NOWAP,F_NOCALLER,F_NOCALENDAR,F_NOPBKUNICODE,F_POWER_BATT,F_PROFILES51,0}},
	{"3210" ,"NSE-9" ,"",           {F_NOWAP,F_NOCALLER,F_NOCALENDAR,F_NOPBKUNICODE,F_POWER_BATT,F_PROFILES51,0}},
	{"3310" ,"NHM-5" ,"",           {F_NOWAP,F_NOCALLER,F_RING_SM,F_CAL33,F_POWER_BATT,F_PROFILES33,0}},
	{"3330" ,"NHM-6" ,"",           {F_NOCALLER,F_RING_SM,F_CAL33,F_PROFILES33,0}},
	{"3390" ,"NPB-1" ,"",           {F_NOWAP,F_NOCALLER,F_RING_SM,F_CAL33,F_PROFILES33,0}},
	{"3410" ,"NHM-2" ,"",           {F_RING_SM,F_CAL33,F_PROFILES33,0}},
#endif
#ifdef GSM_ENABLE_NOKIA6510
	{"3510" ,"NHM-8" ,"",           {0}},
#endif
#ifdef GSM_ENABLE_NOKIA6110
	{"5110" ,"NSE-1" ,"",           {F_NOWAP,F_NOCALLER,F_NORING,F_NOPICTURE,F_NOSTARTUP,F_NOCALENDAR,F_NOPBKUNICODE,F_PROFILES51,F_MAGICBYTES,F_DTMF,F_DISPSTATUS,0}},
	{"5110i","NSE-2" ,"",           {F_NOWAP,F_NOCALLER,F_NORING,F_NOPICTURE,F_NOSTARTUP,F_NOCALENDAR,F_NOPBKUNICODE,F_PROFILES51,F_MAGICBYTES,F_DTMF,F_DISPSTATUS,0}},
	{"5130" ,"NSK-1" ,"",           {F_NOWAP,F_NOCALLER,F_NORING,F_NOPICTURE,F_NOSTARTUP,F_NOCALENDAR,F_NOPBKUNICODE,F_PROFILES51,F_MAGICBYTES,F_DTMF,F_DISPSTATUS,0}},
	{"5190" ,"NSB-1" ,"",           {F_NOWAP,F_NOCALLER,F_NORING,F_NOPICTURE,F_NOSTARTUP,F_NOCALENDAR,F_NOPBKUNICODE,F_PROFILES51,F_MAGICBYTES,F_DTMF,F_DISPSTATUS,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6110)
	{"5210" ,"NSM-5" ,"Nokia 5210", {F_CAL52,F_NOSTARTANI}},
#endif
#ifdef GSM_ENABLE_NOKIA6110
	{"5510" ,"NPM-5" ,"",           {F_NOCALLER,F_PROFILES33,0}},
	{"6110" ,"NSE-3" ,"",           {F_NOWAP,F_NOPICTURE,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DTMF,F_DISPSTATUS,0}},
	{"6130" ,"NSK-3" ,"",           {F_NOWAP,F_NOPICTURE,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DTMF,F_DISPSTATUS,0}},
	{"6150" ,"NSM-1" ,"",           {F_NOWAP,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DTMF,F_DISPSTATUS,0}},
	{"6190" ,"NSB-3" ,"",           {F_NOWAP,F_NOPICTURE,F_NOSTARTANI,F_NOPBKUNICODE,F_MAGICBYTES,F_DTMF,F_DISPSTATUS,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA7110)
	{"6210" ,"NPE-3" ,"Nokia 6210", {0}},
	{"6250" ,"NHM-3" ,"Nokia 6250", {0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6510)
	{"6310" ,"NPE-4" ,"Nokia 6310", {0}},
	{"6310i","NPL-1" ,"Nokia 6310i",{0}},
	{"6510" ,"NPM-9" ,"Nokia 6510", {0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA7110)
	{"7110" ,"NSE-5" ,"Nokia 7110", {0}},
	{"7190" ,"NSB-5" ,"Nokia 7190", {0}},
#endif
#if defined(GSM_ENABLE_ATGEN)
	{"7650" ,"NHL-2" ,"Nokia 7650", {0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6110)
	{"8210" ,"NSM-3" ,"Nokia 8210", {F_NOWAP,F_NOSTARTANI,F_NOPBKUNICODE,0}},
	{"8250" ,"NSM-3D","Nokia 8250", {F_NOWAP,F_NOSTARTANI,F_CAL82,0}},
	{"8290" ,"NSB-7" ,"Nokia 8290", {F_NOWAP,F_NOSTARTANI,F_NOPBKUNICODE,0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6510)
	{"8310" ,"NHM-7" ,"Nokia 8310", {0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6110)
	{"8850" ,"NSM-2" ,"Nokia 8850", {0}},
	{"8855" ,"NSM-4" ,"Nokia 8855", {0}},
	{"8890" ,"NSB-6" ,"Nokia 8890", {0}},
#endif
#if defined(GSM_ENABLE_ATGEN) || defined(GSM_ENABLE_NOKIA6510)
	{"8910" ,"NHM-4" ,"Nokia 8910", {0}},
#endif
#ifdef GSM_ENABLE_NOKIA9110
	{"9110" ,"RAE-2" ,"",           {0}},
	{"9210" ,"RAE-3" ,"",           {0}},
#endif
#ifdef GSM_ENABLE_ATGEN
	{"at"   ,	  "at",		  "",				   {0}},
	{"M20"  ,	  "M20",	  "",				   {0}},
	{"MC35" ,	  "MC35",	  "",				   {0}},
	{"iPAQ" ,	  "iPAQ"  ,	  "",				   {0}},
	{"ONE TOUCH 500", "ONE TOUCH 500","",				   {0}},
	{"9210",	  "RAE-3",	  "Nokia Communicator GSM900/1800",{0}},
#endif
	{""     ,""      ,"",           {0}}
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

bool IsPhoneFeatureAvailable(char *model, int feature)
{
	int	i	= 0;
	bool	retval  = false;

	while (GetModelData(NULL,model,NULL)->features[i] != 0) {
		if (GetModelData(NULL,model,NULL)->features[i] == feature) {
			retval = true;
			break;
		}
		i++;
	}
	return retval;
}

void GSM_DumpMessageLevel2(GSM_StateMachine *s, unsigned char *message, int messagesize, int type)
{
	if (s->di.dl==DL_TEXT || s->di.dl==DL_TEXTALL) {
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
	va_list	argp;
	int 	result=0;

	if (s->di.dl != 0 && s->di.df) {
		va_start(argp, format);
		result = vfprintf(s->di.df, format, argp);
		fflush(s->di.df);
		va_end(argp);
	}
	return result;
}

void GSM_OSErrorInfo(GSM_StateMachine *s, char *description)
{
#ifdef WIN32
	LPVOID lpMsgBuf;

	/* We don't use errno in win32 - GetLastError gives better info */	
	if (GetLastError()!=-1) {
		if (s->di.dl == DL_TEXTERROR || s->di.dl == DL_TEXT || s->di.dl == DL_TEXTALL) {
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
			smprintf(s,"[System error     - %s, %i, \"%s\"]\n",description,GetLastError(),(LPCTSTR)lpMsgBuf);
			LocalFree(lpMsgBuf);
		}
	}
	return;
#endif

	if (errno!=-1) {
		if (s->di.dl == DL_TEXTERROR || s->di.dl == DL_TEXT || s->di.dl == DL_TEXTALL) {
			smprintf(s,"[System error     - %s, %i, \"%s\"]\n",description,errno,strerror(errno));
		}
	}
}
