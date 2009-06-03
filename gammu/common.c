#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>

#include "../helper/locales.h"

#include "common.h"
#include "../helper/formats.h"
#include "../helper/printing.h"

#ifdef CURL_FOUND
#include <curl/curl.h>
#endif

GSM_StateMachine *gsm = NULL;
INI_Section *cfg = NULL;

gboolean always_answer_yes = FALSE;
gboolean always_answer_no = FALSE;
volatile gboolean gshutdown = FALSE;
gboolean batch = FALSE;
gboolean batchConn = FALSE;

void interrupt(int sign)
{
	signal(sign, SIG_IGN);
	gshutdown = TRUE;
}

void PrintSecurityStatus(void)
{
	GSM_SecurityCodeType Status;
	GSM_Error error;

	error = GSM_GetSecurityStatus(gsm, &Status);
	Print_Error(error);
	switch (Status) {
		case SEC_SecurityCode:
			printf("%s\n", _("Waiting for Security Code."));
			break;
		case SEC_Pin:
			printf("%s\n", _("Waiting for PIN."));
			break;
		case SEC_Pin2:
			printf("%s\n", _("Waiting for PIN2."));
			break;
		case SEC_Puk:
			printf("%s\n", _("Waiting for PUK."));
			break;
		case SEC_Puk2:
			printf("%s\n", _("Waiting for PUK2."));
			break;
		case SEC_Phone:
			printf("%s\n", _("Waiting for phone code."));
			break;
		case SEC_Network:
			printf("%s\n", _("Waiting for network code."));
			break;
		case SEC_None:
			printf("%s\n", _("Nothing to enter."));
			break;
#ifndef CHECK_CASES
		default:
			printf("%s\n", _("Unknown security status."));
#endif
	}
}

const char *GetMonthName(const int month)
{
	static char result[100];

	switch (month) {
		case 1:
			strcpy(result, _("January"));
			break;
		case 2:
			strcpy(result, _("February"));
			break;
		case 3:
			strcpy(result, _("March"));
			break;
		case 4:
			strcpy(result, _("April"));
			break;
		case 5:
			strcpy(result, _("May"));
			break;
		case 6:
			strcpy(result, _("June"));
			break;
		case 7:
			strcpy(result, _("July"));
			break;
		case 8:
			strcpy(result, _("August"));
			break;
		case 9:
			strcpy(result, _("September"));
			break;
		case 10:
			strcpy(result, _("October"));
			break;
		case 11:
			strcpy(result, _("November"));
			break;
		case 12:
			strcpy(result, _("December"));
			break;
		default:
			strcpy(result, _("Bad month!"));
			break;
	}
	return result;
}

const char *GetDayName(const int day)
{
	static char result[100];

	switch (day) {
		case 1:
			strcpy(result, _("Monday"));
			break;
		case 2:
			strcpy(result, _("Tuesday"));
			break;
		case 3:
			strcpy(result, _("Wednesday"));
			break;
		case 4:
			strcpy(result, _("Thursday"));
			break;
		case 5:
			strcpy(result, _("Friday"));
			break;
		case 6:
			strcpy(result, _("Saturday"));
			break;
		case 7:
			strcpy(result, _("Sunday"));
			break;
		default:
			strcpy(result, _("Bad day!"));
			break;
	}
	return result;
}

NORETURN
void Terminate(int code)
{
	Cleanup();
	exit(code);
}

void Cleanup(void)
{
	GSM_Debug_Info *di;

	if (gsm != NULL) {
		/* Disconnect from phone */
		if (GSM_IsConnected(gsm)) {
			GSM_TerminateConnection(gsm);
		}

		/* Free state machine */
		GSM_FreeStateMachine(gsm);
	}
	gsm = NULL;

	/* Close debug output if opened */
	di = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(NULL, FALSE, di);

#ifdef CURL_FOUND
	/* Free CURL memory */
	curl_global_cleanup();
#endif

	INI_Free(cfg);
	cfg = NULL;
}

void Print_Error(GSM_Error error)
{
	if (error != ERR_NONE) {
		printf("%s\n", GSM_ErrorString(error));
		/* Check for security error */
		if (error == ERR_SECURITYERROR) {
			printf(LISTFORMAT, _("Security status"));
			PrintSecurityStatus();
		}

		Terminate(3);
	}
}

/**
 * Callback from CURL to get data.
 */
size_t write_mem(void *ptr, size_t size, size_t nmemb, void *data) {
	size_t realsize = size * nmemb;
	GSM_File *file = (GSM_File *)data;

	file->Buffer = realloc(file->Buffer,file->Used + realsize + 1);

	if (file->Buffer) {
		memcpy(file->Buffer + file->Used, ptr, realsize);
		file->Used += realsize;
		file->Buffer[file->Used] = 0;
		return realsize;
	}
	return 0;
}

