#include <stddef.h>
#include <string.h>
#include "test_helper.h"
#include "gammu-error.h"
#include "gammu-statemachine.h"
#include "../../libgammu/protocol/protocol.h"
#include "../../libgammu/gsmstate.h"

extern GSM_Error ATGEN_ReplyGetCNMIMode(GSM_Protocol_Message *msg, GSM_StateMachine * s);

void set_all_params(void) {
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_Protocol_Message msg;
  GSM_Error error;
  static const int setCNMI[] = { 9, 8, 7, 6, 5 };
  const unsigned char *response = "AT+CNMI=?\r+CNMI: (0-9),(0-9),(0-9),(0-9),(0-9)\rOK\r\n";

  msg.Type = 0;
  msg.Length = strlen(response);
  msg.Buffer = (char*)response;
  SplitLines(msg.Buffer, msg.Length, &Priv->Lines, "\x0D\x0A", 2, "\"", 1, TRUE);

  memcpy(s->CurrentConfig->CNMIParams, setCNMI, sizeof(setCNMI));
  error = ATGEN_ReplyGetCNMIMode(&msg, s);

  printf("%d,%d,%d,%d,%d\n",
         Priv->CNMIMode,
         Priv->CNMIProcedure,
         Priv->CNMIBroadcastProcedure,
         Priv->CNMIDeliverProcedure,
         Priv->CNMIClearUnsolicitedResultCodes);

  test_result(error == ERR_NONE);
  test_result(Priv->CNMIMode == 9);
  test_result(Priv->CNMIProcedure == 8);
  test_result(Priv->CNMIBroadcastProcedure == 7);
  test_result(Priv->CNMIDeliverProcedure == 6);
  test_result(Priv->CNMIClearUnsolicitedResultCodes == 5);
  cleanup_state_machine(s);
}

void default_set_5th_param_to_0(void) {
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_Protocol_Message msg;
  GSM_Error error;
  static const int setCNMI[] = { 9, 8, 7, 6 };
  const unsigned char *response = "AT+CNMI=?\r+CNMI: (0-9),(0-9),(0-9),(0-9),(0-9)\rOK\r\n";

  msg.Type = 0;
  msg.Length = strlen(response);
  msg.Buffer = (char*)response;
  SplitLines(msg.Buffer, msg.Length, &Priv->Lines, "\x0D\x0A", 2, "\"", 1, TRUE);

  memcpy(s->CurrentConfig->CNMIParams, setCNMI, sizeof(setCNMI));
  s->CurrentConfig->CNMIParams[4] = -1;
  error = ATGEN_ReplyGetCNMIMode(&msg, s);

  printf("%d,%d,%d,%d,%d\n",
         Priv->CNMIMode,
         Priv->CNMIProcedure,
         Priv->CNMIBroadcastProcedure,
         Priv->CNMIDeliverProcedure,
         Priv->CNMIClearUnsolicitedResultCodes);

  test_result(error == ERR_NONE);
  test_result(Priv->CNMIMode == 9);
  test_result(Priv->CNMIProcedure == 8);
  test_result(Priv->CNMIBroadcastProcedure == 7);
  test_result(Priv->CNMIDeliverProcedure == 6);
  test_result(Priv->CNMIClearUnsolicitedResultCodes == 0);
  cleanup_state_machine(s);
}

extern GSM_Error ATGEN_SetCNMI(GSM_StateMachine *s);

void test_fifth_parameter(const char *range, int configured, int expected)
{
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_Protocol_Message msg;
  char buffer[256], command[50];
  const char *responses[] = { "OK\r\n" };
  int i;

  /* Cinterion MC75i capabilities from issue #1206, with variations of <bfr>. */
  snprintf(buffer, sizeof(buffer), "AT+CNMI=?\r\n+CNMI: (0-3),(0,1),(0,2,3),(0,2)%s\r\nOK\r\n", range);
  msg.Type = 0;
  msg.Buffer = buffer;
  msg.Length = strlen(buffer);
  SplitLines(msg.Buffer, msg.Length, &Priv->Lines, "\x0D\x0A", 2, "\"", 1, TRUE);
  for (i = 0; i < 5; i++) {
    s->CurrentConfig->CNMIParams[i] = -1;
  }
  s->CurrentConfig->CNMIParams[4] = configured;
  test_result(ATGEN_ReplyGetCNMIMode(&msg, s) == ERR_NONE);
  test_result(Priv->CNMIClearUnsolicitedResultCodes == expected);

  bind_response_handling(s);
  s->Phone.Data.EnableIncomingSMS = TRUE;
  SET_RESPONSES(responses);
  test_result(ATGEN_SetCNMI(s) == ERR_NONE);
  snprintf(command, sizeof(command), "AT+CNMI=2,1,0,2,%d\r", expected);
  test_result(strcmp((const char *)last_command(), command) == 0);
  cleanup_state_machine(s);
}

int main(void) {
  set_all_params();
  default_set_5th_param_to_0();
  test_fifth_parameter(",(1)", -1, 1);
  test_fifth_parameter(",(0)", -1, 0);
  test_fifth_parameter(",(0,1)", -1, 0);
  test_fifth_parameter(",(0,1)", 1, 1);
  test_fifth_parameter(",(0,1)", 0, 0);
  test_fifth_parameter(",(1)", 0, 1);
  test_fifth_parameter(",(0)", 1, 0);
  test_fifth_parameter("", -1, 0);
}
