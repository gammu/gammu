/* (c) 2002-2004 by Joergen Thomsen */
/* Copyright (c) 2009 - 2018 Michal Cihar <michal@cihar.com> */

#include <gammu.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#ifdef WIN32
#include <io.h>
#endif
#if defined HAVE_DIRENT_H && defined HAVE_SCANDIR && defined HAVE_ALPHASORT
#define HAVE_DIRBROWSING
#include <dirent.h>
#endif

#include "../core.h"

#include "../../libgammu/misc/string.h"

#ifndef PATH_MAX
#ifdef MAX_PATH
#define PATH_MAX (MAX_PATH)
#else
#define PATH_MAX (4069)
#endif
#endif

/**
 * Helper define to check error code from fwrite.
 */
#define chk_fwrite(data, size, count, file) \
	if (fwrite(data, size, count, file) != count) goto fail;


static void SMSDFiles_EscapeNumber(char *string)
{
	char *pos = string;
	while (*pos) {
		if (*pos == '*') {
			*pos = 'X';

		}
		pos++;
	}
}

static void SMSDFiles_DecodeNumber(char *string)
{
	char *pos = string;
	while (*pos) {
		if (*pos == 'X') {
			*pos = '*';

		}
		pos++;
	}
}

/* Save SMS from phone (called Inbox sms - it's in phone Inbox) somewhere */
static GSM_Error SMSDFiles_SaveInboxSMS(GSM_MultiSMSMessage * sms, GSM_SMSDConfig * Config, char **Locations)
{
	GSM_Error error = ERR_NONE;
	int i, j;
	unsigned char FileName[PATH_MAX], FullName[PATH_MAX], ext[4], buffer[64], buffer2[400];
	gboolean done;
	FILE *file;
	size_t locations_size = 0, locations_pos = 0;
#ifdef GSM_ENABLE_BACKUP
	GSM_SMS_Backup *backup;
#endif
	*Locations = NULL;

	j = 0;
	done = FALSE;
	for (i = 0; i < sms->Number && !done; i++) {
		strcpy(ext, "txt");
		if (sms->SMS[i].Coding == SMS_Coding_8bit)
			strcpy(ext, "bin");
		DecodeUnicode(sms->SMS[i].Number, buffer2);
		SMSDFiles_EscapeNumber(buffer2);
		/* we loop on yy for the first SMS assuming that if xxxx_yy_00.ext is absent,
		   any xxxx_yy_01,02, must be garbage, that can be overwritten */
		file = NULL;
		do {
			sprintf(FileName,
				"IN%02d%02d%02d_%02d%02d%02d_%02i_%s_%02i.%s",
				sms->SMS[i].DateTime.Year, sms->SMS[i].DateTime.Month, sms->SMS[i].DateTime.Day,
				sms->SMS[i].DateTime.Hour, sms->SMS[i].DateTime.Minute, sms->SMS[i].DateTime.Second, j, buffer2, i, ext);
			strcpy(FullName, Config->inboxpath);
			strcat(FullName, FileName);
			if (file) {
				fclose(file);
			}
			file = fopen(FullName, "r");
		} while ((i == 0) && file != NULL && (++j < 100));

		if (file) {
			fclose(file);
			file = NULL;
			if (i == 0) {
				SMSD_Log(DEBUG_ERROR, Config, "Cannot save %s. No available file names", FileName);
				return ERR_CANTOPENFILE;
			}
		}
		errno = 0;

		if ((sms->SMS[i].PDU == SMS_Status_Report) && strcasecmp(Config->deliveryreport, "log") == 0) {
			strcpy(buffer, DecodeUnicodeString(sms->SMS[i].Number));
			SMSD_Log(DEBUG_NOTICE, Config, "Delivery report: %s to %s, message reference 0x%02x",
				 DecodeUnicodeString(sms->SMS[i].Text), buffer, sms->SMS[i].MessageReference);
		} else {
			if (locations_pos + strlen(FileName) + 2 >= locations_size) {
				locations_size += strlen(FileName) + 30;
				*Locations = (char *)realloc(*Locations, locations_size);
				assert(*Locations != NULL);
				if (locations_pos == 0) {
					*Locations[0] = 0;
				}
			}
			strcat(*Locations, FileName);
			strcat(*Locations, " ");
			locations_pos += strlen(FileName) + 1;

			if (strcasecmp(Config->inboxformat, "detail") == 0) {
#ifndef GSM_ENABLE_BACKUP
				SMSD_Log(DEBUG_ERROR, Config, "Saving in detail format not compiled in!");

#else
				backup = malloc(sizeof(GSM_SMS_Backup));
				if (backup == NULL) {
					return ERR_MOREMEMORY;
				}

				for (j = 0; j < sms->Number; j++) {
					backup->SMS[j] = &sms->SMS[j];
				}
				backup->SMS[sms->Number] = NULL;
				error = GSM_AddSMSBackupFile(FullName, backup);
				done = TRUE;
				free(backup);
#endif
			} else {
				file = fopen(FullName, "wb");
				if (file == NULL) {
					SMSD_LogErrno(Config, "Cannot save file!");
					return ERR_CANTOPENFILE;
				}

				switch (sms->SMS[i].Coding) {
					case SMS_Coding_Unicode_No_Compression:
					case SMS_Coding_Default_No_Compression:
						DecodeUnicode(sms->SMS[i].Text, buffer2);
						if (strcasecmp(Config->inboxformat, "unicode") == 0) {
							buffer[0] = 0xFE;
							buffer[1] = 0xFF;
							chk_fwrite(buffer, 1, 2, file);
							chk_fwrite(sms->SMS[i].Text, 1, strlen(buffer2) * 2, file);
						} else {
							chk_fwrite(buffer2, 1, strlen(buffer2), file);
						}
						break;
					case SMS_Coding_8bit:
						chk_fwrite(sms->SMS[i].Text, 1, (size_t) sms->SMS[i].Length, file);
					default:
						break;
				}
				fclose(file);
				file = NULL;
			}

			if (error != ERR_NONE) {
				return error;
			}

			SMSD_Log(DEBUG_INFO, Config, "%s %s", (sms->SMS[i].PDU == SMS_Status_Report ? "Delivery report" : "Received"), FileName);
		}
	}
	return ERR_NONE;
fail:
	if (file) {
		fclose(file);
	}
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
static GSM_Error SMSDFiles_FindOutboxSMS(GSM_MultiSMSMessage * sms, GSM_SMSDConfig * Config, char *ID)
{
	GSM_MultiPartSMSInfo SMSInfo;
	GSM_WAPBookmark Bookmark;
	char FileName[100], FullName[PATH_MAX];
	unsigned char Buffer[(GSM_MAX_SMS_LENGTH * GSM_MAX_MULTI_SMS + 1) * 2];
	unsigned char Buffer2[(GSM_MAX_SMS_LENGTH * GSM_MAX_MULTI_SMS + 1) * 2];
	FILE *File;
	int i;
	size_t len, phlen;
	char *pos1, *pos2, *options = NULL;
	gboolean backup = FALSE;
#ifdef GSM_ENABLE_BACKUP
	GSM_SMS_Backup *smsbackup;
	GSM_Error error;
#endif
#ifdef WIN32
	struct _finddata_t c_file;
	intptr_t hFile;

	strcpy(FullName, Config->outboxpath);
	strcat(FullName, "OUT*.txt*");
	hFile = _findfirst(FullName, &c_file);
	if (hFile == -1) {
		strcpy(FullName, Config->outboxpath);
		strcat(FullName, "OUT*.smsbackup*");
		hFile = _findfirst(FullName, &c_file);
		backup = TRUE;
	}
	if (hFile == -1) {
		return ERR_EMPTY;
	} else {
		strcpy(FileName, c_file.name);
	}
	_findclose(hFile);
#elif defined(HAVE_DIRBROWSING)
	struct dirent **namelist = NULL;
	int cur_file, num_files;
	char *pos;

	strcpy(FullName, Config->outboxpath);

	FullName[strlen(Config->outboxpath) - 1] = '\0';

	num_files = scandir(FullName, &namelist, 0, alphasort);

	for (cur_file = 0; cur_file < num_files; cur_file++) {
		/* Hidden file or current/parent directory */
		if (namelist[cur_file]->d_name[0] == '.') {
			continue;
		}
		/* We care only about files starting with out */
		if (strncasecmp(namelist[cur_file]->d_name, "out", 3) != 0) {
			continue;
		}
		/* Check extension */
		pos = strrchr(namelist[cur_file]->d_name, '.');
		if (pos == NULL) {
			continue;
		}
		if (strncasecmp(pos, ".txt", 4) == 0) {
			/* We have found text file */
			backup = FALSE;
			break;
		}
		if (strncasecmp(pos, ".smsbackup", 10) == 0) {
			/* We have found a SMS backup file */
			backup = TRUE;
			break;
		}
	}
	/* Remember file name */
	if (cur_file < num_files) {
		strcpy(FileName, namelist[cur_file]->d_name);
	}
	/* Free scandir result */
	for (i = 0; i < num_files; i++) {
		free(namelist[i]);
	}
	free(namelist);
	namelist = NULL;
	/* Did we actually find something? */
	if (cur_file >= num_files) {
		return ERR_EMPTY;
	}
#else
	return ERR_NOTSUPPORTED;
#endif
	strcpy(FullName, Config->outboxpath);
	strcat(FullName, FileName);

	if (backup) {
#ifdef GSM_ENABLE_BACKUP
		smsbackup = malloc(sizeof(GSM_SMS_Backup));
		if (smsbackup == NULL) {
			return ERR_MOREMEMORY;
		}

		/* Remember ID */
		strcpy(ID, FileName);
		/* Load backup */
		GSM_ClearSMSBackup(smsbackup);
		error = GSM_ReadSMSBackupFile(FullName, smsbackup);
		if (error != ERR_NONE) {
			free(smsbackup);
			return error;
		}
		/* Copy it to our message */
		sms->Number = 0;
		for (i = 0; smsbackup->SMS[i] != NULL; i++) {
			sms->SMS[sms->Number++] = *(smsbackup->SMS[i]);
		}
		/* Free memory */
		GSM_FreeSMSBackup(smsbackup);
		free(smsbackup);

		/* Set delivery report flag */
		if (sms->SMS[0].PDU == SMS_Status_Report) {
			Config->currdeliveryreport = 1;
		} else {
			Config->currdeliveryreport = -1;
		}
#else
		SMSD_Log(DEBUG_ERROR, Config, "SMS backup loading disabled at compile time!");
		return ERR_DISABLED;

#endif
	} else {
		options = strrchr(FileName, '.') + 4;

		File = fopen(FullName, "rb");
		if (File == NULL) {
			return ERR_CANTOPENFILE;
		}
		len = fread(Buffer, 1, sizeof(Buffer) - 2, File);
		fclose(File);

		if ((len < 2) || (len >= 2 && ((Buffer[0] != 0xFF || Buffer[1] != 0xFE) && (Buffer[0] != 0xFE || Buffer[1] != 0xFF)))) {
			if (len > GSM_MAX_SMS_LENGTH * GSM_MAX_MULTI_SMS)
				len = GSM_MAX_SMS_LENGTH * GSM_MAX_MULTI_SMS;
			EncodeUnicode(Buffer2, Buffer, len);
			len = len * 2;
			memmove(Buffer, Buffer2, len);
			Buffer[len] = 0;
			Buffer[len + 1] = 0;
		} else {
			Buffer[len] = 0;
			Buffer[len + 1] = 0;
			/* Possibly convert byte order */
			ReadUnicodeFile(Buffer2, Buffer);
		}

		GSM_ClearMultiPartSMSInfo(&SMSInfo);
		sms->Number = 0;

		SMSInfo.ReplaceMessage = 0;
		SMSInfo.Entries[0].Buffer = Buffer2;
		SMSInfo.Class = -1;
		SMSInfo.EntriesNum = 1;
		Config->currdeliveryreport = -1;
		if (strchr(options, 'd'))
			Config->currdeliveryreport = 1;
		if (strchr(options, 'f'))
			SMSInfo.Class = 0;	/* flash SMS */

		if (strcasecmp(Config->transmitformat, "unicode") == 0) {
			SMSInfo.Entries[0].ID = SMS_ConcatenatedTextLong;
			SMSInfo.UnicodeCoding = TRUE;
		} else if (strcasecmp(Config->transmitformat, "7bit") == 0) {
			SMSInfo.Entries[0].ID = SMS_ConcatenatedTextLong;
			SMSInfo.UnicodeCoding = FALSE;
		} else {
			/* auto */
			SMSInfo.Entries[0].ID = SMS_ConcatenatedAutoTextLong;
		}

		if (strchr(options, 'b')) {	// WAP bookmark as title,URL
			SMSInfo.Entries[0].Buffer = NULL;
			SMSInfo.Entries[0].Bookmark = &Bookmark;
			SMSInfo.Entries[0].ID = SMS_NokiaWAPBookmarkLong;
			SMSInfo.Entries[0].Bookmark->Location = 0;
			pos2 = mywstrstr(Buffer2, "\0,");
			if (pos2 == NULL) {
				pos2 = Buffer2;
			} else {
				*pos2 = '\0';
				pos2++;
				*pos2 = '\0';
				pos2++;	// replace comma by zero
			}

			len = UnicodeLength(Buffer2);
			if (len > 50) {
				len = 50;
			}
			memmove(&SMSInfo.Entries[0].Bookmark->Title, Buffer2, len * 2);
			pos1 = &SMSInfo.Entries[0].Bookmark->Title[0] + len * 2;
			*pos1 = '\0';
			pos1++;
			*pos1 = '\0';

			len = UnicodeLength(pos2);
			if (len > 255) {
				len = 255;
			}
			memmove(&SMSInfo.Entries[0].Bookmark->Address, pos2, len * 2);
			pos1 = &SMSInfo.Entries[0].Bookmark->Address[0] + len * 2;
			*pos1 = '\0';
			pos1++;
			*pos1 = '\0';
		}

		GSM_EncodeMultiPartSMS(GSM_GetDebug(Config->gsm), &SMSInfo, sms);

		strcpy(ID, FileName);
		pos1 = FileName;
		for (i = 1; i <= 3 && pos1 != NULL; i++) {
			pos1 = strchr(++pos1, '_');
		}
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
			if (pos1 == NULL) {
				return ERR_BUG;
			}
			pos2 = strchr(++pos1, '_');
			if (pos2 == NULL) {
				return ERR_BUG;
			}
			phlen = strlen(pos1) - strlen(pos2);
		} else {
			/* something wrong */
			return ERR_UNKNOWN;
		}

		SMSDFiles_DecodeNumber(pos1);
		for (i = 0; i < sms->Number; i++) {
			EncodeUnicode(sms->SMS[i].Number, pos1, phlen);
		}
	}

	if (sms->Number != 0) {
		DecodeUnicode(sms->SMS[0].Number, Buffer);
		if (options != NULL && strchr(options, 'b')) {	// WAP bookmark as title,URL
			SMSD_Log(DEBUG_NOTICE, Config, "Found %i sms to \"%s\" with bookmark \"%s\" cod %i lgt %i udh: t %i l %i dlr: %i fls: %i",
				 sms->Number,
				 Buffer,
				 DecodeUnicodeString(SMSInfo.Entries[0].Bookmark->Address),
				 sms->SMS[0].Coding, sms->SMS[0].Length, sms->SMS[0].UDH.Type, sms->SMS[0].UDH.Length, Config->currdeliveryreport, SMSInfo.Class);
		} else {
			SMSD_Log(DEBUG_NOTICE, Config, "Found %i sms to \"%s\" with text \"%s\" cod %i lgt %i udh: t %i l %i dlr: %i fls: %i",
				 sms->Number,
				 Buffer,
				 DecodeUnicodeString(sms->SMS[0].Text),
				 sms->SMS[0].Coding, sms->SMS[0].Length, sms->SMS[0].UDH.Type, sms->SMS[0].UDH.Length, Config->currdeliveryreport, sms->SMS[0].Class);
		}
	} else {
		SMSD_Log(DEBUG_NOTICE, Config, "error: SMS-count = 0");
	}

	Config->retries = 0;

	return ERR_NONE;
}

