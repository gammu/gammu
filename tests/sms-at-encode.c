/* Test for encoding SMS using AT driver */

#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../common/protocol/protocol.h" /* Needed for GSM_Protocol_Message */
#include "../common/gsmstate.h" /* Needed for state machine internals */
#include "../common/gsmphones.h" /* Phone data */

extern GSM_Error ATGEN_MakeSMSFrame(GSM_StateMachine *s, GSM_SMSMessage *message, unsigned char *hexreq, int *current, int *length2);

#define BUFFER_SIZE 16384

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
	int			current, current2;
	unsigned char		hexreq[1000];
	GSM_SMS_Backup		Backup;
	bool generate = false;

	/* Enable debugging */
	debug_info = GSM_GetGlobalDebug();
	GSM_SetDebugFileDescriptor(stderr, false, debug_info);
	GSM_SetDebugLevel("textall", debug_info);

	/* Check parameters */
	if (argc != 3 && argc != 4) {
		printf("Not enough parameters!\nUsage: sms-at-encode message.backup message.dump\n");
		return 1;
	}

	/* Check for generating option */
	if (argc == 4 && strcmp(argv[3], "generate") == 0) {
		generate = true;
	}

	/* Read message */
	error = GSM_ReadSMSBackupFile(argv[1], &Backup);
	if (error != ERR_NONE) {
		printf("Could not read SMS backup!\n");
		printf("%s\n", GSM_ErrorString(error));
		return error;
	}

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
	GSM_SetDebugGlobal(true, debug_info);
	GSM_SetDebugFileDescriptor(stderr, false, debug_info);
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
	error = ATGEN_MakeSMSFrame(s, Backup.SMS[0], hexreq, &current, &current2);
	if (error != ERR_NONE) {
		printf("Could not encode SMS!\n");
		printf("%s\n", GSM_ErrorString(error));
		return error;
	}

	/* We don't need this anymore */
	GSM_FreeSMSBackup(&Backup);

	/* Display message */
	if (generate) {
		/* Open file */
		f = fopen(argv[2], "w");
		if (f == NULL) {
			printf("Could not open %s\n", argv[2]);
			return 1;
		}

		/* Read data */
		len = fwrite(hexreq, strlen(hexreq), 1, f);

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
