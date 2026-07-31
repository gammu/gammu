#include <gammu-message.h>
#include "test_helper.h"
#include "../../include/gammu-smsd.h"
#include "../../smsd/core.h"
#include "../../libgammu/gsmstate.h"
#include "../../smsd/services/null.h"

GSM_Error SMSD_ConfigureLogging(GSM_SMSDConfig *Config, gboolean uselog);
GSM_Error SMSD_ProcessSMSInfoCache(GSM_SMSDConfig *Config);
void SMSD_IncomingSMSCallback(GSM_StateMachine *s,  GSM_SMSMessage *sms, void *user_data);

int main(void)
{
  GSM_Error error;
  GSM_SMSMessage sms;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_SMSDStatus status;
  GSM_SMSDConfig *config = SMSD_NewConfig("test");
  GSM_Protocol_Message msg;

  /* This simulates a +CMTI notification for an incoming SMS in SM memory,
   * which is the case for Huawei E303/E3531 modems */
  const char *event = "+CMTI: \"SM\",1\r\n";
  const char *direct_event = "+CMT: ,23\r07918497483252F0040B918496445078F700007121320144744004D4F29C0E\r";
  const char *responses[] = {
      /* Response to AT+CPMS=? (get available memories) */
      "+CPMS: (\"ME\",\"MT\",\"SM\",\"SR\"),(\"ME\",\"MT\",\"SM\",\"SR\"),(\"ME\",\"SM\")\r",
      "OK\r\n",
      /* Response to AT+CPMS="SM" (set read memory to SM) */
      "+CPMS: 0,255,0,255,0,255\r",
      "OK\r\n",
      /* Response to AT+CMGR=1 (read message) */
      "+CMGR: 1,,23\r",
      "07918497483252F0040B918496445078F700007121320144744004D4F29C0E\r",
      "OK\r\n",
      /* First delete attempt re-reads the message and fails. */
      "+CMS ERROR: 300\r\n",
  };
  const char *retry_responses[] = {
      /* Response to AT+CMGR=1 (read cached notification again) */
      "+CMGR: 1,,23\r",
      "07918497483252F0040B918496445078F700007121320144744004D4F29C0E\r",
      "OK\r\n",
      /* Response to the delete pre-read */
      "+CMGR: 1,,23\r",
      "07918497483252F0040B918496445078F700007121320144744004D4F29C0E\r",
      "OK\r\n",
      /* Responses to AT+CPMS="SM","SM","SM" and AT+CMGD=1 */
      "OK\r\n",
      "OK\r\n",
  };

  UNNEEDED(Priv);

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

  memset(&sms, 0, sizeof(sms));
  s->Phone.Data.EnableIncomingSMS = TRUE;
  s->Phone.Data.RequestID = ID_None;
  s->User.IncomingSMS = &SMSD_IncomingSMSCallback;
  s->User.IncomingSMSUserData = config;
  Priv->SIMSMSMemory = AT_AVAILABLE;

  msg.Type = 0;

  /* Process a full callback first, as done by modems which both route and
   * store an incoming message. */
  msg.Length = strlen(direct_event);
  msg.Buffer = (char*)direct_event;
  s->Phone.Data.RequestMsg = &msg;
  error = ATGEN_DispatchMessage(s);
  test_result(error == ERR_NONE);
  test_result(status.Received == 1);

  msg.Length = strlen(event);
  msg.Buffer = (char*)event;
  s->Phone.Data.RequestMsg = &msg;
  error = ATGEN_DispatchMessage(s);
  test_result(error == ERR_NONE);

  /* Verify that the SMS info was cached (not ignored) */
  test_result(Priv->SMSInfoCache.cache_used == 1);
  test_result(Priv->SMSInfoCache.smsInfo_records[0].Memory == MEM_SM);
  test_result(Priv->SMSInfoCache.smsInfo_records[0].Location == 1);

  /* A failed deletion must not process the stored copy or consume its token. */
  error = SMSD_ProcessSMSInfoCache(config);
  test_result(error != ERR_NONE);
  test_result(status.Received == 1);
  test_result(Priv->SMSInfoCache.cache_used == 1);
  test_result(config->ProcessedSMSUsed == 1);

  /* A later retry deletes the stored copy without processing it again. */
  SET_RESPONSES(retry_responses);
  error = SMSD_ProcessSMSInfoCache(config);
  test_result(error == ERR_NONE);
  test_result(status.Received == 1);
  test_result(Priv->SMSInfoCache.cache_used == 0);
  test_result(config->ProcessedSMSUsed == 0);

  SMSD_FreeConfig(config);
}
