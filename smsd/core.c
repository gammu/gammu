/* (c) 2002-2004 by Marcin Wiacek and Joergen Thomsen */
/* (c) 2009 Michal Cihar */

#include <string.h>
#include <signal.h>
#include <time.h>
#include <assert.h>
#ifndef WIN32
#include <sys/types.h>
#include <sys/wait.h>
#endif
#include <gammu-config.h>
#ifdef HAVE_SYSLOG
#include <syslog.h>
#endif
#include <stdarg.h>
#include <stdlib.h>

#include <gammu-smsd.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_DUP_IO_H
#include <io.h>
#endif

#ifdef HAVE_SHM
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdlib.h>
#endif

#include <ctype.h>
#include <errno.h>

/* Some systems let waitpid(2) tell callers about stopped children. */
#if !defined (WCONTINUED)
#  define WCONTINUED 0
#endif
#if !defined (WIFCONTINUED)
#  define WIFCONTINUED(s)	(0)
#endif

#include "core.h"
#include "services/files.h"
#include "services/null.h"
#ifdef HAVE_MYSQL_MYSQL_H
#  include "services/mysql.h"
#endif
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
#  include "services/pgsql.h"
#endif
#ifdef LIBDBI_FOUND
#  include "services/dbi.h"
#endif

#ifdef HAVE_WINDOWS_EVENT_LOG
#include "log-event.h"
#endif

#include "../helper/string.h"

const char smsd_name[] = "gammu-smsd";

/**
 * Checks whether database schema version matches current one.
 */
GSM_Error SMSD_CheckDBVersion(GSM_SMSDConfig *Config, int version)
{
	SMSD_Log(DEBUG_NOTICE, Config, "Database structures version: %d, SMSD current version: %d", version, SMSD_DB_VERSION);

	if (version < SMSD_DB_VERSION) {
		SMSD_Log(DEBUG_ERROR, Config, "Database structure is from older Gammu version");
		SMSD_Log(DEBUG_INFO, Config, "Please update database, you can find SQL scripts in documentation");
		return ERR_UNKNOWN;
	}
	if (version > SMSD_DB_VERSION) {
		SMSD_Log(DEBUG_ERROR, Config, "Database structure is from newer Gammu version");
		SMSD_Log(DEBUG_INFO, Config, "Please update Gammu to use this database");
		return ERR_UNKNOWN;
	}
	return ERR_NONE;
}

/**
 * Signals SMSD to shutdown.
 */
GSM_Error SMSD_Shutdown(GSM_SMSDConfig *Config)
{
	if (!Config->running) {
		return ERR_NOTRUNNING;
	}
	Config->shutdown = TRUE;
	return ERR_NONE;
}

/**
 * Callback from libGammu on sending message.
 */
void SMSD_SendSMSStatusCallback (GSM_StateMachine *sm, int status, int mr, void *user_data)
{
	GSM_SMSDConfig *Config = (GSM_SMSDConfig *)user_data;

	SMSD_Log(DEBUG_NOTICE, Config, "SMS sent on device: \"%s\" status=%d, reference=%d",
			GSM_GetConfig(sm, -1)->Device,
			status,
			mr);
	/* Remember message reference */
	Config->TPMR = mr;
	/* Was message sent okay? */
	if (status == 0) {
		Config->SendingSMSStatus = ERR_NONE;
	} else {
		Config->SendingSMSStatus = ERR_UNKNOWN;
	}
}

/**
 * Closes logging output for SMSD.
 */
void SMSD_CloseLog(GSM_SMSDConfig *Config)
{
	switch (Config->log_type) {
#ifdef HAVE_WINDOWS_EVENT_LOG
		case SMSD_LOG_EVENTLOG:
			eventlog_close(Config->log_handle);
			Config->log_handle = NULL;
			break;
#endif
		case SMSD_LOG_FILE:
			if (Config->log_handle != NULL) {
				fclose(Config->log_handle);
				Config->log_handle = NULL;
			}
			break;
		default:
			break;
	}
	Config->log_type = SMSD_LOG_NONE;
}

/**
 * Logs current errno (or equivalent) details.
 */
void SMSD_LogErrno(GSM_SMSDConfig *Config, const char *message)
{
#ifdef WIN32
	char *lpMsgBuf;

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language */
		(LPTSTR) &lpMsgBuf,
		0,
		NULL
	);
	SMSD_Log(DEBUG_ERROR, Config, "%s, Error %d: %s\n", message, (int)GetLastError(), lpMsgBuf);

	LocalFree(lpMsgBuf);
#else
	SMSD_Log(DEBUG_ERROR, Config, "%s, Error %d: %s\n", message, errno, strerror(errno));
#endif
}

/**
 * Log Gammu error code together with text.
 */
void SMSD_LogError(SMSD_DebugLevel level, GSM_SMSDConfig *Config, const char *message, GSM_Error error)
{
	SMSD_Log(level, Config, "%s: %s (%s[%i])",
		message,
		GSM_ErrorString(error),
		GSM_ErrorName(error),
		error);
}

/**
 * Terminates SMSD, closing phone connection, closing log files and
 * possibly reporting error code.
 */
void SMSD_Terminate(GSM_SMSDConfig *Config, const char *msg, GSM_Error error, gboolean exitprogram, int rc)
{
	GSM_Error ret = ERR_NONE;

	/* Log error message */
	if (error != ERR_NONE && error != 0) {
		SMSD_LogError(DEBUG_ERROR, Config, msg, error);
	} else if (rc != 0) {
		SMSD_LogErrno(Config, msg);
	}

	/* Disconnect from phone */
	if (GSM_IsConnected(Config->gsm)) {
		SMSD_Log(DEBUG_INFO, Config, "Terminating communication...");
		ret = GSM_TerminateConnection(Config->gsm);
		if (ret != ERR_NONE) {
			printf("%s\n", GSM_ErrorString(error));
			/* Try again without checking errors */
			if (GSM_IsConnected(Config->gsm)) {
				SMSD_Log(DEBUG_INFO, Config, "Terminating communication for second time...");
				GSM_TerminateConnection(Config->gsm);
			}
		}
	}

	/* Should we terminate program? */
	if (exitprogram) {
		if (rc == 0) {
			Config->running = FALSE;
			SMSD_CloseLog(Config);
		}
		if (Config->exit_on_failure) {
			exit(rc);
		} else if (error != ERR_NONE) {
			Config->failure = error;
		}
	}
}

GSM_Error SMSD_Init(GSM_SMSDConfig *Config, GSM_SMSDService *Service) {
	GSM_Error error;

	if (Config->connected) return ERR_NONE;

	error = Service->Init(Config);
	if (error == ERR_NONE) {
		Config->connected = TRUE;
	}

	return error;
}

PRINTF_STYLE(3, 4)
void SMSD_Log(SMSD_DebugLevel level, GSM_SMSDConfig *Config, const char *format, ...)
{
	GSM_DateTime 	date_time;
	char 		Buffer[2000];
	va_list		argp;
#ifdef HAVE_SYSLOG
        int priority;
#endif

	va_start(argp, format);
	vsprintf(Buffer,format, argp);
	va_end(argp);

	switch (Config->log_type) {
		case SMSD_LOG_EVENTLOG:
#ifdef HAVE_WINDOWS_EVENT_LOG
			eventlog_log(Config->log_handle, level, Buffer);
#endif
			break;
		case SMSD_LOG_SYSLOG:
#ifdef HAVE_SYSLOG
			switch (level) {
				case -1:
					priority = LOG_ERR;
					break;
				case 0:
					priority = LOG_NOTICE;
					break;
				case 1:
					priority = LOG_INFO;
					break;
				default:
					priority = LOG_DEBUG;
					break;
			}
			syslog(priority, "%s", Buffer);
#endif
			break;
		case SMSD_LOG_FILE:
			if (level != DEBUG_ERROR &&
					level != DEBUG_INFO &&
					(level & Config->debug_level) == 0) {
				return;
			}

			GSM_GetCurrentDateTime(&date_time);

			if (Config->use_timestamps) {
				fprintf(Config->log_handle,"%s %4d/%02d/%02d %02d:%02d:%02d ",
					DayOfWeek(date_time.Year, date_time.Month, date_time.Day),
					date_time.Year, date_time.Month, date_time.Day,
					date_time.Hour, date_time.Minute, date_time.Second);
			}
#ifdef HAVE_GETPID
			fprintf(Config->log_handle, "%s[%ld]: ", Config->program_name, (long)getpid());
#else
			fprintf(Config->log_handle, "%s: ", Config->program_name);
#endif
			fprintf(Config->log_handle,"%s\n",Buffer);
			fflush(Config->log_handle);
			break;
		case SMSD_LOG_NONE:
			break;
	}

	if (Config->use_stderr && level == -1) {
#ifdef HAVE_GETPID
		fprintf(stderr, "%s[%ld]: ", Config->program_name, (long)getpid());
#else
		fprintf(stderr, "%s: ", Config->program_name);
#endif
		fprintf(stderr, "%s\n", Buffer);
	}
}

