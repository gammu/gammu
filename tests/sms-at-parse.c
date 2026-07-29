/* Test for decoding SMS on AT driver */

#include <gammu.h>
#include <gammu-smsd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#endif

#include "../libgammu/misc/array.h"
#include "../libgammu/protocol/protocol.h"	/* Needed for GSM_Protocol_Message */
#include "../libgammu/gsmstate.h"	/* Needed for state machine internals */
#include "../libgammu/gsmphones.h"	/* Phone data */

#include "../helper/message-display.h"

#include "common.h"

extern GSM_Error ATGEN_ReplyGetSMSMessage(GSM_Protocol_Message *msg, GSM_StateMachine * s);
extern void SMSD_RunOnEnvironment(GSM_MultiSMSMessage *sms, GSM_SMSDConfig *Config, const GSM_StringArray *SentIDs, gboolean is_receive);
#define BUFFER_SIZE 16384

static void check_environment(const char *assignment)
{
	const char *separator;
	const char *expected;
	const char *actual;
	char name[100];
	size_t name_length;
#ifdef WIN32
	char actual_buffer[100];
	DWORD length;
#endif

	separator = strchr(assignment, '=');
	if (separator == NULL) {
		fprintf(stderr, "Invalid environment expectation: %s\n", assignment);
		exit(2);
	}
	name_length = separator - assignment;
	if (name_length == 0 || name_length >= sizeof(name)) {
		fprintf(stderr, "Invalid environment variable name: %s\n", assignment);
		exit(2);
	}
	memcpy(name, assignment, name_length);
	name[name_length] = 0;
	expected = separator + 1;

#ifdef WIN32
	actual_buffer[0] = 0;
	SetLastError(ERROR_SUCCESS);
	length = GetEnvironmentVariableA(name, actual_buffer, sizeof(actual_buffer));
	if (length >= sizeof(actual_buffer) ||
	    (length == 0 && GetLastError() == ERROR_ENVVAR_NOT_FOUND)) {
		actual = NULL;
	} else {
		actual = actual_buffer;
	}
#else
	actual = getenv(name);
#endif

	if (actual == NULL || strcmp(actual, expected) != 0) {
		fprintf(stderr, "Environment mismatch for %s:\nexpected: %s\nactual: %s\n",
			name, expected, actual == NULL ? "(unset)" : actual);
		exit(2);
	}
}

int main(int argc, char **argv)
{
	GSM_Debug_Info *debug_info;
	GSM_Phone_ATGENData *Priv;
	GSM_Phone_Data *Data;
	unsigned char buffer[BUFFER_SIZE];
	FILE *f;
	size_t len;
	GSM_StateMachine *s;
	GSM_Protocol_Message msg;
	GSM_Error error;
	GSM_MultiSMSMessage sms;
	GSM_SMSDConfig *smsd;
	const char *mode = "PDU";
	int first_expectation = 2;
	int i;
#if 0
	GSM_SMS_Backup bkp;
#endif

	/* Check parameters */
	if (argc >= 3 &&
	    (strcmp(argv[2], "PDU") == 0 ||
	     strcmp(argv[2], "TXT") == 0 ||
	     strcmp(argv[2], "TXTDETAIL") == 0)) {
		mode = argv[2];
		first_expectation = 3;
	}
	if (argc < 2) {
		printf("Not enough parameters!\n");
		printf("Usage: sms-at-parse comm.dump [PDU|TXT|TXTDETAIL] [NAME=VALUE]...\n");
		return 1;
	}

	/* Open file */
	f = fopen(argv[1], "r");
	if (f == NULL) {
		printf("Could not open %s\n", argv[1]);
		return 1;
	}

	/* Read data */
	len = fread(buffer, 1, sizeof(buffer) - 1, f);
	if (!feof(f)) {
		printf("Could not read whole file %s\n", argv[1]);
		fclose(f);
		return 1;
	}

	smsd = SMSD_NewConfig("test");
	/* Zero terminate data */
	buffer[len] = 0;

	/* Close file */
	fclose(f);

	/* Configure state machine */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	test_result(s != NULL);

	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(TRUE, debug_info);

	/* Initialize AT engine */
	Data = &s->Phone.Data;
	Data->ModelInfo = GetModelData(NULL, NULL, "unknown", NULL);
	Priv = &s->Phone.Data.Priv.ATGEN;
	Priv->ReplyState = AT_Reply_OK;
	Priv->Charset = AT_CHARSET_GSM;
	if (strcmp(mode, "TXT") == 0) {
		Priv->SMSMode = SMS_AT_TXT;
		Priv->SMSTextDetails = FALSE;
	} else if (strcmp(mode, "TXTDETAIL") == 0) {
		Priv->SMSMode = SMS_AT_TXT;
		Priv->SMSTextDetails = TRUE;
	} else {
		Priv->SMSMode = SMS_AT_PDU;
	}

	/* Init message */
	msg.Type = 0;
	msg.Length = len;
	msg.Buffer = buffer;
	SplitLines(msg.Buffer, msg.Length, &Priv->Lines, "\x0D\x0A", 2, "\"", 1, TRUE);

	/* Pointer to store message */
	s->Phone.Data.GetSMSMessage = &sms;

	/* Parse it */
	error = ATGEN_ReplyGetSMSMessage(&msg, s);
	sms.SMS[0].Memory = MEM_SM;

#if 0
	bkp.SMS[0] = &sms.SMS[0];
	bkp.SMS[1] = NULL;

	GSM_AddSMSBackupFile("/tmp/back", &bkp);
#endif

	/* Display message */
	if (error == ERR_NONE) {
		DisplayMultiSMSInfo(&sms, FALSE, TRUE, NULL, NULL);
		DisplayMultiSMSInfo(&sms, TRUE, TRUE, NULL, NULL);
		printf("Parts: %d, count: %d, ID16: %d, ID8: %d\n", sms.SMS[0].UDH.AllParts, sms.Number, sms.SMS[0].UDH.ID16bit, sms.SMS[0].UDH.ID8bit);

		SMSD_RunOnEnvironment(&sms, smsd, NULL, TRUE);
		for (i = first_expectation; i < argc; i++) {
			check_environment(argv[i]);
		}
	}

	/* This is normally done by ATGEN_Terminate */
	FreeLines(&Priv->Lines);
	GetLineString(NULL, NULL, 0);

	/* Free state machine */
	GSM_FreeStateMachine(s);

	gammu_test_result(error, "ATGEN_ReplyGetSMSMessage");

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
