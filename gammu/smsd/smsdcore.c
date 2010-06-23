
#include <string.h>
#include <signal.h>
#include <stdarg.h>

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


void WriteSMSDLog(char *format, ...)
{
	GSM_DateTime date_time;

	char Buffer[2000];
	va_list	argp;
	int 	result;

	if (smsd_log_file != NULL) {
		va_start(argp, format);
		result = vsprintf(Buffer,GetMsg(s.msg,format),argp);
		va_end(argp);

		GSM_GetCurrentDateTime(&date_time);

		fprintf(smsd_log_file,"%s %4d/%02d/%02d %02d:%02d:%02d : %s\n",
			DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
			date_time.Year, date_time.Month, date_time.Day,
			date_time.Hour, date_time.Minute, date_time.Second,Buffer);
		fflush(smsd_log_file);
	}
}

void SMSDaemon(int argc, char *argv[])
{
	int 			errors = 255, i, j, z;
	GSM_SMSMemoryStatus	SMSStatus;
	GSM_MultiSMSMessage 	sms;
	bool			start,process;
	GSM_SMSDaemonService	*Service;
	GSM_SecurityCode 	SecurityCode;
	GSM_Error		error;
	CFG_Header		*smsdcfgfile = NULL;
	GSM_Config 		smsdcfg;	
	CFG_Entry		*IncludeNumbers,*ExcludeNumbers,*e;
	GSM_DateTime		Date;
	char			SMSID[200];
	int			commtimeout,sendtimeout;
	char			*inboxpath,*outboxpath,*sentsmspath,*errorsmspath,*inboxformat,*transmitformat;
	unsigned char	*logfilename,*PINCode,*buffer;
	char			emptyPath[1] = "";

	if (!strcmp(argv[2],"FILES")) {
		Service = &SMSDFiles;
	} else {
		fprintf(stderr,"Unknown service type (\"%s\")\n",argv[2]);
		exit(-1);
	}

	smsdcfgfile=CFG_ReadFile(argv[3], false);
	if (smsdcfgfile==NULL) {
		fprintf(stderr,"Can't find file \"%s\"\n",argv[3]);
		exit(-1);
	}

	/* Include Numbers used, because we don't want create new variable */
	IncludeNumbers=CFG_FindLastSectionEntry(smsdcfgfile, "gammu", false);
	if (IncludeNumbers) {
		CFG_ReadConfig(smsdcfgfile, &smsdcfg);
		memcpy(&s.Config,&smsdcfg,sizeof(GSM_Config));
	}

	PINCode=CFG_Get(smsdcfgfile, "smsd", "PIN", false);
	if (PINCode == NULL) {
		fprintf(stderr,"No PIN code in %s file\n",argv[3]);
		exit(-1);
	}
	fprintf(stderr,"PIN code is \"%s\"\n",PINCode);

	logfilename=CFG_Get(smsdcfgfile, "smsd", "logfile", false);
	if (logfilename != NULL) {
		smsd_log_file=fopen(logfilename,"ab");
		if (smsd_log_file == NULL) {
			fprintf(stderr,"Can't open file \"%s\"\n",logfilename);
			exit(-1);
		}
		fprintf(stderr,"Log filename is \"%s\"\n",logfilename);
	}
	commtimeout=atoi(CFG_Get(smsdcfgfile, "smsd", "commtimeout", false));
	sendtimeout=atoi(CFG_Get(smsdcfgfile, "smsd", "sendtimeout", false));

	inboxpath=CFG_Get(smsdcfgfile, "smsd", "inboxpath", false);
	if (inboxpath == NULL) inboxpath = emptyPath;
	fprintf(stderr,"Inbox is \"%s\"\n",inboxpath);
	
	inboxformat=CFG_Get(smsdcfgfile, "smsd", "inboxformat", false);
	if (inboxformat == NULL ||
		((strcmp(inboxformat, "detail") != 0) && (strcmp(inboxformat, "unicode") != 0)))
		inboxformat = "standard";
	fprintf(stderr,"Inbox format is \"%s\"\n",inboxformat);

	outboxpath=CFG_Get(smsdcfgfile, "smsd", "outboxpath", false);
	if (outboxpath == NULL) outboxpath = emptyPath;
	fprintf(stderr,"Outbox is \"%s\"\n",outboxpath);
	
	transmitformat=CFG_Get(smsdcfgfile, "smsd", "transmitformat", false);
	if (transmitformat == NULL ||
		((strcmp(transmitformat, "auto") != 0) && (strcmp(transmitformat, "unicode") != 0)))
		transmitformat = "7bit";
	fprintf(stderr,"Transmit format is \"%s\"\n",transmitformat);

	sentsmspath=CFG_Get(smsdcfgfile, "smsd", "sentsmspath", false);
	if (sentsmspath == NULL) sentsmspath = outboxpath;
	fprintf(stderr,"Sent SMS moved to \"%s\"\n",sentsmspath);
	
	errorsmspath=CFG_Get(smsdcfgfile, "smsd", "errorsmspath", false);
	if (errorsmspath == NULL) errorsmspath = sentsmspath;
	fprintf(stderr,"SMS with errors moved to \"%s\"\n",errorsmspath);

	IncludeNumbers=CFG_FindLastSectionEntry(smsdcfgfile, "include_numbers", false);
	ExcludeNumbers=CFG_FindLastSectionEntry(smsdcfgfile, "exclude_numbers", false);
	if (IncludeNumbers != NULL) fprintf(stderr,"Include numbers available\n");
	if (ExcludeNumbers != NULL) {
		if (IncludeNumbers == NULL) {
			fprintf(stderr,"Exclude numbers available\n");
		} else {
			fprintf(stderr,"Exclude numbers available, but IGNORED\n");
		}
	}

	Print_Error(Service->Init());

	/* We do not want to monitor serial line forever -
	 * press Ctrl+C to stop the monitoring mode.
	 */
	signal(SIGINT, interrupted);
	fprintf(stderr,"If you want break, press Ctrl+C...\n");

	s.User.SendSMSStatus = SMSSendingSMSStatus;

	/* Loop here indefinitely -
	 * allows you to see messages from GSM code in
	 * response to unknown messages etc.
	 * The loops ends after pressing the Ctrl+C.
	 */
	while (!bshutdown) {
		/* There were errors in communication - try to recover */
		if (errors > 2) {
			if (errors != 255) {
				WriteSMSDLog("Terminating communication (%i)", errors);
				error=GSM_TerminateConnection(&s);
			}
			WriteSMSDLog("Starting communication");
			error=GSM_InitConnection(&s,2);
			Phone=s.Phone.Functions;
			switch (error) {
			case GE_NONE:
				errors = 0;
				break;
			case GE_DEVICEOPENERROR:
				WriteSMSDLog("Can't open device");
				fprintf(stderr,"Can't open device\n");
				exit(-1);
			default:
				errors = 250;
			}
			continue;
		}
		/* Need PIN ? */
		error=Phone->GetSecurityStatus(&s,&SecurityCode.Type);
		if (error != GE_NONE) {
			errors++;
			continue;
		}
		switch (SecurityCode.Type) {
			case GSCT_Pin:
				WriteSMSDLog("Trying to enter PIN");
				strcpy(SecurityCode.Code,PINCode);
				error=Phone->EnterSecurityCode(&s,SecurityCode);
				if (error == GE_SECURITYERROR) {
					WriteSMSDLog("ERROR: incorrect PIN");
					fprintf(stderr,"ERROR: incorrect PIN\n");
					GSM_Terminate();
					if (smsd_log_file!=NULL) fclose(smsd_log_file);
					exit(-1);
				}
				if (error != GE_NONE) {
					errors++;
					continue;
				}
				break;
			case GSCT_SecurityCode:				
			case GSCT_Pin2:
			case GSCT_Puk:
			case GSCT_Puk2:
				WriteSMSDLog("ERROR: phone requires not supported code type");
				fprintf(stderr,"ERROR: phone requires not supported code type\n");
				GSM_Terminate();
				if (smsd_log_file!=NULL) fclose(smsd_log_file);
				exit(-1);
			case GSCT_None:
				break;
		}
		/* Do we have any SMS in phone ? */
		error=Phone->GetSMSStatus(&s,&SMSStatus);
		if (error != GE_NONE) {
			errors++;
			continue;
		}
		/* Yes. We have SMS in phone */
		if (SMSStatus.SIMUsed+SMSStatus.PhoneUsed != 0) {
			start=true;
			while (error == GE_NONE) {
				sms.SMS[0].Folder=0x00;
				error=Phone->GetNextSMSMessage(&s, &sms, start);
				switch (error) {
				case GE_EMPTY:
					break;
				case GE_NONE:
					process=false;
					if (sms.SMS[0].InboxFolder)
					{
						process=true;
						buffer = DecodeUnicodeString(sms.SMS[0].Number);
						if (IncludeNumbers != NULL) {
							e=IncludeNumbers;
							process=false;
							while (1) {
								if (e == NULL) break;
								if (strcmp(buffer,e->value)==0) {
									process=true;
									break;
								}
								e = e->prev;
							}
						} else if (ExcludeNumbers != NULL) {
							e=ExcludeNumbers;
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
						free(buffer);
					if (process) {
							error=Service->SaveInboxSMS(sms, inboxpath, inboxformat);
					}
					for (i=0;i<sms.Number;i++) {
								sms.SMS[i].Folder=0;
						error=Phone->DeleteSMS(&s,&sms.SMS[i]);
						switch (error) {
							case GE_NONE:
							case GE_EMPTY:
								break;
							default:
								errors++;
						}
					}
					}
					break;
				default:
					errors++;
					break;
				}
				start=false;
			}
		}
		if (Service->FindOutboxSMS(&sms, outboxpath, transmitformat, SMSID) != GE_NONE) {
		for (j=0;j<commtimeout;j++) {
			GSM_GetCurrentDateTime (&Date);
			i=Date.Second;
			while (i==Date.Second) {
				mili_sleep(10);
				GSM_GetCurrentDateTime(&Date);
			}
		}
		} else {
			for (i=0;i<sms.Number;i++) {
				SendingSMSStatus = GE_TIMEOUT;
				error=Phone->SendSMSMessage(&s, &sms.SMS[i]);
				if (error!=GE_NONE) {
					WriteSMSDLog("Error sending SMS (retrying) %s (%i): %s", SMSID, error,
									print_error(error,s.di.df,s.msg));
						SendingSMSStatus = GE_UNKNOWN; break;
				}
				j=0;
				while (!bshutdown) {
					GSM_GetCurrentDateTime (&Date);
					z=Date.Second;
					while (z==Date.Second) {
						mili_sleep(10);
						GSM_GetCurrentDateTime(&Date);
						error = GSM_ReadDevice(&s);
						if (error == 0) {SendingSMSStatus = GE_NONE;}
						if (SendingSMSStatus == GE_UNKNOWN) break;
						if (SendingSMSStatus == GE_NONE) break;
					}
					if (SendingSMSStatus == GE_UNKNOWN) break;
					if (SendingSMSStatus == GE_NONE) break;
					if (++j>=sendtimeout)
					{
						SendingSMSStatus = GE_UNKNOWN;
						break;
					}
				}				
				if (SendingSMSStatus == GE_UNKNOWN) {
					WriteSMSDLog("Error sending SMS %s (%i): %s", SMSID, error,
									print_error(error,s.di.df,s.msg));
					Service->MoveSMS(outboxpath, errorsmspath, SMSID, true);
					break;
				}
			}
			if (SendingSMSStatus == GE_UNKNOWN) continue;
			WriteSMSDLog("Transmitted %s (%i parts)", SMSID, sms.Number);
			if (Service->MoveSMS(outboxpath, sentsmspath, SMSID, false) != GE_NONE)
				Service->MoveSMS(outboxpath, errorsmspath, SMSID, true);
		}
	}
}