void SMSD_Log_Function(const char *text, void *data)
{
	GSM_SMSDConfig *Config = (GSM_SMSDConfig *)data;
	size_t pos;
	size_t newsize;

	if (strcmp("\n", text) == 0) {
		SMSD_Log(DEBUG_GAMMU, Config, "gammu: %s", Config->gammu_log_buffer);
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

GSM_SMSDConfig *SMSD_NewConfig(const char *name)
{
	GSM_SMSDConfig *Config;
	Config = (GSM_SMSDConfig *)malloc(sizeof(GSM_SMSDConfig));
	if (Config == NULL) return Config;

	Config->running = FALSE;
	Config->failure = ERR_NONE;
	Config->exit_on_failure = TRUE;
	Config->shutdown = FALSE;
	Config->gsm = NULL;
	Config->gammu_log_buffer = NULL;
	Config->gammu_log_buffer_size = 0;
	Config->logfilename = NULL;
	Config->smsdcfgfile = NULL;
	Config->log_handle = NULL;
	Config->log_type = SMSD_LOG_NONE;
	Config->debug_level = 0;
	Config->Service = NULL;
	if (name == NULL) {
		Config->program_name = smsd_name;
	} else {
		Config->program_name = name;
	}

	return Config;
}

/**
 * Returns SMSD service based on configuration.
 */
GSM_Error SMSGetService(GSM_SMSDConfig *Config, GSM_SMSDService **Service)
{
	if (Config->Service == NULL) {
		return ERR_UNCONFIGURED;
	}
	if (strcasecmp(Config->Service, "FILES") == 0) {
		SMSD_Log(DEBUG_NOTICE, Config, "Using FILES service");
		*Service = &SMSDFiles;
	} else if (strcasecmp(Config->Service, "NULL") == 0) {
		SMSD_Log(DEBUG_NOTICE, Config, "Using NULL service");
		*Service = &SMSDNull;
	} else if (strcasecmp(Config->Service, "DBI") == 0) {
#ifdef LIBDBI_FOUND
		SMSD_Log(DEBUG_NOTICE, Config, "Using DBI service");
		*Service = &SMSDDBI;
#else
		SMSD_Log(DEBUG_ERROR, Config, "DBI service was not compiled in!");
		return ERR_DISABLED;
#endif
	} else if (strcasecmp(Config->Service, "MYSQL") == 0) {
#ifdef HAVE_MYSQL_MYSQL_H
		SMSD_Log(DEBUG_NOTICE, Config, "Using MYSQL service");
		*Service = &SMSDMySQL;
#else
		SMSD_Log(DEBUG_ERROR, Config, "MYSQL service was not compiled in!");
		return ERR_DISABLED;
#endif
	} else if (strcasecmp(Config->Service, "PGSQL") == 0) {
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
		SMSD_Log(DEBUG_NOTICE, Config, "Using PGSQL service");
		*Service = &SMSDPgSQL;
#else
		SMSD_Log(DEBUG_ERROR, Config, "PGSQL service was not compiled in!");
		return ERR_DISABLED;
#endif
	} else {
		SMSD_Log(DEBUG_ERROR, Config, "Unknown SMSD service type: \"%s\"", Config->Service);
		return ERR_UNCONFIGURED;
	}
	return ERR_NONE;
}


void SMSD_FreeConfig(GSM_SMSDConfig *Config)
{
	GSM_SMSDService	*Service;
	GSM_Error error;

	error = SMSGetService(Config, &Service);
	if (error == ERR_NONE && Config->connected) {
		Service->Free(Config);
		Config->connected = FALSE;
	}

	SMSD_CloseLog(Config);

	GSM_StringArray_Free(&(Config->IncludeNumbersList));
	GSM_StringArray_Free(&(Config->ExcludeNumbersList));
	GSM_StringArray_Free(&(Config->IncludeSMSCList));
	GSM_StringArray_Free(&(Config->ExcludeSMSCList));

	free(Config->gammu_log_buffer);

	INI_Free(Config->smsdcfgfile);

	GSM_FreeStateMachine(Config->gsm);

	free(Config);
}

/**
 * Loads list of numbers from defined config file section.
 */
GSM_Error SMSD_LoadIniNumbersList(GSM_SMSDConfig *Config, GSM_StringArray *Array, const char *section)
{
	INI_Entry *e;

	for (e = INI_FindLastSectionEntry(Config->smsdcfgfile, section, FALSE); e != NULL; e = e->Prev) {
		if (!GSM_StringArray_Add(Array, e->EntryValue)) {
			return ERR_MOREMEMORY;
		}
	}

	return ERR_NONE;
}

/**
 * Loads lines from file defined by configuration key.
 */
GSM_Error SMSD_LoadNumbersFile(GSM_SMSDConfig *Config, GSM_StringArray *Array, const char *configkey)
{
	size_t len;
	char *listfilename;
	FILE *listfd;
	char buffer[GSM_MAX_NUMBER_LENGTH + 1];

	listfilename = INI_GetValue(Config->smsdcfgfile, "smsd", configkey, FALSE);
	if (listfilename != NULL) {
		listfd = fopen(listfilename, "r");
		if (listfd == NULL) {
			SMSD_LogErrno(Config, "Failed to open numbers file");
			return ERR_CANTOPENFILE;
		}
		while (fgets(buffer, sizeof(buffer) - 1, listfd)) {
			len = strlen(buffer);
			/* Remove trailing whitespace */
			while (len > 0 && isspace(buffer[len - 1])) {
				buffer[len - 1] = 0;
				len--;
			}
			/* Ignore empty lines */
			if (len == 0) continue;
			/* Add line to array */
			if (!GSM_StringArray_Add(Array, buffer)) {
				return ERR_MOREMEMORY;
			}
		}
		fclose(listfd);
	}
	return ERR_NONE;
}


GSM_Error SMSD_ReadConfig(const char *filename, GSM_SMSDConfig *Config, gboolean uselog)
{
	GSM_Config 		smsdcfg;
	GSM_Config 		*gammucfg;
	unsigned char		*str;
	static unsigned char	emptyPath[1] = "\0";
	GSM_Error		error;
	int			fd;
#ifdef HAVE_SHM
	char			fullpath[PATH_MAX + 1];
#endif
#ifdef WIN32
	size_t i;
	size_t len;
#endif

	memset(&smsdcfg, 0, sizeof(smsdcfg));

	Config->shutdown = FALSE;
	Config->running = FALSE;
	Config->connected = FALSE;
	Config->failure = ERR_NONE;
	Config->exit_on_failure = TRUE;
	Config->gsm = GSM_AllocStateMachine();
	if (Config->gsm == NULL) {
		fprintf(stderr, "Failed to allocate memory for state machine!\n");
		return ERR_MOREMEMORY;
	}
	Config->gammu_log_buffer = NULL;
	Config->gammu_log_buffer_size = 0;
	Config->logfilename = NULL;
	Config->smsdcfgfile = NULL;
	Config->use_timestamps = TRUE;
	Config->log_type = SMSD_LOG_NONE;
	Config->log_handle = NULL;
	Config->use_stderr = TRUE;

#ifdef HAVE_SHM
	/* Calculate key for shared memory */
	if (realpath(filename, fullpath) == NULL) {
		strncpy(fullpath, filename, PATH_MAX);
		fullpath[PATH_MAX] = 0;
	}
	Config->shm_key = ftok(fullpath, SMSD_SHM_KEY);
#endif
#ifdef WIN32
	len = sprintf(Config->map_key, "Gammu-smsd-%s", filename);
	/* Replace some possibly dangerous chars */
	for (i = 0; i < len; i++) {
		if (!isalpha(Config->map_key[i]) && !isdigit(Config->map_key[i])) {
			Config->map_key[i] = '_';
		}
	}
#endif

	error = INI_ReadFile(filename, FALSE, &Config->smsdcfgfile);
	if (Config->smsdcfgfile == NULL || error != ERR_NONE) {
		if (error == ERR_FILENOTSUPPORTED) {
			fprintf(stderr, "Could not parse config file \"%s\"\n",filename);
		} else {
			fprintf(stderr, "Can't find file \"%s\"\n",filename);
		}
		return ERR_CANTOPENFILE;
	}

	str = INI_GetValue(Config->smsdcfgfile, "smsd", "debuglevel", FALSE);
	if (str)
		Config->debug_level = atoi(str);
	else
		Config->debug_level = 0;

	Config->logfilename=INI_GetValue(Config->smsdcfgfile, "smsd", "logfile", FALSE);
	if (Config->logfilename != NULL) {
		if (!uselog) {
			Config->log_type = SMSD_LOG_FILE;
			Config->use_stderr = FALSE;
			fd = dup(1);
			if (fd < 0) return ERR_CANTOPENFILE;
			Config->log_handle = fdopen(fd, "a");
			Config->use_timestamps = FALSE;
#ifdef HAVE_WINDOWS_EVENT_LOG
		} else if (strcmp(Config->logfilename, "eventlog") == 0) {
			Config->log_type = SMSD_LOG_EVENTLOG;
			Config->log_handle = eventlog_init();
			Config->use_stderr = TRUE;
#endif
#ifdef HAVE_SYSLOG
		} else if (strcmp(Config->logfilename, "syslog") == 0) {
			Config->log_type = SMSD_LOG_SYSLOG;
			openlog(Config->program_name, LOG_PID, LOG_DAEMON);
			Config->use_stderr = TRUE;
#endif
		} else {
			Config->log_type = SMSD_LOG_FILE;
			if (strcmp(Config->logfilename, "stderr") == 0) {
				fd = dup(2);
				if (fd < 0) return ERR_CANTOPENFILE;
				Config->log_handle = fdopen(fd, "a");
				Config->use_stderr = FALSE;
			} else if (strcmp(Config->logfilename, "stdout") == 0) {
				fd = dup(1);
				if (fd < 0) return ERR_CANTOPENFILE;
				Config->log_handle = fdopen(fd, "a");
				Config->use_stderr = FALSE;
			} else {
				Config->log_handle = fopen(Config->logfilename, "a");
				Config->use_stderr = TRUE;
			}
			if (Config->log_handle == NULL) {
				fprintf(stderr, "Can't open log file \"%s\"\n", Config->logfilename);
				return ERR_CANTOPENFILE;
			}
			fprintf(stderr, "Log filename is \"%s\"\n",Config->logfilename);
		}
	}

	Config->Service = INI_GetValue(Config->smsdcfgfile, "smsd", "service", FALSE);
	if (Config->Service == NULL) {
		SMSD_Log(DEBUG_ERROR, Config, "No SMSD service configured, please set service to use in configuration file!");
		return ERR_NOSERVICE;
	}

	SMSD_Log(DEBUG_NOTICE, Config, "Configuring Gammu SMSD...");
#ifdef HAVE_SHM
	SMSD_Log(DEBUG_NOTICE, Config, "SHM token: 0x%llx (%lld)", (long long)Config->shm_key, (long long)Config->shm_key);
#endif

	/* Does our config file contain gammu section? */
	if (INI_FindLastSectionEntry(Config->smsdcfgfile, "gammu", FALSE) == NULL) {
 		SMSD_Log(DEBUG_ERROR, Config, "No gammu configuration found (no [gammu] section in SMSD config file)!");
		return ERR_UNCONFIGURED;
	}

	gammucfg = GSM_GetConfig(Config->gsm, 0);
	GSM_ReadConfig(Config->smsdcfgfile, gammucfg, 0);
	GSM_SetConfigNum(Config->gsm, 1);
	gammucfg->UseGlobalDebugFile = FALSE;

	/* Force debug level in Gammu */
	if ((DEBUG_GAMMU & Config->debug_level) != 0) {
		strcpy(gammucfg->DebugLevel, "textall");
		GSM_SetDebugLevel("textall", GSM_GetGlobalDebug());
	}

	Config->PINCode=INI_GetValue(Config->smsdcfgfile, "smsd", "PIN", FALSE);
	if (Config->PINCode == NULL) {
 		SMSD_Log(DEBUG_INFO, Config, "Warning: No PIN code in %s file",filename);
	} else {
		SMSD_Log(DEBUG_NOTICE, Config, "PIN code is \"%s\"",Config->PINCode);
	}

	Config->NetworkCode = INI_GetValue(Config->smsdcfgfile, "smsd", "NetworkCode", FALSE);
	if (Config->NetworkCode != NULL) {
		SMSD_Log(DEBUG_NOTICE, Config, "Network code is \"%s\"",Config->NetworkCode);
	}

	Config->PhoneCode = INI_GetValue(Config->smsdcfgfile, "smsd", "PhoneCode", FALSE);
	if (Config->PhoneCode != NULL) {
		SMSD_Log(DEBUG_NOTICE, Config, "Phone code is \"%s\"",Config->PhoneCode);
	}

	Config->commtimeout = INI_GetInt(Config->smsdcfgfile, "smsd", "commtimeout", 30);
	Config->deliveryreportdelay = INI_GetInt(Config->smsdcfgfile, "smsd", "deliveryreportdelay", 600);
	Config->sendtimeout = INI_GetInt(Config->smsdcfgfile, "smsd", "sendtimeout", 30);
	Config->receivefrequency = INI_GetInt(Config->smsdcfgfile, "smsd", "receivefrequency", 0);
	Config->statusfrequency = INI_GetInt(Config->smsdcfgfile, "smsd", "statusfrequency", 15);
	Config->loopsleep = INI_GetInt(Config->smsdcfgfile, "smsd", "loopsleep", 1);
	Config->checksecurity = INI_GetBool(Config->smsdcfgfile, "smsd", "checksecurity", TRUE);
	Config->checksignal = INI_GetBool(Config->smsdcfgfile, "smsd", "checksignal", TRUE);
	Config->checkbattery = INI_GetBool(Config->smsdcfgfile, "smsd", "checkbattery", TRUE);
	Config->resetfrequency = INI_GetInt(Config->smsdcfgfile, "smsd", "resetfrequency", 0);
	Config->multiparttimeout = INI_GetInt(Config->smsdcfgfile, "smsd", "multiparttimeout", 600);
	Config->maxretries = INI_GetInt(Config->smsdcfgfile, "smsd", "maxretries", 1);

	SMSD_Log(DEBUG_NOTICE, Config, "commtimeout=%i, sendtimeout=%i, receivefrequency=%i, resetfrequency=%i",
			Config->commtimeout, Config->sendtimeout, Config->receivefrequency, Config->resetfrequency);
	SMSD_Log(DEBUG_NOTICE, Config, "checks: security=%d, battery=%d, signal=%d",
			Config->checksecurity, Config->checkbattery, Config->checksignal);

	Config->skipsmscnumber = INI_GetValue(Config->smsdcfgfile, "smsd", "skipsmscnumber", FALSE);
	if (Config->skipsmscnumber == NULL) Config->skipsmscnumber="";

	Config->deliveryreport = INI_GetValue(Config->smsdcfgfile, "smsd", "deliveryreport", FALSE);
	if (Config->deliveryreport == NULL || (strcasecmp(Config->deliveryreport, "log") != 0 && strcasecmp(Config->deliveryreport, "sms") != 0)) {
		Config->deliveryreport = "no";
	}
	SMSD_Log(DEBUG_NOTICE, Config, "deliveryreport = %s", Config->deliveryreport);

	Config->PhoneID = INI_GetValue(Config->smsdcfgfile, "smsd", "phoneid", FALSE);
	if (Config->PhoneID == NULL) Config->PhoneID = "";
	SMSD_Log(DEBUG_NOTICE, Config, "phoneid = %s", Config->PhoneID);

	Config->RunOnReceive = INI_GetValue(Config->smsdcfgfile, "smsd", "runonreceive", FALSE);

	str = INI_GetValue(Config->smsdcfgfile, "smsd", "smsc", FALSE);
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
		Config->inboxpath=INI_GetValue(Config->smsdcfgfile, "smsd", "inboxpath", FALSE);
		if (Config->inboxpath == NULL) Config->inboxpath = emptyPath;

		Config->inboxformat=INI_GetValue(Config->smsdcfgfile, "smsd", "inboxformat", FALSE);
		if (Config->inboxformat == NULL ||
				(strcasecmp(Config->inboxformat, "detail") != 0 &&
				strcasecmp(Config->inboxformat, "standard") != 0 &&
				strcasecmp(Config->inboxformat, "unicode") != 0)) {
			Config->inboxformat = "standard";
		}
		SMSD_Log(DEBUG_NOTICE, Config, "Inbox is \"%s\" with format \"%s\"", Config->inboxpath, Config->inboxformat);


		Config->outboxpath=INI_GetValue(Config->smsdcfgfile, "smsd", "outboxpath", FALSE);
		if (Config->outboxpath == NULL) Config->outboxpath = emptyPath;

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
		if (Config->sentsmspath == NULL) Config->sentsmspath = Config->outboxpath;
		SMSD_Log(DEBUG_NOTICE, Config, "Sent SMS moved to \"%s\"",Config->sentsmspath);

		Config->errorsmspath=INI_GetValue(Config->smsdcfgfile, "smsd", "errorsmspath", FALSE);
		if (Config->errorsmspath == NULL) Config->errorsmspath = Config->sentsmspath;
		SMSD_Log(DEBUG_NOTICE, Config, "SMS with errors moved to \"%s\"",Config->errorsmspath);
	}

#if defined(HAVE_MYSQL_MYSQL_H) || defined(HAVE_POSTGRESQL_LIBPQ_FE_H) || defined(LIBDBI_FOUND)
	Config->user = INI_GetValue(Config->smsdcfgfile, "smsd", "user", FALSE);
	if (Config->user == NULL) Config->user="root";
	Config->password = INI_GetValue(Config->smsdcfgfile, "smsd", "password", FALSE);
	if (Config->password == NULL) Config->password="";
	Config->PC = INI_GetValue(Config->smsdcfgfile, "smsd", "pc", FALSE);
	if (Config->PC == NULL) Config->PC="localhost";
	Config->database = INI_GetValue(Config->smsdcfgfile, "smsd", "database", FALSE);
	if (Config->database == NULL) Config->database="sms";
#endif

#ifdef LIBDBI_FOUND
	if (!strcasecmp(Config->Service,"DBI")) {
		Config->driver = INI_GetValue(Config->smsdcfgfile, "smsd", "driver", FALSE);
		if (Config->driver == NULL) Config->driver="mysql";
		Config->dbdir = INI_GetValue(Config->smsdcfgfile, "smsd", "dbdir", FALSE);
		if (Config->dbdir == NULL) Config->dbdir="./";
		Config->driverspath = INI_GetValue(Config->smsdcfgfile, "smsd", "driverspath", FALSE);
		/* This one can be NULL */
	}
#endif

	/* Prepare lists */
	GSM_StringArray_New(&(Config->IncludeNumbersList));
	GSM_StringArray_New(&(Config->ExcludeNumbersList));
	GSM_StringArray_New(&(Config->IncludeSMSCList));
	GSM_StringArray_New(&(Config->ExcludeSMSCList));

	/* Process include section in config file */
	error = SMSD_LoadIniNumbersList(Config, &(Config->IncludeNumbersList), "include_numbers");
	if (error != ERR_NONE) return error;
	/* Process exclude section in config file */
	error = SMSD_LoadIniNumbersList(Config, &(Config->ExcludeNumbersList), "exclude_numbers");
	if (error != ERR_NONE) return error;

	/* Load include numbers from external file */
	error = SMSD_LoadNumbersFile(Config, &(Config->IncludeNumbersList), "includenumbersfile");
	if (error != ERR_NONE) return error;

	/* Load exclude numbers from external file */
	error = SMSD_LoadNumbersFile(Config, &(Config->ExcludeNumbersList), "excludenumbersfile");

	if (Config->IncludeNumbersList.used > 0) {
		SMSD_Log(DEBUG_NOTICE, Config, "Include numbers available");
	}
	if (Config->ExcludeNumbersList.used > 0) {
		if (Config->IncludeNumbersList.used == 0) {
			SMSD_Log(DEBUG_NOTICE, Config, "Exclude numbers available");
		} else {
			SMSD_Log(DEBUG_INFO, Config, "Exclude numbers available, but IGNORED");
		}
	}

	/* Process include section in config file */
	error = SMSD_LoadIniNumbersList(Config, &(Config->IncludeSMSCList), "include_smsc");
	if (error != ERR_NONE) return error;
	/* Process exclude section in config file */
	error = SMSD_LoadIniNumbersList(Config, &(Config->ExcludeSMSCList), "exclude_smsc");
	if (error != ERR_NONE) return error;

	/* Load include smsc from external file */
	error = SMSD_LoadNumbersFile(Config, &(Config->IncludeSMSCList), "includesmscfile");
	if (error != ERR_NONE) return error;

	/* Load exclude smsc from external file */
	error = SMSD_LoadNumbersFile(Config, &(Config->ExcludeSMSCList), "excludesmscfile");

	if (Config->IncludeSMSCList.used > 0) {
		SMSD_Log(DEBUG_NOTICE, Config, "Include smsc available");
	}
	if (Config->ExcludeSMSCList.used > 0) {
		if (Config->IncludeSMSCList.used == 0) {
			SMSD_Log(DEBUG_NOTICE, Config, "Exclude smsc available");
		} else {
			SMSD_Log(DEBUG_INFO, Config, "Exclude smsc available, but IGNORED");
		}
	}

	Config->retries 	  = 0;
	Config->prevSMSID[0] 	  = 0;
	Config->relativevalidity  = -1;
	Config->Status = NULL;
	Config->IncompleteMessageID = 0;
	Config->IncompleteMessageTime = 0;

	return ERR_NONE;
}

/**
 * Checks whether phone does not need to enter some PIN.
 */
gboolean SMSD_CheckSecurity(GSM_SMSDConfig *Config)
{
	GSM_SecurityCode SecurityCode;
	GSM_Error error;
	const char *code = NULL;

	/* Need PIN ? */
	error = GSM_GetSecurityStatus(Config->gsm, &SecurityCode.Type);

	/* No supported - do not check more */
	if (error == ERR_NOTSUPPORTED) {
		return TRUE;
	}

	/* Unknown error */
	if (error != ERR_NONE) {
		SMSD_LogError(DEBUG_ERROR, Config, "Error getting security status", error);
		return FALSE;
	}

	/* If PIN, try to enter */
	switch (SecurityCode.Type) {
		case SEC_None:
			return TRUE;
		case SEC_Pin:
			code = Config->PINCode;
			break;
		case SEC_Phone:
			code = Config->PhoneCode;
			break;
		case SEC_Network:
			code = Config->NetworkCode;
			break;
		case SEC_SecurityCode:
		case SEC_Pin2:
		case SEC_Puk:
		case SEC_Puk2:
			SMSD_Terminate(Config, "ERROR: phone requires not supported code type", ERR_UNKNOWN, TRUE, -1);
			return FALSE;
	}

	/* Check if the PIN was configured */
	if (code == NULL) {
		SMSD_Log(DEBUG_INFO, Config, "Warning: no code in config when phone might want one!");
		return FALSE;
	}

	/* Enter the PIN */
	SMSD_Log(DEBUG_NOTICE, Config, "Trying to enter code");
	strcpy(SecurityCode.Code, code);
	error = GSM_EnterSecurityCode(Config->gsm, SecurityCode);
	if (error == ERR_SECURITYERROR) {
		SMSD_Terminate(Config, "ERROR: incorrect PIN", error, TRUE, -1);
		return FALSE;
	}
	if (error != ERR_NONE) {
		SMSD_LogError(DEBUG_ERROR, Config, "Error entering PIN", error);
		return FALSE;
	}
	return TRUE;
}

/**
 * Prepares a command line for RunOnReceive command.
 */
char *SMSD_RunOnReceiveCommand(GSM_SMSDConfig *Config, const char *locations)
{
	char *result;

	assert(Config->RunOnReceive != NULL);

	if (locations == NULL) {
		return strdup(Config->RunOnReceive);
	}

	result = (char *)malloc(strlen(locations) + strlen(Config->RunOnReceive) + 20);
	assert(result != NULL);

	result[0] = 0;
	strcat(result, Config->RunOnReceive);
	strcat(result, " ");
	strcat(result, locations);
	return result;
}

#ifdef WIN32
#define setenv(var, value, force) SetEnvironmentVariable(var, value)
#endif

/**
 * Fills in environment with information about messages.
 */
void SMSD_RunOnReceiveEnvironment(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *locations)
{
	GSM_MultiPartSMSInfo SMSInfo;
	char buffer[100], name[100];
	int i;

	/* Raw message data */
	sprintf(buffer, "%d", sms->Number);
	setenv("SMS_MESSAGES", buffer, 1);
	for (i = 0; i < sms->Number; i++) {
		sprintf(buffer, "%d", sms->SMS[i].Class);
		sprintf(name, "SMS_%d_CLASS", i + 1);
		setenv(name, buffer, 1);
		sprintf(name, "SMS_%d_NUMBER", i + 1);
		setenv(name, DecodeUnicodeConsole(sms->SMS[i].Number), 1);
		if (sms->SMS[i].Coding != SMS_Coding_8bit) {
			sprintf(name, "SMS_%d_TEXT", i + 1);
			setenv(name, DecodeUnicodeConsole(sms->SMS[i].Text), 1);
		}
	}

	/* Decoded message data */
	if (GSM_DecodeMultiPartSMS(GSM_GetDebug(Config->gsm), &SMSInfo, sms, TRUE)) {
		sprintf(buffer, "%d", SMSInfo.EntriesNum);
		setenv("DECODED_PARTS", buffer, 1);
		for (i = 0; i < SMSInfo.EntriesNum; i++) {
			switch (SMSInfo.Entries[i].ID) {
				case SMS_ConcatenatedTextLong:
				case SMS_ConcatenatedAutoTextLong:
				case SMS_ConcatenatedTextLong16bit:
				case SMS_ConcatenatedAutoTextLong16bit:
				case SMS_NokiaVCARD21Long:
				case SMS_NokiaVCALENDAR10Long:
					sprintf(name, "DECODED_%d_TEXT", i);
					setenv(name, DecodeUnicodeConsole(SMSInfo.Entries[i].Buffer), 1);
					break;
				case SMS_MMSIndicatorLong:
					sprintf(name, "DECODED_%d_MMS_SENDER", i + 1);
					setenv(name, SMSInfo.Entries[i].MMSIndicator->Sender, 1);
					sprintf(name, "DECODED_%d_MMS_TITLE", i + 1);
					setenv(name, SMSInfo.Entries[i].MMSIndicator->Title, 1);
					sprintf(name, "DECODED_%d_MMS_ADDRESS", i + 1);
					setenv(name, SMSInfo.Entries[i].MMSIndicator->Address, 1);
					sprintf(name, "DECODED_%d_MMS_SIZE", i + 1);
					sprintf(buffer, "%ld", (long)SMSInfo.Entries[i].MMSIndicator->MessageSize);
					setenv(name, buffer, 1);
					break;
				default:
					/* We ignore others for now */
					break;
			}
		}
	} else {
		setenv("DECODED_PARTS", "0", 1);
	}
	GSM_FreeMultiPartSMSInfo(&SMSInfo);
}

#ifdef WIN32
gboolean SMSD_RunOnReceive(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *locations)
{
	BOOL ret;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char *cmdline;

	cmdline = SMSD_RunOnReceiveCommand(Config, locations);

	/* Prepare environment */
	SMSD_RunOnReceiveEnvironment(sms, Config, locations);

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	SMSD_Log(DEBUG_INFO, Config, "Starting run on receive: %s", cmdline);

	ret = CreateProcess(NULL,     /* No module name (use command line) */
			cmdline,	/* Command line */
			NULL,           /* Process handle not inheritable*/
			NULL,           /* Thread handle not inheritable*/
			FALSE,          /* Set handle inheritance to FALSE*/
			0,              /* No creation flags*/
			NULL,           /* Use parent's environment block*/
			NULL,           /* Use parent's starting directory */
			&si,            /* Pointer to STARTUPINFO structure*/
			&pi );           /* Pointer to PROCESS_INFORMATION structure*/
	free(cmdline);
	if (! ret) {
		SMSD_LogErrno(Config, "CreateProcess failed");
	} else {
		/* We don't need handles at all */
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	return ret;
}
#else

gboolean SMSD_RunOnReceive(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, char *locations)
{
	int pid;
	int i;
	pid_t w;
	int status;
	char *cmdline;

	pid = fork();

	if (pid == -1) {
		SMSD_LogErrno(Config, "Error spawning new process");
		return FALSE;
	}

	if (pid != 0) {
		/* We are the parent, wait for child */
		i = 0;
		do {
			w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
			if (w == -1) {
				SMSD_Log(DEBUG_INFO, Config, "Failed to wait for process");
				return FALSE;
			}

			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) == 0) {
					SMSD_Log(DEBUG_INFO, Config, "Process finished successfully");
				} else {
					SMSD_Log(DEBUG_ERROR, Config, "Process failed with exit status %d", WEXITSTATUS(status));
				}
				return (WEXITSTATUS(status) == 0);
			} else if (WIFSIGNALED(status)) {
				SMSD_Log(DEBUG_ERROR, Config, "Process killed by signal %d", WTERMSIG(status));
				return FALSE;
			} else if (WIFSTOPPED(status)) {
				SMSD_Log(DEBUG_INFO, Config, "Process stopped by signal %d", WSTOPSIG(status));
			} else if (WIFCONTINUED(status)) {
				SMSD_Log(DEBUG_INFO, Config, "Process continued");
			}
			usleep(100000);

			if (i++ > 1200) {
				SMSD_Log(DEBUG_INFO, Config, "Waited two minutes for child process, giving up");
				return TRUE;
			}
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));

		return TRUE;
	}

	/* we are the child */

	/* Prepare environment */
	SMSD_RunOnReceiveEnvironment(sms, Config, locations);

	/* Calculate command line */
	cmdline = SMSD_RunOnReceiveCommand(Config, locations);
	SMSD_Log(DEBUG_INFO, Config, "Starting run on receive: %s", cmdline);

	/* Close all file descriptors */
	for (i = 0; i < 255; i++) {
		close(i);
	}

	/* Run the program */
	status = system(cmdline);

	/* Propagate error code */
	if (WIFEXITED(status)) {
		exit(WEXITSTATUS(status));
	} else {
		exit(127);
	}
}
#endif

