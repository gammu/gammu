#include <gammu-message.h>
#include "test_helper.h"

GSM_Error ATGEN_GetSMS(GSM_StateMachine *s, GSM_MultiSMSMessage *sms);

/**
 * Test for text mode fallback when PDU mode parsing fails.
 *
 * This test reproduces the issue where a modem sends text mode SMS response
 * but gammu initially tries to parse it as PDU mode. The fix should detect
 * this and fall back to text mode parsing automatically.
 *
 * Reproduces issue with FT4232H and similar modems that send text mode
 * responses even when PDU mode is requested.
 */
void get_sms_text_mode_fallback(void)
{
  GSM_Error error;
  GSM_MultiSMSMessage msms;
  GSM_SMSMessage *sms = &msms.SMS[0];
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);

  /*
   * Simulate responses from a modem that uses text mode even though
   * gammu set it to PDU mode. This happens with some USB serial adapters
   * like FT4232H.
   */
  const char *responses[] = {
      "+CPMS: (\"ME\",\"SM\",\"SR\"),(\"ME\",\"SM\")\r",
      "OK\r\n",
      "+CPMS: 0,255,0,255,0,255\r",
      "OK\r\n",
      /* Text mode response instead of PDU mode - this is the key test case */
      "+CMGR: \"REC READ\",\"+393312345678\",,\"16/05/10,10:33:31+08\"\r",
      "Test message\r",
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

  /* This should not fail with UNKNOWNRESPONSE anymore */
  error = ATGEN_GetSMS(s, &msms);
  test_result(error == ERR_NONE);
  test_result(sms->Memory == MEM_SM);
  test_result(sms->State == SMS_Read);
  test_result(sms->PDU == SMS_Deliver);

  cleanup_state_machine(s);
}

/**
 * Test text mode fallback with unread message
 */
void get_sms_text_mode_fallback_unread(void)
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
      /* Text mode response with REC UNREAD */
      "+CMGR: \"REC UNREAD\",\"+393312345678\",,\"16/05/10,10:33:31+08\"\r",
      "Test unread message\r",
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
  test_result(sms->State == SMS_UnRead);
  test_result(sms->PDU == SMS_Deliver);

  cleanup_state_machine(s);
}

int main(void)
{
  get_sms_text_mode_fallback();
  get_sms_text_mode_fallback_unread();
  return 0;
}
