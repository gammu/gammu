/* (c) 2002-2004 by Marcin Wiacek and Joergen Thomsen */

#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>

#include "../../common/misc/coding/coding.h"
#include "../../common/misc/locales.h"
#include "../gammu.h"
#include "smsdcore.h"
#include "s_files.h"
#ifdef HAVE_MYSQL_MYSQL_H
#  include "s_mysql.h"
#endif
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
#  include "s_pgsql.h"
#endif

FILE 		 *smsd_log_file = NULL;
static int	 TPMR;
static GSM_Error SendingSMSStatus;

void SMSSendingSMSStatus (GSM_StateMachine *sm, int status, int mr)
{
	dbgprintf("Incoming SMS device: \"%s\" status=%d, reference=%d\n",
			GSM_GetConfig(sm, -1)->Device,
			status,
			mr);
	TPMR = mr;
	if (status==0) {
		SendingSMSStatus = ERR_NONE;
	} else {
		SendingSMSStatus = ERR_UNKNOWN;
	}
}

void GSM_Terminate_SMSD(char *msg, int error, bool exitprogram, int rc)
{
	int ret = ERR_NONE;

	if (GSM_IsConnected(s)) {
		WriteSMSDLog(_("Terminating communication"));
		ret=GSM_TerminateConnection(s);
		if (ret!=ERR_NONE) {
			printf("%s\n",GSM_ErrorString(error));
			if (GSM_IsConnected(s)) {
				GSM_TerminateConnection(s);
			}
		}
	}
	if (error != 0) {
		WriteSMSDLog(msg, error, GSM_ErrorString(error));
		fprintf(stderr, msg, error, GSM_ErrorString(error));
		fprintf(stderr, "\n");
	}
	if (exitprogram) {
		if (smsd_log_file!=NULL) fclose(smsd_log_file);
		exit(rc);
	}
}

#if defined(__GNUC__) && !defined(printf)
__attribute__((format(printf, 1, 2)))
#endif
void WriteSMSDLog(char *format, ...)
{
	GSM_DateTime 	date_time;
	char 		Buffer[2000];
	va_list		argp;
	int 		result;

	if (smsd_log_file != NULL) {
		va_start(argp, format);
		result = vsprintf(Buffer,format,argp);
		va_end(argp);

		GSM_GetCurrentDateTime(&date_time);

		fprintf(smsd_log_file,"%s %4d/%02d/%02d %02d:%02d:%02d : %s\n",
			DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
			date_time.Year, date_time.Month, date_time.Day,
			date_time.Hour, date_time.Minute, date_time.Second,Buffer);
		fflush(smsd_log_file);
	}
}