/**
 * Downloads file from arbitrary URL.
 *
 * \param url URL to download.
 * \param file Storage for data.
 *
 * \returns True on success.
 */
gboolean GSM_ReadHTTPFile(const char *url, GSM_File *file)
{
#ifdef CURL_FOUND
	CURL *dl_handle = NULL;
	CURLcode result;

	dl_handle = curl_easy_init();
	if (dl_handle == NULL) return FALSE;

	curl_easy_setopt(dl_handle, CURLOPT_URL, url);
	curl_easy_setopt(dl_handle, CURLOPT_USERAGENT, "Gammu/" VERSION);
	curl_easy_setopt(dl_handle, CURLOPT_WRITEFUNCTION, write_mem);
	curl_easy_setopt(dl_handle, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(dl_handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(dl_handle, CURLOPT_MAXREDIRS, 10);
#if 0
	/* Enable verbose outpuf from CURL */
	curl_easy_setopt(dl_handle, CURLOPT_VERBOSE, 1);
#endif

	result = curl_easy_perform(dl_handle);

	curl_easy_cleanup(dl_handle);

	return (result == 0) ? TRUE : FALSE;
#else
	return FALSE;
#endif
}

/**
 * Initiates connection to phone.
 *
 * \param checkerror Whether we should check for error.
 */
void GSM_Init(gboolean checkerror)
{
	GSM_Error error;

	/* No checks on existing batch mode */
	if (batch && batchConn)
		return;

	/* Connect to phone */
	error = GSM_InitConnection(gsm, 3);
	if (checkerror)
		Print_Error(error);

	/* Check for batch mode */
	if (batch) {
		if (error == ERR_NONE) {
			batchConn = TRUE;
		}
	}
}

void GSM_Terminate(void)
{
	GSM_Error error;

	if (!batch) {
		error = GSM_TerminateConnection(gsm);
		Print_Error(error);
	}
}

void GetStartStop(int *start, int *stop, int num, int argc, char *argv[])
{
	int tmp;

	if (argc <= num) {
		printf_err("%s\n", _("More parameters required!"));
		Terminate(3);
	}

	*start = atoi(argv[num]);
	if (*start == 0) {
		printf_err("%s\n", _("Please enumerate locations from 1"));
		Terminate(3);
	}

	if (stop != NULL) {
		*stop = *start;
		if (argc >= num + 2)
			*stop = atoi(argv[num + 1]);
		if (*stop == 0) {
			printf_err("%s\n",
				   _("Please enumerate locations from 1"));
			Terminate(3);
		}
		if (*stop < *start) {
			printf_warn("%s\n",
				    _("Swapping start and end location"));
			tmp = *stop;
			*stop = *start;
			*start = tmp;
		}
	}
}

PRINTF_STYLE(1, 2)
gboolean answer_yes(const char *format, ...)
{
	int len;
	char ans[99];
	char buffer[1000];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer) - 1, format, ap);
	va_end(ap);

	while (1) {
		fprintf(stderr, "%s (%s/%s/%s/%s/%s) ", buffer,
			_("yes"), _("no"),
			_("ALL"), _("ONLY"), _("NONE"));
		if (always_answer_yes) {
			fprintf(stderr, "%s\n", _("YES (always)"));
			return TRUE;
		}
		if (always_answer_no) {
			fprintf(stderr, "%s\n", _("NO (always)"));
			return FALSE;
		}
		len = GetLine(stdin, ans, 99);
		if (len == -1)
			Terminate(3);
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (!strcmp(ans, _("NONE"))) {
			always_answer_no = TRUE;
			return FALSE;
		}
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (!strcmp(ans, _("ONLY"))) {
			always_answer_no = TRUE;
			return TRUE;
		}
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (!strcmp(ans, _("ALL"))) {
			always_answer_yes = TRUE;
			return TRUE;
		}
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (strcasecmp(ans, _("yes")) == 0)
			return TRUE;
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (strcasecmp(ans, _("no")) == 0)
			return FALSE;
	}
}

#ifdef GSM_ENABLE_BEEP
void GSM_PhoneBeep(void)
{
	error = PHONE_Beep(gsm);
	if (error != ERR_NOTSUPPORTED && error != ERR_NOTIMPLEMENTED)
		Print_Error(error);
}
#endif

/* How should editor hadle tabs in this file? Add editor commands here.
 * vim: noexpandtab sw=8 ts=8 sts=8:
 */
