/* Test for encoding SMS using AT driver */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../libgammu/protocol/protocol.h"	/* Needed for GSM_Protocol_Message */
#include "../libgammu/gsmstate.h"	/* Needed for state machine internals */
#include "../libgammu/gsmphones.h"	/* Phone data */
#include "../libgammu/misc/coding/coding.h"	/* SMS alphabet sizing */

#include "common.h"

extern GSM_Error ATGEN_MakeSMSFrame(GSM_StateMachine *s, GSM_SMSMessage *message, unsigned char *hexreq, size_t hexlength, int *current, size_t *length2);

#define BUFFER_SIZE 16384

static gboolean SMS_ExceedsUserDataLimit(const GSM_SMSMessage *sms)
{
	size_t source_length, consumed, septets, header_septets;
	size_t udh_length = sms->UDH.Type == UDH_NoUDH ? 0 : sms->UDH.Length;

	if (udh_length > GSM_MAX_8BIT_SMS_LENGTH) return TRUE;
	switch (sms->Coding) {
	case SMS_Coding_8bit:
		return sms->Length < 0 ||
		       (size_t)sms->Length > GSM_MAX_8BIT_SMS_LENGTH - udh_length;
	case SMS_Coding_Unicode_No_Compression:
		return UnicodeLength(sms->Text) >
		       (GSM_MAX_8BIT_SMS_LENGTH - udh_length) / 2;
	case SMS_Coding_Default_No_Compression:
		header_septets = (udh_length * 8 + 6) / 7;
		source_length = UnicodeLength(sms->Text);
		FindDefaultAlphabetLen(sms->Text, &consumed, &septets,
				       GSM_MAX_SMS_CHARS_LENGTH - header_septets);
		return consumed != source_length;
	default:
		return FALSE;
	}
}

int main(int argc, char **argv)
{
	GSM_Debug_Info *debug_info;
	GSM_Phone_ATGENData *Priv;
	GSM_Phone_Data *Data;
	unsigned char dumpbuffer[BUFFER_SIZE];
	FILE *f;
	size_t len;
	GSM_StateMachine *s;
	GSM_Error error;
	int current;
	size_t current2;
	unsigned char hexreq[1000];
	GSM_SMS_Backup *Backup;
	gboolean generate = FALSE;

	Backup = malloc(sizeof(GSM_SMS_Backup));
	if (Backup == NULL) {
		return 99;
	}

	/* Enable debugging */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Check parameters */
	if (argc != 3 && argc != 4) {
		printf("Not enough parameters!\nUsage: sms-at-encode message.backup message.dump\n");
		return 1;
	}

	/* Check for generating option */
	if (argc == 4 && strcmp(argv[3], "generate") == 0) {
		generate = TRUE;
	}

	/* Read message */
	error = GSM_ReadSMSBackupFile(argv[1], Backup);
	gammu_test_result(error, "GSM_ReadSMSBackupFile");

	if (!generate) {
		/* Open file */
		f = fopen(argv[2], "r");
		if (f == NULL) {
			printf("Could not open %s\n", argv[2]);
			return 1;
		}

		/* Read data */
		len = fread(dumpbuffer, 1, sizeof(dumpbuffer) - 1, f);
		if (!feof(f)) {
			printf("Could not read whole file %s\n", argv[2]);
			fclose(f);
			return 1;
		}

		/* Zero terminate data */
		dumpbuffer[len] = 0;

		/* Close file */
		fclose(f);
	}

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL) {
		printf("Could not allocate state machine!\n");
		return 1;
	}
	debug_info = GSM_GetDebug(s);
	GSM_SetDebugGlobal(TRUE, debug_info);
	GSM_SetDebugFileDescriptor(stderr, FALSE, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Initialize AT engine */
	Data = &s->Phone.Data;
	Data->ModelInfo = GetModelData(NULL, NULL, "unknown", NULL);
	Priv = &s->Phone.Data.Priv.ATGEN;
	Priv->ReplyState = AT_Reply_OK;
	Priv->SMSMode = SMS_AT_PDU;
	Priv->PhoneSMSMemory = AT_AVAILABLE;
	Priv->SIMSMSMemory = AT_AVAILABLE;

	/* Format SMS frame */
	error = ATGEN_MakeSMSFrame(s, Backup->SMS[0], hexreq, sizeof(hexreq), &current, &current2);
	if (error == ERR_INVALIDDATA && SMS_ExceedsUserDataLimit(Backup->SMS[0])) {
		GSM_FreeSMSBackup(Backup);
		GSM_FreeStateMachine(s);
		return 0;
	}
	gammu_test_result(error, "ATGEN_MakeSMSFrame");

	/* We don't need this anymore */
	GSM_FreeSMSBackup(Backup);

	/* Display message */
	if (generate) {
		/* Open file */
		f = fopen(argv[2], "w");
		if (f == NULL) {
			printf("Could not open %s\n", argv[2]);
			return 1;
		}

		/* Read data */
		len = fwrite(hexreq, 1, strlen(hexreq), f);
		if (len != strlen(hexreq)) {
			printf("Could not save %s\n", argv[2]);
			return 1;
		}

		/* Close file */
		fclose(f);
	} else {
		if (strcmp(hexreq, dumpbuffer) != 0) {
			printf("Encoded does not match with template!\n");
			printf("Encoded:  %s\n", hexreq);
			printf("Template: %s\n", dumpbuffer);
			return 1;
		}
	}

	/* Free state machine */
	GSM_FreeStateMachine(s);

	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
