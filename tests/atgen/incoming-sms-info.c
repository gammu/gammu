#include <gammu-message.h>
#include "test_helper.h"
#include "../../libgammu/phone/at/atgen.h"
#include "../../libgammu/gsmstate.h"

int is_empty(const char *buffer, size_t length)
{
  size_t i;

  if(buffer != NULL && length > 0)
    for(i = 0; i < length; ++i)
      if(buffer[i] != '\0') return FALSE;

  return TRUE;
}

void IncomingSMS(GSM_StateMachine * s, GSM_SMSMessage *sms, void *user_data)
{
  GSM_MultiSMSMessage msms;
  msms.SMS[0] = *sms;
  msms.Number = 1;

  GSM_GetSMS(s, &msms);
  if(user_data != NULL)
    memcpy(user_data, &msms.SMS[0], sizeof(GSM_SMSMessage));
}

void ignore_if_incoming_sms_disabled(void)
{
  GSM_Error error;
  GSM_SMSMessage sms;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_Protocol_Message msg;

  const char *event = "+CDSI: \"SR\",0\r";
  const char *responses[] = { "ERROR\r\n" };
  SET_RESPONSES(responses);
  bind_response_handling(s);

  puts(__func__);

  UNNEEDED(Priv);

  memset(&sms, 0, sizeof(sms));
  s->Phone.Data.EnableIncomingSMS = FALSE;
  s->Phone.Data.RequestID = ID_None;
  s->User.IncomingSMS = &IncomingSMS;
  s->User.IncomingSMSUserData = &sms;

  msg.Length = strlen(event);
  msg.Buffer = (char*)event;
  msg.Type = 0;

  s->Phone.Data.RequestMsg = &msg;
  error = ATGEN_DispatchMessage(s);
  test_result(error == ERR_NONE);
  test_result(is_empty((char*)&sms, sizeof(sms)));

  cleanup_state_machine(s);
}

void ignore_if_no_handler(void)
{
  GSM_Error error;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_Protocol_Message msg;

  const char *event = "+CDSI: \"SR\",0\r";
  const char *responses[] = { "ERROR\r\n" };
  SET_RESPONSES(responses);
  bind_response_handling(s);

  puts(__func__);

  UNNEEDED(Priv);

  s->Phone.Data.EnableIncomingSMS = TRUE;
  s->Phone.Data.RequestID = ID_None;

  msg.Length = strlen(event);
  msg.Buffer = (char*)event;
  msg.Type = 0;

  s->Phone.Data.RequestMsg = &msg;
  error = ATGEN_DispatchMessage(s);
  test_result(error == ERR_NONE);

  cleanup_state_machine(s);
}

void ignore_if_no_cmti_handler(void)
{
  GSM_Error error;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_Protocol_Message msg;

  const char *event = "+CMTI: \"SR\",0\r";
  const char *responses[] = { "ERROR\r\n" };
  SET_RESPONSES(responses);
  bind_response_handling(s);

  puts(__func__);

  UNNEEDED(Priv);

  s->Phone.Data.EnableIncomingSMS = TRUE;
  s->Phone.Data.RequestID = ID_None;

  msg.Length = strlen(event);
  msg.Buffer = (char*)event;
  msg.Type = 0;

  s->Phone.Data.RequestMsg = &msg;
  error = ATGEN_DispatchMessage(s);
  test_result(error == ERR_NONE);

  cleanup_state_machine(s);
}

