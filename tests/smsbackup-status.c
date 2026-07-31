/**
 * Tests delivery status persistence in SMS backups.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gammu.h>

#include "common.h"

int main(int argc, char **argv)
{
	GSM_SMS_Backup *backup, *loaded;
	GSM_SMSMessage sms;
	GSM_Error error;
	const char *path;

	test_result(argc == 4);
	path = argv[1];
	remove(path);

	backup = malloc(sizeof(GSM_SMS_Backup));
	loaded = malloc(sizeof(GSM_SMS_Backup));
	test_result(backup != NULL);
	test_result(loaded != NULL);

	GSM_ClearSMSBackup(backup);
	GSM_SetDefaultSMSData(&sms);
	sms.PDU = SMS_Status_Report;
	sms.DeliveryStatus = 0x40;
	EncodeUnicode(sms.Number, "+420123456", strlen("+420123456"));
	EncodeUnicode(sms.Text, "Failed", strlen("Failed"));
	sms.Length = UnicodeLength(sms.Text);
	backup->SMS[0] = &sms;
	backup->SMS[1] = NULL;

	error = GSM_AddSMSBackupFile(path, backup);
	gammu_test_result(error, "GSM_AddSMSBackupFile");
	error = GSM_ReadSMSBackupFile(path, loaded);
	gammu_test_result(error, "GSM_ReadSMSBackupFile");
	test_result(loaded->SMS[0] != NULL);
	test_result(loaded->SMS[0]->PDU == SMS_Status_Report);
	test_result(loaded->SMS[0]->DeliveryStatus == 0x40);
	GSM_FreeSMSBackup(loaded);

	test_result(remove(path) == 0);
	error = GSM_ReadSMSBackupFile(argv[2], loaded);
	gammu_test_result(error, "GSM_ReadSMSBackupFile legacy status");
	test_result(loaded->SMS[0] != NULL);
	test_result(loaded->SMS[0]->PDU == SMS_Status_Report);
	test_result(loaded->SMS[0]->DeliveryStatus == 0x40);
	GSM_FreeSMSBackup(loaded);

	error = GSM_ReadSMSBackupFile(argv[3], loaded);
	gammu_test_result(error, "GSM_ReadSMSBackupFile 8-bit status");
	test_result(loaded->SMS[0] != NULL);
	test_result(loaded->SMS[0]->PDU == SMS_Status_Report);
	test_result(loaded->SMS[0]->Coding == SMS_Coding_8bit);
	test_result(loaded->SMS[0]->DeliveryStatus == 0x00);
	GSM_FreeSMSBackup(loaded);
	free(loaded);
	free(backup);

	return 0;
}