/* After sending SMS is moved to Sent Items or Error Items. */
static GSM_Error SMSDFiles_MoveSMS(GSM_MultiSMSMessage * sms UNUSED, GSM_SMSDConfig * Config, char *ID, gboolean alwaysDelete, gboolean sent)
{
	FILE *oFile, *iFile;
	size_t ilen = 0, olen = 0;
	char Buffer[(GSM_MAX_SMS_LENGTH * GSM_MAX_MULTI_SMS + 1) * 2], ifilename[PATH_MAX], ofilename[PATH_MAX];
	const char *sourcepath, *destpath;
	GSM_Error error;

	sourcepath = Config->outboxpath;

	// Determine target based on status
	if (sent) {
		destpath = Config->sentsmspath;
	} else {
		destpath = Config->errorsmspath;
	}

	// Calculate source path
	strcpy(ifilename, sourcepath);
	strcat(ifilename, ID);
	// Calculate destination path
	strcpy(ofilename, destpath);
	strcat(ofilename, ID);

	// Do move only if paths are not same
	if (strcmp(ifilename, ofilename) != 0) {
		// First try rename
		if (rename(ifilename, ofilename) == 0) {
			SMSD_Log(DEBUG_INFO, Config, "Renamed %s to %s", ifilename, ofilename);
			return ERR_NONE;
		}

		// Move across devices
		if (errno != EXDEV) {
			SMSD_LogErrno(Config, "Can not move file");
			SMSD_Log(DEBUG_INFO, Config, "Could move %s to %s", ifilename, ofilename);
			return ERR_UNKNOWN;
		}

		// Read source file
		iFile = fopen(ifilename, "r");
		if (iFile == NULL) {
			SMSD_LogErrno(Config, "Can not open file");
			return ERR_CANTOPENFILE;
		}
		ilen = fread(Buffer, 1, sizeof(Buffer), iFile);
		fclose(iFile);

		// Write target file
		oFile = fopen(ofilename, "w");
		if (oFile == NULL) {
			SMSD_LogErrno(Config, "Can not open file");
			return ERR_CANTOPENFILE;
		}
		olen = fwrite(Buffer, 1, ilen, oFile);
		fclose(oFile);
	}

	// Did we write all data?
	error = ERR_NONE;
	if (ilen != olen) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to copy %s to %s", ifilename, ofilename);
		error = ERR_UNKNOWN;
	} else {
		SMSD_Log(DEBUG_INFO, Config, "Copied %s to %s", ifilename, ofilename);
	}

	// Remove source file
	if (error == ERR_NONE || alwaysDelete) {
		if (unlink(ifilename) != 0) {
			SMSD_LogErrno(Config, "Can not remove file");
			return ERR_UNKNOWN;
		}
	}

	return error;
}