void skip_if_memory_disabled(void)
{
  GSM_Error error;
  GSM_SMSMessage sms;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_Protocol_Message msg;

  const char *event = "+CDSI: \"SR\",0\r";
  const char *responses[] = { "ERROR\r\n" };
  SET_RESPONSES(responses);
  bind_response_handling(s);

  puts(__func__);

  memset(&sms, 0, sizeof(sms));
  s->Phone.Data.EnableIncomingSMS = TRUE;
  s->Phone.Data.RequestID = ID_None;
  s->User.IncomingSMS = &IncomingSMS;
  s->User.IncomingSMSUserData = &sms;

  msg.Length = strlen(event);
  msg.Buffer = (char*)event;
  msg.Type = 0;

  s->Phone.Data.RequestMsg = &msg;
  Priv->SRSMSMemory = AT_NOTAVAILABLE;
  error = ATGEN_DispatchMessage(s);
  test_result(error == ERR_NONE);
  test_result(is_empty((char*)&sms, sizeof(sms)));

  cleanup_state_machine(s);
}

void cdsi_sr_0(void)
{
  GSM_Error error;
  GSM_SMSMessage sms;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_Protocol_Message msg;

  const char *event = "+CDSI: \"SR\",0\r";

  const char *responses[] = {
      "+CPMS: (\"ME\",\"MT\",\"SM\",\"SR\"),(\"ME\",\"MT\",\"SM\"),(\"ME\",\"SM\")\r",
      "OK\r\n",
      "+CPMS: 0,255,0,255,0,255\r",
      "OK\r\n",
      "+CMGR: 1,,28\r",
      "0006D60B911326880736F4111011719551401110117195714000\r",
      "OK\r\n",
  };
  SET_RESPONSES(responses);
  bind_response_handling(s);

  puts(__func__);

  memset(&sms, 0, sizeof(sms));
  s->Phone.Data.EnableIncomingSMS = TRUE;
  s->Phone.Data.RequestID = ID_None;
  s->User.IncomingSMS = &IncomingSMS;
  s->User.IncomingSMSUserData = &sms;

  msg.Length = strlen(event);
  msg.Buffer = (char*)event;
  msg.Type = 0;

  s->Phone.Data.RequestMsg = &msg;
  Priv->SRSMSMemory = AT_AVAILABLE;
  error = ATGEN_DispatchMessage(s);
  test_result(error == ERR_NONE);
  test_result(sms.Location == 0);
  test_result(sms.Memory == MEM_SR);
  test_result(sms.PDU == SMS_Status_Report);

  cleanup_state_machine(s);
}

void cmti_sm_1(void)
{
  GSM_Error error;
  GSM_SMSMessage sms;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_Protocol_Message msg;

  const char *event = "+CMTI: \"SM\",1\r\n";

  const char *responses[] = {
      "+CPMS: (\"ME\",\"MT\",\"SM\",\"SR\"),(\"ME\",\"MT\",\"SM\",\"SR\"),(\"ME\",\"SM\")\r",
      "OK\r\n",
      "+CPMS: 0,255,0,255,0,255\r",
      "OK\r\n",
      "+CMGR: 1,,23\r",
      "07918497483252F0040B918496445078F700007121320144744004D4F29C0E\r",
      "OK\r\n",
  };
  SET_RESPONSES(responses);
  bind_response_handling(s);

  puts(__func__);

  memset(&sms, 0, sizeof(sms));
  s->Phone.Data.EnableIncomingSMS = TRUE;
  s->Phone.Data.RequestID = ID_None;
  s->User.IncomingSMS = &IncomingSMS;
  s->User.IncomingSMSUserData = &sms;

  msg.Length = strlen(event);
  msg.Buffer = (char*)event;
  msg.Type = 0;

  s->Phone.Data.RequestMsg = &msg;
  Priv->SIMSMSMemory = AT_AVAILABLE;
  error = ATGEN_DispatchMessage(s);
  test_result(error == ERR_NONE);
  test_result(sms.Location == 1);
  test_result(sms.Memory == MEM_SM);
  test_result(sms.PDU == SMS_Deliver);

  cleanup_state_machine(s);
}

int main(void)
{
  ignore_if_incoming_sms_disabled();
  ignore_if_no_handler();
  ignore_if_no_cmti_handler();
  skip_if_memory_disabled();
  cdsi_sr_0();
  cmti_sm_1();
}