void SMSD_ReadConfig(char *filename, GSM_SMSDConfig *Config, bool log, char *service)
{
	INI_Section 		*smsdcfgfile = NULL;
	GSM_Config 		smsdcfg;
	GSM_Config 		*gammucfg;
	unsigned char		*str;
	static unsigned char	emptyPath[1] = "\0";
	GSM_Error		error;

	memset(&smsdcfg, 0, sizeof(smsdcfg));

	error = INI_ReadFile(filename, false, &smsdcfgfile);
	if (smsdcfgfile == NULL || error != ERR_NONE) {
		if (error == ERR_FILENOTSUPPORTED) {
			fprintf(stderr, _("Could not parse config file \"%s\"\n"),filename);
		} else {
			fprintf(stderr, _("Can't find file \"%s\"\n"),filename);
		}
		exit(-1);
	}

	Config->logfilename=INI_GetValue(smsdcfgfile, "smsd", "logfile", false);
	if (Config->logfilename != NULL) {
		smsd_log_file=fopen(Config->logfilename,"ab");
		if (smsd_log_file == NULL) {
			fprintf(stderr, _("Can't open file \"%s\"\n"),Config->logfilename);
			exit(-1);
		}
		fprintf(stderr, _("Log filename is \"%s\"\n"),Config->logfilename);
	}
	if (log) WriteSMSDLog(_("Starting GAMMU smsd"));

	/* Include Numbers used, because we don't want create new variable */
	Config->IncludeNumbers=INI_FindLastSectionEntry(smsdcfgfile, "gammu", false);
	if (Config->IncludeNumbers) {
		GSM_ReadConfig(smsdcfgfile, &smsdcfg, 0);
		gammucfg = GSM_GetConfig(s, 0);
		*gammucfg = smsdcfg;
		error = GSM_SetDebugFile(gammucfg->DebugFile, GSM_GetGlobalDebug());
	}

	Config->PINCode=INI_GetValue(smsdcfgfile, "smsd", "PIN", false);
	if (Config->PINCode == NULL) {
 		if (log) WriteSMSDLog(_("Warning: No PIN code in %s file"),filename);
 		fprintf(stderr, _("Warning: No PIN code in %s file\n"),filename);
	} else {
		if (log) WriteSMSDLog(_("PIN code is \"%s\""),Config->PINCode);
	}

	str = INI_GetValue(smsdcfgfile, "smsd", "commtimeout", false);
	if (str) Config->commtimeout=atoi(str); else Config->commtimeout = 1;
	str = INI_GetValue(smsdcfgfile, "smsd", "deliveryreportdelay", false);
	if (str) Config->deliveryreportdelay=atoi(str); else Config->deliveryreportdelay = 10;
	str = INI_GetValue(smsdcfgfile, "smsd", "sendtimeout", false);
	if (str) Config->sendtimeout=atoi(str); else Config->sendtimeout = 10;
	str = INI_GetValue(smsdcfgfile, "smsd", "receivefrequency", false);
	if (str) Config->receivefrequency=atoi(str); else Config->receivefrequency = 0;
	str = INI_GetValue(smsdcfgfile, "smsd", "checksecurity", false);
	if (str) Config->checksecurity=atoi(str); else Config->checksecurity = 1;
	str = INI_GetValue(smsdcfgfile, "smsd", "resetfrequency", false);
	if (str) Config->resetfrequency=atoi(str); else Config->resetfrequency = 0;
	if (log) WriteSMSDLog("commtimeout=%i, sendtimeout=%i, receivefrequency=%i, resetfrequency=%i, checksecurity=%i",
			Config->commtimeout, Config->sendtimeout, Config->receivefrequency, Config->resetfrequency, Config->checksecurity);

	Config->deliveryreport = INI_GetValue(smsdcfgfile, "smsd", "deliveryreport", false);
	if (Config->deliveryreport == NULL || (strncasecmp(Config->deliveryreport, "log", 3) != 0 && strncasecmp(Config->deliveryreport, "sms", 3) != 0)) {
		Config->deliveryreport = "no";
	}
	if (log) WriteSMSDLog("deliveryreport = %s", Config->deliveryreport);

	Config->PhoneID = INI_GetValue(smsdcfgfile, "smsd", "phoneid", false);
	if (Config->PhoneID == NULL) Config->PhoneID = "";
	if (log) WriteSMSDLog("phoneid = %s", Config->PhoneID);

	Config->RunOnReceive = INI_GetValue(smsdcfgfile, "smsd", "runonreceive", false);

	str = INI_GetValue(smsdcfgfile, "smsd", "smsc", false);
	if (str) {
		Config->SMSC.Location		= 1;
		Config->SMSC.DefaultNumber[0]	= 0;
		Config->SMSC.DefaultNumber[1]	= 0;
		Config->SMSC.Name[0]		= 0;
		Config->SMSC.Name[1]		= 0;
		Config->SMSC.Validity.Format	= SMS_Validity_NotAvailable;
		Config->SMSC.Format		= SMS_FORMAT_Text;
		EncodeUnicode(Config->SMSC.Number, str, strlen(str));
	} else {
		Config->SMSC.Location     = 0;
	}

	if (!strcasecmp(service,"FILES")) {
		Config->inboxpath=INI_GetValue(smsdcfgfile, "smsd", "inboxpath", false);
		if (Config->inboxpath == NULL) Config->inboxpath = emptyPath;

		Config->inboxformat=INI_GetValue(smsdcfgfile, "smsd", "inboxformat", false);
		if (Config->inboxformat == NULL || (strncasecmp(Config->inboxformat, "detail", 6) != 0 && strncasecmp(Config->inboxformat, "unicode", 7) != 0)) {
			Config->inboxformat = "standard";
		}
		if (log) WriteSMSDLog(_("Inbox is \"%s\" with format \"%s\""), Config->inboxpath, Config->inboxformat);

		Config->outboxpath=INI_GetValue(smsdcfgfile, "smsd", "outboxpath", false);
		if (Config->outboxpath == NULL) Config->outboxpath = emptyPath;

		Config->transmitformat=INI_GetValue(smsdcfgfile, "smsd", "transmitformat", false);
		if (Config->transmitformat == NULL || (strncasecmp(Config->transmitformat, "auto", 4) != 0 && strncasecmp(Config->transmitformat, "unicode", 7) != 0)) {
			Config->transmitformat = "7bit";
		}
		if (log) WriteSMSDLog(_("Outbox is \"%s\" with transmission format \"%s\""), Config->outboxpath, Config->transmitformat);

		Config->sentsmspath=INI_GetValue(smsdcfgfile, "smsd", "sentsmspath", false);
		if (Config->sentsmspath == NULL) Config->sentsmspath = Config->outboxpath;
		if (log) WriteSMSDLog(_("Sent SMS moved to \"%s\""),Config->sentsmspath);

		Config->errorsmspath=INI_GetValue(smsdcfgfile, "smsd", "errorsmspath", false);
		if (Config->errorsmspath == NULL) Config->errorsmspath = Config->sentsmspath;
		if (log) WriteSMSDLog(_("SMS with errors moved to \"%s\""),Config->errorsmspath);
	}

#ifdef HAVE_MYSQL_MYSQL_H
	if (!strcasecmp(service,"MYSQL")) {
		Config->skipsmscnumber = INI_GetValue(smsdcfgfile, "smsd", "skipsmscnumber", false);
		if (Config->skipsmscnumber == NULL) Config->skipsmscnumber="";
		Config->user = INI_GetValue(smsdcfgfile, "smsd", "user", false);
		if (Config->user == NULL) Config->user="root";
		Config->password = INI_GetValue(smsdcfgfile, "smsd", "password", false);
		if (Config->password == NULL) Config->password="";
		Config->PC = INI_GetValue(smsdcfgfile, "smsd", "pc", false);
		if (Config->PC == NULL) Config->PC="localhost";
		Config->database = INI_GetValue(smsdcfgfile, "smsd", "database", false);
		if (Config->database == NULL) Config->database="sms";
	}
#endif

#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	if (!strcasecmp(service,"PGSQL")) {
		Config->skipsmscnumber = INI_GetValue(smsdcfgfile, "smsd", "skipsmscnumber", false);
		if (Config->skipsmscnumber == NULL) Config->skipsmscnumber="";
		Config->user = INI_GetValue(smsdcfgfile, "smsd", "user", false);
		if (Config->user == NULL) Config->user="root";
		Config->password = INI_GetValue(smsdcfgfile, "smsd", "password", false);
		if (Config->password == NULL) Config->password="";
		Config->PC = INI_GetValue(smsdcfgfile, "smsd", "pc", false);
		if (Config->PC == NULL) Config->PC="localhost";
		Config->database = INI_GetValue(smsdcfgfile, "smsd", "database", false);
		if (Config->database == NULL) Config->database="sms";
	}