/* Adds SMS to Outbox */
static GSM_Error SMSDFiles_CreateOutboxSMS(GSM_MultiSMSMessage * sms, GSM_SMSDConfig * Config, char *NewID)
{
	int i, j;
	int fd;
	unsigned char FileName[PATH_MAX], FullName[PATH_MAX], ext[17], buffer[64], buffer2[400];
	FILE *file = NULL;
	time_t rawtime;
	struct tm *timeinfo;

#ifdef GSM_ENABLE_BACKUP
	GSM_Error error;
	GSM_SMS_Backup *backup;
#endif

	j = 0;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	for (i = 0; i < sms->Number; i++) {
		if (strcasecmp(Config->outboxformat, "detail") == 0) {
			strcpy(ext, "smsbackup");
		} else {
			strcpy(ext, "txt");
		}
		DecodeUnicode(sms->SMS[i].Number, buffer2);
		SMSDFiles_EscapeNumber(buffer2);

		for (j = 0; j < 100; j++) {
			sprintf(FileName,
				"OUTC%04d%02d%02d_%02d%02d%02d_00_%s_sms%d.%s",
				1900 + timeinfo->tm_year, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, buffer2, j, ext);
			strcpy(FullName, Config->outboxpath);
			strcat(FullName, FileName);
			fd = open(FullName, O_CREAT | O_EXCL, 0644);
			if (fd >= 0) {
				close(fd);
				break;
			}
		}

		if (j >= 100) {
			if (i == 0) {
				SMSD_Log(DEBUG_ERROR, Config, "Cannot save %s. No available file names", FileName);
				return ERR_CANTOPENFILE;
			}
		}

		if (strcasecmp(Config->outboxformat, "detail") == 0) {
#ifndef GSM_ENABLE_BACKUP
			SMSD_Log(DEBUG_ERROR, Config, "Saving in detail format not compiled in!");

#else
			backup = malloc(sizeof(GSM_SMS_Backup));
			if (backup == NULL) {
				return ERR_MOREMEMORY;
			}

			for (j = 0; j < sms->Number; j++) {
				backup->SMS[j] = &sms->SMS[j];
			}
			backup->SMS[sms->Number] = NULL;
			error = GSM_AddSMSBackupFile(FullName, backup);

			free(backup);

			if (error != ERR_NONE) {
				return error;
			}
#endif
			/* Force leaving the loop */
			i = sms->Number;
		} else {
			file = fopen(FullName, "wb");
			if (file == NULL) {
				SMSD_LogErrno(Config, "Cannot save file!");
				return ERR_CANTOPENFILE;
			}
			switch (sms->SMS[i].Coding) {
				case SMS_Coding_Unicode_No_Compression:
				case SMS_Coding_Default_No_Compression:
					if (strcasecmp(Config->outboxformat, "unicode") == 0) {
						buffer[0] = 0xFE;
						buffer[1] = 0xFF;
						chk_fwrite(buffer, 1, 2, file);
						chk_fwrite(sms->SMS[i].Text, 1, UnicodeLength(sms->SMS[i].Text) * 2, file);
					} else {
						DecodeUnicode(sms->SMS[i].Text, buffer2);
						chk_fwrite(buffer2, 1, strlen(buffer2), file);
					}
					break;
				case SMS_Coding_8bit:
					chk_fwrite(sms->SMS[i].Text, 1, (size_t) sms->SMS[i].Length, file);
				default:
					break;
			}
			fclose(file);
			file = NULL;
		}

		SMSD_Log(DEBUG_INFO, Config, "Created outbox message %s", FileName);
	}

	if (NewID != NULL) {
		strcpy(NewID, FullName);
	}

	return ERR_NONE;
fail:
	if (file) {
		fclose(file);
	}
	return ERR_WRITING_FILE;
}

