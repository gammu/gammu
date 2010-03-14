
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>

#include "smsdcore.h"
#include "../gammu.h"
#include "s_files.h"
#include "../../common/misc/coding.h"

FILE 		 *smsd_log_file = NULL;
static GSM_Error SendingSMSStatus;

static void SMSSendingSMSStatus (char *Device, int status)
{
	dprintf("Incoming SMS device: \"%s\" status=%d\n",Device, status);
	if (status==0) {
		SendingSMSStatus = GE_NONE;
	} else {
		SendingSMSStatus = GE_UNKNOWN;
	}
}

void GSM_Terminate_SMSD(char *msg, int error, bool exitprogram, int rc)
{
	int ret = GE_NONE;

	if (s.opened) {
		WriteSMSDLog("Terminating communication");
		ret=GSM_TerminateConnection(&s);
		if (ret!=GE_NONE) {
			printf("%s\n",print_error(error,s.di.df,s.msg));
			if (s.opened) GSM_TerminateConnection(&s);
		}
	}
	if (error != 0) {
		WriteSMSDLog(msg, error, print_error(error,s.di.df,s.msg));
		fprintf(stderr, msg, error, print_error(error,s.di.df,s.msg));
		fprintf(stderr, "\n");
	}
	if (exitprogram) {
		if (smsd_log_file!=NULL) fclose(smsd_log_file);
		exit(rc);
	}
}

void WriteSMSDLog(unsigned char *format, ...)
{
	GSM_DateTime 	date_time;
	char 		Buffer[2000];
	va_list		argp;
	int 		result;

	if (smsd_log_file != NULL) {
		va_start(argp, format);
		result = vsprintf(Buffer,GetMsg(s.msg,format),argp);
		va_end(argp);

		GSM_GetCurrentDateTime(&date_time);

		fprintf(smsd_log_file,"%s %4d/%02d/%02d %02d:%02d:%02d [%s]: %s\n",
			DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
			date_time.Year, date_time.Month, date_time.Day,
			date_time.Hour, date_time.Minute, date_time.Second,s.Config.Device,Buffer);
		fflush(smsd_log_file);
	}
}