#endif

	Config->IncludeNumbers=INI_FindLastSectionEntry(smsdcfgfile, "include_numbers", false);
	Config->ExcludeNumbers=INI_FindLastSectionEntry(smsdcfgfile, "exclude_numbers", false);
	if (Config->IncludeNumbers != NULL) {
		if (log) WriteSMSDLog(_("Include numbers available"));
	}
	if (Config->ExcludeNumbers != NULL) {
		if (Config->IncludeNumbers == NULL) {
			if (log) WriteSMSDLog(_("Exclude numbers available"));
		} else {
			if (log) WriteSMSDLog(_("Exclude numbers available, but IGNORED"));
		}
	}

	Config->retries 	  = 0;
	Config->prevSMSID[0] 	  = 0;
	Config->relativevalidity  = -1;
}

bool SMSD_CheckSecurity(GSM_SMSDConfig *Config)
{
	GSM_SecurityCode 	SecurityCode;
	GSM_Error		error;

	/* Need PIN ? */
	error=GSM_GetSecurityStatus(s,&SecurityCode.Type);
	/* Unknown error */
	if (error != ERR_NOTSUPPORTED && error != ERR_NONE) {
		WriteSMSDLog(_("Error getting security status (%s:%i)"), GSM_ErrorString(error), error);
		return false;
	}
	/* No supported - do not check more */
	if (error == ERR_NOTSUPPORTED) return true;

	/* If PIN, try to enter */
	switch (SecurityCode.Type) {
	case SEC_Pin:
		if (Config->PINCode==NULL) {
			WriteSMSDLog(_("Warning: no PIN in config"));
			return false;
		} else {
			WriteSMSDLog(_("Trying to enter PIN"));
			strcpy(SecurityCode.Code,Config->PINCode);
			error=GSM_EnterSecurityCode(s,SecurityCode);
			if (error == ERR_SECURITYERROR) {
				GSM_Terminate_SMSD(_("ERROR: incorrect PIN (%s:%i)"), error, true, -1);
			}
			if (error != ERR_NONE) {
				WriteSMSDLog(_("Error entering PIN (%s:%i)"), GSM_ErrorString(error), error);
				return false;
		  	}
		}
		break;
	case SEC_SecurityCode:
	case SEC_Pin2:
	case SEC_Puk:
	case SEC_Puk2:
	case SEC_Phone:
		GSM_Terminate_SMSD(_("ERROR: phone requires not supported code type (%s:%i)"), ERR_UNKNOWN, true, -1);
	case SEC_None:
		break;
	}
	return true;
}

