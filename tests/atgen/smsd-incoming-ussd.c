#include <gammu-message.h>
#include "test_helper.h"
#include "../../include/gammu-smsd.h"
#include "../../smsd/core.h"
#include "../../libgammu/gsmstate.h"
#include "../../smsd/services/files.h"

GSM_Error SMSD_ConfigureLogging(GSM_SMSDConfig *Config, gboolean uselog);
GSM_Error SMSD_SendSMS(GSM_SMSDConfig *Config);
void SMSD_IncomingUSSDCallback(GSM_StateMachine *sm, GSM_USSDMessage *ussd, void *user_data);

int main(void)
{
  GSM_Error error;
  GSM_StateMachine *s = setup_state_machine();
  GSM_Phone_ATGENData *Priv = setup_at_engine(s);
  GSM_SMSDStatus status;
  GSM_SMSDConfig *config = SMSD_NewConfig("test");
  GSM_Protocol_Message msg;
  const char *event = "+CUSD: 2";// USSD terminate for issue #464

  const char *responses[] = {
    // AT+CSCS=?
    "+CSCS: (\"IRA\",\"GSM\",\"UCS2\")\r",
    "OK\r\n",
    // AT+CUSD=1
    "OK\r\n",
    // AT+CSCS="UCS2"
    "OK\r\n",
    // AT+CSCS?
    "+CSCS:\r",
    "OK\r\n",
    // USSD DialService
    "+CUSD: 0,\"0076006F00740072006500200072006500710075006500740065002000650073007400200065006E00200063006F0075007200730020006400650020007400720061006900740065006D0065006E0074002E00200076006500750069006C006C0065007A0020006C0069007200650020006C00650020006D00650073007300610067006500200070006C0075007300200074006100720064002E\",15\r\n",
    "OK\r\n",
  };

  UNNEEDED(Priv);

  SET_RESPONSES(responses);
  bind_response_handling(s);

  error = SMSD_ReadConfig("smsd.cfg", config, TRUE);
  test_result(error == ERR_NONE);
  SMSD_EnableGlobalDebug(config);

  memset(&status, 0, sizeof(GSM_SMSDStatus));
  config->Status = &status;

  GSM_SetIncomingUSSDCallback(s, &SMSD_IncomingUSSDCallback, config);
  GSM_SetIncomingUSSD(s, TRUE);

  error = GSM_DialService(s, (char*)"*772*51726568*125*1994#");
  test_result(error == ERR_NONE);
  test_result(status.Received == 1);

  msg.Length = strlen(event);
  msg.Buffer = (char*)event;
  msg.Type = 0;

  s->Phone.Data.RequestMsg = &msg;
  error = ATGEN_DispatchMessage(s);
  test_result(error == ERR_NONE);
  test_result(status.Received == 2);

  SMSD_FreeConfig(config);
}