void SMSD_ReadConfig(int argc, char *argv[], GSM_SMSDConfig *Config)
{
	CFG_Header 		*smsdcfgfile = NULL;
	GSM_Config 		smsdcfg;
	unsigned char		*str;
	static unsigned char	emptyPath[1] = "\0";

	smsdcfgfile=CFG_ReadFile(argv[3], false);
	if (smsdcfgfile==NULL) {
		fprintf(stderr,"Can't find file \"%s\"\n",argv[3]);
		exit(-1);
	}

	Config->logfilename=CFG_Get(smsdcfgfile, "smsd", "logfile", false);
	if (Config->logfilename != NULL) {
		smsd_log_file=fopen(Config->logfilename,"ab");
		if (smsd_log_file == NULL) {
			fprintf(stderr,"Can't open file \"%s\"\n",Config->logfilename);
			exit(-1);
		}
		fprintf(stderr,"Log filename is \"%s\"\n",Config->logfilename);
	}
	WriteSMSDLog("Start GAMMU smsd");

	/* Include Numbers used, because we don't want create new variable */
	Config->IncludeNumbers=CFG_FindLastSectionEntry(smsdcfgfile, "gammu", false);
	if (Config->IncludeNumbers) {
		CFG_ReadConfig(smsdcfgfile, &smsdcfg);
		memcpy(&s.Config,&smsdcfg,sizeof(GSM_Config));
	}

	Config->PINCode=CFG_Get(smsdcfgfile, "smsd", "PIN", false);
	if (Config->PINCode == NULL) {
		WriteSMSDLog("No PIN code in %s file",argv[3]);
		fprintf(stderr,"No PIN code in %s file\n",argv[3]);
		exit(-1);
	}
	WriteSMSDLog("PIN code is \"%s\"",Config->PINCode);

	str = CFG_Get(smsdcfgfile, "smsd", "commtimeout", false);
	if (str) Config->commtimeout=atoi(str); else Config->commtimeout = 1;
	str = CFG_Get(smsdcfgfile, "smsd", "sendtimeout", false);
	if (str) Config->sendtimeout=atoi(str); else Config->sendtimeout = 10;
	str = CFG_Get(smsdcfgfile, "smsd", "receivefrequency", false);
	if (str) Config->receivefrequency=atoi(str); else Config->receivefrequency = 0;
	WriteSMSDLog("commtimeout=%i, sendtimeout=%i, receivefrequency=%i", Config->commtimeout, Config->sendtimeout, Config->receivefrequency);

	Config->deliveryreport = CFG_Get(smsdcfgfile, "smsd", "deliveryreport", false);
	if (Config->deliveryreport == NULL || (!mystrncasecmp(Config->deliveryreport, "log", 3) && !mystrncasecmp(Config->deliveryreport, "sms", 3)))
	{
		Config->deliveryreport = "no";
	}
	WriteSMSDLog("deliveryreport = %s", Config->deliveryreport);

	Config->inboxpath=CFG_Get(smsdcfgfile, "smsd", "inboxpath", false);
	if (Config->inboxpath == NULL) Config->inboxpath = emptyPath;

	Config->inboxformat=CFG_Get(smsdcfgfile, "smsd", "inboxformat", false);
	if (Config->inboxformat == NULL || (!mystrncasecmp(Config->inboxformat, "detail", 6) && !mystrncasecmp(Config->inboxformat, "unicode", 7)))
	{
		Config->inboxformat = "standard";
	}
	WriteSMSDLog("Inbox is \"%s\" with format \"%s\"", Config->inboxpath, Config->inboxformat);

	Config->outboxpath=CFG_Get(smsdcfgfile, "smsd", "outboxpath", false);
	if (Config->outboxpath == NULL) Config->outboxpath = emptyPath;

	Config->transmitformat=CFG_Get(smsdcfgfile, "smsd", "transmitformat", false);
	if (Config->transmitformat == NULL || (!mystrncasecmp(Config->transmitformat, "auto", 4) && !mystrncasecmp(Config->transmitformat, "unicode", 7)))
	{
		Config->transmitformat = "7bit";
	}
	WriteSMSDLog("Outbox is \"%s\" with transmission format \"%s\"", Config->outboxpath, Config->transmitformat);

	Config->sentsmspath=CFG_Get(smsdcfgfile, "smsd", "sentsmspath", false);
	if (Config->sentsmspath == NULL) Config->sentsmspath = Config->outboxpath;
	WriteSMSDLog("Sent SMS moved to \"%s\"",Config->sentsmspath);

	Config->errorsmspath=CFG_Get(smsdcfgfile, "smsd", "errorsmspath", false);
	if (Config->errorsmspath == NULL) Config->errorsmspath = Config->sentsmspath;
	WriteSMSDLog("SMS with errors moved to \"%s\"",Config->errorsmspath);

	Config->IncludeNumbers=CFG_FindLastSectionEntry(smsdcfgfile, "include_numbers", false);
	Config->ExcludeNumbers=CFG_FindLastSectionEntry(smsdcfgfile, "exclude_numbers", false);
	if (Config->IncludeNumbers != NULL) {
		WriteSMSDLog("Include numbers available");
	}
	if (Config->ExcludeNumbers != NULL) {
		if (Config->IncludeNumbers == NULL) {
			WriteSMSDLog("Exclude numbers available");
		} else {
			WriteSMSDLog("Exclude numbers available, but IGNORED");
		}
	}

	Config->retries = 0;
	Config->prevSMSID[0] = 0;
}

bool SMSD_CheckSecurity(GSM_SMSDConfig *Config)
{
	GSM_SecurityCode 	SecurityCode;
	GSM_Error		error;

	/* Need PIN ? */
	error=Phone->GetSecurityStatus(&s,&SecurityCode.Type);
	/* Unknown error */
	if (error != GE_NOTSUPPORTED && error != GE_NONE) {
		WriteSMSDLog("Error getting security status (%i)", error);
		return false;
	}
	/* No supported - do not check more */
	if (error == GE_NOTSUPPORTED) return true;
	/* If PIN, try to enter */
	switch (SecurityCode.Type) {
	case GSCT_Pin:
		WriteSMSDLog("Trying to enter PIN");
		strcpy(SecurityCode.Code,Config->PINCode);
		error=Phone->EnterSecurityCode(&s,SecurityCode);
		if (error == GE_SECURITYERROR) {
			GSM_Terminate_SMSD("ERROR: incorrect PIN", error, true, -1);
		}
		if (error != GE_NONE) {
			WriteSMSDLog("Error entering PIN (%i)", error);
			return false;
	  	}
		break;
	case GSCT_SecurityCode:
	case GSCT_Pin2:
	case GSCT_Puk:
	case GSCT_Puk2:
		GSM_Terminate_SMSD("ERROR: phone requires not supported code type", 0, true, -1);
	case GSCT_None:
		break;
	}
	return true;
}

