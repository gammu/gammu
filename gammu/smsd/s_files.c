
#include <string.h>

#include "../../common/misc/coding.h"
#include "smsdcore.h"

/* Save SMS from phone (called Inbox sms - it's in phone Inbox) somewhere */
static GSM_Error SMSDFiles_SaveInboxSMS(GSM_MultiSMSMessage sms)
{
	int 		i;
	unsigned char 	FileName[400],buffer[2];
	FILE 		*file;

	for (i=0;i<sms.Number;i++) {
		switch (sms.SMS[i].Coding) {
		case GSM_Coding_Unicode:
		case GSM_Coding_Default:
			sprintf(FileName,
				"%s_%02d%02d%02d_%02d%02d%02d_%02i.txt",
				DecodeUnicodeString(sms.SMS[i].Number),
				sms.SMS[i].DateTime.Year, sms.SMS[i].DateTime.Month,  sms.SMS[i].DateTime.Day,
				sms.SMS[i].DateTime.Hour, sms.SMS[i].DateTime.Minute, sms.SMS[i].DateTime.Second, i);
			file = fopen(FileName, "wb");
			if (file) {
				buffer[0] = 0xFE;
				buffer[1] = 0xFF;
				fwrite(buffer,1,2,file);
				fwrite(sms.SMS[i].Text,1,strlen(DecodeUnicodeString(sms.SMS[i].Text))*2,file);
				fclose(file);
			} else return GE_CANTOPENFILE;
			break;
		case GSM_Coding_8bit:
			sprintf(FileName,
				"%s_%02d%02d%02d_%02d%02d%02d_%02i.bin",
				DecodeUnicodeString(sms.SMS[i].Number),
				sms.SMS[i].DateTime.Year, sms.SMS[i].DateTime.Month,  sms.SMS[i].DateTime.Day,
				sms.SMS[i].DateTime.Hour, sms.SMS[i].DateTime.Minute, sms.SMS[i].DateTime.Second, i);
			file = fopen(FileName, "wb");
			if (file) {
				fwrite(sms.SMS[i].Text,1,sms.SMS[i].Length,file);
				fclose(file);
			} else return GE_CANTOPENFILE;
		}
	}
	return GE_NONE;
}

/* Find one multi SMS to sending and return it (or return GE_EMPTY)
 * There is also set ID for SMS
 */
static GSM_Error SMSDFiles_FindOutboxSMS(GSM_MultiSMSMessage *sms, char *ID)
{
	return GE_EMPTY;
}

/* After successfull sending SMS is moved to Sent Items */
static GSM_Error SMSDFiles_CopyOutboxSMS2Sent(char *ID)
{
	return GE_UNKNOWN;
}

GSM_SMSDeaemonService SMSDFiles = {
	NONEFUNCTION,			/* Init */
	SMSDFiles_SaveInboxSMS,
	SMSDFiles_FindOutboxSMS,
	SMSDFiles_CopyOutboxSMS2Sent
};
