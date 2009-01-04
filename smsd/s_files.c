/* (c) 2002-2004 by Joergen Thomsen */

#include <gammu.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>

#ifdef WIN32
#  include <io.h>
#endif
#if defined HAVE_DIRENT_H && defined HAVE_SCANDIR && defined HAVE_ALPHASORT
#  define HAVE_DIRBROWSING
#  include <dirent.h>
#endif

#include "smsdcore.h"

/**
 * Helper define to check error code from fwrite.
 */
#define chk_fwrite(data, size, count, file) \
	if (fwrite(data, size, count, file) != count) goto fail;

/* Save SMS from phone (called Inbox sms - it's in phone Inbox) somewhere */
static GSM_Error SMSDFiles_SaveInboxSMS(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config)
{
	GSM_Error	error = ERR_NONE;
	int 		i,j;
	unsigned char 	FileName[100], FullName[400], ext[4], buffer[64],buffer2[400];
	bool		done;
	FILE 		*file;
#ifdef GSM_ENABLE_BACKUP
	GSM_SMS_Backup 	backup;
#endif

	j 	= 0;
	done 	= false;
	for (i=0;i<sms->Number && !done;i++) {
		strcpy(ext, "txt");
		if (sms->SMS[i].Coding == SMS_Coding_8bit) strcpy(ext, "bin");
		DecodeUnicode(sms->SMS[i].Number,buffer2);
		/* we loop on yy for the first SMS assuming that if xxxx_yy_00.ext is absent,
		   any xxxx_yy_01,02, must be garbage, that can be overwritten */
		file = NULL;
		do {
			sprintf(FileName,
				"IN%02d%02d%02d_%02d%02d%02d_%02i_%s_%02i.%s",
				sms->SMS[i].DateTime.Year, sms->SMS[i].DateTime.Month,  sms->SMS[i].DateTime.Day,
				sms->SMS[i].DateTime.Hour, sms->SMS[i].DateTime.Minute, sms->SMS[i].DateTime.Second,
				j, buffer2, i, ext);
			strcpy(FullName, Config->inboxpath);
			strcat(FullName, FileName);
			if (file) fclose(file);
			file = fopen(FullName, "r");
		} while ((i == 0) && file != NULL && (++j < 100));

		if (file) {
			fclose(file);
			if (i == 0) {
				WriteSMSDLog(Config, "Cannot save %s. No available file names", FileName);
				return ERR_CANTOPENFILE;
			}
		}
		errno = 0;

		if ((sms->SMS[i].PDU == SMS_Status_Report) && strncasecmp(Config->deliveryreport, "log", 3) == 0) {
			strcpy(buffer, DecodeUnicodeString(sms->SMS[i].Number));
			WriteSMSDLog(Config, "Delivery report: %s to %s", DecodeUnicodeString(sms->SMS[i].Text), buffer);
		} else {
			if (strcasecmp(Config->inboxformat, "detail") == 0) {
#ifndef GSM_ENABLE_BACKUP
				WriteSMSDLog(Config, "Saving in detail format not compiled in!");

#else
				for (j=0;j<sms->Number;j++) backup.SMS[j] = &sms->SMS[j];
				backup.SMS[sms->Number] = NULL;
				error = GSM_AddSMSBackupFile(FullName, &backup);
				done = true;
#endif
			} else {
				file = fopen(FullName, "wb");
				if (file != NULL) {
					switch (sms->SMS[i].Coding) {
					case SMS_Coding_Unicode_No_Compression:
	    				case SMS_Coding_Default_No_Compression:
					    DecodeUnicode(sms->SMS[i].Text,buffer2);
					    if (strcasecmp(Config->inboxformat, "unicode") == 0) {
						buffer[0] = 0xFE;
	    					buffer[1] = 0xFF;
						chk_fwrite(buffer,1,2,file);
						chk_fwrite(sms->SMS[i].Text,1,strlen(buffer2)*2,file);
					    } else {
						chk_fwrite(buffer2,1,strlen(buffer2),file);
					    }
					    break;
					case SMS_Coding_8bit:
					    chk_fwrite(sms->SMS[i].Text,1,(size_t)sms->SMS[i].Length,file);
					default:
					    break;
					}
					fclose(file);
				} else error = ERR_CANTOPENFILE;
			}
			if (error == ERR_NONE) {
				WriteSMSDLog(Config, "%s %s", (sms->SMS[i].PDU == SMS_Status_Report ? "Delivery report": "Received"), FileName);
			} else {
				WriteSMSDLog(Config, "Cannot save %s (%i)", FileName, errno);
				return ERR_CANTOPENFILE;
			}
		}
	}
	return ERR_NONE;
fail:
	return ERR_WRITING_FILE;
}