#ifdef WIN32
bool SMSD_RunOnReceive(GSM_MultiSMSMessage sms UNUSED, GSM_SMSDConfig *Config)
{
	BOOL ret;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	ret = CreateProcess(NULL,     /* No module name (use command line) */
			Config->RunOnReceive, /* Command line */
			NULL,           /* Process handle not inheritable*/
			NULL,           /* Thread handle not inheritable*/
			FALSE,          /* Set handle inheritance to FALSE*/
			0,              /* No creation flags*/
			NULL,           /* Use parent's environment block*/
			NULL,           /* Use parent's starting directory */
			&si,            /* Pointer to STARTUPINFO structure*/
			&pi );           /* Pointer to PROCESS_INFORMATION structure*/
	if (! ret) {
		WriteSMSDLog("CreateProcess failed (%d)\n", (int)GetLastError());
	} else {
		/* We don't need handles at all */
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return ret;
}
#else
bool SMSD_RunOnReceive(GSM_MultiSMSMessage sms UNUSED, GSM_SMSDConfig *Config)
{
	int pid;
	int i;

	if (Config->RunOnReceive == NULL) {
		return false;
	}

	pid = fork();

	if (pid == -1) {
		WriteSMSDLog("Error spawning new process");
		return false;
	}

	if (pid != 0) {
		/* We are the parent */
		return true;
	}

	/* we are the child */
	for(i = 0; i < 255; i++) {
		close(i);
	}

	execlp(Config->RunOnReceive, Config->RunOnReceive, (char*)NULL);
	exit(2);
}
#endif

bool SMSD_ReadDeleteSMS(GSM_SMSDConfig *Config, GSM_SMSDService *Service)
{
	bool			start,process;
	GSM_MultiSMSMessage 	sms;
	unsigned char 		buffer[100];
	GSM_Error		error=ERR_NONE;
	INI_Entry		*e;
	int			i;

	start=true;
	while (error == ERR_NONE && !gshutdown) {
		sms.SMS[0].Folder=0x00;
		error=GSM_GetNextSMS(s, &sms, start);
		switch (error) {
		case ERR_EMPTY:
			break;
		case ERR_NONE:
			/* Not Inbox SMS - exit */
			if (!sms.SMS[0].InboxFolder) break;
			process=true;
			DecodeUnicode(sms.SMS[0].Number,buffer);
			if (Config->IncludeNumbers != NULL) {
				e=Config->IncludeNumbers;
				process=false;
				while (1) {
					if (e == NULL) break;
					if (strcmp(buffer,e->EntryValue)==0) {
						process=true;
						break;
					}
					e = e->Prev;
				}
			} else if (Config->ExcludeNumbers != NULL) {
				e=Config->ExcludeNumbers;
				process=true;
				while (1) {
					if (e == NULL) break;
					if (strcmp(buffer,e->EntryValue)==0) {
						process=false;
						break;
					}
					e = e->Prev;
				}
			}
			if (process) {
	 			Service->SaveInboxSMS(sms, Config);
				if (Config->RunOnReceive != NULL) {
					SMSD_RunOnReceive(sms,Config);
				}
			} else {
				WriteSMSDLog(_("Excluded %s"), buffer);
			}
			break;
		default:
	 		WriteSMSDLog(_("Error getting SMS (%s:%i)"), GSM_ErrorString(error), error);
			return false;
		}
		if (error == ERR_NONE && sms.SMS[0].InboxFolder) {
			for (i=0;i<sms.Number;i++) {
				sms.SMS[i].Folder=0;
				error=GSM_DeleteSMS(s,&sms.SMS[i]);
				switch (error) {
				case ERR_NONE:
				case ERR_EMPTY:
					break;
				default:
					WriteSMSDLog(_("Error deleting SMS (%s:%i)"), GSM_ErrorString(error), error);
					return false;
				}
			}
		}
		start=false;
	}
	return true;
}

bool SMSD_CheckSMSStatus(GSM_SMSDConfig *Config,GSM_SMSDService *Service)
{
	GSM_SMSMemoryStatus	SMSStatus;
	GSM_Error		error;

	/* Do we have any SMS in phone ? */
	error=GSM_GetSMSStatus(s,&SMSStatus);
	if (error != ERR_NONE) {
		WriteSMSDLog(_("Error getting SMS status (%s:%i)"), GSM_ErrorString(error), error);
		return false;
	}
	/* Yes. We have SMS in phone */
	if (SMSStatus.SIMUsed+SMSStatus.PhoneUsed != 0) {
		return SMSD_ReadDeleteSMS(Config,Service);
	}
	return true;
}

bool SMSD_SendSMS(GSM_SMSDConfig *Config,GSM_SMSDService *Service)
{
	GSM_MultiSMSMessage  	sms;
	GSM_DateTime         	Date;
	GSM_Error            	error;
	unsigned int         	i, j, z;

	error = Service->FindOutboxSMS(&sms, Config, Config->SMSID);

	if (error == ERR_EMPTY || error == ERR_NOTSUPPORTED) {
		/* No outbox sms - wait few seconds and escape */
		for (j=0;j<Config->commtimeout && !gshutdown;j++) {
			GSM_GetCurrentDateTime (&Date);
			i=Date.Second;
	 		while (i==Date.Second && !gshutdown) {
				my_sleep(10);
				GSM_GetCurrentDateTime(&Date);
			}
			Service->RefreshPhoneStatus(Config);
		}
		return true;
	}
	if (error != ERR_NONE) {
		/* Unknown error - escape */
		WriteSMSDLog(_("Error in outbox on %s"), Config->SMSID);
		for (i=0;i<sms.Number;i++) {
			Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i+1, SMSD_SEND_ERROR, -1);
		}
		Service->MoveSMS(&sms,Config, Config->SMSID, true,false);
		return false;
	}

	if (!gshutdown) {
		if (strcmp(Config->prevSMSID, Config->SMSID) == 0) {
			Config->retries++;
			if (Config->retries > MAX_RETRIES) {
				Config->retries = 0;
				strcpy(Config->prevSMSID, "");
				WriteSMSDLog(_("Moved to errorbox: %s"), Config->SMSID);
				for (i=0;i<sms.Number;i++) {
					Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i+1, SMSD_SEND_ERROR, -1);
				}
				Service->MoveSMS(&sms,Config, Config->SMSID, true,false);
				return false;
			}
		} else {
			Config->retries = 0;
			strcpy(Config->prevSMSID, Config->SMSID);
		}
		for (i=0;i<sms.Number;i++) {
			if (sms.SMS[i].SMSC.Location == 1) {
			    	if (Config->SMSC.Location == 0) {
					Config->SMSC.Location = 1;
					error = GSM_GetSMSC(s,&Config->SMSC);
					if (error!=ERR_NONE) {
						WriteSMSDLog(_("Error getting SMSC from phone"));
						return false;
					}

			    	}
			    	memcpy(&sms.SMS[i].SMSC,&Config->SMSC,sizeof(GSM_SMSC));
			    	sms.SMS[i].SMSC.Location = 0;
				if (Config->relativevalidity != -1) {
					sms.SMS[i].SMSC.Validity.Format	  = SMS_Validity_RelativeFormat;
					sms.SMS[i].SMSC.Validity.Relative = Config->relativevalidity;
				}
			}

			if (Config->currdeliveryreport == 1) {
				sms.SMS[i].PDU = SMS_Status_Report;
			} else {
				if ((strcmp(Config->deliveryreport, "no") != 0 && (Config->currdeliveryreport == -1))) sms.SMS[i].PDU = SMS_Status_Report;
			}

			error=GSM_SendSMS(s, &sms.SMS[i]);
			if (error!=ERR_NONE) {
				Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i+1, SMSD_SEND_SENDING_ERROR, -1);
				WriteSMSDLog(_("Error sending SMS %s (%i): %s"), Config->SMSID, error,GSM_ErrorString(error));
				return false;
			}
			Service->RefreshPhoneStatus(Config);
			j    = 0;
			TPMR = -1;
			SendingSMSStatus = ERR_TIMEOUT;
			while (!gshutdown) {
				GSM_GetCurrentDateTime (&Date);
				z=Date.Second;
				while (z==Date.Second) {
					my_sleep(10);
					GSM_GetCurrentDateTime(&Date);
					GSM_ReadDevice(s,true);
					if (SendingSMSStatus != ERR_TIMEOUT) break;
				}
				Service->RefreshSendStatus(Config, Config->SMSID);
				Service->RefreshPhoneStatus(Config);
				if (SendingSMSStatus != ERR_TIMEOUT) break;
				j++;
				if (j>Config->sendtimeout) break;
			}
			if (SendingSMSStatus != ERR_NONE) {
				Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i+1, SMSD_SEND_SENDING_ERROR, TPMR);
				WriteSMSDLog(_("Error getting send status of %s (%i): %s"), Config->SMSID, SendingSMSStatus,GSM_ErrorString(SendingSMSStatus));
				return false;
			}
			error = Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i+1, SMSD_SEND_OK, TPMR);
			if (error!=ERR_NONE) {
				return false;
			}
		}
		strcpy(Config->prevSMSID, "");
		if (Service->MoveSMS(&sms,Config, Config->SMSID, false, true) != ERR_NONE) {
			Service->MoveSMS(&sms,Config, Config->SMSID, true, false);
		}
	}
	return true;
}