bool SMSD_ReadDeleteSMS(GSM_SMSDConfig *Config, GSM_SMSDService *Service)
{
	bool			start,process;
	GSM_MultiSMSMessage 	sms;
	unsigned char 		buffer[100];
	GSM_Error		error=GE_NONE;
	CFG_Entry		*e;
	int			i;

	start=true;
	while (error == GE_NONE && !bshutdown) {
		sms.SMS[0].Folder=0x00;
		error=Phone->GetNextSMS(&s, &sms, start);
		switch (error) {
		case GE_EMPTY:
			break;
		case GE_NONE:
			/* Not Inbox SMS - exit */
			if (!sms.SMS[0].InboxFolder) break;
			process=true;
			DecodeUnicode(sms.SMS[0].Number,buffer);
			if (Config->IncludeNumbers != NULL) {
				e=Config->IncludeNumbers;
				process=false;
				while (1) {
					if (e == NULL) break;
					if (strcmp(buffer,e->value)==0) {
						process=true;
						break;
					}
					e = e->prev;
				}
			} else if (Config->ExcludeNumbers != NULL) {
				e=Config->ExcludeNumbers;
				process=true;
				while (1) {
					if (e == NULL) break;
					if (strcmp(buffer,e->value)==0) {
						process=false;
						break;
					}
					e = e->prev;
				}
			}
			if (process) {
	 			Service->SaveInboxSMS(sms, Config);
			} else {
				WriteSMSDLog("Excluded %s", buffer);
			}
			break;
		default:
	 		WriteSMSDLog("Error getting SMS (%i)", error);
			return false;
		}
		if (error == GE_NONE && sms.SMS[0].InboxFolder) {
			for (i=0;i<sms.Number;i++) {
				sms.SMS[i].Folder=0;
				error=Phone->DeleteSMS(&s,&sms.SMS[i]);
				switch (error) {
				case GE_NONE:
				case GE_EMPTY:
					break;
				default:
					WriteSMSDLog("Error deleting SMS (%i)", error);
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
	error=Phone->GetSMSStatus(&s,&SMSStatus);
	if (error != GE_NONE) {
		WriteSMSDLog("Error getting SMS status (%i)", error);
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

	if (error == GE_EMPTY) {
		/* No outbox sms - wait few seconds and escape */
		for (j=0;j<Config->commtimeout && !bshutdown;j++) {
			GSM_GetCurrentDateTime (&Date);
			i=Date.Second;
	 		while (i==Date.Second && !bshutdown) {
				my_sleep(10);
				GSM_GetCurrentDateTime(&Date);
			}
		}
		return true;
	}
	if (error != GE_NONE) {
		/* Unknown error - escape */
		WriteSMSDLog("Error in outbox on %s", Config->SMSID);
		Service->MoveSMS(Config->outboxpath, Config->errorsmspath, Config->SMSID, true);
		return false;
	}
	if (!bshutdown) {
		if (strcmp(Config->prevSMSID, Config->SMSID) == 0) {
			Config->retries++;
			if (Config->retries > MAX_RETRIES) {
				Config->retries = 0;
				strcpy(Config->prevSMSID, "");
				WriteSMSDLog("Moved to errorbox: %s", Config->SMSID);
				Service->MoveSMS(Config->outboxpath, Config->errorsmspath, Config->SMSID, true);
				return false;
			}
		} else {
			Config->retries = 0;
			strcpy(Config->prevSMSID, Config->SMSID);
		}
		for (i=0;i<sms.Number;i++) {
			if (strcmp(Config->deliveryreport, "no") != 0) sms.SMS[i].PDU = SMS_Status_Report;
			error=Phone->SendSMS(&s, &sms.SMS[i]);
			if (error!=GE_NONE) {
				WriteSMSDLog("Error sending SMS %s (%i): %s", Config->SMSID, error,print_error(error,s.di.df,s.msg));
				return false;
			}
			j=0;
			SendingSMSStatus = GE_TIMEOUT;
			while (!bshutdown) {
				GSM_GetCurrentDateTime (&Date);
				z=Date.Second;
				while (z==Date.Second) {
					my_sleep(10);
					GSM_GetCurrentDateTime(&Date);
					GSM_ReadDevice(&s,true);
					if (SendingSMSStatus != GE_TIMEOUT) break;
				}
				if (SendingSMSStatus != GE_TIMEOUT) break;
				j++;
				if (j>Config->sendtimeout) break;
			}
			if (SendingSMSStatus != GE_NONE) {
				WriteSMSDLog("Error getting send status of %s (%i): %s", Config->SMSID, SendingSMSStatus,print_error(SendingSMSStatus,s.di.df,s.msg));
				return false;
			}
			WriteSMSDLog("Transmitted %s (%s: %i) to %s", Config->SMSID, (i+1 == sms.Number?"total":"part"),i+1,DecodeUnicodeString(sms.SMS[0].Number));
		}
		strcpy(Config->prevSMSID, "");
		if (Service->MoveSMS(Config->outboxpath, Config->sentsmspath, Config->SMSID, false) != GE_NONE)
			Service->MoveSMS(Config->outboxpath, Config->errorsmspath, Config->SMSID, true);
	}
	return true;
}

void SMSDaemon(int argc, char *argv[])
{
	int                     errors = 255, initerrors=0;
	GSM_SMSDService		*Service;
	GSM_Error		error;
 	time_t                  time1;
	GSM_SMSDConfig		Config;

	if (!strcmp(argv[2],"FILES")) {
		Service = &SMSDFiles;
	} else {
		fprintf(stderr,"Unknown service type (\"%s\")\n",argv[2]);
		exit(-1);
	}

	SMSD_ReadConfig(argc, argv, &Config);

	error = Service->Init();
	if (error!=GE_NONE) {
		GSM_Terminate_SMSD("Stop GAMMU smsd (%i)", error, true, -1);
	}

	/* We do not want to monitor serial line forever -
	 * press Ctrl+C to stop the monitoring mode.
	 */
	signal(SIGINT, interrupted);
	signal(SIGTERM, interrupted);
	fprintf(stderr,"Press Ctrl+C to stop the program ...\n");

	time1 			= time(NULL);
	SendingSMSStatus 	= GE_UNKNOWN;

	/* Loop here indefinitely -
	 * allows you to see messages from GSM code in
	 * response to unknown messages etc.
	 * The loops ends after pressing the Ctrl+C.
	 */
	while (!bshutdown) {
		/* There were errors in communication - try to recover */
		if (errors > 2) {
			if (errors != 255) {
				WriteSMSDLog("Terminating communication (%i,%i)", error, errors);
				error=GSM_TerminateConnection(&s);
			}
			if (initerrors++ > 3) my_sleep(30000);
			WriteSMSDLog("Starting communication");
			error=GSM_InitConnection(&s,2);
			switch (error) {
			case GE_NONE:
				s.User.SendSMSStatus 	= SMSSendingSMSStatus;
				Phone			= s.Phone.Functions;
				errors 			= 0;
				/* Marcin Wiacek: FIXME. To check */
//				di 			= s.di;
				break;
			case GE_DEVICEOPENERROR:
				GSM_Terminate_SMSD("Can't open device (%i)", error, true, -1);
			default:
				WriteSMSDLog("Error at init connection (%i)", error);
				errors = 250;
			}
			continue;
		}
		if ((difftime(time(NULL), time1) >= Config.receivefrequency) || (SendingSMSStatus != GE_NONE)) {
	 		time1 = time(NULL);

			if (!SMSD_CheckSecurity(&Config))
			{
				errors++;
				initerrors++;
				continue;
			} else errors=0;

			initerrors = 0;

			if (!SMSD_CheckSMSStatus(&Config,Service))
			{
				errors++;
				continue;
			} else errors=0;
		}
		if (!SMSD_SendSMS(&Config,Service)) continue;
	}
	GSM_Terminate_SMSD("Stop GAMMU smsd", 0, false, 0);
}

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
