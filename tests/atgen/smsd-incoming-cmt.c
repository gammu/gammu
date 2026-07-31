#include <gammu-message.h>
#include "test_helper.h"
#include "../../include/gammu-smsd.h"
#include "../../smsd/core.h"
#include "../../libgammu/gsmstate.h"
#include "../../smsd/services/null.h"

GSM_Error SMSD_ConfigureLogging(GSM_SMSDConfig *Config, gboolean uselog);
gboolean SMSD_CheckSMSStatus(GSM_SMSDConfig *Config);
void SMSD_IncomingSMSCallback(GSM_StateMachine *s, GSM_SMSMessage *sms, void *user_data);

static void dispatch_event(GSM_StateMachine *s, const char *event)
{
  GSM_Error error;
  GSM_Protocol_Message msg;

  msg.Length = strlen(event);
  msg.Buffer = (char*)event;
  msg.Type = 0;
  s->Phone.Data.RequestMsg = &msg;
  error = ATGEN_DispatchMessage(s);
  test_result(error == ERR_NONE);
}

static void run_scenario(const char *stored_pdu, gboolean stage_cmti,
                         int expected_received, size_t expected_cache)
{
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_SMSDStatus status;
  GSM_SMSDConfig *config = SMSD_NewConfig("test");
  const char *event = "+CMT: ,23\r07918497483252F0840B918496445078F700007121320144744004D4F29C0E\r";
  const char *notification = stage_cmti ? "+CMTI: \"ME\",1\r" : "\r";
  const char *responses[] = {
      /* Responses for SMSD_CheckSMSStatus and SMSD_ReadDeleteSMS. */
      "+CPMS: (\"ME\"),(\"ME\")\r",
      "OK\r\n",
      "+CPMS: 1,99,1,99\r",
      "OK\r\n",
      "+CPMS: 1,99,1,99\r",
      "OK\r\n",
      "+CMGL: 1,0,\"\",23\r",
      stored_pdu,
      notification,
      "OK\r\n",
      "+CPMS: 1,99,1,99\r",
      "OK\r\n",
      "+CMGR: 1,\"\",23\r",
      stored_pdu,
      "OK\r\n",
      "+CPMS: 1,99,1,99\r",
      "OK\r\n",
      "+CPMS: 1,99,1,99\r",
      "OK\r\n",
      "OK\r\n",
      "ERROR\r\n",
      "ERROR\r\n",
  };

  SET_RESPONSES(responses);
  bind_response_handling(s);

  config->debug_level = 255;
  config->logfilename = "stderr";
  config->gsm = s;
  config->loopsleep = 8;
  config->deliveryreport = "sms";
  config->gsm->opened = TRUE;
  config->connected = FALSE;
  SMSD_ConfigureLogging(config, TRUE);

  config->Service = &SMSDNull;
  config->Service->ReadConfiguration(config);
  config->Service->Init(config);

  memset(&status, 0, sizeof(GSM_SMSDStatus));
  config->Status = &status;

  s->Phone.Data.EnableIncomingSMS = TRUE;
  s->Phone.Data.RequestID = ID_None;
  s->User.IncomingSMS = &SMSD_IncomingSMSCallback;
  s->User.IncomingSMSUserData = config;
  Priv->SMSMemory = MEM_ME;

  dispatch_event(s, event);
  /* Direct-only devices must still process the full callback immediately. */
  test_result(status.Received == 1);
  test_result(config->ProcessedSMSUsed == 1);

  /* Polling either deduplicates the matching PDU or processes a distinct one. */
  test_result(SMSD_CheckSMSStatus(config));
  test_result(config->ReceivePollCount == 1);
  test_result(status.Received == expected_received);
  test_result(config->ProcessedSMSUsed == expected_cache);

  SMSD_FreeConfig(config);
}