/**
 * Checks whether we are allowed to accept a message from number.
 */
gboolean SMSD_CheckRemoteNumber(GSM_SMSDConfig *Config, GSM_SMSDService *Service, const char *number)
{
	if (Config->IncludeNumbersList.used > 0) {
		if (GSM_StringArray_Find(&(Config->IncludeNumbersList), number)) {
			SMSD_Log(DEBUG_NOTICE, Config, "Number %s matched IncludeNumbers", number);
			return TRUE;
		}
		return FALSE;
	} else if (Config->ExcludeNumbersList.used > 0) {
		if (GSM_StringArray_Find(&(Config->ExcludeNumbersList), number)) {
			SMSD_Log(DEBUG_NOTICE, Config, "Number %s matched ExcludeNumbers", number);
			return FALSE;
		}
		return TRUE;
	}

	return TRUE;
}

/**
 * Checks whether we are allowed to accept a message from number.
 */
gboolean SMSD_CheckSMSCNumber(GSM_SMSDConfig *Config, GSM_SMSDService *Service, const char *number)
{
	if (Config->IncludeSMSCList.used > 0) {
		if (GSM_StringArray_Find(&(Config->IncludeSMSCList), number)) {
			SMSD_Log(DEBUG_NOTICE, Config, "Number %s matched IncludeSMSC", number);
			return TRUE;
		}
		return FALSE;
	} else if (Config->ExcludeSMSCList.used > 0) {
		if (GSM_StringArray_Find(&(Config->ExcludeSMSCList), number)) {
			SMSD_Log(DEBUG_NOTICE, Config, "Number %s matched ExcludeSMSC", number);
			return FALSE;
		}
		return TRUE;
	}

	return TRUE;
}

