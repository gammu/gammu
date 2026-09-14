/* Test for AT+CPMS 3-parameter support */
#include <gammu-message.h>
#include <string.h>
#include <stdio.h>
#include "test_helper.h"
#include "../../libgammu/gsmstate.h"

extern GSM_Error ATGEN_SetSMSMemory(GSM_StateMachine *s, gboolean SIM, gboolean for_write, gboolean outbox);

extern GSM_Error ATGEN_ReplyGetSMSMemories(GSM_Protocol_Message *msg, GSM_StateMachine *s);

static void parse_memories(GSM_StateMachine *s, const char *capabilities)
{
  GSM_Phone_ATGENData *Priv = &s->Phone.Data.Priv.ATGEN;
  GSM_Protocol_Message msg;
  char buffer[256];

  snprintf(buffer, sizeof(buffer), "AT+CPMS=?\r\n+CPMS: %s\r\nOK\r\n", capabilities);
  msg.Type = 0;
  msg.Buffer = buffer;
  msg.Length = strlen(buffer);
  Priv->ReplyState = AT_Reply_OK;
  SplitLines(msg.Buffer, msg.Length, &Priv->Lines, "\x0D\x0A", 2, "\"", 1, TRUE);
  test_result(ATGEN_ReplyGetSMSMemories(&msg, s) == ERR_NONE);
}

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
  Priv->CPMSReceiveMemories[MEM_ME] = TRUE;
  Priv->CPMSReceiveMemories[MEM_SM] = TRUE;
  Priv->CPMSReceiveMemory = TRUE;  /* Phone supports 3rd parameter */

  Priv->SMSMemory = (GSM_MemoryType)0;
  Priv->SMSMemoryWrite = FALSE;

  /* Test with writeable=TRUE should use 3 parameters */
  type = MEM_ME;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, TRUE, ID_SetMemoryType);
  test_result(Priv->SMSMemory == MEM_ME);
  test_result(Priv->SMSMemoryWrite == TRUE);
  test_result(error == ERR_NONE);

  test_result(strcmp((const char *)last_command(), "AT+CPMS=\"ME\",\"ME\",\"ME\"\r") == 0);

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

  test_result(strcmp((const char *)last_command(), "AT+CPMS=\"SM\"\r") == 0);
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
  Priv->CPMSReceiveMemory = FALSE;  /* Phone doesn't advertise 3rd parameter */

  Priv->SMSMemory = (GSM_MemoryType)0;
  Priv->SMSMemoryWrite = FALSE;

  /* Test with writeable=TRUE should use 2 parameters */
  type = MEM_ME;
  SET_RESPONSES(responses);
  error = ATGEN_SetRequestedSMSMemory(s, type, TRUE, ID_SetMemoryType);
  test_result(Priv->SMSMemory == MEM_ME);
  test_result(Priv->SMSMemoryWrite == TRUE);
  test_result(error == ERR_NONE);

  test_result(strcmp((const char *)last_command(), "AT+CPMS=\"ME\",\"ME\"\r") == 0);
  cleanup_state_machine(s);
}

void test_three_param_cpms_status_with_ok_only_selection(gboolean receive_me)
{
  GSM_Error error;
  GSM_SMSMemoryStatus status;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  const char *responses[] = {
    "OK\r\n",
    "+CPMS: \"SM\",3,30,\"SM\",3,30,\"SM\",3,30\r\n",
    "OK\r\n",
    "OK\r\n",
    "+CPMS: \"ME\",4,40,\"ME\",4,40,\"ME\",4,40\r\n",
    "OK\r\n"
  };
  bind_response_handling(s);

  puts(__func__);

  parse_memories(s, receive_me
    ? "(\"SM\",\"ME\"),(\"SM\",\"ME\"),(\"SM\",\"ME\")"
    : "(\"ME\",\"SM\",\"MT\"),(\"ME\",\"SM\",\"MT\"),(\"SM\",\"MT\")");
  Priv->SMSMemory = (GSM_MemoryType)0;
  Priv->SMSMemoryWrite = FALSE;

  memset(&status, 0, sizeof(status));
  SET_RESPONSES(responses);
  error = GSM_GetSMSStatus(s, &status);

  test_result(error == ERR_NONE);
  test_result(status.SIMUsed == 3);
  test_result(status.SIMSize == 30);
  test_result(status.PhoneUsed == 4);
  test_result(status.PhoneSize == 40);
  test_result(strcmp((const char *)last_command(), "AT+CPMS?\r") == 0);

  cleanup_state_machine(s);
}

void test_receive_capabilities(void)
{
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  const char *responses[] = { "OK\r\n" };
  const struct {
    const char *capabilities;
    gboolean third;
    gboolean receive_sm;
    gboolean receive_me;
  } cases[] = {
    { "(\"ME\",\"SM\",\"MT\"),(\"ME\",\"SM\",\"MT\"),(\"SM\",\"MT\")", TRUE, TRUE, FALSE },
    { "(\"SM\",\"ME\"),(\"SM\",\"ME\"),(\"SM\",\"ME\")", TRUE, TRUE, TRUE },
    { "(\"SM\",\"ME\"), (\"SM\",\"ME\"), (\"ME\")", TRUE, FALSE, TRUE },
    { "(\"SM\",\"ME\"),(\"SM\",\"ME\")", FALSE, FALSE, FALSE }
  };
  size_t i;
  int legacy, sim, writeable;
  char expected[50];
  const char *memory;
  gboolean receive;

  bind_response_handling(s);
  puts(__func__);
  /* Reuse the state machine to check that capabilities do not leak between replies. */
  for (i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
    parse_memories(s, cases[i].capabilities);
    test_result(Priv->CPMSReceiveMemory == cases[i].third);
    test_result(Priv->CPMSReceiveMemories[MEM_SM] == cases[i].receive_sm);
    test_result(Priv->CPMSReceiveMemories[MEM_ME] == cases[i].receive_me);
    for (legacy = 0; legacy <= 1; legacy++) {
      for (sim = 0; sim <= 1; sim++) {
        for (writeable = 0; writeable <= 1; writeable++) {
          memory = sim ? "SM" : "ME";
          receive = sim ? cases[i].receive_sm : cases[i].receive_me;
          if (!writeable) {
            snprintf(expected, sizeof(expected), "AT+CPMS=\"%s\"\r", memory);
          } else if (receive) {
            snprintf(expected, sizeof(expected), "AT+CPMS=\"%s\",\"%s\",\"%s\"\r", memory, memory, memory);
          } else {
            snprintf(expected, sizeof(expected), "AT+CPMS=\"%s\",\"%s\"\r", memory, memory);
          }
          Priv->SMSMemory = MEM_INVALID;
          Priv->SMSMemoryWrite = FALSE;
          SET_RESPONSES(responses);
          if (legacy) {
            test_result(ATGEN_SetSMSMemory(s, sim, writeable, FALSE) == ERR_NONE);
          } else {
            test_result(ATGEN_SetRequestedSMSMemory(s, sim ? MEM_SM : MEM_ME, writeable, ID_SetMemoryType) == ERR_NONE);
          }
          test_result(strcmp((const char *)last_command(), expected) == 0);
        }
      }
    }
  }
  cleanup_state_machine(s);
}

int main(void)
{
  test_three_param_cpms_enabled();
  test_three_param_cpms_disabled();
  test_three_param_cpms_status_with_ok_only_selection(TRUE);
  test_three_param_cpms_status_with_ok_only_selection(FALSE);
  test_receive_capabilities();
  return 0;
}
