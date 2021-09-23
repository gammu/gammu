/* Copyright (c) 2002-2004 by Marcin Wiacek and Joergen Thomsen */
/* Copyright (c) 2009 - 2018 Michal Cihar <michal@cihar.com> */

#include <string.h>
#include <signal.h>
#include <time.h>
#include <assert.h>
#include <math.h>
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
#include <fcntl.h>

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
#if defined(HAVE_MYSQL_MYSQL_H) || defined(HAVE_POSTGRESQL_LIBPQ_FE_H) || defined(LIBDBI_FOUND) || defined(ODBC_FOUND)
#include "services/sql.h"
#endif

#ifdef HAVE_WINDOWS_EVENT_LOG
#include "log-event.h"
#endif

#include "../libgammu/misc/string.h"
#include "../libgammu/protocol/protocol.h"
#include "../libgammu/gsmstate.h"

#ifndef PATH_MAX
#ifdef MAX_PATH
#define PATH_MAX (MAX_PATH)
#else
#define PATH_MAX (4069)
#endif
#endif

GSM_Error SMSD_ProcessSMSInfoCache(GSM_SMSDConfig *Config);

const char smsd_name[] = "gammu-smsd";

time_t lastRing=0;

/**
 * Checks whether database schema version matches current one.
 */
GSM_Error SMSD_CheckDBVersion(GSM_SMSDConfig *Config, int version)
{
	SMSD_Log(DEBUG_NOTICE, Config, "Database structures version: %d, SMSD current version: %d", version, SMSD_DB_VERSION);

	if (version < SMSD_DB_VERSION) {
		SMSD_Log(DEBUG_ERROR, Config, "Database structure is from older Gammu version");
		SMSD_Log(DEBUG_INFO, Config, "Please update database, you can find SQL scripts in documentation");
		return ERR_DB_VERSION;
	}
	if (version > SMSD_DB_VERSION) {
		SMSD_Log(DEBUG_ERROR, Config, "Database structure is from newer Gammu version");
		SMSD_Log(DEBUG_INFO, Config, "Please update Gammu to use this database");
		return ERR_DB_VERSION;
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
 * Interruptuptible sleep allowing to terminate SMSD.
 */
void SMSD_InterruptibleSleep(GSM_SMSDConfig *Config, int seconds)
{
	int i, loops;
	loops = seconds * 2;
	for (i = 0; i < loops; i++) {
		if (Config->shutdown) {
			break;
		}
		usleep(500000);
	}
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
	Config->StatusCode = status;
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
	} else {
		SMSD_LogError(DEBUG_INFO, Config, msg, error);
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
			Config->shutdown = TRUE;
			SMSD_CloseLog(Config);
		}
		if (Config->exit_on_failure) {
			exit(rc);
		} else if (error != ERR_NONE) {
			Config->failure = error;
		}
	}
}

GSM_Error SMSD_Init(GSM_SMSDConfig *Config) {
	GSM_Error error;

	if (Config->connected) return ERR_NONE;

	error = Config->Service->Init(Config);
	if (error == ERR_NONE) {
		Config->connected = TRUE;
	}

	return error;
}