void SMSDaemon(int argc UNUSED, char *argv[])
{
	int                     errors = -1, initerrors=0;
	GSM_SMSDService		*Service;
	GSM_Error		error;
 	time_t			lastreceive, lastreset = 0;
	GSM_SMSDConfig		Config;

	if (!strcasecmp(argv[2],"FILES")) {
		Service = &SMSDFiles;
#ifdef HAVE_MYSQL_MYSQL_H
	} else if (!strcasecmp(argv[2],"MYSQL")) {
		Service = &SMSDMySQL;
#endif
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	} else if (!strcasecmp(argv[2],"PGSQL")) {
		Service = &SMSDPgSQL;
#endif
	} else {
		fprintf(stderr, _("Unknown SMSD service type (\"%s\")\n"), argv[2]);
		exit(-1);
	}

	SMSD_ReadConfig(argv[3], &Config, true, argv[2]);

	error = Service->Init(&Config);
	if (error!=ERR_NONE) {
		GSM_Terminate_SMSD(_("Initialisation failed, stopping Gammu smsd (%s:%i)"), error, true, -1);
	}

	signal(SIGINT, interrupt);
	signal(SIGTERM, interrupt);
	fprintf(stderr,"Press Ctrl+C to stop the program ...\n");

	lastreceive		= time(NULL);
	lastreset		= time(NULL);
	SendingSMSStatus 	= ERR_UNKNOWN;

	while (!gshutdown) {
		/* There were errors in communication - try to recover */
		if (errors > 2 || errors == -1) {
			if (errors != -1) {
				WriteSMSDLog(_("Terminating communication (%i,%i)"), error, errors);
				error=GSM_TerminateConnection(s);
			}
			if (initerrors++ > 3) my_sleep(30000);
			WriteSMSDLog(_("Starting communication"));
			error=GSM_InitConnection(s,2);
			switch (error) {
			case ERR_NONE:
				GSM_SetSendSMSStatusCallback(s, SMSSendingSMSStatus);
				if (errors == -1) {
					errors = 0;
					if (GSM_GetIMEI(s, NULL) != ERR_NONE) {
						errors++;
					} else {
						error = Service->InitAfterConnect(&Config);
						if (error!=ERR_NONE) {
							GSM_Terminate_SMSD(_("Post initialisation failed, stopping Gammu smsd (%s:%i)"), error, true, -1);
						}
						GSM_SetFastSMSSending(s, true);
					}
				} else {
					errors = 0;
				}
				if (initerrors > 3 || initerrors < 0) {
					error = GSM_Reset(s, false); /* soft reset */
					WriteSMSDLog(_("Reset return code: %s (%i) "),
							GSM_ErrorString(error),
							error);
					lastreset = time(NULL);
					my_sleep(5000);
				}
				break;
			case ERR_DEVICEOPENERROR:
				GSM_Terminate_SMSD(_("Can't open device (%s:%i)"),
						error, true, -1);
				break;
			default:
				WriteSMSDLog(_("Error at init connection (%s:%i)"),
						GSM_ErrorString(error), error);
				errors = 250;
				break;
			}
			continue;
		}
		if ((difftime(time(NULL), lastreceive) >= Config.receivefrequency) || (SendingSMSStatus != ERR_NONE)) {
	 		lastreceive = time(NULL);


			if (Config.checksecurity && !SMSD_CheckSecurity(&Config)) {
				errors++;
				initerrors++;
				continue;
			} else {
				errors=0;
			}

			initerrors = 0;

			/* read all incoming SMS */
			if (!SMSD_CheckSMSStatus(&Config,Service)) {
				errors++;
				continue;
			} else {
				errors=0;
			}

			/* time for preventive reset */
			if (Config.resetfrequency > 0 && difftime(time(NULL), lastreset) >= Config.resetfrequency) {
				errors = 254;
				initerrors = -2;
				continue;
			}
		}
		if (!SMSD_SendSMS(&Config,Service)) {
			continue;
		}
	}
	GSM_SetFastSMSSending(s,false);
	GSM_Terminate_SMSD(_("Stopping Gammu smsd"), 0, false, 0);
}

