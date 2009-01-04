/* (c) 2002-2004 by Marcin Wiacek and Joergen Thomsen */
/* (c) 2009 Michal Cihar */

#define _BSD_SOURCE

#include <string.h>
#include <signal.h>
#include <time.h>
#include <assert.h>
#ifndef WIN32
#include <sys/types.h>
#include <sys/wait.h>
#endif
#include <gammu-config.h>
#ifdef HAVE_VSYSLOG
#include <syslog.h>
#endif
#include <stdarg.h>
#include <stdlib.h>

#include <gammu-smsd.h>

/* Some systems let waitpid(2) tell callers about stopped children. */
#if !defined (WCONTINUED)
#  define WCONTINUED 0
#endif
#if !defined (WIFCONTINUED)
#  define WIFCONTINUED(s)	(0)
#endif

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
GSM_SMSDConfig		SMSDaemon_Config;

void SMSD_Shutdown(GSM_SMSDConfig *Config)
{
	Config->shutdown = true;
}

void SMSDaemon_Interrupt(int sign)
{
	signal(sign, SIG_IGN);
	SMSD_Shutdown(&SMSDaemon_Config);
}

void SMSSendingSMSStatus (GSM_StateMachine *sm, int status, int mr, void *user_data)
{
	GSM_SMSDConfig *Config = (GSM_SMSDConfig *)user_data;

	WriteSMSDLog(Config, "SMS sent on device: \"%s\" status=%d, reference=%d\n",
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

void SMSD_Terminate(GSM_SMSDConfig *Config, const char *msg, GSM_Error error, bool exitprogram, int rc)
{
	int ret = ERR_NONE;

	if (GSM_IsConnected(Config->gsm)) {
		WriteSMSDLog(Config, "Terminating communication");
		ret=GSM_TerminateConnection(Config->gsm);
		if (ret!=ERR_NONE) {
			printf("%s\n",GSM_ErrorString(error));
			if (GSM_IsConnected(Config->gsm)) {
				GSM_TerminateConnection(Config->gsm);
			}
		}
	}
	if (error != 0) {
		WriteSMSDLog(Config, "%s (%s:%i)", msg, GSM_ErrorString(error), error);
		fprintf(stderr, "%s (%s:%i)\n", msg, GSM_ErrorString(error), error);
	}
	if (exitprogram) {
		if (smsd_log_file!=NULL) fclose(smsd_log_file);
		exit(rc);
	}
}

PRINTF_STYLE(2, 3)
void WriteSMSDLog(GSM_SMSDConfig *Config, const char *format, ...)
{
	GSM_DateTime 	date_time;
	char 		Buffer[2000];
	va_list		argp;

#ifdef HAVE_VSYSLOG
	if (Config->use_syslog) {
		va_start(argp, format);
		vsyslog(LOG_NOTICE, format, argp);
		va_end(argp);
	} else
#endif
	if (smsd_log_file != NULL) {
		va_start(argp, format);
		vsprintf(Buffer,format,argp);
		va_end(argp);

		GSM_GetCurrentDateTime(&date_time);

		fprintf(smsd_log_file,"%s %4d/%02d/%02d %02d:%02d:%02d : %s\n",
			DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
			date_time.Year, date_time.Month, date_time.Day,
			date_time.Hour, date_time.Minute, date_time.Second,Buffer);
		fflush(smsd_log_file);
	}
}

void SMSD_Log_Function(const char *text, void *data)
{
	GSM_SMSDConfig *Config = (GSM_SMSDConfig *)data;
	size_t pos;
	size_t newsize;

	if (strcmp("\n", text) == 0) {
		WriteSMSDLog(Config, "gammu: %s", Config->gammu_log_buffer);
		Config->gammu_log_buffer[0] = 0;
		return;
	}

	if (Config->gammu_log_buffer == NULL) {
		pos = 0;
	} else {
		pos = strlen(Config->gammu_log_buffer);
	}
	newsize = pos + strlen(text) + 1;
	if (newsize > Config->gammu_log_buffer_size) {
		newsize += 50;
		Config->gammu_log_buffer = realloc(Config->gammu_log_buffer, newsize);
		assert(Config->gammu_log_buffer != NULL);
		Config->gammu_log_buffer_size = newsize;
	}

	strcpy(Config->gammu_log_buffer + pos, text);
}

GSM_SMSDConfig *SMSD_NewConfig(void)
{
	GSM_SMSDConfig *Config;
	Config = (GSM_SMSDConfig *)malloc(sizeof(GSM_SMSDConfig));
	if (Config == NULL) return Config;

	Config->shutdown = false;
	Config->gsm = NULL;
	Config->gammu_log_buffer = NULL;
	Config->gammu_log_buffer_size = 0;
	Config->logfilename = NULL;
	Config->smsdcfgfile = NULL;

	return Config;
}

void SMSD_FreeConfig(GSM_SMSDConfig *Config)
{
	free(Config->gammu_log_buffer);
	INI_Free(Config->smsdcfgfile);
	GSM_FreeStateMachine(Config->gsm);
	free(Config);
}


GSM_Error SMSD_ReadConfig(const char *filename, GSM_SMSDConfig *Config, bool uselog, char *service)
{
	GSM_Config 		smsdcfg;
	GSM_Config 		*gammucfg;
	unsigned char		*str;
	static unsigned char	emptyPath[1] = "\0";
	GSM_Error		error;

	memset(&smsdcfg, 0, sizeof(smsdcfg));

	Config->shutdown = false;
	Config->gsm = GSM_AllocStateMachine();
	Config->gammu_log_buffer = NULL;
	Config->gammu_log_buffer_size = 0;
	Config->logfilename = NULL;
	Config->smsdcfgfile = NULL;

	error = INI_ReadFile(filename, false, &Config->smsdcfgfile);
	if (Config->smsdcfgfile == NULL || error != ERR_NONE) {
		if (error == ERR_FILENOTSUPPORTED) {
			fprintf(stderr, "Could not parse config file \"%s\"\n",filename);
		} else {
			fprintf(stderr, "Can't find file \"%s\"\n",filename);
		}
		return ERR_CANTOPENFILE;
	}

	Config->logfilename=INI_GetValue(Config->smsdcfgfile, "smsd", "logfile", false);
	if (Config->logfilename != NULL) {
#ifdef HAVE_VSYSLOG
		if (strcmp(Config->logfilename, "syslog") == 0) {
			openlog("gammu-smsd", LOG_PID, LOG_DAEMON);
			Config->use_syslog = true;
		} else {
#endif
			Config->use_syslog = false;
			smsd_log_file=fopen(Config->logfilename,"a");
			if (smsd_log_file == NULL) {
				fprintf(stderr, "Can't open log file \"%s\"\n", Config->logfilename);
				return ERR_CANTOPENFILE;
			}
			fprintf(stderr, "Log filename is \"%s\"\n",Config->logfilename);
#ifdef HAVE_VSYSLOG
		}
#endif
	}

	if (service != NULL) {
		Config->Service = strdup(service);
	} else {
		Config->Service = INI_GetValue(Config->smsdcfgfile, "smsd", "service", false);
		if (Config->Service == NULL) {
			fprintf(stderr, "No SMSD service configure!\n");
			if (uselog) WriteSMSDLog(Config, "No SMSD service configured!");
			return ERR_UNCONFIGURED;
		}
	}

	if (uselog) WriteSMSDLog(Config, "Starting GAMMU smsd");

	/* Does our config file contain gammu section? */
	if (INI_FindLastSectionEntry(Config->smsdcfgfile, "gammu", false) == NULL) {
 		if (uselog) WriteSMSDLog(Config, "No gammu configuration found!");
 		fprintf(stderr, "No gammu configuration found!\n");
		return ERR_UNCONFIGURED;
	}

	gammucfg = GSM_GetConfig(Config->gsm, 0);
	GSM_ReadConfig(Config->smsdcfgfile, gammucfg, 0);
	GSM_SetConfigNum(Config->gsm, 1);
	gammucfg->UseGlobalDebugFile = false;

	Config->PINCode=INI_GetValue(Config->smsdcfgfile, "smsd", "PIN", false);
	if (Config->PINCode == NULL) {
 		if (uselog) WriteSMSDLog(Config, "Warning: No PIN code in %s file",filename);
 		fprintf(stderr, "Warning: No PIN code in %s file\n",filename);
	} else {
		if (uselog) WriteSMSDLog(Config, "PIN code is \"%s\"",Config->PINCode);
	}

	str = INI_GetValue(Config->smsdcfgfile, "smsd", "debugservice", false);
	if (str)
		Config->debug_service = atoi(str);
	else
		Config->debug_service = 0;

	str = INI_GetValue(Config->smsdcfgfile, "smsd", "commtimeout", false);
	if (str) Config->commtimeout=atoi(str); else Config->commtimeout = 1;
	str = INI_GetValue(Config->smsdcfgfile, "smsd", "deliveryreportdelay", false);
	if (str) Config->deliveryreportdelay=atoi(str); else Config->deliveryreportdelay = 10;
	str = INI_GetValue(Config->smsdcfgfile, "smsd", "sendtimeout", false);
	if (str) Config->sendtimeout=atoi(str); else Config->sendtimeout = 10;
	str = INI_GetValue(Config->smsdcfgfile, "smsd", "receivefrequency", false);
	if (str) Config->receivefrequency=atoi(str); else Config->receivefrequency = 0;
	str = INI_GetValue(Config->smsdcfgfile, "smsd", "checksecurity", false);
	if (str) Config->checksecurity=atoi(str); else Config->checksecurity = 1;
	str = INI_GetValue(Config->smsdcfgfile, "smsd", "resetfrequency", false);
	if (str) Config->resetfrequency=atoi(str); else Config->resetfrequency = 0;
	if (uselog) WriteSMSDLog(Config, "commtimeout=%i, sendtimeout=%i, receivefrequency=%i, resetfrequency=%i, checksecurity=%i",
			Config->commtimeout, Config->sendtimeout, Config->receivefrequency, Config->resetfrequency, Config->checksecurity);

	Config->deliveryreport = INI_GetValue(Config->smsdcfgfile, "smsd", "deliveryreport", false);
	if (Config->deliveryreport == NULL || (strncasecmp(Config->deliveryreport, "log", 3) != 0 && strncasecmp(Config->deliveryreport, "sms", 3) != 0)) {
		Config->deliveryreport = "no";
	}
	if (uselog) WriteSMSDLog(Config, "deliveryreport = %s", Config->deliveryreport);

	Config->PhoneID = INI_GetValue(Config->smsdcfgfile, "smsd", "phoneid", false);
	if (Config->PhoneID == NULL) Config->PhoneID = "";
	if (uselog) WriteSMSDLog(Config, "phoneid = %s", Config->PhoneID);

	Config->RunOnReceive = INI_GetValue(Config->smsdcfgfile, "smsd", "runonreceive", false);

	str = INI_GetValue(Config->smsdcfgfile, "smsd", "smsc", false);
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

	if (!strcasecmp(Config->Service,"FILES")) {
		Config->inboxpath=INI_GetValue(Config->smsdcfgfile, "smsd", "inboxpath", false);
		if (Config->inboxpath == NULL) Config->inboxpath = emptyPath;

		Config->inboxformat=INI_GetValue(Config->smsdcfgfile, "smsd", "inboxformat", false);
		if (Config->inboxformat == NULL || (strncasecmp(Config->inboxformat, "detail", 6) != 0 && strncasecmp(Config->inboxformat, "unicode", 7) != 0)) {
			Config->inboxformat = "standard";
		}
		if (uselog) WriteSMSDLog(Config, "Inbox is \"%s\" with format \"%s\"", Config->inboxpath, Config->inboxformat);

		Config->outboxpath=INI_GetValue(Config->smsdcfgfile, "smsd", "outboxpath", false);
		if (Config->outboxpath == NULL) Config->outboxpath = emptyPath;

		Config->transmitformat=INI_GetValue(Config->smsdcfgfile, "smsd", "transmitformat", false);
		if (Config->transmitformat == NULL || (strncasecmp(Config->transmitformat, "auto", 4) != 0 && strncasecmp(Config->transmitformat, "unicode", 7) != 0)) {
			Config->transmitformat = "7bit";
		}
		if (uselog) WriteSMSDLog(Config, "Outbox is \"%s\" with transmission format \"%s\"", Config->outboxpath, Config->transmitformat);

		Config->sentsmspath=INI_GetValue(Config->smsdcfgfile, "smsd", "sentsmspath", false);
		if (Config->sentsmspath == NULL) Config->sentsmspath = Config->outboxpath;
		if (uselog) WriteSMSDLog(Config, "Sent SMS moved to \"%s\"",Config->sentsmspath);

		Config->errorsmspath=INI_GetValue(Config->smsdcfgfile, "smsd", "errorsmspath", false);
		if (Config->errorsmspath == NULL) Config->errorsmspath = Config->sentsmspath;
		if (uselog) WriteSMSDLog(Config, "SMS with errors moved to \"%s\"",Config->errorsmspath);
	}

#ifdef HAVE_MYSQL_MYSQL_H
	if (!strcasecmp(Config->Service,"MYSQL")) {
		Config->skipsmscnumber = INI_GetValue(Config->smsdcfgfile, "smsd", "skipsmscnumber", false);
		if (Config->skipsmscnumber == NULL) Config->skipsmscnumber="";
		Config->user = INI_GetValue(Config->smsdcfgfile, "smsd", "user", false);
		if (Config->user == NULL) Config->user="root";
		Config->password = INI_GetValue(Config->smsdcfgfile, "smsd", "password", false);
		if (Config->password == NULL) Config->password="";
		Config->PC = INI_GetValue(Config->smsdcfgfile, "smsd", "pc", false);
		if (Config->PC == NULL) Config->PC="localhost";
		Config->database = INI_GetValue(Config->smsdcfgfile, "smsd", "database", false);
		if (Config->database == NULL) Config->database="sms";
	}
#endif

#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
	if (!strcasecmp(Config->Service,"PGSQL")) {
		Config->skipsmscnumber = INI_GetValue(Config->smsdcfgfile, "smsd", "skipsmscnumber", false);
		if (Config->skipsmscnumber == NULL) Config->skipsmscnumber="";
		Config->user = INI_GetValue(Config->smsdcfgfile, "smsd", "user", false);
		if (Config->user == NULL) Config->user="root";
		Config->password = INI_GetValue(Config->smsdcfgfile, "smsd", "password", false);
		if (Config->password == NULL) Config->password="";
		Config->PC = INI_GetValue(Config->smsdcfgfile, "smsd", "pc", false);
		if (Config->PC == NULL) Config->PC="localhost";
		Config->database = INI_GetValue(Config->smsdcfgfile, "smsd", "database", false);
		if (Config->database == NULL) Config->database="sms";
	}
#endif

	Config->IncludeNumbers=INI_FindLastSectionEntry(Config->smsdcfgfile, "include_numbers", false);
	Config->ExcludeNumbers=INI_FindLastSectionEntry(Config->smsdcfgfile, "exclude_numbers", false);
	if (Config->IncludeNumbers != NULL) {
		if (uselog) WriteSMSDLog(Config, "Include numbers available");
	}
	if (Config->ExcludeNumbers != NULL) {
		if (Config->IncludeNumbers == NULL) {
			if (uselog) WriteSMSDLog(Config, "Exclude numbers available");
		} else {
			if (uselog) WriteSMSDLog(Config, "Exclude numbers available, but IGNORED");
		}
	}

	Config->retries 	  = 0;
	Config->prevSMSID[0] 	  = 0;
	Config->relativevalidity  = -1;

	return ERR_NONE;
}

bool SMSD_CheckSecurity(GSM_SMSDConfig *Config)
{
	GSM_SecurityCode 	SecurityCode;
	GSM_Error		error;

	/* Need PIN ? */
	error=GSM_GetSecurityStatus(Config->gsm,&SecurityCode.Type);
	/* Unknown error */
	if (error != ERR_NOTSUPPORTED && error != ERR_NONE) {
		WriteSMSDLog(Config, "Error getting security status (%s:%i)", GSM_ErrorString(error), error);
		return false;
	}
	/* No supported - do not check more */
	if (error == ERR_NOTSUPPORTED) return true;

	/* If PIN, try to enter */
	switch (SecurityCode.Type) {
	case SEC_Pin:
		if (Config->PINCode==NULL) {
			WriteSMSDLog(Config, "Warning: no PIN in config");
			return false;
		} else {
			WriteSMSDLog(Config, "Trying to enter PIN");
			strcpy(SecurityCode.Code,Config->PINCode);
			error=GSM_EnterSecurityCode(Config->gsm,SecurityCode);
			if (error == ERR_SECURITYERROR) {
				SMSD_Terminate(Config, "ERROR: incorrect PIN", error, true, -1);
			}
			if (error != ERR_NONE) {
				WriteSMSDLog(Config, "Error entering PIN (%s:%i)", GSM_ErrorString(error), error);
				return false;
		  	}
		}
		break;
	case SEC_SecurityCode:
	case SEC_Pin2:
	case SEC_Puk:
	case SEC_Puk2:
	case SEC_Phone:
		SMSD_Terminate(Config, "ERROR: phone requires not supported code type", ERR_UNKNOWN, true, -1);
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
			(char *)Config->RunOnReceive, /* Command line */
			NULL,           /* Process handle not inheritable*/
			NULL,           /* Thread handle not inheritable*/
			FALSE,          /* Set handle inheritance to FALSE*/
			0,              /* No creation flags*/
			NULL,           /* Use parent's environment block*/
			NULL,           /* Use parent's starting directory */
			&si,            /* Pointer to STARTUPINFO structure*/
			&pi );           /* Pointer to PROCESS_INFORMATION structure*/
	if (! ret) {
		WriteSMSDLog(Config, "CreateProcess failed (%d)\n", (int)GetLastError());
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
	pid_t w;
	int status;

	if (Config->RunOnReceive == NULL) {
		return false;
	}

	pid = fork();

	if (pid == -1) {
		WriteSMSDLog(Config, "Error spawning new process");
		return false;
	}

	if (pid != 0) {
		/* We are the parent, wait for child */
		i = 0;
		do {
			w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
			if (w == -1) {
				WriteSMSDLog(Config, "Failed to wait for process");
				return false;
			}

			if (WIFEXITED(status)) {
				WriteSMSDLog(Config, "Process exited, status=%d\n", WEXITSTATUS(status));
				return (WEXITSTATUS(status) == 0);
			} else if (WIFSIGNALED(status)) {
				WriteSMSDLog(Config, "Process killed by signal %d\n", WTERMSIG(status));
				return false;
			} else if (WIFSTOPPED(status)) {
				WriteSMSDLog(Config, "Process stopped by signal %d\n", WSTOPSIG(status));
			} else if (WIFCONTINUED(status)) {
				WriteSMSDLog(Config, "Process continued\n");
			}
			usleep(100000);
			if (i++ > 1200) {
				WriteSMSDLog(Config, "Waited two minutes for child process, giving up\n");
				return true;
			}
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));

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
	sms.Number = 0;
	sms.SMS[0].Location = 0;
	while (error == ERR_NONE && !Config->shutdown) {
		sms.SMS[0].Folder = 0;
		error=GSM_GetNextSMS(Config->gsm, &sms, start);
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
	 			Service->SaveInboxSMS(&sms, Config);
				if (Config->RunOnReceive != NULL) {
					SMSD_RunOnReceive(sms,Config);
				}
			} else {
				WriteSMSDLog(Config, "Excluded %s", buffer);
			}
			break;
		default:
	 		WriteSMSDLog(Config, "Error getting SMS (%s:%i)", GSM_ErrorString(error), error);
			return false;
		}
		if (error == ERR_NONE && sms.SMS[0].InboxFolder) {
			for (i=0;i<sms.Number;i++) {
				sms.SMS[i].Folder=0;
				error=GSM_DeleteSMS(Config->gsm,&sms.SMS[i]);
				switch (error) {
				case ERR_NONE:
				case ERR_EMPTY:
					break;
				default:
					WriteSMSDLog(Config, "Error deleting SMS (%s:%i)", GSM_ErrorString(error), error);
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
	error=GSM_GetSMSStatus(Config->gsm,&SMSStatus);
	if (error != ERR_NONE) {
		WriteSMSDLog(Config, "Error getting SMS status (%s:%i)", GSM_ErrorString(error), error);
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
	unsigned int         	j, z;
	int			i;
	unsigned int		second;
	GSM_BatteryCharge  charge;
	GSM_SignalQuality  network;

	/* Clean structure before use */
	for (i = 0; i < GSM_MAX_MULTI_SMS; i++) {
		GSM_SetDefaultSMSData(&sms.SMS[i]);
	}

	error = Service->FindOutboxSMS(&sms, Config, Config->SMSID);

	if (error == ERR_EMPTY || error == ERR_NOTSUPPORTED) {
		/* No outbox sms - wait few seconds and escape */
		for (j=0;j<Config->commtimeout && !Config->shutdown;j++) {
			GSM_GetCurrentDateTime (&Date);
			second = Date.Second;
	 		while (second == Date.Second && !Config->shutdown) {
				usleep(10000);
				GSM_GetCurrentDateTime(&Date);
			}
			GSM_GetBatteryCharge(Config->gsm, &charge);
			GSM_GetSignalQuality(Config->gsm, &network);
			Service->RefreshPhoneStatus(Config, &charge, &network);
		}
		return true;
	}
	if (error != ERR_NONE) {
		/* Unknown error - escape */
		WriteSMSDLog(Config, "Error in outbox on %s", Config->SMSID);
		for (i=0;i<sms.Number;i++) {
			Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i+1, SMSD_SEND_ERROR, -1);
		}
		Service->MoveSMS(&sms,Config, Config->SMSID, true,false);
		return false;
	}

	if (!Config->shutdown) {
		if (strcmp(Config->prevSMSID, Config->SMSID) == 0) {
			Config->retries++;
			if (Config->retries > MAX_RETRIES) {
				Config->retries = 0;
				strcpy(Config->prevSMSID, "");
				WriteSMSDLog(Config, "Moved to errorbox: %s", Config->SMSID);
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
					error = GSM_GetSMSC(Config->gsm,&Config->SMSC);
					if (error!=ERR_NONE) {
						WriteSMSDLog(Config, "Error getting SMSC from phone");
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

			GSM_GetBatteryCharge(Config->gsm, &charge);
			GSM_GetSignalQuality(Config->gsm, &network);
			error=GSM_SendSMS(Config->gsm, &sms.SMS[i]);
			if (error!=ERR_NONE) {
				Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i+1, SMSD_SEND_SENDING_ERROR, -1);
				WriteSMSDLog(Config, "Error sending SMS %s (%i): %s", Config->SMSID, error,GSM_ErrorString(error));
				return false;
			}
			Service->RefreshPhoneStatus(Config, &charge, &network);
			j    = 0;
			TPMR = -1;
			SendingSMSStatus = ERR_TIMEOUT;
			while (!Config->shutdown) {
				GSM_GetCurrentDateTime (&Date);
				z=Date.Second;
				while (z==Date.Second) {
					usleep(10000);
					GSM_GetCurrentDateTime(&Date);
					GSM_ReadDevice(Config->gsm,true);
					if (SendingSMSStatus != ERR_TIMEOUT) break;
				}
				Service->RefreshSendStatus(Config, Config->SMSID);
				Service->RefreshPhoneStatus(Config, &charge, &network);
				if (SendingSMSStatus != ERR_TIMEOUT) break;
				j++;
				if (j>Config->sendtimeout) break;
			}
			if (SendingSMSStatus != ERR_NONE) {
				Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i+1, SMSD_SEND_SENDING_ERROR, TPMR);
				WriteSMSDLog(Config, "Error getting send status of %s (%i): %s", Config->SMSID, SendingSMSStatus,GSM_ErrorString(SendingSMSStatus));
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

/**
 * Returns SMSD service based on configuration.
 */
GSM_Error SMSGetService(GSM_SMSDConfig *Config, GSM_SMSDService **Service)
{
	if (strcasecmp(Config->Service, "FILES") == 0) {
		*Service = &SMSDFiles;
	} else if (strcasecmp(Config->Service, "MYSQL") == 0) {
#ifdef HAVE_MYSQL_MYSQL_H
		*Service = &SMSDMySQL;
#else
		return ERR_DISABLED;
#endif
	} else if (strcasecmp(Config->Service, "PGSQL") == 0) {
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
		*Service = &SMSDPgSQL;
#else
		return ERR_DISABLED;
#endif
	} else {
		WriteSMSDLog(Config, "Unknown SMSD service type: \"%s\"", Config->Service);
		return ERR_UNCONFIGURED;
	}
	return ERR_NONE;
}

GSM_Error SMSD_MainLoop(GSM_SMSDConfig *Config)
{
	GSM_SMSDService		*Service;
	GSM_Error		error;
	int                     errors = -1, initerrors=0;
 	time_t			lastreceive, lastreset = 0;

	error = SMSGetService(Config, &Service);
	if (error!=ERR_NONE) {
		SMSD_Terminate(Config, "Failed to setup SMSD service", error, true, -1);
	}

	error = Service->Init(Config);
	if (error!=ERR_NONE) {
		SMSD_Terminate(Config, "Initialisation failed, stopping Gammu smsd", error, true, -1);
	}

	lastreceive		= time(NULL);
	lastreset		= time(NULL);
	SendingSMSStatus 	= ERR_UNKNOWN;

	while (!Config->shutdown) {
		/* There were errors in communication - try to recover */
		if (errors > 2 || errors == -1) {
			if (errors != -1) {
				WriteSMSDLog(Config, "Terminating communication %s, (%i, %i times)",
						GSM_ErrorString(error), error, errors);
				error=GSM_TerminateConnection(Config->gsm);
			}
			if (initerrors++ > 3) sleep(30);
			WriteSMSDLog(Config, "Starting communication");
			error=GSM_InitConnection_Log(Config->gsm, 2, SMSD_Log_Function, Config);
			switch (error) {
			case ERR_NONE:
				GSM_SetSendSMSStatusCallback(Config->gsm, SMSSendingSMSStatus, Config);
				if (errors == -1) {
					errors = 0;
					if (GSM_GetIMEI(Config->gsm, Config->IMEI) != ERR_NONE) {
						errors++;
					} else {
						error = Service->InitAfterConnect(Config);
						if (error!=ERR_NONE) {
							SMSD_Terminate(Config, "Post initialisation failed, stopping Gammu smsd", error, true, -1);
						}
						GSM_SetFastSMSSending(Config->gsm, true);
					}
				} else {
					errors = 0;
				}
				if (initerrors > 3 || initerrors < 0) {
					error = GSM_Reset(Config->gsm, false); /* soft reset */
					WriteSMSDLog(Config, "Reset return code: %s (%i) ",
							GSM_ErrorString(error),
							error);
					lastreset = time(NULL);
					sleep(5);
				}
				break;
			case ERR_DEVICEOPENERROR:
				SMSD_Terminate(Config, "Can't open device",
						error, true, -1);
				break;
			default:
				WriteSMSDLog(Config, "Error at init connection %s (%i)",
						GSM_ErrorString(error), error);
				errors = 250;
				break;
			}
			continue;
		}
		if ((difftime(time(NULL), lastreceive) >= Config->receivefrequency) || (SendingSMSStatus != ERR_NONE)) {
	 		lastreceive = time(NULL);


			if (Config->checksecurity && !SMSD_CheckSecurity(Config)) {
				errors++;
				initerrors++;
				continue;
			} else {
				errors=0;
			}

			initerrors = 0;

			/* read all incoming SMS */
			if (!SMSD_CheckSMSStatus(Config,Service)) {
				errors++;
				continue;
			} else {
				errors=0;
			}

			/* time for preventive reset */
			if (Config->resetfrequency > 0 && difftime(time(NULL), lastreset) >= Config->resetfrequency) {
				errors = 254;
				initerrors = -2;
				continue;
			}
		}
		if (!SMSD_SendSMS(Config, Service)) {
			continue;
		}
	}
	Service->Free(Config);
	GSM_SetFastSMSSending(Config->gsm,false);
	SMSD_Terminate(Config, "Stopping Gammu smsd", ERR_NONE, false, 0);
	return ERR_NONE;
}

void SMSDaemon(int argc UNUSED, char *argv[])
{
	GSM_Error		error;

	fprintf(stderr,"Warning: This is deprecated functionality and will be removed!\n");

	error = SMSD_ReadConfig(argv[3], &SMSDaemon_Config, true, argv[2]);
	if (error != ERR_NONE) {
		SMSD_Terminate(&SMSDaemon_Config, "Failed to read config, stopping Gammu smsd", error, true, -1);
	}

	signal(SIGINT, SMSDaemon_Interrupt);
	signal(SIGTERM, SMSDaemon_Interrupt);
	fprintf(stderr,"Press Ctrl+C to stop the program ...\n");

	SMSD_MainLoop(&SMSDaemon_Config);
}

GSM_Error SMSDaemonSendSMS(char *service, char *filename, GSM_MultiSMSMessage *sms)
{
	GSM_SMSDService		*Service;
	GSM_SMSDConfig		Config;
	GSM_Error error;

	error = SMSD_ReadConfig(filename, &Config, false, service);
	if (error != ERR_NONE) return ERR_UNKNOWN;

	error = SMSGetService(&Config, &Service);
	if (error != ERR_NONE) return ERR_UNKNOWN;

	error = Service->Init(&Config);
	if (error != ERR_NONE) return ERR_UNKNOWN;

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