/**
 * Performs checks whether given message is valid to be received by SMSD.
 */
gboolean SMSD_ValidMessage(GSM_SMSDConfig *Config, GSM_SMSDService *Service, GSM_MultiSMSMessage *sms)
{
	char buffer[100];

	/* Not Inbox SMS - exit */
	if (!sms->SMS[0].InboxFolder) {
		return FALSE;
	}
	/* Check SMSC number if we want to handle it */
	DecodeUnicode(sms->SMS[0].SMSC.Number, buffer);
	if (!SMSD_CheckSMSCNumber(Config, Service, buffer)) {
		SMSD_Log(DEBUG_NOTICE, Config, "Message excluded because of SMSC: %s", buffer);
		return FALSE;
	}
	/* Check sender number if we want to handle it */
	DecodeUnicode(sms->SMS[0].Number, buffer);
	if (!SMSD_CheckRemoteNumber(Config, Service, buffer)) {
		SMSD_Log(DEBUG_NOTICE, Config, "Message excluded because of sender: %s", buffer);
		return FALSE;
	}
	/* Finally process the message */
	SMSD_Log(DEBUG_NOTICE, Config, "Received message from: %s", buffer);
	return TRUE;
}

/**
 * Does any processing is required for single message after it has been accepted.
 *
 * Stores message in the backend and executes RunOnReceive.
 */
