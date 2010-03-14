
#include <string.h>
#include <signal.h>

#include "smsdcore.h"
#include "../gammu.h"
#include "s_files.h"
#include "../../common/misc/coding.h"

FILE *smsd_log_file = NULL;

void WriteSMSDLog(char *text)
{
	GSM_DateTime date_time;

	if (smsd_log_file != NULL) {
		GSM_GetCurrentDateTime(&date_time);

		fprintf(smsd_log_file,"%s %4d/%02d/%02d %02d:%02d:%02d : %s\n",
			DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
			date_time.Year, date_time.Month, date_time.Day,
			date_time.Hour, date_time.Minute, date_time.Second,text);
		fflush(smsd_log_file);
	}
}

void SMSDeaemon(int argc, char *argv[])
{
	int 			errors = 255, i;
	GSM_SMSMemoryStatus	SMSStatus;
	GSM_MultiSMSMessage 	sms;
	bool			start,process;
	GSM_SMSDeaemonService	*Service;
	GSM_SecurityCode 	SecurityCode;
	GSM_Error		error;
	CFG_Header		*smsdcfg = NULL;

	char			*logfilename;
	char			*PINCode;
	CFG_Entry		*IncludeNumbers,*ExcludeNumbers,*e;

	if (!strcmp(argv[2],"FILES")) {
		Service = &SMSDFiles;
	} else {
		fprintf(stderr,"Unknown service type (\"%s\")\n",argv[2]);
		exit(-1);
	}

	smsdcfg=CFG_ReadFile(argv[3], false);
	if (smsdcfg==NULL) {
		fprintf(stderr,"Can't find file \"%s\"\n",argv[3]);
		exit(-1);
	}

	PINCode=CFG_Get(smsdcfg, "smsd", "PIN", false);
	if (PINCode == NULL) {
		fprintf(stderr,"No PIN code in %s file\n",argv[3]);
		exit(-1);
	}
	fprintf(stderr,"PIN code is \"%s\"\n",PINCode);

	logfilename=CFG_Get(smsdcfg, "smsd", "logfile", false);
	if (logfilename != NULL) {
		smsd_log_file=fopen(logfilename,"wb");
		if (smsd_log_file == NULL) {
			fprintf(stderr,"Can't open file \"%s\"\n",logfilename);
			exit(-1);
		}
		fprintf(stderr,"Log filename is \"%s\"\n",logfilename);
	}

	IncludeNumbers=CFG_FindLastSectionEntry(smsdcfg, "include_numbers", false);
	ExcludeNumbers=CFG_FindLastSectionEntry(smsdcfg, "exclude_numbers", false);
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

	/* Loop here indefinitely -
	 * allows you to see messages from GSM code in
	 * response to unknown messages etc.
	 * The loops ends after pressing the Ctrl+C.
	 */
	while (!bshutdown) {
		/* There were errors in communication - try to recover */
		if (errors > 2) {
			if (errors != 255) {
				WriteSMSDLog("Terminating communication\n");
				error=GSM_TerminateConnection(&s);
			}
			WriteSMSDLog("Starting communication\n");
			error=GSM_InitConnection(&s,2);
			Phone=s.Phone.Functions;
			if (error==GE_NONE) errors = 0;
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
				strcpy(SecurityCode.Code,argv[3]);
				error=Phone->EnterSecurityCode(&s,SecurityCode);
				if (error == GE_SECURITYERROR) {
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
						if (IncludeNumbers != NULL) {
							e=IncludeNumbers;
							process=false;
							while (1) {
								if (e == NULL) break;
								if (strcmp(DecodeUnicodeString(sms.SMS[0].Number),e->value)==0) {
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
								if (strcmp(DecodeUnicodeString(sms.SMS[0].Number),e->value)==0) {
									process=false;
									break;
								}
								e = e->prev;
							}
						}
					}
					if (process) {
						error=Service->SaveInboxSMS(sms);
						Print_Error(error);
					}
					for (i=0;i<sms.Number;i++) {
						error=Phone->DeleteSMS(&s,&sms.SMS[i]);
						switch (error) {
							case GE_NONE:
							case GE_EMPTY:
								break;
							default:
								errors++;
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
	}
}
