#include <gammu-message.h>
#include "test_helper.h"

void memories_not_enabled(void)
{
  GSM_Error error;
  GSM_MemoryType type;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  const char *responses[] = { "ERROR\r\n" };
  bind_response_handling(s);

  puts(__func__);

  type = MEM_ME;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, FALSE, ID_SetMemoryType);
  test_result(Priv->SMSMemoryWrite == FALSE);
  test_result(error == ERR_MEMORY_NOT_AVAILABLE);

  type = MEM_SM;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, TRUE, ID_SetMemoryType);
  test_result(Priv->SMSMemoryWrite == FALSE);
  test_result(error == ERR_MEMORY_NOT_AVAILABLE);

  type = MEM_SR;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, FALSE, ID_SetMemoryType);
  test_result(Priv->SMSMemoryWrite == FALSE);
  test_result(error == ERR_MEMORY_NOT_AVAILABLE);

  type = MEM_MT;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, TRUE, ID_SetMemoryType);
  test_result(Priv->SMSMemoryWrite == FALSE);
  test_result(error == ERR_MEMORY_NOT_AVAILABLE);

  cleanup_state_machine(s);
}

void invalid_memory_type(void)
{
  GSM_Error error;
  GSM_MemoryType type = (GSM_MemoryType)0;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  const char *responses[] = { "ERROR\r\n" };
  bind_response_handling(s);

  puts(__func__);

  Priv->PhoneSMSMemory = Priv->SRSMSMemory = Priv->SIMSMSMemory = AT_AVAILABLE;

  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, FALSE, ID_SetMemoryType);
  test_result(Priv->SMSMemoryWrite == FALSE);
  test_result(error == ERR_INVALID_OPERATION);

  type = MEM_INVALID;

  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, TRUE, ID_SetMemoryType);
  test_result(Priv->SMSMemoryWrite == FALSE);
  test_result(error == ERR_INVALID_OPERATION);

  cleanup_state_machine(s);
}

void enabled_for_reading(void)
{
  GSM_Error error;
  GSM_MemoryType type;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  const char *responses[] = { "OK\r\n" };
  bind_response_handling(s);

  puts(__func__);

  Priv->PhoneSMSMemory = AT_AVAILABLE;

  Priv->SMSMemory = (GSM_MemoryType)0;
  Priv->SMSMemoryWrite = FALSE;

  type = MEM_ME;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, FALSE, ID_SetMemoryType);
  test_result(Priv->SMSMemory == MEM_ME);
  test_result(Priv->SMSMemoryWrite == FALSE);
  test_result(error == ERR_NONE);

  Priv->SMSMemory = (GSM_MemoryType)0;
  Priv->SMSMemoryWrite = FALSE;
  type = MEM_MT;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, FALSE, ID_SetMemoryType);
  test_result(Priv->SMSMemory == MEM_MT);
  test_result(Priv->SMSMemoryWrite == FALSE);
  test_result(error == ERR_NONE);

  Priv->SMSMemory = (GSM_MemoryType)0;
  Priv->SMSMemoryWrite = FALSE;

  type = MEM_SM;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, FALSE, ID_SetMemoryType);
  test_result(Priv->SMSMemory == 0);
  test_result(Priv->SMSMemoryWrite == FALSE);
  test_result(error == ERR_MEMORY_NOT_AVAILABLE);

  Priv->SMSMemory = (GSM_MemoryType)0;
  Priv->SMSMemoryWrite = FALSE;

  type = MEM_ME;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, TRUE, ID_SetMemoryType);
  test_result(Priv->SMSMemory == 0);
  test_result(Priv->SMSMemoryWrite == FALSE);
  test_result(error == ERR_MEMORY_NOT_AVAILABLE);

  cleanup_state_machine(s);
}

void enabled_for_writing(void)
{
  GSM_Error error;
  GSM_MemoryType type;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  const char *responses[] = { "OK\r\n" };
  bind_response_handling(s);

  puts(__func__);

  Priv->PhoneSMSMemory = Priv->SRSMSMemory = Priv->SIMSMSMemory = AT_AVAILABLE;
  Priv->SRSaveSMS = AT_AVAILABLE;

  Priv->SMSMemory = (GSM_MemoryType)0;
  Priv->SMSMemoryWrite = FALSE;

  type = MEM_SR;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, TRUE, ID_SetMemoryType);
  test_result(Priv->SMSMemory == MEM_SR);
  test_result(Priv->SMSMemoryWrite == TRUE);
  test_result(error == ERR_NONE);

  Priv->SMSMemory = (GSM_MemoryType)0;
  Priv->SMSMemoryWrite = FALSE;

  type = MEM_SM;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, TRUE, ID_SetMemoryType);
  test_result(Priv->SMSMemory == 0);
  test_result(Priv->SMSMemoryWrite == FALSE);
  test_result(error == ERR_MEMORY_NOT_AVAILABLE);

  Priv->SIMSaveSMS = AT_AVAILABLE;
  Priv->SMSMemory = (GSM_MemoryType)0;
  Priv->SMSMemoryWrite = FALSE;

  type = MEM_MT;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, TRUE, ID_SetMemoryType);
  test_result(Priv->SMSMemory == MEM_MT);
  test_result(Priv->SMSMemoryWrite == TRUE);
  test_result(error == ERR_NONE);

  cleanup_state_machine(s);
}

void requested_already_set(void)
{
  GSM_Error error;
  GSM_MemoryType type;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  const char *responses[] = { "ERROR\r\n" };
  bind_response_handling(s);

  puts(__func__);

  Priv->SIMSMSMemory = Priv->SIMSaveSMS = AT_AVAILABLE;
  Priv->SRSMSMemory = Priv->SRSaveSMS = AT_AVAILABLE;

  Priv->SMSMemory = MEM_SM;
  Priv->SMSMemoryWrite = FALSE;

  type = MEM_SM;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, FALSE, ID_SetMemoryType);
  test_result(  Priv->SMSMemory == MEM_SM);
  test_result(Priv->SMSMemoryWrite == FALSE);
  test_result(error == ERR_NONE);

  Priv->SMSMemory = MEM_SR;
  Priv->SMSMemoryWrite = TRUE;

  type = MEM_SR;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, TRUE, ID_SetMemoryType);
  test_result(Priv->SMSMemory == MEM_SR);
  test_result(Priv->SMSMemoryWrite == TRUE);
  test_result(error == ERR_NONE);

  cleanup_state_machine(s);
}

int main(void)
{
  invalid_memory_type();
  memories_not_enabled();
  enabled_for_reading();
  enabled_for_writing();
  requested_already_set();
}