GSM_Error SMSD_ProcessSMS(GSM_SMSDConfig *Config, GSM_SMSDService *Service, GSM_MultiSMSMessage *sms)
{
	GSM_Error error = ERR_NONE;
	char *locations = NULL;

	/* Increase message counter */
	Config->Status->Received += sms->Number;
	/* Send message to the backend */
	error = Service->SaveInboxSMS(sms, Config, &locations);
	/* RunOnReceive handling */
	if (Config->RunOnReceive != NULL && error == ERR_NONE) {
		SMSD_RunOnReceive(sms, Config, locations);
	}
	/* Free memory allocated by SaveInboxSMS */
	free(locations);
	return error;
}

/**
 * Reads message from phone, processes it and delete it from phone afterwards.
 *
 * It tries to link multipart messages together if possible.
 */
gboolean SMSD_ReadDeleteSMS(GSM_SMSDConfig *Config, GSM_SMSDService *Service)
{
	gboolean start;
	GSM_MultiSMSMessage sms;
	GSM_MultiSMSMessage *GetSMSData[GSM_PHONE_MAXSMSINFOLDER], *SortedSMS[GSM_PHONE_MAXSMSINFOLDER];
	GSM_Error error = ERR_NONE;
	int GetSMSNumber = 0;
	int i, j;

	/* Read messages from phone */
	start=TRUE;
	sms.Number = 0;
	sms.SMS[0].Location = 0;
	while (error == ERR_NONE && !Config->shutdown) {
		sms.SMS[0].Folder = 0;
		error = GSM_GetNextSMS(Config->gsm, &sms, start);
		switch (error) {
			case ERR_EMPTY:
				break;
			case ERR_NONE:
				if (SMSD_ValidMessage(Config, Service, &sms)) {
					GetSMSData[GetSMSNumber] = malloc(sizeof(GSM_MultiSMSMessage));

					if (GetSMSData[GetSMSNumber] == NULL) {
						SMSD_Log(DEBUG_ERROR, Config, "Failed to allocate memory");
						return FALSE;
					}
					GetSMSData[GetSMSNumber + 1] = NULL;
					memcpy(GetSMSData[GetSMSNumber], &sms, sizeof(GSM_MultiSMSMessage));
					GetSMSNumber++;
				}
				break;
			default:
				SMSD_LogError(DEBUG_INFO, Config, "Error getting SMS", error);
				return FALSE;
		}
		start = FALSE;
	}

	/* No messages to process */
	if (GetSMSNumber == 0) {
		return TRUE;
	}

	/* Link messages */
	error = GSM_LinkSMS(GSM_GetDebug(Config->gsm), GetSMSData, SortedSMS, TRUE);

	/* Free memory */
	i=0;
	while (GetSMSData[i] != NULL) {
		free(GetSMSData[i]);
		GetSMSData[i] = NULL;
		i++;
	}

	/* Process messages */
	i=0;

	for (i = 0; SortedSMS[i] != NULL; i++) {
		/* Check if we have all parts */
		if (SortedSMS[i]->SMS[0].UDH.Type != UDH_NoUDH) {
			if (SortedSMS[i]->SMS[0].UDH.AllParts != SortedSMS[i]->Number) {
				if (Config->IncompleteMessageTime != 0 && (Config->IncompleteMessageID == SortedSMS[i]->SMS[0].UDH.ID16bit || Config->IncompleteMessageID == SortedSMS[i]->SMS[0].UDH.ID8bit) && difftime(time(NULL), Config->IncompleteMessageTime) > Config->multiparttimeout) {
					SMSD_Log(DEBUG_INFO, Config, "Incomplete multipart message 0x%02X, processing after timeout",
						Config->IncompleteMessageID);
				} else {
					if (Config->IncompleteMessageTime == 0) {
						if (SortedSMS[i]->SMS[0].UDH.ID16bit != -1) {
							 Config->IncompleteMessageID = SortedSMS[i]->SMS[0].UDH.ID16bit;
						} else {
							 Config->IncompleteMessageID = SortedSMS[i]->SMS[0].UDH.ID8bit;
						}
						Config->IncompleteMessageTime = time(NULL);
					}
					SMSD_Log(DEBUG_INFO, Config, "Incomplete multipart message 0x%02X, waiting for other parts (waited %.0f seconds)",
						Config->IncompleteMessageID, difftime(time(NULL), Config->IncompleteMessageTime));
					goto cleanup;
				}
			}
		}

		/* Clean multipart wait flag */
		if (Config->IncompleteMessageID == SortedSMS[i]->SMS[0].UDH.ID16bit || Config->IncompleteMessageID == SortedSMS[i]->SMS[0].UDH.ID8bit) {
			Config->IncompleteMessageTime = 0;
		}

		/* Actually process the message */
		error = SMSD_ProcessSMS(Config, Service, SortedSMS[i]);
		if (error != ERR_NONE) {
			SMSD_LogError(DEBUG_INFO, Config, "Error processing SMS", error);
			return FALSE;
		}

		/* Delete processed messages */
		for (j = 0; j < SortedSMS[i]->Number; j++) {
			SortedSMS[i]->SMS[j].Folder = 0;
			error = GSM_DeleteSMS(Config->gsm, &SortedSMS[i]->SMS[j]);
			switch (error) {
				case ERR_NONE:
				case ERR_EMPTY:
					break;
				default:
					SMSD_LogError(DEBUG_INFO, Config, "Error deleting SMS", error);
					return FALSE;
			}
		}

cleanup:
		free(SortedSMS[i]);
		SortedSMS[i] = NULL;
	}
	return TRUE;
}

