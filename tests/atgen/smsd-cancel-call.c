#include "test_helper.h"
#include "../../smsd/core.h"
#include "../../libgammu/gsmstate.h"
#include "../../include/gammu-message.h"
#include "../../include/gammu-smsd.h"
#include "../../smsd/services/files.h"

GSM_Error EventQueue_Push(GSM_StateMachine *s, EventBinding *binding);
GSM_Error ATGEN_BeforeDeferredEventHook(GSM_StateMachine *s);

GSM_Error SMSD_ConfigureLogging(GSM_SMSDConfig *Config, gboolean uselog);
void SMSD_IncomingCallCallback(GSM_StateMachine *s, GSM_Call *call, void *user_data);

void set_config(GSM_StateMachine *s, GSM_SMSDConfig *config)
{
  config->debug_level = 255;
  config->logfilename = "stdout";
  config->gsm = s;
  config->loopsleep = 8;
  config->hangupcalls = TRUE;
  config->gsm->opened = TRUE;
  SMSD_ConfigureLogging(config, TRUE);

  config->Service = &SMSDFiles;
  config->Service->ReadConfiguration(config);
  config->Service->Init(config);
}

void cancel_when_idle(void)
{
  GSM_Error error;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_SMSDStatus status;
  GSM_SMSDConfig *config = SMSD_NewConfig("test");
  GSM_Protocol_Message msg;
  int i;

  const char *events[] = {
    "RING\r",
    "+CLIP: \"+481239823991\",145,\"\",128,\"\",0\r",
  };
  int num_events = sizeof(events) / sizeof(*(events));

  const char *responses[] = {
    // AT+CLIP=1
    "OK\r\n",
    // AT+CRC=1
    "OK\r\n",
    // AT+CCWA=1
    "OK\r\n",
    // AT+CHUP
    "RING\r\n", // ignored event
    "OK\r\n",
  };

  SET_RESPONSES(responses);
  bind_response_handling(s);

  set_config(s, config);
  memset(&status, 0, sizeof(GSM_SMSDStatus));
  config->Status = &status;

  Priv->HasCHUP = TRUE;

  GSM_SetIncomingCall(config->gsm, TRUE);
  GSM_SetIncomingCallCallback(config->gsm, SMSD_IncomingCallCallback, config);

  s->Phone.Data.RequestID = ID_None;

  for(i = 0; i < num_events; ++i) {
    msg.Length = strlen(events[i]);
    msg.Buffer = (char*)events[i];
    msg.Type = 0;

    s->Phone.Data.RequestMsg = &msg;
    error = ATGEN_DispatchMessage(s);
    test_result(error == ERR_NONE)
    test_result(Priv->CancellingCall == FALSE)
    usleep(5000);
  }

  SMSD_FreeConfig(config);
}

void cancel_when_busy(void)
{
  GSM_Error error;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_SMSDStatus status;
  GSM_SMSDConfig *config = SMSD_NewConfig("test");
  GSM_SecurityCodeType security_status;

  const char *responses[] = {
    // AT+CLIP=1
    "OK\r\n",
    // AT+CRC=1
    "OK\r\n",
    // AT+CCWA=1
    "OK\r\n",
    // AT+CPIN?
    "RING\r\n",
    "+CLIP: \"+481239823991\",145,\"\",128,\"\",0\r\n",
    "+CPIN: READY\r",
    "OK\r\n",
    "RING\r\n",
    // AT+CHUP
    "OK\r\n"
  };

  SET_RESPONSES(responses);
  bind_response_handling(s);

  set_config(s, config);
  memset(&status, 0, sizeof(GSM_SMSDStatus));
  config->Status = &status;

  Priv->HasCHUP = TRUE;

  GSM_SetIncomingCall(config->gsm, TRUE);
  GSM_SetIncomingCallCallback(config->gsm, SMSD_IncomingCallCallback, config);

  s->Phone.Data.RequestID = ID_None;

  error = GSM_GetSecurityStatus(s, &security_status);
  test_result(error == ERR_NONE)
  test_result(Priv->CancellingCall == FALSE)

  SMSD_FreeConfig(config);
}

void cancel_on_remote_end(void)
{
  GSM_Error error;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_SMSDStatus status;
  GSM_SMSDConfig *config = SMSD_NewConfig("test");
  GSM_SecurityCodeType security_status;
  GSM_Call call;

  const char *responses[] = {
    // AT+CLIP=1
    "OK\r\n",
    // AT+CRC=1
    "OK\r\n",
    // AT+CCWA=1
    "OK\r\n",
    // AT+CPIN?
    "RING\r\n",
    "+CLIP: \"XXXXXXXXXXXXX\",145,\"\",128,\"\",0\r\n",
    "NO CARRIER\r\n",
    "RING\r\n",
    "+CLIP: \"+481239823991\",145,\"\",128,\"\",0\r\n",
    "+CPIN: READY\r",
    "OK\r\n",
    "RING\r\n",
    // AT+CHUP
    "OK\r\n"
  };

  SET_RESPONSES(responses);
  bind_response_handling(s);

  set_config(s, config);
  memset(&status, 0, sizeof(GSM_SMSDStatus));
  config->Status = &status;

  Priv->HasCHUP = TRUE;

  GSM_SetIncomingCall(config->gsm, TRUE);
  GSM_SetIncomingCallCallback(config->gsm, SMSD_IncomingCallCallback, config);

  memset(&call, 0, sizeof(call));
  s->Phone.Data.RequestID = ID_GetSecurityStatus;

  call.PhoneNumber[1] = 'X';
  GSM_DeferIncomingCallEvent(s, &call, ATGEN_BeforeDeferredEventHook);

  s->Phone.Data.RequestID = ID_None;

  error = GSM_GetSecurityStatus(s, &security_status);
  test_result(error == ERR_NONE)
  test_result(Priv->CancellingCall == FALSE)

  SMSD_FreeConfig(config);
}

int main(void)
{
  cancel_when_idle();
  cancel_when_busy();
  cancel_on_remote_end();
}