static void run_multipart_delete_retry(gboolean partial_callback)
{
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_SMSDStatus status;
  GSM_SMSDConfig *config = SMSD_NewConfig("test");
  const char *pdu1 = "07918497483252F0440B918496445078F70008712132014474400B0800032A02017001000041\r";
  const char *pdu2 = "07918497483252F0440B918496445078F70008712132014474400B0800032A02027001000042\r";
  const char *event1 = "+CMT: ,30\r07918497483252F0440B918496445078F70008712132014474400B0800032A02017001000041\r";
  const char *event2 = "+CMT: ,30\r07918497483252F0440B918496445078F70008712132014474400B0800032A02027001000042\r";
  const char *responses[] = {
      /* Status check and message listing with both stored parts. */
      "+CPMS: 2,99,2,99\r", "OK\r\n",
      "+CPMS: 2,99,2,99\r", "OK\r\n",
      "+CMGL: 1,0,\"\",30\r", pdu1,
      "+CMGL: 2,0,\"\",30\r", pdu2, "OK\r\n",
      /* Delete part one successfully. */
      "+CMGR: 0,,30\r", pdu1, "OK\r\n", "OK\r\n",
      /* Fail while deleting part two. */
      "+CMGR: 0,,30\r", pdu2, "OK\r\n", "+CMS ERROR: 300\r\n",
  };
  const char *retry_responses[] = {
      /* Only part two remains, so the linked message is incomplete. */
      "+CPMS: 1,99,1,99\r", "OK\r\n",
      "+CPMS: 1,99,1,99\r", "OK\r\n",
      "+CMGL: 2,0,\"\",30\r", pdu2, "OK\r\n",
      "+CMGR: 0,,30\r", pdu2, "OK\r\n", "OK\r\n",
  };

  SET_RESPONSES(responses);
  bind_response_handling(s);

  config->debug_level = 255;
  config->logfilename = "stderr";
  config->gsm = s;
  config->loopsleep = 8;
  config->deliveryreport = "sms";
  config->gsm->opened = TRUE;
  config->connected = FALSE;
  SMSD_ConfigureLogging(config, TRUE);
  config->Service = &SMSDNull;
  config->Service->ReadConfiguration(config);
  config->Service->Init(config);

  memset(&status, 0, sizeof(status));
  config->Status = &status;
  s->Phone.Data.EnableIncomingSMS = TRUE;
  s->Phone.Data.RequestID = ID_None;
  s->User.IncomingSMS = &SMSD_IncomingSMSCallback;
  s->User.IncomingSMSUserData = config;
  Priv->PhoneSMSMemory = AT_AVAILABLE;
  Priv->SIMSMSMemory = AT_NOTAVAILABLE;
  Priv->SRSMSMemory = AT_NOTAVAILABLE;
  Priv->SMSMemory = MEM_ME;
  Priv->NumFolders = 1;

  dispatch_event(s, event1);
  if (!partial_callback) {
    dispatch_event(s, event2);
  }
  test_result(status.Received == (partial_callback ? 1 : 2));
  test_result(config->ProcessedSMSUsed == (partial_callback ? 1 : 2));

  test_result(!SMSD_CheckSMSStatus(config));
  test_result(config->ReceivePollCount == 0);
  /* The missing callback part is processed once, while the cached part is not replayed. */
  test_result(status.Received == 2);
  test_result(config->ProcessedSMSUsed == 1);

  SET_RESPONSES(retry_responses);
  test_result(SMSD_CheckSMSStatus(config));
  test_result(config->ReceivePollCount == 1);
  test_result(status.Received == 2);
  test_result(config->ProcessedSMSUsed == 0);

  SMSD_FreeConfig(config);
}

int main(void)
{
  /* A matching stored copy is deleted without being processed again. */
  run_scenario(
      "07918497483252F0840B918496445078F700007121320144744004D4F29C0E\r",
      TRUE, 1, 0);

  /* A same-second message with different binary text must not collide. */
  run_scenario(
      "07918497483252F0840B918496445078F700007121320144744004D4F29C0F\r",
      FALSE, 2, 1);

  /* Cached remaining parts bypass multipart waiting after a delete failure. */
  run_multipart_delete_retry(FALSE);

  /* Mixed multipart messages process only parts without successful callbacks. */
  run_multipart_delete_retry(TRUE);
}
