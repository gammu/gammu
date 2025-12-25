#include <gammu-message.h>
#include "test_helper.h"
#include "../../include/gammu-smsd.h"
#include "../../smsd/core.h"
#include "../../libgammu/gsmstate.h"
#include "../../smsd/services/files.h"

void SMSD_IncomingUSSDCallback(GSM_StateMachine *sm, GSM_USSDMessage *ussd, void *user_data);

void receive_ussd_with_code_only(void)
{
  GSM_Error error;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_SMSDStatus status;
  GSM_SMSDConfig *config = SMSD_NewConfig("test");
  GSM_Protocol_Message msg;
  const char *event = "+CUSD: 2";

  UNNEEDED(Priv);

  error = SMSD_ReadConfig("smsd.cfg", config, TRUE);
  test_result(error == ERR_NONE);
  SMSD_EnableGlobalDebug(config);

  memset(&status, 0, sizeof(GSM_SMSDStatus));
  config->Status = &status;

  s->User.IncomingUSSD = &SMSD_IncomingUSSDCallback;
  s->User.IncomingUSSDUserData = config;
  s->Phone.Data.EnableIncomingUSSD = TRUE;
  s->Phone.Data.RequestID = ID_None;

  msg.Length = strlen(event);
  msg.Buffer = (char*)event;
  msg.Type = 0;

  s->Phone.Data.RequestMsg = &msg;
  error = ATGEN_DispatchMessage(s);
  test_result(error == ERR_NONE);
  test_result(status.Received == 1);

  SMSD_FreeConfig(config);
}

int main(void)
{
  receive_ussd_with_code_only();
}
