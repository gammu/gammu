#include <gammu-message.h>
#include "test_helper.h"
#include "../../libgammu/phone/at/atgen.h"
#include "../../libgammu/gsmstate.h"

int is_empty(const char *buffer, const int length)
{
  int i;

  if(buffer != NULL && length > 0)
    for(i = 0; i < length; ++i)
      if(buffer[i] != '\0')
        return FALSE;

  return TRUE;
}

void IncomingSMS(GSM_StateMachine * s, GSM_SMSMessage *sms, void *user_data)
{
  if(user_data != NULL)
    memcpy(user_data, sms, sizeof(GSM_SMSMessage));
}

void ignore_if_incoming_sms_disabled(void)
{
  GSM_Error error;
  GSM_SMSMessage sms;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_Protocol_Message msg;

  const char *event = "+CDS:25\rBAD\r";
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

void ignore_if_no_cds_handler(void)
{
  GSM_Error error;
  GSM_SMSMessage sms;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_Protocol_Message msg;

  const char *event = "+CDS:25\rBAD\r";
  const char *responses[] = { "ERROR\r\n" };
  SET_RESPONSES(responses);
  bind_response_handling(s);

  puts(__func__);

  UNNEEDED(Priv);

  memset(&sms, 0, sizeof(sms));
  s->Phone.Data.EnableIncomingSMS = TRUE;
  s->Phone.Data.RequestID = ID_None;

  msg.Length = strlen(event);
  msg.Buffer = (char*)event;
  msg.Type = 0;

  s->Phone.Data.RequestMsg = &msg;
  error = ATGEN_DispatchMessage(s);
  test_result(error == ERR_NONE);
  test_result(is_empty((char*)&sms, sizeof(sms)));

  cleanup_state_machine(s);
}

void process_cds(void)
{
  GSM_Error error;
  GSM_SMSMessage sms;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_Protocol_Message msg;

  const char *event = "+CDS:25\r07914875215652F006360B914892347527F3817003018104808170030181448000\r";
  const char *responses[] = { "ERROR\r\n" };
  SET_RESPONSES(responses);
  bind_response_handling(s);

  puts(__func__);

  UNNEEDED(Priv);

  memset(&sms, 0, sizeof(sms));
  s->Phone.Data.EnableIncomingSMS = TRUE;
  s->Phone.Data.RequestID = ID_None;
  s->User.IncomingSMS = &IncomingSMS;
  s->User.IncomingSMSUserData = &sms;

  msg.Length = strlen(event);
  msg.Buffer = (char*)event;
  msg.Type = 0;

  s->Phone.Data.RequestMsg = &msg;
  error = ATGEN_DispatchMessage(s);
  test_result(error == ERR_NONE);
  test_result(sms.PDU == SMS_Status_Report);

  cleanup_state_machine(s);
}

int main(void)
{
  ignore_if_incoming_sms_disabled();
  ignore_if_no_cds_handler();
  process_cds();
}
