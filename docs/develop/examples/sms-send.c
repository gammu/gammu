#include <gammu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

GSM_StateMachine *s;
INI_Section *cfg;
GSM_Error error;
char buffer[100];

/* Function to handle errors */
void error_handler()
{
	if (error != ERR_NONE) {
		printf("ERROR: %s\n", GSM_ErrorString(error));
		if (GSM_IsConnected(s))
			GSM_TerminateConnection(s);
		exit(error);
	}
}

int main(int argc UNUSED, char **argv UNUSED)
{
	GSM_SMSMessage sms;
	GSM_SMSC PhoneSMSC;
	char recipient_number[] = "+1234567890";
	char message_text[] = "Sample Gammu message";

	/* Prepare message */
	/* Cleanup the structure */
	memset(&sms, 0, sizeof(sms));
	/* Encode message text */
	EncodeUnicode(sms.Text, message_text, strlen(message_text));
	/* Encode recipient number */
	EncodeUnicode(sms.Number, recipient_number, strlen(recipient_number));
	/* We want to submit message */
	sms.PDU = SMS_Submit;
	/* We used default coding for text */
	sms.Coding = SMS_Coding_Default_No_Compression;
	/* Class 1 message (normal) */
	sms.Class = 1;

	/* Allocates state machine */
	s = GSM_AllocStateMachine();
	if (s == NULL)
		return 3;

	/* Find configuration file */
	error = GSM_FindGammuRC(&cfg);
	error_handler();

	/* Read it */
	error = GSM_ReadConfig(cfg, GSM_GetConfig(s, 0), 0);
	error_handler();

	/* We have one valid configuration */
	GSM_SetConfigNum(s, 1);

	/* Connect to phone */
	/* 3 means number of replies you want to wait for */
	error = GSM_InitConnection(s, 3);
	error_handler();

	/* We need to know SMSC number */
	PhoneSMSC.Location = 1;
	error = GSM_GetSMSC(s, &PhoneSMSC);
	error_handler();

	/* Set SMSC number in message */
	CopyUnicodeString(sms.SMSC.Number, PhoneSMSC.Number);

	/* Send message */
	error = GSM_SendSMS(s, &sms);
	error_handler();

	/* Terminate connection */
	error = GSM_TerminateConnection(s);
	error_handler();
	return 0;
}

/* Editor configuration
 * vim: noexpandtab sw=8 ts=8 sts=8 tw=72:
 */