/* Find one multi SMS to sending and return it (or return ERR_EMPTY)
 * There is also set ID for SMS
 * File extension convention:
 * OUTxxxxx.txt : normal text SMS
 * Options appended to the extension applying to this SMS only:
 * d: delivery report requested
 * f: flash SMS
 * b: WAP bookmark as name,URL
 * e.g. OUTG20040620_193810_123_+4512345678_xpq.txtdf
 * is a flash text SMS requesting delivery reports
 */
static GSM_Error SMSDFiles_FindOutboxSMS(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *ID)
{
  	GSM_Error			error = ERR_NOTSUPPORTED;
  	GSM_MultiPartSMSInfo		SMSInfo;
	GSM_WAPBookmark			Bookmark;
 	char 			FileName[100],FullName[400];
	unsigned char			Buffer[(GSM_MAX_SMS_LENGTH*GSM_MAX_MULTI_SMS+1)*2];
 	unsigned char			Buffer2[(GSM_MAX_SMS_LENGTH*GSM_MAX_MULTI_SMS+1)*2];
  	FILE				*File;
 	int				i, len, phlen;
 	char				*pos1, *pos2, *options;
#ifdef WIN32
  	struct _finddata_t 		c_file;
  	intptr_t			hFile;

  	strcpy(FullName, Config->outboxpath);
  	strcat(FullName, "OUT*.txt*");
	hFile = _findfirst(FullName, &c_file);
  	if (hFile == -1 ) {
  		return ERR_EMPTY;
  	} else {
  		strcpy(FileName, c_file.name);
  	}
  	_findclose(hFile);
	error = ERR_NONE;
#elif defined(HAVE_DIRBROWSING)
  	struct 				dirent **namelist = NULL;
  	int 				l, m ,n;

  	strcpy(FullName, Config->outboxpath);
  	FullName[strlen(Config->outboxpath)-1] = '\0';
  	n = scandir(FullName, &namelist, 0, alphasort);
  	m = 0;
 	while ((m < n) && ((*(namelist[m]->d_name) == '.') || // directory and UNIX hidden file
 	                   !strncasecmp(namelist[m]->d_name,"out", 3) == 0 || // must start with 'out'
 	                   ((strlen(namelist[m]->d_name) >= 4) &&
 	                    strncasecmp(strrchr(namelist[m]->d_name, '.'),".txt",4) != 0
 	                   )
 	                  )
 	      ) m++;
  	if (m < n) strcpy(FileName,namelist[m]->d_name);
  	for (l=0; l < n; l++) free(namelist[l]);
  	free(namelist);
  	namelist = NULL;
 	if (m >= n) return ERR_EMPTY;
	error = ERR_NONE;
#endif
	if (error != ERR_NONE) return error;
	options = strrchr(FileName, '.') + 4;
  	strcpy(FullName, Config->outboxpath);
  	strcat(FullName, FileName);

  	File = fopen(FullName, "rb");
	if (File == NULL) {
		return ERR_CANTOPENFILE;
	}
 	len  = fread(Buffer, 1, sizeof(Buffer)-2, File);
  	fclose(File);

 	if ((len <  2) ||
            (len >= 2  && ((Buffer[0] != 0xFF || Buffer[1] != 0xFE) &&
	                   (Buffer[0] != 0xFE || Buffer[1] != 0xFF)))) {
 		if (len > GSM_MAX_SMS_LENGTH*GSM_MAX_MULTI_SMS) len = GSM_MAX_SMS_LENGTH*GSM_MAX_MULTI_SMS;
 		EncodeUnicode(Buffer2, Buffer, len);
 		len = len*2;
 		memmove(Buffer, Buffer2, len);
 	}

  	Buffer[len] 	= 0;
  	Buffer[len+1] 	= 0;
  	ReadUnicodeFile(Buffer2,Buffer);

    	GSM_ClearMultiPartSMSInfo(&SMSInfo);
    	sms->Number = 0;

  	SMSInfo.ReplaceMessage  	= 0;
  	SMSInfo.Entries[0].Buffer	= Buffer2;
  	SMSInfo.Class			= -1;
	SMSInfo.EntriesNum		= 1;
	Config->currdeliveryreport	= -1;
	if (strchr(options, 'd')) Config->currdeliveryreport	= 1;
	if (strchr(options, 'f')) SMSInfo.Class 		= 0; /* flash SMS */

 	if (strcasecmp(Config->transmitformat, "unicode") == 0) {
 		SMSInfo.Entries[0].ID = SMS_ConcatenatedTextLong;
 		SMSInfo.UnicodeCoding = true;
 	} else if (strcasecmp(Config->transmitformat, "7bit") == 0) {
 		SMSInfo.Entries[0].ID = SMS_ConcatenatedTextLong;
 		SMSInfo.UnicodeCoding = false;
 	} else {
		/* auto */
 		SMSInfo.Entries[0].ID = SMS_ConcatenatedAutoTextLong;
	}

	if (strchr(options, 'b')) { // WAP bookmark as title,URL
		SMSInfo.Entries[0].Buffer		= NULL;
		SMSInfo.Entries[0].Bookmark		= &Bookmark;
		SMSInfo.Entries[0].ID			= SMS_NokiaWAPBookmarkLong;
		SMSInfo.Entries[0].Bookmark->Location	= 0;
		pos2 = mywstrstr(Buffer2, "\0,");
		if (pos2 == NULL) {
			pos2 = Buffer2;
		} else {
			*pos2 = '\0'; pos2++; *pos2 = '\0'; pos2++; // replace comma by zero
		}

		len = UnicodeLength(Buffer2);
		if (len > 50) len = 50;
		memmove(&SMSInfo.Entries[0].Bookmark->Title, Buffer2, len * 2);
		pos1 = &SMSInfo.Entries[0].Bookmark->Title[0] + len * 2;
		*pos1 = '\0'; pos1++; *pos1 = '\0';

		len = UnicodeLength(pos2);
		if (len > 255) len = 255;
		memmove(&SMSInfo.Entries[0].Bookmark->Address, pos2, len * 2);
		pos1 = &SMSInfo.Entries[0].Bookmark->Address[0] + len * 2;
		*pos1 = '\0'; pos1++; *pos1 = '\0';
	}

  	GSM_EncodeMultiPartSMS(GSM_GetDebug(Config->gsm), &SMSInfo,sms);

 	pos1 = FileName;
	strcpy(ID,FileName);
 	for (i = 1; i <= 3 && pos1 != NULL ; i++) pos1 = strchr(++pos1, '_');
 	if (pos1 != NULL) {
		/* OUT<priority><date>_<time>_<serialno>_<phone number>_<anything>.txt */
 		pos2 = strchr(++pos1, '_');
 		if (pos2 != NULL) {
 			phlen = strlen(pos1) - strlen(pos2);
 		} else {
			/* something wrong */
 			return ERR_UNKNOWN;
		}
 	} else if (i == 2) {
		/* OUTxxxxxxx.txt or OUTxxxxxxx */
 		pos1 = &FileName[3];
		pos2 = strchr(pos1, '.');
 		if (pos2 == NULL) {
			phlen = strlen(pos1);
		} else {
 			phlen = strlen(pos1) - strlen(pos2);
		}
	} else if (i == 4) {
		/* OUT<priority>_<phone number>_<serialno>.txt */
 		pos1 = strchr(FileName, '_');
 		pos2 = strchr(++pos1, '_');
 		phlen = strlen(pos1) - strlen(pos2);
 	} else {
		/* something wrong */
		return ERR_UNKNOWN;
	}

 	for (len=0;len<sms->Number;len++) {
 		EncodeUnicode(sms->SMS[len].Number, pos1, phlen);
 	}

	if ((Config->debug_service & 1) != 0) {
		if (sms->Number != 0) {
			DecodeUnicode(sms->SMS[0].Number,Buffer);
			WriteSMSDLog(Config, "Found %i sms to \"%s\" with text \"%s\" cod %i lgt %i udh: t %i l %i dlr: %i fls: %i",
				sms->Number,
				Buffer,
				DecodeUnicodeString(sms->SMS[0].Text),
				sms->SMS[0].Coding,
				sms->SMS[0].Length,
				sms->SMS[0].UDH.Type,
				sms->SMS[0].UDH.Length,
				Config->currdeliveryreport,
				SMSInfo.Class);
		} else {
			WriteSMSDLog(Config, "error: SMS-count = 0");
		}
	}

  	return ERR_NONE;
}