/**
 * Checks whether there are some messages to process and calls
 * SMSD_ReadDeleteSMS to process them.
 */
gboolean SMSD_CheckSMSStatus(GSM_SMSDConfig *Config,GSM_SMSDService *Service)
{
	GSM_SMSMemoryStatus	SMSStatus;
	GSM_Error		error;
	gboolean new_message = FALSE;
	GSM_MultiSMSMessage sms;

	/* Do we have any SMS in phone ? */

	/* First try SMS status */
	error = GSM_GetSMSStatus(Config->gsm,&SMSStatus);
	if (error == ERR_NONE) {
		new_message = (SMSStatus.SIMUsed + SMSStatus.PhoneUsed > 0);
	} else if (error == ERR_NOTSUPPORTED || error == ERR_NOTIMPLEMENTED) {
		/* Fallback to GetNext */
		sms.Number = 0;
		sms.SMS[0].Location = 0;
		sms.SMS[0].Folder = 0;
		error = GSM_GetNextSMS(Config->gsm, &sms, TRUE);
		new_message = (error == ERR_NONE);
	} else {
		SMSD_LogError(DEBUG_INFO, Config, "Error getting SMS status", error);
		return FALSE;
	}

	/* Yes. We have SMS in phone */
	if (new_message) {
		return SMSD_ReadDeleteSMS(Config,Service);
	}

	return TRUE;
}

/**
 * Reads status from phone to configuration.
 */
