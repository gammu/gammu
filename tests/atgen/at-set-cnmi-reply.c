#include <stddef.h>
#include <string.h>
#include "test_helper.h"
#include "gammu-error.h"
#include "gammu-statemachine.h"
#include "../../libgammu/protocol/protocol.h"
#include "../../libgammu/gsmstate.h"

extern GSM_Error ATGEN_ReplyGetCNMIMode(GSM_Protocol_Message *msg, GSM_StateMachine * s);

int main(void) {
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_Protocol_Message msg;
  GSM_Error error;
  static const int setCNMI[] = { 5, 4, 3, 2, 1 };
  const unsigned char *response = "AT+CNMI=?\r+CNMI: (0-5),(0-5),(0-5),(0-5),(0-5)\rOK\r\n";

  msg.Type = 0;
  msg.Length = strlen(response);
  msg.Buffer = (char*)response;
  SplitLines(msg.Buffer, msg.Length, &Priv->Lines, "\x0D\x0A", 2, "\"", 1, TRUE);

  memcpy(s->CurrentConfig->CNMIParams, setCNMI, sizeof(setCNMI) * sizeof(int));
  error = ATGEN_ReplyGetCNMIMode(&msg, s);

  printf("%d,%d,%d,%d,%d\n",
	 Priv->CNMIMode,
	 Priv->CNMIProcedure,
	 Priv->CNMIBroadcastProcedure,
         Priv->CNMIDeliverProcedure,
	 Priv->CNMIClearUnsolicitedResultCodes);

  test_result(error == ERR_NONE);
  test_result(Priv->CNMIMode == 5);
  test_result(Priv->CNMIProcedure == 4);
  test_result(Priv->CNMIBroadcastProcedure == 3);
  test_result(Priv->CNMIDeliverProcedure == 2);
  test_result(Priv->CNMIClearUnsolicitedResultCodes == 1);
}
