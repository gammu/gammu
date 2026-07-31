#include <gammu-message.h>
#include "test_helper.h"
#include "../../include/gammu-smsd.h"
#include "../../smsd/core.h"
#include "../../smsd/services/null.h"

void SMSD_IncomingSMSCallback(GSM_StateMachine *s, GSM_SMSMessage *sms, void *user_data);

static void init_sms(GSM_SMSMessage *sms, const char *text, int part, int parts)
{
  GSM_SetDefaultReceivedSMSData(sms);
  sms->PDU = SMS_Deliver;
  sms->State = SMS_UnRead;
  sms->InboxFolder = TRUE;
  sms->Coding = SMS_Coding_Default_No_Compression;
  sms->Class = -1;
  sms->Length = strlen(text);
  sms->DateTime.Year = 2026;
  sms->DateTime.Month = 7;
  sms->DateTime.Day = 31;
  sms->DateTime.Hour = 12;
  sms->DateTime.Minute = 34;
  sms->DateTime.Second = 56;
  EncodeUnicode(sms->Number, "+420123456789", 13);
  EncodeUnicode(sms->SMSC.Number, "+420987654321", 13);
  EncodeUnicode(sms->Text, text, strlen(text));

  if (parts > 1) {
    /* Composite headers stay UserUDH until GSM_LinkSMS derives these fields. */
    sms->UDH.Type = UDH_UserUDH;
    sms->UDH.Length = 9;
    sms->UDH.Text[0] = 8;
    sms->UDH.Text[1] = 0;
    sms->UDH.Text[2] = 3;
    sms->UDH.Text[3] = 42;
    sms->UDH.Text[4] = parts;
    sms->UDH.Text[5] = part;
    sms->UDH.Text[6] = 0x70;
    sms->UDH.Text[7] = 1;
    sms->UDH.Text[8] = 0;
    sms->UDH.ID8bit = -1;
    sms->UDH.ID16bit = -1;
    sms->UDH.PartNumber = -1;
    sms->UDH.AllParts = -1;
  }
}

int main(void)
{
  GSM_SMSDConfig *config = SMSD_NewConfig("test");
  GSM_SMSDStatus status;
  GSM_MultiSMSMessage *messages = calloc(6, sizeof(*messages));
  GSM_MultiSMSMessage *multipart;
  GSM_MultiSMSMessage *linked;
  GSM_MultiSMSMessage *distinct;
  GSM_MultiSMSMessage *repeated;
  GSM_MultiSMSMessage *canonical;
  GSM_MultiSMSMessage *report;
  int i;

  test_result(messages != NULL);
  multipart = &messages[0];
  linked = &messages[1];
  distinct = &messages[2];
  repeated = &messages[3];
  canonical = &messages[4];
  report = &messages[5];
  config->gsm = setup_state_machine();
  config->Service = &SMSDNull;
  config->connected = FALSE;
  config->receivefrequency = 15;
  config->loopsleep = 1;
  memset(&status, 0, sizeof(status));
  config->Status = &status;

  multipart->Number = 2;
  init_sms(&multipart->SMS[0], "first", 1, 2);
  init_sms(&multipart->SMS[1], "second", 2, 2);

  SMSD_IncomingSMSCallback(config->gsm, &multipart->SMS[0], config);
  SMSD_IncomingSMSCallback(config->gsm, &multipart->SMS[1], config);
  test_result(SMSD_AllSMSProcessed(config, multipart));

  /* Linker-derived UDH metadata must not change the raw-message identity. */
  *linked = *multipart;
  for (i = 0; i < linked->Number; i++) {
    linked->SMS[i].UDH.ID8bit = 42;
    linked->SMS[i].UDH.ID16bit = -1;
    linked->SMS[i].UDH.PartNumber = i + 1;
    linked->SMS[i].UDH.AllParts = linked->Number;
  }
  test_result(SMSD_AllSMSProcessed(config, linked));

  config->ProcessedSMSUsed = 0;
  SMSD_IncomingSMSCallback(config->gsm, &multipart->SMS[0], config);
  test_result(!SMSD_AllSMSProcessed(config, linked));

  distinct->Number = 1;
  init_sms(&distinct->SMS[0], "different", 0, 1);
  test_result(!SMSD_AllSMSProcessed(config, distinct));

  repeated->Number = 2;
  repeated->SMS[0] = multipart->SMS[0];
  repeated->SMS[1] = multipart->SMS[0];
  test_result(!SMSD_AllSMSProcessed(config, repeated));
  SMSD_IncomingSMSCallback(config->gsm, &multipart->SMS[0], config);
  test_result(SMSD_AllSMSProcessed(config, repeated));

  /* Deliver decoders disagree on class, SMSC, TP-RP, and SMSCTime. */
  config->ProcessedSMSUsed = 0;
  canonical->Number = 1;
  init_sms(&canonical->SMS[0], "canonical", 0, 1);
  canonical->SMS[0].ReplyViaSameSMSC = TRUE;
  SMSD_IncomingSMSCallback(config->gsm, &canonical->SMS[0], config);
  canonical->SMS[0].ReplyViaSameSMSC = FALSE;
  test_result(SMSD_AllSMSProcessed(config, canonical));
  canonical->SMS[0].Class = 1;
  test_result(SMSD_AllSMSProcessed(config, canonical));
  canonical->SMS[0].SMSC.Number[0] = 0;
  canonical->SMS[0].SMSC.Number[1] = 0;
  test_result(SMSD_AllSMSProcessed(config, canonical));
  canonical->SMS[0].SMSCTime.Year = 1999;
  canonical->SMS[0].SMSCTime.Second = 59;
  test_result(SMSD_AllSMSProcessed(config, canonical));
  canonical->SMS[0].Length++;
  test_result(SMSD_AllSMSProcessed(config, canonical));

  /* Detailed messages still use sender and timestamp to avoid collisions. */
  *distinct = *canonical;
  distinct->SMS[0].DateTime.Second++;
  test_result(!SMSD_AllSMSProcessed(config, distinct));

  /* A lossy no-details identity is not sufficient evidence for deletion. */
  memset(&canonical->SMS[0].DateTime, 0, sizeof(canonical->SMS[0].DateTime));
  canonical->SMS[0].Number[0] = 0;
  canonical->SMS[0].Number[1] = 0;
  canonical->SMS[0].UDH.Length = 0;
  canonical->SMS[0].UDH.Type = UDH_NoUDH;
  test_result(!SMSD_AllSMSProcessed(config, canonical));
  test_result(config->ProcessedSMSUsed == 1);

  /* Tokens remain available for two completed receive polls. */
  config->ReceivePollCount = SMSD_PROCESSED_SMS_POLL_CYCLES;
  test_result(!SMSD_AllSMSProcessed(config, distinct));
  test_result(config->ProcessedSMSUsed == 1);
  config->ReceivePollCount++;
  test_result(!SMSD_AllSMSProcessed(config, distinct));
  test_result(config->ProcessedSMSUsed == 0);

  /* Discharge time remains part of a status report's identity. */
  config->ProcessedSMSUsed = 0;
  report->Number = 1;
  init_sms(&report->SMS[0], "Delivered", 0, 1);
  report->SMS[0].PDU = SMS_Status_Report;
  report->SMS[0].SMSCTime.Year = 2026;
  report->SMS[0].SMSCTime.Second = 1;
  SMSD_IncomingSMSCallback(config->gsm, &report->SMS[0], config);
  report->SMS[0].SMSCTime.Second = 2;
  test_result(!SMSD_AllSMSProcessed(config, report));

  SMSD_FreeConfig(config);
  free(messages);
}