void SMSD_PhoneStatus(GSM_SMSDConfig *Config) {
	GSM_Error error;

	if (Config->checkbattery) {
		error = GSM_GetBatteryCharge(Config->gsm, &Config->Status->Charge);
	} else {
		error = ERR_UNKNOWN;
	}
	if (error != ERR_NONE) {
		memset(&(Config->Status->Charge), 0, sizeof(Config->Status->Charge));
	}
	if (Config->checksignal) {
		error = GSM_GetSignalQuality(Config->gsm, &Config->Status->Network);
	} else {
		error = ERR_UNKNOWN;
	}
	if (error != ERR_NONE) {
		memset(&(Config->Status->Network), 0, sizeof(Config->Status->Network));
	}
}

/**
 * Sends a sms message which is provided by the service backend.
 */
GSM_Error SMSD_SendSMS(GSM_SMSDConfig *Config, GSM_SMSDService *Service)
{
	GSM_MultiSMSMessage  	sms;
	GSM_DateTime         	Date;
	GSM_Error            	error;
	unsigned int         	j;
	int			i, z;

	/* Clean structure before use */
	for (i = 0; i < GSM_MAX_MULTI_SMS; i++) {
		GSM_SetDefaultSMSData(&sms.SMS[i]);
	}

	error = Service->FindOutboxSMS(&sms, Config, Config->SMSID);

	if (error == ERR_EMPTY || error == ERR_NOTSUPPORTED) {
		/* No outbox sms */
		return error;
	}
	if (error != ERR_NONE) {
		/* Unknown error - escape */
		SMSD_Log(DEBUG_INFO, Config, "Error in outbox on '%s'", Config->SMSID);
		for (i=0;i<sms.Number;i++) {
			Config->Status->Failed++;
			Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i+1, SMSD_SEND_ERROR, -1);
		}
		Service->MoveSMS(&sms,Config, Config->SMSID, TRUE,FALSE);
		return error;
	}

	if (Config->shutdown) {
		return ERR_NONE;
	}

	if (strcmp(Config->prevSMSID, Config->SMSID) == 0) {
		SMSD_Log(DEBUG_NOTICE, Config, "Same message as previous one: %s", Config->SMSID);
		Config->retries++;
		if (Config->retries > Config->maxretries) {
			Config->retries = 0;
			strcpy(Config->prevSMSID, "");
			SMSD_Log(DEBUG_INFO, Config, "Moved to errorbox: %s", Config->SMSID);
			for (i=0;i<sms.Number;i++) {
				Config->Status->Failed++;
				Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i+1, SMSD_SEND_ERROR, -1);
			}
			Service->MoveSMS(&sms,Config, Config->SMSID, TRUE,FALSE);
			return ERR_UNKNOWN;
		}
	} else {
		SMSD_Log(DEBUG_NOTICE, Config, "New messsage to send: %s", Config->SMSID);
		Config->retries = 0;
		strcpy(Config->prevSMSID, Config->SMSID);
	}
	for (i=0;i<sms.Number;i++) {
		if (sms.SMS[i].SMSC.Location == 1) {
			if (Config->SMSC.Location == 0) {
				Config->SMSC.Location = 1;
				error = GSM_GetSMSC(Config->gsm,&Config->SMSC);
				if (error!=ERR_NONE) {
					SMSD_Log(DEBUG_ERROR, Config, "Error getting SMSC from phone");
					return ERR_UNKNOWN;
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

		SMSD_PhoneStatus(Config);
		Config->TPMR = -1;
		Config->SendingSMSStatus = ERR_TIMEOUT;
		error = GSM_SendSMS(Config->gsm, &sms.SMS[i]);
		if (error != ERR_NONE) {
			SMSD_LogError(DEBUG_INFO, Config, "Error sending SMS", error);
			Config->TPMR = -1;
			goto failure_unsent;
		}
		j    = 0;
		while (!Config->shutdown) {
			/* Update timestamp for SMS in backend */
			Service->RefreshSendStatus(Config, Config->SMSID);
			/* Update timestamp for phone in backend */
			Service->RefreshPhoneStatus(Config);
			GSM_GetCurrentDateTime (&Date);
			z=Date.Second;
			while (z==Date.Second) {
				usleep(10000);
				GSM_GetCurrentDateTime(&Date);
				GSM_ReadDevice(Config->gsm,TRUE);
				if (Config->SendingSMSStatus != ERR_TIMEOUT) break;
			}
			if (Config->SendingSMSStatus != ERR_TIMEOUT) break;
			j++;
			if (j>Config->sendtimeout) break;
		}
		if (Config->SendingSMSStatus != ERR_NONE) {
			SMSD_LogError(DEBUG_INFO, Config, "Error getting send status of message", Config->SendingSMSStatus);
			goto failure_unsent;
		}
		Config->Status->Sent++;
		error = Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i+1, SMSD_SEND_OK, Config->TPMR);
		if (error!=ERR_NONE) {
			goto failure_sent;
		}
	}
	strcpy(Config->prevSMSID, "");
	if (Service->MoveSMS(&sms,Config, Config->SMSID, FALSE, TRUE) != ERR_NONE) {
		Service->MoveSMS(&sms,Config, Config->SMSID, TRUE, FALSE);
	}
	return ERR_NONE;
failure_unsent:
	Config->Status->Failed++;
	Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i + 1, SMSD_SEND_SENDING_ERROR, Config->TPMR);
	Service->MoveSMS(&sms,Config, Config->SMSID, TRUE, FALSE);
	return ERR_UNKNOWN;
failure_sent:
	if (Service->MoveSMS(&sms,Config, Config->SMSID, FALSE, TRUE) != ERR_NONE) {
		Service->MoveSMS(&sms,Config, Config->SMSID, TRUE, FALSE);
	}
	return ERR_UNKNOWN;
}

/**
 * Initializes shared memory segment, writable if asked for it.
 */
GSM_Error SMSD_InitSharedMemory(GSM_SMSDConfig *Config, gboolean writable)
{
#ifdef HAVE_SHM
	/* Allocate world redable SHM segment */
	Config->shm_handle = shmget(Config->shm_key, sizeof(GSM_SMSDStatus), writable ? (IPC_CREAT | S_IRWXU | S_IRGRP | S_IROTH) : 0);
	if (Config->shm_handle == -1) {
		SMSD_Terminate(Config, "Failed to allocate shared memory segment!", ERR_NONE, TRUE, -1);
		return ERR_UNKNOWN;
	}
	Config->Status = shmat(Config->shm_handle, NULL, 0);
	if (Config->Status == (void *) -1) {
		SMSD_Terminate(Config, "Failed to map shared memory segment!", ERR_NONE, TRUE, -1);
		return ERR_UNKNOWN;
	}
	if (!writable && Config->Status->Version != SMSD_SHM_VERSION) {
		shmdt(Config->Status);
		return ERR_WRONGCRC;
	}
#elif defined(WIN32)
	Config->map_handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, writable ? PAGE_READWRITE : PAGE_READONLY, 0, sizeof(GSM_SMSDStatus), Config->map_key);
	if (Config->map_handle == NULL) {
		if (writable) {
			SMSD_Terminate(Config, "Failed to allocate shared memory segment!", ERR_NONE, TRUE, -1);
			return ERR_UNKNOWN;
		} else {
			SMSD_LogErrno(Config, "Can not CreateFileMapping");
			return ERR_NOTRUNNING;
		}
	}
	Config->Status = MapViewOfFile(Config->map_handle, writable ? FILE_MAP_ALL_ACCESS : FILE_MAP_READ, 0, 0, sizeof(GSM_SMSDStatus));
	if (Config->Status == NULL) {
		if (writable) {
			SMSD_Terminate(Config, "Failed to map shared memory!", ERR_NONE, TRUE, -1);
			return ERR_UNKNOWN;
		} else {
			SMSD_LogErrno(Config, "Failet to map shared memory!");
			return ERR_NOTRUNNING;
		}
	}
#else
	if (writable) {
		return ERR_NOTSUPPORTED;
	}
	Config->Status = malloc(sizeof(GSM_SMSDStatus));
	if (Config->Status == NULL) {
		SMSD_Terminate(Config, "Failed to map shared memory segment!", ERR_NONE, TRUE, -1);
		return ERR_UNKNOWN;
	}
