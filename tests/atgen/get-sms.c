#include <gammu-message.h>
#include "test_helper.h"

GSM_Error ATGEN_GetSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms);

void get_sms_default_memory(void)
{
  GSM_Error error;
  GSM_MultiSMSMessage msms;
  GSM_SMSMessage *sms = &msms.SMS[0];
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  const char *responses[] = {
      "+CPMS: (\"ME\",\"SM\",\"SR\"),(\"ME\",\"SM\")\r",
      "OK\r\n",
      "+CPMS: 0,255,0,255,0,255\r",
      "OK\r\n",
      "+CMGR: 0,,23\r",
      "0006D60B911623480736F4111011719551401110117195714000\r",
      "OK\r\n",

      "ERROR\r\n"
  };
  SET_RESPONSES(responses);
  bind_response_handling(s);

  puts(__func__);

  UNNEEDED(Priv);

  memset(&msms, 0, sizeof(msms));
  sms->Location = 0;
  sms->Folder = 1;

  error = ATGEN_GetSMS(s, &msms);
  test_result(error == ERR_NONE);
  test_result(sms->Memory == MEM_SM);

  cleanup_state_machine(s);
}

void get_sms_requested_memory(void)
{
  GSM_Error error;
  GSM_MultiSMSMessage msms;
  GSM_SMSMessage *sms = &msms.SMS[0];
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  const char *responses[] = {
      "+CPMS: (\"ME\",\"SM\",\"SR\"),(\"ME\",\"SM\")\r",
      "OK\r\n",
      "+CPMS: 0,255,0,255,0,255\r",
      "OK\r\n",
      "+CMGR: 0,,23\r",
      "0006D60B919345980736F4111011719551401110117195714000\r",
      "OK\r\n",

      "ERROR\r\n"
  };
  SET_RESPONSES(responses);
  bind_response_handling(s);

  puts(__func__);

  UNNEEDED(Priv);

  memset(&msms, 0, sizeof(msms));
  sms->Memory = MEM_SR;
  sms->Location = 0;
  sms->Folder = 1;

  error = ATGEN_GetSMS(s, &msms);
  test_result(error == ERR_NONE);
  test_result(sms->Memory == MEM_SR);

  cleanup_state_machine(s);
}

int main(void)
{
  get_sms_default_memory();
  get_sms_requested_memory();
}
