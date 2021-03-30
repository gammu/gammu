#include <gammu-message.h>
#include "test_helper.h"
#include "../../include/gammu-smsd.h"
#include "../../smsd/core.h"
#include "../../libgammu/gsmstate.h"
#include "../../smsd/services/files.h"

GSM_Error SMSD_ConfigureLogging(GSM_SMSDConfig *Config, gboolean uselog);
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
  const char *event = "+CDS:25\r07914875215652F006360B914892347527F3817003018104808170030181448000\r";
  const char *responses[] = { "ERROR\r\n" };

  UNNEEDED(Priv);

  SET_RESPONSES(responses);
  bind_response_handling(s);

  config->debug_level = 255;
  config->logfilename = "stderr";
  config->gsm = s;
  config->loopsleep = 8;
  config->deliveryreport = "sms";
  config->gsm->opened = TRUE;
  SMSD_ConfigureLogging(config, TRUE);

  config->Service = &SMSDFiles;
  config->Service->ReadConfiguration(config);
  config->Service->Init(config);

  memset(&status, 0, sizeof(GSM_SMSDStatus));
  config->Status = &status;

  memset(&sms, 0, sizeof(sms));
  s->Phone.Data.EnableIncomingSMS = TRUE;
  s->Phone.Data.RequestID = ID_None;
  s->User.IncomingSMS = &SMSD_IncomingSMSCallback;
  s->User.IncomingSMSUserData = config;

  msg.Length = strlen(event);
  msg.Buffer = (char*)event;
  msg.Type = 0;

  s->Phone.Data.RequestMsg = &msg;
  error = ATGEN_DispatchMessage(s);
  test_result(error == ERR_NONE);
  test_result(status.Received == 1);

  SMSD_FreeConfig(config);
}