PRINTF_STYLE(3, 4)
void SMSD_Log(SMSD_DebugLevel level, GSM_SMSDConfig *Config, const char *format, ...)
{
	GSM_DateTime 	date_time;
	char 		Buffer[65535];
	va_list		argp;
#ifdef HAVE_SYSLOG
	int priority;
#endif

	va_start(argp, format);
	vsprintf(Buffer,format, argp);
	va_end(argp);

	if (level != DEBUG_ERROR &&
			level != DEBUG_INFO &&
			(level & Config->debug_level) == 0) {
		return;
	}


	switch (Config->log_type) {
		case SMSD_LOG_EVENTLOG:
#ifdef HAVE_WINDOWS_EVENT_LOG
			eventlog_log(Config->log_handle, level, Buffer);
#endif
			break;
		case SMSD_LOG_SYSLOG:
#ifdef HAVE_SYSLOG
			switch (level) {
				case DEBUG_ERROR:
					priority = LOG_ERR;
					break;
				case DEBUG_INFO:
					priority = LOG_NOTICE;
					break;
				case DEBUG_NOTICE:
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

/**
 * Function used for passing log from libGammu to SMSD log.
 */
void SMSD_Log_Function(const char *text, void *data)
{
	GSM_SMSDConfig *Config = (GSM_SMSDConfig *)data;
	size_t pos;
	size_t newsize;

	/* Dump the buffer if we got \n */
	if (strcmp("\n", text) == 0) {
		SMSD_Log(DEBUG_GAMMU, Config, "gammu: %s", Config->gammu_log_buffer);
		Config->gammu_log_buffer[0] = 0;
		return;
	}

	/* Find out current position in the buffer */
	if (Config->gammu_log_buffer == NULL) {
		pos = 0;
	} else {
		pos = strlen(Config->gammu_log_buffer);
	}

	/* Calculate how much memory we need */
	newsize = pos + strlen(text) + 1;

	/* Reallocate buffer if needed */
	if (newsize > Config->gammu_log_buffer_size || Config->gammu_log_buffer == NULL) {
		newsize += 50;
		Config->gammu_log_buffer = realloc(Config->gammu_log_buffer, newsize);
		if (Config->gammu_log_buffer == NULL) {
			return;
		}
		Config->gammu_log_buffer_size = newsize;
	}

	/* Copy new text to the log buffer */
	strcpy(Config->gammu_log_buffer + pos, text);
}

/**
 * Allocates and clears new SMSD configuration structure.
 */
GSM_SMSDConfig *SMSD_NewConfig(const char *name)
{
	GSM_SMSDConfig *Config;
	int i;
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
	Config->RunOnFailure = NULL;
	Config->RunOnSent = NULL;
	Config->RunOnReceive = NULL;
	Config->RunOnIncomingCall = NULL;
	Config->smsdcfgfile = NULL;
	Config->log_handle = NULL;
	Config->log_type = SMSD_LOG_NONE;
	Config->debug_level = 0;
	Config->ServiceName = NULL;
	Config->Service = NULL;
	Config->IgnoredMessages = 0;
	Config->PhoneID = NULL;

#if defined(HAVE_MYSQL_MYSQL_H)
	Config->conn.my = NULL;
#endif
#if defined(LIBDBI_FOUND)
	Config->conn.dbi = NULL;
#endif
#if defined(HAVE_POSTGRESQL_LIBPQ_FE_H)
	Config->conn.pg = NULL;
#endif

	for (i = 0; i < GSM_MAX_MULTI_SMS; i++) {
		Config->SkipMessage[i] = FALSE;
	}

	/* Prepare lists */
	GSM_StringArray_New(&(Config->IncludeNumbersList));
	GSM_StringArray_New(&(Config->ExcludeNumbersList));
	GSM_StringArray_New(&(Config->IncludeSMSCList));
	GSM_StringArray_New(&(Config->ExcludeSMSCList));

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
GSM_Error SMSGetService(GSM_SMSDConfig *Config)
{
	if (Config->ServiceName == NULL) {
		SMSD_Log(DEBUG_ERROR, Config, "No SMSD service configured, please set service to use in configuration file!");
		return ERR_NOSERVICE;
	}
	if (strcasecmp(Config->ServiceName, "FILES") == 0) {
		SMSD_Log(DEBUG_NOTICE, Config, "Using FILES service");
		Config->Service = &SMSDFiles;
	} else if (strcasecmp(Config->ServiceName, "NULL") == 0) {
		SMSD_Log(DEBUG_NOTICE, Config, "Using NULL service");
		Config->Service = &SMSDNull;
	} else if (strcasecmp(Config->ServiceName, "SQL") == 0) {
#if defined(ODBC_FOUND) ||  defined(LIBDBI_FOUND) || defined(HAVE_MYSQL_MYSQL_H) || defined(HAVE_POSTGRESQL_LIBPQ_FE_H)
		SMSD_Log(DEBUG_NOTICE, Config, "Using SQL service");
		Config->Service = &SMSDSQL;
		Config->driver = INI_GetValue(Config->smsdcfgfile, "smsd", "driver", FALSE);
#else
		SMSD_Log(DEBUG_ERROR, Config, "SQL service was not compiled in!");
		return ERR_DISABLED;
#endif
	} else if(!strcasecmp("mysql", Config->ServiceName) || !strcasecmp("pgsql", Config->ServiceName) || !strcasecmp("dbi", Config->ServiceName)) {
		SMSD_Log(DEBUG_ERROR, Config, "%s service is deprecated. Please use SQL service with correct driver.", Config->ServiceName);
		if (strcasecmp(Config->ServiceName, "DBI") == 0) {
#ifdef LIBDBI_FOUND
			Config->Service = &SMSDSQL;
			Config->driver = INI_GetValue(Config->smsdcfgfile, "smsd", "driver", FALSE);
#else
			SMSD_Log(DEBUG_ERROR, Config, "DBI service was not compiled in!");
			return ERR_DISABLED;
#endif
		} else if (strcasecmp(Config->ServiceName, "MYSQL") == 0) {
#ifdef HAVE_MYSQL_MYSQL_H
			Config->Service = &SMSDSQL;
			Config->driver = "native_mysql";
#else
			SMSD_Log(DEBUG_ERROR, Config, "MYSQL service was not compiled in!");
			return ERR_DISABLED;
#endif
		} else if (strcasecmp(Config->ServiceName, "PGSQL") == 0) {
#ifdef HAVE_POSTGRESQL_LIBPQ_FE_H
			Config->Service = &SMSDSQL;
			Config->driver = "native_pgsql";
#else
			SMSD_Log(DEBUG_ERROR, Config, "PGSQL service was not compiled in!");
			return ERR_DISABLED;
#endif
		}
	} else {
		SMSD_Log(DEBUG_ERROR, Config, "Unknown SMSD service type: \"%s\"", Config->ServiceName);
		return ERR_UNCONFIGURED;
	}
	return ERR_NONE;
}

/**
 * Frees any data allocated under SMSD configuration.
 */
void SMSD_FreeConfig(GSM_SMSDConfig *Config)
{
	if (Config->Service != NULL && Config->connected) {
		Config->Service->Free(Config);
		Config->connected = FALSE;
		Config->Service = NULL;
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
				fclose(listfd);
				return ERR_MOREMEMORY;
			}
		}
		fclose(listfd);
	}
	return ERR_NONE;
}

/**
 * Configures SMSD logging.
 *
 * \param Config SMSD configuration data.
 * \param uselog Whether to log errors to configured log.
 */
GSM_Error SMSD_ConfigureLogging(GSM_SMSDConfig *Config, gboolean uselog)
{
	int fd;
#ifdef HAVE_SYSLOG
	int facility;
#endif

	/* No logging configured */
	if (Config->logfilename == NULL) {
		return ERR_NONE;
	}

	if (!uselog) {
		Config->log_type = SMSD_LOG_FILE;
		Config->use_stderr = FALSE;
		fd = dup(1);
		if (fd < 0) {
			return ERR_CANTOPENFILE;
		}
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
		if (Config->logfacility == NULL) {
			facility = LOG_DAEMON;
		} else if (!strcasecmp(Config->logfacility, "DAEMON")) {
			facility = LOG_DAEMON;
		} else if (!strcasecmp(Config->logfacility, "USER")) {
			facility = LOG_USER;
		} else if (!strcasecmp(Config->logfacility, "LOCAL0")) {
			facility = LOG_LOCAL0;
		} else if (!strcasecmp(Config->logfacility, "LOCAL1")) {
			facility = LOG_LOCAL1;
		} else if (!strcasecmp(Config->logfacility, "LOCAL2")) {
			facility = LOG_LOCAL2;
		} else if (!strcasecmp(Config->logfacility, "LOCAL3")) {
			facility = LOG_LOCAL3;
		} else if (!strcasecmp(Config->logfacility, "LOCAL4")) {
			facility = LOG_LOCAL4;
		} else if (!strcasecmp(Config->logfacility, "LOCAL5")) {
			facility = LOG_LOCAL5;
		} else if (!strcasecmp(Config->logfacility, "LOCAL6")) {
			facility = LOG_LOCAL6;
		} else if (!strcasecmp(Config->logfacility, "LOCAL7")) {
			facility = LOG_LOCAL7;
		} else {
			fprintf(stderr, "Invalid facility \"%s\"\n", Config->logfacility);
			facility = LOG_DAEMON;
		}
		Config->log_type = SMSD_LOG_SYSLOG;
		openlog(Config->program_name, LOG_PID, facility);
		Config->use_stderr = TRUE;
#endif
	} else {
		Config->log_type = SMSD_LOG_FILE;
		if (strcmp(Config->logfilename, "stderr") == 0) {
			fd = dup(2);
			if (fd < 0) {
				return ERR_CANTOPENFILE;
			}
			Config->log_handle = fdopen(fd, "a");
			Config->use_stderr = FALSE;
		} else if (strcmp(Config->logfilename, "stdout") == 0) {
			fd = dup(1);
			if (fd < 0) {
				return ERR_CANTOPENFILE;
			}
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
		fprintf(stdout, "Log filename is \"%s\"\n",Config->logfilename);
	}
	return ERR_NONE;
}

/**
 * Forces using global debug for smsd.
 */
void SMSD_EnableGlobalDebug(GSM_SMSDConfig *Config)
{
	/* Gammu library wide logging to our log */
	GSM_SetDebugFunction(SMSD_Log_Function, Config, GSM_GetGlobalDebug());
}

/**
 * Reads configuration file and feeds it's content into SMSD configuration structure.
 */
GSM_Error SMSD_ReadConfig(const char *filename, GSM_SMSDConfig *Config, gboolean uselog)
{
	GSM_Config 		smsdcfg;
	GSM_Config 		*gammucfg;
	unsigned char		*str;
	GSM_Error		error;
#ifdef HAVE_SHM
	char			fullpath[PATH_MAX + 1];
#endif
#ifdef WIN32
	size_t i;
	size_t len;
	char config_name[MAX_PATH];
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
	Config->logfacility = NULL;
	Config->smsdcfgfile = NULL;
	Config->use_timestamps = TRUE;
	Config->log_type = SMSD_LOG_NONE;
	Config->log_handle = NULL;
	Config->use_stderr = TRUE;

#ifdef HAVE_SHM
	/* Calculate key for shared memory */
	if (filename == NULL) {
		strcpy(fullpath, ":default:");
	} else if (realpath(filename, fullpath) == NULL) {
		strncpy(fullpath, filename, PATH_MAX);
		fullpath[PATH_MAX] = 0;
	}
	Config->shm_key = ftok(fullpath, SMSD_SHM_KEY);
	if (Config->shm_key < 0) {
		fprintf(stderr, "Failed to generate SHM key!\n");
		return FALSE;
	}
#endif
#ifdef WIN32
	if (filename == NULL) {
		strcpy(config_name, ":default:");
	} else if (GetFullPathName(filename, sizeof(config_name), config_name, NULL) == 0) {
		return FALSE;
	}

	len = sprintf(Config->map_key, "Gammu-smsd-%s", config_name);
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
	if (str) {
		Config->debug_level = atoi(str);
	} else {
		Config->debug_level = 0;
	}

	Config->logfilename = INI_GetValue(Config->smsdcfgfile, "smsd", "logfile", FALSE);
	Config->logfacility = INI_GetValue(Config->smsdcfgfile, "smsd", "logfacility", FALSE);

	error = SMSD_ConfigureLogging(Config, uselog);
	if (error != ERR_NONE) {
		return error;
	}

	Config->ServiceName = INI_GetValue(Config->smsdcfgfile, "smsd", "service", FALSE);

	/* Get service object */
	error = SMSGetService(Config);
	if (error != ERR_NONE) {
		return error;
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
	Config->receivefrequency = INI_GetInt(Config->smsdcfgfile, "smsd", "receivefrequency", 15);
	Config->statusfrequency = INI_GetInt(Config->smsdcfgfile, "smsd", "statusfrequency", 60);
	Config->loopsleep = INI_GetInt(Config->smsdcfgfile, "smsd", "loopsleep", 1);
	Config->checksecurity = INI_GetBool(Config->smsdcfgfile, "smsd", "checksecurity", TRUE);
	Config->hangupcalls = INI_GetBool(Config->smsdcfgfile, "smsd", "hangupcalls", FALSE);
	Config->checksignal = INI_GetBool(Config->smsdcfgfile, "smsd", "checksignal", TRUE);
	Config->checknetwork = INI_GetBool(Config->smsdcfgfile, "smsd", "checknetwork", TRUE);
	Config->checkbattery = INI_GetBool(Config->smsdcfgfile, "smsd", "checkbattery", TRUE);
	Config->enable_send = INI_GetBool(Config->smsdcfgfile, "smsd", "send", TRUE);
	Config->enable_receive = INI_GetBool(Config->smsdcfgfile, "smsd", "receive", TRUE);
	Config->resetfrequency = INI_GetInt(Config->smsdcfgfile, "smsd", "resetfrequency", 0);
	Config->hardresetfrequency = INI_GetInt(Config->smsdcfgfile, "smsd", "hardresetfrequency", 0);
	Config->multiparttimeout = INI_GetInt(Config->smsdcfgfile, "smsd", "multiparttimeout", 600);
	Config->maxretries = INI_GetInt(Config->smsdcfgfile, "smsd", "maxretries", 1);
	Config->retrytimeout = INI_GetInt(Config->smsdcfgfile, "smsd", "retrytimeout", 600);
	Config->backend_retries = INI_GetInt(Config->smsdcfgfile, "smsd", "backendretries", 10);
	if (Config->backend_retries < 1) {
		SMSD_Log(DEBUG_NOTICE, Config, "BackendRetries too low, forcing to 1");
		Config->backend_retries = 1;
	}

	SMSD_Log(DEBUG_NOTICE, Config, "CommTimeout=%i, SendTimeout=%i, ReceiveFrequency=%i, ResetFrequency=%i, HardResetFrequency=%i",
			Config->commtimeout, Config->sendtimeout, Config->receivefrequency, Config->resetfrequency, Config->hardresetfrequency);
	SMSD_Log(DEBUG_NOTICE, Config, "checks: CheckSecurity=%d, CheckBattery=%d, CheckSignal=%d, CheckNetwork=%d",
			Config->checksecurity, Config->checkbattery, Config->checksignal, Config->checknetwork);
	SMSD_Log(DEBUG_NOTICE, Config, "mode: Send=%d, Receive=%d",
			Config->enable_send, Config->enable_receive);

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
	Config->RunOnFailure = INI_GetValue(Config->smsdcfgfile, "smsd", "runonfailure", FALSE);
	Config->RunOnSent = INI_GetValue(Config->smsdcfgfile, "smsd", "runonsent", FALSE);
	Config->RunOnIncomingCall = INI_GetValue(Config->smsdcfgfile, "smsd", "runonincomingcall", FALSE);

	str = INI_GetValue(Config->smsdcfgfile, "smsd", "smsc", FALSE);
	if (str) {
		Config->SMSC.Location		= 0;
		Config->SMSC.DefaultNumber[0]	= 0;
		Config->SMSC.DefaultNumber[1]	= 0;
		Config->SMSC.Name[0]		= 0;
		Config->SMSC.Name[1]		= 0;
		Config->SMSC.Validity.Format	= SMS_Validity_NotAvailable;
		Config->SMSC.Format		= SMS_FORMAT_Text;
		EncodeUnicode(Config->SMSC.Number, str, strlen(str));
	} else {
		Config->SMSC.Location		= -1;
	}

	/* Clear cache */
	Config->SMSCCache.Location     = 0;

	/* Read service specific configuration */
	error = Config->Service->ReadConfiguration(Config);
	if (error != ERR_NONE) return error;

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
	if (error != ERR_NONE) return error;

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
	if (error != ERR_NONE) return error;

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
	Config->IncompleteMessageID = -1;
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
		SMSD_Log(DEBUG_ERROR, Config, "You might want to set CheckSecurity = 0 to avoid checking security status");
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
	error = GSM_EnterSecurityCode(Config->gsm, &SecurityCode);
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
 * Prepares a command line for RunOn() function to execute user command.
 */
char *SMSD_RunOnCommand(const char *locations, const char *command)
{
	char *result;
	size_t len;

	assert(command != NULL);

	if (locations == NULL) {
		result =  strdup(command);
		assert(result != NULL);
		return result;
	}

	len = strlen(locations) + strlen(command) + 4;
	result = (char *)malloc(len);
	assert(result != NULL);

	snprintf(result, len, "%s %s", command, locations);
	return result;
}

#ifdef WIN32
#define setenv(var, value, force) SetEnvironmentVariable(var, value)
#endif

/**
 * Fills in environment with information about messages.
 */
void SMSD_RunOnReceiveEnvironment(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, const char *locations)
{
	GSM_MultiPartSMSInfo SMSInfo;
	char buffer[100], name[100];
	int i;

	/* Raw message data */
	sprintf(buffer, "%d", sms->Number);
	setenv("SMS_MESSAGES", buffer, 1);

	if (Config->PhoneID) {
		setenv("PHONE_ID", Config->PhoneID, 1);
	}

	for (i = 0; i < sms->Number; i++) {
		sprintf(buffer, "%d", sms->SMS[i].Class);
		sprintf(name, "SMS_%d_CLASS", i + 1);
		setenv(name, buffer, 1);
		sprintf(buffer, "%d", sms->SMS[i].MessageReference);
		sprintf(name, "SMS_%d_REFERENCE", i + 1);
		setenv(name, buffer, 1);
		sprintf(name, "SMS_%d_NUMBER", i + 1);
		setenv(name, DecodeUnicodeConsole(sms->SMS[i].Number), 1);
		if (sms->SMS[i].Coding != SMS_Coding_8bit && sms->SMS[i].UDH.Type != UDH_UserUDH) {
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
					sprintf(name, "DECODED_%d_TEXT", i + 1);
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

/**
 * Executes external command.
 *
 * This is Windows variant.
 */
gboolean SMSD_RunOn(const char *command, GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, const char *locations, const char *event)
{
	BOOL ret;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	char *cmdline;

	cmdline = SMSD_RunOnCommand(locations, command);

	/* Prepare environment */
	if (sms != NULL) {
		SMSD_RunOnReceiveEnvironment(sms, Config, locations);
	}

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	SMSD_Log(DEBUG_INFO, Config, "Starting run on %s: %s", event, cmdline);

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

/**
 * Executes external command.
 *
 * This is POSIX variant.
 */
gboolean SMSD_RunOn(const char *command, GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, const char *locations, const char *event)
{
	int pid;
	int pipefd[2];
	int i;
	pid_t w;
	int status;
	char *cmdline;
	ssize_t bytes;
	char buffer[4097];
	gboolean result = FALSE;

	if (pipe(pipefd) == -1) {
		SMSD_LogErrno(Config, "Failed to open pipe for child process!");
		return FALSE;
	}

	pid = fork();

	if (pid == -1) {
		SMSD_LogErrno(Config, "Error spawning new process");
		return FALSE;
	}

	if (pid != 0) {
		/* We are the parent, wait for child */

		/* Close write end of pipe */
		close(pipefd[1]);
		if (fcntl(pipefd[0], F_SETFL, O_NONBLOCK) != 0) {
			SMSD_Log(DEBUG_ERROR, Config, "Failed to set nonblocking pipe to child!");
		}

		i = 0;
		do {
			while ((bytes = read(pipefd[0], buffer, 4096)) > 0) {
				buffer[bytes] = '\0';
				SMSD_Log(DEBUG_INFO, Config, "Subprocess output: %s", buffer);
			}
			w = waitpid(pid, &status, WUNTRACED | WCONTINUED);
			if (w == -1) {
				SMSD_Log(DEBUG_INFO, Config, "Failed to wait for process");
				result = FALSE;
				goto out;
			}

			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) == 0) {
					SMSD_Log(DEBUG_INFO, Config, "Process finished successfully");
				} else {
					SMSD_Log(DEBUG_ERROR, Config, "Process failed with exit status %d", WEXITSTATUS(status));
				}
				result = (WEXITSTATUS(status) == 0);
				goto out;
			} else if (WIFSIGNALED(status)) {
				SMSD_Log(DEBUG_ERROR, Config, "Process killed by signal %d", WTERMSIG(status));
				result = FALSE;
				goto out;
			} else if (WIFSTOPPED(status)) {
				SMSD_Log(DEBUG_INFO, Config, "Process stopped by signal %d", WSTOPSIG(status));
			} else if (WIFCONTINUED(status)) {
				SMSD_Log(DEBUG_INFO, Config, "Process continued");
			}
			usleep(100000);

			if (i++ > 1200) {
				SMSD_Log(DEBUG_INFO, Config, "Waited two minutes for child process, giving up");
				result = TRUE;
				goto out;
			}
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
out:
		while ((bytes = read(pipefd[0], buffer, 4096)) > 0) {
			buffer[bytes] = '\0';
			SMSD_Log(DEBUG_INFO, Config, "Subprocess output: %s", buffer);
		}
		close(pipefd[0]);

		return result;
	}

	/* we are the child */

	/* Close read end of pipe */
	close(pipefd[0]);

	/* Prepare environment */
	if (sms != NULL) {
		SMSD_RunOnReceiveEnvironment(sms, Config, locations);
	}

	/* Calculate command line */
	cmdline = SMSD_RunOnCommand(locations, command);
	SMSD_Log(DEBUG_INFO, Config, "Starting run on %s: %s", event, cmdline);

	/* Close all file descriptors */
	for (i = 0; i < 255; i++) {
		if (i != pipefd[1]) {
			close(i);
		}
	}

	/* Connect stdout and stderr to pipe */
	dup2(pipefd[1], 1);
	dup2(pipefd[1], 2);

	/* Run the program */
	execl("/bin/sh", "sh", "-c", cmdline, NULL);

	/* Happens only in case of error */
	SMSD_LogErrno(Config, "Error executing new process");
	exit(127);
}
#endif

/**
 * Checks whether we are allowed to accept a message from number.
 */
gboolean SMSD_CheckRemoteNumber(GSM_SMSDConfig *Config, const char *number)
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
gboolean SMSD_CheckSMSCNumber(GSM_SMSDConfig *Config, const char *number)
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
gboolean SMSD_ValidMessage(GSM_SMSDConfig *Config, GSM_MultiSMSMessage *sms)
{
	char buffer[100];

	/* Not Inbox SMS - exit */
	if (!sms->SMS[0].InboxFolder) {
		return FALSE;
	}
	/* Check SMSC number if we want to handle it */
	DecodeUnicode(sms->SMS[0].SMSC.Number, buffer);
	if (!SMSD_CheckSMSCNumber(Config, buffer)) {
		SMSD_Log(DEBUG_NOTICE, Config, "Message excluded because of SMSC: %s", buffer);
		return FALSE;
	}
	/* Check sender number if we want to handle it */
	DecodeUnicode(sms->SMS[0].Number, buffer);
	if (!SMSD_CheckRemoteNumber(Config, buffer)) {
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
GSM_Error SMSD_ProcessSMS(GSM_SMSDConfig *Config, GSM_MultiSMSMessage *sms)
{
	GSM_Error error = ERR_NONE;
	char *locations = NULL;

	/* Increase message counter */
	Config->Status->Received += sms->Number;
	/* Send message to the backend */
	error = Config->Service->SaveInboxSMS(sms, Config, &locations);
	/* RunOnReceive handling */
	if (Config->RunOnReceive != NULL && error == ERR_NONE) {
		SMSD_RunOn(Config->RunOnReceive, sms, Config, locations, "receive");
	}
	/* Free memory allocated by SaveInboxSMS */
	free(locations);
	return error;
}

/**
 * Checks whether to process current (possibly) multipart message.
 */
gboolean SMSD_CheckMultipart(GSM_SMSDConfig *Config, GSM_MultiSMSMessage *MultiSMS)
{
	gboolean same_id;
	int current_id;

	/* Does the message have UDH (is multipart)? */
	if (MultiSMS->SMS[0].UDH.Type == UDH_NoUDH || MultiSMS->SMS[0].UDH.AllParts == -1) {
		return TRUE;
	}

	/* Grab current id */
	if (MultiSMS->SMS[0].UDH.ID16bit != -1) {
		 current_id = MultiSMS->SMS[0].UDH.ID16bit;
	} else {
		 current_id = MultiSMS->SMS[0].UDH.ID8bit;
	}

	/* Do we have same id as last incomplete? */
	same_id = (Config->IncompleteMessageID != -1 && Config->IncompleteMessageID == current_id);

	/* Some logging */
	SMSD_Log(DEBUG_INFO, Config, "Multipart message 0x%02X, %d parts of %d",
		current_id, MultiSMS->Number, MultiSMS->SMS[0].UDH.AllParts);

	/* Check if we have all parts */
	if (MultiSMS->SMS[0].UDH.AllParts == MultiSMS->Number) {
		goto success;
	}

	/* Have we seen this message recently? */
	if (same_id) {
		if (Config->IncompleteMessageTime != 0 && difftime(time(NULL), Config->IncompleteMessageTime) >= Config->multiparttimeout) {
			SMSD_Log(DEBUG_INFO, Config, "Incomplete multipart message 0x%02X, processing after timeout",
				Config->IncompleteMessageID);
			Config->IncompleteMessageID = -1;
		} else {
			SMSD_Log(DEBUG_INFO, Config, "Incomplete multipart message 0x%02X, waiting for other parts (waited %.0f seconds)",
				Config->IncompleteMessageID,
				difftime(time(NULL), Config->IncompleteMessageTime));
			return FALSE;
		}
	} else {
		if (Config->IncompleteMessageTime == 0) {
			if (MultiSMS->SMS[0].UDH.ID16bit != -1) {
				 Config->IncompleteMessageID = MultiSMS->SMS[0].UDH.ID16bit;
			} else {
				 Config->IncompleteMessageID = MultiSMS->SMS[0].UDH.ID8bit;
			}
			Config->IncompleteMessageTime = time(NULL);
			SMSD_Log(DEBUG_INFO, Config, "Incomplete multipart message 0x%02X, waiting for other parts",
				Config->IncompleteMessageID);
			return FALSE;
		} else {
			SMSD_Log(DEBUG_INFO, Config, "Incomplete multipart message 0x%02X, but waiting for other one",
				Config->IncompleteMessageID);
			return FALSE;
		}
	}

success:
	/* Clean multipart wait flag */
	if (same_id) {
		Config->IncompleteMessageTime = 0;
		Config->IncompleteMessageID = -1;
	}
	return TRUE;
}

/**
 * Reads message from phone, processes it and delete it from phone afterwards.
 *
 * It tries to link multipart messages together if possible.
 */
gboolean SMSD_ReadDeleteSMS(GSM_SMSDConfig *Config)
{
	gboolean start;
	GSM_MultiSMSMessage sms;
	GSM_MultiSMSMessage **GetSMSData = NULL, **SortedSMS;
	int allocated = 0;
	GSM_Error error = ERR_NONE;
	int GetSMSNumber = 0;
	int i, j;

	/* Read messages from phone */
	Config->IgnoredMessages = 0;
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
				if (SMSD_ValidMessage(Config, &sms)) {
					if (allocated <= GetSMSNumber + 2) {
						GetSMSData = (GSM_MultiSMSMessage **)realloc(GetSMSData, (allocated + 20) * sizeof(GSM_MultiSMSMessage *));
						if (GetSMSData == NULL) {
							SMSD_Log(DEBUG_ERROR, Config, "Failed to allocate memory");
							return FALSE;
						}
						allocated += 20;
					}
					GetSMSData[GetSMSNumber] = malloc(sizeof(GSM_MultiSMSMessage));

					if (GetSMSData[GetSMSNumber] == NULL) {
						SMSD_Log(DEBUG_ERROR, Config, "Failed to allocate memory");
						for (i = 0; GetSMSData[i] != NULL; i++) {
							free(GetSMSData[i]);
							GetSMSData[i] = NULL;
						}
						free(GetSMSData);
						return FALSE;
					}

					*(GetSMSData[GetSMSNumber]) = sms;
					GetSMSNumber++;
					GetSMSData[GetSMSNumber] = NULL;
				} else {
					Config->IgnoredMessages++;
				}
				break;
			default:
				SMSD_LogError(DEBUG_ERROR, Config, "Error getting SMS", error);
				if (GetSMSData != NULL) {
					for (i = 0; GetSMSData[i] != NULL; i++) {
						free(GetSMSData[i]);
						GetSMSData[i] = NULL;
					}
					free(GetSMSData);
				}
				return FALSE;
		}
		start = FALSE;

    /* process any incoming SMS information records to help prevent memory exhaustion, ignore any
     * errors so as not to interfere with this function, they'll be handle in main-loop processing */
    SMSD_ProcessSMSInfoCache(Config);
  }

	/* Log how many messages were read */
	SMSD_Log(DEBUG_INFO, Config, "Read %d messages", GetSMSNumber);

	/* No messages to process */
	if (GetSMSNumber == 0) {
		return TRUE;
	}

	/* Allocate memory for sorted messages */
	SortedSMS = (GSM_MultiSMSMessage **)malloc(allocated * sizeof(GSM_MultiSMSMessage *));
	if (SortedSMS == NULL) {
		SMSD_Log(DEBUG_ERROR, Config, "Failed to allocate memory for linking messages");
		SMSD_Log(DEBUG_ERROR, Config, "Skipping linking messages, long messages will not be connected");
		SortedSMS = GetSMSData;
	} else {
		/* Link messages */
		error = GSM_LinkSMS(GSM_GetDebug(Config->gsm), GetSMSData, SortedSMS, TRUE);
		if (error != ERR_NONE) return FALSE;

		/* Free memory */
		for (i = 0; GetSMSData[i] != NULL; i++) {
			free(GetSMSData[i]);
			GetSMSData[i] = NULL;
		}
		free(GetSMSData);
	}

	/* Process messages */
	for (i = 0; SortedSMS[i] != NULL; i++) {
		/* Check multipart message parts */
		if (!SMSD_CheckMultipart(Config, SortedSMS[i])) {
			goto cleanup;
		}

		/* Actually process the message */
		error = SMSD_ProcessSMS(Config, SortedSMS[i]);
		if (error != ERR_NONE) {
			SMSD_LogError(DEBUG_INFO, Config, "Error processing SMS", error);
			return FALSE;
		}

		/* Delete processed messages */
		for (j = 0; j < SortedSMS[i]->Number; j++) {
			SortedSMS[i]->SMS[j].Folder = 0;
			error = GSM_DeleteSMS(Config->gsm, &SortedSMS[i]->SMS[j]);
			// Empty error can happen if deleting message several times
			if (error != ERR_NONE && error != ERR_EMPTY) {
				SMSD_LogError(DEBUG_INFO, Config, "Error deleting SMS", error);
				return FALSE;
			}
		}

cleanup:
		free(SortedSMS[i]);
		SortedSMS[i] = NULL;
	}
	free(SortedSMS);
	return TRUE;
}

/**
 * Checks whether there are some messages to process and calls
 * SMSD_ReadDeleteSMS to process them.
 */
gboolean SMSD_CheckSMSStatus(GSM_SMSDConfig *Config)
{
	GSM_SMSMemoryStatus	SMSStatus;
	GSM_Error		error;
	gboolean new_message = FALSE;
	GSM_MultiSMSMessage sms;

	/* Do we have any SMS in phone ? */

	/* First try SMS status */
	error = GSM_GetSMSStatus(Config->gsm,&SMSStatus);
	if (error == ERR_NONE) {
		new_message = (SMSStatus.SIMUsed + SMSStatus.PhoneUsed - Config->IgnoredMessages > 0);
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
		return SMSD_ReadDeleteSMS(Config);
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
	if (Config->checknetwork) {
		error = GSM_GetNetworkInfo(Config->gsm, &Config->Status->NetInfo);
	} else {
		error = ERR_UNKNOWN;
	}
	if (error != ERR_NONE) {
		memset(&(Config->Status->NetInfo), 0, sizeof(Config->Status->NetInfo));
	} else if (error == ERR_NONE) {
		if (Config->Status->NetInfo.State == GSM_NoNetwork) {
			GSM_SetPower(Config->gsm, TRUE);
		}
	}
}

/**
 * Sends a sms message which is provided by the service backend.
 */
GSM_Error SMSD_SendSMS(GSM_SMSDConfig *Config)
{
	GSM_MultiSMSMessage  	sms;
	GSM_DateTime         	Date;
	GSM_Error            	error;
	unsigned int         	j;
	int			i, z;
	char destinationnumber[3 * GSM_MAX_NUMBER_LENGTH + 1];

	/* Clean structure before use */
	for (i = 0; i < GSM_MAX_MULTI_SMS; i++) {
		GSM_SetDefaultSMSData(&sms.SMS[i]);
	}

	error = Config->Service->FindOutboxSMS(&sms, Config, Config->SMSID);

	if (error == ERR_EMPTY || error == ERR_NOTSUPPORTED) {
		/* No outbox sms */
		return error;
	}
	if (error != ERR_NONE) {
		/* Unknown error - escape */
		SMSD_Log(DEBUG_INFO, Config, "Error in outbox on '%s'", Config->SMSID);
		for (i = 0; i < sms.Number; i++) {
			Config->Status->Failed++;
			Config->Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i+1, SMSD_SEND_ERROR, -1);
		}
		Config->Service->MoveSMS(&sms,Config, Config->SMSID, TRUE,FALSE);
		return error;
	}

	if (Config->shutdown) {
		return ERR_NONE;
	}

	if (Config->SMSID[0] != 0 && (Config->retries > Config->maxretries)) {
		SMSD_Log(DEBUG_NOTICE, Config, "Moved to errorbox, reached MaxRetries: %s", Config->SMSID);
		for (i=0;i<sms.Number;i++) {
			if (Config->SkipMessage[i] == TRUE) {
				SMSD_Log(DEBUG_NOTICE, Config, "Skipping %s:%d message for errorbox", Config->SMSID, i+1);
				continue;
			}
			Config->Status->Failed++;
			Config->Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i + 1, SMSD_SEND_SENDING_ERROR, Config->TPMR);
		}
		Config->Service->MoveSMS(&sms,Config, Config->SMSID, TRUE,FALSE);
		return ERR_UNKNOWN;
	} else {
		SMSD_Log(DEBUG_NOTICE, Config, "New message to send: %s", Config->SMSID);
		Config->retries++;
	}

	for (i = 0; i < sms.Number; i++) {
		if (Config->SkipMessage[i] == TRUE) {
			SMSD_Log(DEBUG_NOTICE, Config, "Skipping %s:%d message for delivery", Config->SMSID, i+1);
			continue;
		}

		/* No SMSC set in message */
		if (sms.SMS[i].SMSC.Location == 0 && UnicodeLength(sms.SMS[i].SMSC.Number) == 0 && Config->SMSC.Location == 0) {
			SMSD_Log(DEBUG_INFO, Config, "Message without SMSC, using configured one");
			sms.SMS[i].SMSC = Config->SMSC;
			if (Config->relativevalidity != -1) {
				sms.SMS[i].SMSC.Validity.Format	  = SMS_Validity_RelativeFormat;
				sms.SMS[i].SMSC.Validity.Relative = Config->relativevalidity;
			}

		}
		/* Still nothing set after using configured one */
		if (sms.SMS[i].SMSC.Location == 0 && UnicodeLength(sms.SMS[i].SMSC.Number) == 0) {
			SMSD_Log(DEBUG_INFO, Config, "Message without SMSC, assuming you want to use the one from phone");
			sms.SMS[i].SMSC.Location = 1;
		}
		/* Should use SMSC from phone */
		if (sms.SMS[i].SMSC.Location != 0) {
			/* Do we have cached entry? */
			if (Config->SMSCCache.Location != sms.SMS[i].SMSC.Location) {
				Config->SMSCCache.Location = sms.SMS[i].SMSC.Location;
				error = GSM_GetSMSC(Config->gsm, &Config->SMSCCache);
				if (error!=ERR_NONE) {
					SMSD_Log(DEBUG_ERROR, Config, "Error getting SMSC from phone");
					return ERR_UNKNOWN;
				}

			}
			sms.SMS[i].SMSC = Config->SMSCCache;
			/* Reset location to avoid reading from phone */
			sms.SMS[i].SMSC.Location = 0;
			if (Config->relativevalidity != -1) {
				sms.SMS[i].SMSC.Validity.Format	  = SMS_Validity_RelativeFormat;
				sms.SMS[i].SMSC.Validity.Relative = Config->relativevalidity;
			}
		}

		if (Config->currdeliveryreport == 1) {
			sms.SMS[i].PDU = SMS_Status_Report;
		} else if (Config->currdeliveryreport == -1 && strcmp(Config->deliveryreport, "no") != 0) {
			sms.SMS[i].PDU = SMS_Status_Report;
		}

		SMSD_PhoneStatus(Config);
		Config->TPMR = -1;
		Config->SendingSMSStatus = ERR_TIMEOUT;
		Config->StatusCode = -1;
		Config->Part = i + 1;
		if (sms.SMS[i].Class == GSM_SMS_USSD) {
			EncodeUTF8(destinationnumber, sms.SMS[i].Number);
			SMSD_Log(DEBUG_NOTICE, Config, "Sending USSD request to %s", destinationnumber);
			error = GSM_DialService(Config->gsm, destinationnumber);
			/* Fallback to voice call, it can work with some phones */
			if (error == ERR_NOTIMPLEMENTED || error == ERR_NOTSUPPORTED) {
				error = GSM_DialVoice(Config->gsm, destinationnumber, GSM_CALL_DefaultNumberPresence);
			}
			if (error == ERR_NONE) {
				Config->SendingSMSStatus = ERR_NONE;
			}
		} else {
			error = GSM_SendSMS(Config->gsm, &sms.SMS[i]);
		}
		if (error != ERR_NONE) {
			SMSD_LogError(DEBUG_INFO, Config, "Error sending SMS", error);
			Config->TPMR = -1;
			goto failure_unsent;
		}
		j    = 0;
		while (!Config->shutdown) {
			/* Update timestamp for SMS in backend */
			Config->Service->RefreshSendStatus(Config, Config->SMSID);

			GSM_GetCurrentDateTime(&Date);
			z = Date.Second;
			while (z == Date.Second) {
				usleep(10000);
				GSM_GetCurrentDateTime(&Date);
				GSM_ReadDevice(Config->gsm, TRUE);
				if (Config->SendingSMSStatus != ERR_TIMEOUT) {
					break;
				}
			}
			if (Config->SendingSMSStatus != ERR_TIMEOUT) {
				break;
			}
			j++;
			if (j > Config->sendtimeout) {
				break;
			}
		}
		if (Config->SendingSMSStatus != ERR_NONE) {
			SMSD_LogError(DEBUG_INFO, Config, "Error getting send status of message", Config->SendingSMSStatus);
			goto failure_unsent;
		}
		Config->Status->Sent++;
		error = Config->Service->AddSentSMSInfo(&sms, Config, Config->SMSID, i+1, SMSD_SEND_OK, Config->TPMR);
		if (error != ERR_NONE) {
			SMSD_LogError(DEBUG_INFO, Config, "Error setting sent status", error);
			goto failure_sent;
		}

		/* process any incoming SMS information records to help prevent memory exhaustion, ignore any
		 * errors so as not to interfere with this function, they'll be handle in main-loop processing */
		SMSD_ProcessSMSInfoCache(Config);
	}
	strcpy(Config->prevSMSID, "");
	error = Config->Service->MoveSMS(&sms,Config, Config->SMSID, FALSE, TRUE);
	if (error != ERR_NONE) {
		SMSD_LogError(DEBUG_ERROR, Config, "Error moving message", error);
		Config->Service->MoveSMS(&sms,Config, Config->SMSID, TRUE, FALSE);
	}

	if (Config->RunOnSent != NULL && error == ERR_NONE) {
		SMSD_RunOn(Config->RunOnSent, &sms, Config, Config->SMSID, "sent");
	}

	return ERR_NONE;
failure_unsent:
	if (Config->RunOnFailure != NULL) {
		SMSD_RunOn(Config->RunOnFailure, NULL, Config, Config->SMSID, "failure");
	}
	Config->Status->Failed++;

failure_sent:
	Config->Service->UpdateRetries(Config, Config->SMSID);

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
		if (writable) {
			SMSD_Terminate(Config, "Failed to allocate shared memory segment!", ERR_NONE, TRUE, -1);
			return ERR_UNKNOWN;
		} else {
			SMSD_Terminate(Config, "Failed to map shared memory segment!", ERR_NONE, TRUE, -1);
			return ERR_NOTRUNNING;
		}
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
	if (writable) {
		SMSD_Log(DEBUG_INFO, Config, "Created POSIX RW shared memory at %p", Config->Status);
	} else {
		SMSD_Log(DEBUG_INFO, Config, "Mapped POSIX RO shared memory at %p", Config->Status);
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
	if (writable) {
		SMSD_Log(DEBUG_INFO, Config, "Created Windows RW shared memory at %p", Config->Status);
	} else {
		SMSD_Log(DEBUG_INFO, Config, "Mapped Windows RO shared memory at %p", Config->Status);
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
	if (writable) {
		SMSD_Log(DEBUG_INFO, Config, "No shared memory, using standard malloc %p", Config->Status);
	}
#endif
	/* Initial shared memory content */
	if (writable) {
		Config->Status->Version = SMSD_SHM_VERSION;
		strncpy(Config->Status->PhoneID, Config->PhoneID, sizeof(Config->Status->PhoneID) - 1);
		Config->Status->PhoneID[sizeof(Config->Status->PhoneID) - 1] = 0;
		sprintf(Config->Status->Client, "Gammu %s on %s compiler %s",
			GAMMU_VERSION,
			GetOS(),
			GetCompiler());
		memset(&Config->Status->Charge, 0, sizeof(GSM_BatteryCharge));
		memset(&Config->Status->Network, 0, sizeof(GSM_SignalQuality));
		memset(&Config->Status->NetInfo, 0, sizeof(GSM_NetworkInfo));
		Config->Status->Received = 0;
		Config->Status->Failed = 0;
		Config->Status->Sent = 0;
		Config->Status->IMEI[0] = 0;
		Config->Status->IMSI[0] = 0;
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

/** handle incoming calls: hang up.
 */
void SMSD_IncomingCallCallback(GSM_StateMachine *s, GSM_Call *call, void *user_data) {
	GSM_SMSDConfig *Config = user_data;
	GSM_Error error;
	switch (call->Status) {
	case GSM_CALL_IncomingCall: {
		time_t now = time(NULL);
		SMSD_Log(DEBUG_INFO, Config, "Incoming call! # avail? %d %s\n", call->CallIDAvailable, DecodeUnicodeString(call->PhoneNumber) );
		if ( now - lastRing > 5 ) {
			// avoid multiple hangups.
			SMSD_Log(DEBUG_INFO, Config, "Incoming call! # hanging up @%ld %ld.\n", now, lastRing);
			lastRing = now;
			if (call->CallIDAvailable) {
				error = GSM_CancelCall(s, call->CallID, TRUE);
			}
			if (!call->CallIDAvailable || error == ERR_NOTSUPPORTED) {
				error = GSM_CancelCall(s, 0, TRUE);
			}
			if (error != ERR_NONE) {
				SMSD_LogError(DEBUG_ERROR, Config, "Failed call hangup!", error);
			}

			if (Config->RunOnIncomingCall != NULL) {
				SMSD_RunOn(Config->RunOnIncomingCall, NULL, Config, DecodeUnicodeString(call->PhoneNumber), "incoming call");
			}
		}
		break;
	}
	case  GSM_CALL_CallRemoteEnd:
	case GSM_CALL_CallLocalEnd:
		SMSD_Log(DEBUG_INFO, Config, "Call ended(%d).\n", call->Status );
		lastRing = 0;
		break;
	default:
		SMSD_Log(DEBUG_INFO, Config, "Call callback: Unknown status %d\n", call->Status);
	}
}

void SMSD_IncomingUSSDCallback(GSM_StateMachine *sm UNUSED, GSM_USSDMessage *ussd, void *user_data)
{
	GSM_MultiSMSMessage sms;
	GSM_Error error;
	GSM_SMSDConfig *Config = user_data;

	SMSD_Log(DEBUG_NOTICE, Config, "%s", __FUNCTION__);

	memset(&sms, 0, sizeof(GSM_MultiSMSMessage));
	sms.Number = 1;
	sms.SMS[0].Class = GSM_SMS_USSD;
	memcpy(&sms.SMS[0].Text, ussd->Text, UnicodeLength(ussd->Text)*2);
	sms.SMS[0].PDU = SMS_Deliver;
	sms.SMS[0].Coding = SMS_Coding_Unicode_No_Compression;
	GSM_GetCurrentDateTime(&sms.SMS[0].DateTime);
	sms.SMS[0].DeliveryStatus = ussd->Status;

	error = SMSD_ProcessSMS(Config, &sms);
	if (error != ERR_NONE) {
		SMSD_LogError(DEBUG_INFO, Config, "Error processing USSD", error);
	}
}

#define INIT_SMSINFO_CACHE_SIZE 10
/**
 * Handles SMS information messages (+CDSI/+CMTI)
 *
 * SMSD uses polling to read messages from MT memory, to avoid potential
 * conflicts only information on status reports stored in SR memory are handled.
 *
 * The amount of SR memory on a device is generally very small so it's unlikely
 * there would be an issue creating/reallocating the cache, if such an issue
 * occurs some or all status report information records will be lost.
 */
void SMSD_IncomingSMSInfoCallback(GSM_StateMachine *s,  GSM_SMSMessage *sms, void *user_data)
{
  GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
  GSM_AT_SMSInfo_Cache *Cache = &Priv->SMSInfoCache;
  GSM_SMSDConfig *Config = user_data;
  void *reallocated = NULL;

  if (sms->PDU != SMS_Status_Report || sms->Memory != MEM_SR) {
  	SMSD_Log(DEBUG_INFO, Config, "Ignoring incoming SMS info as not a Status Report in SR memory.");
		return;
	}

  SMSD_Log(DEBUG_INFO, Config, "caching incoming status report information.");

  if (Cache->cache_size <= Cache->cache_used) {
    if (Cache->smsInfo_records == NULL) {
      Cache->smsInfo_records = malloc(INIT_SMSINFO_CACHE_SIZE * sizeof(*Cache->smsInfo_records));
      if (Cache->smsInfo_records == NULL) {
        SMSD_Log(DEBUG_ERROR, Config, "failed to allocate SMS information cache, records will not be processed.");
        return;
      }
      Cache->cache_size = INIT_SMSINFO_CACHE_SIZE;
    } else {
      reallocated = realloc(Cache->smsInfo_records, (Cache->cache_size * 2) * sizeof(*Cache->smsInfo_records));
      if (reallocated == NULL) {
        SMSD_Log(DEBUG_ERROR, Config, "failed to reallocate SMS information cache, some records will be lost.");
        return;
      }
      Cache->smsInfo_records = reallocated;
      Cache->cache_size *= 2;
    }
  }

  memcpy(Cache->smsInfo_records + Cache->cache_used, sms, sizeof(*Cache->smsInfo_records));
  Cache->cache_used += 1;
}

/**
 * Handles incoming SMS messages.
 *
 */
void SMSD_IncomingSMSCallback(GSM_StateMachine *s,  GSM_SMSMessage *sms, void *user_data)
{
	GSM_MultiSMSMessage msms;
	GSM_SMSDConfig *Config = user_data;
	GSM_Error error;

	if(sms->PDU == 0) {
		// assume we only have message information, not a full message, handoff to appropriate handler
		SMSD_IncomingSMSInfoCallback(s, sms, user_data);
		return;
	}

	SMSD_Log(DEBUG_INFO, Config, "processing incoming SMS.");

	memset(&msms, 0, sizeof(GSM_MultiSMSMessage));
	msms.Number = 1;
	msms.SMS[0] = *sms;

	error = SMSD_ProcessSMS(Config, &msms);
	if(error != ERR_NONE)
		SMSD_LogError(DEBUG_ERROR, Config, "Error processing SMS", error);
}

GSM_Error SMSD_ProcessSMSInfoCache(GSM_SMSDConfig *Config)
{
	GSM_StateMachine *s = Config->gsm;
	GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
	GSM_AT_SMSInfo_Cache *Cache = &Priv->SMSInfoCache;
	GSM_MultiSMSMessage msms;
	GSM_SMSMessage *sms;
	GSM_Error error = ERR_NONE;
	unsigned int i;

	memset(&msms, 0, sizeof(GSM_MultiSMSMessage));
	msms.Number = 1;

	for(i = 0; i < Cache->cache_used; ++i) {
		sms = Cache->smsInfo_records + i;
		if(sms->Memory == MEM_INVALID) continue;

		msms.SMS[0] = *sms;

		error = GSM_GetSMS(s, &msms);
		if(error != ERR_NONE) {
			SMSD_Log(DEBUG_ERROR, Config, "Error reading SMS from memory %s:%d",
					GSM_MemoryTypeToString(sms->Memory),
					sms->Location);
			break;
		}

		error = SMSD_ProcessSMS(Config, &msms);
		if(error != ERR_NONE) {
			SMSD_LogError(DEBUG_ERROR, Config, "Error processing SMS", error);
			break;
		}

		error = GSM_DeleteSMS(s, sms);
		if (error != ERR_NONE) {
			SMSD_LogError(DEBUG_ERROR, Config, "Error deleting SMS", error);
			break;
		}

		/* successfully processed cache entry, mark invalid to avoid reprocessing
		 * in case of retry loop */
		sms->Memory = MEM_INVALID;
	}

	/* cache processed successfully, reset used count */
	if(error == ERR_NONE)
		Cache->cache_used = 0;

	return error;
}

/**
 * Main loop which takes care of connection to phone and processing of
 * messages.
 */
GSM_Error SMSD_MainLoop(GSM_SMSDConfig *Config, gboolean exit_on_failure, int max_failures)
{
	GSM_Error		error;
	int                     errors = -1, initerrors=0;
	double			lastsleep;
 	time_t			lastreceive = 0, lastreset = time(NULL), lasthardreset = time(NULL), lastnothingsent = 0, laststatus = 0;
	time_t			lastloop = 0;
	gboolean first_start = TRUE, force_reset = FALSE, force_hard_reset = FALSE;

	Config->failure = ERR_NONE;
	Config->exit_on_failure = exit_on_failure;

	/* Init service */
	error = SMSD_Init(Config);
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

	Config->SendingSMSStatus = ERR_NONE;

	while (!Config->shutdown) {
		lastloop = time(NULL);
		/* There were errors in communication - try to recover */
		if (errors > 2 || first_start || force_reset || force_hard_reset) {
			/* Should we disconnect from phone? */
			if (GSM_IsConnected(Config->gsm)) {
				if (! force_reset && ! force_hard_reset) {
					SMSD_Log(DEBUG_INFO, Config, "Already hit %d errors", errors);
				}
				SMSD_LogError(DEBUG_INFO, Config, "Terminating communication", error);
				GSM_TerminateConnection(Config->gsm);
			}
			/* Did we reach limit for errors? */
			if (max_failures != 0 && initerrors > max_failures) {
				Config->failure = ERR_TIMEOUT;
				SMSD_Log(DEBUG_INFO, Config, "Reached maximum number of failures (%d), terminating", max_failures);
				break;
			}
			if (initerrors++ > 3) {
				SMSD_Log(DEBUG_INFO, Config, "Going to 30 seconds sleep because of too many connection errors");

				SMSD_InterruptibleSleep(Config, 30);
			}
			SMSD_Log(DEBUG_INFO, Config, "Starting phone communication...");
			error = GSM_InitConnection_Log(Config->gsm, 2, SMSD_Log_Function, Config);
			/* run on error */
			if (error != ERR_NONE && Config->RunOnFailure != NULL) {
				SMSD_RunOn(Config->RunOnFailure, NULL, Config, "INIT", "failure");
			}
			switch (error) {
			case ERR_NONE:
				if (Config->checksecurity && !SMSD_CheckSecurity(Config)) {
					errors++;
					initerrors++;
					continue;
				}

				/* handle incoming calls: */
				if (Config->hangupcalls) {
					GSM_SetIncomingCallCallback(Config->gsm, SMSD_IncomingCallCallback, Config);
					GSM_SetIncomingCall(Config->gsm, TRUE);
				}

				GSM_SetIncomingSMSCallback(Config->gsm, SMSD_IncomingSMSCallback, Config);

				/* We use polling so store messages to SIM */
				GSM_SetIncomingSMS(Config->gsm, TRUE);
				GSM_SetIncomingUSSDCallback(Config->gsm, SMSD_IncomingUSSDCallback, Config);
				GSM_SetIncomingUSSD(Config->gsm, TRUE);

				GSM_SetSendSMSStatusCallback(Config->gsm, SMSD_SendSMSStatusCallback, Config);
				/* On first start we need to initialize some variables */
				if (first_start) {
					if (GSM_GetIMEI(Config->gsm, Config->Status->IMEI) != ERR_NONE || GSM_GetSIMIMSI(Config->gsm, Config->Status->IMSI) != ERR_NONE) {
						errors++;
					} else {
						errors = 0;
						error = Config->Service->InitAfterConnect(Config);
						if (error!=ERR_NONE) {
							if (Config->RunOnFailure != NULL) {
								SMSD_RunOn(Config->RunOnFailure, NULL, Config, "INIT", "failure");
							}
							SMSD_Terminate(Config, "Post initialisation failed, stopping Gammu smsd", error, TRUE, -1);
							goto done_connected;
						}
						GSM_SetFastSMSSending(Config->gsm, TRUE);
					}
					first_start = FALSE;
				} else {
					errors = 0;
				}

				if (initerrors > 3 || force_reset ) {
					error = GSM_Reset(Config->gsm, FALSE); /* soft reset */
					SMSD_LogError(DEBUG_INFO, Config, "Soft reset return code", error);
					lastreset = time(NULL);
					SMSD_InterruptibleSleep(Config, 5);
					force_reset = FALSE;
				}
				if (force_hard_reset) {
					error = GSM_Reset(Config->gsm, TRUE); /* hard reset */
					SMSD_LogError(DEBUG_INFO, Config, "Hard reset return code", error);
					lasthardreset = time(NULL);
					SMSD_InterruptibleSleep(Config, 5);
					force_hard_reset = FALSE;
				}
				break;
			case ERR_DEVICEOPENERROR:
				SMSD_Terminate(Config, "Can't open device",	error, TRUE, -1);
				goto done;
			default:
				SMSD_LogError(DEBUG_INFO, Config, "Error at init connection", error);
				errors = 250;
				break;
			}
			continue;
		}

		/* Should we receive? */
		if (Config->enable_receive && ((difftime(lastloop, lastreceive) >= Config->receivefrequency) || (Config->SendingSMSStatus != ERR_NONE))) {
	 		lastreceive = time(NULL);

			/* Do we need to check security? */
			if (Config->checksecurity) {
				if (!SMSD_CheckSecurity(Config)) {
					errors++;
					initerrors++;
					continue;
				} else {
					errors = 0;
				}
			}

			initerrors = 0;

			/* process SMS info cache */
			if(!SMSD_ProcessSMSInfoCache(Config)) {
				errors++;
				continue;
			} else {
				errors = 0;
			}

			/* read all incoming SMS */
			if (!SMSD_CheckSMSStatus(Config)) {
				errors++;
				continue;
			} else {
				errors = 0;
			}

		}

		/* time for preventive reset */
		if (Config->resetfrequency > 0 && difftime(lastloop, lastreset) >= Config->resetfrequency) {
			force_reset = TRUE;
			continue;
		}
		if (Config->hardresetfrequency > 0 && difftime(lastloop, lasthardreset) >= Config->hardresetfrequency) {
			force_hard_reset = TRUE;
			continue;
		}
		if (Config->shutdown) {
			break;
		}

		/* Send any queued messages */
		if (Config->enable_send && (difftime(lastloop, lastnothingsent) >= Config->commtimeout)) {
			error = SMSD_SendSMS(Config);
			if (error == ERR_EMPTY) {
				lastnothingsent = lastloop;
			}
			/* We don't care about other errors here, they are handled in SMSD_SendSMS */
		}
		if (Config->shutdown) {
			break;
		}

		/* Refresh phone status in shared memory and in service */
		if ((Config->statusfrequency > 0) && (difftime(lastloop, laststatus) >= Config->statusfrequency)) {
			SMSD_PhoneStatus(Config);
			laststatus = lastloop;
			Config->Service->RefreshPhoneStatus(Config);
		}

		if (Config->shutdown) {
			break;
		}

		/* Sleep some time before another loop */
		/* Duration of last loop cycle */
		lastsleep = difftime(time(NULL), lastloop);
		if (Config->loopsleep > 0 && lastsleep < Config->loopsleep) {
			/* Sleep LoopSleep - time of the loop */
			SMSD_InterruptibleSleep(Config, Config->loopsleep - lastsleep);
		}
	}
	GSM_SetIncomingUSSD(Config->gsm, FALSE);
	Config->Service->Free(Config);

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
GSM_Error SMSD_InjectSMS(GSM_SMSDConfig *Config, GSM_MultiSMSMessage *sms, char *NewID)
{
	GSM_Error error;

	/* Initialize service */
	error = SMSD_Init(Config);
	if (error != ERR_NONE) {
		return error;
	}

	/* Store message in outbox */
	error = Config->Service->CreateOutboxSMS(sms, Config, NewID);
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