#endif
	/* Initial shared memory content */
	if (writable) {
		Config->Status->Version = SMSD_SHM_VERSION;
		strcpy(Config->Status->PhoneID, Config->PhoneID);
		sprintf(Config->Status->Client, "Gammu %s on %s compiler %s",
			VERSION,
			GetOS(),
			GetCompiler());
		memset(&Config->Status->Charge, 0, sizeof(GSM_BatteryCharge));
		memset(&Config->Status->Network, 0, sizeof(GSM_SignalQuality));
		Config->Status->Received = 0;
		Config->Status->Failed = 0;
		Config->Status->Sent = 0;
		Config->Status->IMEI[0] = 0;
	}
	return ERR_NONE;
}

/**
 * Frees shared memory segment, writable if asked for it.
 */
GSM_Error SMSD_FreeSharedMemory(GSM_SMSDConfig *Config, gboolean writable)
{
#ifdef HAVE_SHM
	shmdt(Config->Status);
	if (writable) {
		shmctl(Config->shm_handle, IPC_RMID, NULL);
	}
#elif defined(WIN32)
	UnmapViewOfFile(Config->Status);
	CloseHandle(Config->map_handle);
#else
	if (writable) {
		free(Config->Status);
	}
#endif
	Config->Status = NULL;
	return ERR_NONE;
}
/**
 * Main loop which takes care of connection to phone and processing of
 * messages.
 */
GSM_Error SMSD_MainLoop(GSM_SMSDConfig *Config, gboolean exit_on_failure, int max_failures)
{
	GSM_SMSDService		*Service;
	GSM_Error		error;
	int                     errors = -1, initerrors=0;
 	time_t			lastreceive = 0, lastreset = time(NULL), lastnothingsent = 0, laststatus = 0;
	int i;
	gboolean first_start = TRUE, force_reset = FALSE;

	Config->failure = ERR_NONE;
	Config->exit_on_failure = exit_on_failure;

	/* Get service */
	error = SMSGetService(Config, &Service);
	if (error!=ERR_NONE) {
		SMSD_Terminate(Config, "Failed to setup SMSD service", error, TRUE, -1);
		goto done;
	}

	/* Init service */
	error = SMSD_Init(Config, Service);
	if (error!=ERR_NONE) {
		SMSD_Terminate(Config, "Initialisation failed, stopping Gammu smsd", error, TRUE, -1);
		goto done;
	}

	/* Init shared memory */
	error = SMSD_InitSharedMemory(Config, TRUE);
	if (error != ERR_NONE) {
		goto done;
	}

	Config->running = TRUE;

	Config->SendingSMSStatus 	= ERR_UNKNOWN;

	while (!Config->shutdown) {
		/* There were errors in communication - try to recover */
		if (errors > 2 || first_start || force_reset) {
			/* Should we disconnect from phone? */
			if (GSM_IsConnected(Config->gsm)) {
				if (! force_reset) {
					SMSD_Log(DEBUG_INFO, Config, "Already hit %d errors", errors);
				}
				SMSD_LogError(DEBUG_INFO, Config, "Terminating communication", error);
				error = GSM_TerminateConnection(Config->gsm);
			}
			/* Did we reach limit for errors? */
			if (max_failures != 0 && initerrors > max_failures) {
				Config->failure = ERR_TIMEOUT;
				SMSD_Log(DEBUG_INFO, Config, "Reached maximal number of failures (%d), terminating", max_failures);
				break;
			}
			if (initerrors++ > 3) {
				SMSD_Log(DEBUG_INFO, Config, "Going to 30 seconds sleep because of too much connection errors");

				for (i = 0; i < 60; i++) {
					if (Config->shutdown)
						break;
					usleep(500000);
				}
			}
			SMSD_Log(DEBUG_INFO, Config, "Starting phone communication...");
			error=GSM_InitConnection_Log(Config->gsm, 2, SMSD_Log_Function, Config);
			switch (error) {
			case ERR_NONE:
				GSM_SetSendSMSStatusCallback(Config->gsm, SMSD_SendSMSStatusCallback, Config);
				/* On first start we need to initialize some variables */
				if (first_start) {
					if (GSM_GetIMEI(Config->gsm, Config->Status->IMEI) != ERR_NONE) {
						errors++;
					} else {
						error = Service->InitAfterConnect(Config);
						if (error!=ERR_NONE) {
							SMSD_Terminate(Config, "Post initialisation failed, stopping Gammu smsd", error, TRUE, -1);
							goto done_connected;
						}
						GSM_SetFastSMSSending(Config->gsm, TRUE);
					}
					first_start = FALSE;
				}
				errors = 0;

				if (initerrors > 3 || force_reset ) {
					error = GSM_Reset(Config->gsm, FALSE); /* soft reset */
					SMSD_LogError(DEBUG_INFO, Config, "Reset return code", error);
					lastreset = time(NULL);
					sleep(5);
					force_reset = FALSE;
				}
				break;
			case ERR_DEVICEOPENERROR:
				SMSD_Terminate(Config, "Can't open device",
						error, TRUE, -1);
				goto done;
			default:
				SMSD_LogError(DEBUG_INFO, Config, "Error at init connection", error);
				errors = 250;
				break;
			}
			continue;
		}

		/* Should we receive? */
		if ((difftime(time(NULL), lastreceive) >= Config->receivefrequency) || (Config->SendingSMSStatus != ERR_NONE)) {
	 		lastreceive = time(NULL);

			/* Do we need to check security? */
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

		}

		/* time for preventive reset */
		if (Config->resetfrequency > 0 && difftime(time(NULL), lastreset) >= Config->resetfrequency) {
			force_reset = TRUE;
			continue;
		}

		/* Send any queued messages */
		if (difftime(time(NULL), lastnothingsent) >= Config->commtimeout) {
			error = SMSD_SendSMS(Config, Service);
			if (error == ERR_EMPTY) {
				lastnothingsent = time(NULL);
			}
			/* We don't care about other errors here, they are handled in SMSD_SendSMS */
		}
		/* Refresh phone status in shared memory and in service */
		if ((Config->statusfrequency > 0) && (difftime(time(NULL), laststatus) >= Config->statusfrequency)) {
			SMSD_PhoneStatus(Config);
			laststatus = time(NULL);
		}
		Service->RefreshPhoneStatus(Config);

		/* Sleep some time before another loop */
		sleep(Config->loopsleep);
	}
	Service->Free(Config);

done_connected:
	/* Free shared memory */
	error = SMSD_FreeSharedMemory(Config, TRUE);
	if (error != ERR_NONE) {
		return error;
	}

	GSM_SetFastSMSSending(Config->gsm,FALSE);
done:
	SMSD_Terminate(Config, "Stopping Gammu smsd", ERR_NONE, FALSE, 0);
	return Config->failure;
}

/**
 * Function to inject message to service backend.
 */
GSM_Error SMSD_InjectSMS(GSM_SMSDConfig		*Config, GSM_MultiSMSMessage *sms, char *NewID)
{
	GSM_SMSDService		*Service;
	GSM_Error error;

	/* Get service object */
	error = SMSGetService(Config, &Service);
	if (error != ERR_NONE) return ERR_UNKNOWN;

	/* Initialize service */
	error = SMSD_Init(Config, Service);
	if (error != ERR_NONE) return ERR_UNKNOWN;

	/* Store message in outbox */
	error = Service->CreateOutboxSMS(sms, Config, NewID);
	return error;
}

/**
 * Returns current status of SMSD, either from shared memory segment or
 * from process memory if SMSD is running in same process.
 */
GSM_Error SMSD_GetStatus(GSM_SMSDConfig *Config, GSM_SMSDStatus *status)
{
	GSM_Error error;
	/* Check for local instance */
	if (Config->running) {
		memcpy(status, Config->Status, sizeof(GSM_SMSDStatus));
		return ERR_NONE;
	}

	/* Init shared memory */
	error = SMSD_InitSharedMemory(Config, FALSE);
	if (error != ERR_NONE) {
		return error;
	}

	/* Copy data from shared memory */
	memcpy(status, Config->Status, sizeof(GSM_SMSDStatus));

	/* Free shared memory */
	error = SMSD_FreeSharedMemory(Config, FALSE);
	if (error != ERR_NONE) {
		return error;
	}
	return ERR_NONE;
}

GSM_Error SMSD_NoneFunction(void)
{
	return ERR_NONE;
}

GSM_Error SMSD_EmptyFunction(void)
{
	return ERR_EMPTY;
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
