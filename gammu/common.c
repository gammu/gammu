#include <string.h>
#include <stdarg.h>
#include <signal.h>
#include <stdlib.h>

#include "../common/misc/locales.h"

#include "common.h"
#include "formats.h"

GSM_StateMachine *gsm;

bool always_answer_yes = false;
bool always_answer_no = false;
volatile bool gshutdown = false;
bool phonedb = false;
bool batch = false;
bool batchConn = false;

int printf_err(const char *format, ...)
{
	va_list ap;
	int ret;

	/* l10n: Generic prefix for error messages */
	printf("%s: ", _("Error"));

	va_start(ap, format);
	ret = vprintf(format, ap);
	va_end(ap);

	return ret;
}

int printf_warn(const char *format, ...)
{
	va_list ap;
	int ret;

	/* l10n: Generic prefix for warning messages */
	printf("%s: ", _("Warning"));

	va_start(ap, format);
	ret = vprintf(format, ap);
	va_end(ap);

	return ret;
}

int printf_info(const char *format, ...)
{
	va_list ap;
	int ret;

	/* l10n: Generic prefix for informational messages */
	printf("%s: ", _("Information"));

	va_start(ap, format);
	ret = vprintf(format, ap);
	va_end(ap);

	return ret;
}

void interrupt(int sign)
{
	signal(sign, SIG_IGN);
	gshutdown = true;
}

void PrintSecurityStatus()
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

void Print_Error(GSM_Error error)
{
	if (error != ERR_NONE) {
		printf("%s\n", GSM_ErrorString(error));
		/* Check for security error */
		if (error == ERR_SECURITYERROR) {
			printf(LISTFORMAT, _("Security status"));
			PrintSecurityStatus();
		}

		/* Disconnect from phone */
		if (GSM_IsConnected(gsm)) {
			GSM_TerminateConnection(gsm);
		}

		/* Free state machine */
		GSM_FreeStateMachine(gsm);

		exit(-1);
	}
}

/**
 * Converts memory type from string.
 *
 * \param type String with memory type.
 *
 * \return Parsed memory type or 0 on failure.
 */
GSM_MemoryType MemoryTypeFromString(const char *type)
{
	if (strcasecmp(type, "ME") == 0)
		return MEM_ME;
	if (strcasecmp(type, "SM") == 0)
		return MEM_SM;
	if (strcasecmp(type, "ON") == 0)
		return MEM_ON;
	if (strcasecmp(type, "DC") == 0)
		return MEM_DC;
	if (strcasecmp(type, "RC") == 0)
		return MEM_RC;
	if (strcasecmp(type, "MC") == 0)
		return MEM_MC;
	if (strcasecmp(type, "MT") == 0)
		return MEM_MT;
	if (strcasecmp(type, "FD") == 0)
		return MEM_FD;
	if (strcasecmp(type, "VM") == 0)
		return MEM_VM;
	if (strcasecmp(type, "SL") == 0)
		return MEM_SL;
	return 0;
}

/**
 * Initiates connection to phone.
 *
 * \param checkerror Whether we should check for error.
 */
void GSM_Init(bool checkerror)
{
	GSM_File PhoneDB;
	char model[GSM_MAX_MODEL_LENGTH];
	char current_version[GSM_MAX_VERSION_LENGTH];
	char buff[50 + GSM_MAX_MODEL_LENGTH];
	char latest_version[GSM_MAX_VERSION_LENGTH];
	size_t pos = 0, oldpos = 0, i;
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
			batchConn = true;
		}
	}

	/* No phonedb check? */
	if (!phonedb)
		return;

	/* Get model information */
	error = GSM_GetModel(gsm, model);
	Print_Error(error);

	/* Empty string */
	latest_version[0] = 0;

	/* Request information from phone db */
	sprintf(buff, "support/phones/phonedbxml.php?model=%s", model);
	PhoneDB.Buffer = NULL;
	if (!GSM_ReadHTTPFile("www.gammu.org", buff, &PhoneDB))
		return;

	/* Parse reply */
	while (pos < PhoneDB.Used) {
		if (PhoneDB.Buffer[pos] != 10) {
			pos++;
			continue;
		}
		PhoneDB.Buffer[pos] = 0;
		if (strstr(PhoneDB.Buffer + oldpos, "<firmware>") == NULL) {
			pos++;
			oldpos = pos;
			continue;
		}
		sprintf(latest_version, "%s", strstr(PhoneDB.Buffer + oldpos, "<version>") + 9);
		for (i = 0; i < strlen(latest_version); i++) {
			if (latest_version[i] == '<') {
				latest_version[i] = 0;
				break;
			}
		}
		pos++;
		oldpos = pos;
	}
	free(PhoneDB.Buffer);

	/* Did we find something? */
	if (latest_version[0] == 0) {
		return;
	}

	/* Get phone firmware version */
	error = GSM_GetFirmware(gsm, current_version, NULL, NULL);
	Print_Error(error);

	/* Compare versions */
	if (!GSM_IsNewerVersion(latest_version, current_version))
		return;

	/* Print information to user */
	printf_info(_("Never version of firmware is available!\n"));
	printf_info(_("Latest version is %s and you run %s.\n"),
			latest_version, current_version);
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
		exit(-1);
	}

	*start = atoi(argv[num]);
	if (*start == 0) {
		printf_err("%s\n", _("Please enumerate locations from 1"));
		exit(-1);
	}

	if (stop != NULL) {
		*stop = *start;
		if (argc >= num + 2)
			*stop = atoi(argv[num + 1]);
		if (*stop == 0) {
			printf_err("%s\n",
				   _("Please enumerate locations from 1"));
			exit(-1);
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

bool answer_yes(const char *format, ...)
{
	int len;
	char ans[99];
	char buffer[1000];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer) - 1, format, ap);
	va_end(ap);

	while (1) {
		fprintf(stderr, "%s _(%s/%s/%s/%s/%s)", buffer,
			_("yes"), _("no"),
			_("ALL"), _("ONLY"), _("NONE"));
		if (always_answer_yes) {
			fprintf(stderr, "%s\n", _("YES (always)"));
			return true;
		}
		if (always_answer_no) {
			fprintf(stderr, "%s\n", _("NO (always)"));
			return false;
		}
		len = GetLine(stdin, ans, 99);
		if (len == -1)
			exit(-1);
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (!strcmp(ans, _("NONE"))) {
			always_answer_no = true;
			return false;
		}
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (!strcmp(ans, _("ONLY"))) {
			always_answer_no = true;
			return true;
		}
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (!strcmp(ans, _("ALL"))) {
			always_answer_yes = true;
			return true;
		}
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (strcasecmp(ans, _("yes")) == 0)
			return true;
		/* l10n: Answer to (yes/no/ALL/ONLY/NONE) question */
		if (strcasecmp(ans, _("no")) == 0)
			return false;
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