GSM_Error SMSDaemonSendSMS(char *service, char *filename, GSM_MultiSMSMessage *sms)
{
	GSM_SMSDService		*Service;
	GSM_SMSDConfig		Config;
	GSM_Error error;

	if (!strcasecmp(service,"FILES")) {
		Service = &SMSDFiles;
#ifdef HAVE_MYSQL_MYSQL_H
	} else if (!strcasecmp(service,"MYSQL")) {
		Service = &SMSDMySQL;
#endif
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	} else if (!strcasecmp(service,"PGSQL")) {
		Service = &SMSDPgSQL;
#endif
	} else {
		fprintf(stderr,"Unknown service type (\"%s\")\n",service);
		exit(-1);
	}

	SMSD_ReadConfig(filename, &Config, false, service);

	error = Service->Init(&Config);
	if (error!=ERR_NONE) return ERR_UNKNOWN;

	return Service->CreateOutboxSMS(sms,&Config);
}

GSM_Error SMSD_NoneFunction(void)
{
	return ERR_NONE;
}

GSM_Error SMSD_NotImplementedFunction(void)
{
	return ERR_NOTIMPLEMENTED;
}

GSM_Error SMSD_NotSupportedFunction(void)
{
	return ERR_NOTSUPPORTED;
}
/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
