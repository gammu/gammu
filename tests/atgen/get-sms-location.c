#include <gammu-message.h>
#include "test_helper.h"

#include "../../libgammu/phone/at/atgen.h"
#include "../../libgammu/gsmstate.h"

GSM_Error ATGEN_GetSMSMemories(GSM_StateMachine *s);
GSM_Error ATGEN_GetSMSLocation(GSM_StateMachine *s, GSM_SMSMessage *sms,
    unsigned char *folderid, int *location, gboolean for_write);

void requested_memory(void)
{
  GSM_Error error;
  GSM_SMSMessage sms;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  unsigned char folderid = 0;
  int location = 0;
  const char *responses[] = {
      // GetSMSMemories
      "+CPMS: (\"ME\",\"SM\",\"SR\"),(\"ME\",\"SM\")\r",
      "OK\r\n",
      // SetRequestedSMSMemory
      "+CPMS: 2,4,0,4,0,4\r",
      "OK\r\n",

      "ERROR\r\n", };
  SET_RESPONSES(responses);
  bind_response_handling(s);

  puts(__func__);

  UNNEEDED(Priv);

  memset(&sms, 0, sizeof(sms));
  sms.Memory = MEM_SR;
  sms.Folder = 0;
  sms.Location = 3;

  s->Phone.Data.RequestID = ID_None;
  ATGEN_GetSMSMemories(s);

  error = ATGEN_GetSMSLocation(s, &sms, &folderid, &location, FALSE);
  test_result(error == ERR_NONE);
  test_result(sms.Memory == MEM_SR)
  test_result(location == 3);
}

void computed_memory(void)
{
  GSM_Error error;
  GSM_SMSMessage sms;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  unsigned char folderid = 0;
  int location = 0;
  const char *responses[] = {
      // GetSMSMemories
      "+CPMS: (\"ME\",\"SM\",\"SR\"),(\"ME\",\"SM\")\r",
      "OK\r\n",
      // SetSMSMemory
      "+CPMS: 2,4,0,4,0,4\r",
      "OK\r\n",

      "ERROR\r\n", };
  SET_RESPONSES(responses);
  bind_response_handling(s);

  puts(__func__);

  UNNEEDED(Priv);

  memset(&sms, 0, sizeof(sms));
  sms.Folder = 0;
  sms.Location = 100002;

  s->Phone.Data.RequestID = ID_None;
  ATGEN_GetSMSMemories(s);

  error = ATGEN_GetSMSLocation(s, &sms, &folderid, &location, FALSE);
  test_result(error == ERR_NONE);
  test_result(sms.Memory == MEM_ME)
  test_result(location == 2);
}

int main(void)
{
  requested_memory();
  computed_memory();
}