static GSM_Error SMSDFiles_AddSentSMSInfo(GSM_MultiSMSMessage * sms UNUSED, GSM_SMSDConfig * Config, char *ID UNUSED, int Part, GSM_SMSDSendingError err, int TPMR)
{
	FILE *file;
	GSM_File GSMFile;
	GSM_Error error;
	unsigned char FullPath[PATH_MAX];
	unsigned char *lineStart, *lineEnd;
	/* MessageReference TPMR maximum is "255" */
	char MessageReferenceBuffer[sizeof("MessageReference = \n") + 4];

	if (err == SMSD_SEND_OK) {
		SMSD_Log(DEBUG_INFO, Config, "Transmitted %s (%s: %i) to %s, message reference 0x%02x",
			 Config->SMSID, (Part == sms->Number ? "total" : "part"), Part, DecodeUnicodeString(sms->SMS[0].Number), TPMR);
	}

	strcpy(FullPath, Config->outboxpath);
	strcat(FullPath, Config->SMSID);

	// Read file content
	GSMFile.Buffer = NULL;
	GSMFile.Used = 0;
	error = GSM_ReadFile(FullPath, &GSMFile);

	if (error != ERR_NONE) {
		SMSD_Log(DEBUG_ERROR, Config, "AddSentSMSInfo: Failed to read file!");
		free(GSMFile.Buffer);
		return error;
	}

	// Allocate additional space for trailing zero
	GSMFile.Buffer = realloc(GSMFile.Buffer, GSMFile.Used + 1);

	if (GSMFile.Buffer == NULL) {
		return ERR_MOREMEMORY;
	}

	GSMFile.Buffer[GSMFile.Used] = '\0';

	lineStart = strstr(GSMFile.Buffer, "\nMessageReference = ");

	/* Message reference not found */
	if (lineStart == NULL) {
		free(GSMFile.Buffer);
		return ERR_NONE;
	}
	lineStart++;
	lineEnd = strchr(GSMFile.Buffer, '\n');
	/* End of buffer? */
	if (lineEnd == NULL) {
		lineEnd = GSMFile.Buffer + GSMFile.Used;
	}

	file = fopen(FullPath, "w");
	if (file == NULL) {
		SMSD_LogErrno(Config,  "AddSentSMSInfo: Failed to open file for writing");
		free(GSMFile.Buffer);
		return ERR_CANTOPENFILE;
	}

	chk_fwrite(GSMFile.Buffer, lineStart - GSMFile.Buffer, 1, file);

	snprintf(MessageReferenceBuffer, sizeof(MessageReferenceBuffer),
		 "MessageReference = %d\n", TPMR);


	chk_fwrite(MessageReferenceBuffer, strlen(MessageReferenceBuffer), 1, file);

	chk_fwrite(lineEnd + 1, (GSMFile.Buffer - lineEnd) + GSMFile.Used - 1, 1, file);

	fclose(file);

	free(GSMFile.Buffer);
	return ERR_NONE;
fail:
	SMSD_LogErrno(Config,  "AddSentSMSInfo: Failed to write");
	if (file) {
		fclose(file);
	}
	free(GSMFile.Buffer);
	return ERR_WRITING_FILE;
}