/* After sending SMS is moved to Sent Items or Error Items. */
static GSM_Error SMSDFiles_MoveSMS(GSM_MultiSMSMessage *sms UNUSED,
		GSM_SMSDConfig *Config, char *ID,
		bool alwaysDelete, bool sent)
{
	FILE 	*oFile,*iFile;
	size_t	ilen = 0, olen = 0;
	char 	Buffer[(GSM_MAX_SMS_LENGTH*GSM_MAX_MULTI_SMS+1)*2],ifilename[400],ofilename[400];
	const char	*sourcepath, *destpath;

	sourcepath = Config->outboxpath;
	if (sent) {
	    destpath = Config->sentsmspath;
	} else {
	    destpath = Config->errorsmspath;
	}

	strcpy(ifilename, sourcepath);
	strcat(ifilename, ID);
	strcpy(ofilename, destpath);
	strcat(ofilename, ID);

	if (!strncasecmp(ifilename, ofilename, strlen(ofilename) == 0)) {
		iFile = fopen(ifilename, "r");
		if (iFile == NULL) {
			return ERR_CANTOPENFILE;
		}
		ilen = fread(Buffer, 1, sizeof(Buffer), iFile);
		fclose(iFile);
		oFile = fopen(ofilename, "w");
		if (oFile == NULL) {
			return ERR_CANTOPENFILE;
		}
		olen = fwrite(Buffer, 1, ilen, oFile);
		fclose(oFile);
	}
	if (ilen == olen) {
		if ((strcmp(ifilename, "/") == 0) || (remove(ifilename) != 0)) {
			WriteSMSDLog(Config, "Could not delete %s (%i)", ifilename, errno);
			return ERR_UNKNOWN;
		}
		return ERR_NONE;
	} else {
		WriteSMSDLog(Config, "Error copying SMS %s -> %s", ifilename, ofilename);
		if (alwaysDelete) {
			if ((strcmp(ifilename, "/") == 0) || (remove(ifilename) != 0))
				WriteSMSDLog(Config, "Could not delete %s (%i)", ifilename, errno);
		}
		return ERR_UNKNOWN;
	}
}

static GSM_Error SMSDFiles_AddSentSMSInfo(GSM_MultiSMSMessage *sms UNUSED,
		GSM_SMSDConfig *Config, char *ID UNUSED,
		int Part, GSM_SMSDSendingError err, int TPMR UNUSED)
{
	if (err == SMSD_SEND_OK) {
		WriteSMSDLog(Config, "Transmitted %s (%s: %i) to %s",
				Config->SMSID,
				(Part == sms->Number ? "total" : "part"),
				Part,
				DecodeUnicodeString(sms->SMS[0].Number));
	}

  	return ERR_NONE;
}

GSM_SMSDService SMSDFiles = {
	NONEFUNCTION,			/* Init 		*/
	NONEFUNCTION,			/* Free 		*/
	NONEFUNCTION,			/* InitAfterConnect 	*/
	SMSDFiles_SaveInboxSMS,
	SMSDFiles_FindOutboxSMS,
	SMSDFiles_MoveSMS,
	NOTSUPPORTED,			/* CreateOutboxSMS	*/
	SMSDFiles_AddSentSMSInfo,
	NOTSUPPORTED,			/* RefreshSendStatus	*/
	NOTSUPPORTED			/* RefreshPhoneStatus	*/
};

/* How should editor handle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
