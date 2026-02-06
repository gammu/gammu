/* Test for AT+CPMS 3-parameter support */
#include <gammu-message.h>
#include <string.h>
#include <stdio.h>
#include "test_helper.h"

void test_three_param_cpms_enabled(void)
{
  GSM_Error error;
  GSM_MemoryType type;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  const char *responses[] = { "OK\r\n" };
  bind_response_handling(s);

  puts(__func__);

  /* Enable ME and SM memories and set CPMSReceiveMemory flag */
  Priv->PhoneSMSMemory = AT_AVAILABLE;
  Priv->PhoneSaveSMS = AT_AVAILABLE;
  Priv->SIMSMSMemory = AT_AVAILABLE;
  Priv->SIMSaveSMS = AT_AVAILABLE;
  Priv->CPMSReceiveMemory = TRUE;  /* Phone requires 3rd parameter */

  Priv->SMSMemory = (GSM_MemoryType)0;
  Priv->SMSMemoryWrite = FALSE;

  /* Test with writeable=TRUE should use 3 parameters */
  type = MEM_ME;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, TRUE, ID_SetMemoryType);
  test_result(Priv->SMSMemory == MEM_ME);
  test_result(Priv->SMSMemoryWrite == TRUE);
  test_result(error == ERR_NONE);

  /* Reset state */
  Priv->SMSMemory = (GSM_MemoryType)0;
  Priv->SMSMemoryWrite = FALSE;

  /* Test with writeable=FALSE should use 1 parameter */
  type = MEM_SM;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, FALSE, ID_SetMemoryType);
  test_result(Priv->SMSMemory == MEM_SM);
  test_result(Priv->SMSMemoryWrite == FALSE);
  test_result(error == ERR_NONE);

  cleanup_state_machine(s);
}

void test_three_param_cpms_disabled(void)
{
  GSM_Error error;
  GSM_MemoryType type;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  const char *responses[] = { "OK\r\n" };
  bind_response_handling(s);

  puts(__func__);

  /* Enable ME and SM memories but don't set CPMSReceiveMemory flag */
  Priv->PhoneSMSMemory = AT_AVAILABLE;
  Priv->PhoneSaveSMS = AT_AVAILABLE;
  Priv->SIMSMSMemory = AT_AVAILABLE;
  Priv->SIMSaveSMS = AT_AVAILABLE;
  Priv->CPMSReceiveMemory = FALSE;  /* Phone doesn't require 3rd parameter */

  Priv->SMSMemory = (GSM_MemoryType)0;
  Priv->SMSMemoryWrite = FALSE;

  /* Test with writeable=TRUE should use 2 parameters */
  type = MEM_ME;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, TRUE, ID_SetMemoryType);
  test_result(Priv->SMSMemory == MEM_ME);
  test_result(Priv->SMSMemoryWrite == TRUE);
  test_result(error == ERR_NONE);

  cleanup_state_machine(s);
}

int main(void)
{
  test_three_param_cpms_enabled();
  test_three_param_cpms_disabled();
  return 0;
}