GSM_Error SMSD_Check_Dir(GSM_SMSDConfig *Config, const char *path, const char *name)
{
#ifndef WIN32
	struct stat s;

	if (stat(path, &s) < 0) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to stat \"%s\" (%s)", path, name);
		return ERR_FILENOTEXIST;
	}

	if (!S_ISDIR(s.st_mode)) {
		SMSD_Log(DEBUG_ERROR, Config, "The path \"%s\" (%s) is not a folder", path, name);
		return ERR_FILENOTEXIST;
	}
#else
	DWORD dwAttrib;

	dwAttrib = GetFileAttributes(path);
	if (dwAttrib == INVALID_FILE_ATTRIBUTES) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to stat \"%s\" (%s)", path, name);
		return ERR_FILENOTEXIST;
	}

	if ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) == 0) {
		SMSD_Log(DEBUG_ERROR, Config, "The path \"%s\" (%s) is not a folder", path, name);
		return ERR_FILENOTEXIST;
	}
#endif
	return ERR_NONE;
}

GSM_Error SMSDFiles_ReadConfiguration(GSM_SMSDConfig *Config)
{
	static unsigned char	emptyPath[1] = "\0";
	GSM_Error error;

	Config->inboxpath=INI_GetValue(Config->smsdcfgfile, "smsd", "inboxpath", FALSE);
	if (Config->inboxpath == NULL) {
		Config->inboxpath = emptyPath;
	} else {
		error = SMSD_Check_Dir(Config, Config->inboxpath, "inboxpath");
		if (error != ERR_NONE) {
			return error;
		}
	}

	Config->inboxformat=INI_GetValue(Config->smsdcfgfile, "smsd", "inboxformat", FALSE);
	if (Config->inboxformat == NULL ||
			(strcasecmp(Config->inboxformat, "detail") != 0 &&
			strcasecmp(Config->inboxformat, "standard") != 0 &&
			strcasecmp(Config->inboxformat, "unicode") != 0)) {
		Config->inboxformat = "standard";
	}
	SMSD_Log(DEBUG_NOTICE, Config, "Inbox is \"%s\" with format \"%s\"", Config->inboxpath, Config->inboxformat);


	Config->outboxpath=INI_GetValue(Config->smsdcfgfile, "smsd", "outboxpath", FALSE);
	if (Config->outboxpath == NULL) {
		Config->outboxpath = emptyPath;
	} else {
		error = SMSD_Check_Dir(Config, Config->outboxpath, "outboxpath");
		if (error != ERR_NONE) {
			return error;
		}
	}

	Config->transmitformat=INI_GetValue(Config->smsdcfgfile, "smsd", "transmitformat", FALSE);
	if (Config->transmitformat == NULL || (strcasecmp(Config->transmitformat, "auto") != 0 && strcasecmp(Config->transmitformat, "unicode") != 0)) {
		Config->transmitformat = "7bit";
	}
	Config->outboxformat=INI_GetValue(Config->smsdcfgfile, "smsd", "outboxformat", FALSE);
	if (Config->outboxformat == NULL ||
			(strcasecmp(Config->outboxformat, "detail") != 0 &&
			strcasecmp(Config->outboxformat, "standard") != 0 &&
			strcasecmp(Config->outboxformat, "unicode") != 0)) {
#ifdef GSM_ENABLE_BACKUP
		Config->outboxformat = "detail";
#else
		Config->outboxformat = "standard";
#endif
	}
	SMSD_Log(DEBUG_NOTICE, Config, "Outbox is \"%s\" with format \"%s\" and transmission format \"%s\"",
		Config->outboxpath,
		Config->outboxformat,
		Config->transmitformat);

	Config->sentsmspath=INI_GetValue(Config->smsdcfgfile, "smsd", "sentsmspath", FALSE);
	if (Config->sentsmspath == NULL) {
		Config->sentsmspath = Config->outboxpath;
	} else {
		error = SMSD_Check_Dir(Config, Config->sentsmspath, "sentsmspath");
		if (error != ERR_NONE) {
			return error;
		}
	}
	SMSD_Log(DEBUG_NOTICE, Config, "Sent SMS moved to \"%s\"",Config->sentsmspath);

	Config->errorsmspath=INI_GetValue(Config->smsdcfgfile, "smsd", "errorsmspath", FALSE);
	if (Config->errorsmspath == NULL) {
		Config->errorsmspath = Config->sentsmspath;
	} else {
		error = SMSD_Check_Dir(Config, Config->errorsmspath, "errorsmspath");
		if (error != ERR_NONE) {
			return error;
		}
	}
	SMSD_Log(DEBUG_NOTICE, Config, "SMS with errors moved to \"%s\"",Config->errorsmspath);

	return ERR_NONE;
}

GSM_SMSDService SMSDFiles = {
	NONEFUNCTION,		/* Init                 */
	NONEFUNCTION,		/* Free                 */
	NONEFUNCTION,		/* InitAfterConnect     */
	SMSDFiles_SaveInboxSMS,
	SMSDFiles_FindOutboxSMS,
	SMSDFiles_MoveSMS,
	SMSDFiles_CreateOutboxSMS,
	SMSDFiles_AddSentSMSInfo,
	NOTIMPLEMENTED,		/* UpdateRetries        */
	NOTIMPLEMENTED,		/* RefreshSendStatus    */
	NOTIMPLEMENTED,		/* RefreshPhoneStatus   */
	SMSDFiles_ReadConfiguration
};

/* How should editor handle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
